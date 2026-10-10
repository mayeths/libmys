# FinderTools

FinderTools is a small macOS application containing a Finder Sync extension. It adds these Finder context-menu commands:

- **Copy Path** for one selected item
- **Copy Paths** for multiple selected items, with one absolute path per line
- **Copy Path** for the background of the current folder

The extension monitors `/`, so the commands are available in local folders, mounted volumes, and Finder sidebars where macOS permits Finder Sync extensions.

## Requirements

- macOS 13 or later
- The full Xcode application and its command-line tools
- [XcodeGen](https://github.com/yonaskolb/XcodeGen) 2.34 or later

Xcode is used as a command-line toolchain; its graphical interface does not need to be open.

```bash
# Install XcodeGen
brew install xcodegen
```

## Build and install

```bash
cd "$MYS_DIR/tool/finder-tools"

# Generate the Xcode project and build an ad-hoc signed local application
bash scripts/build.sh

# Build, install to ~/Applications, register the extension, and relaunch Finder
bash scripts/install.sh
```

The installer attempts to enable the extension with `pluginkit`. If the menus do not appear, enable **FinderToolsExtension** once in:

```text
System Settings > General > Login Items & Extensions > Finder Extensions
```

Then relaunch Finder:

```bash
killall Finder
```

The containing application only displays setup instructions and exits. It does not remain running.

## Uninstall

```bash
bash scripts/uninstall.sh
```

## Signing

The default build uses ad-hoc signing, which requires no Apple Developer account and is intended only for the current Mac. Rebuilding may occasionally require enabling the extension again.

Distribution to other Macs requires a stable Developer ID signature and notarization.

## Generated files

`FinderTools.xcodeproj` and `build/` are generated locally and ignored by Git. `project.yml` is the source of truth for the Xcode project.
