#include <iostream>
#include <math.h>
#include <random>
#include <unistd.h>
#include <vector>
#include "GameComponents.h"
#include "MapGenerator.h"

using namespace std;

//#define DEBUG
//#define RESET_PRINT_HEAD
//#define SLOW

const int TerritoryAllocationCell::NO_OWNER = -1;
const int TerritoryAllocationCell::DISPUTED = -2;
const long TerritoryAllocationCell::MIN_CLAIM_TO_OWN = 1024;

TerritoryAllocationCell::TerritoryAllocationCell(): TerritoryAllocationCell(0, TerrainType::TerraIncognita) {}

TerritoryAllocationCell::TerritoryAllocationCell(int numTeams, TerrainType terrain): numTeams(numTeams), terrain(terrain) {
  claims = new long[numTeams];
  for(int i = 0; i < numTeams; ++i) claims[i] = 0;
}

TerritoryAllocationCell::~TerritoryAllocationCell() {
  // cout << "\033[1;41m##### Deleting a territory cell " << claims << "! #####\033[0m" << endl;
  delete [] claims;
}

int TerritoryAllocationCell::getOwner() {
  //cout << "Reading from " << claims << endl;
  int owner = NO_OWNER;
  for(int i = 0; i < numTeams; ++i) {
    if(claims[i] >= MIN_CLAIM_TO_OWN && owner == NO_OWNER) owner = i;
    else if (claims[i] >= MIN_CLAIM_TO_OWN && owner != NO_OWNER) return DISPUTED;
  }

  return owner;
}

int TerritoryAllocationCell::resolveDisputes() {
  int owner = NO_OWNER;
  int maxClaim = 0;

  for(int i = 0; i < numTeams; ++i) {
    if(claims[i] >= MIN_CLAIM_TO_OWN && claims[i] >= maxClaim) {
      owner = i;
      maxClaim = claims[i];
    } 
  }

  for (int i = 0; i < numTeams; ++i) {
    claims[i] = (i == owner ? MIN_CLAIM_TO_OWN + 1 : 0);
  }

  return owner;
}

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
      //  If so, it should be a 
      if (abs(fmod(x, territorySpacing)) < 1.0)
      {
        tiles[y][x] = TerrainType::Border;
        territoryMask[y][x] = -1;
      }
    }
  }
}


PolygonMapGenerator::PolygonMapGenerator(): PolygonMapGenerator(0.01, 0.1, 0.02) {}

PolygonMapGenerator::PolygonMapGenerator(const PolygonMapGenerator& toCopy): 
  PolygonMapGenerator(toCopy.startPolygonRatio, toCopy.endPolygonRatio, toCopy.waterRatio) {}

PolygonMapGenerator::PolygonMapGenerator(double startPolygonRatio, double endPolygonRatio, double waterRatio): 
  startPolygonRatio(startPolygonRatio), endPolygonRatio(endPolygonRatio), waterRatio(waterRatio) {}



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


#ifdef DEBUG
# ifdef RESET_PRINT_HEAD
  cout << "\033[1;1H" << endl;
# endif // RESET_PRINT_HEAD
  PolygonMapGenerator::printPolygonMap(polygons, width, height);
#endif //DEBUG

  }

  // Continue to add polygons until we have reached the target number
  for(int i = startPolygons; i < endPolygons; ++i) {
    double randX = width * (rand() / ((double)RAND_MAX + 1));
    double randY = height * (rand() / ((double)RAND_MAX + 1));
    TerrainType newT = PolygonMapGenerator::getClosestTerrain(randX, randY, polygons, width, height);
    polygons.push_back(Polygon(randX, randY, newT));

#ifdef DEBUG
# ifdef RESET_PRINT_HEAD
  cout << "\033[1;1H" << endl;
# endif //RESET_PRINT_HEAD
  PolygonMapGenerator::printPolygonMap(polygons, width, height);
#endif //DEBUG

  }

  for(int x = 0; x < width; ++x){
    for (int y = 0; y < height; ++y) {
      tiles[y][x] = PolygonMapGenerator::getClosestTerrain(x, y, polygons, width, height);
      territoryMask[y][x] = -1;
    }
  }

#ifdef DEBUG

  cout << "Done generating terrain... Begin allocation of territory" << endl;
  for(int y = height - 1; y >= 0; --y) {
    for(int x = 0; x < width; ++x){
      cout << terrainToANSI(tiles[y][x]) << "&&\033[0m";
    }

    cout << endl;
  }
