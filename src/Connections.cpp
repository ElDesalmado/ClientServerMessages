
#include "server.h"

#include "qtcpsocket.h"
#include "qhostaddress.h"

#include "qlistwidget.h"

#include <functional>
#include <cassert>

TCPConnection::TCPConnection(Container& container, QTcpSocket * tcpSocket)
    : container_(container),
    tcpSocket_(tcpSocket),
    ipStr_(tcpSocket_->peerAddress().toString()),
    connectionLoop_(std::bind(&TCPConnection::LoopConnection, this))
{
    connect(tcpSocket, &QTcpSocket::disconnected, this, &TCPConnection::onDisconnected);
}

const QString & TCPConnection::GetIP() const
{
    return ipStr_;
}

void TCPConnection::Stop()
{
    assert(loop_ && "Loop is not running!");
    loop_ = false;

    if (connectionLoop_.joinable())
        connectionLoop_.join();
}

TCPConnection::~TCPConnection()
{
    // terminate connection
    if (IsRunning())
    {
        Stop();
        tcpSocket_->abort();
    }
}

void TCPConnection::LoopConnection()
{
    assert(tcpSocket_ && "tcpSocket_ is nullptr!");

    QDataStream in { tcpSocket_.get() };
    in.setVersion(QDataStream::Qt_4_0);
    QString protocol{ "#M" };

    // TODO: implement
    while(loop_)
    { 
        // throws when client disconnects
        if (!tcpSocket_->waitForReadyRead(2000))
        {
            container_.AppendMessage(ipStr_ + QString(": counter timeout..."));
            continue;
        }

        // receive message
        QString msgBegin,
            message;
        int counter;

        in.startTransaction();
        in >> msgBegin >> counter >> message;

        if (!in.commitTransaction())
            continue;

        // for an unknown reason the execution does not go beyond this point

        // check if message starts properly and that the counter is not even
        if (msgBegin != protocol ||
            !(counter % 2))
        {
            container_.AppendMessage(ipStr_ + QString(": protocol violation. Corrupted message."));
            continue;
        }

        if (message.isEmpty())
        {
            container_.AppendMessage(ipStr_ + QString(" counter: ") + QString::number(counter));
            continue;
        }

        container_.AppendMessage(ipStr_ + QString(": ") + message);
    }

}

bool TCPConnection::IsRunning() const
{
    return loop_;
}

void TCPConnection::onDisconnected()
{
    emit Disconnected(this);
}


Connections::Connections(QListWidget & listConnections)
    : listConnections_(listConnections)
{}


TCPConnection * Connections::AddConnection(Container& container, QTcpSocket *tcpSocket)
{
    return &connections_.emplace_back(container, tcpSocket);

    // listConnections_.addItem(connections_.back().GetIP());
}

void Connections::TerminateConnection(size_t indx)
{
    assert(indx < connections_.size() && "Invalid index!");

    decltype(connections_)::iterator iter =
        std::next(connections_.begin(), indx);

    iter->Stop();
    connections_.erase(iter);

}

Connections::~Connections()
{
}

std::optional<size_t> Connections::ConnectionIndx(TCPConnection *pConnection) const
{
    auto found = std::find_if(connections_.cbegin(), connections_.cend(),
            [&](const TCPConnection& connection)
            {
                return &connection == pConnection;
            });
    if (found == connections_.cend())
        return std::nullopt;

    return std::distance(connections_.cbegin(), found);
}


