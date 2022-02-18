# Ultimate Wallpaper
Wallpaper for Wallpaper Engine

If you came here from the Steam Workshop see the Workshop section at the end regarding file locations.

# Requirements
- Windows with x86_64 architecture
- OpenGL Version 330 or higher
- Wallpaper Engine (https://store.steampowered.com/app/431960/Wallpaper_Engine/)

# Versions
There are 3 executables included in the dowload:

### UltimateWallpaper-dbg.exe
> Debug version. Not to be (even so one could) run in Wallpaper Engine but as stand-alone for debuggin purposes. Has a smaller window and logs debug information to the console. Contains editor in debug mode aswell

### UltimateWallpaper.exe
> Main version to be used unless you need logs. No console, best performance.

### Editor.exe
> Configuration editor

# Installing
## Steam Workshop
Just click subscribe here https://steamcommunity.com/sharedfiles/filedetails/?id=2756726513

## Manual install
1) Download
2) Unzip (note: its recommended to 'unzip to a folder', as there is more than one file contained in the zip and all are required for the wallpaper to work properly (or rather at all)
3) Drag and drop the `UltimateWallpaper.exe` file into the wallpaper engine window
4) Ignore the red, wild and angry warning (*) for 3 seconds and press proceed
5) Enjoy.

(*) It basically tells you that by importing and running an exe file you literally give it permission to do anything. If you don't like that, don't use the wallpaper. The choice is yours.

# Using multiple montors
If one configuration is sufficient and you don't need a different one for another monitor you can just run the one wallpaper on multiple monitors.

However, because this wallpaper has no settings within wallpaper engine using it on multiple monitors (with different settings per monitor) is a little tricky and you'll have to set that up manually.

Instead of just 'running' it twice on different monitors, you'll have to duplicate the wallpaper for each different configuration you need, and the run a different instance on each monitor. To do so do the following:
1) Open the folder containing the wallpaper
2) Duplicate the whole folder
3) Open the new folder. Here you'll want to:
  1) Rename the UltimateWallpaper.exe to something differen (e.g. UltimateWallpaper-Monitor1.exe)
  2) Run the Editor.exe and change the settings for this instance of the wallpaper, this can also be done at any later point
6) Drag and drop the renamed exe into wallpaper engine like you did when you installed it in the first place. It will then show up with the name you gave it.
7) Assign the right .exe to the right monitor and you are done.

**This also applies to installs from the workshop**

# Editor
Used to create / manage different configurations.

Remember to save and (unless you edit the 'cfg.xml' directly) apply your configuration before you close the editor.
If you ever mess things up you can load the 'backup-cfg' which contains my config (The one loaded as you first opened the wallpaper).
Or you use the 'default-cfg'.
  
The display area for the wallpaper is scaled to 90% in the editor

There are 3 'cfg.xml' files in the distributed folder:

### cfg.xml
> The cfg.xml is the active wallpaper configuration. You can edit it directly if you want. If you apply any configuration in the editor this file will be overwritten. There for it might be better to save your own configuration and 'apply' it instead of editing the active config.

### backup-cfg.xml
> The backup-cfg.xml should not be edited. Its just a backup of (my) default configuration in case you want to reset the wallpaper to its delivery state.

### default-cfg.xml
> The default-cfg.xml can be edited. Its the file that is loaded if you press 'Restore default' in the editor.

## Some side notes
- All number fields can be dragged. If you wish to edit the value by hand you can double click it.
- Pressing alt while draggin a value decreases the step size
- Text fields are only applied if you press enter.

# Steam Workshop
If you installed the Wallpaper via the Steam workshop everything above applies.
The main difference here is that steam will automatically 'install' the Wallpaper and run the right file if you activate it.
This however also means that 'folder contatining the wallpaper' so the files like `Editor.exe` are rather hidden.
To access them right click the wallpaper icon in Wallpaper Engine an select 'Open in Explorer'. This will open the folder containing the wallpaper.

![How to open the 'folder containing the wallpaper' when installed from workshop](https://user-images.githubusercontent.com/18683538/154604797-7cd614a7-5627-4360-b5b5-cf64fd6291a6.png)


# Links
- tinyxml2 (Xml parsing): https://github.com/leethomason/tinyxml2
- stb (Image loading): https://github.com/nothings/stb
- spdlog (Logging): https://github.com/gabime/spdlog
- bass (Audio stream capturing): http://www.un4seen.com/
- imgui (Editor Ui): https://github.com/ocornut/imgui
