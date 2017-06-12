#ifndef EVE_API_H
#define EVE_API_H

#include <QWidget>
#include <QtNetwork/QNetworkAccessManager>
#include <QtSql/QSql>

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
    void inputChanged();
    
private:
    Ui::Widget *ui;
    QNetworkAccessManager *networkAccess;
    bool eventFilter(QObject *watched, QEvent *event);
    
};

#endif // EVE_API_H
