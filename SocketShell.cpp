/*
  This class create a Shell-like object that will listen on a TCP socket and handle requests made
  AUTHOR: Owen O'Connor
*/

// TODO ################################################################# TODO
// TODO     Add destructor of some sort that waits for the connection     TODO
// TODO                      checking thread to join                      TODO
// TODO ################################################################# TODO

#include <atomic>
#include <iostream>
#include <unistd.h>
#include "Scheduler.h"
#include "Session.h"
#include "SocketShell.h"
#include "strutils.h"

using namespace std;

SocketShell::SocketShell() : SocketShell::SocketShell(8042) {}

SocketShell::SocketShell(int port) : SocketShell::SocketShell(port, "socketshell@" + to_string(port) + "$ ") {}

SocketShell::SocketShell(int port, string prompt) : port(port), prompt(prompt), shouldExit(false)
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

  // Not strictly necessary, but keeps the vscode syntax coloring happy
  struct sockaddr *addrPointer =  (struct sockaddr*)&address;

  // Bind the socket
  if (bind(socketFD, addrPointer, sizeof(address)) < 0) 
    throw "Could not bind socket to port " + to_string(port);

  // Start listening on the socket
  if (listen(socketFD, 256))
    throw "Could not listen on socket";

  // Start a thread that will continually check for new connections
  sessionCheckThread = thread([](int socketFD, struct sockaddr *sockAddr, socklen_t *addrLen, Scheduler *scheduler, string defaultPrompt, atomic<bool> *shouldExit) {
    // cout << "Session checking thread started" << endl;

    while (!shouldExit->load())
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
  }, socketFD, (struct sockaddr *)&address, (socklen_t *)&addrLen, &scheduler, prompt, &shouldExit);
}

SocketShell::~SocketShell() {
  shouldExit = true;
  close(socketFD);
  cout << "Waiting for socket listener to stop..." << endl;
  sessionCheckThread.join();
  cout << "Socket has joined" << endl;
}

set<Session *> SocketShell::getSessions()
{
  return scheduler.getSessions();
}

Session *SocketShell::getSessionByID(int id) {
  return scheduler.getSessionByID(id);
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
  // Since this is stored internally as a map and wanted externally as a set, it will have to be converted
  // ? As such, this is a rather expensive function. Maintain set with getters/setters?
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

//#define TESTBED
#ifdef TESTBED

int main()
{
  SocketShell s1;

  s1.addCommand("help", [](int argc, string argv[], SocketShell *shell, Session *callingSession) -> string {
    string message = "The following commands are available: ";

    set<string> commands = shell->listCommands();

    for(string i : commands) {
      message += i + " ";
    }

    if(argc >= 5 && !argv[1].compare("help") && !argv[2].compare("help") && !argv[3].compare("help") && !argv[4].compare("help")) {
      message = "\nA friendly message from the local neighborhood cow:\n";
      message += " ________________________________ \n";
      message += "< THERE SHALL BE NO HELP FOR YOU >\n";
      message += " -------------------------------- \n";                                                            
      message += "        \\   ^__^                  \n";                                                            
      message += "         \\  (oo)\\_______          \n";                                                           
      message += "            (__)\\       )\\/\      \n";  
      message += "                ||----w |           \n";
      message += "                ||     ||           \n";
    }

    return message;
  });

  s1.addCommand("setName", [](int argc, string argv[], SocketShell *shell, Session *callingSession) -> string {
    string name = "";

    for(int i = 1; i < argc; i++) {
      name += argv[i];
      if(i < argc - 1) name += " ";
    }

    callingSession->setName(name);

    return "Name has been set to `" + name + "'";
  });

   s1.addCommand("setPrompt", [](int argc, string argv[], SocketShell *shell, Session *callingSession) -> string {
    
    string prompt = "";

    for( int i = 1; i < argc; i++) {
      prompt += argv[i] + " ";
    }
    
    callingSession->setPrompt(prompt);

    return "";
  });

  s1.addCommand("wall", [](int argc, string argv[], SocketShell *shell, Session *callingSession) -> string {
    string message = "\n";

    if(callingSession->getName().length() > 0) {
      message += callingSession->getName() + " (" + to_string(callingSession->getID()) + "): ";
    }

    else {
      message += to_string(callingSession->getID()) + ": ";
    }

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

  s1.addCommand("write", [](int argc, string argv[], SocketShell *shell, Session *callingSession) -> string {

    if(argc < 2) {
      return "write requires at least one argument\n";
    }

    if(shell->getSessionByID(atoi(argv[1].c_str())) == NULL) {
      return "No user found with id " + argv[1];
    }

    string message = "\n";

    if(callingSession->getName().length() > 0) {
      message += callingSession->getName() + " (" + to_string(callingSession->getID()) + "): ";
    }

    else {
      message += to_string(callingSession->getID()) + ": ";
    }

    for (int i = 1; i < argc; i++)
    {
      message += argv[i] + " ";
    }

    shell->getSessionByID(atoi(argv[1].c_str()))->sendMessage(message);
    shell->getSessionByID(atoi(argv[1].c_str()));

    return "";
  });

  cout << "SocketShell up and running, and waiting for connections." << endl;

  while (true)
  {
    s1.update();
  }

  return 0;
}

#endif