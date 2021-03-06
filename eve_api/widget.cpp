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
    connect(ui->rbESI, SIGNAL(toggled(bool)), ui->sellOrdersPrecision, SLOT(setEnabled(bool)));
    connect(ui->rbESI, SIGNAL(toggled(bool)), ui->precisionLabel, SLOT(setEnabled(bool)));

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

    if (ui->rbESI->isChecked())
    {
        priceCheck->setSource(EvePriceCheck::SOURCE_ESI);
        priceCheck->setESIPrecision(ui->sellOrdersPrecision->value());
    } else {
        priceCheck->setSource(EvePriceCheck::SOURCE_EVE_CENTRAL);
    }

    if (priceCheck->findResult(input, ui->overPrice->value()))
    {
        return;
    }
}

/*====================================================================================================================*/

void Widget::showResult(bool success)
{
    QString newPrice(priceCheck->getNewPriceStr());
    QString basePrice(priceCheck->getBasePriceStr());
    QString buyPrice(priceCheck->getBuyPriceStr());

    if (success)
    {
        ui->basePriceLabel->setText(basePrice + " ISK");
        ui->buyPriceLabel->setText(buyPrice + " ISK");
        ui->newPriceLabel->setText(newPrice + " ISK");
        ui->imageLabel->setPixmap(priceCheck->getPicture());
        ui->diffLabel->setText(priceCheck->getDiffPercentStr());
    } else {
        ui->basePriceLabel->setText("Error");
        ui->buyPriceLabel->setText("Error");
        ui->newPriceLabel->setText("Error");
        ui->diffLabel->clear();
        ui->imageLabel->clear();
    }

    if (priceCheck->getDiffPercent() <= 15.0)
    {
        ui->diffLabel->setStyleSheet("QLabel { color : green; }");
    } else {
        ui->diffLabel->setStyleSheet("QLabel { color : red; }");
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













