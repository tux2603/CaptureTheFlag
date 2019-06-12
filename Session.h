#ifndef SESSION_H_BLOCK
#define SESSION_H_BLOCK

#include <queue>
#include <string>

class Session {
  private:
    static const int BUFFER_SIZE = 1024;
    static int uniqueID;
    int id;
    bool isClosed;
    std::string prompt;
    int sessionFD;
    std::queue<std::string> linesIn;
  public:
    /**
     * Creates a new Session object that will listen on the given file descriptor
     * @param socketFD The file descriptor of the Session socket to listen to
     */
    Session(int sessionFD, std::string prompt);

    /**
     * Copy constructor for the Session class. Creates a new Session object
     *  that is a copy of the old one
     * @param toCopy The session to be copied
     */
    Session(const Session &toCopy);

    ~Session();

    // #######################################################################
    // #####                  BEGIN GETTERS AND SETTERS                  #####
    // #######################################################################

    int getID();
    int getSessionFD();

    bool getIsClosed();
    bool getIsDone();

    void setPrompt(std::string);
    std::string getPrompt();

    // #######################################################################
    // #####                   END GETTERS AND SETTERS                   #####
    // #######################################################################

    int numQueuedLines();
    void pushRequest(std::string request);
    std::string readLine();
    void sendMessage(std::string message);
    void sendPrompt();
};

#endif