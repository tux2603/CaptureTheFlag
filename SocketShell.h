#ifndef SOCKETSHELL_H_BLOCK
#define SOCKETSHELL_H_BLOCK

/*
  This class create a Shell-like object that will listen on a TCP socket and handle requests made
  AUTHOR: Owen O'Connor
*/

#include <map>
#include <netinet/in.h>
#include <set>
#include <string>
#include <thread>
#include "DummyShell.h"
#include "Session.h"
#include "Scheduler.h"

using namespace std;

class SocketShell : public DummyShell {
  private:
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
    map<string, function<string(int, string[], SocketShell*, Session*)>> commandDictionary;

    Scheduler scheduler;

    // TODO ############################################################# TODO
    // TODO                 Replace this with a scheduler                 TODO
    // TODO ############################################################# TODO
    set<Session*> sessions;

    /// A thread that will continually check for new connection requests on the socket
    thread sessionCheckThread;

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
    SocketShell(int port, string prompt);

    // TODO ############################################################# TODO
    // TODO                        Add destructors                        TODO
    // TODO ############################################################# TODO
    
    /**
     * Gets a set of sessions that are connected to the SocketShell object
     * @returns A std::set<Session*> object containing all of the sessions 
     *  that are currently connected
     */
    set<Session*> getSessions();

    /**
     * Gets the port that the SocketShell object is listening on
     * @returns The numeric identity of the TCP port
     */
    int getPort();

    /**
     * Adds a command to the SocketShell object's command dictionary.
     * @param name The name that the command is to be listed under
     * @param lambda A lambda of type function<string(int, string[], SocketShell*, Session*)> 
     *  that will be executed when the given command is invoked.
     */
    void addCommand(string name, function<string(int, string[], SocketShell*, Session*)> lambda);
    
    /**
     * Triggers a single update cycle on the SocketShell object. This will poll
     *  the Scheduler for the next request to execute and execute it. If the 
     *  Scheduler does not have a request ready, it will wait indefinitely until
     *  one becomes available.
     */
    void update();
};

#endif