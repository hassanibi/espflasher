#include "mainwindow.h"
#include "tools.h"
#include <QApplication>
#include <QFontDatabase>
#include <QStyle>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFontDatabase::addApplicationFont(":/fonts/code128.ttf");

    if (ESPFlasher::Tools::isWindowsHost()
            && !qFuzzyCompare(qApp->devicePixelRatio(), 1.0)
            && QApplication::style()->objectName().startsWith(
                QLatin1String("windows"), Qt::CaseInsensitive)) {
        QApplication::setStyle(QLatin1String("fusion"));
    }

    MainWindow w;
    w.show();

    return a.exec();
}
