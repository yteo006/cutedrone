#ifndef CUTEDRONE_H
#define CUTEDRONE_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QThread>
#include <QFile>

#define INT_32 unsigned long
#define INT_16 unsigned int
#define INT_8 unsigned char

typedef struct _NAVDATA_OPTION_T {
    INT_16 id;
    INT_16 size;
    INT_8 data;
} NAVDATA_OPTION_T;

typedef struct _NAVDATA_CHECKSUM_T {
    INT_16 id;
    INT_16 size;
    INT_32 data;
} NAVDATA_CHECKSUM_T;

typedef struct _NAVDATA_T {
    INT_32 header;
    INT_32 droneState;
    INT_32 sequenceNumber;
    NAVDATA_OPTION_T options;
    NAVDATA_CHECKSUM_T checksum;
} NAVDATA_T;

class CuteDrone : public QThread {
    Q_OBJECT

public:
    void initDrone();
    void run();

protected slots:
    void telemetryReady();
    void videoReady();
    void commandSent(qint64);
    void sendCommand(QByteArray);

    void takeOff();
    void land();

protected:
    QHostAddress* address;
    QUdpSocket* commandAndControlSocket;
    QUdpSocket* videoSocket;
    QUdpSocket* commandAndControlCriticalSocket;
    QUdpSocket* telemetryAndTrackingSocket;
    QUdpSocket* telemetrySender;
    QFile log;
    QFile video;
    bool bInitialized;
    int counter;
};

#endif // CUTEDRONE_H
