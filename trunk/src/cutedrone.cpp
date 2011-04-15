#include "cutedrone.h"

 const char CuteDrone::triggerByte[4] = {0x01, 0x00, 0x00, 0x00};

void CuteDrone::initDrone()
{
    counter = 1;

       qDebug() << ">> init()";
           this->bInitialized = false;
           log.setFileName("telemetry.txt");
           video.setFileName("video-drone.bin");
           this->commandAndControlSocket = new QUdpSocket();
           this->commandAndControlCriticalSocket = new QUdpSocket();
           this->telemetryAndTrackingSocket = new QUdpSocket();
           this->videoSocket = new QUdpSocket();
           this->stateTimer = new QTimer();
           this->stateTimer->setInterval(200);
           connect(this->stateTimer,SIGNAL(timeout()),this,SLOT(stateTimerElapsed()));
           this->stateTimer->start();

           QHostAddress address = QHostAddress::Any;

           if (!this->commandAndControlSocket->bind(address,5556)) {
               qDebug() << "FAIL: Could not bind commandAndControlSocket to 5556";
               return;
           } else {
               qDebug() << "OK: bind commandAndControlSocket to 5556";
           }
           if (!this->commandAndControlCriticalSocket->bind(address,5559)) {
               qDebug() << "FAIL: Could not bind commandAndControlCriticalSocket to 5559";
               return;
           } else {
               qDebug() << "OK: bind commandAndControlCriticalSocket to 5559";
           }

           if (!this->telemetryAndTrackingSocket->bind(address,5554)) {
               qDebug() << "FAIL: Could not bind telemetryAndTrackingSocket to 5554";
               return;
           } else {
               qDebug() << "OK: bind telemetryAndTrackingSocket to 5554";

           }

           if (!this->videoSocket->bind(address,5555)) {
               qDebug() << "FAIL: Could not bind videoSocket to 5555";
               return;
           } else {
               qDebug() << "OK: bind videoSocket to 5555";
           }

           this->bInitialized = true;
       qDebug() << "<< init()";
}

void CuteDrone::run() {
    qDebug() << ">> run()";

    connect(this->telemetryAndTrackingSocket,SIGNAL(readyRead()),this,SLOT(telemetryReady()));
    connect(this->telemetryAndTrackingSocket,SIGNAL(bytesWritten(qint64)),this,SLOT(commandSent(qint64)));
    connect(this->commandAndControlCriticalSocket,SIGNAL(readyRead()),this,SLOT(telemetryReady()));
    connect(this->commandAndControlSocket,SIGNAL(readyRead()),this,SLOT(telemetryReady()));
    connect(this->videoSocket,SIGNAL(readyRead()),this,SLOT(videoReady()));

    setDroneGain(1,1,2,1);
    telemetrySender = new QUdpSocket();
    connect(telemetrySender,SIGNAL(readyRead()),this,SLOT(telemetryReady()));
    connect(telemetrySender,SIGNAL(bytesWritten(qint64)),this,SLOT(commandSent(qint64)));

    m_iStartBit = (1<<18)|(1<<20)|(1<<22)|(1<<24)|(1<<28);

    setHorizontalLevel();
    QThread::msleep(500);

    takeOff();
    int wait = 0;
    while (wait<10) {
        // sendCommand(dgram);
        QThread::msleep(500);
        wait++;
    }

    state = flying;

    // go right
    setDroneControl(0,0,150,0);
    wait = 0;
    while (wait<5) {
        QThread::msleep(500);
        wait++;
    }

    // go left
     setDroneControl(0,0,-150,0);
    wait = 0;
    while (wait<5) {
        QThread::msleep(500);
        wait++;
    }

    land();
    wait = 0;
    while (wait<20) {
        QThread::msleep(500);
        wait++;
        land();
    }
    log.close();
    video.close();
 qDebug() << "<< run()";
}

void CuteDrone::setDroneControl(float pitch,float roll,float yaw,float vv)
{
    m_pitch=pitch/200.0*m_fgain;
    m_roll=roll/200.0*m_rgain;
    m_yaw=yaw/200.0;
    m_vv=vv/200.0;
    qDebug()  << QString("pitch=%1 roll=%2 yaw=%3 vv=%4\r").arg(m_pitch,3,'F',2).arg(m_roll,3,'F',2).arg(m_yaw,3,'F',2).arg(m_vv,3,'F',2);
};

void CuteDrone::setDroneGain(float fgain,float bgain,float lgain,float rgain)
{
    m_fgain=fgain;
    m_bgain=bgain;
    m_rgain=rgain;
    m_lgain=lgain;
};

void CuteDrone::stateTimerElapsed()
{
    qDebug() << "thread Timer" << state;
    switch(state) {
        case notInitialized:
            sendNav("AT");
            break;
        case initialized:
            break;
    case ready:
            sendCommand(QString("AT*REF=%1,%2\r").arg(counter++).arg((1<<18) + (1<<20) + (1<<22) + (1<<24) +(1<<28)).toAscii());
            break;
    case flying: {
            float_or_int_t _pitch,_roll,_yaw,_vv;
            int r=(m_pitch!=0.0 || m_roll!=0.0 || m_yaw!=0.0)?1:0;

            _pitch.f=m_pitch;
            _roll.f=m_roll;
            _yaw.f=m_yaw;
            _vv.f=m_vv;
//            qDebug()  << QString("AT*PCMD=%1,0,%2,%3,%4,%5\r").arg(seq).arg(_roll.f,3,'F',2).arg(_pitch.f,3,'F',2).arg(_vv.f,3,'F',2).arg(_yaw.f,3,'F',2);
//            qDebug() << QString("AT*PCMD=%1,0,%2,%3,%4,%5\r").arg(seq).arg(_roll.i,8,16).arg(_pitch.i,8,16).arg(_vv.i,8,16).arg(_yaw.i,8,16);
            sendCommand(QString("AT*COMWDG=%1\r").arg(counter++).toAscii());
            sendCommand(QString("AT*PCMD=%1,%2,%3,%4,%5,%6\r").arg(counter++).arg(r).arg(_roll.i).arg(_pitch.i).arg(_vv.i).arg(_yaw.i).toAscii());


            break;
        }
}
}

