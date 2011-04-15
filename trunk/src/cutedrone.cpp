#include "cutedrone.h"

 const char CuteDrone::triggerByte[4] = {0x01, 0x00, 0x00, 0x00};

void CuteDrone::initDrone()
{
    counter = 1;

       qDebug() << ">> init()";
           this->bInitialized = false;
           log.setFileName("telemetry.txt");
           //video.setFileName("video-drone.bin");
           this->commandAndControlSocket = new QUdpSocket();
           this->commandAndControlCriticalSocket = new QUdpSocket();
           this->telemetryAndTrackingSocket = new QUdpSocket();
           this->videoSocket = new QUdpSocket();
           this->stateTimer = new QTimer();
           this->stateTimer->setInterval(200);
           connect(this->stateTimer,SIGNAL(timeout()),this,SLOT(stateTimerElapsed()));
           this->stateTimer->start();
           //this->image = new QImage(320,240, QImage::Format_RGB32);
           //this->image->fill(0x555555);
           image=new QImage(320,240, QImage::Format_RGB16);
           image->fill(0x5555);

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
    pictureWidth = 320;
    pictureHeight = 240;
    num_picture_decoded=0;
    /// Picture configuration
    picture.format        = PIX_FMT_YUV420P;
    picture.width         = pictureWidth;
    picture.height        = pictureHeight;
    picture.framerate     = 30;
    picture.y_buf         = (uint8_t*)(void*)vp_os_malloc((size_t) pictureWidth*pictureHeight );
    picture.cr_buf        = (uint8_t*)vp_os_malloc( pictureWidth*pictureHeight/4 );
    picture.cb_buf        = (uint8_t*)vp_os_malloc( pictureWidth*pictureHeight/4 );
    picture.y_line_size   = pictureWidth;
    picture.cb_line_size  = pictureWidth / 2;
    picture.cr_line_size  = pictureWidth / 2;
    picture.y_pad         = 0;
    picture.c_pad         = 0;
    video_codec_open(&controller, (codec_type_t)UVLC_CODEC);

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
    QThread::msleep(200);
    receive_video();
    QThread::msleep(200);

/*
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

    // stand still
    setDroneControl(0,0,0,0);
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
    }*/
    log.close();
   // video.close();
 qDebug() << "<< run()";
}

void CuteDrone::turnLeft() {
    receive_video();
    setDroneControl(0,0,-50,0);
}

void CuteDrone::turnRight() {
    receive_video();
    setDroneControl(0,0,50,0);
}

void CuteDrone::stay() {
    receive_video();
    setDroneControl(0,0,0,0);
}

void CuteDrone::forward() {
    receive_video();
    setDroneControl(-50,0,0,0);
}

void CuteDrone::backward() {
    receive_video();
    setDroneControl(50,0,0,0);
}

void CuteDrone::moveLeft() {
    receive_video();
    setDroneControl(0,50,0,0);
}

void CuteDrone::moveRight() {
    receive_video();
    setDroneControl(0,-50,0,0);
}

void CuteDrone::higher() {
    receive_video();
    setDroneControl(0,0,0,50);
}

void CuteDrone::lower() {
    receive_video();
    setDroneControl(0,0,0,-50);
}


void CuteDrone::setDroneControl(float pitch,float roll,float yaw,float vv)
{
    m_pitch=pitch/200.0*m_fgain;
    m_roll=roll/200.0*m_rgain;
    m_yaw=yaw/200.0;
    m_vv=vv/200.0;
    qDebug()  << QString("pitch=%1 roll=%2 yaw=%3 vv=%4\r").arg(m_pitch,3,'F',2).arg(m_roll,3,'F',2).arg(m_yaw,3,'F',2).arg(m_vv,3,'F',2);
};

void CuteDrone::decodeTransform(QByteArray &videoData)
{
    controller.in_stream.bytes   = (uint32_t*)videoData.data();
    controller.in_stream.used    = videoData.size();
    controller.in_stream.size    = videoData.size();
    controller.in_stream.index   = 0;
    controller.in_stream.length  = 32;
    controller.in_stream.code    = 0;
    bool_t got_image = FALSE;
    video_decode_blockline( &controller, &picture, &got_image );
    if( got_image )
        {
          picture.complete     = 1;
          //num_picture_decoded++;
          vp_stages_YUV420P_to_RGB565(NULL,&picture,image->bits(),image->bytesPerLine());
        }
    emit videoUpdated(*image);
    //image->save(QString("pic-%0.jpg").arg(num_picture_decoded),"jpg");
    //qDebug() << "<<" << __PRETTY_FUNCTION__;
}


