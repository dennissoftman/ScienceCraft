#ifndef GAMEWINDOW_HPP
#define GAMEWINDOW_HPP

#define GAME_TITLE "ScienceCraft"

#include <SDL2/SDL.h>

#include "shadermanager.hpp"
#include "texmanager.hpp"
#include "mdlmanager.hpp"

#include "camera.hpp"

#include "server.hpp"
#include "client.hpp"
#include <list>
#include <mutex>

struct PlayerInfo
{
    uint16_t pid;
    glm::vec3 pos;
    glm::vec2 rot; // yaw and pitch
    glm::vec3 col;
};

struct BlockUpdate
{
    glm::ivec3 pos;
    int bid;
};

class Chunk;

class GameWindow
{
public:
    GameWindow(int width=1280, int height=720);

    void initGL();

    void host(uint16_t port); // create server
    void connect(const std::string &ip, uint16_t port); // connect to server

    void regenerateWorld();

    PlayerInfo *spawnPlayer(uint16_t pid);
    void updatePlayer(uint16_t pid, const glm::vec3 &np, const glm::vec2 &nr);
    void removePlayer(uint16_t pid);

    void updateBlock(const glm::ivec3 &pos, int bid);

    uint16_t selfPID() const;

    int exec();
    void cleanup();

    void loadConfig();

    static GameWindow *gameInstance;
    static int m_scrWidth, m_scrHeight;

    std::mutex m_playersLock;

    PlayerInfo *m_selfInfo;
    static uint32_t m_seed;
private:
    void createCursor();

    bool m_quit;
    SDL_GLContext m_glctx;
    SDL_Window *m_window;

    uint64_t m_ticksElapsed;

    ShaderManager *m_shmgr;
    TexManager *m_texmgr;
    MdlManager *m_mdlmgr;

    Camera *m_camera;

    GLuint m_cursorVAO, m_cursorVBO;
    GLuint m_quadVAO, m_quadVBO;

    std::unordered_map<std::string, Chunk*> m_chunks;

    // Multiplayer
    std::unordered_map<uint16_t, PlayerInfo*> m_players;

    Server *m_svHandle;
    Client *m_clHandle;
};

#endif // GAMEWINDOW_HPP
