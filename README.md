# Teardown Converter

**Teardown Converter** is an application for parsing and converting maps from the game Teardown.  
It allows to convert maps from the binary .tdbin format into editable .xml and .vox formats.

![UI](https://github.com/TTFH/Teardown-Converter/blob/main/converter_ui.png)

## How to download and use
> [!Note]
> You can download this tool from the [Releases page](https://github.com/TTFH/Teardown-Converter/releases) on GitHub.

> [!Tip]
> You may encounter red voxels in the editor, these are added to keep the original shape size when splitting compounds or removing snow from vehicle wheels.
> To hide these red voxels, press Shift + V in the editor to toggle their visibility.

> [!Important]
> This tool leaves uncompressed .tdbin files in the next locations, you can delete them to save disk space.
> `AppData\Local\Teardown\quicksave.tdbin`,`Teardown\data\bin\*.tdbin`,`Teardown\dlcs\*\*.tdbin` & `./*.tdbin`

> [!Caution]
> Those files may be outdated and needs to be manually removed to convert maps from the latest game version.

## How to compile
1. Open a **Terminal or Command Prompt** in the folder containing the `Makefile`.
2. Run the following command:
```bash
    make
```
3. After compilation, the executable will be located in the `/release` folder.

![Editor Example](https://github.com/TTFH/Teardown-Converter/blob/main/converter_map.png)  
