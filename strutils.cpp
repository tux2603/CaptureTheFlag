#include "strutils.h"

#include <stdlib.h> 
#include <string.h> 
#include <string>
#include <ctype.h>

// Counts the number of words in str, c++ way
int countWords(std::string str) {
  // Stores whether or not the last character seen was a space
  bool lastCharSpace = true;

  // Stores the number of words that have been seen so far
  int numWords = 0;

  // Itereate over every character in the string
  for(int i = 0; i < str.length(); i++) {
    // If this character is at the begining of a word, increment the number of words
    if(!isspace(str[i]) && lastCharSpace) numWords++;
    lastCharSpace = isspace(str[i]);
  }

  return numWords;
}
// Returns an array holding all the words in str, c++ way
std::string *getWords(std::string str) {
  // Get the number of words in the string
  int numWords = countWords(str);

  // Declare an array to hold the list of words
  std::string *words = new std::string[numWords];

  // Create an index variable that will be used to search through the string
  int j = 0;

  // Add each word to the array, one bt one
  for (int i = 0; i < numWords; i++) {

    // Skip any whitespace that may come before the word
    while(isspace(str[j])) j++;

    // Create a new string to hold the word
    std::string word = "";

    // Store all the characters in the word into the string
    while(!isspace(str[j]) && j != str.length()) {
      word += str[j];
      j++;
    }

    // Put the word in the proper location in the array
    words[i] = word;
  }

  return words;
}