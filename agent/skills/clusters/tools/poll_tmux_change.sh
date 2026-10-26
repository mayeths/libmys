#!/bin/bash
# Usage: poll_tmux_change.sh <session:window> <deadline_seconds> <interval_seconds> <settle_rounds> <keyword>
# For commands that switch shells (ssh, salloc, srun --pty) and cannot append a DONE marker.
# Send the command first, then run this script with the command name as <keyword>.
#
# Returns only when both conditions hold:
#   1. Command finished: the last non-empty line no longer contains <keyword>.
#      The echoed command line and salloc's own "salloc: ..." lines contain it; the next shell prompt does not.
#      Pick a keyword that never appears in the prompt (e.g. "ssh", not the node name).
#   2. Output settled: the pane stayed unchanged for <settle_rounds> consecutive polls.
#      Covers output that keeps arriving after the keyword is gone, e.g. the multi-part login banner of ssh,
#      and output that has not arrived yet when polling starts.
#
# Settled does not mean succeeded: read the printed tail (prompt, error, password prompt, ...).
# Exit 0: settled. Exit 1: deadline reached. Exit 2: usage or tmux error.

if (( $# != 5 )); then
    echo "Usage: $0 <session:window> <deadline_seconds> <interval_seconds> <settle_rounds> <keyword>" >&2
    exit 2
fi

target="$1"
deadline=$((SECONDS + $2))
interval="$3"
settle_rounds="$4"
keyword="$5"
previous=""
stable=0

while true; do
    if ! current=$(tmux capture-pane -t "$target" -p -J -S -100 2>&1); then
        echo "__POLL_ERROR__ ${current}"
        exit 2
    fi
    current=$(grep -v '^$' <<< "$current")
    # Condition 2: output settled
    if [[ "$current" == "$previous" ]]; then
        stable=$((stable + 1))
    else
        stable=0
        previous="$current"
    fi
    # Condition 1: command finished
    last_line=$(tail -n 1 <<< "$current")
    if ! grep -qF -- "$keyword" <<< "$last_line"; then
        if (( stable >= settle_rounds )); then
            echo "__SETTLED__ after ${SECONDS}s"
            tail -n 8 <<< "$current"
            exit 0
        fi
    fi
    remaining=$((deadline - SECONDS))
    if (( remaining <= 0 )); then
        echo "__POLL_TIMEOUT__ after ${SECONDS}s"
        tail -n 8 <<< "$current"
        exit 1
    fi
    if (( remaining >= interval )); then
        sleep "$interval"
    else
        sleep "$remaining"
    fi
done
