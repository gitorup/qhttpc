#include "widget.h"
#include "ui_widget.h"
#include "widget.h"

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
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

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
    ui->usernameLineEdit->setText(tr("admin"));
    ui->passwordLineEdit->setText(tr("admin"));
    ui->urlLineEdit->setText(tr("http://192.168.0.1"));
    ui->urlLineEdit->setClearButtonEnabled(true);
    ui->usernameLineEdit->setClearButtonEnabled(true);
    ui->passwordLineEdit->setClearButtonEnabled(true);

    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
                this, SLOT(authRequiredReply(QNetworkReply*,QAuthenticator*)));
    /* send button */
    connect(ui->sendBtn, SIGNAL(clicked(bool)), this, SLOT(sendBtnClicked()));
    connect(ui->sendQueryAppendBtn, SIGNAL(clicked(bool)), this, SLOT(sendQueryAppendBtnClicked()));
    connect(ui->sendQueryClearBtn, SIGNAL(clicked(bool)), this, SLOT(sendQueryClearBtnClicked()));
}

Widget::~Widget()
{
    delete ui;
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

void Widget::sendQueryAppendBtnClicked(void)
{
    QString queryKey;
    QString queryValue;
    QString url;

    queryKey = ui->sendQueryKeyLineEdit->text();
    queryValue = ui->sendQueryValueLineEdit->text();
    url = ui->urlLineEdit->text();

    if (queryKey.isEmpty() || queryValue.isEmpty()) {
        QMessageBox::information(this, tr("Error"), tr("Please input valid key and value!"));
        return ;
    }

    if (!url.contains("?", Qt::CaseInsensitive)) {
        url.append("?");
    } else {
        url.append("&");
    }

    url.append(queryKey + "=" + queryValue);
    ui->urlLineEdit->setText(url);
}

void Widget::sendQueryClearBtnClicked(void)
{
    ui->sendQueryKeyLineEdit->clear();
    ui->sendQueryValueLineEdit->clear();
}

int Widget::httpRequest(httpRequest_t *httpRequest, httpReply_t *httpReply)
{
    QNetworkRequest request;
    QNetworkReply *reply;
    QVariant statusCode;
    QEventLoop eventLoop;
    QTimer timer;

    qDebug() << "url: " << httpRequest->url;
    request.setUrl(httpRequest->url);
    for (int i = 0; i < httpRequest->header.size(); i++) {
        request.setRawHeader(httpRequest->header.at(i).key, httpRequest->header.at(i).value);
        qDebug() << "key " << httpRequest->header.at(i).key << " value " << httpRequest->header.at(i).value;
    }

    switch (httpRequest->method) {
    case HTTP_METHOD_GET:
        reply = manager->get(request);
        break;

    case HTTP_METHOD_POST:
        reply = manager->post(request, httpRequest->postData);
        break;

    case HTTP_METHOD_PUT:
        reply = manager->put(request, httpRequest->postData);
        break;

    case HTTP_METHOD_HEAD:
        reply = manager->head(request);
        break;

    case HTTP_METHOD_DELETE:
        reply = manager->deleteResource(request);
        break;

    default:
        qDebug() << "method invalid: " << httpRequest->method;
        return -1;
    }

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

    httpReply->response = reply->readAll();
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.isValid()) {
        httpReply->httpCode = statusCode.toInt();
        qDebug() << "recv finished: " << statusCode.toInt();
    }

    QList<QByteArray> list = reply->rawHeaderList();
    for (int i = 0; i < list.size(); i++) {
        qDebug() << "header: " << list.at(i).data() << " " << reply->rawHeader(list.at(i));
        httpHeader_t header;
        header.key = list.at(i);
        header.value = reply->rawHeader(list.at(i));
        httpReply->header.append(header);
    }

    httpReply->httpCode = statusCode.toInt();
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "recv data: " << httpReply->httpCode;
    } else {
        qDebug() << "reply error: " << reply->error();
    }

    if (reply != NULL) {
        disconnect(reply, SIGNAL(readyRead()), &eventLoop, SLOT(quit()));
        reply->abort();
        reply->deleteLater();
    }

    return httpReply->httpCode;
}

void Widget::sendBtnClicked(void)
{
    httpRequest_t request;
    httpHeader_t userAgent;
    httpHeader_t auth;
    QString urlString;
    httpReply_t reply;
    QString authString;
    QByteArray basic;
    QUrl url;
    int retCode = 0;

    urlString = ui->urlLineEdit->text();
    if (urlString.isEmpty()) {
        QMessageBox::information(this, tr("Error"), tr("Please input valid URL!"));
        return ;
    }

    /* http url + query */
    qDebug() << "Send url: " << urlString;
    url.setUrl(urlString);
    request.url = url;

    authString = ui->usernameLineEdit->text() + ":" + ui->passwordLineEdit->text();
    basic = authString.toLocal8Bit();

    /* http header: User-Agent: qhttpc 1.0 */
    userAgent.key.append("User-Agent");
    userAgent.value.append("qhttpc 1.0");
    request.header.append(userAgent);

    /* http header: Basic auth */
    if (ui->sendAuthComboBox->currentIndex() == 1) {
        // Authorization: Basic base64(username:password)
        auth.key.append("Authorization");
        auth.value.append("Basic " + basic.toBase64());
        request.header.append(auth);
    }

    /* append post data */
    if (!ui->sendBodyTextEdit->toPlainText().isEmpty()) {
        request.postData.append(ui->sendBodyTextEdit->toPlainText());
        qDebug() << "post data: " << request.postData;
    }

    /* http method */
    switch (ui->methodComboBox->currentIndex()) {
    case 0:
        request.method = HTTP_METHOD_GET;
        break;

    case 1:
        request.method = HTTP_METHOD_POST;
        break;

    case 2:
        request.method = HTTP_METHOD_PUT;
        break;

    case 3:
        request.method = HTTP_METHOD_HEAD;
        break;

    case 4:
        request.method = HTTP_METHOD_DELETE;
        break;
    }

    retCode = httpRequest(&request, &reply);
    qDebug() << "Send finished response: " << retCode;

    QString res = reply.response;
    ui->recvBodyTextBrowser->setText(res);

    QVBoxLayout *mainLayout =  new QVBoxLayout();
    for (int i = 0; i < reply.header.size(); i++) {
        QHBoxLayout *vLayout = new QHBoxLayout();
        QString keyString = reply.header.at(i).key;
        QString valueString = reply.header.at(i).value;
        QLineEdit *key = new QLineEdit();
        QLineEdit *value = new QLineEdit();

        qDebug() << "key " << keyString << " value " << valueString;
        key->setText(keyString);
        value->setText(valueString);
        vLayout->addWidget(key);
        vLayout->addWidget(value);
        mainLayout->addLayout(vLayout);
    }
    ui->recvHeaderTab->setLayout(mainLayout);
}
