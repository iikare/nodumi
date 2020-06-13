#include <algorithm>

#include "window.h"
#include "note.h"

using std::cerr;
using std::endl;
using std::swap;
using std::fill;

window::window(string title) : 
  windowA(nullptr), renderer(nullptr), texture(nullptr), 
  buffer(nullptr), backBuffer(nullptr), colbuf(nullptr) {
  this->title = title;
}

bool window::init() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    cerr << "Failed to initialize SDL" << endl;
    return false;
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
  backBuffer = new Uint32[WIDTH * HEIGHT];
  colbuf = new unsigned char[WIDTH * HEIGHT];
  
  int x, y = 0;

  SDL_GetWindowPosition(windowA, &x, &y);

  return true;
}

unsigned char window::eventHandler(SDL_Event &event, int &shiftX) {
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        return 1;
        break;
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_SPACE) {
          return 2;
        }
        else if (event.key.keysym.sym == SDLK_RIGHT) {
          return 3;
        }
        else if(event.key.keysym.sym == SDLK_LEFT) {
          return 4;
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

void window::setPixelHex(int x, int y, unsigned char hex) {
  colbuf[(WIDTH * y) + x] = hex;
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

unsigned char window::getPixelHex(int x, int y) {
  return colbuf[(WIDTH * y) + x];
}

void window::blur(int intensity) {
  swapBuffer();

  Uint8 meanrgb[3];

  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; x < HEIGHT; y++) {
      int r, g, b = 0;
      
      for(int col = -1; col <= 1; col++) {
        for(int row = -1; row <= 1; row++) {
          int cx = x + col;
          int cy = y + row;
          if(pointVisible(cx, cy)) {
            Uint32 blurcol = backBuffer[(cy * WIDTH) + cx];
            
            r += ((blurcol & 0xFF000000) >> 24) + intensity;
            g += ((blurcol & 0x00FF0000) >> 16) + intensity;
            b += ((blurcol & 0x0000FF00) >> 8) + intensity;
          }
        }
      }

      meanrgb[0] = r/9;
      meanrgb[1] = g/9;
      meanrgb[2] = b/9;

      setPixelRGB(x, y, meanrgb[0], meanrgb[1], meanrgb[2]);
    } 
  }
}  

void window::swapBuffer() {
  swap(buffer, backBuffer);
}

void window::update() {
  SDL_UpdateTexture(texture, nullptr, buffer, WIDTH * sizeof(Uint32));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  SDL_RenderPresent(renderer);
}

void window::clearBuffer() {
  memset(buffer, 0, WIDTH * HEIGHT * sizeof(Uint32));
  memset(backBuffer, 0, WIDTH * HEIGHT * sizeof(Uint32));
  memset(colbuf, 0, WIDTH * HEIGHT * sizeof(unsigned char));
}

void window::terminate() {
  
  delete[] buffer;
  delete[] backBuffer;
  delete colbuf;
  
  SDL_DestroyRenderer(renderer);
  SDL_DestroyTexture(texture);
  SDL_DestroyWindow(windowA);
  SDL_Quit();
}

void colorRGB::setRGB(char red, char green, char blue) {
  r = red;
  g = green;
  b = blue;
}
