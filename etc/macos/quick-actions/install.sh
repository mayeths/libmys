#!/bin/sh

set -eu

script_dir=$(/usr/bin/dirname "$0")
if ! cd "${script_dir}"; then
    printf 'Cannot enter script directory: %s\n' "${script_dir}" >&2
    exit 1
fi
script_dir=$(pwd -P)
source_workflow="${script_dir}/Copy Path.workflow"
services_dir="${HOME}/Library/Services"
installed_workflow="${services_dir}/Copy Path.workflow"

if [ ! -d "${source_workflow}" ]; then
    printf 'Missing workflow: %s\n' "${source_workflow}" >&2
    exit 1
fi

/bin/mkdir -p "${services_dir}"
/usr/bin/ditto "${source_workflow}" "${installed_workflow}"
/usr/bin/touch "${installed_workflow}"

services_cache_tool="/System/Library/CoreServices/pbs"
if [ -x "${services_cache_tool}" ]; then
    if ! "${services_cache_tool}" -flush; then
        printf '%s\n' "Warning: failed to refresh the Services cache" >&2
    fi
fi

printf 'Installed: %s\n' "${installed_workflow}"
printf '%s\n' "In Finder, choose Quick Actions > Customize and enable Copy Path once."
printf '%s\n' "Then select files and choose Quick Actions > Copy Path."
