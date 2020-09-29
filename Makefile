CC = g++

CFLAGS = --std=c++17 -Wall -Wextra -g -fuse-ld=gold
CFLAGSOSD = --std=c99 -w -fpermissive -g -fuse-ld=gold $(shell pkg-config --cflags gtk+-3.0)
CFLAGSRTM = $(CFLAGS) -w -D__LINUX_ALSA__ -D__UNIX_JACK__ -D__RTMIDI_DEBUG__

LFLAGS = -lraylib -lasound -lpthread -ljack $(shell pkg-config --libs gtk+-3.0)

MFDIR = dpd/midifile
OSDDIR = dpd/osdialog
RTMDIR = dpd/rtmidi
SRCDIR = src
BUILDDIR = build
BINDIR = bin

NAME =  $(addprefix $(BINDIR)/, kelumi)

SRCS = $(wildcard $(SRCDIR)/*.cc)
OBJS = $(patsubst $(SRCDIR)/%.cc, $(BUILDDIR)/%.o, $(SRCS))

SRCSMF = $(wildcard $(MFDIR)/*.cpp)
OBJSMF = $(patsubst $(MFDIR)/%.cpp, $(BUILDDIR)/%.o, $(SRCSMF))

SRCSOSD = $(OSDDIR)/osdialog.c $(OSDDIR)/osdialog_gtk3.c
OBJSOSD = $(patsubst $(OSDDIR)/%.c, $(BUILDDIR)/%.o, $(SRCSOSD))

SRCSRTM = $(RTMDIR)/RtMidi.cpp
OBJSRTM = $(patsubst $(RTMDIR)/%.cpp, $(BUILDDIR)/%.o, $(SRCSRTM))

all: $(NAME)

re: clean
	$(MAKE)

$(NAME): $(OBJS) $(OBJSMF) $(OBJSOSD) $(OBJSRTM) | $(@D)
	$(CC) $(CFLAGS) $(LFLAGS) -o $(NAME) $(OBJS) $(OBJSMF) $(OBJSOSD) $(OBJSRTM)

$(OBJS): $(BUILDDIR)/%.o: $(SRCDIR)/%.cc
	$(CC) $(CFLAGS) -o $@ -c $< 

$(OBJSMF): $(BUILDDIR)/%.o: $(MFDIR)/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $< 

$(OBJSOSD): $(BUILDDIR)/%.o: $(OSDDIR)/%.c
	$(CC) $(CFLAGSOSD) -o $@ -c $< 

$(OBJSRTM): $(BUILDDIR)/%.o: $(RTMDIR)/%.cpp
	$(CC) $(CFLAGSRTM) -o $@ -c $< 

clean:
	rm -rf build/* $(NAME)

.PHONY: all clean

