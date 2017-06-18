#ifndef EVE_API_H
#define EVE_API_H

#include <QWidget>
#include "evepricecheck.h"

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
    void showResult(bool success);
    
private:
    Ui::Widget *ui;
    EvePriceCheck *priceCheck;

    bool eventFilter(QObject *watched, QEvent *event);

signals:
    void enterPressed();
};


#endif // EVE_API_H
