#include "output_sync.h"
#include <chrono>

void outputInstance::init(midiOutput* out) {
  output = out;

  oThread = thread(&outputInstance::process, this);
}
void outputInstance::updateOffset(double offset) {
  disableInterrupt();
  //if (fabs(offset - this->offset) > std::numeric_limits<double>::epsilon()) {
    //interrupt = false;
    //return;
  //}

  unsigned int index_last = index;
  for (unsigned int idx = 0; idx < message.size(); ++idx) {
    if (message[idx].first < 0.01) {
      //logQ("alert");
      continue;
    }
    if (message[idx].first >= offset) {  
      index = idx;//idx == 0 ? 0 : idx - 1;
      break;
    }
  }
  
  // TODO: a bunch of notes are stuck at time = 0 and are skipped
  
  if (index_last > index) {
    //logQ("skipped note (backward):", index_last, index);
  }
  if (index_last < index) {
    logQ("skipped note (forward):", index_last, index);
    index = index_last;
  }
  else {
    this->offset = offset;
    last_update = std::chrono::high_resolution_clock::now();
  }
  enableInterrupt();
}

void outputInstance::terminate() {
  end = true;
  oThread.join();
}

void outputInstance::process() {   
  constexpr int msgLimit = 30;
  while (true) {
    if (end) {
      break;
    }
    if (!interrupt) {
      interrupt_ack = false;
      if (send) {
        std::chrono::duration<double> elapsedTime = std::chrono::high_resolution_clock::now() - last_update;

        int msgFound = 0;
        for (unsigned int idx = index; idx < message.size(); idx++) {
          if (message[idx].first >= offset && message[idx].first <= offset+elapsedTime.count()*500) {
            //logQ("time", message[idx].first, formatVector(message[idx].second));
            msgFound++;
            //if (msgFound < msgLimit || message[idx].first < 0.1) {
                output->sendMessage(&message[idx].second);
            //}
          }
          if (message[idx].first > offset+elapsedTime.count()*500) {
            break;
          }
        }


        index += msgFound;

      
        //if (msgFound) logQ("count", msgFound);
        //logQ("time since last offset update", elapsedTime);
        //logQ("hi from RT thread:", offset, message.size());
      }
    }
    else {
      //logQ("INT");
      interrupt_ack = true;
    }
  }
}

void outputInstance::load(const vector<pair<double, vector<unsigned char>>>& message) {
  disableInterrupt();
  this->message = message;
  enableInterrupt();
}

void outputInstance::allow() {
  disableInterrupt();
  send = true;
  enableInterrupt();
}
void outputInstance::disallow() {
  disableInterrupt();
  send = false;
  vector<unsigned char> s = {120, 0};
  output->sendMessage(&s);
  enableInterrupt();
}

void outputInstance::enableInterrupt() {
  interrupt = false;
}

void outputInstance::disableInterrupt() {
  interrupt = true;
  while (!interrupt_ack);
}
