ifeq ($(strip $(asan)),)
NONSTD=
else # use address sanitizer
NONSTD=-fsanitize=address -fno-omit-frame-pointer 
endif

ifeq ($(strip $(prof)),)
#NONSTD+=
else
NONSTD+=-pg -g 
endif

ifeq ($(strip $(arch)),)

CXX=clang++
C=clang
LD=-fuse-ld=mold
CFLAGSOSD=--std=c99  $(shell pkg-config --cflags gtk+-3.0)
LFLAGSOSD=$(shell pkg-config --libs gtk+-3.0)
SRCSOSD=$(OSDDIR)/osdialog.c $(OSDDIR)/osdialog_gtk3.c

ifneq ($(strip $(torch)),)
LIBTORCH_ROOT := /opt/libtorch-cpu
LIBTORCH_INCLUDES:= -I$(LIBTORCH_ROOT)/include \
										-I$(LIBTORCH_ROOT)/include/torch/csrc \
										-I$(LIBTORCH_ROOT)/include/torch/csrc/api/include
LIBTORCH_LIB_PATH := -L$(LIBTORCH_ROOT)/lib
LIBTORCH_LIB_FLAGS := -Wl,-rpath=$(LIBTORCH_ROOT)/lib -ltorch -ltorch_cpu -lc10
LIBTORCH_DEFS=-DUSE_TORCH_CLASSIFIER
endif

else ifeq ($(strip $(arch)),win)

CXX=x86_64-w64-mingw32-g++
CC=x86_64-w64-mingw32-ld
C=x86_64-w64-mingw32-gcc
#LD=-fuse-ld=mold
CFLAGSOSD=--std=c99
LFLAGSOSD=-lwinmm -lcomdlg32 -lgdi32
SRCSOSD=$(OSDDIR)/osdialog.c $(OSDDIR)/osdialog_win.c
DEPDEF=-DTARGET_WIN

endif

ifeq ($(strip $(rel)),)
ifneq ($(strip $(torch)),)
#RELFLAGS=-O3
RELFLAGS=-Og -g
else
RELFLAGS=-Og -g
endif
else # release build
RELFLAGS=-DTARGET_REL -O3 -ffast-math
# thinLTO doesn't exist for cross-compiler
ifeq ($(strip $(arch)),)
RELFLAGS+=-flto=thin
LD+=-flto=thin
else ifeq ($(strip $(arch)),win)
#LFLAGS+=-mwindows # remove console window
endif

endif


# annoying whitespace
ifeq ($(strip $(arch)),)
CFLAGS=--std=c++23 -Wall -Wextra -fopenmp $(NONSTD)$(RELFLAGS)$(DEPDEF)
else ifeq ($(strip $(arch)),win)
CFLAGS=--std=c++23 -Wall -Wextra -fopenmp $(NONSTD)$(RELFLAGS) $(DEPDEF)
endif

CFLAGSSTD=$(CFLAGS) -fno-exceptions
CFLAGSRTM=$(CFLAGS) -w # suppress library warnings

ifeq ($(strip $(arch)),)
LFLAGS+=$(LD) -lraylib -lasound -lpthread -ljack $(LFLAGSOSD)
CFLAGSRTM+=-D__LINUX_ALSA__ -D__UNIX_JACK__
else ifeq ($(strip $(arch)),win)
LFLAGS+=-static -static-libgcc -static-libstdc++ -L./dpd/raylib/src -lraylib -lpthread $(LFLAGSOSD) data/misc/i.res
CFLAGSRTM=-D__WINDOWS_MM__
endif

PREREQ_DIR=@mkdir -p $(@D)

MFDIR=dpd/midifile
OSDDIR=dpd/osdialog
RTMDIR=dpd/rtmidi
SRCDIR=src
BUILDDIR=build
BINDIR=bin

NAME=$(addprefix $(BINDIR)/, nodumi)

SRCSTORCH=$(SRCDIR)/classifier.cc
OBJSTORCH=$(BUILDDIR)/classifier.o

