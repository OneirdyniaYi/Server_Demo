
#ifndef TCPSOCKET_H
#define TCPSOCKET_H
#include <functional>
#include "base.h"
#include "Buffer.h"
#include "util.h"
using namespace std;

class TCPSocket
{
public:
    TCPSocket();
    TCPSocket(int fd);
    ~TCPSocket();

private:
    TCPSocket(const TCPSocket &socket);
    TCPSocket &operator=(const TCPSocket &socket);

public:
    void close_socket();

    int recv_data();

    int process_data(std::function<int(const char *, const int, int)> callback);

    int send_data(char *data, size_t size);

    int get_fd();

    int open_as_server(uint16_t port, char *ip = NULL);

    int open_as_client(char *localIP = NULL, uint16_t localPort = 0, int buffLen = -1);

    int connect_to(u_long ip, uint16_t port, bool nonblock = true, int msecond = 100);

    int accept_fd(int fd);

    void set_socket_fd(int fd);

protected:
    int m_fd;
    //receiving buffer
    std::shared_ptr<Buffer> m_buffer;
};
#endif