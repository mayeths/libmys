# This script find the node cluster by using OSU/p2p_latency.
# Treat nodes that pingpong < 1.80us in the same cluster

if [[ $# -ne 2 ]]; then
    echo "USAGE: $0 <origin_index> <last_index>"
    exit 1
fi

left="$1"
right="$2"
good=$left
bad=$right
pivot=$right

while true; do
    if [[ $pivot -le $left || $pivot -gt $right ]]; then
        echo "Out of range. Already to $pivot."
        exit 0
    fi
    NODE_FIRST=$(python3 -c "print(f'{$left:03d}')")
    NODE_PIVOT=$(python3 -c "print(f'{$pivot:03d}')")
    salloc --exclusive -w bn${NODE_FIRST},bn${NODE_PIVOT} -N 2 --tasks-per-node=1 mpirun -n 2 ./p2p_latency | tee temp.log
    RES=$(grep -P "^4       " temp.log  | awk '{printf $2}')
    python3 -c "exit(0) if $RES < 1.80 else exit(1)"
    RET=$?
    if [[ $RET -eq 0 ]]; then
        good=$pivot
        next=$(($pivot+($bad-$good)/2))
    else
        bad=$pivot
        next=$(($pivot-($bad-$good)/2))
    fi
    if [[ $(($bad-$good)) -eq 1 ]]; then
        echo "The good range is $left to $good"
        exit 0
    fi

    if [[ $next -eq $pivot ]]; then
        echo "Cannot determined next pivot. At $pivot currently."
        exit 0
    fi
    pivot=$next
done
