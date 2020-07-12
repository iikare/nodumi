#ifndef WINDOW_H
#define WINDOW_H

#include <iostream>
#include <string>
#include <deque>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include "note.h"
#include "misc.h"
#include "color.h"
#include "box.h"

using std::string;
using std::deque;

class window {
  public:
    const static int WIDTH = WIN_WIDTH;
    const static int HEIGHT = WIN_HEIGHT;
    bool cursorVisible;
     
    window(string title);
    bool init();
    unsigned char eventHandler(SDL_Event &event);
    bool pointVisible(int x, int y);
    bool noteVisible(note n);
    bool cursorChange() { return (lastMouseX != mouseX || lastMouseY != mouseY); }
    void updateCursor();

    void saveBuffer();
    void loadBuffer();

    int getWidth();
    int getHeight();
    point getSize() { windowXY.x = windowX; windowXY.y = windowY; return windowXY; }
    int getMouseX();
    int getMouseY();
    point getMouseXY() { mouseXY.x = mouseX; mouseXY.y = mouseY; return mouseXY; }

    void renderText(int x, int y, string text, colorRGB col = {0, 0, 0});
    void fillBG(colorRGB col);

    void drawLine(int x0, int y0, int x1, int y1, colorRGB col);
     
    void setPixelRGB(const int& x, const int& y, const uint8_t& r, const uint8_t& g, const uint8_t& b);
    void setPixelRGB(const int& x, const int& y, const colorRGB& col);
    void setPixelRGBA(int x, int y, const colorRGB& col, unsigned char alpha);
    void setPixelRGBO(int x, int y);
    void setPixelHSV(int x, int y, colorHSV col);
    void setPixelHSV(int x, int y, double h, double s, double v);

    colorRGB getPixelRGB(int x, int y);
    
    void update();
    void clearBuffer();
    void terminate();
  private:
    void renderTextToTexture(int x, int y, string text, colorRGB col);
    
    string title;
    SDL_Window* windowA;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    int windowX;
    int windowY;
    point windowXY;
    
    deque<int>* messageX;
    deque<int>* messageY;
    deque<string>* messageText;
    deque<colorRGB>* messageCol;

    SDL_Texture* tTexture;
    SDL_Surface* tSurface;
    SDL_Rect clip;
    int clipX;
    int clipY;

    uint32_t* buffer;
    uint32_t* buffer2;
    
    TTF_Font* menuFont;
    int fontSize;
  
    int lastMouseX;
    int lastMouseY;
    int mouseX;
    int mouseY;
    point mouseXY;
};

#endif
