# nodumi - graphical MIDI visualizer
Currently, docs and info are in the process of being updated -- please wait warmly!.

At some point in the future as development progresses, a usage guide will be available [on my website](https://iika.re/nodumi/ "nodumi website").

A project logo will eventually take this statement's place.

`nodumi` — ನೋಡು (look/see) + [mi]di

# dependencies
This project requires several dependencies, many of which are submodules to this repository.

* `CIEDE2000` (supplied as a submodule)
* `midifile` (supplied as a submodule)
* `osdialog` (supplied as a submodule)
* `rtmidi` (supplied as a submodule)
* `raylib`* (see below)
* `make`
* `xxd`


Cross-compilation will require you to recompile `raylib` for the target platform. For this purpose, the included submodule must be used. Compiling for
a linux platform does not require the use of the bundled `raylib` submodule. For these cases, the build process will link against the system `raylib`
library.

The current build script assumes you are building on a Linux environment with raylib installed in the appropriate directories (`/lib/libraylib.so` and `/usr/include/raylib.h`). This project should be compiler-agnostic, but for best results, use `clang`/`clang++`. For cross-compilation to Windows, the following tools are required:

* `x86_64-w64-mingw32-g++`
* `x86_64-w64-mingw32-gcc`
* `x86_64-w64-mingw32-ld`

Note that CIEDE2000 is only required if `COLDIST_CIE00` is defined at compile-time, and you wish to use the CIEDE2000 
deltaE color-distance algorithm in generating a color palette from a background image.

# build
Don't forget to `git clone --recurse-submodules` when obtaining the source code!

Currently, only Linux and to a lesser extent, Windows, are supported, however, the dependencies and codebase are
purposely OS-agnostic, and with some tweaking can run on MacOS as
well. Contributions to improve cross-platform interoperability are welcomed.

Simply run `make` in the project's root directory. The resulting executable
will be formed at `./bin/nodumi`

Cross-compilation to Windows is done through `make arch=win`, and the following executable is formed at `./bin/nodumi.exe`

# usage
If executing `nodumi` from the command-line, up to two optional arguments (in order) are accepted:
```sh
./bin/nodumi path/to/file.(mid|midi) path/to/image.(png|jpg)
```

Otherwise, use the built-in graphical commands.

# license
`nodumi` is licensed under GPLv3.
