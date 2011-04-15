#ifndef CUTEDRONE_H
#define CUTEDRONE_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QThread>
#include <QFile>
#include <QTimer>
#include <QImage>
#include <navdata_common.h>
#include <VP_Os/vp_os_malloc.h>
#include <VP_Os/vp_os_print.h>
extern "C"
{
#include <VLIB/Stages/vlib_stage_decode.h>
void vp_stages_YUV420P_to_RGB565(void *cfg, vp_api_picture_t *picture, uint8_t *dst, uint32_t dst_rbytes);

}

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

public slots:
    void takeOff();
    void land();
    void turnLeft();
    void turnRight();
    void stay();

protected slots:
    void telemetryReady();
    void videoReady();
    void commandSent(qint64);
    void sendCommand(QByteArray);
    void sendNav(QByteArray);
    void decodeTransform(QByteArray &videoData);
    void stateTimerElapsed();
    void exit_bootstrap_mode();
    void receive_video();
    void receive_telemetry();
    void setHorizontalLevel();
    void setDroneControl(float pitch,float roll,float yaw,float vv);
    void setDroneGain(float fgain,float bgain,float lgain,float rgain);

signals:
    void videoUpdated(QImage);

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
    QImage *image;
    video_controller_t controller;
    vp_api_picture_t picture;
    int pictureWidth;
    int pictureHeight;
    unsigned int num_picture_decoded;
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
