#include "track_split.h"

#include "controller.h"
#include "define.h"

int findTrack(const note& n, const midi& m_stream, bool live, int numOn) {
  if (!live) {
    // return n.y % 2;
  }

  constexpr int seqLimit = 2000;

  // find latest old note
  int i = 0;
  double minX = std::numeric_limits<double>::max();
  int minIdx = 0;

  // go back a minimum of seqLimit v. the oldest noteOn event
  int adjNotes = 0;
  int noteCount = live ? m_stream.getNoteCount() : numOn;
  int j = noteCount;

  // logQ("test", n.x, n.isOn);

  while (m_stream.notes[j].x >= 0 && j >= 0) {
    // logW(LL_CRIT, "break param:",  m_stream.notes[j].x+seqLimit, n.x);
    if (m_stream.notes[j].x + seqLimit <= n.x) {
      if (adjNotes == 0) {
        minX = 0;
      }

      // logW(LL_WARN, "while BREAK on note:", m_stream.notes[j].x,
      // m_stream.notes[j].y);

      break;
    }
    // logW(LL_WARN, "while on note:", m_stream.notes[j].x,
    // m_stream.notes[j].y);

    if (m_stream.notes[j].isOn) {
      i++;
    }

    adjNotes++;
    minX = m_stream.notes[j].x;
    minIdx = j;
    j--;
  }

  if (i != numOn && live) {
    for (; j >= 0; --j) {
      // avoid this note (which is ON by definition)
      if (i == numOn) {
        // all on notes shifted
        break;
      }
      if (m_stream.notes[j].isOn) {
        if (m_stream.notes[j].x < minX) {
          minX = m_stream.notes[j].x;
          minIdx = j;
        }
        // logQ("j, X", j, m_stream.notes[j].x);
        i++;
      }
    }
  }

  // logQ("MINIDX MINX", minIdx, minX, "|",i,numOn);

  // minIdx = max(minIdx - 5, 0);

  // find track range
  int tr0minY = 127;
  int tr0maxY = 0;
  int tr1minY = 127;
  int tr1maxY = 0;

  // one hand range (approx. a 10th)
  const int handRange = ctr.option.get(OPTION::HAND_RANGE);
  constexpr int onLimit = 350;

  vector<int> considerN;
  // vector<int> onN;
  for (int j = minIdx; j <= noteCount; j++) {
    if (m_stream.notes[j].isOn || m_stream.notes[j].x + onLimit > n.x) {
      // 1, then 0
      if (j != noteCount) {  // exclude this note
        if (m_stream.notes[j].track == 1) {
          tr1maxY = max(tr1maxY, m_stream.notes[j].y);
          tr1minY = min(tr1minY, m_stream.notes[j].y);
        }
        else {
          tr0maxY = max(tr0maxY, m_stream.notes[j].y);
          tr0minY = min(tr0minY, m_stream.notes[j].y);
        }
      }

      // onN.push_back(j);
    }

    considerN.push_back(j);
  }

  // logQ("range:", max(tr0maxY,tr1maxY) -  min(tr0minY,tr1minY));
  // logQ("range0:", tr0maxY - tr0minY);
  // logQ("range1:", tr1maxY - tr1minY);

  // consider range WITH this note

  int new_tr1maxY = max(tr1maxY, n.y);
  int new_tr1minY = min(tr1minY, n.y);
  int new_tr0maxY = max(tr0maxY, n.y);
  int new_tr0minY = min(tr0minY, n.y);

  // logQ("range0 (UPD):", new_tr0maxY - new_tr0minY);
  // logQ("range1 (UPD):", new_tr1maxY - new_tr1minY);

  // logQ("consider Y", relevantY);

  if (considerN.size() == 1) {
    if (m_stream.notes[considerN[0]].y >= 60) {
      return 1;
    }
    return 0;
  }

  bool outOfRange0 = new_tr0maxY - new_tr0minY > handRange;
  bool outOfRange1 = new_tr1maxY - new_tr1minY > handRange;

  // force assign to other hand if adding to this track causes an out of range
  if (!outOfRange0 && outOfRange1) {
    return 0;
  }
  else if (outOfRange0 && !outOfRange1) {
    return 1;
  }

  // int tr0sum = 0;
  // int tr1sum = 0;
  int tr0ct = 0;
  int tr1ct = 0;

  for (auto considerNote : considerN) {
    //(m_stream.notes[considerNote].track ? tr1sum : tr0sum) +=
    // m_stream.notes[considerNote].y;
    (m_stream.notes[considerNote].track ? tr1ct : tr0ct)++;
  }

  // double tr0avg = ((tr0ct == 0) ? -1 : tr0sum/static_cast<double>(tr0ct));
  // double tr1avg = ((tr1ct == 0) ? -1 : tr1sum/static_cast<double>(tr1ct));
  // double dist0 = fabs(tr0avg - n.y);
  // double dist1 = fabs(tr1avg - n.y);

  // logQ("consider track avg", tr0avg, tr1avg);

  // if both in range, choose track with least notes
  if (!outOfRange0 && !outOfRange1) {
    // return tr0ct >= tr1ct;
    // if (fabs(dist0-dist1) < 2) {
    // logW(LL_CRIT, "avg. mediator");
    // return tr0ct >= tr1ct;
    //}
    return m_stream.notes[noteCount - 1].track;
  }

  return 0;
}
