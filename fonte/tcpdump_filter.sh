#!/bin/bash
nohup $SHELL <<EOF > /dev/null &
sudo tcpdump -l -U -vvv -n -tttt -i enp6s0f1 "$@"  | ./filter_c
EOF
