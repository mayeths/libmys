#!/bin/bash

set -euo pipefail

script_dir=$(/usr/bin/dirname "${BASH_SOURCE[0]}")
if ! cd "${script_dir}"; then
    printf 'Cannot enter script directory: %s\n' "${script_dir}" >&2
    exit 1
fi

script_dir=$(pwd -P)
project_dir=$(/usr/bin/dirname "${script_dir}")
built_app="${project_dir}/build/DerivedData/Build/Products/Release/FinderTools.app"
applications_dir="${HOME}/Applications"
installed_app="${applications_dir}/FinderTools.app"
installed_extension="${installed_app}/Contents/PlugIns/FinderToolsExtension.appex"
extension_identifier="com.mayeths.FinderTools.FinderSync"

/bin/bash "${script_dir}/build.sh"
/bin/mkdir -p "${applications_dir}"

if [ -d "${installed_app}" ]; then
    /bin/rm -rf "${installed_app}"
fi

/usr/bin/ditto "${built_app}" "${installed_app}"

if ! /usr/bin/pluginkit -a "${installed_extension}"; then
    printf '%s\n' "Warning: Finder extension registration failed" >&2
fi

if ! /usr/bin/pluginkit -e use -i "${extension_identifier}"; then
    printf '%s\n' "Warning: Finder extension could not be enabled automatically" >&2
fi

if ! /usr/bin/killall Finder; then
    printf '%s\n' "Warning: Finder was not running, so it was not relaunched" >&2
fi

/usr/bin/open "${installed_app}"

printf 'Installed: %s\n' "${installed_app}"
printf '%s\n' "If the menus are missing, enable FinderToolsExtension in:"
printf '%s\n' "System Settings > General > Login Items & Extensions > Finder Extensions"
