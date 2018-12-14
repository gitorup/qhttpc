#include "widget.h"
#include "ui_widget.h"

#include <QDateTime>
#include <QUrl>
#include <QVariant>
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
    qDebug() << "recv finished: " << statusCode;

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
    if (urlString.isNull() || urlString.isEmpty()) {
        QMessageBox::information(this, tr("Error"), tr("Please input valid URL!"));
        return ;
    }

    QUrl url(urlString);
    manager->get(QNetworkRequest(url));
    qDebug() << "Send finished url: " << urlString;
}
