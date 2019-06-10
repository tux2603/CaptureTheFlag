/*
  This class create a Shell-like object that will listen on a TCP socket and handle requests made
  AUTHOR: Owen O'Connor
*/

#include <iostream>
#include "Scheduler.h"
#include "Session.h"
#include "SocketShell.h"
#include "strutils.h"

#define DEBUG

using namespace std;

SocketShell::SocketShell() : SocketShell::SocketShell(8042) {}

SocketShell::SocketShell(int port) : SocketShell::SocketShell(port, "socketshell@" + to_string(port) + "$ ") {}

SocketShell::SocketShell(int port, string prompt) : port(port)
{
  // Set the prompt. For some reason it doesn't like being set with the variable initializers
  this->prompt = prompt;

  // Try to create a file descriptor for the socket
  if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    throw "Could not create a file descriptor for the socket";

  // Set the options for the socket
  if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    throw "Could not set socket options";

  // Set some parameters for the address
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  // Bind the socket
  if (bind(socketFD, (struct sockaddr *)&address, sizeof(address)) < 0)
    throw "Could not bind socket to port " + to_string(port);

  // Start listening on the socket
  if (listen(socketFD, 256))
    throw "Could not listen on socket";

  // Start a thread that will continually check for new connections
  sessionCheckThread = thread([](int socketFD, struct sockaddr *sockAddr, socklen_t *addrLen, set<Session *> *sessions, Scheduler *scheduler) {
    cout << "Session checking thread started" << endl;

    while (true)
    {
      // Wait for a new connection request to come in on the socket
      int sessionFD = accept(socketFD, sockAddr, addrLen);

      // Make sure that the file descriptor returned is valid
      if (sessionFD >= 0)
      {
        // !!!!! ####################################################### !!!!!
        // !!!!!                  POSSIBLE MEMORY LEAK                   !!!!!
        // !!!!! ####################################################### !!!!!
        // Create a new session object to handle the session
        Session *sessionObject = new Session(sessionFD);

        // Add the session to the list
        scheduler->addSession(sessionObject);
      }
    }
  },
                              socketFD, (struct sockaddr *)&address, (socklen_t *)&addrLen, &sessions, &scheduler);
}

set<Session *> SocketShell::getSessions()
{
  return sessions;
}

int SocketShell::getPort()
{
  return port;
}

void SocketShell::addCommand(string name, function<string(int, string[], SocketShell *, Session *)> lambda)
{
  commandDictionary.insert(pair<string, function<string(int, string[], SocketShell *, Session *)>>(name, lambda));
}

void SocketShell::update()
{
  // TODO ############################################################### TODO
  // TODO    find a way to flush each sessions queue in an 'fair' way     TODO
  // TODO ############################################################### TODO
  for (auto session : sessions)
  {
    int numlinesIn = session->numQueuedLines();

    if (numlinesIn > 0 || true)
    {
      string request = session->readLine();

      if(request.length() > 0) {
        cout << "Got request " << request << flush;

        // Break the request into tokens
        int argc = countWords(request);
        string *argv = getWords(request);
        string message = "";

        if (argc > 0)
        {
          if (commandDictionary.count(argv[0]))
          {
            message = commandDictionary.at(argv[0])(argc, argv, this, session);
          }

          else
          {
            message = argv[0] + ": command not found. Type `help' for a list of available commands";
          }
        }

        session->sendMessage(message + "\n");

        delete [] argv;
      }
    }
  }
}

// ###########################################################################
// #####                          BEGIN TEST BED                         #####
// ###########################################################################

int main()
{
  SocketShell *s1 = new SocketShell();

  s1->addCommand("wall", [](int argc, string argv[], SocketShell *s, Session *c) -> string {
    string message = c->getUsrID() + ": ";

    for (int i = 1; i < argc; i++)
    {
      message += argv[i] + " ";
    }

    set<Session *> sessions = s->getSessions();

    for (Session *session : sessions)
    {
      session->sendMessage(message + "\n");
    }

    return "";
  });

  cout << s1->getPrompt() << endl
       << s1->getPort() << endl;

  while (true)
  {
    this_thread::sleep_for(chrono::milliseconds(100));
    //s1->update();
  }

  return 0;
}