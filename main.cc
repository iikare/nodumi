#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include "misc.h"
#include "window.h"
#include "note.h"
#include "dpd/osdialog/osdialog.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::ifstream;
using std::min;

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
  
  // program exit control
  bool state = true;

  // menu constants
  const static int menuHeight = 20;
  const static int areaTop = 20;
  
  // note shift controls
  int x, y, width = 0;
  double widthModifier = 2;
  int shiftTime = 0;
  double shiftX = 200;

  // color and cursor/note collision  
  colorRGB lineColor(233, 0, 22); 
  colorRGB noteColorOn1(0, 100, 255);
  colorRGB noteColorOff1(0, 0, 255);
  colorRGB noteColorOn2(233, 50, 50);
  colorRGB noteColorOff2(120, 20, 20);
  colorRGB noteColorOn = noteColorOn1;
  colorRGB noteColorOff = noteColorOff1;

  bool colorByPart = true;
  bool noteOn = false;

  bool mouseVisible = false;
  bool mouseOnNote = true;
  int lastMouseX = 0;
  int lastMouseY = 0;
  
  // determine where to draw rightclick menu
  int clickNoteX = 0; 
  int clickNoteY = 0;
  int clickNoteWidth = 0;
  int clickNoteHeight = 0;

  int rightClickMenuX = 0;
  int rightClickMenuY = 0;
  int rightClickMenuWidth = 0;
  int rightClickMenuHeight = 0;
  
  // play state controls
  bool run = false;
  bool oneTimeFlag = true;
  bool end = false;
  bool drawLine = true;
  
  // note info
  note* notes = input.getNotes();

  note renderNote = notes[0];
  note& firstNote = notes[0];
  note& lastNote = notes[input.getNoteCount()-1];
  shiftTime = firstNote.tempo;

  // event controller
  SDL_Event event;

  // color picker
  osdialog_color color = {255, 0, 255, 255};
  int res = 0;
  
  // menu control
  bool fileClicked = true;
  int fileSubMenuWidth = 100;
  int fileSubMenuHeight = 200;

  /*
   *  TODO:
   *    add user-customizable line color
   *    add menu bar on top
   *    add file picker
   *    add color picker for parts
   *    add color by parts                        DONE
   *    add color by tonic
   *    add config file parsing
   *    add note mouse detection
   *    add note outlines
   *    add ctrl left/right to scale faster       DONE
   *    add home/end functionality                DONE
   *    up/down arrow control horizontal scale    DONE 
   *    left/right arrow able to move             DONE  
   *    scale notes to window size                DONE (test with note value 0)
   */
  
  // debug track info 
  for (int i = 0; i < input.getNoteCount(); i++) {
    cerr << "note: " << i << "on track: " << notes[i].track << endl;
  }

  while (state){

    // render menu
    for (int x = 0; x <= main.getWidth(); x++) {
        for (int y = 0; y <= menuHeight; y++) {
          main.setPixelRGB(x, y, 255, 255, 255);
        }
    }
    
    // for menu : in progress  
    main.renderTextToTexture(4, 4, "file", 24);

    if (fileClicked) {
      for (int x = 0; x < fileSubMenuWidth; x++) {
        for (int y = menuHeight; y < fileSubMenuHeight; y++) {
          main.setPixelRGB(x, y, 255, 255, 255);
        }
      }
    }
   

    // clear false end flags
    if (end && lastNote.x + lastNote.duration > 0) {
      end = false;
    }

    if (!end || oneTimeFlag) {
      if (run || oneTimeFlag) {
        cerr << "--------------------------------" << endl; 
       
        // ensure rerender flag is unset
        oneTimeFlag = false;

        // render notes
        for (int i = 0; i < input.getNoteCount(); i++) {
          // get current note
          renderNote = notes[i];
          mouseOnNote = false;
          
          // calculate note coordinates 
          x = main.getWidth()/2 + renderNote.x;
          y = (main.getHeight() - round((main.getHeight() - areaTop) * static_cast<double>(renderNote.y - MIN_NOTE_IDX + 3)/(NOTE_RANGE + 3)));
          width = renderNote.duration;
          

          // perform note / cursor collision detection
          if (mouseVisible && hoverOnNote(main.getMouseX(), main.getMouseY(), x, y, width, renderNote.height)) {
           //cout << "note " << i << " overlaps" << endl;
           //cout << "x min, x max, x actual" << x << ", " << x + width<< ", " << main.getMouseX() << endl;
           //cout << "y min, y max, y actual" << y << ", " << y + renderNote.height<< ", " << main.getMouseY() << endl;
           mouseOnNote = true;

           // set the note location variables to be used on right click
           clickNoteX = x;
           clickNoteY = y;
           clickNoteWidth = width;
           clickNoteHeight = renderNote.height;
          } 
          //cerr << "render note y is " << renderNote.y << " while calc y is " << y << endl;
          
          // only render note if it's visible 
          if (x < main.getWidth() && x > - width) {
            
            // check if note is currently playing
            if (x <= main.getWidth()/2 && x >= main.getWidth()/2 - width) {
              noteOn = true;
            }
            else {
              noteOn = false;
            }

            // rendering of the note itself 
            for (int j = 0; j < width; j++) {
              for (int k = 0; k < renderNote.height; k++) {
                
                // set color based on note track 
                if (colorByPart) {
                  switch (renderNote.track) {
                    case 0:
                      noteColorOn = noteColorOn1;
                      noteColorOff = noteColorOff1;
                      break;
                    case 1:
                      noteColorOn = noteColorOn2;
                      noteColorOff = noteColorOff2;
                      break;
                  }

                  // render note specially if cursor is on it
                  if (mouseOnNote) {
                    main.setPixelRGB(x + j, y + k, noteColorOn2.r, noteColorOn2.g, noteColorOn2.b);
                  }
                  else if (noteOn) {
                    main.setPixelRGB(x + j, y + k, noteColorOn.r, noteColorOn.g, noteColorOn.b);
                  }
                  else {
                    main.setPixelRGB(x + j, y + k, noteColorOff.r, noteColorOff.g, noteColorOff.b);
                  }
                }
                else {
                  // implement color by tonic
                }
              }
            }
          }
        }
        // only update position if running (oneTimeFlag redraws buffer)
        if (run) {
          // end of file was reached
          if(lastNote.x + lastNote.duration<= 0) {
            run = false;
            end = true;
          }
          else {
            // shift normally as per tempo, or until end, whichever comes first
            if (lastNote.x + lastNote.duration > 0 && (shiftTime * input.getTimeScale())/TIME_MODIFIER < lastNote.x + lastNote.duration) {
              input.shiftTime(shiftTime * input.getTimeScale());
            }
            else if (lastNote.x + lastNote.duration > 0) {
              input.shiftTime((lastNote.x + lastNote.duration) * TIME_MODIFIER);
            }
          }
        }
      }
      // now line will always render regardless of play state
      if (drawLine) {
        for (int y = areaTop; y < main.getHeight(); y++) {
          main.setPixelRGB(main.getWidth()/2, y, lineColor.r, lineColor.g, lineColor.b);
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
        }
        break;
      case 3: // left arrow
        //res = osdialog_color_picker(&color, 0);
        //cerr << "test: {" << static_cast<int>(color.r) << ", " << static_cast<int>(color.g) << ", " << static_cast<int>(color.b) << "}"<< endl;

        //cerr << "run " << run  << " end " << end << endl;
        //cerr << "firstNote.x " << firstNote.x << endl;
        if (firstNote.x >= 0) {
          break;
        }
        
        oneTimeFlag = true;
        if (end) {
          end = false;
        }
        // case1: can shift entire specified width
        if (firstNote.x < 0 && firstNote.x + shiftX * input.getTimeScale() < 0) {
          input.shiftX(shiftX * input.getTimeScale());
        }
        // case2: can only shift to start
        else if (firstNote.x < 0 && firstNote.x + shiftX * input.getTimeScale() >= 0){
          input.shiftX(-firstNote.x);
        }
        break;
      case 4: // right arrow
        //cerr << "run " << run  << " end " << end << endl;
        //cerr << "firstNote.x " << firstNote.x << endl;
        if (end || lastNote.x + lastNote.duration <= 0) {
          break;
        }

        oneTimeFlag = true;
        
        // case1: can shift entire specified width
        if (lastNote.x > 0 && lastNote.x - shiftX * input.getTimeScale() > 0) {
          input.shiftX(-shiftX * input.getTimeScale());
        }
        // case2: can only shift to end
        else if (lastNote.x > 0 && lastNote.x - shiftX * input.getTimeScale() <= 0) {
          input.shiftX(-(lastNote.x + lastNote.duration));
        }
        break;
      case 5: // up arrow or scroll up
        oneTimeFlag = true;
        if (input.getTimeScale() < 64) {
          input.scaleTime(widthModifier);
        }
        break;
      case 6: //down arrow or scroll down
        oneTimeFlag = true;
        if (input.getTimeScale() > static_cast<double>(1)/4096) {
          input.scaleTime(1/widthModifier);
        }
        break;
      case 7: // home
        oneTimeFlag = true;
        run = false;
        end = false;
        input.shiftX(-firstNote.x);
        break;
      case 8: //end
        oneTimeFlag = true;
        run = false;
        end = true;
        input.shiftX(-(lastNote.x + lastNote.duration));
        break;
      case 9: // ctrl + left
        if (firstNote.x >= 0) {
          break;
        }
        
        oneTimeFlag = true;
        if (end) {
          end = false;
        }
        // case1: can shift entire specified width
        if (firstNote.x < 0 && firstNote.x + shiftX * CTRL_MODIFIER * input.getTimeScale() < 0) {
          input.shiftX(shiftX * CTRL_MODIFIER * input.getTimeScale());
        }
        // case2: can only shift to start
        else if (firstNote.x < 0 && firstNote.x + shiftX * CTRL_MODIFIER * input.getTimeScale() >= 0){
          input.shiftX(-firstNote.x);
        }
        break;
      case 10: // ctrl + right
        if (end || lastNote.x + lastNote.duration <= 0) {
          break;
        }

        oneTimeFlag = true;
        
        // case1: can shift entire specified width
        if (lastNote.x > 0 && lastNote.x - shiftX * CTRL_MODIFIER * input.getTimeScale() > 0) {
          input.shiftX(-shiftX * CTRL_MODIFIER * input.getTimeScale());
        }
        // case2: can only shift to end
        else if (lastNote.x > 0 && lastNote.x - shiftX * CTRL_MODIFIER * input.getTimeScale() <= 0) {
          cerr << " nonstandard shiftX: " << -(lastNote.x + lastNote.duration) * input.getTimeScale() << endl; 
          input.shiftX(-(lastNote.x + lastNote.duration));
        }
        break;
      case 11: // right click
        if (hoverOnNote(main.getMouseX(), main.getMouseY(), clickNoteX, clickNoteY, clickNoteWidth, clickNoteHeight)) {
            cerr  << "right clicked on note!" << endl;
        }

        break;
    }
    
    // update mouse position and check for changes
    lastMouseX = main.getMouseX();
    lastMouseY = main.getMouseY();
    mouseVisible = main.mouseVisible();
    if (lastMouseX != main.getMouseX() || lastMouseY != main.getMouseY()) {
      // redraw buffer if mouse has moved
      oneTimeFlag = true;
    }


    main.update();
    
    if (run || oneTimeFlag) {
      main.clearBuffer();
      //cerr << "--------------------------------" << endl; 
    }
    else if (!end || lastNote.x + lastNote.duration == 0) {
      end = true;
    }
  }
  
  main.terminate();
  return 0;
}
