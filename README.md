# Ultimate Wallpaper
Wallpaper for Wallpaper Engine

# Requirements
- Windows with x86_64 architecture
- OpenGL Version 330 or higher
- Wallpaper Engine (https://store.steampowered.com/app/431960/Wallpaper_Engine/)

# Versions
<p> There are 3 executables included in the dowload </p>

- UltimateWallpaper-dbg.exe
<p>Debug version. Not to be (even so one could) run in Wallpaper Engine but as stand-alone for debuggin purposes. Has a smaller window and logs debug information to the console. Contains editor in debug mode aswell</p>

- UltimateWallpaper.exe
<p>Main version to be used unless you need logs. No console, Most performance.</p>

- Editor.exe
<p>Configuration editor</p>

# Editor
<p>Create / Manage different configurations.</p>
<p>Remember to save and (unless you edit the 'cfg.xml' directly) apply your configuration before you close the editor.
If you ever mess things up you can load the 'backup-cfg' which contains my config (The one loaded as you first opened the wallpaper).
Or you use the 'default-cfg'. </p>
<p>The display area for the wallpaper is scaled to 90% in editor</p>
<h2>There are 3 'cfg.xml' files in the distributed folder:</h2>

- cfg.xml
<p>The cfg.xml is the active wallpaper configuration. You can edit it directly if you want. If you apply any configuration in the editor
  this file will be overwritten. There for it might be better to save your own configuration and 'apply' it instead of editing the active config.</p>

- backup-cfg.xml
<p>The backup-cfg.xml should not be edited. Its just a backup of (my) default configuration in case you want to reset the wallpaper to its delivery state.</p>

- default-cfg.xml
<p>The default-cfg.xml can be edited. Its the file that is loaded if you press 'Restore default' in the editor.</p>

<h2>Some side notes</h2>
<p>All number fields can be dragged. If you wish to edit the value by hand you can double click it.</p>
<p>Pressing alt while draggin a value decreases the step size</p>
<p>Text fields are only applied if you press enter.</p>



# Links
- tinyxml2 (Xml parsing): https://github.com/leethomason/tinyxml2
- stb (Image loading): https://github.com/nothings/stb
- spdlog (Logging): https://github.com/gabime/spdlog
- bass (Audio stream capturing): http://www.un4seen.com/
- imgui (Editor Ui): https://github.com/ocornut/imgui
