#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <iostream>
#include "strutils.h"

#define PORT 13001

using namespace std;

int main(int argc, char const *argv[]) {

  int serverFD, newSocket, charsRead;
  struct sockaddr_in address;
  int opt = 1;
  int addrLength = sizeof(address);
  char buffer[1024] = {0};
  string message;

  char* prompt = (char*)"\ndummyshell$ ";

  // Try to make a file descriptor that will read from the socket
  if( (serverFD = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("Could not create socket file descriptor");
    exit(1);
  }

  // Set socket options
  if (setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    perror("setsockopt failed");
    exit(1);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // Bind the socket to the defined port
  if (bind(serverFD, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Could not bind socket");
    exit(1);
  }

  // Start the socket listening
  if (listen(serverFD, 32)) {
    perror("A problem happened when the computer tried to listen...");
    exit(1);
  }

  // Start accepting messages
  if ( (newSocket = accept(serverFD, (struct sockaddr *)&address, (socklen_t*)&addrLength)) < 0) {
    perror("Socket could not accept messages");
    exit(1);
  }


  cout << "We got this far at least..." << endl;

  string testString = "Twas brillig and the slithy toves";
  int numTestWords = countWords(testString);
  cout << "Let's try some stuff, there are " << numTestWords << " words in '" << testString << "', and they are:" << endl;

  string *testWords = getWords(testString);

  for(int i = 0; i < numTestWords; i++) {
    cout << "\t" << testWords[i] << endl;
  }

  delete [] testWords;


  do {

    send(newSocket, prompt, strlen(prompt), 0);
    // Read a value from the socket
    charsRead = read( newSocket, buffer, 1024);


    // Make sure the buffer is properly terminated
    if(charsRead < 1024) buffer[charsRead] = 0;
    else buffer[1023] = 0;

    // Convert the buffe to a string
    string socketString = buffer;

    cout << socketString << endl;

    int numWords = countWords(socketString);
    string *words = getWords(socketString);

    if(numWords > 0) {
      cout << "The first word is " << words[0] << ", and there are " << numWords << " words" << endl;

      cout << "The words are:" << endl;

      for(int i = 0; i < numWords; i++) {
        cout << "\t" << words[i] << endl;
      }

      if(words[0] == "look") {
        message = "~~.  \n~..TT\n..X.T\n....T\n+..TT";
      }

      else {
        message = "The computer does not know what '" + words[0] + "' is. To figure out what the computer does know, say 'help'";
      }
    }

    else {
      message = "No words were given... I don't know what to do about that";
    }

    // Write a message to the socket
    send(newSocket, message.c_str(), message.length(), 0);
    
    delete [] words;

  } while (strcmp(buffer, "quit") != 0);

  return 0;
}
