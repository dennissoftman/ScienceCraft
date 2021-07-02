#ifndef SERVER_HPP
#define SERVER_HPP

#include <kissnet.hpp>
#include <glm/glm.hpp>
#include <list>

uint16_t checksum(void *addr, int count);

struct PlayerInfo;

class Server
{
public:
    Server(uint16_t port=25565);
    ~Server();

    void sendPlayerInfo(PlayerInfo *inf);
    void sendBlockUpdate(const glm::ivec3 &pos, int bid);

    static void recvHandler(kissnet::tcp_socket &&sock, uint16_t pid);
    static void clientHandler(kissnet::tcp_socket *sock);

    uint16_t getPID() const;

    static Server *serverInstance;
private:
    kissnet::tcp_socket *m_socket;
    uint16_t m_selfPID;
    std::list<kissnet::tcp_socket*> m_clients;
};

#endif // SERVER_HPP
