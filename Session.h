#ifndef SESSION_H_BLOCK
#define SESSION_H_BLOCK

#include <queue>
#include <string>

class Session {
  private:
    static const int BUFFER_SIZE = 1024;
    bool isClosed;
    int sessionFD;
    std::queue<std::string> linesIn;
    std::string usrid;
  public:
    /**
     * Creates a new Session object that will listen on the given file descriptor
     * @param socketFD The file descriptor of the Session socket to listen to
     */
    Session(int sessionFD);

    ~Session();

    // #######################################################################
    // #####                  BEGIN GETTERS AND SETTERS                  #####
    // #######################################################################

    int getSessionFD();

    bool getIsClosed();
    bool getIsDone();

    std::string getUsrID();
    bool setUsrID(std::string usrid);

    // #######################################################################
    // #####                   END GETTERS AND SETTERS                   #####
    // #######################################################################

    int numQueuedLines();
    void pushRequest(std::string request);
    std::string readLine();
    void sendMessage(std::string message);
};

#endif