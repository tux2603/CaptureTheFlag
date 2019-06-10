#ifndef SCHEDULER_H_BLOCK
#define SCHEDULER_H_BLOCK

#include <future>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <thread>
#include "Session.h"

using namespace std;

// A simple data structure to store the pertinent information to a single request
struct request
{
  Session *session;
  string text;
};

class Scheduler
{
  /* Scheduler class will have to:
      - Maintain a list of all extant sessions
      - Check all sessions for requests in a timely, non-busy manner
      - Queue requests from the sessions so that all requests from any given 
        session are kept in the same order, and no one thread can hog all of 
        the resources */
private:
  /// Stores futures that monitor requests coming in from the various sessions
  map<Session *, future<string> *> sessionMonitors;

  /// Lock objects to try to keep the various threads working nicely together
  mutex queueLock;
  mutex sessionLock;

  /// Stores the requests that come in in order of assigned importance
  queue<request> requests;

  /// Stores all the sessions that the scheduler will handle requests from
  set<Session *> sessions;

  /// The thread that will perform the scheduling stuff and manage the queue
  thread schedulerThread;

  atomic<bool> shouldExit;

public:
  Scheduler();

  void addSession(Session *);
  
  /**
   * Gets a list of active sessions that the scheduler is listening to.
   */
  set<Session *> getSessions();

  /**
   * Gets the top request on the queue
   */
  string getNextRequest();
};

#endif