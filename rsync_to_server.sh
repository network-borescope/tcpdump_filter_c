#!/bin/bash

if [ "$(ls -A ./data)" ]; then
	echo 1 > /dev/null
	rsync -e "ssh -p 57005 -i /home/borescope/.ssh/borescope_id_rsa" --remove-source-files data/*  ubuntu@gwrec.cloudnext.rnp.br:2021_03_05_c
fi

