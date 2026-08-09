# Save Inventory Module

This is a module for [AzerothCore](http://www.azerothcore.org) that adds the option to instantly save items after they have been looted, preventing them from being lost in case of crashes. 

## Requirements

This module currently requires:

AzerothCore v1.0.2+

## How to install

### 1) Simply place the module under the `modules` folder of your AzerothCore source folder.

You can do clone it via git under the azerothcore/modules directory:

```sh
cd path/to/azerothcore/modules
git clone https://github.com/Nyeriah/mod-save-inventory.git
```


### 2) Re-run cmake and launch a clean build of AzerothCore

**That's it.**

### (Optional) Edit module configuration

If you need to change the module configuration, go to your server configuration folder (e.g. **etc**), copy `mod-save-inventory.conf.dist` to `mod-save-inventory.conf` and edit it as you prefer.

### Configuration Options

This module offers two options to select which items to automatically save once looted:

1) ModSaveItenventory.MinItemQuality

       Description: Minimum quality required to save items after looting.
       Default:     2 - Uncommon (Green)
       
       Possible Values:
                    0 - Poor (Gray)
                    1 - Normal (White)
                    2 - Uncommon (Green)
                    3 - Rare (Blue)
                    4 - Epic (Purple)
                    5 - Legendary (Orange)
                    6 - Artifact (Light Yellow)
                    7 - Heirloom

2) ModSaveItenventory.AlwaysSaveList

        Description: List of items separated by space that will always be saved regardless of the minimum quality set.
        Example:     "16328 16329"
        Default:     "" - None, empty list.

Separately, the module can report a loot window the first time it is opened, before anything is taken. This writes to its own log file, not to the looted items log:

3) ModSaveInventory.LogLootWindow

        Description: Print the contents of a loot window to the log file when it is first opened.
                     Only applies inside instances, and only to boss corpses and gameobjects.
        Default:     0 - Disabled
                     1 - Enabled

        Example output:
                     SaveInventory: Loot: 603, 4711, creature Flame Leviathan (entry 33113), Items: 45038, 45087
                     SaveInventory: Players eligible: Thrall (12), Jaina (34)

### 3) Add the `Appender` and `Logger` from below to your `worldserver.conf`

        Appender.Items=2,5,15,LootItem_%s.log
        Logger.items= 4,Items

If you enable `ModSaveInventory.LogLootWindow`, add this second pair as well. Without it the loot
window lines fall back to the root logger and end up in the server log.

        Appender.LootWindow=2,5,15,LootWindow_%s.log
        Logger.lootwindow= 4,LootWindow

## License

This module is released under the [GNU AGPL license](https://github.com/azerothcore/mod-transmog/blob/master/LICENSE)

## Authors

- [Nyeriah](https://github.com/Nyeriah)





