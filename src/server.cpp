
#include <algorithm>
#include <cassert>
#include <functional>

#include "ui_serverdialog.h"
#include "server.h"

int Container::rowCount(const QModelIndex & parent) const
{
    return messages_.size();
}

QVariant Container::data(const QModelIndex & index, int role) const
{
    std::shared_lock<std::shared_mutex> shlock{ mutexMsg_ };

    switch (role)
    {
    case Qt::DisplayRole:
        return messages_[index.row()];
    default:
        break;
    }
    return QVariant();
}

void Container::AppendMessage(const QString & msg)
{
    beginResetModel();
    {
        std::unique_lock<std::shared_mutex> ulock{ mutexMsg_ };
        // TODO: rigth shift vector
        std::rotate(messages_.rbegin(), std::next(messages_.rbegin()), messages_.rend());
        messages_[0] = msg;
    }
    endResetModel();
}


Server::Server(QWidget *parent)
    : QDialog(parent),
    ui_(std::make_unique<Ui_ServerDialog>())
{
    // setup ui
    ui_->setupUi(this);
    ui_->lineEditMsg->setPlaceholderText(QString("Enter your message..."));

    connect(ui_->btnSendMsg, &QAbstractButton::clicked, this, &Server::SendMessage);
    connect(ui_->lineEditMsg, &QLineEdit::returnPressed, this, &Server::SendMessage);
    connect(ui_->lineEditMsg, &QLineEdit::textChanged, this, &Server::EnableBtnSend);

    ui_->listViewMessages->setModel(&container_);

    container_.AppendMessage(QString("Server: begin log"));
    servermsgLoop_.Start();

}

Server::~Server()
{
}

void Server::SendMessage()
{
    QLineEdit& lineEditMsg = *ui_->lineEditMsg;
    container_.AppendMessage(QString("Server: ") + lineEditMsg.text());
    lineEditMsg.setText(QString());
}

void Server::EnableBtnSend(const QString & text)
{
    ui_->btnSendMsg->setEnabled(!text.isEmpty());
}

void ServerMsgLoop::SendCounter()
{
    container_.AppendMessage(QString("Server counter: ") + QString::number(counter_));
    counter_ += 2;
}

void ServerMsgLoop::Start()
{
    assert(end_ && "Loop is already running!");
    end_ = false;

    std::function<void()> runLoop = std::bind(&ServerMsgLoop::RunLoop, this);
    msgLoop_ = std::thread(runLoop);
}

void ServerMsgLoop::Stop()
{
    end_ = true;
    counter_ = 0;
}

void ServerMsgLoop::RunLoop()
{
    while (!end_)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        SendCounter();
    }
}

ServerMsgLoop::~ServerMsgLoop()
{
    if (!end_)
        Stop();
    msgLoop_.join();
}
