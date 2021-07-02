#include "gamewindow.hpp"
#include <SDL2/SDL_image.h>
#include <unordered_map>
#include <map>
#include <set>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <algorithm>
#include <future>
#include <thread>

#include "chunk.hpp"

#include "dda.hpp"
#include "ray.hpp"
#include "PerlinNoise.hpp"

GameWindow* GameWindow::gameInstance = nullptr;
int GameWindow::m_scrWidth  = 0;
int GameWindow::m_scrHeight = 0;
uint32_t GameWindow::m_seed = 0;

GameWindow::GameWindow(int width, int height)
    : m_quit(false), m_ticksElapsed(0),
      m_svHandle(nullptr), m_clHandle(nullptr)
{
    GameWindow::gameInstance = this;
    GameWindow::m_seed = time(0);
    srand(GameWindow::m_seed);

    m_scrWidth  = width;
    m_scrHeight = height;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER);

    m_window = SDL_CreateWindow(GAME_TITLE,
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                m_scrWidth, m_scrHeight,
                                SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if(m_window == nullptr)
    {
        fprintf(stderr, "SDL window creation error\nwhat: %s", SDL_GetError());
        exit(1);
    }

    initGL();

    //
    m_shmgr = new ShaderManager();
    m_texmgr = new TexManager();
    m_mdlmgr = new MdlManager();

    m_camera = new Camera(90.f, (float)width / (float)height);
    m_camera->restrict(glm::vec3(1, 0, 0), -glm::pi<float>()/2.f, glm::pi<float>()/2.f);

    m_camera->setPos(glm::vec3(1, 16*4, 1));

    loadConfig();
}

void GameWindow::initGL()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_CreateContext(m_window);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16); // 16 bits
    SDL_GL_SetSwapInterval(1); // enable vsync

    #ifndef __APPLE__
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to init GLEW\n");
        exit(-1);
    }
    #endif

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_NORMALIZE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    glShadeModel(GL_SMOOTH);

    glEnable(GL_MULTISAMPLE);

    glViewport(0, 0, m_scrWidth, m_scrHeight);

    // create cursor
    static const float cursorPoints[] =
    {
        0.0f, 0.0f
    };

    glGenVertexArrays(1, &m_cursorVAO);
    glGenBuffers(1, &m_cursorVBO);

    glBindVertexArray(m_cursorVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_cursorVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cursorPoints), cursorPoints, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), (void*)0);
    glBindVertexArray(0);

    // create quad
    static const float quadPoints[] =
    {
        // x      y     z     u     v
        -0.5f, +0.5f, 0.0f, 0.0f, 0.0f,
        +0.5f, +0.5f, 0.0f, 1.0f, 0.0f,
        +0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);

    glBindVertexArray(m_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadPoints), quadPoints, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void*)0); // vertCoord
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void*)(3*sizeof(GLfloat))); // texCoord
    glBindVertexArray(0);
}

void GameWindow::host(uint16_t port)
{
    m_svHandle = new Server(port);
    m_selfInfo = new PlayerInfo;
    m_selfInfo->pos = m_camera->getPos();
    m_selfInfo->rot = m_camera->getRot();
    m_svHandle->sendPlayerInfo(m_selfInfo);
}

void GameWindow::connect(const std::string &ip, uint16_t port)
{
    m_clHandle = new Client(ip, port);
    m_selfInfo = new PlayerInfo;
    m_selfInfo->pos = m_camera->getPos();
    m_selfInfo->rot = m_camera->getRot();
    m_clHandle->sendPlayerInfo(m_selfInfo);
}

void GameWindow::regenerateWorld()
{
    m_chunks.clear();

    std::vector<std::future<void>> chunkFutures;
    const int ch_x = 64, ch_z = 64;
    for(int i=0; i < ch_x; i++)
    {
        for(int j=0; j < ch_z; j++)
        {
            glm::ivec2 chPos = glm::ivec2(i - ch_x/2, j - ch_z/2);
            chunkFutures.push_back(std::async(std::launch::async, Chunk::generateChunk, chPos, std::ref(m_chunks)));
        }
    }
    for(std::future<void> &f : chunkFutures) // wait for chunks
        f.get();
}

