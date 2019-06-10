#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include "Scheduler.h"

using namespace std;

Scheduler::Scheduler() {
  shouldExit = false;

  schedulerThread = thread([](Scheduler *s) {
    while (!s->shouldExit.load()) {
      // Make sure that all the connections have a future in the future list, and that all closed connections are removed
      {
        // Acquire a lock to make sure that other threads won't modify the set while it is being operated on
        lock_guard<mutex> sessionGaurd(s->sessionLock);

        vector<Session *> toRemove;

        for(Session *session : s->sessions) {
          // If the session has stopped, remove it from the set
          if(session->getIsDone()) {
            toRemove.push_back(session);
          }

          // Make sure all sessions have a future running
          else if(s->sessionMonitors.count(session) == 0) {
            future<string> *monitor = new future<string>(async(&Session::readLine, session));

            s->sessionMonitors.insert(pair<Session *, future<string> *>(session, monitor));
          }
        } 

        for(Session *session : toRemove) {
          s->sessions.erase(session);
          s->sessionMonitors.erase(session);
          delete session;
        }
      }; 

      vector<Session *> toRemove;

      for(pair<Session *, future<string> *> const& i : s->sessionMonitors) {

        future_status status = i.second->wait_for(chrono::milliseconds(100));

        if(status == future_status::ready) {
          string message = i.second->get();
          cout << i.first->getSessionFD() << ": " << message << endl;

          // Mark the future for removal
          toRemove.push_back(i.first);
        }
      }

      // Remove all finished futures from the map
      for(Session *session: toRemove) {
        delete s->sessionMonitors[session];
        s->sessionMonitors.erase(session);
      }
    } 
  }, this);
}

void Scheduler::addSession(Session *session) {
  sessions.insert(session);
}