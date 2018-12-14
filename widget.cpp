#include "widget.h"
#include "ui_widget.h"

#include <QDateTime>
#include <QUrl>
#include <QVariant>
#include <QAuthenticator>
#include <QMessageBox>
#include <QDebug>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    QDateTime DateTime = QDateTime::currentDateTime();
    setWindowTitle("QHttp-Tool - " + DateTime.toString("yyyy.MM.dd"));

    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(managerFinished(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
                this, SLOT(authRequiredReply(QNetworkReply*,QAuthenticator*)));
    /* send button */
    connect(ui->sendBtn, SIGNAL(clicked(bool)), this, SLOT(sendBtnClicked()));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::managerFinished(QNetworkReply *reply)
{
    QByteArray byteArray = reply->readAll();
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.isValid()) {
        qDebug() << "recv finished: " << statusCode.toInt();
    }

    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "recv data: " << byteArray;
        ui->recvBodyTextBrowser->setText(byteArray);
    } else {
        qDebug() << "reply error: " << reply->error();
    }

    reply->deleteLater();
}

void Widget::sendBtnClicked(void)
{
    QString urlString = ui->urlLineEdit->text();
    if (urlString.isEmpty()) {
        QMessageBox::information(this, tr("Error"), tr("Please input valid URL!"));
        return ;
    }

    QUrl url(urlString);
    manager->get(QNetworkRequest(url));
    qDebug() << "Send finished url: " << urlString;
}

void Widget::authRequiredReply(QNetworkReply *, QAuthenticator *auth)
{
    QString username = ui->usernameLineEdit->text();
    QString password =ui->passwordLineEdit->text();

    qDebug() << "username: " << username << "password: " << password;
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::information(this, tr("Error"), tr("Please input valid username and passowrd!"));
        return ;
    }

    auth->setUser(username);
    auth->setPassword(password);
}
