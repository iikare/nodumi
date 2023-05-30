# nodumi - real-time MIDI visualizer

<img align="left" src="https://user-images.githubusercontent.com/13582030/193442487-40aadaf9-0b11-4309-907b-e481856bcb1d.svg" width="130px">


A usage guide is now available [on my website](https://iika.re/nodumi/guide.pdf "nodumi website").

`nodumi` — ನೋಡು (look/see) + [mi]di

<br>

# dependencies
This project requires several dependencies, many of which are submodules to this repository.

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

Keep in mind that Windows support is currently in an experimental stage, so expect weird quirks and bugs.

# build
Don't forget to `git clone --recurse-submodules` when obtaining the source code!

Currently, only Linux and to a lesser extent, Windows, are supported, however, the dependencies and codebase are
purposely OS-agnostic, and with some tweaking can run on MacOS as
well. Contributions to improve cross-platform interoperability are welcomed.

Simply run `make` in the project's root directory. The resulting executable
will be formed at `./bin/nodumi`

Cross-compilation to Windows is done through `make arch=win`, and the following executable is formed at `./bin/nodumi.exe`

To compile the documentation, run `make doc` (a $\LaTeX$ compiler and related software is required).

# usage
If executing `nodumi` from the command-line, up to two optional arguments (in any order) are accepted:
```sh
./bin/nodumi path/to/file.(mid|midi) path/to/image.(png|jpg)
```

Otherwise, use the built-in graphical commands.

# license
`nodumi` is licensed under GPLv3.
