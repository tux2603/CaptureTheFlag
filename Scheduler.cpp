#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <thread>
#include "Scheduler.h"

using namespace std;

Request::Request(int sessionID, string text) : sessionID(sessionID), text(text) {}
Request::Request(const Request& toCopy) : Request(toCopy.sessionID, toCopy.text) {}

Scheduler::Scheduler()
{
  shouldExit = false;

  schedulerThread = thread([](Scheduler *s) {
    while (!s->shouldExit.load())
    {
      // Make sure that all the connections have a future in the future list, and that all closed connections are removed
      // Acquire a lock to make sure that other threads won't modify the set while it is being operated on
      lock_guard<mutex> sessionGaurd(s->sessionLock);

      vector<int> toRemove;

      // If there are no sessions connected, simply sit and wait for a bit so that the thread doesn't
      //  gobble CPU time
      if (s->sessions.size() < 1)
        this_thread::sleep_for(chrono::milliseconds(10));

      // Else, there are sessions connected, so perform the necessary logic to handle them
      else
      {
        for (pair<int, Session *> const& i : s->sessions)
        {
          // If the session has stopped, remove it from the set
          if (i.second->getIsDone())
            toRemove.push_back(i.first);

          // Make sure all sessions have a future running
          else if (s->sessionMonitors.count(i.first) == 0)
          {
            // If the session does not have a future running, create one that attempts to read a line from that session
            future<string> *monitor = new future<string>(async(&Session::readLine, i.second));

            // Add the newly created future to the map
            s->sessionMonitors.insert(pair<int, future<string> *>(i.first, monitor));
          }
        }

        // Remove all sessions that were marked for deletion in the previous loop
        for (int id : toRemove)
        {
          Session *session = s->sessions[id];
          s->sessions.erase(id);
          s->sessionMonitors.erase(id);
          delete session;
        }

        // Clear the list of sessions to be removed
        toRemove.clear();

        for (pair<int, future<string>*> const& i : s->sessionMonitors)
        {

          future_status status = i.second->wait_for(chrono::milliseconds(100));

          if (status == future_status::ready)
          {
            string message = i.second->get();
            cout << i.first << ": " << message << endl;

            // If there was some text, add it to the list of requests.
            if(message.length() > 0) {
              // Create a request struct to store the info
              Request r(i.first, message);

              cout << "Created request object thing" << endl;

              // Add the new request to the list of queued requests
              s->requests.push(r);

              cout << "Pushed request to queue" << endl;
            }

            // Mark the future for removal
            toRemove.push_back(i.first);
          }
        }

        // Remove all finished futures from the map
        for (int id : toRemove)
        {
          delete s->sessionMonitors[id];
          s->sessionMonitors.erase(id);
        }
      }
    }
  }, this);
}

set<Session *> Scheduler::getSessions() {
  set<Session *> sessionSet;
  for(pair<int, Session *> const& i : sessions) sessionSet.insert(i.second);
  return sessionSet;
}

Session *Scheduler::getSessionByID(int id) {
  // If there is no such session with that ID, return null
  if(sessions.count(id) < 1) return NULL;

  return sessions[id];
}

void Scheduler::addSession(Session *session)
{
  sessions.insert(pair<int, Session *>(session->getID(), session));
}

Request Scheduler::getNextRequest() {
  Request r(-1, "");

  if (requests.size() > 0) {
    r = requests.front();
    requests.pop();
  }

  return r;
}

bool Scheduler::hasRequest() {
  return requests.size() > 0;
}