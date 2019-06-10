#include <iostream>
#include <thread>

using namespace std;

int main() {
  cout << "This computer can run " << thread::hardware_concurrency() << " threads at a time" << endl;  
  return 0;
}