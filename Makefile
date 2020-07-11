CC = g++

CFLAGS = --std=c++17 -Wall -Wextra -g -Wno-class-memaccess -D__LINUX_ALSA__ -D__UNIX_JACK__ -D__RTMIDI_DEBUG__ 

CFLAGSOD = --std=c99 -w -fpermissive -g  $(shell pkg-config --cflags gtk+-3.0)

LFLAGS = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lasound -lpthread -ljack $(shell pkg-config --libs gtk+-3.0)

OBJS = main.o note.o window.o misc.o menu.o file.o color.o colorgen.o input.o image.o Binasc.o MidiEvent.o MidiEventList.o MidiFile.o MidiMessage.o osdialog.o osdialog_gtk3.o RtMidi.o picopng.o

DIR = build
SRC = src

NAME = viewer

all: $(NAME)

$(NAME): $(addprefix $(DIR)/, $(OBJS)) | $(@D)
	$(CC) $(CFLAGS) $(LFLAGS) -o $(NAME) $(addprefix $(DIR)/, $(OBJS)) 

$(DIR)/main.o: ./src/main.cc
	mkdir -p $(dir $@)	
	$(CC) $(CFLAGS) -c ./src/main.cc  -o build/main.o

$(DIR)/note.o: ./src/note.cc ./src/note.h
	$(CC) $(CFLAGS) -c ./src/note.cc -o build/note.o

$(DIR)/window.o: ./src/window.cc ./src/window.h
	$(CC) $(CFLAGS) -c ./src/window.cc -o build/window.o

$(DIR)/misc.o: ./src/misc.cc ./src/misc.h
	$(CC) $(CFLAGS) -c ./src/misc.cc -o build/misc.o

$(DIR)/menu.o: ./src/menu.cc ./src/menu.h
	$(CC) $(CFLAGS) -c ./src/menu.cc -o build/menu.o

$(DIR)/file.o: ./src/file.cc ./src/file.h
	$(CC) $(CFLAGS) -c ./src/file.cc -o build/file.o

$(DIR)/color.o: ./src/color.cc ./src/color.h
	$(CC) $(CFLAGS) -c ./src/color.cc -o build/color.o

$(DIR)/colorgen.o: ./src/colorgen.cc
	$(CC) $(CFLAGS) -c ./src/colorgen.cc -o build/colorgen.o

$(DIR)/input.o: ./src/input.cc
	$(CC) $(CFLAGS) -c ./src/input.cc -o build/input.o

$(DIR)/image.o: ./src/image.cc
	$(CC) $(CFLAGS) -c ./src/image.cc -o build/image.o

$(DIR)/osdialog.o: ./dpd/osdialog/osdialog.c ./dpd/osdialog/osdialog.h
	$(CC)  $(CFLAGSOD) -c ./dpd/osdialog/osdialog.c -o build/osdialog.o

$(DIR)/osdialog_gtk3.o: ./dpd/osdialog/osdialog_gtk3.c
	$(CC) $(CFLAGSOD) -c ./dpd/osdialog/osdialog_gtk3.c -o build/osdialog_gtk3.o

$(DIR)/Binasc.o: ./dpd/midifile/Binasc.cpp ./dpd/midifile/Binasc.h
	$(CC) $(CFLAGS) -c ./dpd/midifile/Binasc.cpp -o build/Binasc.o

$(DIR)/MidiEvent.o: ./dpd/midifile/MidiEvent.cpp ./dpd/midifile/MidiEvent.h
	$(CC) $(CFLAGS) -c ./dpd/midifile/MidiEvent.cpp -o build/MidiEvent.o

$(DIR)/MidiEventList.o: ./dpd/midifile/MidiEventList.cpp ./dpd/midifile/MidiEventList.h
	$(CC) $(CFLAGS) -c ./dpd/midifile/MidiEventList.cpp -o build/MidiEventList.o

$(DIR)/MidiFile.o: ./dpd/midifile/MidiFile.cpp ./dpd/midifile/MidiFile.h
	$(CC) $(CFLAGS) -c ./dpd/midifile/MidiFile.cpp -o build/MidiFile.o

$(DIR)/MidiMessage.o: ./dpd/midifile/MidiMessage.cpp ./dpd/midifile/MidiMessage.h
	$(CC) $(CFLAGS) -c ./dpd/midifile/MidiMessage.cpp -o build/MidiMessage.o

$(DIR)/RtMidi.o: ./dpd/rtmidi/RtMidi.cpp ./dpd/rtmidi/RtMidi.h
	$(CC) $(CFLAGS) -c -w ./dpd/rtmidi/RtMidi.cpp -o build/RtMidi.o

$(DIR)/picopng.o: ./dpd/picoPNG/picopng.cpp ./dpd/picoPNG/picopng.h
	$(CC) $(CFLAGS) -c -w ./dpd/picoPNG/picopng.cpp -o build/picopng.o

clean: 
	rm -f $(NAME) build/*.o build/*.gch
