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
}

void MainWindow::updateImage(QImage img) {
    ui->graphicsView->setScene(new QGraphicsScene());
    ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(img));
}

MainWindow::~MainWindow()
{
    delete ui;
}
