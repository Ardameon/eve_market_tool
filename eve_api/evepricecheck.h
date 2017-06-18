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
    const QString getPriceURL = QString("http://api.eve-central.com/api/marketstat?usesystem=30000142&typeid=");

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

    int findTypeId(QByteArray &byteArr);
    int findPrices(QByteArray &byteArr);
    int findPicture(QByteArray &byteArr);
    void getNext();

signals:
    void finished(bool success);
    void progress(int i);


};

#endif // EVEPRICECHECK_H
