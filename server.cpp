#include "server.hpp"
#include "gamewindow.hpp"
#include <algorithm>
#include <thread>

// ----- RFC 1071 checksum algorithm --------
uint16_t checksum(void *addr, int count)
{
    long sum = 0;

    uint16_t *ptr16 = (uint16_t*)addr;
    int i = 0;
    while(count > 1)
    {
        sum += *ptr16++;
        i += 2;
        count -= 2;
    }

    if(count > 0)
        sum += ((uint8_t*)addr)[i];

    while(sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return ~sum;
}
// ------------------------------------------

Server *Server::serverInstance = nullptr;
static std::mutex clientsLock;

Server::Server(uint16_t port)
{
    Server::serverInstance = this;

    m_socket = new kissnet::tcp_socket(kissnet::endpoint("0.0.0.0", port));
    m_socket->bind();
    m_socket->listen();

    std::string addr = m_socket->get_bind_loc().address + ":" + std::to_string(m_socket->get_bind_loc().port);
    m_selfPID = checksum((void*)addr.c_str(), addr.size());

    std::thread(Server::clientHandler, m_socket).detach();
}

Server::~Server()
{
    m_clients.clear();
    m_socket->shutdown();
    delete m_socket;
}

void Server::sendPlayerInfo(PlayerInfo *inf)
{
    kissnet::buffer<64> data;
    data[0] = std::byte(0xC0);

    inf->pid = m_selfPID;
    uint8_t *binaryInfo = (uint8_t*)inf;

    for(size_t i=0; i < sizeof(PlayerInfo); i++)
        data[i+1] = std::byte(binaryInfo[i]);

    clientsLock.lock();
    for(kissnet::tcp_socket *cl : m_clients)
        cl->send(data);
    clientsLock.unlock();
}

void Server::sendBlockUpdate(const glm::ivec3 &pos, int bid)
{
    kissnet::buffer<64> data;
    data[0] = std::byte(0xB0);

    BlockUpdate bUpdate = {pos, bid};
    uint8_t *binaryInfo = (uint8_t*)&bUpdate;

    for(size_t i=0; i < sizeof(PlayerInfo); i++)
        data[i+1] = std::byte(binaryInfo[i]);

    clientsLock.lock();
    for(kissnet::tcp_socket *cl : m_clients)
        cl->send(data);
    clientsLock.unlock();
}

void Server::recvHandler(kissnet::tcp_socket &&sock, uint16_t pid)
{
    kissnet::tcp_socket *sockPtr = &sock;
    clientsLock.lock();
    Server::serverInstance->m_clients.push_back(sockPtr);
    clientsLock.unlock();

    kissnet::buffer<1024> data;
    while(sock.is_valid())
    {
        const auto [size, status] = sock.recv(data);

        if(size == 0)
            break;

        if(data[0] == std::byte(0xB0))
        {
            BlockUpdate *bUpdate = (BlockUpdate*)(data.data() + 1);

            GameWindow::gameInstance->updateBlock(bUpdate->pos, bUpdate->bid);
        }
        else if(data[0] == std::byte(0xC0))
        {
            PlayerInfo *pInfo = (PlayerInfo*)(data.data() + 1);

            GameWindow::gameInstance->m_playersLock.lock();
            GameWindow::gameInstance->updatePlayer(pid, pInfo->pos, pInfo->rot);
            GameWindow::gameInstance->m_playersLock.unlock();
        }
    }
    clientsLock.lock();
    Server::serverInstance->m_clients.remove(sockPtr);
    clientsLock.unlock();
    fprintf(stderr, "[%04X] Disconnected\n", pid);
    GameWindow::gameInstance->m_playersLock.lock();
    GameWindow::gameInstance->removePlayer(pid);
    GameWindow::gameInstance->m_playersLock.unlock();
}

void Server::clientHandler(kissnet::tcp_socket *sock)
{
    while(sock->is_valid())
    {
        try
        {
            auto client = sock->accept();

            std::string addr = client.get_bind_loc().address + ":" + std::to_string(client.get_bind_loc().port);
            uint16_t pid = checksum((void*)addr.c_str(), addr.size());

            GameWindow::gameInstance->m_playersLock.lock();
            GameWindow::gameInstance->spawnPlayer(pid);
            GameWindow::gameInstance->m_playersLock.unlock();

            kissnet::buffer<8> data;
            data[0] = std::byte(0xAA); // pid assign
            data[1] = std::byte(pid>>8);
            data[2] = std::byte(pid&0xFF);

            uint32_t seed = GameWindow::m_seed;
            data[3] = std::byte(seed >> 24);
            data[4] = std::byte(seed >> 16);
            data[5] = std::byte(seed >> 8);
            data[6] = std::byte(seed & 0xFF);
            client.send(data);

            std::thread(recvHandler, std::move(client), pid).detach();
        }
        catch(std::runtime_error)
        {
            break;
        }
    }
}

uint16_t Server::getPID() const
{
    return m_selfPID;
}
