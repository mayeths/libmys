#!/bin/bash

[[ $# -ne 5 ]] && echo "[ERROR] Usage: $0 NMACHINE NPROC NTHREAD EXE RUN_DIR" && exit 1
export NMACHINE=$1
export NPROC=$2
export NTHREAD=$3
export EXE=$4
export RUN_DIR=$5

module purge
module load TODO

# TYPE 1: Generate rankfile on the fly
export rankgen_cmd="\$(rankgen -S 2 -N 1 -C 64 -t -w \$SLURM_NODELIST $NMACHINE $NPROC $NTHREAD)"
# TYPE 2: Pre-generated rankfile
export NODE_FIRST="101"
export node_first=$(python3 -c "print(f'{$NODE_FIRST:03d}')")
export node_last=$(python3 -c "print(f'{$NODE_FIRST+$NMACHINE-1:03d}')")
export node_range="TODO[$node_first-$node_last]"
export rankfile="./rf18-${NMACHINE}N/${NPROC}P${NTHREAD}T.rf"
rankgen -q -w "$node_range" -S 2 -N 2 -C 32 -o $rankfile $NMACHINE $NPROC $NTHREAD

SOPT=""
SOPT+=" --time=4:30"
SOPT+=" --exclusive"
SOPT+=" -N $NMACHINE"
SOPT+=" -w $node_range"

MOPT=""
MOPT+=" -n $NPROC"
MOPT+=" -rf $rankfile"
MOPT+=" -x NPB_TIMER_FLAG=true"
MOPT+=" -x OMP_NUM_THREADS=$NTHREAD"
MOPT+=" -x GPTL_OUTDIR=$RUN_DIR/GPTL"

OPT=""

STDOUT=$RUN_DIR/stdout.log
STDERR=$RUN_DIR/stderr.log
mkdir -p $RUN_DIR
rm -f $STDOUT $STDERR

echo ">>>>>> Running salloc $SOPT mpirun $MOPT $EXE $OPT" | tee $RUN_DIR/invoke.log
module list >> $RUN_DIR/invoke.log 2>&1
ldd $EXE >> $RUN_DIR/invoke.log
env >> $RUN_DIR/invoke.log
cat $0 >> $RUN_DIR/invoke.log

salloc $SOPT mpirun $MOPT $EXE $OPT > >(tee -a $STDOUT) 2> >(tee -a $STDERR >&2)