ifeq ($(strip $(asan)),)
NONSTD=
else # use address sanitizer
NONSTD=-fsanitize=address -fno-omit-frame-pointer 
endif

ifeq ($(strip $(prof)),)
#NONSTD+=
else # for valgrind bug reading DWARF-5 format w/ clang 14
NONSTD+=-gdwarf-4
endif

ifeq ($(strip $(arch)),)

CXX=clang++
C=clang
LD=-fuse-ld=mold
CFLAGSOSD=--std=c99  $(shell pkg-config --cflags gtk+-3.0) 
LFLAGSOSD=$(shell pkg-config --libs gtk+-3.0)
SRCSOSD=$(OSDDIR)/osdialog.c $(OSDDIR)/osdialog_gtk3.c
DEPDEF=-D__LINUX_ALSA__ -D__UNIX_JACK__

else ifeq ($(strip $(arch)),win)

CXX=x86_64-w64-mingw32-g++
CC=x86_64-w64-mingw32-ld
C=x86_64-w64-mingw32-gcc
#LD=-fuse-ld=mold
CFLAGSOSD=--std=c99
LFLAGSOSD=-lwinmm -lcomdlg32 -lgdi32
SRCSOSD=$(OSDDIR)/osdialog.c $(OSDDIR)/osdialog_win.c
DEPDEF=-D__WINDOWS_MM__ -DLOCRAY -D_USE_MATH_DEFINES 

endif

ifeq ($(strip $(rel)),)
RELFLAGS=-Og -g
else # release build
RELFLAGS=-DNO_DEBUG -DREL -O3

# thinLTO doesn't exist for cross-compiler
ifeq ($(strip $(arch)),)
RELFLAGS+=-flto=thin
LD += -flto=thin
else ifeq ($(strip $(arch)),win)
LFLAGS+=-mwindows # remove console window 
endif

endif


CFLAGS=--std=c++20 -Wall -Wextra $(NONSTD)$(RELFLAGS) $(DEPDEF)
CFLAGSSTD=$(CFLAGS) -fno-exceptions
CFLAGSRTM=$(CFLAGS) -w # suppress library warnings 
CFLAGSCIE=$(CFLAGS) 

ifeq ($(strip $(arch)),)
LFLAGS+=$(LD) -lraylib -lasound -lpthread -ljack $(LFLAGSOSD) 
else ifeq ($(strip $(arch)),win)
LFLAGS+= -static -static-libgcc -static-libstdc++ -L./dpd/raylib/src -lraylib -lpthread $(LFLAGSOSD) data/misc/i.res 
endif

PREREQ_DIR=@mkdir -p $(@D)

MFDIR=dpd/midifile
OSDDIR=dpd/osdialog
RTMDIR=dpd/rtmidi
CIEDIR=dpd/CIEDE2000
SRCDIR=src
BUILDDIR=build
BINDIR=bin

NAME= $(addprefix $(BINDIR)/, nodumi)

SRCS=$(wildcard $(SRCDIR)/*.cc)
OBJS=$(patsubst $(SRCDIR)/%.cc, $(BUILDDIR)/%.o, $(SRCS))

SRCSMF=$(wildcard $(MFDIR)/*.cpp)
OBJSMF=$(patsubst $(MFDIR)/%.cpp, $(BUILDDIR)/%.o, $(SRCSMF))

#SRCSOSD=$(OSDDIR)/osdialog.c $(OSDDIR)/osdialog_win.c
OBJSOSD=$(patsubst $(OSDDIR)/%.c, $(BUILDDIR)/%.o, $(SRCSOSD))

SRCSRTM=$(RTMDIR)/RtMidi.cpp
OBJSRTM=$(patsubst $(RTMDIR)/%.cpp, $(BUILDDIR)/%.o, $(SRCSRTM))

SRCSCIE=$(CIEDIR)/CIEDE2000.cpp
OBJSCIE=$(patsubst $(CIEDIR)/%.cpp, $(BUILDDIR)/%.o, $(SRCSCIE))

all:
	@mkdir -p ./src/agh

ifeq ($(strip $(arch)),win)
	@./tool/cross.sh
endif

	@$(MAKE) --no-print-directory pre
	@$(MAKE) --no-print-directory $(NAME)
	@./tool/generate.sh

ifeq ($(strip $(arch)),win)
	@$(MAKE) --no-print-directory --silent cleanbuild
endif

re: clean
	@$(MAKE) --no-print-directory

f: clean
	@$(MAKE) rel=a --no-print-directory
	@$(MAKE) arch=win rel=a cleanbuild
	@$(MAKE) arch=win rel=a --no-print-directory
	@$(MAKE) arch=win rel=a cleanbuild

pre:
	@./tool/generate.sh ON
	@./tool/codepoint.sh

$(NAME): $(OBJS) $(OBJSMF) $(OBJSOSD) $(OBJSRTM) $(OBJSCIE) | $(@D)
	$(PREREQ_DIR)
	$(CXX) $(CFLAGSSTD) -o $(NAME) $(OBJS) $(OBJSMF) $(OBJSOSD) $(OBJSRTM) $(OBJSCIE) $(LFLAGS)

$(OBJS): $(BUILDDIR)/%.o: $(SRCDIR)/%.cc
	$(PREREQ_DIR)
	$(CXX) $(CFLAGSSTD) -o $@ -c $< 

$(OBJSMF): $(BUILDDIR)/%.o: $(MFDIR)/%.cpp
	$(PREREQ_DIR)
	$(CXX) $(CFLAGSSTD) -o $@ -c $< 

$(OBJSOSD): $(BUILDDIR)/%.o: $(OSDDIR)/%.c
	$(PREREQ_DIR)
	$(C) $(CFLAGSOSD) -o $@ -c $< 

$(OBJSRTM): $(BUILDDIR)/%.o: $(RTMDIR)/%.cpp
	$(PREREQ_DIR)
	$(CXX) $(CFLAGSRTM) -o $@ -c $< 

$(OBJSCIE): $(BUILDDIR)/%.o: $(CIEDIR)/%.cpp
	$(PREREQ_DIR)
	$(CXX) $(CFLAGSCIE) -o $@ -c $< 

clean:
	@echo
	@echo "cleaning old build files"
	@echo
	@$(MAKE) --no-print-directory cleanbuild
	@$(MAKE) --no-print-directory cleanexec

cleanbuild:
	rm -f build/*.o
	rm -f src/agh/*

cleanexec:
	rm -f $(NAME) $(NAME).exe

.PHONY: all clean

