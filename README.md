# nodumi - real-time MIDI visualizer

<img align="left" src="https://user-images.githubusercontent.com/13582030/193442487-40aadaf9-0b11-4309-907b-e481856bcb1d.svg" width="130px">


For more information, see the [project page](https://iika.re/nodumi/ "nodumi website").

A usage guide is now [available](https://iika.re/nodumi/guide.pdf "nodumi usage guide").


`nodumi` — ನೋಡು (look/see) + [mi]di

<br>

# dependencies
This project requires several dependencies, many of which are submodules to this repository. Some of these modules are optional and/or platform specific.

* `midifile`  (supplied as a submodule)
* `osdialog`  (supplied as a submodule)
* `rtmidi`    (supplied as a submodule)
* `raylib` 5.0 or newer (supplied as a submodule, see below)
* `libtorch`  (optional, see below)
* `clang`/`clang++` with C++23 support
* `mold` (linker)
* `make`, `zsh`, `git`, `xxd` and `pkg-config` (used by the build scripts)
* GTK 3 development files (file dialogs on Linux)
* ALSA and JACK development files (MIDI I/O on Linux)

Compiling for a Linux platform does not require the bundled `raylib` submodule. In that case, the build process links against the system `raylib`
library, and assumes it is installed in the usual directories (`/lib/libraylib.so` and `/usr/include/raylib.h`). Cross-compilation will require
you to recompile `raylib` for the target platform, and for this purpose the included submodule must be used.

The makefile calls `clang`/`clang++` and links with `mold` directly. The code itself should be compiler-agnostic, so another compiler or linker
only needs the `CXX`, `C` and `LD` variables at the top of the makefile changed.

Live track-splitting via real-time inference using a small LSTM model is supported via `libtorch` on Linux only. To enable this support, use `make torch=y` while building, and ensure LibTorch is available on your system (expected at `/opt/libtorch-cpu/`, although you may have to change it manually depending on your system configuration).

For cross-compilation to Windows, the following tools are required:

* `x86_64-w64-mingw32-g++`
* `x86_64-w64-mingw32-gcc`
* `x86_64-w64-mingw32-ld`

Keep in mind that Windows support is currently in an experimental stage, so expect weird quirks and bugs.

# build
Don't forget to `git clone --recurse-submodules` when obtaining the source code!

Currently, only Linux and to a lesser extent, Windows, are supported, however, the dependencies and codebase are
purposely OS-agnostic, and with some tweaking can run on MacOS as
well. Contributions to improve cross-platform interoperability are welcomed.

Simply run `make` in the project's root directory for a debug build, or `make rel=y` for an optimized release build. The resulting executable
will be formed at `./bin/nodumi`

Cross-compilation to Windows is done through `make arch=win`, and the following executable is formed at `./bin/nodumi.exe`

To compile the documentation, run `make doc`. This needs `xelatex` (run with `--shell-escape`), the `minted` package with `pygments`, and the `svg`
package with `inkscape`.

# usage
If executing `nodumi` from the command-line, up to two optional arguments (in any order) are accepted:
```sh
./bin/nodumi path/to/file.(mid|midi|mki) path/to/image.(png|jpg|jpeg)
```

Files can also be dropped onto the window. Otherwise, use the built-in graphical commands.

# license
`nodumi` is licensed under GPLv3.