PlayerInfo *GameWindow::spawnPlayer(uint16_t pid)
{
    PlayerInfo *p = new PlayerInfo;
    p->pid = pid;
    int r = (pid >> 10) & 0b111111;
    int g = (pid >> 5) & 0b11111;
    int b = pid  & 0b11111;
    p->col = glm::vec3(r / 64.f, g / 32.f, b / 32.f);
    m_players.emplace(std::pair<uint16_t, PlayerInfo*>(pid, p));

    return p;
}

void GameWindow::updatePlayer(uint16_t pid, const glm::vec3 &np, const glm::vec2 &nr)
{
    if(m_players.find(pid) == m_players.end())
        spawnPlayer(pid);
    m_players[pid]->pos = np;
    m_players[pid]->rot = nr;
}

void GameWindow::removePlayer(uint16_t pid)
{
    if(m_players.find(pid) != m_players.end())
    {
        PlayerInfo *p = m_players[pid];
        delete p;
        m_players.erase(pid);
    }
}

void GameWindow::updateBlock(const glm::ivec3 &pos, int bid)
{
    std::string cid = asString(glm::ivec3(pos / 4));
    Chunk::chunkMutex->lock();
    if(m_chunks.find(cid) != m_chunks.end())
    {
        m_chunks[cid]->setBlock(pos%4, bid);
    }
    Chunk::chunkMutex->unlock();
}

uint16_t GameWindow::selfPID() const
{
    return m_selfInfo->pid;
}

namespace std
{
    template<> struct less<glm::ivec3>
    {
       bool operator() (const glm::ivec3& lhs, const glm::ivec3& rhs) const
       {
           return (lhs.x < rhs.x && lhs.y < rhs.y && lhs.z < rhs.z);
       }
    };
}

float angleBetween(glm::vec2 a, glm::vec2 b)
{
    float dot = a.x*b.x + a.y*b.y;
    float det = a.x*b.y - a.y*b.x;
    return atan2(det, dot);
}

