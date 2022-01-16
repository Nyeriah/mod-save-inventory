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


## License

This module is released under the [GNU AGPL license](https://github.com/azerothcore/mod-transmog/blob/master/LICENSE)

## Authors

- [Nyeriah](https://github.com/Nyeriah)





