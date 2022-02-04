#!/bin/bash
internal_ether="cc:4e:24:42:55:0d"
nohup $SHELL <<EOF > /dev/null &
sudo tcpdump -l -U -vvv -n -tttt -i bond1 ether src ${internal_ether}  "$@"  | ./filter_c
EOF
