#include <iostream>
#include <math.h>
#include <string>
#include "GameComponents.h"
#include "MapGenerator.h"

using namespace std;

// ###########################################################################
// #####                       GENERAL ALGORITHMS                        #####
// ###########################################################################

double getToroidalDistance(double x1, double y1, double x2, double y2, double width, double height)
{
  double dx = x1 - x2;
  double dy = y1 - y2;

  if (dx > width / 2)
    dx -= width;
  else if (dx < -width / 2)
    dx += width;

  if (dy > height / 2)
    dy -= height;
  else if (dy < -height / 2)
    dy += height;

  return sqrt(dx * dx + dy * dy);
}

/*
  Utility function that gives an ANSI color code for each terrain type
    - Black         - Terra Incognita
    - Medium Green  - Field
    - Light Green   - Hills
    - Dark Green    - Forest
    - Brown         - Brambles
    - Gray          - Border
    - Dark Blue     - Water
    - Tan           - Path
    - Red           - Prison
*/
string terrainToANSI(TerrainType t)
{
  //Set map color according to the terrain
  switch (t)
  {
  case TerrainType::TerraIncognita:
    return "\033[37;40m";

  case TerrainType::Field:
    return "\033[30;42m";

  case TerrainType::Forest:
    return "\033[37;48;2;0;85;0m";

  case TerrainType::Hills:
    return "\033[30;48;2;127;255;0m";

  case TerrainType::Brambles:
    return "\033[37;48;2;85;85;0m";

  case TerrainType::Water:
    return "\033[37;44m";

  case TerrainType::Border:
    return "\033[30;47m";

  case TerrainType::Path:
    return "\033[30;48;2;170;170;127m";

  case TerrainType::Prison:
    return "\033[37;41m";

  default:
    return "\033[31;1m";
  }
}

// ###########################################################################
// #####                  BEGIN PLAYER IMPLEMENTATIONS                   #####
// ###########################################################################

Player::Player() : Player::Player(0, 0, -1) {}
Player::Player(int x, int y) : Player::Player(x, y, -1) {}
Player::Player(int x, int y, int team) : x(x), y(y), team(team) {}

int Player::getTeam() { return team; }
void Player::setTeam(int team) { this->team = team; }

int Player::getX() { return x; }
void Player::setX(int x) { this->x = x; }

int Player::getY() { return y; }
void Player::setY(int y) { this->y = y; }

void Player::move(Direction direction)
{
  if (direction & Direction::North)
    ++y;
  if (direction & Direction::South)
    --y;
  if (direction & Direction::East)
    ++x;
  if (direction & Direction::West)
    --x;
}

// ###########################################################################
// #####                    BEGIN MAP IMPLEMENTATIONS                    #####
// ###########################################################################

TerrainMap::TerrainMap() : width(15), height(15), numTeams(2)
{
  tiles = new TerrainType *[height];
  territoryMask = new int *[height];

  for (int y = 0; y < height; ++y)
  {
    tiles[y] = new TerrainType[width];
    territoryMask[y] = new int[width];

    for (int x = 0; x < width; ++x)
    {
      tiles[y][x] = TerrainType::Field;

      if (x < width / 2)
      {
        territoryMask[y][x] = 0;
      }
      else if (x == width / 2)
      {
        territoryMask[y][x] = -1;
        tiles[y][x] = TerrainType::Border;
      }
      else
      {
        territoryMask[y][x] = 1;
      }
    }
  }

  tiles[0][width - 1] = TerrainType::Prison;
  tiles[height - 1][0] = TerrainType::Prison;
}

TerrainMap::TerrainMap(int width, int height) : TerrainMap::TerrainMap(width, height, 2) {}

TerrainMap::TerrainMap(int width, int height, int numTeams) : width(width), height(height), numTeams(numTeams)
{

  cout << "Called three argument constructor for terrain map" << endl;

  // Allocate memory for the data arrays
  tiles = new TerrainType *[height];
  territoryMask = new int *[height];

  for (int y = 0; y < height; ++y)
  {
    tiles[y] = new TerrainType[width];
    territoryMask[y] = new int[width];
  }
}

TerrainMap::TerrainMap(const TerrainMap &toCopy)
{
  width = toCopy.width;
  height = toCopy.height;
  numTeams = toCopy.numTeams;

  tiles = new TerrainType *[height];
  territoryMask = new int *[height];

  for (int y = 0; y < height; ++y)
  {
    tiles[y] = new TerrainType[width];
    territoryMask[y] = new int[width];

    for (int x = 0; x < width; ++x)
    {
      tiles[y][x] = toCopy.tiles[y][x];
      territoryMask[y][x] = territoryMask[y][x];
    }
  }

  cout << "Called copy constructor for terrain map" << endl;
}

TerrainMap::~TerrainMap()
{
  // Release the memory for holding the map
  for (int y = 0; y < height; y++)
  {
    delete[] tiles[y];
    delete[] territoryMask[y];
  }
  delete[] tiles;
  delete[] territoryMask;
}

int TerrainMap::getWidth()
{
  return width;
}

int TerrainMap::getHeight()
{
  return height;
}

int TerrainMap::getNumTeams()
{
  return numTeams;
}

TerrainType TerrainMap::getTerrainAt(int x, int y)
{
  if (x < 0 || y < 0 || x >= width || y >= height)
    return TerrainType::TerraIncognita;
  else
    return tiles[y][x];
}

int TerrainMap::getTerritoryAt(int x, int y)
{
  if (x < 0 || y < 0 || x >= width || y >= height)
    return -1;
  else
    return territoryMask[y][x];
}

TerrainMap *TerrainMap::getViewFrom(int x, int y)
{
  int maxDistance = 4;

  if (getTerrainAt(x, y) == TerrainType::Hills)
    maxDistance = 6;
  else if (getTerrainAt(x, y) == TerrainType::Forest)
    maxDistance = 3;

  TerrainMap *view = new TerrainMap(13, 13, numTeams);

  for (int dy = -6; dy <= 6; ++dy)
  {
    for (int dx = -6; dx <= 6; ++dx)
    {
      double distance = sqrt(dx * dx + dy * dy);

      if (distance > maxDistance || x + dx < 0 || x + dx > width - 1 || y + dy < 0 || y + dy > height - 1)
      {
        view->tiles[dy + 6][dx + 6] = TerrainType::TerraIncognita;
        view->territoryMask[dy + 6][dx + 6] = -1;
      }

      else
      {
        view->tiles[dy + 6][dx + 6] = getTerrainAt(x + dx, y + dy);
        view->territoryMask[dy + 6][dx + 6] = getTerritoryAt(x + dx, y + dy);
      }
    }
  }

  return view;
}

/*
  Utility function to create a ANSI compatible version of the map
  The colors used are:
    - Black         - Terra Incognita
    - Medium Green  - Field
    - Light Green   - Hills
    - Dark Green    - Forest
    - Brown         - Brambles
    - Gray          - Border
    - Dark Blue     - Water
    - Tan           - Path
    - Red           - Prison
*/
string TerrainMap::ansi()
{
  string map = "";

  for (int y = height - 1; y >= 0; --y)
  {
    for (int x = 0; x < width; ++x)
    {
      TerrainType t = getTerrainAt(x, y);

      //Set map color according to the terrain
      map += terrainToANSI(t);

      map += (char)(getTerritoryAt(x, y) + 97);
      map += " \033[0m";
    }

    map += "\n";
  }

  return map;
}
