#include "io.h"
#include "define.h"

ioController::ioController(osdialog_file_action action, string types) {
  this->action = action;
  filter = osdialog_filters_parse(types.c_str());
}

ioController::~ioController() {
  osdialog_filters_free(filter); 
}

void ioController::dialog(const char* defName) {
  // prevent buffer overrun while osdialog blocks the main thread 
  ctr.criticalSection(true);

  char* result = osdialog_file(action, working_dir.c_str(), defName, filter);
 
  if (result != nullptr) {
    path = static_cast<string>(result);
    working_dir = getDirectory(path);

    pending_item = true;
  }

  free(result);

  ctr.criticalSection(false);
}

void ioController::reset() {
  path = "";
  pending_item = false;
}
    
void ioController::resetPending() {
  pending_item = false;
}

void ioController::setPending(string path) {
  this->path = path;
  pending_item = true;
}
