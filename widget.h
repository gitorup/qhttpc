#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

typedef struct {
    QByteArray key;
    QByteArray value;
} httpHeader_t;

typedef enum {
    HTTP_METHOD_GET = 0,
    HTTP_METHOD_POST = 1,
    HTTP_METHOD_PUT = 2,
    HTTP_METHOD_HEAD = 3,
    HTTP_METHOD_DELETE = 4,
} httpMethod_e;

typedef struct {
    httpMethod_e method;
    QUrl url;
    QList<httpHeader_t> header;
    QByteArray postData;
} httpRequest_t;

typedef struct {
    int httpCode;
    QList<httpHeader_t> header;
    QByteArray response;
} httpReply_t;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private:
    Ui::Widget *ui;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    int httpRequest(httpRequest_t *request, httpReply_t *reply);

private slots:
    void authRequiredReply(QNetworkReply *reply, QAuthenticator *auth);
    void sendBtnClicked(void);
    void sendQueryAppendBtnClicked(void);
    void sendQueryClearBtnClicked(void);
};

#endif // WIDGET_H
