#include "WebServer.hpp"

int main(int argc, char **argv, char **envp)
{
    WebServer webserv;
    std::string configFile;

    if (argc > 2)
        std::cerr << "Error too many args" << std::endl;
    else if (argc < 2)
        configFile = "configurations/default.conf";
    else
        configFile = argv[1];
    
    if (webserv.initialize())
        return (1);
    webserv.serverLoop();

}