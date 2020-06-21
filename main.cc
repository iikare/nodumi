#include <iostream>
#include <stdlib.h>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <string>
#include "misc.h"
#include "window.h"
#include "note.h"
#include "menu.h"
#include "color.h"
#include "colorgen.h"
#include "dpd/osdialog/osdialog.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ifstream;
using std::min;
using std::to_string;

int main(int argc, char* argv[]) {

  if (argc !=2) {
    cerr << "error: invalid usage - specify a input file!" << endl;
    exit(1);
  }
  
  string filename = argv[1];
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
  
  // new file controller
  bool newFile = false;
  char* filenameC;
  osdialog_filters* filetypes = osdialog_filters_parse("MIDI:mid;MKI:mki");

  // menu constants
  const static int areaTop = MAIN_MENU_HEIGHT;
  
  // note shift controls
  int x, y, width = 0;
  double widthModifier = 2;
  int shiftTime = 0;
  double shiftX = 200;

  // color and cursor/note collision  
  colorRGB lineColor(233, 0, 22); 
  colorRGB songTimeColor(233, 233, 233); 
  colorRGB lightBG(199,199,199); 

  colorRGB menuColor(222, 222, 222);
  colorRGB menuColorClick(155, 155, 155); 
  colorRGB menuLineColor(22, 22, 22);

  colorRGB noteColorOn1(0, 100, 255);
  colorRGB noteColorOff1(0, 0, 255);
  colorRGB noteColorOn2(233, 50, 50);
  colorRGB noteColorOff2(120, 20, 20);
  colorRGB noteColorOn = noteColorOn1;
  colorRGB noteColorOff = noteColorOff1;

  vector<colorRGB> noteColorA = {noteColorOn1, noteColorOn2};
  vector<colorRGB> noteColorB = {noteColorOff1, noteColorOff2};
  vector<colorRGB> noteColorC;
  vector<colorRGB> noteColorD;
  getColorScheme(input.getTrackCount(), noteColorC, noteColorD);

  bool colorByPart = true;
  bool noteOn = false;

  bool mouseOnNote = true;
  int lastMouseX = 0;
  int lastMouseY = 0;
  
  // right click menu constants
  // determine where to draw rightclick menu
  int clickNoteX = 0; 
  int clickNoteY = 0;
  int clickNoteWidth = 0;
  int clickNoteHeight = 0;
  int clickNoteTrack = 0;
  bool clickNoteOn = false;

  int rightClickX = 0;
  int rightClickY = 0;

  int colorSelectX = 0;
  int colorSelectY = 0;

  vector<string> rightClickContents = {"Change Part Color", "Set Tonic"};
  menu rightMenu(main.getSize(), rightClickContents, false, -100,-100);
  
  colorMenu colorSelect(0, 0, menuColor);

  bool colorChange = false;

  // play state controls
  bool run = false;
  bool oneTimeFlag = true;
  bool end = false;

  // view flags and related variables
  bool drawLine = true;
  bool songTime = false;
  bool invertColor = false;
  string songTimeText = "";
  
  // note info
  note* notes = input.getNotes();

  note renderNote = notes[0];
  note& firstNote = notes[0];
  note& lastNote = notes[input.getNoteCount()-1];
  shiftTime = firstNote.tempo;

  // event controller
  SDL_Event event;

  // mainmenu controls
  vector<string> fileMenuContents = {"File", "Open File", "Save", "Save As", "Exit"};
  menu fileMenu(main.getSize(), fileMenuContents, true, 0, 0);
  
  vector<string> editMenuContents = {"Edit", "A", "B", "C", "D"};
  menu editMenu(main.getSize(), editMenuContents, true, EDIT_X, 0);
  
  vector<string> viewMenuContents = {"View", "Display Mode:", "Hide Now Line", "Invert Color Scheme",
                                     "Display Song Time", "Set Tonic"};
  menu viewMenu(main.getSize(), viewMenuContents, true, VIEW_X, 0);

  /*
   *  TODO:
   *    add line color blend mode
   *    add background selection
   *    add menu bar on top                       DONE
   *    add file picker                           DONE (fix memory leaks)
   *    add color picker for parts                DONE (set default size bigger)
   *    add color by parts                        DONE
   *    add color by tonic
   *    add color generation algorithm
   *    add config file parsing
   *    add save file ability
   *    add note mouse detection                  DONE
   *    add note outlines/shadow
   *    add bg image support
   *    add ctrl left/right to scale faster       DONE
   *    add home/end functionality                DONE
   *    up/down arrow control horizontal scale    DONE 
   *    left/right arrow able to move             DONE  
   *    scale notes to window size                DONE (test with note value 0)
   */
  
  // debug track info 
  for (int i = 0; i < input.getNoteCount(); i++) {
    //cerr << "note " << i << " is on track " << notes[i].track << endl;
  }

  while (state){
    // load new file
    if (newFile) {
      newFile = false;

      input.load(filename);

      notes = input.getNotes();

      renderNote = notes[0];
      shiftTime = firstNote.tempo;

      x = 0;
      y = 0;
      width = 0;

      run = false;
      end = false;
      fileMenu.render = false;
      editMenu.render = false;
      viewMenu.render = false;
      rightMenu.render = false;
    }

    // clear false end flags
    if (end && lastNote.x + lastNote.duration > 0) {
      end = false;
    }

    // force rerender if menu is on
    if (rightMenu.render || fileMenu.render) {
      oneTimeFlag = true;
    }

    // update note references
    lastNote = notes[input.getNoteCount()-1];
    firstNote = notes[0];




    // begin render logic
    if (!end || oneTimeFlag) {
      if (run || oneTimeFlag) {
        // paint background before anything else
        if (invertColor) {
          for (int i = 0; i < main.getWidth(); i++) {
            for (int j = 0; j < main.getHeight(); j++) {
              main.setPixelRGB(i, j, lightBG);
            }
          }
        }
       
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
          if (main.cursorVisible && !colorSelect.render &&
              hoverOnBox(main.getMouseXY(), x, y, width, renderNote.height)) {
            mouseOnNote = true;

            // set the note location variables to be used on right click
            clickNoteX = x;
            clickNoteY = y;
            clickNoteWidth = width;
            clickNoteHeight = renderNote.height;
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
              for (int k = 0; k < renderNote.height; k++) {
                
                // set color based on note track 
                if (colorByPart) {
                  noteColorOff = noteColorA[renderNote.track];
                  noteColorOn = noteColorB[renderNote.track];

                  // render note by play status
                  if (noteOn) {
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
    }
    
    // everything after this point will render on top of notes

    // now line will always render regardless of play state
    if (drawLine) {
      for (int y = areaTop; y < main.getHeight(); y++) {
        main.setPixelRGB(main.getWidth()/2, y, lineColor.r, lineColor.g, lineColor.b);
      }
    }

    if (songTime) {
      songTimeText = getSongPercent(firstNote.x, input.getLastTick(), end);

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
      for (int x = 0; x < fileMenu.getWidth() ; x++) {
        for (int y = 0; y < fileMenu.getHeight(); y++) {
          if (!hoverOnBox(x, y, fileMenu.getX(), fileMenu.getY(),
              fileMenu.getWidth(), fileMenu.getItemY(1))) {
            if((y - fileMenu.getY()) % ITEM_HEIGHT != 0) { 
              main.setPixelRGB(x, y, menuColor);
            }
            else {
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
      for (int x = EDIT_X; x < EDIT_X + editMenu.getWidth() ; x++) {
        for (int y = 0; y < editMenu.getHeight(); y++) {
          if (!hoverOnBox(x, y, editMenu.getX(), editMenu.getY(),
              editMenu.getWidth(), editMenu.getItemY(1))) {
            if((y - editMenu.getY()) % ITEM_HEIGHT != 0) { 
              main.setPixelRGB(x, y, menuColor);
            }
            else {
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
      for (int x = VIEW_X; x < VIEW_X + viewMenu.getWidth() ; x++) {
        for (int y = 0; y < viewMenu.getHeight(); y++) {
          if (!hoverOnBox(x, y, viewMenu.getX(), viewMenu.getY(),
              viewMenu.getWidth(), viewMenu.getItemY(1))) {
            if((y - viewMenu.getY()) % ITEM_HEIGHT != 0) { 
              main.setPixelRGB(x, y, menuColor);
            }
            else {
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

    // draw the note right click menu
    if (rightMenu.render) {
      for (int x = rightMenu.getX(); x < rightMenu.getX() + rightMenu.getWidth(); x++) {
        for (int y = rightMenu.getY(); y < rightMenu.getY() + rightMenu.getHeight(); y++) {
          if((y - rightMenu.getY()) % ITEM_HEIGHT != 0 || y == rightMenu.getY()) { 
            main.setPixelRGB(x, y, menuColor);
          }
          else {
            main.setPixelRGB(x, y, menuLineColor);
          }
        }
      }
      for (int i = 0; i < rightMenu.getSize(); i++) {
        main.renderText(rightMenu.getItemX(i), rightMenu.getItemY(i), rightMenu.getContent(i));
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
        if(clickNoteOn) {
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
            vratio = 1-static_cast<double>(y - colorSelect.getSquareY()) / colorSelect.getSquareSize();
            
            
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
        else if (lastNote.x + lastNote.duration > 0 &&
                 lastNote.x - shiftX * input.getTimeScale() <= 0) {
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
          input.shiftX(-(lastNote.x + lastNote.duration));
        }
        break;
      case 11: // left click

        // note rightclick menu should only be active until left click
        
        rightMenu.findActiveElement(main.getMouseXY());
        fileMenu.findActiveElement(main.getMouseXY());
        editMenu.findActiveElement(main.getMouseXY());
        viewMenu.findActiveElement(main.getMouseXY());
        
        // ensure color selector is not selected
        if (!hoverOnBox(main.getMouseXY(), colorSelect.getBoundingBox())) {
          colorSelect.render = false;
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
            case 2: // save
              cerr << "info: function not implemented" << endl;
              break;
            case 3: // save as
              cerr << "info: function not implemented" << endl;
              break;
            case 4: // exit
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
              break;
          }
        }

        //handle view menu actions
        if (viewMenu.render || !viewMenu.getActiveElement()) {
          switch(viewMenu.getActiveElement()) {
            case -1: // clicked outside menu bounds
              viewMenu.render = false;
              break;
            case 0: // click on view menu again
              if (hoverOnBox(main.getMouseXY(), viewMenu.getX(), viewMenu.getY(),
                             VIEW_MENU_WIDTH, ITEM_HEIGHT)){
                fileMenu.render = false;
                editMenu.render = false;
                viewMenu.render = !viewMenu.render;
              }
              break;
            case 1: //display mode
              cerr << "info: function not implemented" << endl;
              break;
            case 2: // now line
              drawLine = !drawLine;
              if(!drawLine){
                viewMenu.setContent("Display Now Line", 2);
              }
              else {
                viewMenu.setContent("Hide Now Line", 2);
              }
              break;
            case 3: // invert color scheme
              invertColor = !invertColor;
              break;
            case 4: // display song time 
              songTime = !songTime;
              if(!songTime){
                viewMenu.setContent("Display Song Time", 4);
              }
              else {
                viewMenu.setContent("Hide Song Time", 4);
              }
              break;
            case 5: //
              cerr << "info: function not implemented" << endl;
              break;
            case 6: // 
              cerr << "info: function not implemented" << endl;
              break;
          }
        }

        //handle right menu actions
        if (colorSelect.render | rightMenu.render || !rightMenu.getActiveElement()) {
          switch (rightMenu.getActiveElement()) {
            case -1: // clicked outside menu bounds
              rightMenu.render = false;
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
            case 0: // change part color
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
              break;
            case 1: // set tonic
              colorSelect.render = false;
              cerr << "info: function not implemented" << endl;
              break;
            case 2: //
              cerr << "info: function not implemented" << endl;
              break;
            case 3: // 
              cerr << "info: function not implemented" << endl;
              break;
          }
        }
        oneTimeFlag = true;
        break;
      case 12: // right click
        // deactivate both rightmenu and color menu
        colorSelect.render = false;
        rightMenu.render = false;

        if (hoverOnBox(main.getMouseXY(), clickNoteX, clickNoteY, clickNoteWidth, clickNoteHeight)) {
          cerr  << "right clicked on note!" << endl;
          
          // stop rendering if clicked again
          colorSelect.render = false;

          // find coordinate to draw right click menu
          getMenuLocation(main.getWidth(), main.getHeight(), main.getMouseX(), main.getMouseY(),
                          rightClickX, rightClickY, rightMenu.getWidth(), rightMenu.getHeight());
          
          // pass that coordinate to the menu class
          rightMenu.setXY(rightClickX, rightClickY);

          if (clickNoteOn) {
            colorSelect.setColor(noteColorB[clickNoteTrack]);
          }
          else { 
            colorSelect.setColor(noteColorA[clickNoteTrack]);
          }

          cerr << "the menu for this note will take the space {" 
               << rightMenu.getX() << ", " << rightMenu.getY() << "} to {"
               << rightMenu.getX() + rightMenu.getWidth() << ", " 
               << rightMenu.getY() + rightMenu.getHeight() << "}" << endl;

          rightMenu.render = true;
        }
        else { 
          cerr << "right clicked on a non-note!" << endl;
        }
        break;
      case 13: // leftclick button up
        colorSelect.squareClick = false;
        colorSelect.circleClick = false;
        oneTimeFlag = true;
        break;
    }
    
    // update mouse position and check for changes
    lastMouseX = main.getMouseX();
    lastMouseY = main.getMouseY();
    main.updateCursor();
    if (main.cursorVisible && (lastMouseX != main.getMouseX() || lastMouseY != main.getMouseY())) {
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
  
  osdialog_filters_free(filetypes); 
  main.terminate();
  return 0;
}
