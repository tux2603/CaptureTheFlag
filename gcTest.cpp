#include <cassert>
#include <iostream>
#include "GameComponents.h"
#include "MapGenerator.h"

using namespace std;

int main() {
  srand(time(0));

  cout << " +++++ BEGIN DIRECTION TESTING +++++ " << endl;

  // Test direction name aliases
  assert(Direction::North == Direction::Up);
  assert(Direction::South == Direction::Down);
  assert(Direction::East == Direction::Right);
  assert(Direction::West == Direction::Left);

  // Test compounding of simple directions to secondary directions
  assert(Direction::NorthWest == (Direction::North | Direction::West));
  assert(Direction::NorthEast == (Direction::North | Direction::East));
  assert(Direction::SouthWest == (Direction::South | Direction::West));
  assert(Direction::SouthEast == (Direction::South | Direction::East));

  // Partial test for uniqueness of directions
  assert(Direction::South != Direction::North);
  assert(Direction::NorthWest != (Direction::South | Direction::East));

  cout << " +++++ BEGIN PLAYER TESTING +++++ " << endl;

  Player p1;
  Player p2(1, 2);

  // Test proper initialization of location values
  assert(p1.getX() == 0);
  assert(p1.getY() == 0);
  assert(p2.getX() == 1);
  assert(p2.getY() == 2);
  
  // Test for reassignment of location values
  p1.setX(5);
  p1.setY(42);
  assert(p1.getX() == 5);
  assert(p1.getY() == 42);

  // Test for moving in the right directions...
  p2.move(Direction::North);
  assert(p2.getX() == 1);
  assert(p2.getY() == 3);

  p2.move(Direction::South);
  assert(p2.getX() == 1);
  assert(p2.getY() == 2);

  p2.move(Direction::East);
  assert(p2.getX() == 2);
  assert(p2.getY() == 2);

  p2.move(Direction::West);
  assert(p2.getX() == 1);
  assert(p2.getY() == 2);

  p2.move(Direction::NorthWest);
  assert(p2.getX() == 0);
  assert(p2.getY() == 3);

  p2.move(Direction::SouthWest);
  assert(p2.getX() == -1);
  assert(p2.getY() == 2);

  cout << " +++++ BEGIN MAP TESTING +++++ " << endl;

  Map m1;

  assert(m1.getTerrainAt(0,0) == TerrainType::Field);
  assert(m1.getTerrainAt(14, 14) == TerrainType::Field);
  assert(m1.getTerrainAt(0, 14) == TerrainType::Prison);
  assert(m1.getTerrainAt(7,3) == TerrainType::Border);
  assert(m1.getTerrainAt(-1, -1) == TerrainType::TerraIncognita);

  assert(m1.getTerritoryAt(0, 0) == 0);
  assert(m1.getTerritoryAt(7, 7) == -1);
  assert(m1.getTerritoryAt(14, 14) == 1);

  assert(m1.getNumTeams() == 2);

  cout << m1.ansi() << endl;

  int m2Width = 4;
  int m2Height = 4;
  int m2Teams = 2;

  Map m2(m2Width, m2Height, m2Teams);

  PolygonMapGenerator gen(0.25, 0.5, 0.9);

  gen.fillMap(m2);

  assert(m2.getWidth() == m2Width);
  assert(m2.getHeight() == m2Height);
  assert(m2.getNumTeams() == m2Teams);

  cout << m2.ansi() << endl;

  cout << " +++++ ALL TESTS SUCCESSFUL +++++ " << endl;

  return 0;
}