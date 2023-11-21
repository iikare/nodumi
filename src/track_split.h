#pragma once

#include "build_target.h"
#include "note.h"
#include "midi.h"

int findTrack(const note& n, const midi& m_stream, bool live = true, int numOn = -1); 
