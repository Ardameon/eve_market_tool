#ifndef EVE_API_H
#define EVE_API_H

#include <QWidget>
#include <QtNetwork/QNetworkAccessManager>
#include <QtSql/QSql>

namespace Ui {
class Widget;
}

class EvePriceCheck;

class Widget : public QWidget
{
    Q_OBJECT
    
public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    
public slots:
    void run();
    void RequestFinished(QNetworkReply *reply);
    void inputChanged();
    void showResult();
    
private:
    Ui::Widget *ui;
    QNetworkAccessManager *networkAccess;
    EvePriceCheck *priceCheck;
    bool eventFilter(QObject *watched, QEvent *event);
    
};


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
    QString getBasePrice();
    QString getNewPrice();
    QPixmap getPicture();

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
    double basePrice;
    double newPrice;
    QPixmap picture;
    ePriceCheckState state;

    int findTypeId(QByteArray &byteArr);
    int findPrices(QByteArray &byteArr);
    int findPicture(QByteArray &byteArr);
    void getNext();

signals:
    void finished();


};

#endif // EVE_API_H
