#include <algorithm>
#include <SDL2/SDL_ttf.h>

#include "window.h"
#include "note.h"

using std::cerr;
using std::endl;
using std::swap;
using std::fill;

window::window(string title) : 
  windowA(nullptr), renderer(nullptr), texture(nullptr), 
  buffer(nullptr), menuFont(nullptr), menuColor(0, 0, 0), fontSize(0) {
  this->title = title;
}

bool window::init() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    cerr << "Failed to initialize SDL" << endl;
    return false;
  } 

  if (TTF_Init() < 0) {
    cerr << "warn: TTF initialization failed" << endl;
  }
      
  windowA = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);

  if (windowA == nullptr) {
    SDL_Quit();
    return false;
  }

  renderer = SDL_CreateRenderer(windowA, -1, SDL_RENDERER_PRESENTVSYNC);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);

  if (renderer == nullptr) {
    cerr << "Failed to create renderer." << endl;
    SDL_DestroyWindow(windowA);
    SDL_Quit();
    return false;
  }

  if (texture == nullptr) {
    cerr << "Failed to create texture" << endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(windowA);
    SDL_Quit();
    return false;
  }
  
  buffer = new Uint32[WIDTH * HEIGHT];
  
  fontSize = 24;
  menuColor.setRGB(0, 244, 244);
  
  FC_Font* menuFont = FC_CreateFont();
  FC_LoadFont(menuFont, renderer, "dpd/fonts/lazy.ttf", fontSize, FC_MakeColor(menuColor.r, menuColor.g, menuColor.b, 255), TTF_STYLE_NORMAL);

  int x, y = 0;

  SDL_GetWindowPosition(windowA, &x, &y);

  return true;
}

void window::renderFont(int x, int y, string text) {
  FC_Draw(menuFont, renderer, x, y, text.c_str(), text.c_str());
}

unsigned char window::eventHandler(SDL_Event &event) {
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        return 1;
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
          case SDLK_SPACE:
            return 2;
            break;
          case SDLK_LEFT:
            return 3;
            break;
          case SDLK_RIGHT:
            return 4;
            break;
        }
        break;
    }
  }
  return 0;
}

bool window::pointVisible(int x, int y) {
  return (x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT);
}

bool window::noteVisible(note n) {
  return pointVisible(n.x, n.y);
}

int window::getWidth() {
  return WIDTH;
}

int window::getHeight() {
  return HEIGHT;
}

void window::setPixelRGB(int x, int y, Uint8 r, Uint8 g, Uint8 b) {
  if (!pointVisible(x, y)) {
    return;
  }
  
  Uint32 color = 0x00000000;

  color += r;
  color <<= 8;
  color += g;
  color <<= 8;
  color += b;
  color <<=8;
  color += 0xFF;

  buffer[(WIDTH * y) + x] = color;
}

Uint8* window::getPixelRGB(int x, int y) {
  Uint8 rgb[3];
  Uint32 hex = buffer[(WIDTH * y) + x];

  rgb[0] = (hex & 0xFF000000) >> 24;
  rgb[1] = (hex & 0x00FF0000) >> 16;
  rgb[2] = (hex & 0x0000FF00) >> 8;

  Uint8* col = rgb;
  return col;
}

void window::update() {
  SDL_UpdateTexture(texture, nullptr, buffer, WIDTH * sizeof(Uint32));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  SDL_RenderPresent(renderer);
}

void window::clearBuffer() {
  memset(buffer, 0, WIDTH * HEIGHT * sizeof(Uint32));
}

void window::terminate() {
  
  delete[] buffer;

  FC_FreeFont(menuFont);
  
  SDL_DestroyRenderer(renderer);
  SDL_DestroyTexture(texture);
  SDL_DestroyWindow(windowA);
  SDL_Quit();
}

colorRGB::colorRGB() : r(0), g(0), b(0) {}

colorRGB::colorRGB(unsigned char red, unsigned char green, unsigned char blue) : r(red), g(green), b(blue) {}

void colorRGB::setRGB(unsigned char red, unsigned char green, unsigned char blue) {
  r = red;
  g = green;
  b = blue;
}
