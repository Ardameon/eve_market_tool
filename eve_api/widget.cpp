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

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    
    priceCheck = new EvePriceCheck(this);
    
    connect(ui->runButton, SIGNAL(clicked(bool)), SLOT(run()));
    connect(this, SIGNAL(enterPressed()), ui->runButton, SLOT(click()));
    connect(ui->runButton, SIGNAL(clicked(bool)), ui->progressBar, SLOT(show()));
    connect(priceCheck, SIGNAL(finished(bool)), SLOT(showResult(bool)));
    connect(priceCheck, SIGNAL(progress(int)), ui->progressBar, SLOT(setValue(int)));

    ui->inputEdit->installEventFilter(this);
    ui->progressBar->hide();
}

/*====================================================================================================================*/

Widget::~Widget()
{
    delete ui;
}

void Widget::run()
{
    QString input = ui->inputEdit->text();
    int i;

    for (i = 0; i < input.length(); i++)
    {
        if (input[i] != ' ') break;
    }
    input.remove(0, i);

    if (priceCheck->findResult(input, ui->overPrice->value()))
    {
        return;
    }
}

/*====================================================================================================================*/

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

/*====================================================================================================================*/

bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    QKeyEvent *key;

    switch (event->type())
    {
        case QEvent::FocusIn:
        case QEvent::FocusOut:
            if (watched == ui->inputEdit)
            {
                ui->inputEdit->setFocus();
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

    return false;
}

/*====================================================================================================================*/













