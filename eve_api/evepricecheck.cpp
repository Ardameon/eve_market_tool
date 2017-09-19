#include <QDebug>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QtXml>
#include <QClipboard>
#include <QKeyEvent>

#include "evepricecheck.h"

EvePriceCheck::EvePriceCheck(QObject *parent) :
    QObject(parent), state(STATE_START)
{
    networkAccess = new QNetworkAccessManager(this);

    connect(networkAccess, SIGNAL(finished(QNetworkReply*)), SLOT(redirectionCheck(QNetworkReply*)));

}

/*====================================================================================================================*/

EvePriceCheck::~EvePriceCheck()
{
}

/*====================================================================================================================*/

int EvePriceCheck::findResult(const QString &itemName, qint8 percent)
{
    typeName = itemName;
    pricePercent = percent;

    state = STATE_START;

    emit progress(0);

    getNext();

    return 0;
}

/*====================================================================================================================*/

void EvePriceCheck::setSource(EvePriceCheck::ePriceCheckSrc source)
{
    priceSource = source;
}

/*====================================================================================================================*/

QString EvePriceCheck::getBasePriceStr()
{
    return QString::number(basePrice, 'f', 2);
}

/*====================================================================================================================*/

QString EvePriceCheck::getNewPriceStr()
{
    return QString::number(newPrice, 'f', 0);
}

/*====================================================================================================================*/

QString EvePriceCheck::getBuyPriceStr()
{
    return QString::number(buyPrice, 'f', 2);
}

/*====================================================================================================================*/

QString EvePriceCheck::getDiffPercentStr()
{
    return QString::number(sellBuyDiff, 'f', 2) + " %";
}

/*====================================================================================================================*/

QPixmap EvePriceCheck::getPicture()
{
    return picture;
}

/*====================================================================================================================*/

double EvePriceCheck::getBasePrice()
{
    return basePrice;
}

/*====================================================================================================================*/

double EvePriceCheck::getNewPrice()
{
    return newPrice;
}

/*====================================================================================================================*/

double EvePriceCheck::getBuyPrice()
{
    return buyPrice;
}

/*====================================================================================================================*/

double EvePriceCheck::getDiffPercent()
{
    return sellBuyDiff;
}

/*====================================================================================================================*/

void EvePriceCheck::redirectionCheck(QNetworkReply *reply)
{
    QUrl redirect;
    int res;

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
                if (priceSource == EvePriceCheck::SOURCE_EVE_CENTRAL)
                {
                    res = findPricesEveCentral(arr);
                } else {
                    res = findPricesESI(arr);
                }

                if (res) {
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

/*====================================================================================================================*/

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

/*====================================================================================================================*/

int EvePriceCheck::findPricesEveCentral(QByteArray &byteArr)
{
    QXmlStreamReader xmlReader(byteArr);

    while (!xmlReader.atEnd())
    {
        xmlReader.readNext();
        if (xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name() == "buy")
        {
            xmlReader.readNext();

            while ((xmlReader.tokenType() != QXmlStreamReader::StartElement)
                   || (xmlReader.name().toString() != "percentile"))
            {
                xmlReader.readNext();
            }

            xmlReader.readNext();

            buyPrice = xmlReader.text().toDouble();

            emit progress(2);

        } else if (xmlReader.tokenType() == QXmlStreamReader::StartElement && xmlReader.name() == "sell")
        {
            xmlReader.readNext();

            while ((xmlReader.tokenType() != QXmlStreamReader::StartElement)
                   || (xmlReader.name().toString() != "percentile"))
            {
                xmlReader.readNext();
            }

            xmlReader.readNext();

            basePrice = xmlReader.text().toDouble();
            newPrice = basePrice * (1.0 + (0.01 * pricePercent));

            if (basePrice > 0.01)
            {
                sellBuyDiff = (basePrice - buyPrice) / basePrice * 100.0;
            } else {
                sellBuyDiff = -100.0;
            }

            emit progress(3);

            return 0;
        }
    }

    qDebug() << "Can't find price in XML";

    return -1;
}

/*====================================================================================================================*/

int EvePriceCheck::findPricesESI(QByteArray &byteArr)
{
    QJsonDocument jdoc = QJsonDocument::fromJson(byteArr);
    QJsonObject jobj;
    QJsonArray jarr;
    QJsonValue is_buy_order;
    QJsonValue price;
    QJsonValue location_id;
    QVector<double> sell_prices;
    QVector<double> buy_prices;
    const int jita_4_4_loc_id = 60003760;
    const int prefer_precision = 5;
    int i, precision;

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

    for (i = 0; i < jarr.size(); i++)
    {
        jobj = jarr[i].toObject();

        location_id = jobj.value("location_id");
        price = jobj.value("price");
        is_buy_order = jobj.value("is_buy_order");

        if (location_id.toInt() == jita_4_4_loc_id)
        {
            if (is_buy_order.toBool())
            {
                buy_prices.append(price.toDouble());
            } else {
                sell_prices.append(price.toDouble());
            }
        }
    }

    qSort(sell_prices);
    qSort(buy_prices);

    basePrice = buyPrice = 0.0;

    /* Find SELL price */
    precision = sell_prices.size() < prefer_precision ? sell_prices.size() : prefer_precision;
    for (i = 0; i < precision; i++)
    {
        basePrice += sell_prices.at(i);
    }

    if (precision) basePrice /= precision;
    newPrice = basePrice * (1.0 + (0.01 * pricePercent));

    /* Find BUY price */
    precision = buy_prices.size() < prefer_precision ? buy_prices.size() : prefer_precision;
    for (i = buy_prices.size() - 1; i > buy_prices.size() - precision - 1; i--)
    {
        buyPrice += buy_prices.at(i);
    }

    if (precision) buyPrice /= precision;

    /* Find difference */
    if (basePrice > 0.01)
    {
        sellBuyDiff = (basePrice - buyPrice) / basePrice * 100.0;
    } else {
        sellBuyDiff = -100.0;
    }

    emit progress(3);

    return 0;
}

/*====================================================================================================================*/

int EvePriceCheck::findPicture(QByteArray &byteArr)
{
    picture.loadFromData(byteArr);

    emit progress(4);

    return 0;
}

/*====================================================================================================================*/

void EvePriceCheck::getNext()
{
    switch (state)
    {
        case STATE_START:
            networkAccess->get(QNetworkRequest(QUrl(getIdURL + typeName.replace(' ', '+'))));
            break;

        case STATE_GET_ID:
            if (priceSource == EvePriceCheck::SOURCE_EVE_CENTRAL)
            {
                networkAccess->get(QNetworkRequest(QUrl(getPriceURLEveC + typeID)));
            } else {
                networkAccess->get(QNetworkRequest(QUrl(getPriceURLESI + typeID)));
            }
            break;

        case STATE_GET_PRICES:
            networkAccess->get(QNetworkRequest(QUrl(getImageURL + typeID + QString("_64.png"))));
            break;

        default: break;
    }
}

