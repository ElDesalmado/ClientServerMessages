
#include "server.h"

#include "qtcpsocket.h"
#include "qhostaddress.h"

#include "qlistwidget.h"

TCPConnection::TCPConnection(Container& container, QTcpSocket * tcpSocket)
    : container_(container),
    tcpSocket_(tcpSocket),
    ipStr_(tcpSocket_->peerAddress().toString()),
    connectionLoop_(std::bind(&TCPConnection::LoopConnection, this))
{
}

/*
TCPConnection::TCPConnection(TCPConnection&& other)
    : container_(other.container_),
    tcpSocket_(std::move(other.tcpSocket_)),
    ipStr_(std::move(other.ipStr_)),
    connectionLoop_(std::move(other.connectionLoop_))
{
    loop_ = other.loop_.load();
    other.loop_ = false;
}
*/

const QString & TCPConnection::GetIP() const
{
    return ipStr_;
}


TCPConnection::~TCPConnection()
{
    /*
    if (loop_)
    {
        loop_ = false;
        connectionLoop_.join();
    }

    if (!tcpSocket_)
        return;
    */

    loop_ = false;
    connectionLoop_.join();

    // terminate connection
    tcpSocket_->abort();
}

void TCPConnection::LoopConnection()
{
    // TODO: implement
    while(loop_)
    { 
        if (!tcpSocket_->waitForReadyRead(500))
        {
            container_.AppendMessage(ipStr_ + QString(": counter timeout..."));
            continue;
        }

        // receive message
        // check message
        // append msg to the container

    }
}


Connections::Connections(QListWidget & listConnections)
    : listConnections_(listConnections)
{}

void Connections::AddConnection(Container& container, QTcpSocket *tcpSocket)
{
    connections_.emplace_back(container, tcpSocket);
    listConnections_.addItem(connections_.back().GetIP());
}

void Connections::TerminateConnection(size_t indx)
{
    assert(indx != -1 && "Invalid index!");

    decltype(connections_)::const_iterator iter = 
        std::next(connections_.cbegin(), indx);
    connections_.erase(iter);
}

Connections::~Connections()
{
}
