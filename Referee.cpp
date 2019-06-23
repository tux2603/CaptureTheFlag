#include <cstring>
#include <ctime>
#include <functional>
#include <iostream>
#include <map>

#include "GameComponents.h"
#include "Session.h"
#include "SocketShell.h"

using namespace std;

string joinArray(string[], int, int);

string listPlayers(int, string[], SocketShell*, Session*);
string listTeams(int, string[], SocketShell*, Session*);
string playerSignIn(int, string[], SocketShell*, Session*);
string playerSignOut(int, string[], SocketShell*, Session*);
string setPlayerNickname(int, string[], SocketShell*, Session*);
string setPlayerTeam(int, string[], SocketShell*, Session*);

int port = 8042;
int numTeams = 2;

/// Map from player session ID to player object, stores all players in this game
map<int, Player*> players;
string* teamNames = new string[numTeams];


int main(int argc, char **argv) {
  // If there are command line arguments to be parsed, then parse them
  if(argc > 0) {
    for(int i = 0; i < argc; i++) {

      // -p option specifies what port the server should listen on
      if(i < argc - 1 && !strcmp(argv[i], (char *)"-p"))
        port = atoi(argv[i+1]);

      else if(i < argc - 1 && !strcmp(argv[i], (char *)"-t")) {
        numTeams = atoi(argv[i+1]);
        teamNames = new string[numTeams];
      }
    }
  }

  SocketShell gameShell(port);

  gameShell.addCommand("joinTeam", setPlayerTeam);
  gameShell.addCommand("listPlayers", listPlayers);
  gameShell.addCommand("listTeams", listTeams);
  gameShell.addCommand("setNickname", setPlayerNickname);
  gameShell.addCommand("signIn", playerSignIn);
  gameShell.addCommand("signOut", playerSignOut);

  cout << "Server started on local port " << to_string(gameShell.getPort()) << endl;


  while(true) {
    gameShell.update();
  }

  return 0;
}

string joinArray(string sections[], int start, int end) {
  string joined = "";

  for(int i = start; i < end; ++i) {
    joined += sections[i];
    if(i < end-1) joined += " ";
  }

  return joined;
}

string listTeams(int argc, string argv[], SocketShell *gameShell, Session *playerSession) {
  string teamList = "";

  for(int i = 0; i < numTeams; ++i) {
    // TODO Team alias names...
    teamList += "Team " + to_string(i) + ":  ";
    if(teamNames[i] != "") teamList += teamNames[i] + "\n";
    else teamList += "<NO NAME YET>\n";
  }

  return teamList;
}

string listPlayers(int argc, string argv[], SocketShell *gameShell, Session *playerSession) {
  string playersList = "";

  for(const pair<int, Player*>& i : players) {
    playersList += "Player " + to_string(i.first);

    if(gameShell->getSessionByID(i.first)->getName() != "") 
      playersList += " (" + gameShell->getSessionByID(i.first)->getName() + ")";

    if(i.second->getTeam() >= 0) {
      playersList += " -- Team " + to_string(i.second->getTeam());

      if(teamNames[i.second->getTeam()] != "")
        playersList += " (" + teamNames[i.second->getTeam()] + ")";
    }

    else 
      playersList += " -- No team selected";

    playersList += "\n";
  }

  return playersList;
}

string playerSignIn(int argc, string argv[], SocketShell *gameShell, Session *playerSession) {
  cout << "A player on session " << playerSession->getID() << " is signing in" << endl;

  // TODO Set players location and team

  players.insert(pair<int, Player*>(playerSession->getID(), new Player()));

  return "You have signed in";
}

string playerSignOut(int argc, string argv[], SocketShell *gameShell, Session *playerSession) {
  cout << "The player on session " << playerSession->getID() << " is signing out" << endl;

  players.erase(playerSession->getID());

  return "You have signed out";
}

string setPlayerNickname(int argc, string argv[], SocketShell *gameShell, Session *playerSession) {
  string nickname = joinArray(argv, 1, argc);

  playerSession->setName(nickname);

  return "Nickname has been set to " + nickname;
}

string setPlayerTeam(int argc, string argv[], SocketShell *gameShell, Session *playerSession) {
  // Make sure that the player is signed in
  if(players.count(playerSession->getID()) <= 0) 
    return "You must sign in before joining a team";

  else if(argc <= 1) 
    return "At least one argument is required";

  // If there was only one argument and it was an integer
  else if (argc == 2 && argv[1].find_first_not_of("0123456789")) {
    // parse the integer given
    int teamNum = atoi(argv[1].c_str());

    cout << "Player " << playerSession->getID() << " requested to join team number " << teamNum << endl;

    // Check to make sure that that was a valid team number
    if (teamNum >= numTeams) return "The requested team does not exist. Type `listTeams' to get a list of teams.";

    // If the team number passed all checks, assign it to the player that requesed it
    players[playerSession->getID()]->setTeam(teamNum);
  }

  // A string team name was specified
  else {
    string teamName = joinArray(argv, 1, argc);

    cout << "Player " << playerSession->getID() << " requested to join team " << teamName << endl; 

    // Check to see if any existing teams have that name
    int teamNum = -1;

    for(int i = 0; i < numTeams; ++i) {
      if(teamNames[i] == teamName) teamNum = i;
    }
    
    // If it is a preexisting team, join it
    if(teamNum >= 0)
      players[playerSession->getID()]->setTeam(teamNum);

    else {
      // Check to see if there is an unnamed team
      for(int i = 0; i < numTeams; ++i) {
        if(teamNames[i] == "") {
          teamNum = i;
          break;
        }
      }

      // If there was an unamed team, name the team the requested name and then join it
      if(teamNum >= 0) {
        cout << "Setting the name of team number " << teamNum << " to " << teamName << endl;
        teamNames[teamNum] = teamName;
        players[playerSession->getID()]->setTeam(teamNum);
      }

      // Else there was not and can not be a team with that name
      else return "The requested team does not exist. Type `listTeams' to get a list of teams.";
        
      
    }
    
  }

  return "Joined team " +  joinArray(argv, 1, argc);
}