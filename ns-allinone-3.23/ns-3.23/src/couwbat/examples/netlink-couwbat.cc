#include <netlink/netlink.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/couwbat-module.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/applications-module.h"

/**
 * \file
 * \ingroup examples
 * 
 * netlink-couwbat is a wrapper for the Couwbat module which allows it to run standalone and communicate with the printk-only AED PHY kernel module
 */

/*
 * *****************************
 * COUWBAT NETLINK PROTOCOL HEADERS/DEFINITIONS
 * *****************************
 */

#define CW_CMD_DEBUG_ECHO 0x02
#define CW_CMD_DEBUG_DUMP 0x03

enum {
        CW_CMD_ATTR_UNSPEC = 0,
        CW_CMD_ATTR_FLAGS,
        CW_CMD_ATTR_FREQUENCY_BAND,
        CW_CMD_ATTR_OFDM_SYM_SFRAME_COUNT,
        CW_CMD_ATTR_OFDM_SYM_OFFSET,
        CW_CMD_ATTR_OFDM_SYM_LEN,
        CW_CMD_ATTR_MCS,
        CW_CMD_ATTR_CQI,
        CW_CMD_ATTR_RESERVED,
        CW_CMD_ATTR_PAYLOAD,
        CW_CMD_ATTR_DEBUG_STRING,
        __CW_CMD_ATTR_MAX,
};
#define CW_CMD_ATTR_MAX (__CW_CMD_ATTR_MAX-1)

static struct nla_policy cw_att_genl_policy[CW_CMD_ATTR_MAX+1];
static void init_cw_att_genl_policy()
{
  // Init policies
  struct nla_policy temp;
  temp.type = NLA_U32;
  cw_att_genl_policy[CW_CMD_ATTR_FLAGS] = temp;
  cw_att_genl_policy[CW_CMD_ATTR_OFDM_SYM_SFRAME_COUNT] = temp;
  temp.type = NLA_U16;
  cw_att_genl_policy[CW_CMD_ATTR_FREQUENCY_BAND] = temp;
  cw_att_genl_policy[CW_CMD_ATTR_OFDM_SYM_OFFSET] = temp;
  cw_att_genl_policy[CW_CMD_ATTR_OFDM_SYM_LEN] = temp;
  cw_att_genl_policy[CW_CMD_ATTR_RESERVED] = temp;
  temp.type = NLA_U8;
  cw_att_genl_policy[CW_CMD_ATTR_MCS] = temp;
  cw_att_genl_policy[CW_CMD_ATTR_CQI] = temp;
  cw_att_genl_policy[CW_CMD_ATTR_PAYLOAD] = temp;
  temp.type = NLA_STRING;
  cw_att_genl_policy[CW_CMD_ATTR_DEBUG_STRING] = temp;
}


using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("NetlinkCouwbatComponent");


/*
 * *****************************
 * STATIC VARIABLES
 * *****************************
 */

// Program argument values
static bool arg_sta = false; // If set, the MAC will be CR-STA type, else CR-BS.
static bool arg_tbpu_enabled = true; // If set, a TraceBasedPuNetDevice will be created
static std::string arg_tbpu_filename = "../../traces/500_1012MHz_skopje_0_4am.csv";

// Netlink variables
static struct nl_sock *nl_socket;
static int fam_id;
static Callback<void,Ptr<Packet> > mac_rx_cb;

// Variable for communication with upper layers (applications)
#define APP_IN_PORT 43765
#define APP_OUT_PORT 43766
#define APP_BUF_SIZE 5000
static uint8_t app_in_buf[APP_BUF_SIZE];
static uint8_t app_out_buf[APP_BUF_SIZE];
static struct sockaddr_in app_addr_self;
static struct sockaddr_in app_addr_out;
static int app_in_sockfd;
static int app_out_sockfd;
pthread_t app_listener_thread;


// Persistent ns3 objects
static NodeContainer specDbNodes;
static NodeContainer puNodes;
static Ptr<CouwbatMac> mac;
static Ptr<TraceBasedPuNetDevice> tbpu;