#endif //DEBUG

  // Create two arrays to store territory allocations
  TerritoryAllocationCell ***territoryCellsA = new TerritoryAllocationCell **[height];
  TerritoryAllocationCell ***territoryCellsB = new TerritoryAllocationCell **[height];

  // Initialize the values in the arrays
  for(int y = 0; y < height; ++y) {
    territoryCellsA[y] = new TerritoryAllocationCell *[width];
    territoryCellsB[y] = new TerritoryAllocationCell *[width];

    for(int x = 0; x < width; x++) {
      territoryCellsA[y][x] = new TerritoryAllocationCell(numTeams, tiles[y][x]);
      territoryCellsB[y][x] = new TerritoryAllocationCell(numTeams, tiles[y][x]);
    }
  }

  // Create two pointers to the territory arrays that will be used to alternate between successive generations
  TerritoryAllocationCell ****currentTerritory = &territoryCellsA;
  TerritoryAllocationCell ****nextTerritory = &territoryCellsB;


  // Thingies to generate random numbers
  default_random_engine g(time(0));
  uniform_int_distribution<int> rX(0, width - 1);
  uniform_int_distribution<int> rY(0, height - 1);

  // Set random starting locations for each of the teams
  for (int i = 0; i < numTeams; ++i) {

    // Get a candidate starting location
    int randX = rX(g);
    int randY = rY(g);

    // If that candidate location is already taken, get a new candidate
    while((*currentTerritory)[randY][randX]->getOwner() != TerritoryAllocationCell::NO_OWNER) {
      randX = rX(g);
      randY = rY(g);
    }
    
    // Once a suitable candidate is found, assign the owner at that location to be team i
    (*currentTerritory)[randY][randX]->claims[i] = TerritoryAllocationCell::MIN_CLAIM_TO_OWN;
    (*nextTerritory)[randY][randX]->claims[i] = TerritoryAllocationCell::MIN_CLAIM_TO_OWN;

    // Make this cell team i's prison
    tiles[randY][randX] = TerrainType::Prison;
  }

  // Keep track of many cells are unclaimed. To begin, there will only be as many cells claimed as there are teams
  long long unclaimdCells = width * height - numTeams;

#ifdef DEBUG
# ifdef RESET_PRINT_HEAD
  cout << "\033[2J" << endl;
