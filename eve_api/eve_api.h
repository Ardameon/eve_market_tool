#ifndef EVE_API_H
#define EVE_API_H

#include <QWidget>
#include <QtNetwork/QNetworkAccessManager>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT
    
public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    
public slots:
    void run();
    void RequestFinished(QNetworkReply *reply);
        
    
private:
    Ui::Widget *ui;
    QNetworkAccessManager *networkAccess;
    
};

#endif // EVE_API_H
