#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

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
    int httpRequest(QUrl url, QByteArray &response);

private slots:
    void authRequiredReply(QNetworkReply *reply, QAuthenticator *auth);
    void sendBtnClicked(void);
};

#endif // WIDGET_H
