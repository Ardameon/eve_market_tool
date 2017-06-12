#include "eve_api.h"
#include "ui_widget.h"
#include <QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QUrl>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    
    networkAccess = new QNetworkAccessManager(this);
    
    connect(ui->runButton, SIGNAL(clicked(bool)), this, SLOT(run()));
    connect(networkAccess, SIGNAL(finished(QNetworkReply*)), SLOT(RequestFinished(QNetworkReply*)));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::run()
{
    qDebug() << "RUN:" << " " << ui->inText->toPlainText();
    networkAccess->get(QNetworkRequest(QUrl(ui->inText->toPlainText())));
}

void Widget::RequestFinished(QNetworkReply *reply)
{
    qDebug() << "Request finished";
    if (reply->error() == QNetworkReply::NoError)
    {
        qDebug() << "Success";
        
        QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        
        if(redirect.isValid() && reply->url() != redirect)
        {
            qDebug() << "Redirect";
            if(redirect.isRelative())
                redirect = reply->url().resolved(redirect);
            QNetworkRequest req(redirect);
            networkAccess->get(req);
            return;
        }
        
        QString string(reply->readAll());
        qDebug() << string;
        ui->outText->setPlainText(string);
    } else {
        qDebug() << "Failed";
        ui->outText->setPlainText(reply->errorString());
    }
}
