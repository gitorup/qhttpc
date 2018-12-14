#include "widget.h"
#include "ui_widget.h"

#include "qnetworkrequest.h"
#include "qdatetime.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    QDateTime DateTime = QDateTime::currentDateTime();
    setWindowTitle("QHttp-Tool - " + DateTime.toString("yyyy.MM.dd"));
}

Widget::~Widget()
{
    delete ui;
}
