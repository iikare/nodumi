#pragma once


#include <string>
#include "log.h"
#include "aghdef.h"

using std::string;

class dialogController {
  public:

    void render();

    bool hover();
    
    bool preferenceDisplay = false;
    bool infoDisplay = false;

  private:

    void renderPreference();
    void renderInfo();

    static constexpr int iconTextSize = 30;
    static constexpr int iconTextAdjust = 10;
    static constexpr double borderMargin = 20;
    static constexpr double iconScale = 0.3;
    const string copySym = "©";
    const string copy = " iika-re 2020-" + string(COPY_YEAR);
    const string license = "Licensed under GPLv3";

    static constexpr int copySymSize = 22;

};
