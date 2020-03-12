
#include <algorithm>
#include <cassert>
#include <functional>

#include "ui_serverdialog.h"
#include "server.h"

#include "qtcpserver.h"
#include "qtcpsocket.h"
#include "qnetworksession.h"

#include "qmessagebox.h"

int Container::rowCount(const QModelIndex & parent) const
{
    std::shared_lock<std::shared_mutex> shlock{ mutexMsg_ };
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
    std::unique_lock<std::shared_mutex> ulock{ mutexMsg_ };
    beginResetModel();

    std::rotate(messages_.rbegin(), std::next(messages_.rbegin()), messages_.rend());
    messages_[0] = msg;

    endResetModel();
}


Server::Server(QWidget *parent)
    : QDialog(parent),
    ui_(SetupUi(this)),
    connections_(*ui_->listConnections),
    tcpServer_(std::make_unique<QTcpServer>())
{
    // setup ui
    //ui_->setupUi(this);



    ui_->listViewMessages->setModel(&container_);

    // Start TCP Server
    if (!tcpServer_->listen(QHostAddress::Any, 69))
    {
        QMessageBox::critical(this, tr("Server Error."),
            tr("Unable to start the server: %1").arg(tcpServer_->errorString()));
        close();
        return;
    }

    QString ipAddress;

    QList<QHostAddress> ipAddrList = QNetworkInterface::allAddresses();
    for (const QHostAddress& addr : ipAddrList)
        if (addr != QHostAddress::LocalHost &&
            addr.toIPv4Address())
        {
            ipAddress = addr.toString();
            break;
        }

    if (ipAddrList.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

    ui_->labelServerStatus->setText(tr("Server IP: %1 Port: %2")
        .arg(ipAddress)
        .arg(tcpServer_->serverPort()));

    connect(tcpServer_.get(), &QTcpServer::newConnection, this, &Server::NewConnection);

    container_.AppendMessage(QString("Server: begin log"));
    servermsgLoop_.Start();

}

Server::~Server()
{
}

std::unique_ptr<Ui_ServerDialog> Server::SetupUi(Server * dialog)
{
    assert(dialog && "Dialog is nullptr!");
    auto ptr = std::make_unique<Ui_ServerDialog>();

    ptr->setupUi(dialog);
    ptr->lineEditMsg->setPlaceholderText(QString("Enter your message..."));
    ptr->listViewMessages->setModel(&dialog->container_);

    connect(ptr->btnSendMsg, &QAbstractButton::clicked, dialog, &Server::SendMessage);
    connect(ptr->lineEditMsg, &QLineEdit::returnPressed, dialog, &Server::SendMessage);
    connect(ptr->lineEditMsg, &QLineEdit::textChanged, dialog, &Server::EnableBtnSend);
    connect(ptr->listConnections, &QListWidget::itemSelectionChanged, dialog, &Server::ConnectionSelected);
    connect(ptr->btnTerminate, &QAbstractButton::clicked, dialog, &Server::TerminateSelectedConnections);

    return std::move(ptr);
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

void Server::ConnectionSelected()
{
    ui_->btnTerminate->setEnabled(!ui_->listConnections->selectedItems().isEmpty());
}

void Server::NewConnection()
{
    connections_.AddConnection(container_, 
        tcpServer_->nextPendingConnection());
}

void Server::TerminateSelectedConnections()
{
    QListWidget& listConnections = *ui_->listConnections;
    QList<QListWidgetItem*> selected = listConnections.selectedItems();
    for (QListWidgetItem *item : selected)
    {
        // index in the widget is the same as in connections_
        int indx = listConnections.row(item);

        // delete to remove from the widget list, this will update indices
        delete item;

        connections_.TerminateConnection(indx);
    }
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
    if (msgLoop_.joinable())
        msgLoop_.join();
}
 
