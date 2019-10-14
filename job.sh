#!/bin/bash

echo "JOB SCRIPT HAS bEEN CALLED WITH;"
echo "	 par1 = $1"
echo "	 par2 = $2"
echo "	 par3 = $3"

app=$1
jobarrayFile=$2
arrayStartPos=$3
taskId=$SLURM_ARRAY_TASK_ID
hldFile=`cat $jobarrayFile | sed -n "$(($arrayStartPos+$taskId))p"`

source /cvmfs/hades.gsi.de/install/5.34.34/hydra2-5.1a/defall.sh

comm="time $app $hldFile"
command $comm