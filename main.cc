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
  
  cerr << "info: initializing render logic" << endl;

  bool state = true;
  
  int x, y, width = 0;
  int tempo = 0;

  Uint8* col[3]{reinterpret_cast<Uint8*>(0xAA), reinterpret_cast<Uint8*>(0x00), reinterpret_cast<Uint8*>(0xAA)};
  Uint8 r, g, b = 0;
  Sint32 shiftX = 0;
  colorRGB lineColor(233,0,22); 
  colorRGB noteColorOn(0,100,255);
  colorRGB noteColorOff(0,0,255);
  colorRGB colorByNote(255,255,255);
  bool colorByPart = true;
  bool noteOn = false;
  
  bool run = false;
  bool drawLine = true;
  bool applyTempoChange = false;
  bool mouseDown = false;
  
  char noteOverlap = 1;

  note* notes = input.getNotes();
  tempo = notes[0].tempo;
  
  /*
   *  TODO:
   *    add user-customizable line color
   *    add menu bar on top
   *    add file picker
   *    add color picker for parts
   *    add config file parsing
   */


  while (state){
    note& renderNote = notes[0];
    
    // now line will always render regardless of play state
    if (drawLine) {
      for (int y = 0; y < main.getHeight(); y++) {
        main.setPixelRGB(main.getWidth()/2, y, lineColor.r, lineColor.g, lineColor.b);
      }
    }
    cerr << "is running: " << run << endl; 
    if (run) {
      main.clearBuffer();

      // render notes
      for (int i = 0; i < input.getNoteCount(); i++) {
        // get current note
        renderNote = notes[i];

        // assume note is offscreen
        
        if(!main.noteVisible(renderNote)){
          renderNote.render = false;
        }
        else {
          renderNote.render = true;
        }

        x = main.getWidth() + round(renderNote.x/9);
        y = -(renderNote.y - 63) * 14 + main.getHeight()/2;
        width = renderNote.duration; //main.getWidth() + round((renderNote.x + renderNote.duration)/9) - x - 1;
        cerr << "width in pixels is: " << width << endl;

        
        if (colorByPart) {
         if (x <= main.getHeight()/2 && x >= main.getWidth()/2 - width) {
            colorByNote.setRGB(255, 255, 255);
            tempo = renderNote.tempo;
         }
        }
        if (x < main.getHeight() && x > -width) {

          if (x <= main.getWidth()/2 && x >= main.getWidth()/2 - width) {
            noteOn = true;
          }
          else {
            noteOn = false;
          }

          
          for (int j = 0; j < width; j++) {
            for (int k = 0; k < renderNote.height; k++) {
              
              if (colorByPart) {
                // implement color shifting
              }

              if (noteOn) {
                main.setPixelRGB(x + j, y + k, noteColorOn.r, noteColorOn.g, noteColorOn.b);
              }
              else {
                main.setPixelRGB(x + j, y + k, noteColorOff.r, noteColorOff.g, noteColorOff.b);
              }

              main.setPixelHex(x + j, y + k, main.getPixelHex(x + j, y + k) + 1);
            }
          }
        }
      }
    }

    if (applyTempoChange) {
      input.update(tempo);
    }

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
    main.update();
    main.clearBuffer();
  }
  
  main.terminate();
  return 0;
}
