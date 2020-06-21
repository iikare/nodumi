CC = g++

CFLAGS = --std=c++11 -Wall -Wextra -g
CFLAGSOD = --std=c99 -w -fpermissive -g $(shell pkg-config --cflags gtk+-3.0)

LFLAGS = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf $(shell pkg-config --libs gtk+-3.0)

OBJS = main.o note.o window.o misc.o menu.o color.o colorgen.o Binasc.o MidiEvent.o MidiEventList.o MidiFile.o MidiMessage.o osdialog.o osdialog_gtk3.o

DIR = build
SRC = src

NAME = viewer

all: $(NAME)

$(NAME): $(addprefix $(DIR)/, $(OBJS))
	$(CC) $(CFLAGS) $(LFLAGS) -o $(NAME) $(addprefix $(DIR)/, $(OBJS)) 

$(DIR)/main.o: ./src/main.cc
	$(CC) $(CFLAGS) -c ./src/main.cc  -o build/main.o

$(DIR)/note.o: ./src/note.cc ./src/note.h
	$(CC) $(CFLAGS) -c ./src/note.cc -o build/note.o

$(DIR)/window.o: ./src/window.cc ./src/window.h
	$(CC) $(CFLAGS) -c ./src/window.cc -o build/window.o

$(DIR)/misc.o: ./src/misc.cc ./src/misc.h
	$(CC) $(CFLAGS) -c ./src/misc.cc -o build/misc.o

$(DIR)/menu.o: ./src/menu.cc ./src/menu.h
	$(CC) $(CFLAGS) -c ./src/menu.cc -o build/menu.o

$(DIR)/color.o: ./src/color.cc ./src/color.h
	$(CC) $(CFLAGS) -c ./src/color.cc -o build/color.o

$(DIR)/colorgen.o: ./src/colorgen.cc
	$(CC) $(CFLAGS) -c ./src/colorgen.cc -o build/colorgen.o

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

clean: 
	rm -f $(NAME) *.o *.gch build/*
