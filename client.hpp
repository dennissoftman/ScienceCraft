#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <kissnet.hpp>
#include <glm/glm.hpp>

struct PlayerInfo;

class Client
{
public:
    Client(const std::string &ip, uint16_t port);
    ~Client();

    static void recvHandler(kissnet::tcp_socket *sock);

    void sendPlayerInfo(PlayerInfo *inf);
    void sendBlockUpdate(const glm::ivec3 &pos, int bid);

    uint16_t getPID() const;
private:
    kissnet::tcp_socket *m_socket;

    static uint16_t m_selfPID;
};

#endif // CLIENT_HPP
