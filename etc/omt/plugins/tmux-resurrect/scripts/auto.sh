#!/usr/bin/env bash

CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$CURRENT_DIR/variables.sh"
source "$CURRENT_DIR/helpers.sh"

###

auto_interval=$1

while true; do
    sleep $auto_interval
    if [[ $? -ne 0 ]]; then
        break
    fi
    $TMUX_PROGRAM run-shell "$CURRENT_DIR/save.sh"
done
