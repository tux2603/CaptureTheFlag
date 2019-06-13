#include <cstring>
#include <ctime>
#include <iostream>
#include "SocketShell.h"

using namespace std;

int port = 8042;
string globalString = "Original value!";

int main(int argc, char **argv) {

  // If there are command line arguments to be parsed, then parse them
  if(argc > 0) {
    for(int i = 0; i < argc; i++) {

      // -p option specifies what port the server should listen on
      if(i < argc - 1 && !strcmp(argv[i], (char *)"-p"))
        port = atoi(argv[i+1]);
    }
  }

  SocketShell gameShell(port);

  cout << "Server started on local port " << to_string(gameShell.getPort()) << endl;

  gameShell.addCommand("logCalls", [](int argc, string argv[], SocketShell *shell, Session *callingSession) -> string {
    static string callLog = "Call log:\n";
    static int numCalls = 0;

    callLog += "\tCall " + to_string(numCalls++) + " from session ";

    if(callingSession->getName().length() > 0) {
      callLog += callingSession->getName() + " (" + to_string(callingSession->getID()) + ") was received on ";
    }

    else {
      callLog += to_string(callingSession->getID()) + ": ";
    }

    time_t currentTime = time(0);
    callLog += ctime(&currentTime);

    return callLog;
  });

  gameShell.addCommand("getGlobal", [](int argc, string argv[], SocketShell *shell, Session *callingSession) -> string {
    return globalString;
  });

  gameShell.addCommand("setGlobal", [](int argc, string argv[], SocketShell *shell, Session *callingSession) -> string {
    if (argc > 1) {
      globalString = argv[1];
      return "Global string set to " + argv[1];
    }

    return "setGlobal requires 1 argument"; 
  });

  while(true) {
    gameShell.update();
  }

  return 0;
}