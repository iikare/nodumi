#pragma once 

#include <string>
#include "enum.h"

using std::string;


class textController {
  public:


    string getString(const string& str_id) const;

    void setLanguage(langType lang);
    
    langType getLanguage() const { return lang; }


  private:

    langType lang;

};
