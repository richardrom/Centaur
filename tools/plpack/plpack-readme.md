### Plugin Packager

Generates a compressed zip file ***(a package)*** that can be used to distribute plugins.<br>
This **packages** can be directly installed from the Centaur UI<br>
The way plpack works is to pack in a single mainly two files and generate an output file with the CPK extension:

#### A JSON File containing the next metadata:

```json
{
  "version": "0.1.0",
  "identification": {
    "name": "",
    "theme": true/false,
    "extras": true/false
    "version": "",
    "uuid": "",
    "manufacturer": "",
    "checksum": "",
    "protected": true/false,
    "dynamic": "",
    "ui-version": {
      "min-uuid": ""
    }
  }
}
```

#### Where the rooted version is the package version.

* **version**: The version of the package, which as for the writing of this documentation is **0.1.0**.

1. **name**: Name of the plugin
2. **theme**: The plugin implements a theme
3. **extras**: The package file contains extras
4. **version**: Version of the plugin
5. **uuid**: Identification string of the plugin without the curly braces
6. **manufacturer**: Developer of the plugin
7. **checksum**: Main SHA224 checksum of the **lib*****name***(.dylib)/(.dll) of the file[^1]
8. **protected**: Indicate that the plugin might store sensitive data that shall be encrypted
9. **dynamic**: Name of the dynamic library file
10. **ui-version:min-uuid**: Minimum version of the CentaurUI that can be run

[^1]: For Debugging and testing purposes if compile with _NO_PLUGIN_CHECKSUM_CHECK. The UI will ignore this parameter.
This setup will be informed in the Settings page of the plugins.

### plpack command line

```
-n,--name: Name of the plugin
-v,--version: Version of the plugin
-u,--uuid: Plugin identification string
-m,--man: Developer
-i,--min: Minimum UI version, if not specified, plpack will use the must recent version internally
-l,--lib: The path where the actual plugin file is located. 
          Beware that plpack will not check if its a valid plugin
-p: Protected
-o,--out: Output path for the resulting file. The file name will have the format {#1}-{#2}.cpk
          Where #1 is '--name' with the spaces replaced by '-' and #2 are the first 8 characters of the uuid
-t,--theme: The package is a plugin that implements a plugin
-e,--extra: Specify a file that will added to the extra directory. For more than more file set this option as needed
            example: plpack -e file1.txt -e file2.txt -e file3.txt 
                     OR
                     plpack --extra=file1.txt --extra=file2.txt --extra=file3.txt 
```

### Builtin Plugins

On the **local** path will be located the internal CPK files for all the builtin plugins

### Modifying builtin plugins or creating new ones

As mentioned, for testing purposes compile with _NO_PLUGIN_CHECKSUM_CHECK to avoid checksum checking
at startup, to avoid recalculating/recopying the SHA224 or modifying the internal plugin structure manually.

