# CaptureTheFlag

Socket based capture the flag game, inspired by project brainstorming for CS-3900 Practical Software Development.

This project is being written partially for fun, and partially to learn c++ and make files.

# Overview

The capture the flag program will work by starting a "referee" program listening on a TCP port. This referee will
  handle all of the players and teams that are participating in a given game, and take requests from the players:
  eg, `look`, `go north`, `go south`, `say <message>`, etc. To play, the competitors will connect to the socket and
  issue streams of commands to get interact with the game map and other players. Because the commands and the return
  values are both human and machine readable, the game can either be played in person or using AIs written in a language
  that is capable of connecting to TCP sockets.

# Building/Compiling

This should simply be a matter of running `make` in the project directory. Currently, this builds a 
  test bed called `socketshell` that will start a simplified version of the referee program that will
  listen on port 8042, log all requests that come in over the socket, and execute them. 

Currently, the following commands are available:
 - `wall [_message_]` - send `[_message_]` to all connected sessions

So far, this has only been tested on a system running Linux Mint 19 and a virtual machine running Lubuntu 19.04. Most Linux distros and
  Mac _should_ work, but Windows<sup>™©®☣§℠℗</sup> is not guaranteed to work, since various UNIX system calls are used.


