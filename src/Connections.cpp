
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

const QString & TCPConnection::GetIP() const
{
    return ipStr_;
}

TCPConnection::~TCPConnection()
{
    loop_ = false;
    if (connectionLoop_.joinable())
        connectionLoop_.join();

    // terminate connection
    tcpSocket_->abort();
}

void TCPConnection::LoopConnection()
{
    assert(tcpSocket_ && "tcpSocket_ is nullptr!");

    QDataStream in{ tcpSocket_.get() };
    in.setVersion(QDataStream::Qt_4_0);
    QString protocol{ "#M" };

    // TODO: implement
    while(loop_ &&
        tcpSocket_->state() != QAbstractSocket::ConnectedState)
    { 
        if (!tcpSocket_->waitForReadyRead(500))
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

        // check if message starts properly and that the counter is not even
        if (msgBegin != protocol ||
            !(counter % 2))
        {
            container_.AppendMessage(ipStr_ + QString(": protocol violation. Corrupted message."));
            continue;
        }

        if (message.isEmpty())
            continue;

        container_.AppendMessage(ipStr_ + QString(": ") + message);
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