SRCS2=$(wildcard $(SRCDIR)/*.cc)
OBJS2=$(patsubst $(SRCDIR)/%.cc, $(BUILDDIR)/%.o, $(SRCS2))

SRCS=$(filter-out $(SRCSTORCH), $(SRCS2))
OBJS=$(filter-out $(OBJSTORCH), $(OBJS2))

SRCSMF=$(wildcard $(MFDIR)/*.cpp)
OBJSMF=$(patsubst $(MFDIR)/%.cpp, $(BUILDDIR)/%.o, $(SRCSMF))

#SRCSOSD=$(OSDDIR)/osdialog.c $(OSDDIR)/osdialog_win.c
OBJSOSD=$(patsubst $(OSDDIR)/%.c, $(BUILDDIR)/%.o, $(SRCSOSD))

SRCSRTM=$(RTMDIR)/RtMidi.cpp
OBJSRTM=$(patsubst $(RTMDIR)/%.cpp, $(BUILDDIR)/%.o, $(SRCSRTM))

all:
	@mkdir -p ./src/agh

ifeq ($(strip $(arch)),win)
	@./tool/cross.sh
	@./tool/objtype.sh win
else
	@./tool/objtype.sh linux
endif

ifneq ($(strip $(relp)),end)
	@$(MAKE) --no-print-directory pre
endif
	@$(MAKE) --no-print-directory $(NAME)
ifneq ($(strip $(relp)),begin)
	@./tool/generate.sh
endif

ifneq ($(strip $(rel)),)
ifeq ($(strip $(prof)),)
ifeq ($(strip $(arch)),win)
	@strip $(NAME).exe
else
	@strip $(NAME)
endif
endif
endif


re: clean
	@$(MAKE) --no-print-directory

f: clean
	@$(MAKE) rel=a relp=begin torch=y --no-print-directory
	@$(MAKE) arch=win rel=a relp=end --no-print-directory

pre:
	@./tool/generate.sh ON
	@./tool/codepoint.sh

doc:
	@$(MAKE) -C doc

$(NAME): $(OBJSTORCH) $(OBJS) $(OBJSMF) $(OBJSOSD) $(OBJSRTM) | $(@D)
	$(PREREQ_DIR)

ifeq ($(strip $(torch)),)
	$(CXX) $(CFLAGSSTD) -o $(NAME) $(OBJS) $(OBJSTORCH) $(OBJSMF) $(OBJSOSD) $(OBJSRTM) $(LFLAGS)
else
	$(CXX) $(CFLAGSSTD) -o $(NAME) $(OBJS) $(OBJSTORCH) $(OBJSMF) $(OBJSOSD) $(OBJSRTM) $(LFLAGS) $(LIBTORCH_LIB_PATH) $(LIBTORCH_LIB_FLAGS)
endif

$(OBJS): $(BUILDDIR)/%.o: $(SRCDIR)/%.cc
	$(PREREQ_DIR)
	$(CXX) $(CFLAGSSTD) -o $@ -c $<

$(OBJSTORCH): $(BUILDDIR)/%.o: $(SRCDIR)/%.cc
	$(PREREQ_DIR)

ifeq ($(strip $(torch)),)
	$(CXX) $(CFLAGSSTD) -o $@ -c $<
else
ifeq ($(strip $(win)),)
	$(CXX) $(CFLAGSSTD) -w -o $@ $(LIBTORCH_DEFS) $(LIBTORCH_INCLUDES) -c $<
else
	$(CXX) $(CFLAGSSTD) -o $@ -c $<
endif
endif

$(OBJSMF): $(BUILDDIR)/%.o: $(MFDIR)/%.cpp
	$(PREREQ_DIR)
	$(CXX) $(CFLAGSSTD) -o $@ -c $<

$(OBJSOSD): $(BUILDDIR)/%.o: $(OSDDIR)/%.c
	$(PREREQ_DIR)
	$(C) $(CFLAGSOSD) -o $@ -c $<

$(OBJSRTM): $(BUILDDIR)/%.o: $(RTMDIR)/%.cpp
	$(PREREQ_DIR)
	$(CXX) $(CFLAGSRTM) -o $@ -c $<

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

