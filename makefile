CXX=g++
CXXFLAGS=-std=c++17

all: referee # socketshell

# socketshell: SocketShell.* Scheduler.* Session.* strutils.* 
# 	$(CXX) -o $@ $^ -pthread

referee: Referee.cpp SocketShell.* Scheduler.* Session.* strutils.*
	$(CXX) -o $@ $^ -pthread
