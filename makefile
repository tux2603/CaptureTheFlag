CXX=g++
CXXFLAGS=-std=c++17

all: referee gcTest# socketshell

# socketshell: SocketShell.* Scheduler.* Session.* strutils.* 
# 	$(CXX) -o $@ $^ -pthread

referee: Referee.cpp SocketShell.* Scheduler.* Session.* strutils.*
	$(CXX) -o $@ $^ -pthread

gcTest: gcTest.cpp GameComponents.* MapGenerator.*
	$(CXX) -o $@ $^
