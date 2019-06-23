CXX=g++
CXXFLAGS=-std=c++17

all: referee gcTest# socketshell

# socketshell: SocketShell.* Scheduler.* Session.* strutils.* 
# 	$(CXX) -o $@ $^ -pthread

referee: Referee.cpp SocketShell.cpp Scheduler.cpp Session.cpp strutils.cpp GameComponents.cpp 
	$(CXX) -o $@ $^ -pthread

gcTest: gcTest.cpp GameComponents.cpp MapGenerator.cpp
	$(CXX) -o $@ $^