/*
 * *****************************
 * FUNCTION PROTOTYPES
 * *****************************
 */

static void parse_args(int argc, char *argv[]);
static int init_netlink();
static int init_app();
static int init_couwbat();
static int cwnl_phy_rx_cb(struct nl_msg *msg, void *arg);
static int cwnl_couwbat_send (CouwbatMetaHeader mh, Ptr<Packet> packet);
static void cwnl_forwardup_cb (Ptr<Packet> packet, Mac48Address from, Mac48Address to);
static void *cwnl_app_listener (void *);


/*
 * *****************************
 * IMPLEMENTATION
 * *****************************
 */

int main (int argc, char *argv[])
{
  parse_args(argc, argv);

  int retVal = init_netlink();
  if (retVal)
    {
      printf("Failed to initialize netlink, aborting\n");
      return -1;
    }

  retVal = init_app();
  if (retVal)
    {
      printf("Failed to initialize application sockets, aborting\n");
      return -2;
    }

  init_couwbat();

  int sf_cnt = 0;
  printf("\n\nnetlink-couwbat running in testing mode: generating SF_START for MAC every 5 seconds, do not receive any netlink messages on socket\n");
  while (1)
    {
      ////////////////////
      // For testing purposes generate SF_START and give to MAC
      ////////////////////
      printf("\n=====================================================================\n");
      printf("== Starting SF %d\n", sf_cnt);
      printf(  "=====================================================================\n");
      Ptr<Packet> zeroSFpacket = Create<Packet> ();
      CouwbatMetaHeader mh;
      mh.m_flags = CW_CMD_WIFI_EXTRA_ZERO_SF_START;
      mh.m_ofdm_sym_sframe_count = sf_cnt++;
      zeroSFpacket->AddHeader(mh);
      mac_rx_cb (zeroSFpacket);

      usleep(5000000); // 5 sec

      tbpu->Update (); // Do an update tick in TraceBasedPu for test
      ////////////////////
      // End test
      ////////////////////

      // What it should be (at the very least?)
//      nl_recvmsgs_default(nl_socket);
    }

  nl_socket_free(nl_socket);
  return 0;
}

static void parse_args(int argc, char *argv[])
{
  CommandLine cmd;
  cmd.AddValue ("sta", "If set, the MAC will be CR-STA type, else CR-BS", arg_sta);

  cmd.Parse (argc, argv);
}

static int init_netlink()
{
  init_cw_att_genl_policy();

  int retVal;
  int retValComb = 0;
  char family[] = "cw0";

  // Allocating socket
  nl_socket = nl_socket_alloc();
  if(nl_socket == NULL) {printf("Error allocating socket\n");};

  // Keep it simple
  nl_socket_disable_seq_check(nl_socket);
  nl_socket_disable_auto_ack(nl_socket);

  // Connecting to Netlink Generic socket
  retVal = genl_connect(nl_socket); //nl_connect(sk, NETLINK_GENERIC);
  if(retVal < 0) {printf("Error conecting to the socket. Error Code: %d\n",retVal); retValComb |= retVal;}

  // Finding the family identifier, not needed in this example but it is useful to know how to do it.
  fam_id = genl_ctrl_resolve(nl_socket,family);
  if(fam_id < 0) {printf("Error resolving cw0 family. Error Code: %d\n",fam_id); retValComb |= fam_id;}
  else{printf("cw0 family ID: %d\n",fam_id);}

  // Find the multicast group identifier and register ourselves to it.
  int group = genl_ctrl_resolve_grp(nl_socket, family, "cw_broadcast_0");
  if(group < 0) {printf("Error resolving cw_broadcast_0 group. Error Code: %d\n",group); retValComb |= group;}
  else{printf("cw_broadcast_0 group is %u.\n", group);}

  retVal = nl_socket_add_memberships(nl_socket, group, 0);
  retValComb |= retVal;
  if (retVal) {printf("nl_socket_add_memberships() failed: %d\n", retVal);}

  // Call to modify default callback function
  retVal = nl_socket_modify_cb(nl_socket, NL_CB_MSG_IN, NL_CB_CUSTOM , cwnl_phy_rx_cb, NULL);
  if(retVal < 0) {printf("Error modifying socket callback. Error Code: %d\n",retVal); retValComb |= retVal;}

  return retValComb;
}

