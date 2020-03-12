
#include <cassert>

#include "client.h"

#include "qtcpsocket.h"

Connection::Connection(Client *client)
    : tcpSocket_(std::make_unique<QTcpSocket>()),
    out_(tcpSocket_.get())
{
    connect(tcpSocket_.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
        client, &Client::ConnectionError);
    out_.setVersion(QDataStream::Qt_4_0);
}

void Connection::SendMessage(const QString & msg)
{
    // is there any difference between using the byte array or a << operator for io device?
    QByteArray block;
    QDataStream out{ &block, QIODevice::WriteOnly };
    out.setVersion(QDataStream::Qt_4_0);
    out << "#M" << counter_ << msg;

    std::lock_guard<std::mutex> lg{ socketMutex_ };
    tcpSocket_->write(block);
    tcpSocket_->waitForBytesWritten(500);
    
    counter_ += 2;
}

bool Connection::Connect(const QString & ipAddress, quint16 port)
{
    tcpSocket_->connectToHost(ipAddress, port);
    if (tcpSocket_->waitForConnected(3000))
        return true;

    return false;
}

const bool Connection::IsConnected() const
{
    return tcpSocket_->state() == QAbstractSocket::ConnectedState;
}

void Connection::Disconnect()
{
    assert(IsConnected() && "Not connected!");

    tcpSocket_->disconnectFromHost();
    tcpSocket_->waitForDisconnected();
}

Connection::~Connection()
{
}

