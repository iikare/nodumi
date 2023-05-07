#pragma once


#include <string>
#include "log.h"
#include "aghdef.h"
#include "dia_opt.h"

using std::string;

class dialogController {
  public:

    void init();

    void render();

    void process();

    bool hover();
    
    bool preferenceDisplay = false;
    bool infoDisplay = false;

  private:

    vector<dialogOption> dia_opts;

    void renderPreference();
    void renderInfo();

    int getItemX(int pos, bool box = false) const;
    int getItemY(int pos, bool box = false) const;

    static constexpr int iconTextSize = 30;
    static constexpr int iconTextAdjust = 10;
    static constexpr double borderMargin = 20;
    static constexpr double iconScale = 0.3;
    const string copySym = "©";
    const string copy = " iika-re 2020-" + string(COPY_YEAR);

    static constexpr int copySymSize = 22;

    static constexpr int prefItemXSpacing = 30;
    static constexpr int prefItemYSpacing = 28;
    static constexpr int itemRectSize = 24;
    static constexpr int itemRectInnerSize = 12;
    static constexpr int itemFontSize = 18;
};
