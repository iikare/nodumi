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

  const int menuHeight = 20;
  const int areaTop = 20;
  
  int x, y, width = 0;
  double widthModifier = 1;
  int shiftTime = 0;
  int shiftX = 200 * widthModifier;
  
  colorRGB lineColor(233, 0, 22); 
  colorRGB noteColorOn(0, 100, 255);
  colorRGB noteColorOff(0, 0, 255);
  colorRGB colorByNote(255, 255, 255);
  bool colorByPart = true;
  bool noteOn = false;
  
  bool run = false;
  bool oneTimeFlag = true;
  bool end = false;
  bool drawLine = true;
  bool noteShift = false;
  
  note* notes = input.getNotes();

  note& renderNote = notes[0];
  note& firstNote = notes[0];
  note& lastNote = notes[input.getNoteCount()-1];
  shiftTime = firstNote.tempo;

  SDL_Event event;

  /*
   *  TODO:
   *    add user-customizable line color
   *    add menu bar on top
   *    add file picker
   *    add color picker for parts
   *    add color by parts
   *    add config file parsing
   *    up/down arrow control horizontal scale    DONE (add upper zoom limit)
   *    left/right arrow able to move             DONE (add right bound restriction) 
   *    scale notes by window size                DONE (test with note value 0)
   */
  
  bool fileClicked = true;
  int fileSubMenuWidth = 100;
  int fileSubMenuHeight = 200;
  
  for (int i = 0; i < input.getNoteCount(); i++) {
    cerr << "note: " << i << "vs. note tick: " << notes[i].x << endl;
    if (i > 0 && notes[i].x < notes[i-1].x) {
      cerr << "warn: misordered note at position " << i << ": this note at tick " << notes[i].x << " is less than last note at tick " << notes[i-1].x << endl;
    } 
  }
  while (state){
    // render menu
    for (int x = 0; x <= main.getWidth(); x++) {
        for (int y = 0; y <= menuHeight; y++) {
          main.setPixelRGB(x, y, 255, 255, 255);
        }
    }
  
    main.renderTextToTexture(4, 4, "file", 24);

    if (fileClicked) {
      for (int x = 0; x < fileSubMenuWidth; x++) {
        for (int y = menuHeight; y < fileSubMenuHeight; y++) {
          main.setPixelRGB(x, y, 255, 255, 255);
        }
      }
    }

    if (!end) {

      // now line will always render regardless of play state
      if (drawLine) {
        for (int y = areaTop; y < main.getHeight(); y++) {
          main.setPixelRGB(main.getWidth()/2, y, lineColor.r, lineColor.g, lineColor.b);
        }
      }
      if (run || oneTimeFlag) {
        oneTimeFlag = false;

        // render notes
        for (int i = 0; i < input.getNoteCount(); i++) {
          // get current note
          renderNote = notes[i];
         
          x = main.getWidth()/2 + renderNote.x;
          y = (main.getHeight() - round((main.getHeight() - areaTop) * static_cast<double>(renderNote.y - MIN_NOTE_IDX + 3)/(NOTE_RANGE + 3)));
          width = renderNote.duration;
          
          //cerr << "render note y is " << renderNote.y << " while calc y is " << y << endl;
          
          if (x < main.getWidth() && x > - width) {

            if (x <= main.getWidth()/2 && x >= main.getWidth()/2 - width) {
              noteOn = true;
            }
            else {
              noteOn = false;
            }

            // rendering of the note itself 
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
        cout << lastNote.x + lastNote.duration << "is last note x" << endl;
        cout << input.getNoteCount() << endl;
        if (noteShift) {
          //cerr << lastNote.x/TICKS_TO_SEC + main.getWidth()/2 << " vs. " << main.getWidth()/2 - lastNote.duration << endl;
          if(lastNote.x + lastNote.duration<= 0) {
            run = false;
            end = true;
          }
          else {
            input.shiftTime(shiftTime);
          }
        }
      }
    }

    switch (main.eventHandler(event)){
      case 1: // program closing
        state = false;
        break;
      case 2: // play/pause (spacebar)
        if(!end) {
          run = !run;
          noteShift = true;
        }
        break;
      case 3: // left arrow 
        oneTimeFlag = true;
        if (firstNote.x < 0 && firstNote.x + shiftX < 0) {
          input.shiftX(shiftX);
        }
        else if (firstNote.x < 0 && firstNote.x + shiftX >= 0){ 
          input.shiftX(-firstNote.x);
        }
        break;
      case 4: // right arrow
        oneTimeFlag = true;
        input.shiftX(-shiftX);
        break;
      case 5: // up arrow or scroll up
        oneTimeFlag = true;
        if (widthModifier > 0.25) { 
          widthModifier -= 0.25;
        }
        break;
      case 6: //down arrow or scroll down
        oneTimeFlag = true;
        widthModifier += 0.25;
        break;
    }
    main.update();

    if (run || oneTimeFlag) {
      main.clearBuffer();
    }
  }
  
  main.terminate();
  return 0;
}
