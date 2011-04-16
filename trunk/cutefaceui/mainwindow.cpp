#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    drone = new CuteDrone();
    connect(drone,SIGNAL(videoUpdated(QImage)),this,SLOT(updateImage(QImage)));
    drone->initDrone();
    drone->start();

    this->controlSocket = new QUdpSocket();
    QHostAddress address = QHostAddress::Any;
    this->controlSocket->bind(address,4321);
    connect(this->controlSocket,SIGNAL(readyRead()),this,SLOT(controlMessageReceived()));
}

void MainWindow::controlMessageReceived() {
    qDebug() << "message received";
    QUdpSocket* udpSocket = qobject_cast<QUdpSocket*>(sender());
    while (udpSocket->hasPendingDatagrams()) {
             QByteArray datagram;
             datagram.resize(udpSocket->pendingDatagramSize());
             QHostAddress sender;
             quint16 senderPort;

             udpSocket->readDatagram(datagram.data(), datagram.size(),
                                     &sender, &senderPort);
             if (datagram=="left") {
                 drone->moveLeft();
             } else if (datagram== "right") {
                 drone->moveRight();
             } else if (datagram=="stayx" && datagram=="stayy"){
                 drone->stay();
             } else if (datagram=="up") {
                 drone->higher();
             } else if (datagram=="down") {
                 drone->lower();
             } else {
                 qDebug() << "invalid command received";
             }
         }

}

void MainWindow::updateImage(QImage img) {
    ui->graphicsView->setScene(new QGraphicsScene());
    ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(img));
}

MainWindow::~MainWindow()
{
    delete ui;
}