static int init_app()
{
  int retVal = 0;

  // Init application data input socket
  app_in_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (app_in_sockfd == -1)
    {
      printf("Error with socket app_in_sockfd: %d\n", app_in_sockfd);
      retVal |= app_in_sockfd;
    }

  memset((char *) &app_addr_self, 0, sizeof(app_addr_self));
  app_addr_self.sin_family = AF_INET;
  app_addr_self.sin_port = htons(APP_IN_PORT);
  app_addr_self.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(app_in_sockfd, (struct sockaddr *) &app_addr_self, sizeof(app_addr_self)) == -1)
    {
      printf("Error with bind app_in_sockfd");
      retVal |= -1;
    }


  // Init application data output socket
  app_out_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (app_out_sockfd == -1)
    {
      printf("Error with socket app_out_sockfd: %d\n", app_out_sockfd);
      retVal |= app_out_sockfd;
    }

  memset((char *) &app_addr_out, 0, sizeof(app_addr_out));
  app_addr_out.sin_family = AF_INET;
  app_addr_out.sin_port = htons(APP_OUT_PORT);
  app_addr_out.sin_addr.s_addr = inet_addr("127.0.0.1");


  // Init application data input socket listener thread
  int tmp = pthread_create(&app_listener_thread, NULL, cwnl_app_listener, NULL);
  if (tmp != 0)
    {
      printf("Error with pthread_create for listener: %d\n", tmp);
      retVal |= tmp;
    }

  return retVal;
}

static int init_couwbat()
{
  LogComponentEnable ("BsCouwbatMac", LOG_LEVEL_ALL);
  LogComponentEnable ("BsCouwbatMac", LOG_PREFIX_LEVEL);

  LogComponentEnable ("StaCouwbatMac", LOG_LEVEL_ALL);
  LogComponentEnable ("StaCouwbatMac", LOG_PREFIX_LEVEL);

  LogComponentEnable ("TraceBasedPuNetDevice", LOG_LEVEL_INFO);
  LogComponentEnable ("TraceBasedPuNetDevice", LOG_PREFIX_LEVEL);

  Packet::EnablePrinting ();

  CouwbatHelper couwbat;

  // SpecDb and PU
  specDbNodes.Create (1);
  couwbat.InstallSpectrumDb (specDbNodes.Get (0));

  if (arg_tbpu_enabled)
    {
      puNodes.Create (1);
      Ptr<Node> pu = puNodes.Get (0);
      tbpu = couwbat.InstallTraceBasedPu (pu, 0, arg_tbpu_filename, Time (), 1);
      tbpu->Start (1);
    }

  // MAC
  if (arg_sta)
    {
      mac = CreateObject<StaCouwbatMac> ();
    }
  else
    {
      mac = CreateObject<BsCouwbatMac> ();
    }

  mac->SetAddress (Mac48Address::Allocate ());
  mac->SetNetlinkMode (true);
  mac->Initialize ();

  Callback<int, CouwbatMetaHeader, Ptr<Packet> > mac_send_cb = MakeCallback(cwnl_couwbat_send);

  Ptr<BsCouwbatMac> bsMac = mac->GetObject<BsCouwbatMac> ();
  // add a spectrum manager to the device if the mac is a base station
  if (bsMac)
    {
      Ptr<SpectrumManager> spectrumManager = CreateObject<SpectrumManager> ();
      Ptr<SpectrumDb> specDb = specDbNodes.Get (0)->GetObject<SpectrumDb> ();
      spectrumManager->SetSpectrumDb (specDb);
      bsMac->SetSpectrumManager (spectrumManager);

      bsMac->SetCwnlSendCallback (mac_send_cb);
      mac_rx_cb = bsMac->GetRxOkCallback ();
    }

  Ptr<StaCouwbatMac> staMac = mac->GetObject<StaCouwbatMac> ();
  if (staMac)
    {
      staMac->SetCwnlSendCallback (mac_send_cb);
      mac_rx_cb = staMac->GetRxOkCallback ();
    }

  Callback<void, Ptr<Packet>, Mac48Address, Mac48Address> mac_forwardup_cb = MakeCallback(cwnl_forwardup_cb);
  mac->SetNetlinkForwardUpCallback (mac_forwardup_cb);

  return 0;
}

