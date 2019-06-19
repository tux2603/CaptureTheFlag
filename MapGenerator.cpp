#include <iostream>
#include <math.h>
#include <vector>
#include "GameComponents.h"
#include "MapGenerator.h"

using namespace std;



void MapGenerator::fillMap(Map &map) {
  this->fillRawMap(map.tiles, map.territoryMask, map.width, map.height, map.numTeams);
}

BarMapGenerator::BarMapGenerator() {}
BarMapGenerator::BarMapGenerator(const BarMapGenerator& toCopy) {}

void BarMapGenerator::fillRawMap(TerrainType **tiles, int **territoryMask, int width, int height, int numTeams) {
  // Get the spacing between territories. Assume that all of the territories are
  //  split along the length of the map
  double territorySpacing = width / (double)numTeams;

  // Fill the map with flat, boring terrain
  for (int x = 0; x < width; x++)
  {
    for (int y = 0; y < height; y++)
    {

      // Simple boring tile...
      tiles[y][x] = TerrainType::Field;

      // Assign the team at that tile
      territoryMask[y][x] = (int)floor(x / territorySpacing);

      // Check to see if this is on the boundary between two teams territory
      //  If so, it should be a stream
      if (abs(fmod(x, territorySpacing)) < 1.0)
      {
        tiles[y][x] = TerrainType::Stream;
        territoryMask[y][x] = -1;
      }
    }
  }
}


PolygonMapGenerator::PolygonMapGenerator(): PolygonMapGenerator(0.01, 0.1) {}

PolygonMapGenerator::PolygonMapGenerator(const PolygonMapGenerator& toCopy): 
  PolygonMapGenerator(toCopy.startPolygonRatio, toCopy.endPolygonRatio) {}

PolygonMapGenerator::PolygonMapGenerator(double startPolygonRatio, double endPolygonRatio): 
  startPolygonRatio(startPolygonRatio), endPolygonRatio(endPolygonRatio) {}



TerrainType PolygonMapGenerator::getClosestTerrain(double x, double y, vector<PolygonMapGenerator::Polygon> polygons, int width, int height) {
  if(polygons.size() > 0) {
    double minDistance = getToroidalDistance(x, y, polygons[0].x, polygons[0].y, width, height);
    TerrainType closestTerrain = polygons[0].t;

    for(PolygonMapGenerator::Polygon p : polygons) {
      double distance = getToroidalDistance(x, y, p.x, p.y, width, height);
      

      if(distance < minDistance) {
        closestTerrain = p.t;
        minDistance = distance;
      }
    }

    return closestTerrain;
  }

  return TerrainType::TerraIncognita;
}

void PolygonMapGenerator::printPolygonMap(vector<PolygonMapGenerator::Polygon>& polygons, int width, int height) {
  for(int y = height - 1; y >= -0; --y) {
    for (int x = 0; x < width; ++x) {
      cout << terrainToANSI(PolygonMapGenerator::getClosestTerrain(x, y, polygons, width, height)) << "  \033[0m";
    }
    cout << endl;
  }
}

void PolygonMapGenerator::fillRawMap(TerrainType **tiles, int **territoryMask, int width, int height, int numTeams) {
  int area = width * height;

  // Since area of the map will always be positive, this is a perfectly valid way of 
  //  rounding to the nearest int
  int startPolygons = (int)(area * startPolygonRatio + 0.5);
  int endPolygons = (int)(area * endPolygonRatio + 0.5);

  cout << "Area of the map is " << area << ". The generator will start with " << startPolygons << \
    " polygons and end with " << endPolygons << " polygons." << endl;



  vector<Polygon> polygons;

  cout << "\033[2J" << flush;
  // Create the initial polygons
  for(int i = 0; i < startPolygons; ++i ) {

    double randX = width * (rand() / ((double)RAND_MAX + 1));
    double randY = height * (rand() / ((double)RAND_MAX + 1));

    double randT = rand() / ((double)RAND_MAX + 1);
    TerrainType randomTerrain = TerrainType::TerraIncognita;
    if(randT < 0.60) randomTerrain = TerrainType::Field;
    else if (randT < 0.80) randomTerrain = TerrainType::Forest;
    else if (randT < 0.95) randomTerrain = TerrainType::Hills;
    else randomTerrain = TerrainType::Brambles;

    polygons.push_back(Polygon(randX, randY, randomTerrain));

    cout << "\033[1;1H" << flush;
    cout << "\nAdded polygon #" << i << "(random numbers were " << randX << ", " << randY << ", and " << randT << ")" << endl;
    PolygonMapGenerator::printPolygonMap(polygons, width, height);
  }

  for(int i = startPolygons; i < endPolygons; ++i) {
    double randX = width * (rand() / ((double)RAND_MAX + 1));
    double randY = height * (rand() / ((double)RAND_MAX + 1));
    TerrainType newT = PolygonMapGenerator::getClosestTerrain(randX, randY, polygons, width, height);
    polygons.push_back(Polygon(randX, randY, newT));

    cout << "\033[1;1H" << flush;
    cout << "\nAdded polygon #" << i << "(random numbers were " << randX << " and " << randY << ")" << endl;
    PolygonMapGenerator::printPolygonMap(polygons, width, height);
  }
}