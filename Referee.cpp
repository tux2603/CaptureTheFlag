#include <cstring>
#include <ctime>
#include <functional>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <vector>

#include "GameComponents.h"
#include "MapGenerator.h"
#include "Session.h"
#include "SocketShell.h"

#define MUST_SIGN_IN "You must be signed in to access that feature"

using namespace std;

string joinArray(string[], int, int);
void trimPlayers(SocketShell *);

string getPlayerLocation(int, string[], SocketShell *, Session *);
string listCommands(int, string[], SocketShell *, Session *);
string listPlayers(int, string[], SocketShell *, Session *);
string listTeams(int, string[], SocketShell *, Session *);
string movePlayer(int, string[], SocketShell *, Session *);
string playerLook(int, string[], SocketShell *, Session *);
string playerSignIn(int, string[], SocketShell *, Session *);
string playerSignOut(int, string[], SocketShell *, Session *);
string setPlayerNickname(int, string[], SocketShell *, Session *);
string setPlayerTeam(int, string[], SocketShell *, Session *);

int port = 8042;
int numTeams = 2;

int mapWidth = 24;
int mapHeight = 24;
TerrainMap gameMap = TerrainMap();

/// Map from player session ID to player object, stores all players in this game
map<int, Player *> players;
string *teamNames = new string[numTeams];

int main(int argc, char **argv)
{
  srand(time(0));

  // If there are command line arguments to be parsed, then parse them
  if (argc > 0)
  {
    for (int i = 0; i < argc; i++)
    {

      // -h option specifies the height of the map
      if (i < argc - 1 && !strcmp(argv[i], (char *)"-h"))
        mapHeight = atoi(argv[i + 1]);

      // -p option specifies what port the server should listen on
      else if (i < argc - 1 && !strcmp(argv[i], (char *)"-p"))
        port = atoi(argv[i + 1]);

      else if (i < argc - 1 && !strcmp(argv[i], (char *)"-t"))
      {
        numTeams = atoi(argv[i + 1]);
        teamNames = new string[numTeams];
      }

      // -w option specifies the width of the map
      else if (i < argc - 1 && !strcmp(argv[i], (char *)"-w"))
        mapWidth = atoi(argv[i + 1]);
    }
  }

  SocketShell gameShell(port);

  gameShell.addCommand("getLocation", getPlayerLocation);
  gameShell.addCommand("help", listCommands);
  gameShell.addCommand("joinTeam", setPlayerTeam);
  gameShell.addCommand("listPlayers", listPlayers);
  gameShell.addCommand("listTeams", listTeams);
  gameShell.addCommand("look", playerLook);
  gameShell.addCommand("move", movePlayer);
  gameShell.addCommand("setNickname", setPlayerNickname);
  gameShell.addCommand("signIn", playerSignIn);
  gameShell.addCommand("signOut", playerSignOut);

  // Initialize the map with the parameters taken from the command prompt
  TerrainMap newMap = TerrainMap(mapWidth, mapHeight, numTeams);
  gameMap = newMap;
  cout << "Created map. Width is " << gameMap.getWidth() << ", height is " << gameMap.getHeight() << endl;
  PolygonMapGenerator terrainGenerator(0.02, 0.1);
  SmartWedgeTerritoryAllocator territoryAllocator(0.1);
  cout << "Created terrain generator" << endl;
  terrainGenerator.fillMap(gameMap);
  territoryAllocator.fillMap(gameMap);
  cout << "Filled map" << endl;

  cout << "The game will be played on the map pictured below:\n"
       << gameMap.ansi() << endl;

  cout << "Server started on local port " << to_string(gameShell.getPort()) << endl;

  while (true)
  {
    trimPlayers(&gameShell);
    gameShell.update();
  }

  return 0;
}

string joinArray(string sections[], int start, int end)
{
  string joined = "";

  for (int i = start; i < end; ++i)
  {
    joined += sections[i];
    if (i < end - 1)
      joined += " ";
  }

  return joined;
}

void trimPlayers(SocketShell *gameShell)
{
  // Vector to store what players need to be removes
  vector<int> playersIDsToRemove;

  // Find all players that have closed their session without signing out
  for (const pair<int, Player *> &i : players)
  {
    if (gameShell->getSessionByID(i.first) == NULL)
      playersIDsToRemove.push_back(i.first);
  }

  for (int id : playersIDsToRemove)
  {
    // ! DESTROY THE PLAYER!!!!!!
    delete players[id];

    // ! ERASE ALL MEMORY OF PLAYER!!!!!!!
    players.erase(id);
  }

  // Well, that was violent.
}

/// One line comment... \\\
  on two lines!!!

