#pragma once

#include <string>
#include "../dpd/osdialog/osdialog.h"

using std::string;

class ioController {
  public:

    ioController() = delete;
    ioController(osdialog_file_action action, string types);
    ~ioController();

    void dialog(const char* defName = nullptr);
    void reset();
    void resetPending();
    void setPending(string path);

    bool pending() const { return pending_item; }
    string getPath() const { return path; }


  private:

    osdialog_file_action action;
    osdialog_filters* filter;
    
    bool pending_item = false;
    string path;
    string working_dir = ".";


};
