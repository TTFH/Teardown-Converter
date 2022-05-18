# Teardown-Converter
An application for parsing and converting maps of the game Teardown from binary format (.tdbin) to editable format (.xml, .vox)  

![UI](https://github.com/TTFH/Teardown-Converter/blob/8878910b36c8e2e56314c35f2178230a7b4aea10/utils/converter_ui.png)  

## How to Download
**You can download a precompiled version from:** https://github.com/TTFH/Teardown-Converter/releases  

## Free Disk Space
After using this converter you can remove the .tdbin files that are created in: `Teardown\data\bin`  
Those files are leave there so it's faster to convert the same map again.  

### There are red voxels on the editor
Those voxels are used so the shapes don't change the size when removing the snow or spliting compounds.  
You can use Shift + 'V' in the editor to hide them.  
When you remove the snow, all vehicle wheels will have some extra voxels on them, those need to be removed manually (will be fixed in the next update).  

## How to Compile
This program needs SDL2 and Zlib, check the Makefile to know how to install them  
Open a Terminal / Command Prompt in the same folder that Makefile  
Type: `make`  
You can find the execulable in the `release` folder  

## Know Bugs
- Some shapes are rotated 90° on the y axis from the correct position.  
- There are some lines on the terrain where tiles are split a few voxels apart.  

## Task list
#### Add the next attributes to the xml file:
- [ ] Shape collide
- [ ] Vehicle driven

#### Fixes
- [ ] Clean up XML (default attributes and order)
- [ ] Improve progress bar

![Editor Example](https://github.com/TTFH/Teardown-Converter/blob/415d7c086d4669cbbaa9762657e97eb9a1d57d97/utils/converter_mall.png)  
