CC = g++

CFLAGS = --std=c++11 -Wall -Wextra -g

LFLAGS = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf

OBJS = main.o note.o window.o Binasc.o MidiEvent.o MidiEventList.o MidiFile.o MidiMessage.o Options.o

FILES = ./build/*.o

NAME = viewer

all: viewer

viewer: $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) -o viewer $(FILES) 

main.o: main.cc
	$(CC) $(CFLAGS) -c main.cc  -o build/main.o

note.o: note.cc note.h
	$(CC) $(CFLAGS) -c note.cc -o build/note.o

window.o: window.cc window.h
	$(CC) $(CFLAGS) -c window.cc -o build/window.o

Binasc.o: ./dpd/midifile/Binasc.cpp ./dpd/midifile/Binasc.h
	$(CC) $(CFLAGS) -c ./dpd/midifile/Binasc.cpp -o build/Binasc.o

Options.o: ./dpd/midifile/Options.cpp ./dpd/midifile/Options.h
	$(CC) $(CFLAGS) -c ./dpd/midifile/Options.cpp -o build/Options.o

MidiEvent.o: ./dpd/midifile/MidiEvent.cpp ./dpd/midifile/MidiEvent.h
	$(CC) $(CFLAGS) -c ./dpd/midifile/MidiEvent.cpp -o build/MidiEvent.o

MidiEventList.o: ./dpd/midifile/MidiEventList.cpp ./dpd/midifile/MidiEventList.h
	$(CC) $(CFLAGS) -c ./dpd/midifile/MidiEventList.cpp -o build/MidiEventList.o

MidiFile.o: ./dpd/midifile/MidiFile.cpp ./dpd/midifile/MidiFile.h
	$(CC) $(CFLAGS) -c ./dpd/midifile/MidiFile.cpp -o build/MidiFile.o

MidiMessage.o: ./dpd/midifile/MidiMessage.o ./dpd/midifile/MidiMessage.h
	$(CC) $(CFLAGS) -c ./dpd/midifile/MidiMessage.cpp -o build/MidiMessage.o

clean: 
	rm -f viewer *.o *.gch build/*
