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

// A simple data structure to store the pertinent information to a single request
class Request
{
  public:
    Request(int, std::string);
    Request(const Request&);
    int sessionID;
    std::string text;
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
  std::map<int, std::future<std::string> *> sessionMonitors;

  /// Lock objects to try to keep the various threads working nicely together
  std::mutex futureLock;
  std::mutex sessionLock;

  /// Stores the requests that come in in order of assigned importance
  std::queue<Request> requests;

  /// Stores all the sessions that the scheduler will handle requests from
  std::map<int, Session *> sessions;

  /// The thread that will perform the scheduling stuff and manage the queue
  std::thread schedulerThread;

  std::atomic<bool> shouldExit;

public:
  Scheduler();
  ~Scheduler();

  void addSession(Session *);
  
  /**
   * Gets a list of active sessions that the scheduler is listening to.
   */
  std::set<Session *> getSessions();

  Session *getSessionByID(int);

  /**
   * Gets the top request on the queue
   */
  Request getNextRequest();

  bool hasRequest();
};

#endif