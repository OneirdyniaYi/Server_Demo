#include "BaseServer.h"

int BaseServer::init()
{
    if (m_server_socket->open_as_server(m_port, const_cast<char *>(m_ip.c_str())) < 0)
    {
        printf("[Common][BaseServer.cpp:%d][ERROR]:m_server_socket->open_as_server failed\n", __LINE__);
        return fail;
    }

    if (m_epoll.epoll_init(MAX_SOCKET_COUNT) < 0)
    {
        printf("[Common][BaseServer.cpp:%d][ERROR]:epoll_init failed\n", __LINE__);
        return fail;
    }
    printf("[Common][BaseServer.cpp:%d][INFO]:Server Epoll Has Inited...\n", __LINE__);

    if (m_epoll.epoll_add(m_server_socket->get_fd()) < 0)
    {
        printf("[Common][BaseServer.cpp:%d][ERROR]:m_epoll.epoll_add fd:%d failed\n", __LINE__, m_server_socket->get_fd());
        return fail;
    }

    m_sockets_map[m_server_socket->get_fd()] = m_server_socket;
    return success;
}

int BaseServer::epoll_recv()
{
    int fd_count = m_epoll.epoll_wait(-1);
    for (int i = 0; i < fd_count; i++)
    {
        struct epoll_event *pstEvent = m_epoll.get_event(i);
        int socketfd = pstEvent->data.fd;
        std::shared_ptr<TCPSocket> pstSocket = m_sockets_map[socketfd];
        if (pstSocket == NULL || pstSocket->get_fd() < 0)
        {
            printf("[Common][BaseServer.cpp:%d][ERROR]:get_server_tcpsocket failed fd:%d\n", __LINE__, socketfd);
            return fail;
        }

        if (pstSocket->get_fd() == m_server_socket->get_fd())
        {
            printf("[Common][BaseServer.cpp:%d][INFO]:Listening Fd Has New Fd\n", __LINE__);
            int accepted_sockfd = pstSocket->accept_fd(socketfd);
            if (accepted_sockfd < 0)
            {
                printf("[Common][BaseServer.cpp:%d][ERROR]:accept_fd  failed\n", __LINE__);
                continue;
            }

            m_sockets_map[accepted_sockfd] = make_shared<TCPSocket>(accepted_sockfd);

            if (m_epoll.epoll_add(accepted_sockfd) < 0)
            {
                printf("[Common][BaseServer.cpp:%d][ERROR]:m_epoll.epoll_add fd:%d failed\n", __LINE__, m_server_socket->get_fd());
                return fail;
            }
        }
        else
        {
            int ret = pstSocket->process_data(std::bind(&BaseServer::parsing_and_send, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            if (ret == -3)
            {
                printf("[Common][BaseServer.cpp:%d][WARNING]:socket fd:[%d] Has CLosed\n", __LINE__, socketfd);
                m_sockets_map.erase(socketfd);
            }
            else if (ret)
            {
                printf("[Common][BaseServer.cpp:%d][ERROR]:socket process_data failed fd:%d\n", __LINE__, pstSocket->get_fd());
                return ret;
            }
        }
    }
    return 0;
}