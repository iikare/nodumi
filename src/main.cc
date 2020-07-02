#include <iostream>
#include <stdlib.h>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <string>
#include <cstring>
#include <future>
#include <thread>
#include "box.h"
#include "misc.h"
#include "window.h"
#include "file.h"
#include "note.h"
#include "menu.h"
#include "color.h"
#include "colorgen.h"
#include "input.h"
#include "image.h"
#include "../dpd/osdialog/osdialog.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ifstream;
using std::min;
using std::max;
using std::to_string;
using std::memcpy;
using std::thread;

int main(int argc, char* argv[]) {

  if (argc !=2) {
    cerr << "error: invalid usage - specify a input file!" << endl;
    exit(1);
  }
  
  string filename = argv[1];
  transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
  string ext = filename.substr(filename.size() - 3);

  ifstream filecheck;
  filecheck.open(filename);
  if (!filecheck || (ext != "mid" && ext != "mki")) {
    cerr << "error: invalid file: " << filename << "!" << endl;
    exit(1);
  }
  filecheck.close();

  window main = window(static_cast<string>("mviewer ") +static_cast<string>(VERSION));

  if (!main.init()) {
    cerr << "error: failed to initialize window" << endl;
    exit(1);
  }

  // get variables for MKI loader
  mfile* input = new mfile;

  vector<colorRGB> noteColorA; //off
  vector<colorRGB> noteColorB; //on

  colorRGB darkBG;

  bool drawLine, songTime, invertColor;
  bool fromMKI = false;

  int displayMode, colorMode = 0;
  
  if (ext == "mid") {
    input->load(filename);
  }
  else {
    fromMKI = true;
    loadFileMKI(filename, input, noteColorA, noteColorB, darkBG, displayMode, colorMode, drawLine, songTime, invertColor); 
  }
  
  cerr << "info: initializing render logic" << endl;

  /*
   * * * * * * * 
   * VARIABLES *
   * * * * * * *
   */ 
  
  // program exit control
  bool state = true;
  bool showFPS = false;

  // new file controller
  bool newFile = false;
  char* filenameC;
  osdialog_filters* filetypes = osdialog_filters_parse("midi:mid;mki:mki");
  osdialog_filters* savetypes = osdialog_filters_parse("mki:mki");
  osdialog_filters* imagetypes = osdialog_filters_parse("png:png");

  // midi input controller
  MidiInput* userInput = new MidiInput;
  bool livePlay = false;

  // menu constants
  const static int areaTop = MAIN_MENU_HEIGHT;
  const static int noteHeight = round(WIN_HEIGHT/NOTE_RANGE);
  
  // note shift controls
  int x, y, width, deltaX, deltaY, noteLimit = 0;
  double widthModifier = 1.25;
  double shiftTime = 0;
  double shiftX = 200;

  // color and cursor/note collision  
  colorRGB lineColor(233, 0, 22); 
  colorRGB songTimeColor(233, 233, 233); 
  colorRGB lightBG(199,199,199); 

  colorRGB menuColor(222, 222, 222);
  colorRGB menuColorClick(155, 155, 155); 
  colorRGB menuLineColor(22, 22, 22);

  vector<colorRGB> noteColorC; //off
  vector<colorRGB> noteColorD; //on

  vector<colorRGB> noteColorE; //off
  vector<colorRGB> noteColorF; //on

  // color scheme for velocity based coloring
  getColorScheme(128, noteColorE, noteColorF);

  // color scheme for tonic
  getColorScheme(12, noteColorC, noteColorD);
  int tonic = 0;

  bool noteOn = false;

  bool mouseOnNote = true;

  
  // right click menu constants
  // determine where to draw rightclick menu
  int clickNoteX = 0; 
  int clickNoteY = 0;
  int clickNoteWidth = 0;
  int clickNoteHeight = 0;
  int clickNoteTrack = 0;
  int clickNoteTonic = 0;
  int clickNoteNumber = 0;
  bool clickNoteOn = false;

  int rightClickX = 0;
  int rightClickY = 0;

  int colorSelectX = 0;
  int colorSelectY = 0;

  vector<string> rightClickContents = {"Info", "Change Part Color", "Set Tonic"};
  menu rightMenu(main.getSize(), rightClickContents, false, -100,-100);
  
  vector<string> scaleMenuContents = {"Reset Scale", "x1.25", "x1.1", "x1.025", "x0.975", "x0.9", "x0.75"};
  menu scaleMenu(main.getSize(), scaleMenuContents, false, -100,-100);
  
  vector<string> moveMenuContents = {"Reset Position", "Center Image"};
  menu moveMenu(main.getSize(), moveMenuContents, false, -100,-100);
  
  vector<string> flipMenuContents = {"Horizontal", "Vertical"};
  menu flipMenu(main.getSize(), flipMenuContents, false, -100,-100);

  
  colorMenu colorSelect(0, 0, menuColor);

  
  bool colorChange = false;
  bool rightImage = false;

  // play state controls
  bool run = false;
  bool oneTimeFlag = true;
  bool end = false;

  // these are saved in MKI format and need to be conditionally updated
  if (!fromMKI) {
    getColorScheme(input->getTrackCount(), noteColorA, noteColorB);
    darkBG.setRGB(12,12,12); 
    drawLine = true;
    songTime = false;
    invertColor = false;
    displayMode = 1;
    colorMode = 1;
  }
  
  // song time text
  string songTimeText = "";
  int songMode = 1;
  
  // note info
  note* notes = input->getNotes();
  note* oNotes = new note[input->getNoteCount()];
  memcpy(oNotes, notes, input->getNoteCount() * sizeof(note));

  note renderNote = notes[0];
  note& firstNote = notes[0];
  note& lastNote = notes[input->getNoteCount()-1];
  shiftTime = input->findCurrentTempo();

  // image controller/
  BGImage* bgI = new BGImage;
  string bgPath = "";
  bool bgRender = false;
  bool bgMove = false;
  bool loadBG = false;
  bool updateBG = true;

  // event controller
  SDL_Event event;

  // mainmenu controls
  vector<string> fileMenuContents = {"File", "Open File", "Open Image", "Save", "Save As", "Exit"};
  menu fileMenu(main.getSize(), fileMenuContents, true, 0, 0);
  
  vector<string> editMenuContents = {"Edit", "A", "B", "C", "Preferences"};
  menu editMenu(main.getSize(), editMenuContents, true, EDIT_X, 0);
  
  vector<string> viewMenuContents = {"View", "Display Mode:", "Hide Now Line", "Invert Color Scheme",
                                     "Display Song Time", "Color By:", "Show Background", "Swap Colors", "Show FPS"};
  menu viewMenu(main.getSize(), viewMenuContents, true, VIEW_X, 0);

  vector<string> displayMenuContents = {"Standard", "Line", "Ball"};
  menu displayMenu(main.getSize(), displayMenuContents, false, VIEW_X + viewMenu.getWidth(), viewMenu.getItemY(1));

  vector<string> songMenuContents = {"Relative", "Absolute"};
  menu songMenu(main.getSize(), songMenuContents, false, VIEW_X + viewMenu.getWidth(), viewMenu.getItemY(4));

  vector<string> colorMenuContents = {"Part", "Velocity", "Tonic"};
  menu colorMenu(main.getSize(), colorMenuContents, false, VIEW_X + viewMenu.getWidth(), viewMenu.getItemY(5));

  vector<string> midiMenuContents = {"MIDI", "Input", "Output", "Enable Live Mode"};
  menu midiMenu(main.getSize(), midiMenuContents, true, MIDI_X, 0);

  vector<string> inputMenuContents = {""};
  menu inputMenu(main.getSize(), inputMenuContents, false, MIDI_X + midiMenu.getWidth(), midiMenu.getItemY(1));
  
  /*
   *  TODO:
   *    add alpha blend mode
   *    add background color selection            DONE
   *    add menu bar on top                       DONE
   *    add file picker                           DONE (fix memory leaks)
   *    add color picker for parts                DONE (set default size bigger)
   *    add color by parts                        DONE
   *    add color by tonic                        DONE
   *    add color generation algorithm            DONE
   *    add save file ability                     DONE
   *    add save existing file                    DONE 
   *    add colorMode to MKI
   *    add image to MKI (maybe)                
   *    add tempo detection                       DONE
   *    fix note timing detection                 DONE
   *    add note mouse detection                  DONE
   *    add note outlines/shadow
   *    add image support                         DONE (make image scalable) (right menu options scale/flip/remove)
   *    add color by velocity                     DONE
   *    add more display modes                    2 done so far
   *    support live MIDI input                   DONE
   *    add ctrl left/right to scale faster       DONE
   *    add home/end functionality                DONE
   *    up/down arrow control horizontal scale    DONE 
   *    left/right arrow able to move             DONE  
   *    scale notes to window size                DONE
   */
  
  // debug track info 
  //for (int i = 0; i < input->getNoteCount(); i++) {
   // cerr << "note " << i << " is on track " << notes[i].track << endl;
  //}
  

  // debug bg
  bgI->loadPNG("tests/large.png");
  bgRender = true;

  //fps
  uint32_t tick = SDL_GetTicks();
  uint32_t fps = 0;
  uint32_t fCount = 0;

  while (state){
    
    // refresh note count
    if (livePlay) {
      userInput->update();
    }

    // load new file
    if (newFile) {
      newFile = false;

      string ext = filename.substr(filename.size() - 3);
      if (ext == "mid") {
        fromMKI = false;
        input->load(filename);
      }
      else {
        fromMKI = true;
        loadFileMKI(filename, input, noteColorA, noteColorB, darkBG, displayMode, colorMode, drawLine, songTime, invertColor); 
      }     
      
      if (!fromMKI) {
        getColorScheme(input->getTrackCount(), noteColorA, noteColorB);
        darkBG.setRGB(12,12,12); 
        drawLine = true;
        songTime = false;
        invertColor = false;
        displayMode = 1;
        colorMode = 1;
      }

      delete[] oNotes; 
      notes = input->getNotes();
      oNotes = new note[input->getNoteCount()];
      memcpy(oNotes, notes, input->getNoteCount() * sizeof(note));

      renderNote = notes[0];
      shiftTime = input->findCurrentTempo();

      x = 0;
      y = 0;
      width = 0;

      run = false;
      end = false;
      livePlay = false;
      fileMenu.render = false;
    }

    if (loadBG) {
      updateBG = true;
      loadBG = false;
      bgI->loadPNG(bgPath);
      bgRender = true;
      livePlay = false;
      fileMenu.render = false;
    }

    // update bg position if needed
    if (bgMove && bgRender && bgI->getWidth()) {
      run = false;
      bgI->setXY(main.getMouseXY());
      oneTimeFlag = true;
      updateBG = true;
    }
    
    // clear false end flags
    if (end && lastNote.x + lastNote.duration > 0) {
      end = false;
    }


    // live play
    if (livePlay) {
      notes = userInput->getNotes();
      noteLimit = userInput->getNoteCount();
    //  input = userInput->noteStream;
      oneTimeFlag = true;
    }
    else {
      noteLimit = input->getNoteCount();
    }


    // update note references
    lastNote = notes[input->getNoteCount()-1];
    firstNote = notes[0];
    shiftTime = input->findCurrentTempo();

    // begin render logic
    if (!end || livePlay || oneTimeFlag) {
      if (run || livePlay || oneTimeFlag) {
        // ensure rerender flag is unset
        oneTimeFlag = false;
       

        // this will rerender the background and save it to the secondary buffer
        // the background must be rerendered if:
        //    BG color changes
        //    invert light/dark is pressed 
        //    image is scaled / flipped
        //    image is removed
        if (updateBG) { 

          updateBG = false;

          if (invertColor) {
            main.fillBG(lightBG);
          }
          else {
            main.fillBG(darkBG);
          }
          if (bgRender) {
            // render bg
            for (int i = max(0, bgI->getX()); i < min(main.getWidth(), bgI->getX() + bgI->getWidth()); i++) {
              for (int j = max(0, areaTop + bgI->getY()); j < min(main.getHeight(), areaTop + bgI->getY() + bgI->getHeight()); j++) {
                // to be optimized
                if (main.pointVisible(i, j)) { 
                  main.setPixelRGB(i, j, bgI->getPixelRGB(i - bgI->getX(), j - bgI->getY() - areaTop));
                }
              }
            }
          }
          main.saveBuffer();
        }
        else {
          main.loadBuffer();
        }
        
        // render notes
        for (int i = 0; i < noteLimit; i++) {
          // get current note
          renderNote = notes[i];
          mouseOnNote = false;
          
          // calculate note coordinates 
          x = main.getWidth()/2 + renderNote.x;
          y = (main.getHeight() - round((main.getHeight() - areaTop) * static_cast<double>(renderNote.y - MIN_NOTE_IDX + 3)/(NOTE_RANGE + 3)));
          
          // prevent notes from disappearing at high scaling
          width = renderNote.duration < 1 ? 1 : renderNote.duration * 120.0/input->findCurrentTempo();

          switch (displayMode) {
            case 1: // standard mode

              // perform note / cursor collision detection
              if (main.cursorVisible && !colorSelect.render &&
                  hoverOnBox(main.getMouseXY(), x, y, width, noteHeight)) {
                mouseOnNote = true;

                // set the note location variables to be used on right click
                clickNoteX = x;
                clickNoteY = y;
                clickNoteWidth = width;
                clickNoteHeight = noteHeight;
                clickNoteNumber = (renderNote.y - MIN_NOTE_IDX);
                clickNoteTonic = (renderNote.y - MIN_NOTE_IDX) % 12;
                clickNoteTrack = renderNote.track;
                // check if note is currently playing
                if (x <= main.getWidth()/2 && x >= main.getWidth()/2 - width) {
                  clickNoteOn = true;
                }
                else {
                  clickNoteOn = false;
                }
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
                
                // render with the other color if hovered over
                if (mouseOnNote) {
                  noteOn = !noteOn;
                }

                // rendering of the note itself 
                for (int j = 0; j < width; j++) {
                  for (int k = 0; k < noteHeight; k++) {
                    switch (colorMode) {
                      case 1: // part
                        if (noteOn) {
                          main.setPixelRGB(x + j, y + k, noteColorB[renderNote.track]);
                        }
                        else {
                          main.setPixelRGB(x + j, y + k, noteColorA[renderNote.track]);
                        }
                        break;
                      case 2: // velocity
                        if (noteOn) {
                          main.setPixelRGB(x + j, y + k, noteColorF[renderNote.velocity % 127]);
                        }
                        else {
                          main.setPixelRGB(x + j, y + k, noteColorE[renderNote.velocity % 127]);
                        }
                        break;
                      case 3: // tonic
                        if (noteOn) { 
                          main.setPixelRGB(x + j, y + k, noteColorD[(renderNote.y - MIN_NOTE_IDX + tonic) % 12]);
                        }
                        else {
                          main.setPixelRGB(x + j, y + k, noteColorC[(renderNote.y - MIN_NOTE_IDX + tonic) % 12]);
                        }
                        break;
                    }
                  }
                }
              break;
            case 2: //  line mode
              // calculate XY offset from last note
              if (i + 1 >= input->getNoteCount()) {
                deltaX = 0;
                deltaY = 0;

                // stop rendering line at last note
                break;
              }
              else {
                // get average y value of chord
                int offset = 1;   
                int avgY = 0;
                int avgNextY = 0;

                avgY = renderNote.y - MIN_NOTE_IDX;
                
                // get average of current y
                while (renderNote.x == notes[i + offset].x) {
                  if (renderNote.track == notes[i + offset].track) {
                    avgY += notes[i + offset].y - MIN_NOTE_IDX;
                  }
                  offset++;
                }
  
                // preserve original offset
                int sOff = offset;
                avgNextY = notes[i + offset + 1].y - MIN_NOTE_IDX;

                // get average of next Y
                while (notes[i + offset].x == notes[i + offset + 1].x) {
                  if (notes[i + offset].track == notes[i + offset + 1].track) {
                    avgNextY += notes[i + offset + 1].y - MIN_NOTE_IDX;
                  }
                  offset++;
                }


                avgY = avgY/sOff + MIN_NOTE_IDX;
                avgNextY = avgNextY/offset + MIN_NOTE_IDX;

                int nextX = main.getWidth()/2 + notes[i + offset].x;
                int nextY = (main.getHeight() - round((main.getHeight() - areaTop) * static_cast<double>(avgNextY - MIN_NOTE_IDX + 3)/(NOTE_RANGE + 3)));
                x = main.getWidth()/2 + renderNote.x;
                y = (main.getHeight() - round((main.getHeight() - areaTop) * static_cast<double>(avgY - MIN_NOTE_IDX + 3)/(NOTE_RANGE + 3)));
                deltaX = nextX - x;
                deltaY = nextY - y;

                // check if note is currently playing
                if (x <= main.getWidth()/2 && x >= main.getWidth()/2 - width) {
                  noteOn = true;
                }
                else {
                  noteOn = false;
                }
                
                // render only if the line is visible
                if (x < main.getWidth() && nextX > 0) {
                  cout << "X, Y" << deltaX << ", " << deltaY << endl;
                  for (int j = 0; j < deltaX; j++) {
                    for (int k = min(nextY, y); k < max(nextY, y) + 4; k++) {
                      int distLine = getDistance(x + j, y + j * static_cast<double>(deltaY)/deltaX, x + j, k);
                      if (distLine < 2) {
                        switch (colorMode) {
                          case 1: // part
                            if (noteOn) {
                              main.setPixelRGB(x + j, y + k, noteColorB[renderNote.track]);
                            }
                            else {
                              main.setPixelRGB(x + j, y + k, noteColorA[renderNote.track]);
                            }
                            break;
                          case 2: // velocity
                            if (noteOn) {
                              main.setPixelRGB(x + j, y + k, noteColorF[renderNote.velocity % 127]);
                            }
                            else {
                              main.setPixelRGB(x + j, y + k, noteColorE[renderNote.velocity % 127]);
                            }
                            break;
                          case 3: // tonic
                            if (noteOn) { 
                              main.setPixelRGB(x + j, y + k, noteColorD[(renderNote.y - MIN_NOTE_IDX + tonic) % 12]);
                            }
                            else {
                              main.setPixelRGB(x + j, y + k, noteColorC[(renderNote.y - MIN_NOTE_IDX + tonic) % 12]);
                            }
                            break;
                        }
                      }
                    }
                  }
                }
              }
              break;
            case 3: // balls
              int radius = 3 * log(width) < 1 ? 1 : 3 * log(width);
              
              // perform note / cursor collision detection
              if (main.cursorVisible && !colorSelect.render &&
                  getDistance(main.getMouseXY(), x, y) < radius) {
                mouseOnNote = true;

                // set the note location variables to be used on right click
                clickNoteX = x;
                clickNoteY = y;
                clickNoteWidth = radius;
                clickNoteHeight = radius;
                clickNoteNumber = (renderNote.y - MIN_NOTE_IDX);
                clickNoteTonic = (renderNote.y - MIN_NOTE_IDX) % 12;
                clickNoteTrack = renderNote.track;
                // check if note is currently playing
                if (x <= main.getWidth()/2 && x >= main.getWidth()/2 - width) {
                  clickNoteOn = true;
                }
                else {
                  clickNoteOn = false;
                }
              } 
          
              if (x < main.getWidth() && x > - width) {
                if (x <= main.getWidth()/2 && x >= main.getWidth()/2 - width) {
                  noteOn = true;
                }
                else {
                  noteOn = false;
                }

                // render with the other color if hovered over              
                if (mouseOnNote) {
                  noteOn = !noteOn;
                }    
                
                for (int j = x - radius; j < x + radius; j++) {
                  for (int k = y - radius; k < y + radius; k++) {
                    if (getDistance(x, y, j, k) <= radius) {
                      switch (colorMode) {
                        case 1: // part
                          if (noteOn) {
                            main.setPixelRGB(j, k, noteColorB[renderNote.track]);
                          }
                          else {
                            main.setPixelRGB(j, k, noteColorA[renderNote.track]);
                          }
                          break;
                        case 2: // velocity
                          if (noteOn) {
                            main.setPixelRGB(j, k, noteColorF[renderNote.velocity % 127]);
                          }
                          else {
                            main.setPixelRGB(j, k, noteColorE[renderNote.velocity % 127]);
                          }
                          break;
                        case 3: // tonic
                          if (noteOn) { 
                            main.setPixelRGB(j, k, noteColorD[(renderNote.y - MIN_NOTE_IDX + tonic) % 12]);
                          }
                          else {
                            main.setPixelRGB(j, k, noteColorC[(renderNote.y - MIN_NOTE_IDX + tonic) % 12]);
                          }
                          break;
                      }
                    }
                  }
                }
              }           
              break;
            }
          }
        }
        // the shift is constant if in live mode
        if (livePlay) {
          //cerr << "a, b " << shiftTime << userInput->noteStream->getTimeScale() << endl;
          //userInput->noteStream->shiftTime(shiftTime * userInput->noteStream->getTimeScale());
        }
        // only update position if running (oneTimeFlag redraws buffer)
        else if (run) {
          // end of file was reached
          if(lastNote.x + lastNote.duration<= 0) {
            run = false;
            end = true;
          }
          else {
          //  cerr << "shiftTime: " << shiftTime << endl;
            // shift normally as per tempo, or until end, whichever comes first
            if (lastNote.x + lastNote.duration > 0 && (shiftTime * input->getTimeScale())/TIME_MODIFIER < lastNote.x + lastNote.duration) {
              input->shiftTime(shiftTime * input->getTimeScale());
            }
            else if (lastNote.x + lastNote.duration > 0) {
              input->shiftTime((lastNote.x + lastNote.duration) * TIME_MODIFIER);
            }
          }
        }
      }
    }
    
    // everything after this point will render on top of notes

    // now line will always render regardless of play state
    if (drawLine) {
      for (int y = areaTop; y < main.getHeight(); y++) {
        main.setPixelRGB(main.getWidth()/2, y, lineColor.r, lineColor.g, lineColor.b);
      }
    }

    // only render song time if no main menus are rendered to prevent text overlap
    if (songTime && !fileMenu.render && !editMenu.render && !viewMenu.render && !midiMenu.render) {
      if (songMode == 1) {
        songTimeText = getSongPercent(static_cast<long double>(firstNote.x) * 1000000 * input->getTimeScale(), static_cast<long double>(1000000) * input->getLastTick(), end);
      }
      else if (songMode == 2) {
        songTimeText = getSongTime(firstNote.x, input->findCurrentNote(), input->getLastTime());
      }

      // choose text color based on background
      if (!invertColor) {
        main.renderText(0, MAIN_MENU_HEIGHT, songTimeText, songTimeColor);
      }
      else {
        main.renderText(0, MAIN_MENU_HEIGHT, songTimeText);
      }
    } 

    
    // render main menu bar
    for (int x = 0; x <= main.getWidth(); x++) {
        for (int y = 0; y <= MAIN_MENU_HEIGHT; y++) {
          main.setPixelRGB(x, y, menuColor);
        }
    }
    
    // render file menu
    if (fileMenu.render) {
      fileMenu.findActiveElement(main.getMouseXY()); 
      for (int x = 0; x < fileMenu.getWidth() ; x++) {
        for (int y = 0; y < fileMenu.getHeight(); y++) {
          if (!hoverOnBox(x, y, fileMenu.getX(), fileMenu.getY(), fileMenu.getWidth(), fileMenu.getItemY(1))) {
            main.setPixelRGB(x, y, menuColor);

            if (hoverOnBox(x, y, fileMenu.getBox(fileMenu.getActiveElement()))) {
              main.setPixelRGB(x, y, menuColorClick);
            }
            
            if ((y - fileMenu.getY()) % ITEM_HEIGHT == 0) {
              main.setPixelRGB(x, y, menuLineColor);
            }
          }
          else {
            if (x < FILE_MENU_WIDTH) { 
              main.setPixelRGB(x, y, menuColorClick);
            }
          }
        }
      }
      for (int i = 0; i < fileMenu.getSize(); i++) {
        main.renderText(fileMenu.getItemX(i), fileMenu.getItemY(i), fileMenu.getContent(i));
      }
    }
    else {
      // only render the "File"
      main.renderText(fileMenu.getItemX(0), fileMenu.getItemY(0), fileMenu.getContent(0));
    }

    // render edit menu
    if (editMenu.render) {
      editMenu.findActiveElement(main.getMouseXY());
      for (int x = EDIT_X; x < EDIT_X + editMenu.getWidth() ; x++) {
        for (int y = 0; y < editMenu.getHeight(); y++) {
          if (!hoverOnBox(x, y, editMenu.getX(), editMenu.getY(), editMenu.getWidth(), editMenu.getItemY(1))) {
            main.setPixelRGB(x, y, menuColor);
            
            if (hoverOnBox(x, y, editMenu.getBox(editMenu.getActiveElement()))) {
              main.setPixelRGB(x, y, menuColorClick);
            }
            
            if ((y - editMenu.getY()) % ITEM_HEIGHT == 0) {
              main.setPixelRGB(x, y, menuLineColor);
            }
          }
          else {
            if (x < EDIT_X + EDIT_MENU_WIDTH) { 
              main.setPixelRGB(x, y, menuColorClick);
            }
          }
        }
      }
      for (int i = 0; i < editMenu.getSize(); i++) {
        main.renderText(editMenu.getItemX(i), editMenu.getItemY(i), editMenu.getContent(i));
      }
    }
    else {
      // only render the "Edit"
      main.renderText(editMenu.getItemX(0), editMenu.getItemY(0), editMenu.getContent(0));
    }

    // render view menu
    if (viewMenu.render) {
      viewMenu.findActiveElement(main.getMouseXY());
      for (int x = VIEW_X; x < VIEW_X + viewMenu.getWidth() ; x++) {
        for (int y = 0; y < viewMenu.getHeight(); y++) {
          if (!hoverOnBox(x, y, viewMenu.getX(), viewMenu.getY(), viewMenu.getWidth(), viewMenu.getItemY(1))) {
            main.setPixelRGB(x, y, menuColor);
            
            if (displayMenu.render && hoverOnBox(x, y, viewMenu.getBox(1))) {
              main.setPixelRGB(x, y, menuColorClick);
            }
            else if (songMenu.render && hoverOnBox(x, y, viewMenu.getBox(4))) {
              main.setPixelRGB(x, y, menuColorClick);
            }
            else if (colorMenu.render && hoverOnBox(x, y, viewMenu.getBox(5))) {
              main.setPixelRGB(x, y, menuColorClick);
            }

            if (hoverOnBox(x, y, viewMenu.getBox(viewMenu.getActiveElement()))) {
              main.setPixelRGB(x, y, menuColorClick);
            }
            
            if ((y - viewMenu.getY()) % ITEM_HEIGHT == 0) {
              main.setPixelRGB(x, y, menuLineColor);
            }
          }
          else {
            if (x < VIEW_X + VIEW_MENU_WIDTH) { 
              main.setPixelRGB(x, y, menuColorClick);
            }
          }
        }
      }
      for (int i = 0; i < viewMenu.getSize(); i++) {
        main.renderText(viewMenu.getItemX(i), viewMenu.getItemY(i), viewMenu.getContent(i));
      }
    }
    else {
      // only render the "View"
      main.renderText(viewMenu.getItemX(0), viewMenu.getItemY(0), viewMenu.getContent(0));
    }

    // render display menu
    if (displayMenu.render && viewMenu.render) {
      displayMenu.findActiveElement(main.getMouseXY());
      for (int x = displayMenu.getX(); x < displayMenu.getX() + displayMenu.getWidth() ; x++) {
        for (int y = displayMenu.getY(); y < displayMenu.getY() + displayMenu.getHeight(); y++) {
          main.setPixelRGB(x, y, menuColor);
          
          if (hoverOnBox(x, y, displayMenu.getBox(displayMenu.getActiveElement()))) {
            main.setPixelRGB(x, y, menuColorClick);
          }
          
          if (((y - displayMenu.getY()) % ITEM_HEIGHT == 0 || x == displayMenu.getX()) && y != displayMenu.getY()) {
            main.setPixelRGB(x, y, menuLineColor);
          }
        }
      }
      for (int i = 0; i < displayMenu.getSize(); i++) {
        main.renderText(displayMenu.getItemX(i), displayMenu.getItemY(i), displayMenu.getContent(i));
      }
    }

    // render song menu
    if (songMenu.render && viewMenu.render) {
      songMenu.findActiveElement(main.getMouseXY());
      for (int x = songMenu.getX(); x < songMenu.getX() + songMenu.getWidth() ; x++) {
        for (int y = songMenu.getY(); y < songMenu.getY() + songMenu.getHeight(); y++) {
            main.setPixelRGB(x, y, menuColor);
            
            if (hoverOnBox(x, y, songMenu.getBox(songMenu.getActiveElement()))) {
              main.setPixelRGB(x, y, menuColorClick);
            }
            
          if (((y - songMenu.getY()) % ITEM_HEIGHT == 0 && y != songMenu.getY()) || x == songMenu.getX()) {
            main.setPixelRGB(x, y, menuLineColor);
          }
        }
      }
      for (int i = 0; i < songMenu.getSize(); i++) {
        main.renderText(songMenu.getItemX(i), songMenu.getItemY(i), songMenu.getContent(i));
      }
    }

    // render color menu
    if (colorMenu.render && viewMenu.render) {
      colorMenu.findActiveElement(main.getMouseXY());
      for (int x = colorMenu.getX(); x < colorMenu.getX() + colorMenu.getWidth() ; x++) {
        for (int y = colorMenu.getY(); y < colorMenu.getY() + colorMenu.getHeight(); y++) {
            main.setPixelRGB(x, y, menuColor);
            
            if (hoverOnBox(x, y, colorMenu.getBox(colorMenu.getActiveElement()))) {
              main.setPixelRGB(x, y, menuColorClick);
            }
            
          if (((y - colorMenu.getY()) % ITEM_HEIGHT == 0 && y != colorMenu.getY()) || x == colorMenu.getX()) {
            main.setPixelRGB(x, y, menuLineColor);
          }
        }
      }
      for (int i = 0; i < colorMenu.getSize(); i++) {
        main.renderText(colorMenu.getItemX(i), colorMenu.getItemY(i), colorMenu.getContent(i));
      }
    }

    // draw the note right click menu
    if (rightMenu.render) {
      rightMenu.findActiveElement(main.getMouseXY());
      for (int x = rightMenu.getX(); x < rightMenu.getX() + rightMenu.getWidth(); x++) {
        for (int y = rightMenu.getY(); y < rightMenu.getY() + rightMenu.getHeight(); y++) {
          main.setPixelRGB(x, y, menuColor);
          
          if (!colorSelect.clickBG && colorSelect.render && hoverOnBox(x, y, rightMenu.getBox(1))) {
            main.setPixelRGB(x, y, menuColorClick);
          }       
          
          if (hoverOnBox(x, y, rightMenu.getBox(rightMenu.getActiveElement()))) {
            main.setPixelRGB(x, y, menuColorClick);
          }
          
          if ((y - rightMenu.getY()) % ITEM_HEIGHT == 0 && y != rightMenu.getY()) {
            main.setPixelRGB(x, y, menuLineColor);
          }
        }
      }
      for (int i = 0; i < rightMenu.getSize(); i++) {
        main.renderText(rightMenu.getItemX(i), rightMenu.getItemY(i), rightMenu.getContent(i));
      }
    }
     
    // render scale menu
    if (scaleMenu.render && rightMenu.render) {
      scaleMenu.findActiveElement(main.getMouseXY());
      for (int x = scaleMenu.getX(); x < scaleMenu.getX() + scaleMenu.getWidth() ; x++) {
        for (int y = scaleMenu.getY(); y < scaleMenu.getY() + scaleMenu.getHeight(); y++) {
          main.setPixelRGB(x, y, menuColor);
          
          if (hoverOnBox(x, y, scaleMenu.getBox(scaleMenu.getActiveElement()))) {
            main.setPixelRGB(x, y, menuColorClick);
          }
          
          if (((y - scaleMenu.getY()) % ITEM_HEIGHT == 0 || x == scaleMenu.getX()) && y != scaleMenu.getY()) {
            main.setPixelRGB(x, y, menuLineColor);
          }
        }
      }
      for (int i = 0; i < scaleMenu.getSize(); i++) {
        main.renderText(scaleMenu.getItemX(i), scaleMenu.getItemY(i), scaleMenu.getContent(i));
      }
    }   

    // render move menu
    if (moveMenu.render && rightMenu.render) {
      moveMenu.findActiveElement(main.getMouseXY());
      for (int x = moveMenu.getX(); x < moveMenu.getX() + moveMenu.getWidth() ; x++) {
        for (int y = moveMenu.getY(); y < moveMenu.getY() + moveMenu.getHeight(); y++) {
          main.setPixelRGB(x, y, menuColor);
          
          if (hoverOnBox(x, y, moveMenu.getBox(moveMenu.getActiveElement()))) {
            main.setPixelRGB(x, y, menuColorClick);
          }
          
          if (((y - moveMenu.getY()) % ITEM_HEIGHT == 0 || x == moveMenu.getX()) && y != moveMenu.getY()) {
            main.setPixelRGB(x, y, menuLineColor);
          }
        }
      }
      for (int i = 0; i < moveMenu.getSize(); i++) {
        main.renderText(moveMenu.getItemX(i), moveMenu.getItemY(i), moveMenu.getContent(i));
      }
    }   

    // render flip menu
    if (flipMenu.render && rightMenu.render) {
      flipMenu.findActiveElement(main.getMouseXY());
      for (int x = flipMenu.getX(); x < flipMenu.getX() + flipMenu.getWidth() ; x++) {
        for (int y = flipMenu.getY(); y < flipMenu.getY() + flipMenu.getHeight(); y++) {
          main.setPixelRGB(x, y, menuColor);
          
          if (hoverOnBox(x, y, flipMenu.getBox(flipMenu.getActiveElement()))) {
            main.setPixelRGB(x, y, menuColorClick);
          }
          
          if (((y - flipMenu.getY()) % ITEM_HEIGHT == 0 || x == flipMenu.getX()) && y != flipMenu.getY()) {
            main.setPixelRGB(x, y, menuLineColor);
          }
        }
      }
      for (int i = 0; i < flipMenu.getSize(); i++) {
        main.renderText(flipMenu.getItemX(i), flipMenu.getItemY(i), flipMenu.getContent(i));
      }
    }

    if (colorSelect.render) {
      if(colorSelect.squareClick) {
        colorSelect.setSPointXY(main.getMouseXY());
        colorChange = true;
      }
      if(colorSelect.circleClick) {
        colorSelect.setCPointXY(main.getMouseXY());
        colorChange = true;
      }
      if (colorChange) {
        colorChange = false;
        oneTimeFlag = true;

        if (colorSelect.circleClick) {
          // adjust only hue
          colorSelect.findHSVFromSquare(true);
        }
        else {
          // adjust saturation/value
          colorSelect.findHSVFromSquare(false);
        }
        if (colorSelect.clickBG) {
          updateBG = true;
          darkBG = colorSelect.getColor();
        }
        else if(clickNoteOn) {
          noteColorB[clickNoteTrack] = colorSelect.getColor();
        }
        else {
          noteColorA[clickNoteTrack] = colorSelect.getColor();
        }
      }
        colorSelect.findAngleFromColor();
      //  cout << colorSelect.circleClick << endl;
        
     // cerr << colorSelect.getSPointX()<< ", " << colorSelect.getSPointY() << endl;
      for (int x = colorSelect.getX(); x < colorSelect.getX() + colorSelect.getWidth(); x++) {
        for (int y = colorSelect.getY(); y < colorSelect.getY() + colorSelect.getHeight(); y++) {
          
          double dist = getDistance(x, y, colorSelect.getCenterX(), colorSelect.getCenterY());
          double distSP = getDistance(x, y, colorSelect.getSPointX(), colorSelect.getSPointY());
          double distCP = getDistance(x, y, colorSelect.getCPointX(), colorSelect.getCPointY());

          colorRGB circleColor = getHueByAngle(x, y, colorSelect.getCenterX(),colorSelect.getCenterY());
          double sratio = 1;
          double vratio = 1;

         // cout << "testC"<< colorSelect.getSPointX() << ", " << colorSelect.getSPointY() << endl;
          //cout << "testP"<< colorSelect.getPointX() << ", " << colorSelect.getPointY() << endl;
          if (hoverOnBox(x, y, colorSelect.getSquareX(), colorSelect.getSquareY(),
                              colorSelect.getSquareSize())){
            
            // x increases saturation, y increases value
            sratio = static_cast<double>(x - colorSelect.getSquareX()) / colorSelect.getSquareSize();
            vratio = 1 - static_cast<double>(y - colorSelect.getSquareY()) / colorSelect.getSquareSize();
            
            
            main.setPixelHSV(x, y, colorSelect.getHue(), sratio, vratio * 255);

          }
          else{
            main.setPixelRGB(x, y, menuColor);
          }
          if (dist > colorSelect.getInner() && dist < colorSelect.getOuter()) {
            main.setPixelRGB(x, y, circleColor);
          }
          if (distCP < 4) {
            if (distCP < 3) {
              main.setPixelHSV(x, y, 0, 0, 144);
            }
            else {
              main.setPixelHSV(x, y, 0, 0, 0);
            }
          }
          if (distSP < 4) {
            //cout << colorSelect.getSPointX() << ", " << colorSelect.getSPointY() << endl;
            if (distSP < 3) {
              main.setPixelHSV(x, y, 0, 0, 144);
            }
            else {
              main.setPixelHSV(x, y, 0, 0, 0);
            }
          }
        }
      } 
    } 
  
    // render midi menu
    if (midiMenu.render) {
      midiMenu.findActiveElement(main.getMouseXY());
      for (int x = MIDI_X; x < MIDI_X + midiMenu.getWidth() ; x++) {
        for (int y = 0; y < midiMenu.getHeight(); y++) {
          if (!hoverOnBox(x, y, midiMenu.getX(), midiMenu.getY(),
              midiMenu.getWidth(), midiMenu.getItemY(1))) {
            main.setPixelRGB(x, y, menuColor);

            if (inputMenu.render && hoverOnBox(x, y, midiMenu.getBox(1))) {
              main.setPixelRGB(x, y, menuColorClick);
            }     

            if (hoverOnBox(x, y, midiMenu.getBox(midiMenu.getActiveElement()))) {
              main.setPixelRGB(x, y, menuColorClick);
            }
            
            if ((y - midiMenu.getY()) % ITEM_HEIGHT == 0) {
              main.setPixelRGB(x, y, menuLineColor);
            }
          }
          else {
            if (x < MIDI_X + MIDI_MENU_WIDTH) { 
              main.setPixelRGB(x, y, menuColorClick);
            }
          }
        }
      }
      for (int i = 0; i < midiMenu.getSize(); i++) {
        main.renderText(midiMenu.getItemX(i), midiMenu.getItemY(i), midiMenu.getContent(i));
      }
    }
    else {
      // only render the "MIDI"
      main.renderText(midiMenu.getItemX(0), midiMenu.getItemY(0), midiMenu.getContent(0));
    }

    // render input menu
    if (inputMenu.render && midiMenu.render) {
      inputMenu.findActiveElement(main.getMouseXY());
      for (int x = inputMenu.getX(); x < inputMenu.getX() + inputMenu.getWidth() ; x++) {
        for (int y = inputMenu.getY(); y < inputMenu.getY() + inputMenu.getHeight(); y++) {
          main.setPixelRGB(x, y, menuColor);
          
          if (hoverOnBox(x, y, inputMenu.getBox(inputMenu.getActiveElement()))) {
            main.setPixelRGB(x, y, menuColorClick);
          }
          
          if (((y - inputMenu.getY()) % ITEM_HEIGHT == 0 || x == inputMenu.getX()) && y != inputMenu.getY()) {
            main.setPixelRGB(x, y, menuLineColor);
          }
        }
      }
      for (int i = 0; i < inputMenu.getSize(); i++) {
        main.renderText(inputMenu.getItemX(i), inputMenu.getItemY(i), inputMenu.getContent(i));
      }
    }
 // cerr << firstNote.x << endl;
    // event handler
    switch (main.eventHandler(event)){
      case 1: // program closing
        state = false;
        break;
      case 2: // play/pause (spacebar)
        if(!end) {
          run = !run;
          rightMenu.render = false;
          colorSelect.render = false;
        }
        oneTimeFlag = true;
        break;
      case 3: // left arrow
        //res = osdialog_color_picker(&color, 0);
        //cerr << "test: {" << static_cast<int>(color.r) << ", "
        //     << static_cast<int>(color.g) << ", " << static_cast<int>(color.b) << "}"<< endl;

        //cerr << "run " << run  << " end " << end << endl;
        //cerr << "firstNote.x " << firstNote.x << endl;
        if (firstNote.x >= 0) {
          break;
        }
        
        oneTimeFlag = true;
        end = false;
        // case1: can shift entire specified width
        if (firstNote.x < 0 && firstNote.x + shiftX * input->getTimeScale() < 0) {
          input->shiftX(shiftX * input->getTimeScale());
        }
        // case2: can only shift to start
        else if (firstNote.x < 0 && firstNote.x + shiftX * input->getTimeScale() >= 0){
          input->shiftX(-firstNote.x);
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
        if (lastNote.x > 0 && lastNote.x - shiftX * input->getTimeScale() > 0) {
          input->shiftX(-shiftX * input->getTimeScale());
        }
        // case2: can only shift to end
        else if (lastNote.x + lastNote.duration > 0 &&
                 lastNote.x - shiftX * input->getTimeScale() <= 0) {
          input->shiftX(-(lastNote.x + lastNote.duration));
        }
        break;
      case 5: // up arrow or scroll up
        oneTimeFlag = true;
        if (input->getTimeScale() < 2 && !livePlay) {
          input->scaleTime(widthModifier);
        }
        else if (livePlay && userInput->noteStream->getTimeScale() < 2) {
          userInput->noteStream->scaleTime(widthModifier);
        }
        break;
      case 6: //down arrow or scroll down
        oneTimeFlag = true;
        if (input->getTimeScale() > static_cast<double>(1)/4096 && !livePlay) {
          input->scaleTime(1/widthModifier);
        }
        else if (livePlay && userInput->noteStream->getTimeScale() > static_cast<double>(1)/4096) {
          userInput->noteStream->scaleTime(1/widthModifier);
        }
        break;
      case 7: // home
        oneTimeFlag = true;
        run = false;
        end = false;
        input->shiftX(-firstNote.x);
        break;
      case 8: //end
        oneTimeFlag = true;
        run = false;
        end = true;
        input->shiftX(-(lastNote.x + lastNote.duration));
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
        if (firstNote.x < 0 && firstNote.x + shiftX * CTRL_MODIFIER * input->getTimeScale() < 0) {
          input->shiftX(shiftX * CTRL_MODIFIER * input->getTimeScale());
        }
        // case2: can only shift to start
        else if (firstNote.x < 0 && firstNote.x + shiftX * CTRL_MODIFIER * input->getTimeScale() >= 0){
          input->shiftX(-firstNote.x);
        }
        break;
      case 10: // ctrl + right
        if (end || lastNote.x + lastNote.duration <= 0) {
          break;
        }

        oneTimeFlag = true;
        
        // case1: can shift entire specified width
        if (lastNote.x > 0 && lastNote.x - shiftX * CTRL_MODIFIER * input->getTimeScale() > 0) {
          input->shiftX(-shiftX * CTRL_MODIFIER * input->getTimeScale());
        }
        // case2: can only shift to end
        else if (lastNote.x > 0 && lastNote.x - shiftX * CTRL_MODIFIER * input->getTimeScale() <= 0) {
          input->shiftX(-(lastNote.x + lastNote.duration));
        }
        break;
      case 11: // left click

        // note rightclick menu should only be active until left click
        
        rightMenu.findActiveElement(main.getMouseXY());
        scaleMenu.findActiveElement(main.getMouseXY());
        moveMenu.findActiveElement(main.getMouseXY());
        flipMenu.findActiveElement(main.getMouseXY());
        fileMenu.findActiveElement(main.getMouseXY());
        editMenu.findActiveElement(main.getMouseXY());
        viewMenu.findActiveElement(main.getMouseXY());
        displayMenu.findActiveElement(main.getMouseXY());
        songMenu.findActiveElement(main.getMouseXY());
        colorMenu.findActiveElement(main.getMouseXY());
        midiMenu.findActiveElement(main.getMouseXY());
        inputMenu.findActiveElement(main.getMouseXY());
        
        // ensure color selector is not selected
        if (!hoverOnBox(main.getMouseXY(), colorSelect.getBoundingBox())) {
          colorSelect.render = false;
        }       

        // ensure rightImage is corrected
        if (!bgRender) {
          rightImage = false;
        }

        //handle file menu actions
        if (fileMenu.render || !fileMenu.getActiveElement()) {
          switch (fileMenu.getActiveElement()) {
            case -1: //clicked outside menu bounds
              fileMenu.render = false;
              break;
            case 0: // click on file menu again
              if (hoverOnBox(main.getMouseXY(), fileMenu.getX(), fileMenu.getY(), FILE_MENU_WIDTH, ITEM_HEIGHT)){
                editMenu.render = false;
                viewMenu.render = false;
                midiMenu.render = false;
                rightMenu.render = false;
                fileMenu.render = !fileMenu.render;
              }
              break;
            case 1: // open file
		          filenameC = osdialog_file(OSDIALOG_OPEN, ".", nullptr, filetypes);
              
              if (filenameC != nullptr) {
                filename = static_cast<string>(filenameC);
                newFile = true; 
                oneTimeFlag = true;
              }
              break;
            case 2: // open image
		          filenameC = osdialog_file(OSDIALOG_OPEN, ".", nullptr, imagetypes);
              
              if (filenameC != nullptr) {
                bgPath = static_cast<string>(filenameC);
                loadBG = true; 
                oneTimeFlag = true;
              }
              break;
            case 3: // save
              if (fromMKI && !livePlay) {
                cerr << "info: saving MKI - " << filename << endl;
                saveFile(filename, input, noteColorA, noteColorB, darkBG, displayMode, colorMode, drawLine, songTime, invertColor);
              }
              oneTimeFlag = true;
              break;
            case 4: // save as
              if (!livePlay) {
                filenameC = osdialog_file(OSDIALOG_SAVE, ".", nullptr, savetypes);
                
                if (filenameC != nullptr) {
                  filename = static_cast<string>(filenameC);
                  cerr << "info: saving MKI - " << filename << endl;
                  saveFile(filename, input, noteColorA, noteColorB, darkBG, displayMode, colorMode, drawLine, songTime, invertColor);
                  oneTimeFlag = true;
                }
              }
              break;
            case 5: // exit
              if (fileMenu.render){
                state = false;
              }
              break;
          }
        }

        //handle edit menu actions
        if (editMenu.render || !editMenu.getActiveElement()) {
          switch (editMenu.getActiveElement()) {
            case -1: // clicked outside menu bounds
              editMenu.render = false;
              break;
            case 0: // click on edit menu again
              if (hoverOnBox(main.getMouseXY(), editMenu.getX(), editMenu.getY(),
                             EDIT_MENU_WIDTH, ITEM_HEIGHT)){
                fileMenu.render = false;
                viewMenu.render = false;
                midiMenu.render = false;
                rightMenu.render = false;
                editMenu.render = !editMenu.render;
              }
              break;
            case 1: // 
              cerr << "info: function not implemented" << endl;
              break;
            case 2: // 
              cerr << "info: function not implemented" << endl;
              break;
            case 3: //
              cerr << "info: function not implemented" << endl;
              break;
            case 4: // 
              cerr << "info: function not implemented" << endl;
          }
        }

        //handle view menu actions
        if (viewMenu.render || !viewMenu.getActiveElement()) {
          switch(viewMenu.getActiveElement()) {
            case -1: // clicked outside menu bounds
              if (viewMenu.getActiveElement() == -1 &&  displayMenu.getActiveElement() == -1) {
                displayMenu.render = false;
              } 
              if (viewMenu.getActiveElement() == -1 &&  songMenu.getActiveElement() == -1) {
                songMenu.render = false;
              } 
              if (viewMenu.getActiveElement() == -1 &&  colorMenu.getActiveElement() == -1) {
                colorMenu.render = false;
              } 
              if (!displayMenu.render && !songMenu.render && !colorMenu.render && viewMenu.getActiveElement() == -1) {
                viewMenu.render = false;
              }
              break;
            case 0: // click on view menu again
              if (hoverOnBox(main.getMouseXY(), viewMenu.getX(), viewMenu.getY(),
                             VIEW_MENU_WIDTH, ITEM_HEIGHT)){
                fileMenu.render = false;
                editMenu.render = false;
                midiMenu.render = false;
                rightMenu.render = false;
                viewMenu.render = !viewMenu.render;
                if (!songMode) {          
                  viewMenu.setContent("Display Song Time", 4);
                }
              }
              break;
            case 1: //display mode
              displayMenu.render = !displayMenu.render;
              songMenu.render = false;
              colorMenu.render = false;
              break;
            case 2: // now line
              drawLine = !drawLine;
              if(!drawLine){
                viewMenu.setContent("Display Now Line", 2);
              }
              else {
                viewMenu.setContent("Hide Now Line", 2);
              }
              displayMenu.render = false;
              songMenu.render = false;
              colorMenu.render = false;
              break;
            case 3: // invert color scheme
              invertColor = !invertColor;
              displayMenu.render = false;
              songMenu.render = false;
              colorMenu.render = false;
              break;
            case 4: // display song time
              if (songTime) {
                viewMenu.setContent("Display Song Time", 4);
                songTime = false;
                songMenu.render = false;
              }
              else {
                viewMenu.setContent("Hide Song Time" , 4);
                songMenu.render = true;
                songMode = 0;
              }

              // check for if no selection was mode
              displayMenu.render = false;
              colorMenu.render = false;
              break;
            case 5: //set color method
              colorMenu.render = !colorMenu.render;
              displayMenu.render = false;
              songMenu.render = false;
              break;
            case 6: // toggle bg image
              if (!bgI->getWidth()) { 
                cerr << "info: no image loaded" << endl;
                viewMenu.setContent("Show Background", 6);
              }
              else{
                updateBG = true;
                bgRender = !bgRender;
                if (!bgRender) {
                  viewMenu.setContent("Hide Background", 6);
                }
                else {
                  viewMenu.setContent("Show Background", 6);
                }
                displayMenu.render = false;
                songMenu.render = false;
                colorMenu.render = false;
              }
              break;
            case 7: // swap note colors
              swap(noteColorA, noteColorB);
              swap(noteColorC, noteColorD);
              swap(noteColorE, noteColorF);
              
              displayMenu.render = false;
              songMenu.render = false;
              colorMenu.render = false;
              break;
            case 8:
              showFPS = !showFPS;
              
              displayMenu.render = false;
              songMenu.render = false;
              colorMenu.render = false;
              break;
          }
        }
         
        //handle display menu actions
        if (displayMenu.render || !displayMenu.getActiveElement()) {
          switch (displayMenu.getActiveElement()) {
            case -1: // clicked outside menu bounds
              // handled inside parent menu
              break;
            case 0: // standard
              displayMode = 1;
              break;
            case 1: // line
              displayMode = 2; 
              break;
            case 2: // balls
              displayMode = 3;
              break;
            case 3: //
              cerr << "info: function not implemented" << endl;
              break;
            case 4: // 
              cerr << "info: function not implemented" << endl;
              break;
          }
        }  

        //handle song menu actions
        if (songMenu.render || !songMenu.getActiveElement()) {
          switch (songMenu.getActiveElement()) {
            case -1: // clicked outside menu bounds
              // handled inside parent menu
              break;
            case 0: // relative
              songTime = true;
              songMode = 1;
              break;
            case 1: // absolute
              songTime = true;
              songMode = 2; 
              break;
            case 2: // 
              cerr << "info: function not implemented" << endl;
              break;
          }
        }  

        //handle color menu actions
        if (colorMenu.render || !colorMenu.getActiveElement()) {
          switch (colorMenu.getActiveElement()) {
            case -1: // clicked outside menu bounds
              // handled inside parent menu
              break;
            case 0: // part
              colorMode = 1;
              break;
            case 1: // velocity
              colorMode = 2; 
              break;
            case 2: // tonic
              colorMode = 3;
              break;
            case 3: //
              cerr << "info: function not implemented" << endl;
              break;
            case 4: // 
              cerr << "info: function not implemented" << endl;
              break;
          }
        }
        
        //handle right menu actions
        if (colorSelect.render || rightMenu.render || !rightMenu.getActiveElement()) {
          switch (rightMenu.getActiveElement()) {
            case -1: // clicked outside menu bounds
              if (rightMenu.getActiveElement() == -1 &&  scaleMenu.getActiveElement() == -1) {
                scaleMenu.render = false;
              } 
              if (rightMenu.getActiveElement() == -1 &&  moveMenu.getActiveElement() == -1) {
                moveMenu.render = false;
              } 
              if (rightMenu.getActiveElement() == -1 &&  flipMenu.getActiveElement() == -1) {
                flipMenu.render = false;
              } 
              if (!scaleMenu.render && !flipMenu.render && !colorSelect.render && viewMenu.getActiveElement() == -1) {
                rightMenu.render = false;
              }
              if (!hoverOnBox(main.getMouseXY(), colorSelect.getBoundingBox())) {
                colorSelect.render = false;
              }
              else {
                if (getDistance(main.getMouseXY(), colorSelect.getCenterXY()) > colorSelect.getInner()) {
                  colorSelect.circleClick = true;
                }
                else {
                  colorSelect.squareClick = true;
                }
              }
              break;
            case 0: // note info (note)  change color (bg) scale image (bg image)
              if (colorSelect.clickBG) {
                if (colorSelect.render) {
                 // turn off the color picker if clicked again
                  colorSelect.render = false;
                  break;
                } 
                getColorSelectLocation(main.getWidth(), main.getHeight(), colorSelectX, colorSelectY,
                                       rightClickX, rightClickY, rightMenu.getWidth(),
                                       rightMenu.getHeight());
                colorSelect.setXY(colorSelectX, colorSelectY);
                colorSelect.render = true;
              }
              else if (rightImage) {
                scaleMenu.render = !scaleMenu.render;
                flipMenu.render = false;
                moveMenu.render = false;
              }
              break;
            case 1: // change part color (note) open move menu (bg image)
              if (!colorSelect.clickBG && !rightImage) {
                if (colorSelect.render) {
                 // turn off the color picker if clicked again
                  colorSelect.render = false;
                  break;
                } 
                getColorSelectLocation(main.getWidth(), main.getHeight(), colorSelectX, colorSelectY,
                                       rightClickX, rightClickY, rightMenu.getWidth(),
                                       rightMenu.getHeight());
                colorSelect.setXY(colorSelectX, colorSelectY);
                colorSelect.render = true;
              }
              else if (rightImage) {
                moveMenu.render = !moveMenu.render;
                flipMenu.render = false;
                scaleMenu.render = false;
              }
              break;
            case 2: // set tonic (note) flip menu (bg image)
              // only if render by tonic is selected
              if (!rightImage) {
                if (colorMode == 3) {
                  tonic = clickNoteTonic;
                } 
                colorSelect.render = false;
              }
              else {
                flipMenu.render = !flipMenu.render;
                moveMenu.render = false;
                scaleMenu.render = false;
              }
              break;
            case 3: // remove bg image
              updateBG = true;
              bgI->clear();

              bgRender = false;
              scaleMenu.render = false;
              flipMenu.render = false;
              moveMenu.render = false;

              break;
          }
        }

        //handle scale menu actions
        if (scaleMenu.render || !scaleMenu.getActiveElement()) {
          switch (scaleMenu.getActiveElement()) {
            case -1: // clicked outside menu bounds
              // handled inside parent menu
              break;
            case 0:
              updateBG = true;
              bgI->resetScale();
              break;
            case 1: // 1.25
              updateBG = true;
              bgI->scale(1.25);
              break;
            case 2: // 1.1
              updateBG = true;
              bgI->scale(1.1);
              break;
            case 3: // 1.025 
              updateBG = true;
              bgI->scale(1.025);
              break;
            case 4: // 0.975
              updateBG = true;
              bgI->scale(0.975);
              break;
            case 5: // 0.9
              updateBG = true;
              bgI->scale(0.9);
              break;
            case 6: // 0.75 
              updateBG = true;
              bgI->scale(0.75);
              break;
          }
        } 

        //handle move menu actions
        if (moveMenu.render || !moveMenu.getActiveElement()) {
          switch (moveMenu.getActiveElement()) {
            case -1: // clicked outside menu bounds
              // handled inside parent menu
              break;
            case 0: // reset position
              updateBG = true;
              bgI->resetPosition();
              break;
            case 1: // center image
              updateBG = true;
              bgI->centerImage(main.getWidth(), main.getHeight());
              break;
          }
        }  

        //handle flip menu actions
        if (flipMenu.render || !flipMenu.getActiveElement()) {
          switch (flipMenu.getActiveElement()) {
            case -1: // clicked outside menu bounds
              // handled inside parent menu
              break;
            case 0: // horizontal
              updateBG = true;
              bgI->flip(true);
              break;
            case 1: // vertical
              updateBG = true;
              bgI->flip(false);
              break;
          }
        }  

        //handle midi menu actions
        if (midiMenu.render || !midiMenu.getActiveElement()) {
          switch (midiMenu.getActiveElement()) {
            case -1: // clicked outside menu bounds
              if (midiMenu.getActiveElement() == -1 && inputMenu.getActiveElement() == -1) {
                midiMenu.render = false;
              }
              break;
            case 0: // click on midi menu again
              if (hoverOnBox(main.getMouseXY(), midiMenu.getX(), midiMenu.getY(),
                             MIDI_MENU_WIDTH, ITEM_HEIGHT)){
                fileMenu.render = false;
                editMenu.render = false;
                viewMenu.render = false;
                rightMenu.render = false;
                midiMenu.render = !midiMenu.render;
              }
              break;
            case 1: // input 
              inputMenu.update(userInput->getPorts());
              inputMenu.render = !inputMenu.render;
              break;
            case 2: // output
              inputMenu.render = false;
              cerr << "info: function not implemented" << endl;
              break;
            case 3: // enable/disable live play
              livePlay = !livePlay;

              if(!livePlay){
                midiMenu.setContent("Enable Live Mode", 3);
              }
              else {
                // set default input port
                userInput->openPort(1);

                // default to color by velocity
                colorMode = 3;

                midiMenu.setContent("Disable Live Mode", 3);
              }
              break;
            case 4: // 
              cerr << "info: function not implemented" << endl;
              break;
          }
        }

        //handle input menu actions
        if (inputMenu.render || !inputMenu.getActiveElement()) {
          switch (inputMenu.getActiveElement()) {
            case -1: // clicked outside menu bounds
              if (!midiMenu.render) {
                inputMenu.render = false;
              }
              break;
            default:
              if (inputMenu.render) {
                userInput->openPort(inputMenu.getActiveElement());
              }
              break;
          }
        }
        
        oneTimeFlag = true;
        break;
      case 12: // right click
        // deactivate both rightmenu and color menu
        colorSelect.render = false;
        rightMenu.render = false;
        rightImage = false;

        // turn off all other menus
        fileMenu.render = false;
        editMenu.render = false;
        viewMenu.render = false;
        displayMenu.render = false;
        songMenu.render = false;
        colorMenu.render = false;
        midiMenu.render = false;
        inputMenu.render = false;
        scaleMenu.render = false;
        moveMenu.render = false;
        flipMenu.render = false;

        // unselect BG
        colorSelect.clickBG = false;


        if ((hoverOnBox(main.getMouseXY(), clickNoteX, clickNoteY, clickNoteWidth, clickNoteHeight) && displayMode == 1) ||
            (getDistance(main.getMouseXY(), clickNoteX, clickNoteY) < clickNoteWidth && displayMode == 3)) {
          //cerr  << "right clicked on note!" << endl;

          rightMenu.update(rightClickContents);
          
          // stop rendering if clicked again
          colorSelect.render = false;

          // find coordinate to draw right click menu
          getMenuLocation(main.getWidth(), main.getHeight(), main.getMouseX(), main.getMouseY(),
                          rightClickX, rightClickY, rightMenu.getWidth(), rightMenu.getHeight());
          
          // pass that coordinate to the menu class
          rightMenu.setXY(rightClickX, rightClickY);

          // get info about selected note
          rightMenu.setContent(getNoteInfo(clickNoteTrack, clickNoteNumber), 0);

          if (clickNoteOn) {
            colorSelect.setColor(noteColorB[clickNoteTrack]);
          }
          else { 
            colorSelect.setColor(noteColorA[clickNoteTrack]);
          }

          /*cerr << "the menu for this note will take the space {" 
               << rightMenu.getX() << ", " << rightMenu.getY() << "} to {"
               << rightMenu.getX() + rightMenu.getWidth() << ", " 
               << rightMenu.getY() + rightMenu.getHeight() << "}" << endl;*/

          rightMenu.render = true;
        }
        else if (hoverOnBox(main.getMouseXY(), 0, areaTop, main.getWidth(), main.getHeight() - areaTop) && (!bgRender || !hoverOnBox(main.getMouseXY(), bgI->getBox()))) {
          // ensure BG is selected
          colorSelect.clickBG = true;

          // stop rendering if clicked again
          colorSelect.render = false;

          // generate Menu
          vector<string> BGRight = {"Change Color"};
          rightMenu.update(BGRight);

          // find coordinate to draw right click menu
          getMenuLocation(main.getWidth(), main.getHeight(), main.getMouseX(), main.getMouseY(),
                          rightClickX, rightClickY, rightMenu.getWidth(), rightMenu.getHeight());
          
          // pass that coordinate to the menu class
          rightMenu.setXY(rightClickX, rightClickY);
          
          colorSelect.setColor(darkBG);
          rightMenu.render = true;
          //cerr << "right clicked on the background!" << endl;
        }
        else if (bgRender && hoverOnBox(main.getMouseXY(), bgI->getBox())) {
          vector<string> IRight = {"Scale Image", "Move Image", "Flip Image", "Remove Image"};
          rightMenu.update(IRight);
 
          // find coordinate to draw right click menu
          getMenuLocation(main.getWidth(), main.getHeight(), main.getMouseX(), main.getMouseY(),
                          rightClickX, rightClickY, rightMenu.getWidth(), rightMenu.getHeight());
          
          // pass that coordinate to the menu class
          rightMenu.setXY(rightClickX, rightClickY);  

          // update position for submenus
          scaleMenu.setXY(rightMenu.getX() + rightMenu.getWidth(), rightMenu.getItemY(0));
          moveMenu.setXY(rightMenu.getX() + rightMenu.getWidth(), rightMenu.getItemY(1));
          flipMenu.setXY(rightMenu.getX() + rightMenu.getWidth(), rightMenu.getItemY(2));
          
          rightMenu.render = true;
          colorSelect.render = false;
          rightImage = true;

          //cerr << "right clicked on an image!" << endl;
        }
        else { 
          rightMenu.setContent("", 0);
          //cerr << "right clicked on a non-note!" << endl;
        }
        oneTimeFlag = true;
        break;
      case 13: // leftclick button up
        colorSelect.squareClick = false;
        colorSelect.circleClick = false;
        bgMove = false;
        bgI->clearXYOffset();
        oneTimeFlag = true;
        break;
      case 14: // background move
        // only move bg image if clicked on
        if (hoverOnBox(main.getMouseXY(), bgI->getBox())) {
          bgMove = true;
          bgI->setXYOffset(main.getMouseXY());
          oneTimeFlag = true;
        }
        break;
    }
    // update mouse position and check for changes
    main.updateCursor();
    if (main.cursorVisible && main.cursorChange()) {
      // redraw buffer if mouse has moved
      oneTimeFlag = true;
    }
    
    fCount++;
    if (SDL_GetTicks() > 1000 && tick < SDL_GetTicks() - 1000) {
      tick = SDL_GetTicks();
      fps = fCount;
      fCount = 0;
    }  
    if (showFPS) {
      main.renderText(main.getWidth() - 22, 0, to_string(fps));
    }
    main.update();
 
    if (run || livePlay || oneTimeFlag) {
      main.clearBuffer();
    }
    else if (!end || lastNote.x + lastNote.duration == 0) {
      end = true;
    }
  }
 
  delete bgI; 
  delete input;
  delete userInput;
  delete[] oNotes;  
  osdialog_filters_free(filetypes); 
  osdialog_filters_free(savetypes); 
  osdialog_filters_free(imagetypes); 
  main.terminate();
  return 0;
}
