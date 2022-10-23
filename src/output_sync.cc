#include "output_sync.h"
#include <chrono>

void outputInstance::init(midiOutput* out) {
  output = out;

  oThread = thread(&outputInstance::process, this);
}
void outputInstance::updateOffset(double offset) {
  interrupt = true;
  while (!interrupt_ack);
  //if (fabs(offset - this->offset) > std::numeric_limits<double>::epsilon()) {
    //interrupt = false;
    //return;
  //}

  unsigned int index_last = index;
  for (unsigned int idx = 0; const auto& msg : message) {
    if (msg.first >= offset) {  
      index = idx;//idx == 0 ? 0 : idx - 1;
      break;
    }
    idx++;
  }
  
  if (index_last != index) {
    logQ("skipped note:", index_last, index);
    ////index = \cc;
    /////index = index_last;
  }
  this->offset = offset;
  last_update = std::chrono::high_resolution_clock::now();
  
  interrupt = false;
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
  interrupt = true;
  while (!interrupt_ack);
  this->message = message;
  interrupt = false;
}

void outputInstance::allow() {
  interrupt = true;
  while (!interrupt_ack);
  send = true;
  interrupt = false;
}
void outputInstance::disallow() {
  interrupt = true;
  while (!interrupt_ack);
  send = false;
  interrupt = false;
}

