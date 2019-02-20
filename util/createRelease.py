from shutil import copyfile, copy
import os

releaseDir = "D:/Dev/CPP/UltimateWallpaper-2_0/build/release/"
buildDir = r"D:/Dev/CPP/UltimateWallpaper-2_0/build/bin/UltimateWallpaper/"


def copyRecurs(src, dst):
    for entry in os.scandir(src):
        if entry.is_file():
            if "pdb" not in entry.name and "ilk" not in entry.name:
                copyfile(src + entry.name, dst + entry.name)
        else:
            os.mkdir(dst + entry.name)
            copyRecurs(src + entry.name + "/", dst + entry.name + "/")


if __name__ == '__main__':
    # copy dist version

    if not os.path.exists(releaseDir):
        os.makedirs(releaseDir)
    copyRecurs(buildDir + "Dist-windows-x86_64/", releaseDir)

    # copy debug version
    copyfile(buildDir + "Debug-windows-x86_64/UltimateWallpaper-dbg.exe", releaseDir + "UltimateWallpaper-dbg.exe")

    # copy release version as editor
    copyfile(buildDir + "Release-windows-x86_64/UltimateWallpaper-rls.exe", releaseDir + "Editor.exe")

    input("Press enter to exit...")
