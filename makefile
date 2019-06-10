CXX=g++
CXXFLAGS=-std=c++17

all: socketshell

# sock: sock.cpp strutils.cpp strutils.h
# 	$(CXX) -o sock sock.cpp strutils.cpp

# dummyshell: DummyShell.* strutils.*
# 	$(CXX) -o dummyshell DummyShell.cpp strutils.cpp

socketshell: SocketShell.* Scheduler.* Session.* DummyShell.* strutils.* 
	$(CXX) -o $@ $^ -pthread
