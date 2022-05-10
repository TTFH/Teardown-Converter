# Teardown-Converter
An application for parsing and converting maps of the game Teardown from binary format (.tdbin) to editable format (.xml, .vox)

![UI](https://github.com/TTFH/Teardown-Converter/blob/8878910b36c8e2e56314c35f2178230a7b4aea10/utils/converter_ui.png)  

## How to Download
**There are no releases yet**, you will be able to download a precompiled version from: https://github.com/TTFH/Teardown-Converter/releases

## How to Compile
This program needs SDL2 and Zlib, check the Makefile to know how to install them  
Open a Terminal / Command Prompt in the same folder that Makefile  
Type: `make`  
You can find the execulable in the `release` folder  

## Task list
#### Add the next attributes to the xml file:
- [ ] Shape collide
- [ ] Rope slack
- [ ] Vehicle driven
- [ ] Screen fxglitch

#### Fixes
- [ ] Fix joints positions
- [ ] Add all 112 maps
- [ ] Clean up XML (default attributes and order)
- [ ] Fix progress bar
- [ ] Other minor fixes

![Editor Example](https://github.com/TTFH/Teardown-Converter/blob/8878910b36c8e2e56314c35f2178230a7b4aea10/utils/converter_lee.png)  
