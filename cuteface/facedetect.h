#ifndef FACEDETECT_H
#define FACEDETECT_H

#include <QObject>
#include <QImage>
#include <QDebug>
#include "cv.h"
#include "highgui.h"

class FaceDetect : public QObject
{
    Q_OBJECT
public:
    explicit FaceDetect(QObject *parent = 0);

    QImage detectFace(QString);

    QImage IplImage2QImage(const IplImage *);
    IplImage* qImage2IplImage(const QImage& qImage);

signals:

public slots:

};

#endif // FACEDETECT_H
