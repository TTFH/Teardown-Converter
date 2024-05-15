# Teardown Converter 2
An application for parsing and converting maps of the game Teardown from binary format (.tdbin) to editable format (.xml, .vox)  

![UI](https://github.com/TTFH/Teardown-Converter/blob/main/converter_ui.png)  

## How to Download
**You can download a precompiled version here:** https://github.com/TTFH/Teardown-Converter/releases  

### There are red voxels on the editor
Those voxels are used so the shapes don't change the size when removing the snow or spliting compounds.  
You can use Shift + 'V' in the editor to hide them.  

## How to Compile
This program needs SDL2 and Zlib, check the Makefile to know how to install them  
Open a Terminal / Command Prompt in the same folder that Makefile  
Type: `make`  
You can find the execulable in the `release` folder  

![Editor Example](https://github.com/TTFH/Teardown-Converter/blob/415d7c086d4669cbbaa9762657e97eb9a1d57d97/utils/converter_mall.png)  
