#pragma once


#include <string>
#include "log.h"
#include "aghdef.h"

using std::string;

class dialogController {
  public:

    void render();

    void process();

    bool hover();
    
    bool preferenceDisplay = false;
    bool infoDisplay = false;

  private:

    void renderPreference();
    void renderInfo();

    void renderHandRangeSelector();
    int getHRSX(int pos) const;
    int getHRSValue(int index) const;

    int getItemX(int pos, bool box = false) const;
    int getItemY(int pos, bool box = false) const;

    static constexpr int iconTextSize = 30;
    static constexpr int iconTextAdjust = 10;
    static constexpr double borderMargin = 20;
    static constexpr double iconScale = 0.3;
    const string copySym = "©";
    const string copy = " iika-re 2020-" + string(COPY_YEAR);
    const string license = "Licensed under GPLv3";

    static constexpr int copySymSize = 22;

    static constexpr int prefItemXSpacing = 30;
    static constexpr int prefItemYSpacing = 28;
    static constexpr int itemRectSize = 24;
    static constexpr int itemRectInnerSize = 12;
    static constexpr int itemFontSize = 18;

    static constexpr int hrs_boxW = 30;
    static constexpr int hrs_boxH = 30;
    static constexpr int hrs_boxSpacing = 5;
    static constexpr int hrs_max_item = 4;
};
