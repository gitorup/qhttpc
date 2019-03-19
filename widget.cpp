#include "widget.h"
#include "ui_widget.h"

#include <QDateTime>
#include <QUrl>
#include <QUrlQuery>
#include <QVariant>
#include <QAuthenticator>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>
#include <QDialog>
#include <QList>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    QDateTime DateTime = QDateTime::currentDateTime();
    setWindowTitle("QHttp-Tool - " + DateTime.toString("yyyy.MM.dd"));

    QString styleSheet = tr("QPushButton {"
                                "background-color: rgb(70, 140, 255);"
                                "color: white;"
                                "border-radius: 5px;"
                                "outline: none;"
                            "}"
                            "QPushButton:pressed {"
                                "background-color: rgb(35, 90, 157);"
                            "}"
                            "QComboBox {"
                                "border-width: 1px;"
                                "border-radius: 5px;"
                                "border: 1px solid gray;"
                            "}"
                            "QLineEdit {"
                                "border-width: 1px;"
                                "border-radius: 4px;"
                                "font-size: 12px;"
                                "color: black;"
                                "border: 1px solid gray;}"
                           "QLineEdit:hover {"
                                "border-width:1px;"
                                "border-radius:4px;"
                                "font-size:12px;"
                                "color:black;"
                                "border:1px solid rgb(70, 140, 255);"
                           "}");

    setStyleSheet(styleSheet);
    ui->usernameLineEdit->setText(tr("WeEyE"));
    ui->passwordLineEdit->setText(tr("&$ChuTian_91"));
    ui->urlLineEdit->setText(tr("http://192.168.20.18/devices/settings"));
    ui->urlLineEdit->setClearButtonEnabled(true);

    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
                this, SLOT(authRequiredReply(QNetworkReply*,QAuthenticator*)));
    /* send button */
    connect(ui->sendBtn, SIGNAL(clicked(bool)), this, SLOT(sendBtnClicked()));
}

Widget::~Widget()
{
    delete ui;
}

int Widget::httpRequest(QUrl url, QByteArray &response)
{
    QNetworkRequest request;
    QNetworkReply *reply;
    QVariant statusCode;
    QEventLoop eventLoop;
    QTimer timer;
    int retCode = 0;

    qDebug() << "url: " << url;
    request.setUrl(url);
    request.setRawHeader("User-Agent", "qHttp-client 1.0");
    reply = manager->get(request);

    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
    timer.start(30 * 1000);
    eventLoop.exec(); // block until finish

    if (timer.isActive()) {
        timer.stop();
    } else {
        disconnect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
        reply->abort();
        reply->deleteLater();
        return -1;
    }

    response = reply->readAll();
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.isValid()) {
        qDebug() << "recv finished: " << statusCode.toInt();
    }


    QList<QByteArray> list = reply->rawHeaderList();
    for (int i = 0; i < list.size(); i++) {
        qDebug() << "header: " << list.at(i).data() << " " << reply->rawHeader(list.at(i));
    }

    retCode = statusCode.toInt();
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "recv data: " << retCode;
    } else {
        qDebug() << "reply error: " << reply->error();
    }

    if (reply != NULL) {
        disconnect(reply, SIGNAL(readyRead()), &eventLoop, SLOT(quit()));
        reply->abort();
        reply->deleteLater();
    }

    return retCode;
}

void Widget::sendBtnClicked(void)
{
    QString urlString;
    QByteArray response;
    QUrl url;
    //QUrlQuery urlQuery;
    int retCode = 0;

    urlString = ui->urlLineEdit->text();
    if (urlString.isEmpty()) {
        QMessageBox::information(this, tr("Error"), tr("Please input valid URL!"));
        return ;
    }

    //urlQuery.addQueryItem("sourceApp", 8);
    //urlQuery.addQueryItem("t", "tStr");

    qDebug() << "Send url: " << urlString;
    url.setUrl(urlString);
    //url.setQuery(urlQuery);

    retCode = httpRequest(url, response);
    qDebug() << "Send finished response: " << response;

    //QString res = response;
    //ui->recvBodyTextBrowser->setText(res);
}

void Widget::authRequiredReply(QNetworkReply *reply, QAuthenticator *auth)
{
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();

    qDebug() << "username: " << username << "password: " << password;
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::information(this, tr("Error"), tr("Please input valid username and passowrd!"));
        return ;
    }

    auth->setUser(username);
    auth->setPassword(password);
}
