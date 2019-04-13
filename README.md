# Umihara Kawase Loader
A simple dinput8.dll wrapper DLL that allows extra DLL loading and rebindable keys for the **Umihara Kawase PC game series**.

### Usage
Download the latest release zip and place the zip contents into the root directory of the game(s). **Read `umi_loader/config.ini` thoroughly for extra details.**

**VERY IMPORTANT**: Your in-game "keybind" settings must look like this: 
* Umihara Kawase:          https://i.imgur.com/o8V25ii.png
* Umihara Kawase Shun:     https://i.imgur.com/Q8c4eDm.png
* Sayonara Umihara Kawase: https://i.imgur.com/okawTSU.png

Optional: Place all the extra DLLs you want loaded into the `umi_loader/dlls` directory. Note: There isn't a specific order here when other DLLs are loaded since the filesystem iteration order is unspecified.

For example, here's what your folder structure should look like after you extracted the release zip to the first game (the extra DLLs are optional, don't worry about them if you don't need them):

```
C:\Program Files (x86)\Steam\steamapps\common\UmiharaKawase
|   ...
│   UmiFstWin.exe
│   dinput8.dll
|   ...
├───umi_loader
├──────dlls
|      ...
│      foo.dll
│      bar.dll
|      ...
|
|   config.ini
|   config_backup.ini
|   log.txt (created at runtime)
```

## Credits and thanks
frost  
n0x  
mpu  
dan  
keegan  