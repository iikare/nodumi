CC = clang++
C = clang
#CC = x86_64-w64-mingw32-gcc-11.2.0 

#LD =
LD = -DLLVM_ENABLE_LLD=ON #-fuse-ld=gold -gdwarf-4

LINUX = -D__LINUX_ALSA__ -D__UNIX_JACK__ 
#WINDOWS = -D__WINDOWS_MM__

# use clang13 or higher
NONSTD =  

CFLAGS = --std=c++20 -Wall -Wextra $(NONSTD) -g $(LD) $(LINUX) 
CFLAGSOSD = --std=c99 -w -fpermissive -g $(LD) $(shell pkg-config --cflags gtk+-3.0) 
CFLAGSRTM = $(CFLAGS) -w
CFLAGSCIE = $(CFLAGS) -w

LFLAGS = -lraylib -lasound -lpthread -ljack $(shell pkg-config --libs gtk+-3.0)

PREREQ_DIR=@mkdir -p $(@D)

MFDIR = dpd/midifile
OSDDIR = dpd/osdialog
RTMDIR = dpd/rtmidi
CIEDIR = dpd/CIEDE2000
SRCDIR = src
BUILDDIR = build
BINDIR = bin

NAME =  $(addprefix $(BINDIR)/, nodumi)

SRCS = $(wildcard $(SRCDIR)/*.cc)
OBJS = $(patsubst $(SRCDIR)/%.cc, $(BUILDDIR)/%.o, $(SRCS))

SRCSMF = $(wildcard $(MFDIR)/*.cpp)
OBJSMF = $(patsubst $(MFDIR)/%.cpp, $(BUILDDIR)/%.o, $(SRCSMF))

SRCSOSD = $(OSDDIR)/osdialog.c $(OSDDIR)/osdialog_gtk3.c
#SRCSOSD = $(OSDDIR)/osdialog.c $(OSDDIR)/osdialog_win.c
OBJSOSD = $(patsubst $(OSDDIR)/%.c, $(BUILDDIR)/%.o, $(SRCSOSD))

SRCSRTM = $(RTMDIR)/RtMidi.cpp
OBJSRTM = $(patsubst $(RTMDIR)/%.cpp, $(BUILDDIR)/%.o, $(SRCSRTM))

SRCSCIE = $(CIEDIR)/CIEDE2000.cpp
OBJSCIE = $(patsubst $(CIEDIR)/%.cpp, $(BUILDDIR)/%.o, $(SRCSCIE))

all: 
	@$(MAKE) --no-print-directory pre
	@$(MAKE) --no-print-directory $(NAME)
	@./tool/generate.sh 

re: clean
	@$(MAKE) --no-print-directory

pre:
	@./tool/generate.sh ON
	@./tool/codepoint.sh

$(NAME): $(OBJS) $(OBJSMF) $(OBJSOSD) $(OBJSRTM) $(OBJSCIE) | $(@D)
	$(PREREQ_DIR)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(OBJSMF) $(OBJSOSD) $(OBJSRTM) $(OBJSCIE) $(LFLAGS)

$(OBJS): $(BUILDDIR)/%.o: $(SRCDIR)/%.cc
	$(PREREQ_DIR)
	$(CC) $(CFLAGS) -o $@ -c $< 

$(OBJSMF): $(BUILDDIR)/%.o: $(MFDIR)/%.cpp
	$(PREREQ_DIR)
	$(CC) $(CFLAGS) -o $@ -c $< 

$(OBJSOSD): $(BUILDDIR)/%.o: $(OSDDIR)/%.c
	$(PREREQ_DIR)
	$(C) $(CFLAGSOSD) -o $@ -c $< 

$(OBJSRTM): $(BUILDDIR)/%.o: $(RTMDIR)/%.cpp
	$(PREREQ_DIR)
	$(CC) $(CFLAGSRTM) -o $@ -c $< 

$(OBJSCIE): $(BUILDDIR)/%.o: $(CIEDIR)/%.cpp
	$(PREREQ_DIR)
	$(CC) $(CFLAGSCIE) -o $@ -c $< 

clean:
	@echo
	@echo "cleaning old build files"
	@echo
	rm -f build/*.o $(NAME)
	rm -f src/agh/*

.PHONY: all clean

