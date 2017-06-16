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
#include <QKeyEvent>
//https://image.eveonline.com/Type/3530_64.png
//https://www.fuzzwork.co.uk/api/typeid2.php?typename=Tritanium|Pyerite
//http://api.eve-central.com/api/marketstat?typeid=3530&usesystem=30000142

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    
    priceCheck = new EvePriceCheck(this);
    
    connect(ui->runButton, SIGNAL(clicked(bool)), SLOT(run()));
//    connect(this, SIGNAL(enterPressed()), SLOT(run()));
    connect(this, SIGNAL(enterPressed()), ui->runButton, SLOT(click()));
    connect(ui->runButton, SIGNAL(clicked(bool)), ui->progressBar, SLOT(show()));
    connect(priceCheck, SIGNAL(finished(bool)), SLOT(showResult(bool)));
    connect(priceCheck, SIGNAL(progress(int)), ui->progressBar, SLOT(setValue(int)));
//    connect(priceCheck, SIGNAL(finished()), ui->progressBar, SLOT(hide()));
//    connect(priceCheck, SIGNAL(finished()), ui->progressBar, SLOT(setMinimum(int)));
//    connect(networkAccess, SIGNAL(finished(QNetworkReply*)), SLOT(RequestFinished(QNetworkReply*)));
//    connect(ui->inText, SIGNAL(textChanged()), SLOT(inputChanged()));
//    connect(ui->inText, SIGNAL(textChanged()), SLOT(run()));

    ui->inputEdit->installEventFilter(this);

    QStringList list;
    list << "FIrst item";
    list << "Second item";
    list << "third item";
    list << "fa item";

    ui->comboBox->addItems(list);
    ui->comboBox->addItem("TTESX");
}

Widget::~Widget()
{
    delete ui;
}

void Widget::run()
{

    if (priceCheck->findResult(ui->inputEdit->text(), ui->overPrice->value()))
    {
        return;
    }
}

void Widget::showResult(bool success)
{
    QString newPrice(priceCheck->getNewPrice());
    QString basePrice(priceCheck->getBasePrice());

    if (success)
    {
        ui->basePriceLabel->setText(basePrice + " ISK");
        ui->newPriceLabel->setText(newPrice + " ISK");
        ui->imageLabel->setPixmap(priceCheck->getPicture());
    } else {
        ui->basePriceLabel->setText("Error");
        ui->newPriceLabel->setText("Error");
        ui->imageLabel->clear();
    }

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(newPrice);

    ui->progressBar->hide();
}

bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    QKeyEvent *key;

    switch (event->type())
    {
        case QEvent::FocusIn:
        case QEvent::FocusOut:
            if (watched == ui->inputEdit)
            {
//                ui->inputEdit->setFocus();
                ui->inputEdit->selectAll();

                return true;
            }
            break;

        case QEvent::KeyPress:
            key = static_cast<QKeyEvent *>(event);
            if (key->key() == Qt::Key_Enter || key->key()==Qt::Key_Return)
            {
                emit enterPressed();
                return true;
            }
            break;

        default:
            break;
    }

//    if (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut)
//    {
//        if (watched == ui->inputEdit)
//        {
//            ui->inputEdit->setFocus();
//            ui->inputEdit->selectAll();

//            return true;
//        }
//    }
//    return QDialog::eventFilter(watched, event);
    return false;
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

    emit progress(0);

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
                if (findTypeId(arr)) {
                    emit finished(false);
                    return;
                }
                state = STATE_GET_ID;
                break;

            case STATE_GET_ID:
                if (findPrices(arr)) {
                    emit finished(false);
                    return;
                }
                state = STATE_GET_PRICES;
                break;

            case STATE_GET_PRICES:
                findPicture(arr);
                state = STATE_GET_PIC;
                break;

            default: break;
        }
    }

    getNext();

    reply->deleteLater();

    if (state == STATE_GET_PIC)
    {
        emit finished(true);
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

    emit progress(1);

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

            emit progress(2);

            return 0;
        }
    }

    qDebug() << "Can't find price in XML";

    return -1;
}

int EvePriceCheck::findPicture(QByteArray &byteArr)
{
    picture.loadFromData(byteArr);

    emit progress(3);

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






















