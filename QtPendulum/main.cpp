#include "pendulummainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PendulumMainWindow w;
    w.show();

    return a.exec();
}
