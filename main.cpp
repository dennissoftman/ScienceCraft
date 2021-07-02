#include "gamewindow.hpp"

/*
#ifdef _WIN32
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow )
#else
*/

#ifdef _WIN32
#undef main
#endif

int main(int argc, char **argv)
{
    GameWindow *win = new GameWindow(1280, 720);

    for(int i=1; i < argc; i++)
    {
        if(strcmp(argv[i], "--server") == 0)
        {
            assert((i+1) < argc && "Port required");
            uint16_t port = atoi(argv[i+1]);
            assert(port > 0 && "Incorrect port");
            win->host(port);
            continue;
        }
        else if(strcmp(argv[i], "--connect") == 0)
        {
            assert((i+1) < argc && "Insufficient arguments");
            std::string dest = argv[i+1];
            size_t iplen = dest.find_first_of(':');
            assert(iplen < dest.length() && "Port required");
            assert(iplen != std::string::npos && "Insufficient arguments");
            std::string ip = dest.substr(0, iplen);
            uint16_t port = atoi(dest.substr(iplen+1).c_str());
            win->connect(ip, port);
        }
    }

    return win->exec();
}
