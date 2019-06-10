/*
  This class create a Shell-like object that will listen on a TCP socket and handle requests made
  AUTHOR: Owen O'Connor
*/

#include <iostream>
#include "Scheduler.h"
#include "Session.h"
#include "SocketShell.h"
#include "strutils.h"

using namespace std;

SocketShell::SocketShell() : SocketShell::SocketShell(8042) {}

SocketShell::SocketShell(int port) : SocketShell::SocketShell(port, "socketshell@" + to_string(port) + "$ ") {}

SocketShell::SocketShell(int port, string prompt) : port(port), prompt(prompt)
{
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
  sessionCheckThread = thread([](int socketFD, struct sockaddr *sockAddr, socklen_t *addrLen, Scheduler *scheduler, string defaultPrompt) {
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
        Session *sessionObject = new Session(sessionFD, defaultPrompt);

        // Add the session to the list
        scheduler->addSession(sessionObject);
      }
    }
  }, socketFD, (struct sockaddr *)&address, (socklen_t *)&addrLen, &scheduler, prompt);
}

set<Session *> SocketShell::getSessions()
{
  return scheduler.getSessions();
}

int SocketShell::getPort()
{
  return port;
}

string SocketShell::getPrompt()
{
  return prompt;
}

void SocketShell::setPrompt(string p)
{
  prompt = p;
}

void SocketShell::addCommand(string name, function<string(int, string[], SocketShell *, Session *)> lambda)
{
  commandDictionary.insert(pair<string, function<string(int, string[], SocketShell *, Session *)>>(name, lambda));
}

set<string> SocketShell::listCommands() {
  set<string> commandsSet = set<string>();

  for (pair<string, function<string(int, string[], SocketShell *, Session *)>> const& commandPair : commandDictionary) {
    commandsSet.insert(commandPair.first);
  }

  return commandsSet;
}

void SocketShell::update()
{
  if(scheduler.hasRequest()) {
    cout << "Got a request" << endl;
    Request r = scheduler.getNextRequest();
    cout << "Session ID: " << r.sessionID << "\t Request text: " << r.text << endl;

    // Make sure that the session still exists
    if(scheduler.getSessionByID(r.sessionID) == NULL) {
      cout << " ...but that session is no longer here. Oh well" << endl;
    }

    else {
      // If the session does still exist, split the request into tokens
      int argc = countWords(r.text);
      string *argv = getWords(r.text);

      string message = "";

      // Execute the command (if we got one...)
      if (argc > 0)
      {
        // If the command is in the dictionary, execute it
        if (commandDictionary.count(argv[0]))
          message = commandDictionary.at(argv[0])(argc, argv, this, scheduler.getSessionByID(r.sessionID));

        //If the command wasn't in the dictionary, give an error message
        else
          message = argv[0] + ": command not found. Type `help' for a list of available commands";
      }

      // If no command was given, give an error message
      else 
        message = "No command given. Type `help' for a list of available commands";

      // Send the return message to the session
      scheduler.getSessionByID(r.sessionID)->sendMessage(message + "\n");
      scheduler.getSessionByID(r.sessionID)->sendPrompt();

      // So as to have no memory leaks
      delete [] argv;
    }
  }

  else {
    // TODO Magic number
    this_thread::sleep_for(chrono::milliseconds(10));
  }
}

// ###########################################################################
// #####                          BEGIN TEST BED                         #####
// ###########################################################################

#define TESTBED
#ifdef TESTBED

int main()
{
  SocketShell *s1 = new SocketShell();

  s1->addCommand("wall", [](int argc, string argv[], SocketShell *shell, Session *callingSession) -> string {
    string message = "\n" + to_string(callingSession->getID()) + ": ";

    for (int i = 1; i < argc; i++)
    {
      message += argv[i] + " ";
    }

    set<Session *> sessions = shell->getSessions();

    for (Session *session : sessions)
    {
      if(session->getID() != callingSession->getID()) {
        session->sendMessage(message + "\n");
        session->sendPrompt();
      }
    }

    return "";
  });

  cout << "SocketShell up and running, and waiting for connections." << endl;

  while (true)
  {
    s1->update();
  }

  return 0;
}

#endif