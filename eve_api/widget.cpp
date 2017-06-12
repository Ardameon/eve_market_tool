#include "eve_api.h"
#include "ui_widget.h"
#include <QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QUrl>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QtXml>
#include <QClipboard>
//https://image.eveonline.com/Type/3530_64.png
//https://www.fuzzwork.co.uk/api/typeid2.php?typename=Tritanium|Pyerite
//http://api.eve-central.com/api/marketstat?typeid=3530&usesystem=30000142

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    
    networkAccess = new QNetworkAccessManager(this);
    
    connect(ui->runButton, SIGNAL(clicked(bool)), this, SLOT(run()));
    connect(networkAccess, SIGNAL(finished(QNetworkReply*)), SLOT(RequestFinished(QNetworkReply*)));
    connect(ui->inText, SIGNAL(textChanged()), SLOT(inputChanged()));
    connect(ui->inText, SIGNAL(textChanged()), SLOT(run()));
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
        
        QByteArray byteArray(reply->readAll());
        QString string(byteArray);
        qDebug() << string;
//        ui->outText->setPlainText(string);

        QGraphicsScene scene;
        QPixmap pix;
        QGraphicsPixmapItem pixItem;
        pix.loadFromData(byteArray);
        pixItem.setPixmap(pix);
        scene.addItem(&pixItem);
        ui->graphicsView->setScene(&scene);
        ui->graphicsView->show();
        ui->label->setPixmap(pix);

        QXmlStreamReader xmlReader(byteArray);

        while (!xmlReader.atEnd())
        {
            xmlReader.readNext();
            if (xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name() == "sell")
            {
                xmlReader.readNext();
                while (!xmlReader.tokenType() != QXmlStreamReader::StartElement && xmlReader.name() != "percentile")
                {
                    xmlReader.readNext();
                }

                xmlReader.readNext();


                double new_price = xmlReader.text().toDouble() * 1.1;
                QClipboard *clipboard = QGuiApplication::clipboard();

                ui->outText->append("Jita sell price: " + xmlReader.text().toString() + " ISK" + " Jita + 10%: " + QString::number(new_price, 'f', 2) + " ISK");
                clipboard->setText(QString::number(new_price, 'f', 2));
            }
//            ui->outText->append(xmlR  eader.tokenString() + QString(" ") + xmlReader.name().toString() + QString(" ") + xmlReader.text().toString());
        }

        if (xmlReader.hasError())
        {
            ui->outText->append(xmlReader.errorString());
        }

    } else {
        qDebug() << "Failed";
        ui->outText->setPlainText(reply->errorString());
    }
}

void Widget::inputChanged()
{
    qDebug() << "Input changed: " << ui->inText->toPlainText();
}

bool Widget::eventFilter(QObject *watched, QEvent *event)
{
//    static int i = 0;
//    if(event->type() ==QEvent::FocusIn && watched == ui->inText)
//    {
//        QClipboard *clipboard = QGuiApplication::clipboard();
//        qDebug() << "Focus " << QString::number(i) << " cliboard " << ((clipboard->ownsClipboard()) ? "yes" : "no")
//                 << " selection " << (clipboard->ownsSelection() ? "yes" : "no")
//                 << " find buffer " << (clipboard->ownsFindBuffer() ? "yes" : "no");
//        if (1/*clipboard->ownsClipboard()*/)
//        {
//            if (clipboard->text().length())
//                ui->inText->setPlainText(clipboard->text());
//            clipboard->clear();
//        }
//        i++;
//        return false;
//    }
//    else
//    {
//    	return false;
//    }
    return true;
}