int GameWindow::exec()
{
    Shader *mainShader = m_shmgr->get("main");
    Shader *cubeShader = m_shmgr->get("cube");
    Shader *cursorShader = m_shmgr->get("cursor");
    Shader *selectionShader = m_shmgr->get("selection");

    Model3D *cubeMdl   = m_mdlmgr->get("cube");
    Model3D *monkeyMdl = m_mdlmgr->get("monkey");
    //

    // Generate map
    if(!m_clHandle)
        regenerateWorld();

    //
    int keymap[512];
    memset(keymap, 0, 512*sizeof(int));
    //
    bool lastPosValid = false;
    glm::ivec3 lastPos;
    //
    SDL_Event ev;
    while(!m_quit)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::ivec3 curChunk = glm::ivec3(m_camera->getPos().x/CHUNK_WIDTH,
                                         m_camera->getPos().y/CHUNK_HEIGHT,
                                         m_camera->getPos().z/CHUNK_DEPTH);

        // TODO: RAYCASTING
        Ray rayCast(m_camera->getPos(),
                    m_camera->fwdVector());
        rayCast.setLimit(4.f);
        lastPosValid = false;
        do
        {
            glm::ivec3 chPos = glm::ivec3(ceil(rayCast.getPos().x / 4.f),
                                          ceil(rayCast.getPos().y / 4.f),
                                          ceil(rayCast.getPos().z / 4.f));
            std::string cid = asString(chPos);
            if(m_chunks.find(cid) == m_chunks.end())
                continue;
            Chunk *ch = m_chunks[cid];
            int bid;
            if((bid = ch->getBlock(glm::ivec3(rayCast.getPos())%4)) > 0)
            {
                lastPosValid = true;
                lastPos = rayCast.getPos();
                break;
            }
        } while (rayCast.step(0.2f));
        //

        while(SDL_PollEvent(&ev))
        {
            if(ev.type == SDL_QUIT)
            {
                m_quit = true;
                break;
            }
            else if(ev.type == SDL_KEYDOWN)
            {
                if(ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                {
                    SDL_SetRelativeMouseMode(SDL_GetRelativeMouseMode() ? SDL_FALSE : SDL_TRUE);
                }
                keymap[ev.key.keysym.scancode] = 1;
            }
            else if(ev.type == SDL_KEYUP)
                keymap[ev.key.keysym.scancode] = 0;
            else if(ev.type == SDL_MOUSEMOTION)
            {
                const float sens = 0.3f;

                float pitch = ev.motion.yrel * sens;
                float yaw   = ev.motion.xrel * sens;

                m_camera->rotate(glm::vec3(0, 1, 0), yaw);
                m_camera->rotate(glm::vec3(-1, 0, 0), pitch);
            }
            else if(ev.type == SDL_MOUSEBUTTONDOWN)
            {
                if(ev.button.button == SDL_BUTTON_LEFT)
                {
                    if(!lastPosValid)
                        continue;
                    std::string cid = asString(glm::ivec3(ceil(lastPos.x/4.f), ceil(lastPos.y/4.f), ceil(lastPos.z/4.f)));
                    Chunk *ch = m_chunks[cid];
                    ch->setBlock(lastPos%4, 0);

                    if(m_clHandle)
                        m_clHandle->sendBlockUpdate(lastPos, 0);
                    else if(m_svHandle)
                        m_svHandle->sendBlockUpdate(lastPos, 0);
                }
            }
        }

        // camera logic ------------------------------------------------------------------------
        float mul = 1.f;
        if(keymap[SDL_SCANCODE_LCTRL])
            mul = 0.4f;
        else if(keymap[SDL_SCANCODE_LSHIFT])
            mul = 1.6f;

        float delta = 0.f;
        if(keymap[SDL_SCANCODE_W])
            delta = mul * 0.05f;
        else if(keymap[SDL_SCANCODE_S])
            delta = -mul * 0.05f;
        m_camera->move(delta * m_camera->fwdVector());

        delta = 0.f;
        if(keymap[SDL_SCANCODE_D])
            delta = mul * 0.05f;
        else if(keymap[SDL_SCANCODE_A])
            delta = -mul * 0.05f;
        m_camera->move(delta * glm::normalize(glm::cross(m_camera->fwdVector(), m_camera->getUpAxis())));

        if(keymap[SDL_SCANCODE_SPACE])
            m_camera->move(0.05f * m_camera->getUpAxis());
        // -------------------------------------------------------------------------------------

        if(m_clHandle || m_svHandle)
        {
            if(glm::length(m_selfInfo->pos - m_camera->getPos()) >= 0.1f ||
               glm::length(m_selfInfo->rot - glm::vec2(m_camera->getRot().x, m_camera->getRot().y)) >= .1f)
            {
                m_selfInfo->pos = m_camera->getPos();
                m_selfInfo->rot = glm::vec2(m_camera->getRot().x, m_camera->getRot().y);
                if(m_clHandle)
                    m_clHandle->sendPlayerInfo(m_selfInfo);
                else
                    m_svHandle->sendPlayerInfo(m_selfInfo);
            }
        }
        //
        glm::mat4 modelMatrix;
        // render chunks
        glBindVertexArray(cubeMdl->getVAO());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_texmgr->getArray("blocks"));
        for(int i=-4; i < 4; i++)
        {
            for(int j=-4; j < 4; j++)
            {
                for(int k = -4; k < 4; k++)
                {
                    glm::ivec3 chPos = glm::ivec3(curChunk.x + i,
                                                  curChunk.y + k,
                                                  curChunk.z + j);

                    std::string cid = asString(chPos);
                    if(m_chunks.find(cid) == m_chunks.end())
                        continue;

                    Chunk *ch = m_chunks[cid];

                    modelMatrix = glm::translate(glm::mat4(1.f), 4.f * glm::vec3(ch->getPos()));

                    int chunkPart[64];
                    for(int m = 0; m < 64; m++)
                        chunkPart[m] = ch->data()[m];

                    cubeShader->use();
                    cubeShader->setMat4("Proj", m_camera->GetProjection());
                    cubeShader->setMat4("View", m_camera->GetView());
                    cubeShader->setMat4("Model", modelMatrix);
                    cubeShader->setInt("palette", 1); // texture unit 1
                    cubeShader->setIntArray("chunk", chunkPart, 64);
                    glDrawArraysInstanced(GL_TRIANGLES, 0, cubeMdl->getSize(), 64);
                }
            }
        }

        // selection box
        if(lastPosValid)
        {
            modelMatrix = glm::translate(glm::mat4(1.f), glm::vec3(lastPos));
            selectionShader->use();
            cubeShader->setMat4("Proj", m_camera->GetProjection());
            cubeShader->setMat4("View", m_camera->GetView());
            cubeShader->setMat4("Model", modelMatrix);
            glDrawArrays(GL_LINES, 0, cubeMdl->getSize());
        }
        //

        glBindVertexArray(0);
        //

        //
        glBindVertexArray(monkeyMdl->getVAO());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texmgr->get("cobblestone"));
        m_playersLock.lock();
        //

        for(auto &p : m_players)
        {
            if(glm::length(p.second->pos - m_camera->getPos()) > 100)
                continue;

            glm::quat rot = glm::vec3(2*glm::pi<float>()-p.second->rot.x, p.second->rot.y+glm::radians(90.f), 0);
            modelMatrix = glm::translate(glm::mat4(1.f), p.second->pos) * glm::toMat4(rot);

            mainShader->use();
            mainShader->setMat4("Proj", m_camera->GetProjection());
            mainShader->setMat4("View", m_camera->GetView());
            mainShader->setMat4("Model", modelMatrix);
            glDrawArrays(GL_TRIANGLES, 0, monkeyMdl->getSize());

            glDisable(GL_CULL_FACE); // draw text

            glEnable(GL_CULL_FACE);
        }
        m_playersLock.unlock();
        glBindVertexArray(0);
        //

        //
        glDisable(GL_DEPTH_TEST);
        cursorShader->use();
        glBindVertexArray(m_cursorVAO);
            cursorShader->setFloat("aspect", (float)m_scrWidth / (float)m_scrHeight);
            glDrawArrays(GL_POINTS, 0, 1);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
        //

        m_camera->update();
        SDL_GL_SwapWindow(m_window);
        m_ticksElapsed++;
    }
    //
    /*
    for(auto &p : chunks)
        delete p.second;
    */
    //
    cleanup();
    return 0;
}

