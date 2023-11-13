# Refer to https://github.com/ohmyzsh/ohmyzsh/blob/master/plugins/pyenv/pyenv.plugin.zsh
# This plugin provides prompt info via the 'mpi_env_prompt_info' function.

# Look for compiler and mpi in $PATH

function get_compiler_type() {
    if [[ $($1 --version | head -1 | cut -c1-3) == "gcc" ]]; then
        echo "gcc"
    elif [[ $($1 --version | head -1 | cut -c1-3) == "icc" ]]; then
        echo "icc"
    elif [[ $($1 --version) == *"clang version"* ]]; then
        echo "clang"
    fi
}

function get_version_XYZ() {
    if $(grep --version | grep -q "GNU grep"); then # GNU grep
        echo $($1 --version 2>/dev/null | grep -oP '\d*\.\d*\.\d*' | head -1)
    else # BSD grep
        echo $($1 --version 2>/dev/null | grep -oe '\d*\.\d*\.\d*' | head -1)
    fi
}

function get_icc_version() {
    if $(grep --version | grep -q "GNU grep"); then # GNU grep
        echo 20$($1 --version 2>/dev/null | grep -oP '\d*\.\d*\.\d*\.\d*' | head -1 | cut -d'.' -f1)
    else # BSD grep
        echo 20$($1 --version 2>/dev/null | grep -oe '\d*\.\d*\.\d*\.\d*' | head -1 | cut -d'.' -f1)
    fi
}

function get_intelmpi_version() {
    if $(grep --version | grep -q "GNU grep"); then # GNU grep
        echo $(mpirun --version 2>/dev/null | grep -oP 'Version \d*' | awk '{print $2}')
    else # BSD grep
        echo $(mpirun --version 2>/dev/null | grep -oe 'Version \d*' | awk '{print $2}')
    fi
}

function mpi_env_prompt_info() {
    local found_gcc found_icc found_clang found_mpi
    command -v gcc &>/dev/null && found_gcc=1 || found_gcc=0
    command -v icc &>/dev/null && found_icc=1 || found_icc=0
    command -v clang &>/dev/null && found_clang=1 || found_clang=0
    command -v mpirun &>/dev/null && found_mpi=1 || found_mpi=0

    local array=()

    if [[ $(mpirun --version 2>/dev/null) == *"Intel Corporation"* ]]; then
        # If use Intel MPI, imply Intel compiler
        local intelmpi_ver=$(get_intelmpi_version)
        array+=("intel-$intelmpi_ver")
    elif [[ $(mpirun --version 2>/dev/null) == *"Open MPI"* ]]; then
        # If use Open MPI, show actual compiler.
        local ompi_ver=$(get_version_XYZ mpirun)
        array+=("ompi-$ompi_ver")
    else
        # no mpi is running, print nothing
    fi

    local compiler_type=$(get_compiler_type mpicc)
    if [[ "$compiler_type" == "gcc" ]]; then
        local gcc_ver=$(get_version_XYZ mpicc)
        array+=("gcc-$gcc_ver")
    elif [[ "$compiler_type" == "icc" ]]; then
        local icc_ver=$(get_icc_version mpicc)
        array+=("icc-$icc_ver")
    elif [[ "$compiler_type" == "clang" ]]; then
        local clang_ver=$(get_version_XYZ mpicc)
        array+=("clang-$clang_ver")
    fi

    echo "${(j: :)array}"
}

# type 'p10k segment help' to see all example or search prompt_example in config/p10k.sh
function prompt_mpi_env() {
    local text=$(mpi_env_prompt_info)
    p10k segment -f 4 -t "$text"
}

# mpi_env_prompt_info
