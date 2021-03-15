#!bin/bash
APP="tcpdump_filter_c.sh"
count=`ps awx | grep -c ./filter_c`
echo $count
if  [ $count -lt 2 ]
then
   echo "Starting $APP"
   cd /home/borescope/tools
   sudo bash $APP
   echo "done"
else
   echo "Running"
fi

