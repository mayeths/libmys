# macOS Quick Actions

Install the version-controlled workflows into `~/Library/Services`:

```bash
sh "$MYS_DIR/etc/macos/quick-actions/install.sh"
```

After installation, open Finder's **Quick Actions > Customize** and enable **Copy Path**. This is a one-time macOS confirmation.

`Copy Path` accepts files and folders selected in Finder. It copies one absolute path per line and preserves spaces and non-ASCII characters.

If the action does not appear immediately, relaunch Finder or log out and back in.
