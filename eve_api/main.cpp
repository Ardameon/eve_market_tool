#include "eve_api.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QStyle>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.move(QApplication::desktop()->screen()->rect().center() - w.rect().center());

    w.setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            w.size(),
            qApp->desktop()->availableGeometry()
        )
    );

    w.show();
    
    return a.exec();
}
