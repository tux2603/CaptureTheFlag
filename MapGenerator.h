#ifndef MAP_GENERATOR_H_BLOCK
#define MAP_GENERATOR_H_BLOCK

#include <vector>
#include "GameComponents.h"

class TerritoryAllocationCell {
  private:
    int numTeams;
  public: 
    static const int NO_OWNER;
    static const int DISPUTED;
    static const long MIN_CLAIM_TO_OWN;
    long *claims;
    TerrainType terrain;

    TerritoryAllocationCell();
    TerritoryAllocationCell(const TerritoryAllocationCell& toCopy) = delete;
    
    TerritoryAllocationCell(int numTeams, TerrainType terrain);
    ~TerritoryAllocationCell();

    int getOwner();
    int resolveDisputes();
};

class MapGenerator {
  public:
    virtual void fillRawMap(TerrainType **tiles, int **territoryMask, int width, int height, int numTeams) = 0;
    void fillMap(TerrainMap &map);
};

class BarMapGenerator: public MapGenerator{
  public:
    BarMapGenerator();
    BarMapGenerator(const BarMapGenerator& toCopy);
    void fillRawMap(TerrainType **tiles, int **territoryMask, int width, int height, int numTeams);
};

class PolygonMapGenerator: public MapGenerator {
  private:
    struct Polygon {
      /** The x coordinate of the center of the polygon */
      double x;
      /** The y coordinate of the center of the polygon */
      double y;
      /** The terrain contained in the polygon */
      TerrainType t;
      Polygon(int x, int y, TerrainType t): x(x), y(y), t(t){}
    };

    static TerrainType getClosestTerrain(double x, double y, std::vector<PolygonMapGenerator::Polygon> polygons, int width, int height);
    static void printPolygonMap(std::vector<PolygonMapGenerator::Polygon>& polygons, int width, int heigth);
  public:
    

    
    double startPolygonRatio, endPolygonRatio, waterRatio;
    PolygonMapGenerator();
    PolygonMapGenerator(double startPolygonRatio, double endPolygonRatio, double waterRatio);
    PolygonMapGenerator(const PolygonMapGenerator& toCopy);
    void fillRawMap(TerrainType **tiles, int **territoryMask, int width, int height, int numTeams);
};

#endif //MAP_GENERATOR_H_BLOCK