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



public:
    int findResult(const QString &itemName, qint8 perncent);
    QString getBasePrice();
    QString getNewPrice();
    QPixmap getPicture();



private:
    QNetworkAccessManager *networkAccess;
    QString typeName;
    QString typeID;
    double basePrice;
    double newPrice;
    QPixmap picture;

    int findTypeId(QByteArray &byteArr);
    int findPrices(QByteArray &byteArr, quint8 percent);
    int findPicture(QByteArray &byteArr);


};

#endif // EVE_API_H
