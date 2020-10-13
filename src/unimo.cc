#include "unimo.h"
#include "log.h"

void UMO::addNote(note* nNote) {
  if (getType() != UMO_NOTE) {
    logII(LL_WARN, "attempt to add note to non-note UMO");
    return;
  }
  n.push_back(nNote);
}

int UMO::getNumEvents() const {
  if (getType() == UMO_NOTE) {
    return n.size();
  }
  return 1;
}

void* UMO::getRawEvent() {
  switch (getType()) {
    case UMO_NOTE:
      return &n;
    case UMO_TIME:
      return ts;
    case UMO_KEY:
      return ks;
    default:
      return nullptr;
  }
}
