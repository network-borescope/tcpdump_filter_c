#!/bin/bash

DATE=`date "+%M"`
MIN=$DATE

REMAINDER=$((MIN%5))

if [ ${REMAINDER} != 0 ]; then
	exit 0
fi

CHECK_RSYNC=`ps awx | grep -c "ssh \-p 57008 \-i /home/borescope/.ssh/perfsonar_id_rsa .* incomming/pop_df/ttl[s]"`

if [ "$(ls -A ./data)" ] && [ $CHECK_RSYNC -eq 0 ]; then
	echo 1 > /dev/null
	#rsync -u -e "ssh -p 57008 -i /home/borescope/.ssh/perfsonar_id_rsa" data/* datasender@gwrec.cloudnext.rnp.br:incomming/pop_df/ttl
	
	
	
	
	#rsync -e "ssh -p 57008  -o StrictHostKeyChecking=no -i /home/borescope/.ssh/borescope_id_rsa" data/*  ubuntu@gwrec.cloudnext.rnp.br:20210824a/ttls

	# old
	# rsync -e "ssh -p 57008 -i /home/borescope/.ssh/borescope_id_rsa" --removea-source-files data/*  ubuntu@gwrec.cloudnext.rnp.br:20210824a
fi

