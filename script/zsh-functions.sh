###

### https://stackoverflow.com/a/24522107
# SYNOPSIS: env_contain varName fieldVal [sep]
#   SEP defaults to ':'
# Note: Find if a substring is in path-like environment variable
# EXAMPLE: env_contain PATH /usr/local/bin
env_contain() {
    [[ $# -ge 1 && $# -le 3 ]] || return 1
    local varName="$1" fieldVal="$2" IFS=${3:-':'}
    local auxArr=($(eval printf '%s' "\"\$${varName}\""))
    for i in {1..$#auxArr}; do
        [[ ${auxArr[i]} == "$fieldVal" ]] && return 0
    done
    return 1
}

# SYNOPSIS: env_prepend varName fieldVal [sep]
#   SEP defaults to ':'
# Note: Forces fieldVal into the first position, if already present.
#       Duplicates and empties are removed, too.
# EXAMPLE: env_prepend PATH /usr/local/bin
env_prepend() {
    [[ $# -ge 1 && $# -le 3 ]] || return 1
    local varName="$1" fieldVal="$2" IFS=${3:-':'}
    local auxArr=($(eval printf '%s' "\"\$${varName}\""))
    for i in {1..$#auxArr}; do
        [[ ${auxArr[i]} == "$fieldVal" ]] && unset 'auxArr[i]'
    done
    auxArr=( "$fieldVal" "${auxArr[@]}" )
    printf -v "$varName" '%s' "${(@j.:.)auxArr:#}" # https://unix.stackexchange.com/a/590047
    export "$varName"
}

# SYNOPSIS: env_append varName fieldVal [sep]
#   SEP defaults to ':'
# Note: Forces fieldVal into the last position, if already present.
#       Duplicates and empties are removed, too.
# EXAMPLE: env_append PATH /usr/local/bin
env_append() {
    [[ $# -ge 1 && $# -le 3 ]] || return 1
    local varName="$1" fieldVal="$2" IFS=${3:-':'}
    local auxArr=($(eval printf '%s' "\"\$${varName}\""))
    for i in {1..$#auxArr}; do
        [[ ${auxArr[i]} == "$fieldVal" ]] && unset 'auxArr[i]'
    done
    auxArr=( "${auxArr[@]}" "$fieldVal" )
    printf -v "$varName" '%s' "${(@j.:.)auxArr:#}"
    export "$varName"
}

# SYNOPSIS: env_remove varName fieldVal [sep]
#   SEP defaults to ':'
# Note: Remove the fieldVal in varName.
#       Duplicates and empties are removed, too.
# EXAMPLE: env_remove PATH /usr/local/bin
env_remove() {
    [[ $# -ge 1 && $# -le 3 ]] || return 1
    local varName="$1" fieldVal="$2" IFS=${3:-':'}
    local auxArr=($(eval printf '%s' "\"\$${varName}\""))
    for i in {1..$#auxArr}; do
        [[ ${auxArr[i]} == "$fieldVal" ]] && unset 'auxArr[i]'
    done
    auxArr=( "${auxArr[@]}" )
    printf -v "$varName" '%s' "${(@j.:.)auxArr:#}"
    export "$varName"
}

