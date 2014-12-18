k=1;
for((i=1; i<=4; i++))
do
  for((j=1; j<=3; j++))
  do
     /home/work/soft/src/build64_release/ecfs/tool/ecfs_format --root_dir=/home/data$k/ --node_id=$i --disk_id=$j --seq_start=1 --seq_end=1000
     k=$[$k+1];
  done
done
