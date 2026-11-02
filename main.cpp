#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow w;
#ifdef Q_OS_SYMBIAN
    w.showMaximized();
#else
    w.resize(480, 640);
    w.show();
#endif
    return app.exec();
}
