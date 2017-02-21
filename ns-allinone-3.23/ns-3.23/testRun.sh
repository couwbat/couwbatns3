#!/bin/bash

for run in {1..100}
do

for sta in {32,16,8,4,2} 
do 

for slot in {1,2,3,4,5,6,7,8}
do

if [ $slot -eq 1 ];then
  d=10
else
  d=5
fi

./run_couwbat_association.sh --silent --nopu --stas=$sta --slots=$slot --plot=1 --d=$d --nrccs=1 --ofname="all.csv"

done
done
done
