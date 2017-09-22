#ifndef EVEPRICECHECK_H
#define EVEPRICECHECK_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QPixmap>



//https://image.eveonline.com/Type/3530_64.png
//https://www.fuzzwork.co.uk/api/typeid2.php?typename=Tritanium|Pyerite
//http://api.eve-central.com/api/marketstat?typeid=3530&usesystem=30000142

class EvePriceCheck : public QObject
{
    Q_OBJECT
    const QString getIdURL = QString("https://www.fuzzwork.co.uk/api/typeid2.php?typename=");
    const QString getImageURL = QString("https://image.eveonline.com/Type/");
    const QString getPriceURLEveC = QString("http://api.eve-central.com/api/marketstat?usesystem=30000142&typeid=");
    const QString getPriceURLESI = QString("https://esi.tech.ccp.is/latest/markets/10000002/orders/?datasource=tranquility&order_type=all&page=1&type_id=");

public:
    EvePriceCheck(QObject *parent = 0);
    ~EvePriceCheck();
    int findResult(const QString &itemName, qint8 perncent);
    QString getBasePriceStr();
    QString getNewPriceStr();
    QString getBuyPriceStr();
    QString getDiffPercentStr();
    QPixmap getPicture();

    double getBasePrice();
    double getNewPrice();
    double getBuyPrice();
    double getDiffPercent();

    typedef enum {
        STATE_START,
        STATE_GET_ID,
        STATE_GET_PRICES,
        STATE_GET_PIC
    } ePriceCheckState;

    typedef enum {
        SOURCE_EVE_CENTRAL,
        SOURCE_ESI
    } ePriceCheckSrc;

    void setSource(ePriceCheckSrc source);
    void setESIPrecision(quint8 precision);

public slots:
    void redirectionCheck(QNetworkReply *reply);

private:
    QNetworkAccessManager *networkAccess;
    QString typeName;
    qint8 pricePercent;
    QString typeID;
    double buyPrice;
    double sellBuyDiff;
    double basePrice;
    double newPrice;
    QPixmap picture;
    ePriceCheckState state;
    ePriceCheckSrc priceSource;
    quint8 pricePrecision;

    int findTypeId(QByteArray &byteArr);
    int findPricesEveCentral(QByteArray &byteArr);
    int findPricesESI(QByteArray &byteArr);
    int findPicture(QByteArray &byteArr);
    void getNext();

signals:
    void finished(bool success);
    void progress(int i);


};

#endif // EVEPRICECHECK_H