void CuteDrone::receive_telemetry() {
    //////////////////////////////////
    // lets receive the telemetry
    telemetryAndTrackingSocket->writeDatagram(QByteArray::fromRawData(triggerByte,sizeof(triggerByte)),QHostAddress("192.168.1.1"),5554);
}

void CuteDrone::receive_video() {
    // lets receive the video
    this->videoSocket->writeDatagram(QByteArray::fromRawData(triggerByte,sizeof(triggerByte)),QHostAddress("192.168.1.1"),5555);
    QString cmd("AT*CONFIG=%0,\"general:video_enable\",\"TRUE\"\r");
    QByteArray dgram = cmd.arg(counter++).toLatin1();
    sendCommand(dgram);
}

void CuteDrone::exit_bootstrap_mode() {
    //////////////////////////////////7
    // lets exit from bootstrap mode

    //the reduced set is sent by the drone
    QString cmd = "AT*CONFIG=%0,\"general:navdata_demo\",\"TRUE\"\r";
    QByteArray dgram = cmd.arg(counter++).toLatin1();
    sendCommand(dgram);

    cmd = "AT*CTRL=0\r";
    dgram=cmd.toLatin1();
    sendCommand(dgram);
}

/// we should send this in the background in a loop...
// AT*COMWDG=1

void CuteDrone::land() {
    m_iStartBit=m_iStartBit&~(1<<9);
    ///////////////////////////////////7
    // lets land the ship
    QString cmd("AT*REF=%0,%1\r");
     QByteArray dgram = cmd.arg(counter++).arg(m_iStartBit).toLatin1();
     sendCommand(dgram);
     dgram = cmd.arg(counter++).arg(m_iStartBit).toLatin1();
     sendCommand(dgram);
}


void CuteDrone::takeOff() {

             m_iStartBit=m_iStartBit|(1<<9);
     ////////////////////////////////////
     // Lets take off
     QString cmd("AT*REF=%0,%1\r");
     QByteArray dgram = cmd.arg(counter++).arg(m_iStartBit).toLatin1();
     sendCommand(dgram);
     dgram = cmd.arg(counter++).arg(m_iStartBit).toLatin1();
     sendCommand(dgram);
}

void CuteDrone::setHorizontalLevel() {
        QString cmd("AT*FTRIM=%0\r");
        QByteArray dgram = cmd.arg(counter++).toLatin1();
        sendCommand(dgram);
        dgram = cmd.arg(counter++).toLatin1();
        sendCommand(dgram);
}

void CuteDrone::sendCommand(QByteArray dgram) {
    commandAndControlSocket->writeDatagram(dgram.data(),dgram.size(),QHostAddress("192.168.1.1"),5556);
}

void CuteDrone::sendNav(QByteArray dgram) {
    telemetryAndTrackingSocket->writeDatagram(dgram.data(),dgram.size(),QHostAddress("192.168.1.1"),5554);
}

void CuteDrone::commandSent(qint64 d) {
}

void CuteDrone::videoReady() {
    if (video.isOpen()==false) {
        video.open(QIODevice::WriteOnly);
    }
    QUdpSocket* udpSocket = qobject_cast<QUdpSocket*>(sender());
    while (udpSocket->hasPendingDatagrams()) {
             QByteArray datagram;
             datagram.resize(udpSocket->pendingDatagramSize());
             QHostAddress sender;
             quint16 senderPort;

             udpSocket->readDatagram(datagram.data(), datagram.size(),
                                     &sender, &senderPort);

             if (video.isWritable()) {
                video.write(datagram);
                video.flush();
             }

         }

}

void CuteDrone::telemetryReady() {
    if (log.isOpen()==false) {
        log.open(QIODevice::WriteOnly);
    }
    QUdpSocket* udpSocket = qobject_cast<QUdpSocket*>(sender());
    while (udpSocket->hasPendingDatagrams()) {
             QByteArray datagram;
             datagram.resize(udpSocket->pendingDatagramSize());
             QHostAddress sender;
             quint16 senderPort;

             udpSocket->readDatagram(datagram.data(), datagram.size(),
                                     &sender, &senderPort);

             //char* header = new char[sizeof(INT_32)];
             char* raw = new char[datagram.size()];
             memcpy(raw,datagram.data(),datagram.size());
             qDebug() << QByteArray::fromRawData(raw,sizeof(raw)).toHex();

            //QByteArray data = (datagram << 32);
             //raw = raw<<sizeof(INT_32);
             //qDebug() << QByteArray::fromRawData(header,sizeof(header)).toHex();
             qDebug() << QByteArray::fromRawData(raw,sizeof(raw)).toHex();

             if (log.isWritable()) {
                     log.write(datagram.toHex());
                 log.write("\r\n");
                log.flush();
             }

         }

}
