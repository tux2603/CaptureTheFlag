/*
  This class handles a single connection to a socket, with its main purpose to provide
  an easy interface to read and write strings to the connection
*/

#include <cstring>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h> 
#include "Session.h"

using namespace std;

int Session::uniqueID = 1;

Session::Session(int sessionFD, string prompt) : sessionFD(sessionFD), isClosed(false), prompt(prompt)
{
  id = uniqueID++;

  cout << "Creating new session (ID=" << id << ")" << endl;

  sendPrompt();
}

Session::Session(const Session &toCopy) {
  id = toCopy.id;
  isClosed = toCopy.isClosed;
  prompt = toCopy.prompt;
  sessionFD = toCopy.sessionFD;
  linesIn = toCopy.linesIn;
}

// On destruction of the session object
Session::~Session()
{
  // Clear all buffered lines...
  while(!linesIn.empty()) {
    linesIn.pop();
  }
  
  // Send a message to whatever is connected to the socket so that it knows what happened
  char *message = (char *)"Your session was destroyed\n";
  send(sessionFD, message, strlen(message), 0);
  

  // Close the file descriptor associated with the session
  close(sessionFD);

  // Set the closed variable of the
  isClosed = true;

  // Print out debugging message
  cout << "Session " << sessionFD << " destroyed!" << endl;
}

int Session::getID() { return id; }

int Session::getSessionFD() { return sessionFD; }

bool Session::getIsClosed() { return isClosed; }
bool Session::getIsDone() {return isClosed && linesIn.empty(); }

string Session::getName() { return name; }
void Session::setName(string name) { this->name = name; }

string Session::getPrompt() {
  return prompt;
}

void Session::setPrompt(string prompt) {
  this->prompt = prompt;
}

int Session::numQueuedLines()
{
  return linesIn.size();
}

string Session::readLine()
{
  string line = "";

  // If there is already a line in queue, then there is no need to deal with the socket
  if (linesIn.size() >= 1)
  {
    line = linesIn.front();
    linesIn.pop();
  }

  // If there are no lines queued, new lines will have to be read from the socket
  else if (!isClosed);
  {
    int charsRead = 0;
    char buffer[BUFFER_SIZE] = {0};

    // Read a set of characters form the session, waiting if none are available
    charsRead = read(sessionFD, buffer, BUFFER_SIZE);

    // Check to make sure that the number of characters read is inside the acceptable range
    // TODO There is probably a better way to check that the socket is still open
    if (charsRead > 0 && charsRead <= BUFFER_SIZE)
    {
      // Terminate the string of character read with the null character
      if (charsRead < BUFFER_SIZE)
        buffer[charsRead] = 0;
      else
        buffer[BUFFER_SIZE - 1] = 0;

      // Since multiple lines could theoretically come in at once, the input
      //  will have t be split on new line characters

      // Convert the incoming request to a string buffer in preparation for tokenization
      stringstream request(buffer);

      // String to temporarily store all individual lines from the request
      string requestLine;

      // Iterate over every line in the request string
      while (getline(request, requestLine))
      {
        linesIn.push(requestLine);
      }

      // Add the new request to the queue
      //linesIn.push(request);
    }

    // If the number of characters is outside of the acceptable range, force the socket to close
    else
    {
      close(sessionFD);
      isClosed = true;
    }
  }

  // Again check to see if we can pop something off the queue
  if (linesIn.size() > 0)
  {
    line = linesIn.front();
    linesIn.pop();
  }

  return line;
}

void Session::sendMessage(string message)
{
  send(sessionFD, message.c_str(), message.length(), 0);
}

void Session::sendPrompt() {
  send(sessionFD, prompt.c_str(), prompt.length(), 0);
}