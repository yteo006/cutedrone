#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../src/cutedrone.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void updateImage(QImage img);
    void on_pushButtonTurnLeft_clicked() { drone->turnLeft(); }
    void on_pushButtonTurnRight_clicked() { drone->turnRight(); }
    void on_pushButtonStay_clicked() { drone->stay(); }
    void on_pushButtonLand_clicked() { drone->land(); }
    void on_pushButtonTakeOff_clicked() { drone->takeOff(); }

private:
    Ui::MainWindow *ui;
    CuteDrone* drone;
};

#endif // MAINWINDOW_H
