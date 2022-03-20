#pragma once

#include <vector>
#include <algorithm>

using std::find;
using std::vector;
using std::is_enum;

template <class T>
class enumChecker {
  public:

    enumChecker() : items(0), itemsLastFrame(0) {
      static_assert(is_enum<T>::value, "class requires an enum type");
    }

    void add(T item) {
      if (!contains(item)) {
        items.push_back(item);
      }
    }

    void remove(T item) {
      items.remove(item);
    }

    void clear() {
      itemsLastFrame = items;
      items.clear();
    }

    bool contains(T item) {
      return find(items.begin(), items.end(), item) != items.end(); 
    }

    bool contains(T item1, T item2...) {
      return contains(item1) || contains(item2);
    }
    
    bool containsLastFrame(T item) {
      return find(itemsLastFrame.begin(), itemsLastFrame.end(), item) != itemsLastFrame.end(); 
    }

    bool containsLastFrame(T item1, T item2...) {
      return containsLastFrame(item1) || containsLastFrame(item2);
    }


  private:
    vector<T> items;
    vector<T> itemsLastFrame;
};

enum schemeType {
  SCHEME_TRACK,
  SCHEME_TONIC,
  SCHEME_KEY,
  SCHEME_NONE
};

enum colorType {
  SELECT_BG,
  SELECT_LINE,
  SELECT_NOTE,
  SELECT_SHEET,
  SELECT_MEASURE,
  SELECT_NONE
};

enum displayType {
  DISPLAY_LINE,
  DISPLAY_BAR,
  DISPLAY_BALL,
  DISPLAY_BALLLINE
};

enum menuType {
  TYPE_MAIN,
  TYPE_SUB,
  TYPE_RIGHT,
  TYPE_RIGHTSUB,
  TYPE_COLOR
};

enum songTimeType {
  SONGTIME_RELATIVE,
  SONGTIME_ABSOLUTE,
  SONGTIME_NONE
};

enum hoverType {
  HOVER_NOW,
  HOVER_NOTE,
  HOVER_IMAGE,
  HOVER_MEASURE,
  HOVER_SHEET,
  HOVER_MENU,
  HOVER_BG, //default, should not be added
  HOVER_NONE
};

enum colorMode {
  COLOR_PART,
  COLOR_VELOCITY,
  COLOR_TONIC
};
