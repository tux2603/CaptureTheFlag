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

// TODO ################################################################# TODO
// TODO  Is it really necessary to have this as a separate implementation TODO
// TODO                    from the SocketShell class?                    TODO
// TODO ################################################################# TODO

DummyShell::DummyShell() : shellHasExited(false), prompt("dummyshell$ ") {}

DummyShell::DummyShell(string prompt) : shellHasExited(false), prompt(prompt) {}

string DummyShell::getPrompt()
{
  return prompt;
}

void DummyShell::setPrompt(string p)
{
  prompt = p;
}

void DummyShell::addCommand(string name, function<string(int, string[], DummyShell *)> lambda)
{
  commandDictionary.insert(pair<string, function<string(int, string[], DummyShell *)>>(name, lambda));
}

string *DummyShell::listCommands()
{
  string *commands = new string[commandDictionary.size()];

  int i = 0;

  for (auto const &commandPair : commandDictionary)
  {
    commands[i] = commandPair.first;
    ++i;
  }

  return commands;
}

int DummyShell::numCommands()
{
  return commandDictionary.size();
}

void DummyShell::exit()
{
  shellHasExited = true;
}

bool DummyShell::hasExited()
{
  return shellHasExited;
}

string DummyShell::exec(string str)
{
  if (shellHasExited)
  {
    return "Cut that out!";
  }

  // Tokenize the input
  int numWords = countWords(str);
  string *words = getWords(str);
  string message = "";

  if (numWords > 0)
  {
    if (commandDictionary.count(words[0]))
    {
      message = commandDictionary.at(words[0])(numWords, words, this);
    }

    else
    {
      message = words[0] + ": command not found. Type `help' for a list of available commands";
    }
  }

  else
  {
    message = "Oh come on, ya gotta say \033[3msomething\033[0m";
  }

  return message;
}