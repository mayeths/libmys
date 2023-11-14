# Refer to https://github.com/ohmyzsh/ohmyzsh/blob/master/plugins/pyenv/pyenv.plugin.zsh
# This plugin provides prompt info via the 'mpi_env_prompt_info' function.

# Look for compiler and mpi in $PATH

function mpi_env_prompt_info() {
    function get_compiler_type() {
        if [[ $(echo "$1" | head -1 | cut -c1-3) == "gcc" ]]; then
            echo "gcc"
        elif [[ $(echo "$1" | head -1 | cut -c1-3) == "icc" ]]; then
            echo "icc"
        elif [[ "$1" == *"clang version"* ]]; then
            echo "clang"
        fi
    }

    function get_version_XYZ() {
        if $(grep --version | grep -q "GNU grep"); then # GNU grep
            echo $(echo "$1" | grep -oP '\d*\.\d*\.\d*' | head -1)
        else # BSD grep
            echo $(echo "$1" | grep -oe '\d*\.\d*\.\d*' | head -1)
        fi
    }

    function get_icc_version() {
        if $(grep --version | grep -q "GNU grep"); then # GNU grep
            echo 20$(echo "$1" | grep -oP '\d*\.\d*\.\d*\.\d*' | head -1 | cut -d'.' -f1)
        else # BSD grep
            echo 20$(echo "$1" | grep -oe '\d*\.\d*\.\d*\.\d*' | head -1 | cut -d'.' -f1)
        fi
    }

    function get_intelmpi_version() {
        if $(grep --version | grep -q "GNU grep"); then # GNU grep
            echo $(echo "$1" | grep -oP 'Version \d*' | awk '{print $2}')
        else # BSD grep
            echo $(echo "$1" | grep -oe 'Version \d*' | awk '{print $2}')
        fi
    }

    local array=()

    mpirun_version_out=$(mpirun --version 2>/dev/null)
    if [[ "$mpirun_version_out" == *"Intel Corporation"* ]]; then
        # If use Intel MPI, imply Intel compiler
        local intelmpi_ver=$(get_intelmpi_version "$mpirun_version_out")
        array+=("intel-$intelmpi_ver")
    elif [[ "$mpirun_version_out" == *"Open MPI"* ]]; then
        # If use Open MPI, show actual compiler.
        local ompi_ver=$(get_version_XYZ "$mpirun_version_out")
        array+=("ompi-$ompi_ver")
    else
        # no mpi is running, print nothing
    fi

    mpicc_version_out=$(mpicc --version 2>/dev/null)
    local compiler_type=$(get_compiler_type "$mpicc_version_out")
    if [[ "$compiler_type" == "gcc" ]]; then
        local gcc_ver=$(get_version_XYZ "$mpicc_version_out")
        array+=("gcc-$gcc_ver")
    elif [[ "$compiler_type" == "icc" ]]; then
        local icc_ver=$(get_icc_version "$mpicc_version_out")
        array+=("icc-$icc_ver")
    elif [[ "$compiler_type" == "clang" ]]; then
        local clang_ver=$(get_version_XYZ "$mpicc_version_out")
        array+=("clang-$clang_ver")
    fi

    echo "${(j: :)array}"
}

# type 'p10k segment help' to see all example or search prompt_example in config/p10k.sh
function prompt_mpi_env() {
    local text=$(mpi_env_prompt_info)
    p10k segment -f 178 -t "$text"
}

# mpi_env_prompt_info
