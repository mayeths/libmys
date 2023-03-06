
if ! command -v realpath >/dev/null 2>&1; then
    realpath() {
        echo "$(cd "$(dirname -- "$1")" >/dev/null; pwd -P)/$(basename -- "$1")"
    }
fi