void CuteDrone::setDroneGain(float fgain,float bgain,float lgain,float rgain)
{

    m_fgain=fgain;
    m_bgain=bgain;
    m_rgain=rgain;
    m_lgain=lgain;
};

void CuteDrone::stateTimerElapsed()
{
    //qDebug() << ">>" << __PRETTY_FUNCTION__ << state;
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
    //qDebug() << "<<" << __PRETTY_FUNCTION__ << state;
}

void CuteDrone::receive_telemetry() {
    qDebug() << ">>" << __PRETTY_FUNCTION__;
    //////////////////////////////////
    // lets receive the telemetry
    telemetryAndTrackingSocket->writeDatagram(QByteArray::fromRawData(triggerByte,sizeof(triggerByte)),QHostAddress("192.168.1.1"),5554);
    qDebug() << "<<" << __PRETTY_FUNCTION__;
}

void CuteDrone::receive_video() {
    qDebug() << ">>" << __PRETTY_FUNCTION__;
    // lets receive the video
    this->videoSocket->writeDatagram(QByteArray::fromRawData(triggerByte,sizeof(triggerByte)),QHostAddress("192.168.1.1"),5555);
    QString cmd("AT*CONFIG=%0,\"general:video_enable\",\"TRUE\"\r");
    QByteArray dgram = cmd.arg(counter++).toLatin1();
    sendCommand(dgram);
    qDebug() << "<<" << __PRETTY_FUNCTION__;
}

void CuteDrone::exit_bootstrap_mode() {
    qDebug() << ">>" << __PRETTY_FUNCTION__;
    //////////////////////////////////7
    // lets exit from bootstrap mode

    //the reduced set is sent by the drone
    QString cmd = "AT*CONFIG=%0,\"general:navdata_demo\",\"TRUE\"\r";
    QByteArray dgram = cmd.arg(counter++).toLatin1();
    sendCommand(dgram);

    cmd = "AT*CTRL=0\r";
    dgram=cmd.toLatin1();
    sendCommand(dgram);
    qDebug() << "<<" << __PRETTY_FUNCTION__;
}

/// we should send this in the background in a loop...
// AT*COMWDG=1

void CuteDrone::land() {
    qDebug() << ">>" << __PRETTY_FUNCTION__;
    m_iStartBit=m_iStartBit&~(1<<9);
    ///////////////////////////////////7
    // lets land the ship
    QString cmd("AT*REF=%0,%1\r");
     QByteArray dgram = cmd.arg(counter++).arg(m_iStartBit).toLatin1();
     sendCommand(dgram);
     dgram = cmd.arg(counter++).arg(m_iStartBit).toLatin1();
     sendCommand(dgram);
     qDebug() << "<<" << __PRETTY_FUNCTION__;
}


void CuteDrone::takeOff() {
    qDebug() << ">>" << __PRETTY_FUNCTION__;
             m_iStartBit=m_iStartBit|(1<<9);
     setHorizontalLevel();
     QThread::msleep(200);
     receive_video();
     QThread::msleep(200);
     ////////////////////////////////////
     // Lets take off
     QString cmd("AT*REF=%0,%1\r");
     QByteArray dgram = cmd.arg(counter++).arg(m_iStartBit).toLatin1();
     sendCommand(dgram);
     dgram = cmd.arg(counter++).arg(m_iStartBit).toLatin1();
     sendCommand(dgram);
     state = flying;
     qDebug() << "<<" << __PRETTY_FUNCTION__;
}

void CuteDrone::setHorizontalLevel() {
    qDebug() << ">>" << __PRETTY_FUNCTION__;
        QString cmd("AT*FTRIM=%0\r");
        QByteArray dgram = cmd.arg(counter++).toLatin1();
        sendCommand(dgram);
        dgram = cmd.arg(counter++).toLatin1();
        sendCommand(dgram);
        qDebug() << "<<" << __PRETTY_FUNCTION__;
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
   /* if (video.isOpen()==false) {
        video.open(QIODevice::WriteOnly);
    }*/

    QUdpSocket* udpSocket = qobject_cast<QUdpSocket*>(sender());
    while (udpSocket->hasPendingDatagrams()) {
             QByteArray datagram;
             datagram.resize(udpSocket->pendingDatagramSize());
             QHostAddress sender;
             quint16 senderPort;

             udpSocket->readDatagram(datagram.data(), datagram.size(),
                                     &sender, &senderPort);
             decodeTransform(datagram);
            /* if (video.isWritable()) {
                video.write(datagram);
                video.flush();
             }*/

         }
}

void CuteDrone::telemetryReady() {
    qDebug() << ">>" << __PRETTY_FUNCTION__;
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
    qDebug() << "<<" << __PRETTY_FUNCTION__;

}
