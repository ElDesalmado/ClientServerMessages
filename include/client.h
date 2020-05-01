#pragma once

#include "qdialog.h"
#include "qabstractsocket.h"
#include "qdatastream.h"

#include <memory>
#include <atomic>
#include <thread>
#include <mutex>

class Ui_ClientDialog;


class QTcpSocket;
class Client;

class Connection : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<QTcpSocket> tcpSocket_;
    QDataStream out_;
    std::atomic<int> counter_ = 1;
    std::atomic_bool loop_ = true;
    mutable std::mutex socketMutex_;

    std::thread msgLoop_;

public:

    Connection(Client *client);

    void SendMessage(const QString& msg = QString());
    bool Connect(const QString& ipAddress, quint16 port);
    const bool IsConnected() const;

    void Disconnect();

    ~Connection();

//signals:

   // void Connected();
   // void Disconnected();

private:
    void RunLoop();
};

class Client : public QDialog
{
    Q_OBJECT

private:
    
    std::unique_ptr<Ui_ClientDialog> ui_;
    Connection connection;

public:
    explicit Client(QWidget *parent = nullptr);

    ~Client();

private:
    static std::unique_ptr<Ui_ClientDialog> SetupUI(Client *client);

    void ValidateIpAndPort();
    void EnableMsgInput();
    void DisableMsgInput();

    void SwitchConnection();

    void SendMsg();

    void SetUiStateConnected();
    void SetUiStateDisconnected();

public:
    void ConnectionError(QAbstractSocket::SocketError error);
};
