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
  int shiftX = 100;
  
  colorRGB lineColor(233,0,22); 
  colorRGB noteColorOn(0,100,255);
  colorRGB noteColorOff(0,0,255);
  colorRGB colorByNote(255,255,255);
  bool colorByPart = true;
  bool noteOn = false;
  
  bool run = false;
  bool end = false;
  bool drawLine = true;
  bool noteShift = false;
  
  note* notes = input.getNotes();

  input.scaleToWindow(main.getHeight());

  note& renderNote = notes[0];
  note& lastNote = notes[sizeof(notes)];
  tempo = notes[0].tempo;

  SDL_Event event;

  /*
   *  TODO:
   *    add user-customizable line color
   *    add menu bar on top
   *    add file picker
   *    add color picker for parts
   *    add color by parts
   *    add config file parsing
   *    left/right arrow able to move
   *    scale notes by window size            DONE (test with note value 0)
   */


  while (state){
    //cerr << "note height is " << noteHeight << endl; 
    if (!end) {

      if (run) {
        main.clearBuffer();
      }


      // now line will always render regardless of play state
      if (drawLine) {
        for (int y = 0; y < main.getHeight(); y++) {
          main.setPixelRGB(main.getWidth()/2, y, lineColor.r, lineColor.g, lineColor.b);
        }
      }
      if (run) {
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

          x = main.getWidth() + round(renderNote.x/TICKS_TO_SEC);
          y = main.getHeight() - round(main.getHeight() * static_cast<double>(renderNote.y - MIN_NOTE_IDX + 3)/(NOTE_RANGE + 3));
          width = renderNote.duration/TICKS_TO_SEC;
          
          //cerr << "render note y is " << renderNote.y << " while calc y is " << y << endl;
          
          if (colorByPart) {
           if (x <= main.getHeight()/2 && x >= main.getWidth()/2 - width) {
              colorByNote.setRGB(255, 255, 255);
              tempo = renderNote.tempo;
           }
          }
          if (x < main.getWidth() && x > - width) {

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

              }
            }
          }
        }
        if (noteShift) {
          //cerr << lastNote.x/TICKS_TO_SEC + main.getWidth() << " vs. " << main.getWidth()/2 + lastNote.duration << endl;
          if(lastNote.x/TICKS_TO_SEC + main.getWidth() <= main.getWidth()/2 - lastNote.duration) {
            run = false;
            end = true;
          }
          else{
            input.updateTempo(tempo);
          }
        }
      }
    }
    switch (main.eventHandler(event)){
      case 1: // program closing
        state = false;
        break;
      case 2: // play/pause (spacebar)
        run = !run;
        noteShift = true;
        break;
      case 3: // right arrow
        input.shift(shiftX);
        break;
      case 4: // left arrow
        input.shift(-shiftX);
        break;
    }
    main.update();
  }
  
  main.terminate();
  return 0;
}
