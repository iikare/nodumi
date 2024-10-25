#pragma once

#include "build_target.h"
#include "midi.h"
#include "note.h"

int findTrack(const note& n, const midi& m_stream, bool live = true, int numOn = -1);
