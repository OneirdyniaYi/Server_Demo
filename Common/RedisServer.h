#ifndef REDISSERVER_H_
#define REDISSERVER_H_

#include <vector>
#include <string>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <hiredis/hiredis.h>
class RedisServer
{
public:
    RedisServer(const std::string &ip, int port) : m_ip(ip), m_port(port), m_pcmd(new char[20])
    {
    }

    int GetByBit(const std::string &_key, void **_buf, int *_len)
    {
        snprintf(m_pcmd, 20, "GET %s", _key.c_str());
        m_reply = (redisReply *)redisCommand(m_context, m_pcmd);

        if (m_reply == nullptr || m_reply->type == REDIS_REPLY_ERROR)
        {
            // error
            std::cout << "get faild" << std::endl;
            Restart();
            return -1;
        }
        std::cout << "get sucess" << std::endl;
        *_buf = m_reply->str;
        *_len = m_reply->len;
        return 0;
    }

    ~RedisServer()
    {
        DisConnect();
        FreeReply();
        delete[] m_pcmd;
        std::cout << "toredis dtor!" << std::endl;
    }

    int Restart()
    {
        DisConnect();
        FreeReply();
        if (Connect())
        {
        }
    }

    bool Connect()
    {
        m_context = redisConnect(m_ip.c_str(), m_port);

        if (m_context == nullptr || m_context->err)
        {
            // error tracer log
            std::cout << "connect faild" << std::endl;
            return false;
        }
        // demo 用的 redis 没有设置验证
        std::cout << "connect sucess" << std::endl;
        return true;
    }

    bool DisConnect()
    {
        if (m_context)
        {
            redisFree(m_context);
            m_context = nullptr;
        }
        return true;
    }

    int FreeReply()
    {
        if (m_reply)
        {
            freeReplyObject(m_reply);
            m_reply = nullptr;
        }
        return 0;
    }

    int SetByBit(const std::string &_key, void *_buf, int _len)
    {
        std::cout << "start set\n";
        snprintf(m_pcmd, 20, "SET %s \%b", _key.c_str());
        std::cout << "cmd is " << m_pcmd << std::endl;
        m_reply = (redisReply *)redisCommand(m_context, m_pcmd, _buf, _len);
        std::cout << "redisCommand failed";
        if (m_reply == nullptr || m_reply->type == REDIS_REPLY_ERROR)
        {
            // error
            std::cout << "set faild" << std::endl;
            Restart();
            return -1;
        }
        std::cout << "set sucess" << std::endl;
        return 0;
    }

private:
    redisReply *m_reply;
    redisContext *m_context;
    char *m_pcmd;
    std::string m_ip;
    int m_port;
};
#endif