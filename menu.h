#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>

using std::string;
using std::vector;

class menuItem {
  public:
    menuItem();
    
    string getContent();
    int getX();
    int getY();
    int getWidth();
    int getHeight();

    void setContent(string nContent);
    void setX(int nX);
    void setY(int nY);
    void setWidth(int nWidth);
    void setHeight(int nHeight);


  private:
    int x;
    int y;
    int width;
    int height;
    string content;
};

class menu {
  public:
    menu();
    menu(int winX, int winY, vector<string> itemNames);
    ~menu();

    int getX() { return x; }
    int getY() { return y; }
    int getWidth() { return width; }
    int getHeight() { return height; }
    int getSize() { return itemCount; }
    int getItemX(int idx); 
    int getItemY(int idx); 
    string getContent(int idx);

    void setXY(int nX, int nY);
    void setContent(string nContent, int idx);

    bool render;

  private:
    int x;
    int y;
    int width;
    int height;
    int itemCount;
    int mainX;
    int mainY;
    menuItem* items;
};




#endif
