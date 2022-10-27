#include "io.h"
#include "define.h"

ioController::ioController(osdialog_file_action action, string types) {
  this->action = action;
  filter = osdialog_filters_parse(types.c_str());
}

ioController::~ioController() {
  osdialog_filters_free(filter); 
}

void ioController::dialog(const char* cdir, const char* defName) {
  // prevent buffer overrun while osdialog blocks the main thread 
  if (ctr.getLiveState()) {
    ctr.liveInput.pauseInput();
  }
  else {
    ctr.fileOutput.disallow();
  }

  char* result = osdialog_file(action, cdir, defName, filter);
 
  if (result != nullptr) {
    path = static_cast<string>(result);
    pending_item = true;
  }

  free(result);

  if (ctr.getLiveState()) {
    ctr.liveInput.resumeInput();
  }
  else {
    ctr.fileOutput.allow();
  }
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
