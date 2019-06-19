#ifndef GAME_COMPONENTS_H_BLOCK
#define GAME_COMPONENTS_H_BLOCK


#include <string>

// ! IMPORTANT
// Directions and corresponding changes in coordinates correspond to real-life 
//  longitude/latitude coordinates, not the typical computer screen reversed-y 
//  coordinate system. North being up is sort of hardwired into humans minds 
//  from looking at maps, so I decided to keep north up, and have up be an 
//  increase in y coordinate, like it would in actual navigation.
// Apologies to all programmers that this might throw off.
enum Direction {
  // North is equivalent to an increase in y coordinate
  North = 0b0001, Up = 0b0001,

  // South is equivalent to a decrease in y coordinate
  South = 0b0010, Down = 0b0010,

  // East is equivalent to an increase in x coordinate
  East = 0b0100, Right = 0b0100,

  // West is equivalent to a decrease in x coordinate
  West = 0b1000, Left = 0b1000,

  // Ordinal directions are the cardinal direction bitwise-or'd together
  NorthWest = 0b1001,
  NorthEast = 0b0101,
  SouthWest = 0b1010,
  SouthEast = 0b0110
};

/*
  List of all terrains that the game will have. Each basic terrain type will be given a
    unique bit to represent them. This means that we will be able to 
    have at most 64 basic terrain types. 
*/
enum TerrainType {
  TerraIncognita = 0,
  Field,    // Basic terrain
  Forest,   // Decreases visibility
  Hills,    // Increases visibility 
  Brambles, // Decreases mobility
  Water,    // Impassible
  Stream,   // Boundary between territories
  Path,     // Increases mobility
  Prison    // Can only leave if in your own territory, or if a 
            //  teammate is standing in an adjacent cell
  // TODO Do we need some sort of 'flag zone' type?
};

enum GenerationAlgorithm {
  Dummy, // Divides the land into equal pieces
  Polygon,
  Default = Polygon
};


/**
 * Gets the distance between two points in a 2D toroidal wrap around universe
 * @param x1 The x coordinate of the first point
 * @param y1 The y coordinate of the first point
 * @param x2 The x coordinate of the second point
 * @param y2 The y coordinate of the second point
 * @param width The width of the universe
 * @param height The height of the universe
 */
double getToroidalDistance(double x1, double y1, double x2, double y2, double width, double height);

/**
 * Gets an ANSI color code chosen to represent a terrain type
 */
std::string terrainToANSI(TerrainType t);

// Player class keeps track of an individual player. For now, all this means is keeping an eye on where they are
class Player {
  private:
    int x, y;
  public:
    /**
     * Creates a new player object at location (0, 0)
     */
    Player();

    /**
     * Creates a new player object at location (x, y)
     * @param x Integer value for the x coordinate that the player should be initialized at
     * @param y Integer value for the y coordinate that the player should be initialized at
     */
    Player(int x, int y);
    
    /// Gets the player's x coordinate
    int getX();

    /// Sets the player's x coordinate
    void setX(int x);

    /// Gets the player's y coordinate
    int getY();

    /// Sets the player's y coordinate
    void setY(int y);

    void move(Direction direction);
};

class Map {
  private:
    /// Fill a map with computer generated terrain
    static void fillMap(TerrainType **tiles, int **territoryMask, int width, int height, int numTeams);
    static void fillMap(TerrainType **tiles, int **territoryMask, int width, int height, int numTeams, GenerationAlgorithm alg);

    int width, height, numTeams;

    /// First index is y, second index is x
    TerrainType **tiles;

    /// Used by the map to determine whose territory is whose...
    int **territoryMask;
    

  public:
    /**
     * Creates a 15x15 map of field with a stream running from top to bottom in the center
     * Prisons are located in the upper left and lower right hand corners
     */
    Map();

    /**
     * Creates a randomly generated map of specified width and height and two teams
     */
    Map(int width, int height);

    Map(int width, int height, int numTeams);

    ~Map();

    int getWidth();
    int getHeight();

    int getNumTeams();

    TerrainType getTerrainAt(int x, int y);
    int getTerritoryAt(int x, int y);

    // Returns a string with an ANSI-formatted representation of the map
    std::string ansi();

    friend class MapGenerator;
};

#endif //GAME_COMPONENTS_H_BLOCK