void GameWindow::cleanup()
{
    if(m_clHandle)
    {
        delete m_selfInfo;
        delete m_clHandle;
    }
    if(m_svHandle)
        delete m_svHandle;

    delete m_mdlmgr;
    delete m_texmgr;
    delete m_shmgr;

    SDL_GL_DeleteContext(m_glctx);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void GameWindow::loadConfig()
{
    m_shmgr->loadShader({"data/shaders/main.vert",
                         "data/shaders/main.frag"}, "main");
    m_shmgr->loadShader({"data/shaders/cube.vert",
                         "data/shaders/cube.frag"}, "cube");
    m_shmgr->loadShader({"data/shaders/cursor.vert",
                         "data/shaders/cursor.frag",
                         "data/shaders/cursor.geom"}, "cursor");
    m_shmgr->loadShader({"data/shaders/selectBlock.vert",
                         "data/shaders/selectBlock.frag"}, "selection");
    //
    m_texmgr->loadTex("data/textures/grass.png", "grass");
    m_texmgr->loadTex("data/textures/dirt.png", "dirt");
    m_texmgr->loadTex("data/textures/stone.png", "stone");
    m_texmgr->loadTex("data/textures/cobblestone.png", "cobblestone");
    m_texmgr->loadTex("data/textures/bedrock.png", "bedrock");

    m_texmgr->createArray("blocks",
                          {
                              {1, "stone"},
                              {2, "grass"},
                              {3, "dirt"},
                              {4, "cobblestone"},
                              {7, "bedrock"}
                          },
                          glm::ivec2(64, 64));
    //
    m_mdlmgr->loadModel("data/models/cube.obj", "cube");
    m_mdlmgr->loadModel("data/models/monkey.obj", "monkey");
}
