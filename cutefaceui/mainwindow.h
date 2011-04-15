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
    void on_pushButtonForward_clicked() { drone->forward(); }
    void on_pushButtonBackward_clicked() { drone->backward(); }
    void on_pushButtonMoveLeft_clicked() { drone->moveLeft(); }
    void on_pushButtonMoveRight_clicked() { drone->moveRight(); }
    void on_pushButtonHigher_clicked() { drone->higher(); }
    void on_pushButtonLower_clicked() { drone->lower(); }

private:
    Ui::MainWindow *ui;
    CuteDrone* drone;
};

#endif // MAINWINDOW_H
