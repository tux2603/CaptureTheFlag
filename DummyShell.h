// This class creates a simple shell-ish thing
#include <functional>
#include <map>
#include <string>

using namespace std;

class DummyShell {
  protected:
    bool shellHasExited;
    string prompt; 
    map<string, function<string(int, string[], DummyShell*)>> commandDictionary;
  public:
    /**
     * Creates a new DummyShell object with default prompt (`dummyshell$ `)
     */
    DummyShell();

    /**
     * Creates a new DummyShell object with a specified prompt
     * @parem prompt The prompt that will be used by the dummy shell
     */
    DummyShell(string prompt);

    /**
     * Gets the object's shell prompt
     * @return A string containing the prompt
     */
    string getPrompt();

    /**
     * Sets the object's shell prompt
     * @parem prompt The new shell prompt for the object
     */
    void setPrompt(string prompt);

    /**
     * Adds a command to the object's command dictionary
     * @parem name The name that the command will be listed under in the dictionary
     * @parem lambda The lambda that is to be executed when the command is called
     */
    void addCommand(string name, function<string(int, string[], DummyShell*)> lambda);

    /**
     * Returns the names of all of the commands in the object's command dictionary
     * @return An array of strings holding the names of all the commands
     */ 
    string* listCommands();

    /**
     * Gets the number of commands in the objetc's command dictionary
     * @return The number of commands in the objetc's command dictionary
     */
    int numCommands();

    // ! #####################################################################
    // TODO Finish documentation 
    // ! #####################################################################

    void exit();

    bool hasExited();

    //bool removeCommand(string cName);

    string exec(string str); 
};