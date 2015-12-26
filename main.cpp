#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFontDatabase::addApplicationFont(":/fonts/code128.ttf");

    a.setStyle("fusion");
    MainWindow w;
    w.show();

    return a.exec();
}
