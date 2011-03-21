#include "cutedrone.h"

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

    QByteArray dgram;
    connect(this->telemetryAndTrackingSocket,SIGNAL(readyRead()),this,SLOT(telemetryReady()));
    connect(this->telemetryAndTrackingSocket,SIGNAL(bytesWritten(qint64)),this,SLOT(commandSent(qint64)));
    connect(this->commandAndControlCriticalSocket,SIGNAL(readyRead()),this,SLOT(telemetryReady()));
    connect(this->commandAndControlSocket,SIGNAL(readyRead()),this,SLOT(telemetryReady()));
    connect(this->videoSocket,SIGNAL(readyRead()),this,SLOT(videoReady()));

    telemetrySender = new QUdpSocket();
    connect(telemetrySender,SIGNAL(readyRead()),this,SLOT(telemetryReady()));
    connect(telemetrySender,SIGNAL(bytesWritten(qint64)),this,SLOT(commandSent(qint64)));

  /*  telemetrySender->writeDatagram("AT*CTRL=0",9,QHostAddress::LocalHost,5555);
    telemetrySender->writeDatagram("AT*CTRL=0",9,QHostAddress::LocalHost,5554);
    telemetrySender->writeDatagram("AT*CTRL=0",9,QHostAddress::LocalHost,5556);
    telemetrySender->writeDatagram("AT*CTRL=0",9,QHostAddress::LocalHost,5559);
*/

    //////////////////////////////////
    // lets receive the telemetry
    const char triggerByte[] = {0x01, 0x00, 0x00, 0x00};
    telemetryAndTrackingSocket->writeDatagram(QByteArray::fromRawData(triggerByte,sizeof(triggerByte)),QHostAddress("192.168.1.1"),5554);
    //telemetryAndTrackingSocket->writeDatagram(QByteArray("s"),QHostAddress("192.168.1.1"),5554);
    //telemetryAndTrackingSocket->writeDatagram(QByteArray("s"),QHostAddress("192.168.1.1"),5554);

    // lets receive the video
    this->videoSocket->writeDatagram(QByteArray::fromRawData(triggerByte,sizeof(triggerByte)),QHostAddress("192.168.1.1"),5555);
    QString cmd("AT*CONFIG=%0,\"general:video_enable\",\"TRUE\"\r");
    dgram=cmd.arg(counter++).toLatin1();
    sendCommand(dgram);



    //////////////////////////////////7
    // lets exit from bootstrap mode
    cmd = "AT*CONFIG=%0,\"general:navdata_demo\",\"TRUE\"\r";
    dgram=cmd.arg(counter++).toLatin1();
    sendCommand(dgram);

    cmd = "AT*CTRL=0\r";
    dgram=cmd.toLatin1();
    sendCommand(dgram);


/*    int m_iStartBit = (1<<18)|(1<<20)|(1<<22)|(1<<24)|(1<<28);
            iStartBit=iStartBit|(1<<9);
    ////////////////////////////////////
    // Lets take off
    QString cmd2("AT*REF=%0,%1\r");
    dgram = cmd2.arg(counter++).arg(iStartBit).toLatin1();
    sendCommand(dgram);
    dgram = cmd2.arg(counter++).arg(iStartBit).toLatin1();
    sendCommand(dgram);

  //  cmd2 = "AT*CTRL=2,0\n";
  //  dgram = cmd2.toLatin1();
  //  sendCommand(dgram);

    //telemetrySender->writeDatagram(QByteArray("AT*CTRL=0\r\n"),QHostAddress("192.168.1.255"),5556);
    //telemetrySender->writeDatagram(QByteArray("AT*CTRL=0\r\n"),QHostAddress("192.168.1.255"),5559);
int wait = 0;
 while (wait<5) {
    // sendCommand(dgram);
    QThread::msleep(500);
    wait++;
}
///////////////////////////////////7
// lets land the ship
 iStartBit=iStartBit&~(1<<9);
 dgram = cmd2.arg(counter++).arg(iStartBit).toLatin1();
 sendCommand(dgram);
 dgram = cmd2.arg(counter++).arg(iStartBit).toLatin1();
 sendCommand(dgram);
*/
    int wait = 0;
     while (wait<5) {
        QThread::msleep(500);
        wait++;
    }
     log.close();
     video.close();
 qDebug() << "<< run()";
}

/// we should send this in the background in a loop...
// AT*COMWDG=1

void CuteDrone::land() {}
void CuteDrone::takeOff() {}

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
