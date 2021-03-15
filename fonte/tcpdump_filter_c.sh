#!/bin/bash
sudo tcpdump -l -U -vvv -n -tttt "$@"  | ./filter_c