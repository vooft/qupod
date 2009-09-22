#include <QtGui/QApplication>
#include "mainwindow.h"

#include <QtDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("qupod");

    MainWindow w;
    w.show();

    return a.exec();
}