// ###########################################################################
// #####                   GAME SHELL COMMAND FUNCTIONS                  #####
// ###########################################################################

string getPlayerLocation(int argc, string argv[], SocketShell *gameShell, Session *playerSession)
{
  // First check to make sure that the player is logged in
  if (players.count(playerSession->getID()) > 0)
  {

    // If no arguments were given
    if (argc == 1)
    {
      Player *p = players[playerSession->getID()];
      return to_string(p->getX()) + " " + to_string(p->getY());
    }

    // If an argument was given... we don't know about it!
    else
    {
      return "Unknown option " + argv[1];
    }
  }

  else
  {
    return MUST_SIGN_IN;
  }
}

string listCommands(int argc, string argv[], SocketShell *gameShell, Session *playerSession)
{
  string message = "The following commands are available: ";

  set<string> commands = gameShell->listCommands();

  for (string i : commands)
  {
    message += i + " ";
  }

  if (argc >= 5 && !argv[1].compare("help") && !argv[2].compare("help") && !argv[3].compare("help") && !argv[4].compare("help"))
  {
    message = "\nA friendly message from the local neighborhood cow:\n";
    message += " _________________\n";
    message += "< Enough of that! >\n";
    message += " ----------------- \n";
    message += "        \\   ^__^                  \n";
    message += "         \\  (oo)\\_______          \n";
    message += "            (__)\\       )\\/\\      \n";
    message += "                ||----w |           \n";
    message += "                ||     ||           \n";
  }

  return message;
}

string listPlayers(int argc, string argv[], SocketShell *gameShell, Session *playerSession)
{
  trimPlayers(gameShell);
  string playersList = "";

  for (const pair<int, Player *> &i : players)
  {
    playersList += "Player " + to_string(i.first);

    if (gameShell->getSessionByID(i.first)->getName() != "")
      playersList += " (" + gameShell->getSessionByID(i.first)->getName() + ")";

    if (i.second->getTeam() >= 0)
    {
      playersList += " -- Team " + to_string(i.second->getTeam());

      if (teamNames[i.second->getTeam()] != "")
        playersList += " (" + teamNames[i.second->getTeam()] + ")";
    }

    else
      playersList += " -- No team selected";

    playersList += "\n";
  }

  return playersList;
}

string listTeams(int argc, string argv[], SocketShell *gameShell, Session *playerSession)
{
  string teamList = "";

  for (int i = 0; i < numTeams; ++i)
  {
    // TODO Team alias names...
    teamList += "Team " + to_string(i) + ":  ";
    if (teamNames[i] != "")
      teamList += teamNames[i] + "\n";
    else
      teamList += "<NO NAME YET>\n";
  }

  return teamList;
}

string movePlayer(int argc, string argv[], SocketShell *gameShell, Session *playerSession)
{
  if (players.count(playerSession->getID()) == 0)
  {
    return MUST_SIGN_IN;
  }

  else if (argc == 1)
  {
    return "You must specify a direction in which to move";
  }

  // TODO Incur movement cost and check that you can move in that direction

  else
  {
    if (argv[1] == "north" || argv[1] == "n" || argv[1] == "up" || argv[1] == "u")
    {
      players[playerSession->getID()]->move(Direction::North);
      return "Moved north";
    }

    else if (argv[1] == "south" || argv[1] == "s" || argv[1] == "down" || argv[1] == "d")
    {
      players[playerSession->getID()]->move(Direction::South);
      return "Moved south";
    }

    else if (argv[1] == "east" || argv[1] == "e" || argv[1] == "right" || argv[1] == "r")
    {
      players[playerSession->getID()]->move(Direction::East);
      return "Moved east";
    }

    else if (argv[1] == "west" || argv[1] == "w" || argv[1] == "left" || argv[1] == "l")
    {
      players[playerSession->getID()]->move(Direction::West);
      return "Moved west";
    }

    else
    {
      return "Unknown direction " + argv[1];
    }
  }
}

