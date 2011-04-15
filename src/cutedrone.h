#ifndef CUTEDRONE_H
#define CUTEDRONE_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QThread>
#include <QFile>
#include <QTimer>
#include <navdata_common.h>

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
    enum droneState {
        notInitialized,
        initialized,
        ready,
        flying
    };

protected slots:
    void telemetryReady();
    void videoReady();
    void commandSent(qint64);
    void sendCommand(QByteArray);
    void sendNav(QByteArray);

    void stateTimerElapsed();
    void takeOff();
    void land();
    void exit_bootstrap_mode();
    void receive_video();
    void receive_telemetry();
    void setHorizontalLevel();
    void setDroneControl(float pitch,float roll,float yaw,float vv);
    void setDroneGain(float fgain,float bgain,float lgain,float rgain);

protected:
    QHostAddress* address;
    QUdpSocket* commandAndControlSocket;
    QUdpSocket* videoSocket;
    QUdpSocket* commandAndControlCriticalSocket;
    QUdpSocket* telemetryAndTrackingSocket;
    QUdpSocket* telemetrySender;
    QTimer* stateTimer;
    QFile log;
    QFile video;

    bool bInitialized;
    int counter;
    int m_iStartBit;

    static const char triggerByte[4];
     int state;

     float m_pitch;     // pitch in horizon
     float m_roll;      // Roll in horizon
     float m_yaw;       // low value to display
     float m_vv;        // Verticl velocity
     float m_altitude;  // altitude
     float m_rotRefX;   // Rotation zero reference
     float m_rotRefY;
     float m_rotRefZ;
     float m_fgain;
     float m_bgain;
     float m_rgain;
     float m_lgain;
};

#endif // CUTEDRONE_H
