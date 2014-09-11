pid1=`top -n 1 | grep -v "grep" | grep "node" | cut -d' ' -f2`
echo "Process with -15 priority on OOMK: $pid1"
echo -15 > /proc/$pid1/oom_adj
