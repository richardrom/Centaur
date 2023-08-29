import argparse
import os
import subprocess
import sys
import platform
import shutil

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        prog='ctheme-copy',
        description='For testing session: Copy the data to the local installation path',
        epilog='CtTheme generator')

    parser.add_argument('-c', '--ct', type=str)
    parser.add_argument('-l', '--local', type=str)
    parser.add_argument('-b', '--lib', type=str)

    args = parser.parse_args()

    if len(args.ct) == 0:
        print('CTTHEME-COPY.py: ctpath path is empty')
        sys.exit(1)

    if len(args.local) == 0:
        print('CTTHEME-COPY.py: ctpath path is empty')
        sys.exit(1)

    if len(args.lib) == 0:
        print('CTTHEME-COPY.py: library path is empty')
        sys.exit(1)

    libraryName = ""
    ctpathExecutable = ""
    currentSystem = platform.system()
    if currentSystem == "Linux":
        libraryName = "libCentTheme.so"
        ctpathExecutable = args.ct + "/ctpath"
    elif currentSystem == "Windows":
        libraryName = "CentTheme.dll"
        ctpathExecutable = args.ct + "\\ctpath.exe"
    elif currentSystem == "Darwin":
        libraryName = "libCentTheme.dylib"
        ctpathExecutable = args.ct + "/ctpath"
    else:
        libraryName = "libCentTheme"
        ctpathExecutable = args.ct + "/ctpath"

    process = subprocess.Popen(ctpathExecutable, shell=True, text=True,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)

    # wait for the process to terminate
    out, err = process.communicate()
    errcode = process.returncode

    if errcode != 0:
        print('CTTHEME-COPY.py: ctpath return and error code')
        exit(1)

    if out == "":
        print('CTTHEME-COPY.py: ctpath returned an empty path')
        exit(1)

    themePath = out + "/theme"
    extraPath = out + "/Plugins/extra/2df95e88-7eae-5940-ab0d-b53f2df855e2"

    os.makedirs(extraPath, exist_ok=True)
    os.makedirs(themePath, exist_ok=True)

    themeLocalFile = args.local + "/dark.theme.xml"
    themeExtraFile = extraPath + "/dark.theme.xml"

    if not os.path.isfile(themeLocalFile):
        print("CTTHEME-COPY.py: Local theme file does not exist")
        exit(1)

    try:
        shutil.copy(themeLocalFile, themeExtraFile)
    except EnvironmentError:
        print("CTTHEME-COPY.py: Unable to copy the theme file")
    else:
        print("CTTHEME-COPY.py: Theme file copied")

    themeLibrarySource = args.lib + "/" + libraryName
    themeLibraryDest = themePath + "/" + libraryName

    if not os.path.isfile(themeLibrarySource):
        print("CTTHEME-COPY.py: Local theme file does not exist")
        exit(1)

    try:
        shutil.copy(themeLibrarySource, themeLibraryDest)
    except EnvironmentError:
        print("CTTHEME-COPY.py: Unable to copy the library file")
    else:
        print("CTTHEME-COPY.py: Library theme file copied")
