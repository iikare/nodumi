# nodumi - graphical MIDI visualizer
Currently, docs and info are in the process of being updated -- please wait warmly!.

At some point in the future as development progresses, a usage guide will be available [on my website](https://iika.re/nodumi/ "nodumi website").

A project logo will eventually take this statement's place.

# dependencies
This project requires several dependencies, many of which are submodules to this repository.

* `CIEDE2000` (supplied as a submodule)
* `midifile` (supplied as a submodule)
* `osdialog` (supplied as a submodule)
* `rtmidi` (supplied as a submodule)
* `raylib`
* GNU `make`

Cross-compilation will require you to recompile raylib for the target platform. The current build script assumes you are building on a Linux environment with raylib installed in the appropriate directories (`/lib/libraylib.so` and `/usr/include/raylib.h`).

Note that CIEDE2000 is only required if `COLDIST_CIE00` is defined at compile-time, and you wish to use the CIEDE2000 
deltaE color-distance algorithm in generating a color palette from a background image.

# build
Don't forget to `git clone --recurse-submodules` when obtaining the source code!

Currently, only Linux is supported, however, the dependencies and codebase are
purposely OS-agnostic, and with some tweaking can run on Windows and MacOS, as
well. Contributions to implement cross-platform interoperability are welcomed.

Simply run `make` in the project's root directory. The resulting executable
will be formed at `./bin/nodumi`

# usage
If executing `nodumi` from the command-line, one optional argument is accepted:
```
./bin/nodumi path/to/file.(mid|midi)
```

# license
`nodumi` is licensed under GPLv3.
