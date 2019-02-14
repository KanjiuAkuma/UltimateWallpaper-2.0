# UltimateWallpaper
Wallpaper for Wallpaper Engine

# Requirements
- Windows with x86_64 architecture
- OpenGL Version 330 or higher
- Wallpaper Engine (https://store.steampowered.com/app/431960/Wallpaper_Engine/)

# Versions
- DEBUG:
<p>Not to be run in Wallpaper Engine but as stand-alone for debuggin purposes. Has a smaller window and logs debug information to the console. Also contains editor</p>

- DIST:
<p>Main version to be used unless you need logs. No console, Most performance.</p>

# Editor
Create / Manage different configurations.
Remember to save and (unless you edit the 'cfg.xml' directly) apply your configuration before you close the editor.
If you ever mess things up you can load the 'backup-cfg' which contains my config (The one loaded as you first opened the wallpaper).
Or you use the 'default-cfg'.

# Links
- tinyxml2 (xml parsing): https://github.com/leethomason/tinyxml2
- stb (image loading): https://github.com/nothings/stb
- spdlog (logging): https://github.com/gabime/spdlog
- bass (audio visualisation): http://www.un4seen.com/
- imgui (Editor Ui): https://github.com/ocornut/imgui
