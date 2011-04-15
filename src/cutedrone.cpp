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

    telemetrySender = new QUdpSocket();
    connect(telemetrySender,SIGNAL(readyRead()),this,SLOT(telemetryReady()));
    connect(telemetrySender,SIGNAL(bytesWritten(qint64)),this,SLOT(commandSent(qint64)));

    m_iStartBit = (1<<18)|(1<<20)|(1<<22)|(1<<24)|(1<<28);

    takeOff();
    int wait = 0;
    while (wait<5) {
        // sendCommand(dgram);
        QThread::msleep(500);
        wait++;
    }

    land();
    wait = 0;
    while (wait<5) {
        QThread::msleep(500);
        wait++;
        land();
    }
    log.close();
    video.close();
 qDebug() << "<< run()";
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

void CuteDrone::sendCommand(QByteArray dgram) {
    commandAndControlSocket->writeDatagram(dgram.data(),dgram.size(),QHostAddress("192.168.1.1"),5556);
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
