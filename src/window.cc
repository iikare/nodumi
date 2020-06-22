#include <algorithm>
#include <SDL2/SDL_ttf.h>

#include "window.h"
#include "note.h"

using std::cerr;
using std::endl;
using std::swap;
using std::fill;

window::window(string title) : 
  cursorVisible(false), windowA(nullptr), renderer(nullptr), texture(nullptr),
  windowX(0), windowY(0), messageX(0), messageY(0), messageText(0), messageCol(0), tTexture(nullptr), tSurface(nullptr), clipX(0), clipY(0),
  buffer(nullptr), menuFont(nullptr), menuColor(0, 0, 0), fontSize(0),
  mouseX(0), mouseY(0) {
  this->title = title;
  clip = {0, 0, 0, 0};
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
  
  fontSize = 14;
  menuColor.setRGB(0, 0, 0);
 
  if (TTF_Init() < 0) {
    cerr << "warn: font engine initialization failed" << endl;
  }
  
  menuFont = TTF_OpenFont("dpd/fonts/yklight.ttf", fontSize);
  
  if(menuFont == nullptr) {
    cerr << "error: font initialization failed" << endl;
    exit(1);
  } 
  int x, y = 0;

  SDL_GetWindowPosition(windowA, &x, &y);

  return true;
}

void window::renderText(int x, int y, string text, colorRGB col) {
  messageX.push_back(x);
  messageY.push_back(y);
  messageText.push_back(text);
  messageCol.push_back(col);
}

void window::renderTextToTexture(int x, int y, string text, colorRGB col) {
  SDL_Color color = {static_cast<Uint8>(col.r), static_cast<Uint8>(col.g),
                     static_cast<Uint8>(col.b), 255};
  tSurface = TTF_RenderText_Blended(menuFont, text.c_str(), color);
  tTexture = SDL_CreateTextureFromSurface(renderer, tSurface);
  SDL_FreeSurface(tSurface);
  SDL_QueryTexture(tTexture, nullptr, nullptr, &clipX, &clipY);
  clip = {x + TEXT_OFFSET, y + TEXT_OFFSET, clipX, clipY};
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
            if (event.key.keysym.mod == KMOD_LCTRL || event.key.keysym.mod == KMOD_RCTRL){
              return 9;
              break;
            }
            return 3;
            break;
          case SDLK_RIGHT:
            if (event.key.keysym.mod == KMOD_LCTRL || event.key.keysym.mod == KMOD_RCTRL){
              return 10;
              break;
            }
            return 4;
            break;
          case SDLK_UP:
            return 5;
            break;
          case SDLK_DOWN:
            return 6;
            break;
          case SDLK_HOME:
            return 7;
            break;
          case SDLK_END:
            return 8;
            break;
        }
        break;
      case SDL_MOUSEWHEEL:
        if (event.wheel.y > 0) {
          return 5;
        }
        else if (event.wheel.y < 0) {
          return 6;
        }
        break;
      case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_LEFT) {
          return 11;
        }
        else if (event.button.button == SDL_BUTTON_RIGHT) {
          return 12;
        }
        break;
      case SDL_MOUSEBUTTONUP:
        if (event.button.button == SDL_BUTTON_LEFT) {
          return 13;
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

void window::updateCursor() {
  SDL_GetGlobalMouseState(&mouseX, &mouseY);
  SDL_GetWindowPosition(windowA, &windowX, &windowY);
  mouseX = mouseX - windowX;
  mouseY = mouseY - windowY;

  cursorVisible =  mouseX < WIDTH && mouseX > 0 && mouseY < HEIGHT && mouseY > 0;
}

int window::getWidth() {
  return WIDTH;
}

int window::getHeight() {
  return HEIGHT;
}

int window::getMouseX() {
  return mouseX;
}

int window::getMouseY() {
  return mouseY;
}

void window::setPixelHSV(int x, int y, double h, double s, double v) {
  colorHSV col(h, s, v);
  colorRGB col2;
  col2.setRGB(col);
  setPixelRGB(x, y, col2);
}

void window::setPixelHSV(int x, int y, colorHSV col) {
  colorRGB col2;
  col2.setRGB(col);
  setPixelRGB(x, y, col2);
}

void window::setPixelRGB(int x, int y, colorRGB col) {
  setPixelRGB(x, y, col.r, col.g, col.b);
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

  int messageCount = messageX.size();
  for (int i = 0; i < messageCount; i++){
    renderTextToTexture(messageX.back(), messageY.back(), messageText.back(), messageCol.back());
    SDL_RenderCopy(renderer, tTexture, nullptr, &clip);
    messageX.pop_back();
    messageY.pop_back();
    messageText.pop_back();
    messageCol.pop_back();
  }

  SDL_RenderPresent(renderer);

}

void window::clearBuffer() {
  memset(buffer, 0, WIDTH * HEIGHT * sizeof(Uint32));
}

void window::terminate() {
  
  delete[] buffer;

  TTF_CloseFont(menuFont);
  menuFont = nullptr;

  SDL_DestroyTexture(texture);
  SDL_DestroyTexture(tTexture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(windowA);
  TTF_Quit();
  SDL_Quit();
}
