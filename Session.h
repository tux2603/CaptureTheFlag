#ifndef SESSION_H_BLOCK
#define SESSION_H_BLOCK

#include <queue>
#include <string>

using namespace std;


class Session {
  private:
    static const int BUFFER_SIZE = 1024;
    bool isClosed;
    int sessionFD;
    queue<string> linesIn;
    string usrid;
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

    string getUsrID();
    bool setUsrID(string usrid);

    // #######################################################################
    // #####                   END GETTERS AND SETTERS                   #####
    // #######################################################################

    int numQueuedLines();
    void pushRequest(string request);
    string readLine();
    void sendMessage(string message);
};

#endif