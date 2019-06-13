#ifndef SOCKETSHELL_H_BLOCK
#define SOCKETSHELL_H_BLOCK

/*
  This class create a Shell-like object that will listen on a TCP socket and handle requests made
  AUTHOR: Owen O'Connor
*/

#include <atomic>
#include <functional>
#include <map>
#include <netinet/in.h>
#include <set>
#include <string>
#include <thread>
#include "Session.h"
#include "Scheduler.h"

class SocketShell {
  private:

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~ BASIC SHELL STUFF ~~~~~~~~~~~~~~~~~~~~~~~~~~

    std::atomic<bool> shouldExit;
    std::string prompt; 

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SOCKET STUFF ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    int port, /// The port that the socket will listen for connections on
    socketFD, /// The file descriptor that the the socket is interfaced to
    opt=1;    /// Dunno. Some magic variable that is used in setting up the socket

    const int addrLen = sizeof(address); /// Also some magic variable that is used in setting up the socket

    struct sockaddr_in address; /// More magic

    /// Stores all of the commands known to the shell. The parameters to the function are
    ///   - int argc:           The number of arguments passed in argv
    ///   - string argv[]:      The arguments to the lambda
    ///   - SocketShell *shell: The SocketShell object that is invoking the lambda
    ///   - Session *session:   The session that made the request that caused the lambda to be invoked
    std::map<std::string, std::function<std::string(int, std::string[], SocketShell*, Session*)>> commandDictionary;

    /// Scheduler keeps track of all connections and gets requests from them
    Scheduler scheduler;

    /// A thread that will continually check for new connection requests on the socket
    std::thread sessionCheckThread;

  public:
    /**
     * Creates a new shell-like object that will listen on a TCP socket for 
     *  commands to execute, with default port (8042) and prompt
     *  (socketshell@8042$ ).
     */
    SocketShell();

    /**
     * Creates a new shell-like object that will listen on a TCP socket for
     *  commands to execute, with a specified port and default prompt 
     *  (socketshell@$PORT$ ).
     * 
     * @param port The port for the socket to listen on
     */
    SocketShell(int port);

    /**
     * Creates a new shell-like object that will listen on a TCP socket for
     *  commands to execute, with a specified port and prompt.
     */
    SocketShell(int port, std::string prompt);

    ~SocketShell();
    
    /**
     * Gets a set of sessions that are connected to the SocketShell object
     * @returns A std::set<Session*> object containing all of the sessions 
     *  that are currently connected
     */
    std::set<Session*> getSessions();

    Session *getSessionByID(int id);

    /**
     * Gets the port that the SocketShell object is listening on
     * @returns The numeric identity of the TCP port
     */
    int getPort();

    /**
     * Gets the default prompt that will be assigned to all sessions that 
     *  connect to the SocketShell object.
     * @return A string containing the default prompt
     */
    std::string getPrompt();

    /**
     * Sets the default prompt that will be assigned to all sessions that
     *  connect to the SocketShell object
     * @param prompt The new default prompt
     */
    void setPrompt(std::string prompt);

    /**
     * Adds a command to the SocketShell object's command dictionary.
     * @param name The name that the command is to be listed under
     * @param lambda A lambda of type function<string(int, string[], SocketShell*, Session*)> 
     *  that will be executed when the given command is invoked.
     */
    void addCommand(std::string name, std::function<std::string(int, std::string[], SocketShell*, Session*)> lambda);
    
    /**
     * Returns the names of all of the commands in the object's command dictionary
     * @return An array of strings holding the names of all the commands
     */ 
    std::set<std::string> listCommands();

    /**
     * Triggers a single update cycle on the SocketShell object. This will poll
     *  the Scheduler for the next request to execute and execute it. If the 
     *  Scheduler does not have a request ready, it will wait indefinitely until
     *  one becomes available.
     */
    void update();
};

#endif