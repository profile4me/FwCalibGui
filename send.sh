#!/bin/bash



# for hld in `cat task.list`
# do
	# comm="./analysisDST $hld"
	# command $comm
# done

#count number of job arrays

submissiondir="/lustre/nyx/hades/user/dborisen/dstProduction"
stop=`cat ${submissiondir}/task.list | wc -l`
resources="--mem=500 --time=0-0:15:00"
pathoutputlog="${submissiondir}/dst_mar19/log"
jobscript="job.sh"
cp ${submissiondir}/../TreeList/$jobscript ${submissiondir}
jobarrayFile="task.list"
app="analysisDST"
cp ${submissiondir}/../TreeList/$app ${submissiondir}
pidTxt="pids.list"   # file with two colomns. First - jobArrayId; second - pid of slurm job

> ${submissiondir}/${pidTxt}
for arrayStartPos in `cat ${submissiondir}/${jobarrayFile} | sed -n '/#[0-9]*/='`
do
	numOfTasks=`cat ${submissiondir}/${jobarrayFile} | tail -n +$((arrayStartPos+1)) | sed -n '{p; :loop n; /#[0-9]*/q; p; b loop}' | wc -l`
	command="--array=1-${numOfTasks} ${resources} -D ${submissiondir}  --output=${pathoutputlog}/slurm-%A_%a.out ${submissiondir}/$jobscript ${submissiondir}/$app ${submissiondir}/${jobarrayFile} ${arrayStartPos}"
	arrayId=`cat ${submissiondir}/${jobarrayFile} | sed -n "${arrayStartPos}p" |  sed 's/#\([0-9]*\)/\1/'`
	paste -d'\t' <(echo $arrayId) <(sbatch $command | awk '{print $4;}')  >> ${submissiondir}/${pidTxt}
	# sbatch $command 
	# echo $command
done