###

if [[ -z $MYS_DIR ]]; then
    echo "ERROR: MYS_DIR is not defined. Stop sourcing $0"
    return 0
fi

source $MYS_DIR/script/zsh-functions.sh

# Write to .zsh_history immediately
unsetopt INC_APPEND_HISTORY
unsetopt SHARE_HISTORY
setopt INC_APPEND_HISTORY_TIME
