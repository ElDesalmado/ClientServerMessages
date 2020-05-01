#pragma once

#include <qdialog.h>
#include "qabstractitemmodel.h"

#include "qvariant.h"

#include <vector>
#include <list>
#include <shared_mutex>
#include <atomic>
#include <thread>
#include <optional>

class Container : public QAbstractListModel
{
    std::vector<QString> messages_{ 100 };
    mutable std::shared_mutex mutexMsg_;

public:
    // Inherited via QAbstractListModel
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

    void AppendMessage(const QString& msg);
};

class ServerMsgLoop
{
    std::atomic_bool end_{false};
    Container& container_;

    std::thread msgLoop_;

    int counter_ = 0;

public:

    explicit ServerMsgLoop(Container& container)
        : container_(container)
    {}

    void SendCounter();

    void Start();
    void Stop();

   
    ~ServerMsgLoop();

private:
    void RunLoop();

};


class Ui_ServerDialog;
class QTcpServer;
class QNetworkSession;
class QTcpSocket;
class QListWidget;

class TCPConnection : public QObject
{
    Q_OBJECT

    Container& container_;
    std::unique_ptr<QTcpSocket> tcpSocket_;
    QString ipStr_;

    std::atomic_bool loop_ = true;
    std::thread connectionLoop_;

public:
    TCPConnection(Container& container, QTcpSocket *tcpSocket);

    /* this class is not movable due to inability to properly move the
    connection thread: LoopConnection functor will hold a invalid pointer.*/
    TCPConnection(const TCPConnection&) = delete;
    //TCPConnection(TCPConnection&& other) = delete;

    const QString& GetIP() const;
    bool IsRunning() const;

    void Stop();

    ~TCPConnection();

    signals:

    void Disconnected(TCPConnection *connection);

private:

    void LoopConnection();
    void onDisconnected();

};

class Connections
{
    // TODO: remove connection if client have disconnected

    // this reference is used to append new ip names
    QListWidget& listConnections_;

    std::list<TCPConnection> connections_;

public:
    explicit Connections(QListWidget& listConnections);


    TCPConnection * AddConnection(Container& container, QTcpSocket *tcpSocket);
    std::optional<size_t> ConnectionIndx(TCPConnection *pConnection) const;
    void TerminateConnection(size_t indx);

    ~Connections();
};



class Server : public QDialog
{
    Q_OBJECT

private:
    Container container_;

    std::unique_ptr<Ui_ServerDialog> ui_;
    ServerMsgLoop servermsgLoop_{ container_ };

    // tcp server owns all the existing sockets, so it should be deleted 
    // after all the connections have been terminated
    std::unique_ptr<QTcpServer> tcpServer_;
    std::unique_ptr<QNetworkSession> networkSession_;

    Connections connections_;



public:
    explicit Server(QWidget *parent = nullptr);
    ~Server() override;

private:

    static std::unique_ptr<Ui_ServerDialog> SetupUi(Server *dialog);

    void SendMessage();
    void EnableBtnSend(const QString& text);
    void ConnectionSelected();
    void NewConnection();
    void TerminateSelectedConnections();

    void onDisconnected(TCPConnection *pConnection);
};

