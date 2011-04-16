#include <QtCore/QCoreApplication>
#include "../src/cutedrone.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    CuteDrone* drone = new CuteDrone();
    drone->initDrone();
    drone->start();

    int retval = a.exec();

    drone->deleteLater();
    return 0;

}
