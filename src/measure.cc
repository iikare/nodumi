#include "measure.h"

void measureController::clear() {
  // NOTE: not used
  // notes.clear();
  // timeSignatures.clear();
  // keySignatures.clear();
  // currentTime = timeSig();
  // currentKey = keySig();
}

void measureController::addNote(note& note) {
  // quantization and splitting happen when the sheet controller dissects the measure
  notes.push_back(note.number);
}
