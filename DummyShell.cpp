// This class creates a simple shell-ish thing
#include <cstring>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include "strutils.h"
#include "DummyShell.h"

using namespace std;






// ###########################################################################
// #####                 BEGIN DUMMYSHELL IMPLEMENTATIONS                #####
// ###########################################################################

DummyShell::DummyShell(): shellHasExited(false), prompt("dummyshell$ ") {}

DummyShell::DummyShell(string prompt): shellHasExited(false), prompt(prompt) {}

string DummyShell::getPrompt() {
  return prompt;
}

void DummyShell::setPrompt(string p) {
  prompt = p;
}

void DummyShell::addCommand(string name, function<string(int, string[], DummyShell*)> lambda) {
  commandDictionary.insert(pair<string, function<string(int, string[], DummyShell*)>>(name, lambda));
}

string* DummyShell::listCommands() {
  string* commands = new string[commandDictionary.size()];
  
  int i = 0;

  for(auto const &commandPair : commandDictionary ) { 
    commands[i] = commandPair.first;
    ++i;
  }

  return commands;
}

int DummyShell::numCommands() {
  return commandDictionary.size();
}

void DummyShell::exit() {
  shellHasExited = true;
}

bool DummyShell::hasExited() {
  return shellHasExited;
}

string DummyShell::exec(string str) {
  if(shellHasExited) {
    return "Cut that out!";
  }

  // Tokenze the input
  int numWords = countWords(str);
  string *words = getWords(str);
  string message = "";

  if (numWords > 0) {
    if(commandDictionary.count(words[0])) {
      message = commandDictionary.at(words[0])(numWords, words, this);
    }

    else {
      message = words[0] + ": command not found. Type `help' for a list of available commands";
    }
  }

  else {
    message = "Oh come on, ya gotta say \033[3msomething\033[0m";
  }

  return message;
}

// ###########################################################################
// #####                  END DUMMYSHELL IMPLEMENTATIONS                 #####
// ###########################################################################





// ###########################################################################
// #####                          BEGIN TESTBED                          #####
// ###########################################################################

// int main() {

//   DummyShell d1;
 

//   d1.addCommand("add", [](int argc, string argv[], DummyShell *shell) -> string {
//     int sum = 0;

//     for(int i = 1; i < argc; ++i) {
//       sum += stoi(argv[i]);
//     }

//     return to_string(sum);
//   });

//   d1.addCommand("exit", [](int argc, string argv[], DummyShell *shell) -> string {
//     shell->exit();
//     return "Shell has exited";
//   });

//   d1.addCommand("help", [](int argc, string argv[], DummyShell *shell) -> string {
//     string* commands = shell->listCommands();
//     int numCommands = shell->numCommands();

//     string returnString = "Available commands:\n    ";

//     int lineLength = 4;

//     for (int i = 0; i < numCommands; i++) {

//       //Wrap around at 80 character line length
//       // TODO allow this to be set via commmand arguments?
//       if(lineLength + commands[i].length() > 80) {
//         returnString += "\n    ";
//         lineLength = 4;
//       }

//       returnString += commands[i] + " ";
//       lineLength += commands[i].length() + 1;
//     }

//     return returnString;
//   });

//   d1.addCommand("setprompt", [](int argc, string argv[], DummyShell *shell) -> string {
//     if (argc <= 1) {
//       return "setPrompt takes at least 1 argument (" + to_string(argc-1) + " given)";
//     }

//     else {
//       string newPrompt = "";

//       for(int i = 1; i < argc; ++i) {
//         newPrompt += argv[i] + " ";
//       }

//       shell->setPrompt(newPrompt);
//     }

//     return "";
//   });

//   string usrin;

//   while(!d1.hasExited()) {
//     cout << "\n" << d1.getPrompt();
//     getline(cin, usrin);
//     cout << d1.exec(usrin);

//   }

//   return 0;
// }