#include <iostream>
#include <fstream>
#include <string>

#include "window.h"
#include "note.h"

using std::cerr;
using std::endl;
using std::string;
using std::ifstream;

#define VERSION "0.01a"

int main(int argc, char* argv[]) {

  if (argc !=2) {
    cerr << "Invalid Usage: specify a input file!" << endl;
    exit(1);
  }
  
  string filename = argv[1];

  ifstream filecheck;
  filecheck.open(filename);
  if (!filecheck) {
    cerr << "Invalid file: " << filename << "!" << endl;
    exit(1);
  }
  filecheck.close();

  window main = window(static_cast<string>("mviewer ") +static_cast<string>(VERSION));

  if (!main.init()) {
    cerr << "Failed to initialize window" << endl;
    exit(1);
  }


  mfile input;
  
  input.load(filename);

  main.clearBuffer();
  
  //draw middle line
 // bool drawLine = true; //TODO: add config file
  //middle line
  if (true){//drawLine) { 
    for (int y = 0; y < main.getHeight(); y++) {
    //  main.setPixelRGB(main.getWidth()/2, y, 255, 255, 255);
    }
  }
  
  bool state = true;
  
  int x, y, width = 0;
  int tempo = 0;

  Uint8* col[3]{reinterpret_cast<Uint8*>(0xAA), reinterpret_cast<Uint8*>(0x00), reinterpret_cast<Uint8*>(0xAA)};
  Uint8 r, g, b = 0;
  Sint32 shiftX = 0;
  colorRGB linecol(233,0,22);

  bool run = false;
  bool drawLine = true;
  bool applyTempoChange = false;
  bool mouseDown = false;
  
  char noteOverlap = 1;

  note* notes = input.getAllNotes();
  tempo = notes[0].tempo;

  while (state){
    
    note renderNote;

    if (run) {
      main.clearBuffer();

      if (drawLine) {
        for (int y = 0; y < main.getHeight(); y++) {
          main.setPixelRGB(main.getWidth()/2, y, linecol.r, linecol.g, linecol.b);
        }
        main.update();
      }
        
    }

    main.update();
    SDL_Event event;

    switch (main.eventHandler(event, shiftX)){
      case 1: // program closing
        state = false;
        break;
      case 2: // play/pause (spacebar)
        run = !run;
        applyTempoChange = true;
        break;
      case 3:
        mouseDown = true;
        run = false;
        break;
      case 4:
        mouseDown = false;
        run = false;
        applyTempoChange = true;
        break;
      case 5:
        if (mouseDown) {
          input.shift(static_cast<int>(shiftX));
          run = true;
          applyTempoChange = false;
        }
        break;
    }

  }
  
  main.terminate();
  return 0;
}
