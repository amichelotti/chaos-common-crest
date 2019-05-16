#!/bin/sh
## defaults
#10.10.6.131 chaost-webui2
#10.10.5.131 chaos-webui2
server="10.10.5.131:8081"
#server="10.10.6.131:8081"
name="DAFNE/LUMINOMETER/HET/HETP"
if [ -n "$1" ];then
    ## name
    name=$1
fi
if [ -n "$2" ];then
    ## server
    server=$2
fi
while true;do
date
echo "* launching acquisition as $name on $server"
./hetAcquire -n $name -s $server -c 2
echo "* exiting"

done    
