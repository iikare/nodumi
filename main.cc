#include <iostream>
#include <fstream>
#include <string>

#include "misc.h"
#include "window.h"
#include "note.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::ifstream;


int main(int argc, char* argv[]) {

  if (argc !=2) {
    cerr << "error: invalid usage - specify a input file!" << endl;
    exit(1);
  }
  
  string filename = argv[1];

  ifstream filecheck;
  filecheck.open(filename);
  if (!filecheck) {
    cerr << "error: invalid file: " << filename << "!" << endl;
    exit(1);
  }
  filecheck.close();

  window main = window(static_cast<string>("mviewer ") +static_cast<string>(VERSION));

  if (!main.init()) {
    cerr << "error: failed to initialize window" << endl;
    exit(1);
  }


  mfile input;
  
  input.load(filename);

  main.clearBuffer();
  
  cerr << "info: initializing render logic" << endl;

  /*
   * * * * * * * 
   * VARIABLES *
   * * * * * * *
   */ 

  bool state = true;
  
  int x, y, width = 0;
  int tempo = 0;

  Sint32 shiftX = 0;
  colorRGB lineColor(233,0,22); 
  colorRGB noteColorOn(0,100,255);
  colorRGB noteColorOff(0,0,255);
  colorRGB colorByNote(255,255,255);
  bool colorByPart = true;
  bool noteOn = false;
  
  bool run = false;
  bool end = false;
  bool drawLine = true;
  bool applyTempoChange = false;
  bool mouseDown = false;
  
  char noteOverlap = 1;

  note* notes = input.getNotes();

  input.scaleToWindow(main.getHeight());

  note& renderNote = notes[0];
  tempo = notes[0].tempo;
  int noteHeight = notes[0].height;

  SDL_Event event;

  /*
   *  TODO:
   *    add user-customizable line color
   *    add menu bar on top
   *    add file picker
   *    add color picker for parts
   *    add color by parts
   *    add config file parsing
   *    scale notes by window size
   */


  while (state){
    //cerr << "note height is " << noteHeight << endl; 

    if (run && !end) {
      main.clearBuffer();
    }


    // now line will always render regardless of play state
    if (drawLine) {
      for (int y = 0; y < main.getHeight(); y++) {
        main.setPixelRGB(main.getWidth()/2, y, lineColor.r, lineColor.g, lineColor.b);
      }
    }
    if (run && !end) {
      // render notes
      for (int i = 0; i < input.getNoteCount(); i++) {
        // get current note
        renderNote = notes[i];
       
        // check visibility 
        if(!main.noteVisible(renderNote)){
          renderNote.render = false;
        }
        else {
          renderNote.render = true;
        }

        x = main.getWidth() + round(renderNote.x/9);
       // y = round(main.getHeight() *(static_cast<double>(renderNote.y - 0x80)/input.getNoteRange()));
        //y =  -(renderNote.y - 63) * 14 + main.getHeight()/2;
        y = main.getHeight() - round(main.getHeight() * static_cast<double>(renderNote.y - MIN_NOTE_IDX + 1)/(NOTE_RANGE + 1));
        cout << "render note y is " << renderNote.y << " while calc y is " << y << endl;
        width = renderNote.duration/TICK_TO_SEC;
        
        if (colorByPart) {
         if (x <= main.getHeight()/2 && x >= main.getWidth()/2 - width) {
            colorByNote.setRGB(255, 255, 255);
            tempo = renderNote.tempo;
         }
        }
        if (x < main.getWidth() && x > -width) {

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
      if (applyTempoChange) {
        input.updateTempo(tempo);
      }
    }

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
  }
  
  main.terminate();
  return 0;
}