string playerLook(int argc, string argv[], SocketShell *gameShell, Session *playerSession)
{
  if (players.count(playerSession->getID()) == 0)
    return MUST_SIGN_IN;

  string viewString = "";

  // TODO add ability to return info in different formats
  TerrainMap *view = gameMap.getViewFrom(players[playerSession->getID()]->getX(), players[playerSession->getID()]->getY());

  // By default, return the view in ANSI format
  if (argc < 2 || argv[1] == "ansi")
  {
    viewString = "";

    for (int y = view->getHeight() - 1; y >= 0; --y)
    {
      for (int x = 0; x < view->getWidth(); ++x)
      {
        int viewOffsetX = players[playerSession->getID()]->getX() - view->getWidth() / 2;
        int viewOffsetY = players[playerSession->getID()]->getY() - view->getHeight() / 2;

        char ownerChar = ' ';

        if(view->getTerritoryAt(x, y) != players[playerSession->getID()]->getTeam())
          ownerChar = (char)(view->getTerritoryAt(x, y) + 97);
        

        if (view->getTerrainAt(x, y) == TerrainType::TerraIncognita) {
          if (y == view->getHeight() / 2 && x == view->getWidth() / 2)
            ownerChar = '+';
          else if (y == view->getHeight() / 2)
            ownerChar = '-';
          else if (x % 2 == 0 && x == view->getWidth() / 2)
            ownerChar = '|'; 
        }

        char playerChar = ' ';

        // TODO Check for location of other players!
        for(const pair<int, Player*> i : players) {
          if (i.first != playerSession->getID()) {
            if(view->getTerrainAt(x, y) != TerrainType::TerraIncognita) {
              if(i.second->getX() == x + viewOffsetX && i.second->getY() == y + viewOffsetY) {
                if(i.second->getTeam() == players[playerSession->getID()]->getTeam())
                  playerChar = '$';
                else 
                  playerChar = (char)(i.second->getTeam() + 65);
              }
            }
          }
        }

        if(y == view->getHeight() / 2 && x == view->getWidth() / 2)
          playerChar = '#';

        viewString += terrainToANSI(view->getTerrainAt(x, y)) + ownerChar + playerChar;

      }

      viewString += "\033[0m\n";
    }
  }

  else
  {
    viewString = "Unrecognized format `" + argv[1] + "'";
  }

  delete view;

  return viewString;
}

string playerSignIn(int argc, string argv[], SocketShell *gameShell, Session *playerSession)
{
  cout << "A player on session " << playerSession->getID() << " is signing in" << endl;

  // TODO Set players location and team

  players.insert(pair<int, Player *>(playerSession->getID(), new Player()));

  return "You have signed in";
}

string playerSignOut(int argc, string argv[], SocketShell *gameShell, Session *playerSession)
{
  if (players.count(playerSession->getID()) == 0)
  {
    return MUST_SIGN_IN;
  }

  else
  {
    cout << "The player on session " << playerSession->getID() << " is signing out" << endl;

    delete players[playerSession->getID()];
    players.erase(playerSession->getID());

    return "You have signed out";
  }
}

string setPlayerNickname(int argc, string argv[], SocketShell *gameShell, Session *playerSession)
{
  string nickname = joinArray(argv, 1, argc);

  playerSession->setName(nickname);

  return "Nickname has been set to " + nickname;
}

string setPlayerTeam(int argc, string argv[], SocketShell *gameShell, Session *playerSession)
{
  // Make sure that the player is signed in
  if (players.count(playerSession->getID()) <= 0)
    return "You must sign in before joining a team";

  else if (argc <= 1)
    return "At least one argument is required";

  // If there was only one argument and it was an integer
  else if (argc == 2 && argv[1].find_first_not_of("0123456789"))
  {
    // parse the integer given
    int teamNum = atoi(argv[1].c_str());

    cout << "Player " << playerSession->getID() << " requested to join team number " << teamNum << endl;

    // Check to make sure that that was a valid team number
    if (teamNum >= numTeams)
      return "The requested team does not exist. Type `listTeams' to get a list of teams.";

    // If the team number passed all checks, assign it to the player that requested it
    players[playerSession->getID()]->setTeam(teamNum);
  }

  // A string team name was specified
  else
  {
    string teamName = joinArray(argv, 1, argc);

    cout << "Player " << playerSession->getID() << " requested to join team " << teamName << endl;

    // Check to see if any existing teams have that name
    int teamNum = -1;

    for (int i = 0; i < numTeams; ++i)
    {
      if (teamNames[i] == teamName)
        teamNum = i;
    }

    // If it is a preexisting team, join it
    if (teamNum >= 0)
      players[playerSession->getID()]->setTeam(teamNum);

    else
    {
      // Check to see if there is an unnamed team
      for (int i = 0; i < numTeams; ++i)
      {
        if (teamNames[i] == "")
        {
          teamNum = i;
          break;
        }
      }

      // If there was an unnamed team, name the team the requested name and then join it
      if (teamNum >= 0)
      {
        cout << "Setting the name of team number " << teamNum << " to " << teamName << endl;
        teamNames[teamNum] = teamName;
        players[playerSession->getID()]->setTeam(teamNum);
      }

      // Else there was not and can not be a team with that name
      else
        return "The requested team does not exist. Type `listTeams' to get a list of teams.";
    }
  }

  return "Joined team " + joinArray(argv, 1, argc);
}