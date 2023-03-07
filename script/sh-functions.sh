
mys_backup_command() {
    local s
    $SHELL --version | grep -q bash && s=bash
    $SHELL --version | grep -q zsh && s=zsh
    local t
    [[ $s == "bash" ]] && t=$(type -t $1)
    [[ $s == "zsh"  ]] && t=$(whence -w $1 | awk '{ print $NF }')
    if [[ $t == "function" ]]; then
        eval "$2() $(declare -f $1 | sed 1d)"
    elif [[ $t == "builtin" || $t == "file" || $t == "command" ]]; then
        eval "$2() { command $1 \"\$@\" }"
    elif [[ $t == "alias" ]]; then
        local p
        [[ "$s" == "bash" ]] && p="$2"
        [[ "$s" == "zsh" ]] && p="alias $2"
        eval $(alias $1 | awk "NR==1,/$1/{sub(/$1/, \"$p\")} 1")
    else # "keyword" or "reserved"
        return 1
    fi
}

mys_load_script() {
    local path
    for path in "$@"; do
        [[ -f "$path" ]] && source $path
    done
}

mys_realpath() {
    echo "$(cd "$(dirname -- "$1")" >/dev/null; pwd -P)/$(basename -- "$1")"
}
