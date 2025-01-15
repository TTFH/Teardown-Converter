# Teardown Converter

**Teardown Converter** is an application for parsing and converting maps from the game Teardown.  
It allows you to transform maps from the binary .tdbin format into editable .xml and .vox formats.

![UI](https://github.com/TTFH/Teardown-Converter/blob/main/converter_ui.png)

## How to download and use
> [!Note]
> You can download a precompiled version of the tool from the [Releases page](https://github.com/TTFH/Teardown-Converter/releases) on GitHub.

> [!Important]
> You may encounter red voxels in the editor, these are intentionally added to ensure that shapes don't change size when removing snow or splitting compounds.

> [!Tip]
> If you wish to hide these red voxels, press Shift + V in the editor to toggle their visibility.

> [!Caution]
> Make sure to delete the uncompressed .tdbin files generated by the tool after each game update.

## How to compile
1. Open a **Terminal or Command Prompt** in the folder containing the Makefile.
2. Run the following command:
```bash
    make
```
3. After compilation, the executable will be located in the `/release` folder.

![Editor Example](https://github.com/TTFH/Teardown-Converter/blob/415d7c086d4669cbbaa9762657e97eb9a1d57d97/utils/converter_mall.png)  
