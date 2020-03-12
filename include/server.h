#pragma once

#include <qdialog.h>
#include "qabstractitemmodel.h"

#include "qvariant.h"

#include <vector>
#include <shared_mutex>
#include <atomic>
#include <thread>

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
    std::atomic_bool end_;
    Container& container_;

    std::thread msgLoop_;

    int counter_ = 0;

public:

    ServerMsgLoop(Container& container)
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

class Server : public QDialog
{
    Q_OBJECT

private:
    std::unique_ptr<Ui_ServerDialog> ui_;
    Container container_;
    ServerMsgLoop servermsgLoop_{ container_ };

public:
    explicit Server(QWidget *parent = nullptr);
    ~Server();

private:
    void SendMessage();

    void EnableBtnSend(const QString& text);

};

