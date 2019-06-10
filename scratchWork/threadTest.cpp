#include <chrono>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

int main() {
  bool hasExited = false;

  thread t([](bool *hasExited) {
    cout << "Type 'exit' to exit" << endl;
    string usrin = "";
    while(usrin != "exit") {
      getline(cin, usrin);
      cout << "You said '" << usrin << "'. Fascinating..." << endl;
    }
    this_thread::sleep_for(chrono::seconds(1));

    cout << "Good bye" << endl;

    *hasExited = true;
  }, &hasExited);

  for(int i = 100; i >= 0; --i) {
    if(!hasExited) {
      this_thread::sleep_for(chrono::seconds(1));
    }

    else {
      this_thread::sleep_for(chrono::milliseconds(10));
    }

    cout << i << endl;
  }

  this_thread::sleep_for(chrono::seconds(2));
  cout << "\033[41;1m";

  for(int i = 0; i < 1000; ++i) {
    cout << "  BOOOM!!!!!  " << flush;
    this_thread::sleep_for(chrono::milliseconds(2));
  }
  cout << "\033[0m" << endl;
  t.join();
  return 0;
}