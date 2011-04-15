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

private:
    Ui::MainWindow *ui;
    CuteDrone* drone;
};

#endif // MAINWINDOW_H