# endif //RESET_PRINT_HEAD
#endif

  // While there are still too many unclaimed cells. The number of cells that will be claimed can be set
  //  using the object's waterRatio field. Any unclaimed cells after this loop exits will automatically
  //  be set to water
  while(unclaimdCells > width * height * waterRatio) {  

#ifdef DEBUG
# ifdef RESET_PRINT_HEAD
    cout << "\033[1;1H" << flush;
# endif //RESET_PRINT_HEAD
    for(int y = height - 1; y >= 0; --y) {
      for (int x = 0; x < width; ++x) {
        if((*currentTerritory)[y][x]->getOwner() == TerritoryAllocationCell::DISPUTED)
          cout << terrainToANSI(TerrainType::Border) << "++\033[0m";
        else if ((*currentTerritory)[y][x]->getOwner() == TerritoryAllocationCell::NO_OWNER)
          cout << terrainToANSI((*currentTerritory)[y][x]->terrain) << "&&\033[0m";
        else
          cout << terrainToANSI((*currentTerritory)[y][x]->terrain) << (char)((*currentTerritory)[y][x]->getOwner() + 97) << " \033[0m";
      }

      cout << endl;
    }
#endif //DEBUG

    // Perform the cellular automaton rules on every cell in the map
    // The cellular automaton works very simply. If any cell is claimed, it starts claiming cells
    //  that are next to it. If two teams try to claim the same cell, it will be marked as disputed and prevent
    //  any further spread in that direction. Claim spreads fastest between two cells of the same terrain type.
    for(int x = 0; x < width; ++x) {
      for(int y = 0; y < height; ++y) {

        // Get the owner of the cell
        int cellOwner = (*currentTerritory)[y][x]->getOwner();
        
        // If the cell doesn't have an owner, it doesn't have to spread claims
        if(cellOwner >= 0) {

          // Make sure that if a cell is owned by team x in this generation, it will also be owned by team x in the next generation
          if((*nextTerritory)[y][x]->claims[cellOwner] < TerritoryAllocationCell::MIN_CLAIM_TO_OWN) (*nextTerritory)[y][x]->claims[cellOwner] = TerritoryAllocationCell::MIN_CLAIM_TO_OWN;

          // Get an array of the adjacent cells in this generation for reading from
          TerritoryAllocationCell **currentAdjacentCells = new TerritoryAllocationCell*[8];
          
          currentAdjacentCells[0] = (*currentTerritory)[(y+1)%height][(x+width-1)%width];
          currentAdjacentCells[1] = (*currentTerritory)[(y+1)%height][x];
          currentAdjacentCells[2] = (*currentTerritory)[(y+1)%height][(x+1)%width];
          currentAdjacentCells[3] = (*currentTerritory)[y][(x+1) % width];
          currentAdjacentCells[4] = (*currentTerritory)[y][(x+width-1)%width];
          currentAdjacentCells[5] = (*currentTerritory)[(y+height-1)%height][(x+width-1)%width];
          currentAdjacentCells[6] = (*currentTerritory)[(y+height-1)%height][x];
          currentAdjacentCells[7] = (*currentTerritory)[(y+height-1)%height][(x+1)%width];
          

          // Get an array of the adjacent cells in the next generation for writing to
          TerritoryAllocationCell **nextAdjacentCells = new TerritoryAllocationCell*[8];

          nextAdjacentCells[0] = (*nextTerritory)[(y+1)%height][(x+width-1)%width];
          nextAdjacentCells[1] = (*nextTerritory)[(y+1)%height][x];
          nextAdjacentCells[2] = (*nextTerritory)[(y+1)%height][(x+1)%width];
          nextAdjacentCells[3] = (*nextTerritory)[y][(x+1) % width];
          nextAdjacentCells[4] = (*nextTerritory)[y][(x+width-1)%width];
          nextAdjacentCells[5] = (*nextTerritory)[(y+height-1)%height][(x+width-1)%width];
          nextAdjacentCells[6] = (*nextTerritory)[(y+height-1)%height][x];
          nextAdjacentCells[7] = (*nextTerritory)[(y+height-1)%height][(x+1)%width];
          

          // Iterate over each of the adjacent cells to attempt to claim them
          for(int i = 0; i < 8; ++i) {
            // Get the current owner of the adjacent cell
            int adjacentCellOwner = currentAdjacentCells[i]->getOwner();

            // If the cells already have the same owner, no spreading of claim is necessary
            if(adjacentCellOwner != cellOwner) {

              // Claim travels fastest over field and slowest through brambles
              if(currentAdjacentCells[i]->terrain == TerrainType::Brambles) nextAdjacentCells[i]->claims[cellOwner] += 1;
              else if(currentAdjacentCells[i]->terrain == TerrainType::Forest) nextAdjacentCells[i]->claims[cellOwner] += 2;
              else if(currentAdjacentCells[i]->terrain == TerrainType::Hills) nextAdjacentCells[i]->claims[cellOwner] += 2;
              else if(currentAdjacentCells[i]->terrain == TerrainType::Field) nextAdjacentCells[i]->claims[cellOwner] += 4;

              // If the cell is the same territory and unclaimed, then claim it particularly fast. This way, lines of claim will
              //  roughly follow the terrain
              if(currentAdjacentCells[i]->terrain == (*currentTerritory)[y][x]->terrain && adjacentCellOwner == TerritoryAllocationCell::NO_OWNER)
                nextAdjacentCells[i]->claims[cellOwner] += (long)(nextAdjacentCells[i]->claims[cellOwner] * 1.1);

              // If someone else ones the adjacent cell, dispute it
              // if(adjacentCellOwner >= 0 && adjacentCellOwner != cellOwner)
              //   nextAdjacentCells[i]->claims[cellOwner] += (long)(nextAdjacentCells[i]->claims[cellOwner] * 4);

              // make sure we haven't overflowed long
              if(currentAdjacentCells[i]->claims[cellOwner] < 0 ) nextAdjacentCells[i]->claims[cellOwner] = TerritoryAllocationCell::MIN_CLAIM_TO_OWN;

            }
          }

          delete [] currentAdjacentCells;
          delete [] nextAdjacentCells;
        }

        // If the cell doesn't have an owner but is disputed, make sure that each team has an even claim on it
        // ! Is this the proper way to handle this?
        else if(cellOwner == TerritoryAllocationCell::DISPUTED) {
          for(int i = 0; i < numTeams; ++i) {
            (*nextTerritory)[y][x]->claims[i] = TerritoryAllocationCell::MIN_CLAIM_TO_OWN;
          }
        }
      }
    }

    // Swap the next territory pointer and the current territory pointer for the next generation of the automaton
    TerritoryAllocationCell ****temp = nextTerritory;
    nextTerritory = currentTerritory;
    currentTerritory = temp;

    // Reset the count of cells that nobody owns
    unclaimdCells = 0;

    // Find the number of cells that nobody owns
    for(int x = 0; x < width; ++x) {
      for(int y = 0; y < height; y++) {
        if((*currentTerritory)[y][x]->getOwner() == TerritoryAllocationCell::NO_OWNER) ++unclaimdCells;
      }
    }

#ifdef SLOW
    usleep(500000);
#endif //SLOW
  }

  // Transfer the values from the territory allocation array to the territory mask that was passed in 
  for(int x = 0; x < width; ++x) {
    for(int y = 0; y < height; ++y) {

      // If nobody owns the cell, it becomes water and no-mans land
      if((*currentTerritory)[y][x]->getOwner() == TerritoryAllocationCell::NO_OWNER) {
        tiles[y][x] = TerrainType::Water;
        territoryMask[y][x] = -1;
      }

      // If the cell is disputed, resolve the dispute in whatever team has the most claim over it
      else if ((*currentTerritory)[y][x]->getOwner() == TerritoryAllocationCell::DISPUTED){
        territoryMask[y][x] = (*currentTerritory)[y][x]->resolveDisputes();
      }

      // In any other case, it's just a simple matter of copying numbers
      else {
        territoryMask[y][x] = (*currentTerritory)[y][x]->getOwner();
      }
    }
  }

#ifdef DEBUG
  cout << "Done with territory allocation cells" << endl;
#endif
  // Release the memory used by the assignment variables
  for(int y = 0; y < height; y++) {

    for(int x = 0; x < width; x++) {
      delete territoryCellsA[y][x];
      delete territoryCellsB[y][x];
    }

    delete[] territoryCellsA[y];
    delete[] territoryCellsB[y];
  }

  delete[] territoryCellsA;
  delete[] territoryCellsB;
}