static int cwnl_phy_rx_cb(struct nl_msg *msg, void *arg){
  printf("cwnl_phy_rx_cb enter\n");

  struct nlmsghdr *nlhdr = reinterpret_cast<nlmsghdr *>(nlmsg_hdr(msg));
  struct genlmsghdr *genlhdr = reinterpret_cast<genlmsghdr *>(nlmsg_data(nlhdr));
  struct nlattr *attrs[CW_CMD_ATTR_MAX+1];

  unsigned int i;

  CouwbatMetaHeader mh;
  int payload_len = 0;
  Ptr<Packet> packet;

  genlmsg_parse(nlhdr, 0, attrs, CW_CMD_ATTR_MAX, cw_att_genl_policy);

  printf("-------------------------- BEGIN RECEIVED NETLINK MESSAGE ---------------------------\n");

  if (nlhdr)
    {
      printf("nl_hdr_nlmsg_len: %d \n", nlhdr->nlmsg_len);
      printf("nl_hdr_nlmsg_type: %d \n", nlhdr->nlmsg_type);
      printf("nl_hdr_nlmsg_flags: %d \n", nlhdr->nlmsg_flags);
      printf("nl_hdr_nlmsg_seq: %u \n", nlhdr->nlmsg_seq);
      printf("nl_hdr_nlmsg_pid: %u \n", nlhdr->nlmsg_pid);
    }
  else
    {
      printf("NO nlhdr.\n");
    }

  if (genlhdr)
    {
      printf("genlhdr_cmd: %d \n", genlhdr->cmd);
      printf("genlhdr_version: %d \n", genlhdr->version);
      printf("genlhdr_reserved: %d \n", genlhdr->reserved);
    }
  else
    {
      printf("NO genlhdr.\n");
    }

  if(attrs[CW_CMD_ATTR_UNSPEC                 ]   != NULL)
    {
      uint32_t val = nla_get_u32(attrs[CW_CMD_ATTR_UNSPEC]);
      printf("CW_CMD_ATTR_UNSPEC: %u \n"                    ,val);
    }
  if(attrs[CW_CMD_ATTR_FLAGS                  ]   != NULL)
    {
      uint32_t val = nla_get_u32(attrs[CW_CMD_ATTR_FLAGS]);
      mh.m_flags = val;
      printf("CW_CMD_ATTR_FLAGS: %u \n"                     ,val);
    }
  if(attrs[CW_CMD_ATTR_FREQUENCY_BAND         ]   != NULL)
    {
      uint16_t val = nla_get_u16(attrs[CW_CMD_ATTR_FREQUENCY_BAND]);
      mh.m_frequency_band = val;
      printf("CW_CMD_ATTR_FREQUENCY_BAND: %u \n"            ,val);
    }
  if(attrs[CW_CMD_ATTR_OFDM_SYM_SFRAME_COUNT  ]   != NULL)
    {
      uint32_t val = nla_get_u32(attrs[CW_CMD_ATTR_OFDM_SYM_SFRAME_COUNT]);
      mh.m_ofdm_sym_sframe_count = val;
      printf("CW_CMD_ATTR_OFDM_SYM_SFRAME_COUNT: %u \n"     ,val);
    }
  if(attrs[CW_CMD_ATTR_OFDM_SYM_OFFSET        ]   != NULL)
    {
      uint16_t val = nla_get_u16(attrs[CW_CMD_ATTR_OFDM_SYM_OFFSET]);
      mh.m_ofdm_sym_offset = val;
      printf("CW_CMD_ATTR_OFDM_SYM_OFFSET: %u \n"           ,val);
    }
  if(attrs[CW_CMD_ATTR_OFDM_SYM_LEN           ]   != NULL)
    {
      uint16_t val = nla_get_u16(attrs[CW_CMD_ATTR_OFDM_SYM_LEN]);
      mh.m_ofdm_sym_len = val;
      printf("CW_CMD_ATTR_OFDM_SYM_LEN: %u \n"              ,val);
    }
  if(attrs[CW_CMD_ATTR_MCS                    ]   != NULL)
    {
      nla_memcpy(mh.m_MCS, attrs[CW_CMD_ATTR_MCS], Couwbat::MAX_SUBCHANS);
      printf("CW_CMD_ATTR_MCS: {[%u]=%u", 0, mh.m_MCS[0]);
      for(i = 1; i < Couwbat::MAX_SUBCHANS; ++i)
        {
          printf(",[%u]=%u", i, mh.m_MCS[i]);

          if (mh.m_MCS[i] != COUWBAT_MCS_SUBCARRIER_NOT_AVAILABLE)
            {
              mh.m_allocatedSubChannels.set (i, true);
            }
        }
      printf("}\n");
    }
  if(attrs[CW_CMD_ATTR_CQI                    ]   != NULL)
    {
    nla_memcpy(mh.m_CQI, attrs[CW_CMD_ATTR_CQI], Couwbat::MAX_SUBCHANS);
    printf("CQI Values: {[%u]=%u", 0, mh.m_CQI[0]);
    for(i = 1; i < Couwbat::MAX_SUBCHANS; ++i)
      {
        printf(",[%u]=%u", i, mh.m_CQI[i]);
      }
    printf("}\n");
  }
  if(attrs[CW_CMD_ATTR_RESERVED               ]   != NULL)
    {
      uint16_t val = nla_get_u16(attrs[CW_CMD_ATTR_RESERVED]);
      mh.m_reserved = val;
      printf("CW_CMD_ATTR_RESERVED: %u \n"                    ,val);
    }
  if(attrs[CW_CMD_ATTR_PAYLOAD                ]   != NULL)
    {
      payload_len = nla_len(attrs[CW_CMD_ATTR_PAYLOAD]);
      printf("CW_CMD_ATTR_PAYLOAD size: %d \n"                ,payload_len);
    }
  if(attrs[CW_CMD_ATTR_DEBUG_STRING           ]   != NULL)
    {
      char *val = reinterpret_cast<char *>(nla_data(attrs[CW_CMD_ATTR_DEBUG_STRING]));
      printf("CW_CMD_ATTR_DEBUG_STRING: %s \n"                ,val);
    }

  printf("---------------------------  END RECEIVED NETLINK MESSAGE  ---------------------------\n");

  if(attrs[CW_CMD_ATTR_PAYLOAD                ]   != NULL)
    {
      uint8_t *payload = reinterpret_cast<uint8_t *>(nla_data(attrs[CW_CMD_ATTR_PAYLOAD]));
      packet = Create<Packet> (payload, payload_len);

      // Call MAC callback
      packet->AddHeader(mh);
      mac_rx_cb (packet);
    }

  printf("cwnl_phy_rx_cb exit\n\n");
  return 0;
}

