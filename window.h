#ifndef SCREEN_H
#define SCREEN_H

#include <iostream>
#include <string>
#include <deque>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "note.h"
#include "window.h"
#include "color.h"

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
    void updateCursor();

    int getWidth();
    int getHeight();
    int getMouseX();
    int getMouseY();

    void renderText(int x, int y, string text, colorRGB col = {0, 0, 0});
     
    void setPixelRGB(int x, int y, Uint8 r, Uint8 g, Uint8 b);
    void setPixelRGB(int x, int y, colorRGB col);
    void setPixelHSV(int x, int y, colorHSV col);
    void setPixelHSV(int x, int y, double h, double s, double v);
    Uint8* getPixelRGB(int x, int y);
    
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
    
    deque<int> messageX;
    deque<int> messageY;
    deque<string> messageText;
    deque<colorRGB> messageCol;

    SDL_Texture* tTexture;
    SDL_Surface* tSurface;
    SDL_Rect clip;
    int clipX;
    int clipY;

    Uint32* buffer;
    
    TTF_Font* menuFont;
    colorRGB menuColor;
    int fontSize;

    int mouseX;
    int mouseY;
};

#endif
