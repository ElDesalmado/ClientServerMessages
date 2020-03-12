#include "client.h"

#include "ui_clientdialog.h"

#include "qvalidator.h"
#include "qmessagebox.h"

Client::Client(QWidget *parent)
    : QDialog(parent),
    ui_(SetupUI(this)),
    connection(this)
{

}

Client::~Client()
{
}

std::unique_ptr<Ui_ClientDialog> Client::SetupUI(Client * client)
{
    std::unique_ptr<Ui_ClientDialog> ui = std::make_unique<Ui_ClientDialog>();
    ui->setupUi(client);

    ui->lineServerPort->setPlaceholderText("Enter Port");
    ui->lineServerPort->setValidator(new QIntValidator(1, 65535, client));

    ui->lineServerIP->setPlaceholderText("Enter IP address or \"localhost\"");
    
    ui->lineEditMsg->setPlaceholderText("Enter your message...");

    // lineEdits
    connect(ui->lineServerPort, &QLineEdit::textChanged, client, &Client::ValidateIpAndPort);
    connect(ui->lineServerIP, &QLineEdit::textChanged, client, &Client::ValidateIpAndPort);
    connect(ui->lineEditMsg, &QLineEdit::returnPressed, client, &Client::SendMsg);

    // buttons
    connect(ui->btnConnect, &QAbstractButton::clicked, client, &Client::SwitchConnection);
    connect(ui->btnSend, &QAbstractButton::clicked, client, &Client::SendMsg);
 

    return std::move(ui);
}

void Client::ValidateIpAndPort()
{
    // TODO: IP validation??
    ui_->btnConnect->setEnabled(!ui_->lineServerIP->text().isEmpty() &&
        !ui_->lineServerPort->text().isEmpty());
}

void Client::EnableMsgInput()
{
    ui_->lineEditMsg->setEnabled(true);
    ui_->btnSend->setEnabled(true);
}

void Client::DisableMsgInput()
{
    ui_->lineEditMsg->setEnabled(false);
    ui_->btnSend->setEnabled(false);
}

void Client::SwitchConnection()
{
    QString ipAddress = ui_->lineServerIP->text(),
        portStr = ui_->lineServerPort->text();
    assert(!ipAddress.isEmpty() || portStr.isEmpty());

    ui_->btnConnect->setEnabled(false);

    if (connection.IsConnected())
    {
        connection.Disconnect();
        SetUiStateDisconnected();
    }
    else if (connection.Connect(ipAddress, portStr.toInt()))
        SetUiStateConnected();
   
    ui_->btnConnect->setEnabled(true);
}

void Client::SendMsg()
{
    assert(connection.IsConnected());

    connection.SendMessage(ui_->lineEditMsg->text());
    ui_->lineEditMsg->clear();
}

void Client::SetUiStateConnected()
{
    EnableMsgInput();
    ui_->labelStatus->setText("Status: connected");
    ui_->btnConnect->setText("Disconnect");
    ui_->lineServerIP->setEnabled(false);
    ui_->lineServerPort->setEnabled(false);
}

void Client::SetUiStateDisconnected()
{
    DisableMsgInput();
    ui_->labelStatus->setText("Status: not connected");
    ui_->btnConnect->setText("Connect");
    ui_->lineServerIP->setEnabled(true);
    ui_->lineServerPort->setEnabled(true);
}

// this does not fire for some reason
void Client::ConnectionError(QAbstractSocket::SocketError error)
{
    switch (error)
    {
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Connection error"),
            tr("The host was not found. Please check the "
                "IP address and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Connection error"),
            tr("The connection was refused by the peer. "
                "Make sure the server is running, "
                "and check that the IP address and port "
                "settings are correct."));
        break;
    case QAbstractSocket::UnknownSocketError:
        QMessageBox::information(this, tr("Connection error"),
            tr("Something went wrong! Perhabs, the server is not running at current address."));
        break;
    case QAbstractSocket::RemoteHostClosedError:
        QMessageBox::information(this, tr("Connection error"),
            tr("The connection has been closed by the remote host."));
        break;
    default:
        break;
    }
    SetUiStateDisconnected();
}
