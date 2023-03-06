# alias enter_mayeths="MYS_WORLD=....; $MYS_WORLD/project/libmys/script/welcome_to_may_world.sh"

# Enter mayeths' world (tmux), which normally called from enter.sh
# For non default value,
# Define MYS_WORLD then we use HOME=$MYS_WORLD in the tmux
# Define MYS_TMUX then we use it as the tmux, "$MYS_MODDIR/BASE/bin/tmux" and "tmux" otherwise
# Define MYS_TMUX_CONF then we use it as tmux profile, "$MYS_DIR/etc/tmux.conf" otherwise
# Define MYS_TMUX_SOCKET then we use it as tmux socket name, "mayeths-tmux-socket" otherwise
# Define MYS_TMUX_SESSION then we use it as tmux session name, "main" otherwise
welcome_to_may_world() {
    if [[ -n $TMUX ]]; then
        echo "ERROR: Already inside a tmux session ($TMUX)"
        return 1
    fi

    if [[ -n "$MYS_WORLD" ]]; then
        export OLD_HOME="$HOME"
        export HOME="$MYS_WORLD"
        if [[ $($SHELL --version | grep -q bash) -eq 0 ]]; then
            if [[ -f "$HOME/.bashrc" ]]; then
                source "$HOME/.bashrc"
            fi
        elif [[ $($SHELL --version | grep -q zsh) -eq 0 ]]; then
            if [[ -f "$HOME/.zshrc" ]]; then
                source "$HOME/.zshrc"
            fi
        fi
        if [[ $? -ne 0 ]]; then
            export HOME="$OLD_HOME"
            echo "ERROR: Cannot source bashrc or zshrc"
            return 1
        fi
    fi

    if [[ -z "$MYS_DIR" ]]; then
        MYS_DIR=$(dirname $(dirname "${BASH_SOURCE[0]:-$0}")) # BASH_SOURCE works in bash and zsh
    fi
    # [ -n "$MYS_MODDIR" ] || return 1

    [ -n "$(command -v tmux)" ] && THE_TMUX="$(command -v tmux)"
    [ -x "$MYS_MODDIR/BASE/bin/tmux" ] && THE_TMUX="$MYS_MODDIR/BASE/bin/tmux"
    [ -x "$MYS_TMUX" ] && THE_TMUX=$MYS_TMUX

    if [[ "$(echo $($THE_TMUX -V) | awk '{printf $2}' | awk -F '.' '{printf $1}')" -lt 3 ]]; then
        echo "ERROR: Require tmux version > 3 ($(command -v $THE_TMUX) is $($THE_TMUX -V) here)"
        return 1
    fi

    THE_CONF=${MYS_TMUX_CONF:-"$MYS_DIR/etc/tmux.conf"}
    THE_SOCKET=${MYS_TMUX_SOCKET:-"mayeths-tmux-socket"}
    THE_SESSION=${MYS_TMUX_SESSION:-"main"}

    THE_HOST="$MYS_HOST"

    COMMAND="$THE_TMUX -L $THE_SOCKET a || $THE_TMUX -f $THE_CONF -L $THE_SOCKET new -s $THE_SESSION"
    echo "$COMMAND"
    if [[ -z "$THE_HOST" ]]; then
        $COMMAND
    else
        ssh -t "$THE_HOST" "$COMMAND"
    fi
}
