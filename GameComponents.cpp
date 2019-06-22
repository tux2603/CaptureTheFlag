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

Player::Player() : Player::Player(0, 0) {}
Player::Player(int x, int y) : x(x), y(y) {}

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

Map::Map() : width(15), height(15), numTeams(2)
{
  tiles = new TerrainType *[height];
  territoryMask = new int *[height];

  for (int y = 0; y < height; y++)
  {
    tiles[y] = new TerrainType[width];
    territoryMask[y] = new int[width];

    for (int x = 0; x < width; x++)
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

Map::Map(int width, int height) : Map::Map(width, height, 2) {}

Map::Map(int width, int height, int numTeams) : width(width), height(height), numTeams(numTeams)
{
  // Allocate memory for the data arrays
  tiles = new TerrainType *[height];
  territoryMask = new int *[height];

  for (int y = 0; y < height; y++)
  {
    tiles[y] = new TerrainType[width];
    territoryMask[y] = new int[width];
  }
}

Map::~Map()
{
  // Release the memory for holding the map
  for (int y = 0; y < height; y++) {
    delete[] tiles[y];
    delete[] territoryMask[y];
  }
  delete[] tiles;
  delete[] territoryMask;
}

int Map::getWidth()
{
  return width;
}

int Map::getHeight()
{
  return height;
}

int Map::getNumTeams()
{
  return numTeams;
}

TerrainType Map::getTerrainAt(int x, int y)
{
  if (x < 0 || y < 0 || x >= width || y >= height)
    return TerrainType::TerraIncognita;
  else
    return tiles[y][x];
}

int Map::getTerritoryAt(int x, int y)
{ 
  if (x < 0 || y < 0 || x >= width || y >= height)
    return -1;
  else
    return territoryMask[y][x];
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
string Map::ansi()
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
