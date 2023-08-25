import argparse
import subprocess
import sys
import platform

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        prog='theme-plypack',
        description='Generates the plpack from the build system for the CtTheme',
        epilog='CtTheme plpack generator')

    parser.add_argument('-o', '--output', type=str)
    parser.add_argument('-l', '--local', type=str)
    parser.add_argument('-b', '--bin', type=str)
    parser.add_argument('-i', '--lib', type=str)

    args = parser.parse_args()

    if len(args.output) == 0:
        print('Output path is empty')
        sys.exit(1)

    if len(args.local) == 0:
        print('Local path is empty')
        sys.exit(1)

    if len(args.bin) == 0:
        print('Binary path is empty')
        sys.exit(1)

    if len(args.lib) == 0:
        print('Library path is empty')
        sys.exit(1)

    libraryName = ""
    currentSystem = platform.system()
    if currentSystem == "Linux":
        libraryName = "libCentTheme.so"
    elif currentSystem == "Windows":
        libraryName = "CentTheme.dll"
    elif currentSystem == "Darwin":
        libraryName = "libCentTheme.dylib"
    else:
        libraryName = "libCentTheme"

    exit(subprocess.run("{0}/plpack --name=CtTheme"
                        " --version=0.1.0 --man=\"Centaur Project\" --min=0.1.0"
                        " --lib=\"{1}/{4}\" --out=\"{2}\""
                        " --uuid=\"2df95e88-7eae-5940-ab0d-b53f2df855e2\""
                        " --theme --extra=\"{3}/dark.theme.json\"".format(args.bin, args.lib, args.output,
                                                                          args.local, libraryName),
                        shell=True).returncode)
