# Teardown-Converter
An application for parsing and converting maps of the game Teardown from binary format (.tdbin) to editable format (.xml, .vox)  

![UI](https://github.com/TTFH/Teardown-Converter/blob/main/converter_ui.png)  

## How to Download
**You can download a precompiled version here:** https://github.com/TTFH/Teardown-Converter/releases  

## Free Disk Space
After using this converter you can remove the .tdbin files that are created in: `Teardown\data\bin`  
Those files are leave there so it's faster to convert the same map again.  
When an update is released for Teardown, you NEED to remove those files or this tool will convert the old version of the map.

### There are red voxels on the editor
Those voxels are used so the shapes don't change the size when removing the snow or spliting compounds.  
You can use Shift + 'V' in the editor to hide them.  

## How to Compile
This program needs SDL2 and Zlib, check the Makefile to know how to install them  
Open a Terminal / Command Prompt in the same folder that Makefile  
Type: `make`  
You can find the execulable in the `release` folder  

![Editor Example](https://github.com/TTFH/Teardown-Converter/blob/415d7c086d4669cbbaa9762657e97eb9a1d57d97/utils/converter_mall.png)  
