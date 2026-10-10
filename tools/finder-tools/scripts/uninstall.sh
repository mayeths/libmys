#!/bin/bash

set -euo pipefail

installed_app="${HOME}/Applications/FinderTools.app"
installed_extension="${installed_app}/Contents/PlugIns/FinderToolsExtension.appex"
extension_identifier="com.mayeths.FinderTools.FinderSync"

if ! /usr/bin/pluginkit -e ignore -i "${extension_identifier}"; then
    printf '%s\n' "Warning: Finder extension was not enabled" >&2
fi

if [ -d "${installed_extension}" ]; then
    if ! /usr/bin/pluginkit -r "${installed_extension}"; then
        printf '%s\n' "Warning: Finder extension deregistration failed" >&2
    fi
fi

if [ -d "${installed_app}" ]; then
    /bin/rm -rf "${installed_app}"
fi

if ! /usr/bin/killall Finder; then
    printf '%s\n' "Warning: Finder was not running, so it was not relaunched" >&2
fi

printf 'Removed: %s\n' "${installed_app}"
