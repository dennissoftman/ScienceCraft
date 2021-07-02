#include "client.hpp"
#include "server.hpp"
#include "gamewindow.hpp"
#include <thread>

uint16_t Client::m_selfPID = 0;

Client::Client(const std::string &ip, uint16_t port)
{
    m_socket = new kissnet::tcp_socket(kissnet::endpoint(ip, port));
    m_socket->connect();

    //
    std::thread(Client::recvHandler, m_socket).detach();
}

void Client::recvHandler(kissnet::tcp_socket *sock)
{
    kissnet::buffer<1024> data;
    while(sock->is_valid())
    {
        const auto [size, status] = sock->recv(data);

        if(size == 0)
            break;

        if(data[0] == std::byte(0xAA)) // PID and seed
        {
            Client::m_selfPID = ((uint16_t)data[1] << 8) | (uint16_t)data[2];
            uint32_t seed = ((uint32_t)data[3] << 24) |
                            ((uint32_t)data[4] << 16) |
                            ((uint32_t)data[5] << 8) |
                             (uint32_t)data[6];
            GameWindow::m_seed = seed;
            srand(GameWindow::m_seed);
            GameWindow::gameInstance->regenerateWorld();
        }
        else if(data[0] == std::byte(0xB0)) // block update
        {
            BlockUpdate *bUpdate = (BlockUpdate*)(data.data() + 1);

            GameWindow::gameInstance->updateBlock(bUpdate->pos, bUpdate->bid);
        }
        else if(data[0] == std::byte(0xC0)) // player update
        {
            PlayerInfo *pInfo = (PlayerInfo*)(data.data()+1);
            if(pInfo->pid == Client::m_selfPID)
                continue;

            GameWindow::gameInstance->m_playersLock.lock();
            GameWindow::gameInstance->updatePlayer(pInfo->pid, pInfo->pos, pInfo->rot);
            GameWindow::gameInstance->m_playersLock.unlock();
        }
    }
}

void Client::sendPlayerInfo(PlayerInfo *inf)
{
    kissnet::buffer<64> data;
    data[0] = std::byte(0xC0); // update

    inf->pid = Client::m_selfPID;
    uint8_t *binaryInfo = (uint8_t*)inf;

    for(size_t i=0; i < sizeof(PlayerInfo); i++)
        data[i+1] = std::byte(binaryInfo[i]);

    m_socket->send(data);
}

void Client::sendBlockUpdate(const glm::ivec3 &pos, int bid)
{
    kissnet::buffer<64> data;
    data[0] = std::byte(0xB0); // block update

    BlockUpdate bUpdate = {pos, bid};
    uint8_t *binaryInfo = (uint8_t*)&bUpdate;

    for(size_t i=0; i < sizeof(PlayerInfo); i++)
        data[i+1] = std::byte(binaryInfo[i]);

    m_socket->send(data);
}

uint16_t Client::getPID() const
{
    return Client::m_selfPID;
}

Client::~Client()
{
    m_socket->shutdown();
    delete m_socket;
}