static int cwnl_couwbat_send (CouwbatMetaHeader mh, Ptr<Packet> packet)
{
  printf("cwnl_couwbat_send enter\n");
  int retVal = 0;

  // Allocate Payload
  int payload_size = packet->GetSize ();
  uint8_t *payload = NULL;
  if (payload_size > 0)
    {
      payload = new (std::nothrow) uint8_t[payload_size];
      if (!payload)
        {
          printf("Memory allocation for payload buffer failed.\n");
          return -1;
        }

      packet->CopyData (payload, payload_size);
    }

  //------------------------------------------------------------------//
  //                                                                  //
  //                            MSG CONSTRUCTION                      //
  //                                                                  //
  //------------------------------------------------------------------//

  struct nl_msg *msg = nlmsg_alloc();

  // Using CW_CMD_DEBUG_DUMP as dummy due to command/flags confusion and lack of CW_CMD_WIFI_EXTRA_TX
  genlmsg_put(msg,NL_AUTO_PORT,NL_AUTO_SEQ,fam_id,0,0,CW_CMD_DEBUG_DUMP,1);

  nla_put_u32(msg, CW_CMD_ATTR_FLAGS, (uint32_t) mh.m_flags);
  nla_put_u16(msg, CW_CMD_ATTR_FREQUENCY_BAND, (uint16_t) mh.m_frequency_band);
  nla_put_u32(msg, CW_CMD_ATTR_OFDM_SYM_SFRAME_COUNT, (uint32_t) mh.m_ofdm_sym_sframe_count);
  nla_put_u16(msg, CW_CMD_ATTR_OFDM_SYM_OFFSET, (uint16_t) mh.m_ofdm_sym_offset);
  nla_put_u16(msg, CW_CMD_ATTR_OFDM_SYM_LEN, (uint16_t) mh.m_ofdm_sym_len);
  nla_put(msg, CW_CMD_ATTR_MCS, Couwbat::MAX_SUBCHANS, mh.m_MCS);
  // CQI not applicable for MAC->PHY communication, skip adding this attribute?
  // reserved currently has no purpose, skip adding this attribute?

  // debug description
  retVal = nla_put_string(msg,CW_CMD_ATTR_DEBUG_STRING, "FROM USER TO KERNEL, sent by ns3 Netlink Mode MAC");
  if (retVal)
    {
      printf("Error at nla_put_string. Error Code %d, Error Description: %s\n", retVal, nl_geterror (retVal));
    }

  if (payload_size > 0)
    {
      retVal = nla_put(msg, CW_CMD_ATTR_PAYLOAD, payload_size, payload);
      if (retVal)
        {
          printf("Error at nla_put (payload). Error Code %d, Error Description: %s\n", retVal, nl_geterror (retVal));
        }
    }

  //Send message
  retVal = nl_send_auto(nl_socket, msg);
  if (retVal < 0)
    {
      printf("Error with nl_send_auto(). Error Code: %d, Error Description: %s\n", retVal, nl_geterror (retVal));
    }
  else
    {
      printf("Message SENT.\n");
    }

  //------------------------------------------------------------------//
  //                                                                  //
  //                            RELEASE RESOURCES                     //
  //                                                                  //
  //------------------------------------------------------------------//

  delete[] payload;
  nlmsg_free(msg);
  printf("cwnl_couwbat_send exit\n");
  return retVal;
}

static void cwnl_forwardup_cb (Ptr<Packet> packet, Mac48Address from, Mac48Address to)
{
  int size = packet->CopyData(app_out_buf, APP_BUF_SIZE);

  if (sendto(app_out_sockfd, app_out_buf, size, 0, (struct sockaddr *) &app_addr_out, sizeof(app_addr_out)) == -1)
    {
      printf("Error with sendto() call in cwnl_forwardup_cb()\n");
      return;
    }
}

static void *cwnl_app_listener (void *)
{
  int recv_size = 0;
  struct sockaddr_in recv_src;

  while (1)
    {
      socklen_t addr_len = sizeof(recv_src);
      recv_size = recvfrom(app_in_sockfd, (void *) app_in_buf, APP_BUF_SIZE, 0, (struct sockaddr *) &recv_src, &addr_len);
      if (recv_size < 1)
        {
          printf("Error with recvfrom() call in cwnl_app_listener()\n");
          continue;
        }

      Ptr<Packet> packet = Create<Packet> (app_in_buf, recv_size);
      Mac48Address to; // TODO MAC address is not set correctly here
      mac->Enqueue (packet, to);
    }

  return 0;
}
