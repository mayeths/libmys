#!/bin/bash
# Usage: poll_tmux_done.sh <session:window> <tid|-> <deadline_seconds> <interval_seconds>
# Polls the tmux pane for __TASK_<tid>_DONE_<code>__ and reports __TASK_<tid>_STEP<n>_<code>__ markers as they appear.
# If DONE shows up implausibly early, the TID was probably reused in this pane's history: resend with a new TID.
#
# With <tid> = "-", waits for a short command sent as '<command>; echo "__DONE_$?__"'.
# The echoed command line contains the literal __DONE_$?__, the result contains __DONE_<code>__.
# Only output after the last echoed command line counts, so older DONE markers in the history are ignored.
# Start it only after the command line has been echoed (e.g. after the initial sleep 5 + capture), not right after send-keys.
#
# Exit 0: DONE seen. Exit 1: deadline reached. Exit 2: usage or tmux error.

if (( $# != 4 )); then
    echo "Usage: $0 <session:window> <tid|-> <deadline_seconds> <interval_seconds>" >&2
    exit 2
fi

target="$1"
tid="$2"
deadline=$((SECONDS + $3))
interval="$4"
if [[ "$tid" == "-" ]]; then
    pattern="__DONE_[0-9]+__"
else
    pattern="__TASK_${tid}_(STEP[0-9]+|DONE)_[0-9]+__"
fi
seen=""

while true; do
    if ! pane=$(tmux capture-pane -t "$target" -p -J -S - 2>&1); then
        echo "__POLL_ERROR__ ${pane}"
        exit 2
    fi
    search="$pane"
    if [[ "$tid" == "-" ]]; then
        anchor=$(grep -nF '__DONE_$?__' <<< "$pane" | tail -n 1 | cut -d: -f1)
        if [[ -z "$anchor" ]]; then
            echo "__POLL_ERROR__ no command line containing __DONE_\$?__ in ${target}"
            exit 2
        fi
        search=$(tail -n +$((anchor + 1)) <<< "$pane")
    fi
    markers=$(grep -oE "$pattern" <<< "$search" | awk '!seen[$0]++')
    if [[ "$markers" != "$seen" ]]; then
        echo "[${SECONDS}s] ${markers//$'\n'/ }"
        seen="$markers"
    fi
    if grep -qE "_DONE_[0-9]+__" <<< "$markers"; then
        exit 0
    fi
    remaining=$((deadline - SECONDS))
    if (( remaining <= 0 )); then
        echo "__POLL_TIMEOUT__ after ${SECONDS}s"
        grep -v '^$' <<< "$pane" | tail -n 8
        exit 1
    fi
    if (( remaining < interval )); then
        sleep "$remaining"
    else
        sleep "$interval"
    fi
done
