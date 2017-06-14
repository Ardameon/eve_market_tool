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

    priceCheck = new EvePriceCheck(this);
    
    connect(ui->runButton, SIGNAL(clicked(bool)), SLOT(run()));
    connect(priceCheck, SIGNAL(finished()), SLOT(showResult()));
//    connect(networkAccess, SIGNAL(finished(QNetworkReply*)), SLOT(RequestFinished(QNetworkReply*)));
//    connect(ui->inText, SIGNAL(textChanged()), SLOT(inputChanged()));
//    connect(ui->inText, SIGNAL(textChanged()), SLOT(run()));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::run()
{
    qDebug() << "RUN:" << " " << ui->inText->toPlainText();
//    networkAccess->get(QNetworkRequest(QUrl(ui->inText->toPlainText())));

    if (priceCheck->findResult(ui->inputEdit->text(), ui->overPrice->value()))
    {
        return;
    }

    QString newPrice(priceCheck->getNewPrice());
    QString basePrice(priceCheck->getBasePrice());

    ui->basePriceLabel->setText(basePrice + " ISK");
    ui->newPriceLabel->setText(newPrice + " ISK");
    ui->imageLabel->setPixmap(priceCheck->getPicture());

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(newPrice);
}

void Widget::RequestFinished(QNetworkReply *reply)
{
//    qDebug() << "Request finished";
//    if (reply->error() == QNetworkReply::NoError)
//    {
//        qDebug() << "Success";
        
//        QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        
//        if(redirect.isValid() && reply->url() != redirect)
//        {
//            qDebug() << "Redirect";
//            if(redirect.isRelative())
//                redirect = reply->url().resolved(redirect);
//            QNetworkRequest req(redirect);
//            networkAccess->get(req);
//            return;
//        }
        
//        QByteArray byteArray(reply->readAll());
//        QString string(byteArray);
//        qDebug() << string;
////        ui->outText->setPlainText(string);

//        QGraphicsScene scene;
//        QPixmap pix;
//        QGraphicsPixmapItem pixItem;
//        pix.loadFromData(byteArray);
//        pixItem.setPixmap(pix);
//        scene.addItem(&pixItem);
//        ui->graphicsView->setScene(&scene);
//        ui->graphicsView->show();
////        ui->label->setPixmap(pix);

//        QXmlStreamReader xmlReader(byteArray);

//        while (!xmlReader.atEnd())
//        {
//            xmlReader.readNext();
//            if (xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name() == "sell")
//            {
//                xmlReader.readNext();
//                while (!xmlReader.tokenType() != QXmlStreamReader::StartElement && xmlReader.name() != "percentile")
//                {
//                    xmlReader.readNext();
//                }

//                xmlReader.readNext();


//                double new_price = xmlReader.text().toDouble() * 1.1;
//                QClipboard *clipboard = QGuiApplication::clipboard();

//                ui->outText->append("Jita sell price: " + xmlReader.text().toString() + " ISK" + " Jita + 10%: " + QString::number(new_price, 'f', 2) + " ISK");
//                clipboard->setText(QString::number(new_price, 'f', 2));
//            }
////            ui->outText->append(xmlR  eader.tokenString() + QString(" ") + xmlReader.name().toString() + QString(" ") + xmlReader.text().toString());
//        }

//        if (xmlReader.hasError())
//        {
//            ui->outText->append(xmlReader.errorString());
//        }

//    } else {
//        qDebug() << "Failed";
//        ui->outText->setPlainText(reply->errorString());
//    }
}

void Widget::inputChanged()
{
    qDebug() << "Input changed: " << ui->inText->toPlainText();
}

void Widget::showResult()
{
    QString newPrice(priceCheck->getNewPrice());
    QString basePrice(priceCheck->getBasePrice());

    ui->basePriceLabel->setText(basePrice + " ISK");
    ui->newPriceLabel->setText(newPrice + " ISK");
    ui->imageLabel->setPixmap(priceCheck->getPicture());

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(newPrice);
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

EvePriceCheck::EvePriceCheck(QObject *parent) :
    QObject(parent), state(STATE_START)
{
    networkAccess = new QNetworkAccessManager(this);

    connect(networkAccess, SIGNAL(finished(QNetworkReply*)), SLOT(redirectionCheck(QNetworkReply*)));

}

EvePriceCheck::~EvePriceCheck()
{
}

int EvePriceCheck::findResult(const QString &itemName, qint8 percent)
{
    typeName = itemName;
    pricePercent = percent;

    state = STATE_START;

    getNext();

    return 0;
}

QString EvePriceCheck::getBasePrice()
{
    return QString::number(basePrice, 'f', 2);
}

QString EvePriceCheck::getNewPrice()
{
    return QString::number(newPrice, 'f', 2);
}

QPixmap EvePriceCheck::getPicture()
{
    return picture;
}

void EvePriceCheck::redirectionCheck(QNetworkReply *reply)
{
    QUrl redirect;

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Redirection check error: " << reply->errorString();
        return;
    }

    redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if(redirect.isValid() && reply->url() != redirect)
    {
        if(redirect.isRelative())
            redirect = reply->url().resolved(redirect);

        networkAccess->get(QNetworkRequest(redirect));
    } else {
        QByteArray arr(reply->readAll());

        switch (state)
        {
            case STATE_START:
                findTypeId(arr);
                state = STATE_GET_ID;
                break;

            case STATE_GET_ID:
                findPrices(arr);
                state = STATE_GET_PRICES;
                break;

            case STATE_GET_PRICES:
                findPicture(arr);
                state = STATE_GET_PIC;
                break;
        }
    }

    getNext();

    reply->deleteLater();

    if (state == STATE_GET_PIC)
    {
        emit finished();
    }
}

int EvePriceCheck::findTypeId(QByteArray &byteArr)
{
    QJsonDocument jdoc = QJsonDocument::fromJson(byteArr);
    QJsonObject jobj;
    QJsonArray jarr;
    QJsonValue jvalue;

    if (jdoc.isNull() || !jdoc.isArray())
    {
        qDebug() << "Jdoc is NULL";
        return -1;
    }

    jarr = jdoc.array();

    if (!jarr.size())
    {
        qDebug() << "Jarr is empty";
        return -2;
    }

    jobj = jarr[0].toObject();

    jvalue = jobj.value("typeID");

    if (jvalue.isNull())
    {
        qDebug() << "Can't find typeID in Json";
        return -3;
    }

    typeID = jvalue.toString();

    qDebug() << "TypeID: " << typeID;

    return 0;
}

int EvePriceCheck::findPrices(QByteArray &byteArr)
{
    QXmlStreamReader xmlReader(byteArr);

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

            basePrice = xmlReader.text().toDouble();
            newPrice = basePrice * (1.0 + (0.01 * pricePercent));

            return 0;
        }
    }

    qDebug() << "Can't find price in XML";

    return -1;
}

int EvePriceCheck::findPicture(QByteArray &byteArr)
{
    picture.loadFromData(byteArr);

    return 0;
}

void EvePriceCheck::getNext()
{
    switch (state)
    {
        case STATE_START:
            networkAccess->get(QNetworkRequest(QUrl(getIdURL + typeName.replace(' ', '+'))));
            break;

        case STATE_GET_ID:
            networkAccess->get(QNetworkRequest(QUrl(getPriceURL + typeID)));
            break;

        case STATE_GET_PRICES:
            networkAccess->get(QNetworkRequest(QUrl(getImageURL + typeID + QString("_64.png"))));
            break;

        default: break;
    }
}






















