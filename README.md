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
  test bed called `gcTest` that will run some checks of various game objects and a referee program 
  called `referee`.

# `referee` Program

## Command Line Arguments

 - `-p` Sets the port that the referee will listen for connections on. __Default:__ 8042
 - `-t` Sets the number of teams in the game. __Default:__ 2

## Player Commands

The commands that are available to be run by the players are as follows. Commands that
  require a player to be signed in are marked by a star (*).

 - *`getLocation` Gets the x/y coordinates of the player. Ability to select output formatting (eg, tuple, JSON, plain text) will 
    be added at a later date
 - `help` Prints a list of available commands
 - *`joinTeam [team number | team name]` Joins a specified team. If one integer argument is given, the referee will try to 
    add the player to that team. Else, the referee will try to add the player to a team whose name is all of the arguments 
    concatenated together. A maximum number of players per team will be enforced at a later date.
 - `listPlayers` Gets a list of the ID, nicknames, and team of every player that is currently signed in.
 - `listTeams` Gets a list of the ID and name of all teams in the game.
 - *`move [direction]` Moves the player in the specified direction, if allowed. Available directions are north, south, east,
    west, up, down, right, and left. The initial letter of each of these options is also understood.
 - `setNickname` Sets the nickname of the player as it will be displayed to other players. Filtering of names will be
    added at a later date.
 - `signIn` Signs the player in. A maximum number of players will be enforced at a later date.
 - *`signOut` Signs the player out.