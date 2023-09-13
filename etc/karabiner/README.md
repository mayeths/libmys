##

Karabiner Version: 14.2.0

```bash
mkdir -p ~/.config/karabiner/automatic_backups
cp ~/.config/karabiner/karabiner.json "~/.config/karabiner/automatic_backups/karabiner_$(date '+%Y%m%d_%H%M%S').json"
cp $MYS_DIR/etc/karabiner/karabiner.json ~/.config/karabiner/karabiner.json
mkdir -p ~/.config/karabiner/assets/complex_modifications/
ln -s $MYS_DIR/etc/karabiner/assets/complex_modifications/external_keyboard_only.json ~/.config/karabiner/assets/complex_modifications/external_keyboard_only.json
ln -s $MYS_DIR/etc/karabiner/assets/complex_modifications/internal_keyboard_only.json ~/.config/karabiner/assets/complex_modifications/internal_keyboard_only.json
ln -s $MYS_DIR/etc/karabiner/assets/complex_modifications/test.json                   ~/.config/karabiner/assets/complex_modifications/test.json
ln -s $MYS_DIR/etc/karabiner/assets/complex_modifications/universal.json              ~/.config/karabiner/assets/complex_modifications/universal.json
```

### Note
- Every time you modify settings in Karabiner-Elements Preferences, The config file `karabiner.json` will be backup to `automatic_backups/` and replace with a normal file.
- Unclick all Logitech mouse devices in 'Devices' Tab of Preferences, that is, don't modify events from those devices, otherwise Logitech G HUB cannot detect them.
