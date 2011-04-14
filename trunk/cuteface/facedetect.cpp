#include "facedetect.h"

FaceDetect::FaceDetect(QObject *parent) :
    QObject(parent)
{
    qDebug() << ">>" << __PRETTY_FUNCTION__;

    qDebug() << "<<" << __PRETTY_FUNCTION__;
}

QImage FaceDetect::IplImage2QImage(const IplImage *iplImage)
{
    qDebug() << ">>" << __PRETTY_FUNCTION__;
        int height = iplImage->height;
        int width = iplImage->width;

        if  (iplImage->depth == IPL_DEPTH_8U && iplImage->nChannels == 3)
        {
          const uchar *qImageBuffer = (const uchar*)iplImage->imageData;
          QImage img(qImageBuffer, width, height, QImage::Format_RGB888);
          return img.rgbSwapped();
        } else if  (iplImage->depth == IPL_DEPTH_8U && iplImage->nChannels == 1){
            const uchar *qImageBuffer = (const uchar*)iplImage->imageData;
            QImage img(qImageBuffer, width, height, QImage::Format_Indexed8);

            QVector<QRgb> colorTable;
            for (int i = 0; i < 256; i++){
                colorTable.push_back(qRgb(i, i, i));
            }
            img.setColorTable(colorTable);
            return img;
        }else{
          qWarning() << "Image cannot be converted.";
          return QImage();
        }
    qDebug() << "<<" << __PRETTY_FUNCTION__;
}

IplImage* FaceDetect::qImage2IplImage(const QImage& qImage)
  {
    qDebug() << ">>" << __PRETTY_FUNCTION__;
    int width = qImage.width();
    int height = qImage.height();

    // Creates a iplImage with 3 channels
    IplImage *img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
    char * imgBuffer = img->imageData;

    //Remove alpha channel
    int jump = (qImage.hasAlphaChannel()) ? 4 : 3;

    for (int y=0;y<img->height;y++){
      QByteArray a((const char*)qImage.scanLine(y), qImage.bytesPerLine());
      for (int i=0; i<a.size(); i+=jump){
          //Swap from RGB to BGR
          imgBuffer[2] = a[i];
          imgBuffer[1] = a[i+1];
          imgBuffer[0] = a[i+2];
          imgBuffer+=3;
      }
  }

    qDebug() << "<<" << __PRETTY_FUNCTION__;
  return img;
  }

QImage FaceDetect::detectFace(QString filename) {
    qDebug() << ">>" << __PRETTY_FUNCTION__;
     IplImage* img = cvLoadImage(filename.toAscii().data(),0);
    int scale = 1;
    CvMemStorage* storage = 0;
    CvHaarClassifierCascade* cascade = 0;
    const char* cascade_name = "../3rdparty/opencv/data/haarcascades/haarcascade_frontalface_alt.xml";
    cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
    if( !cascade ) {
        fprintf( stderr, "ERROR: Could not load classifier cascade\n" );
        return QImage();
    }

    // Allocate the memory storage
    storage = cvCreateMemStorage(0);


    // Create a new image based on the input image
    IplImage* temp = cvCreateImage( cvSize(img->width/scale,img->height/scale), 8, 3 );

    // Create two points to represent the face locations
    CvPoint pt1, pt2;
    int i;

    // Clear the memory storage which was used before
    cvClearMemStorage( storage );

    // Find whether the cascade is loaded, to find the faces. If yes, then:
    if( cascade )
    {

        // There can be more than one face in an image. So create a growable sequence of faces.
        // Detect the objects and store them in the sequence
        CvSeq* faces = cvHaarDetectObjects( img, cascade, storage,
                                            1.1, 2, CV_HAAR_DO_CANNY_PRUNING,
                                            cvSize(40, 40) );

        // Loop the number of faces found.
        for( i = 0; i < (faces ? faces->total : 0); i++ )
        {
           // Create a new rectangle for drawing the face
            CvRect* r = (CvRect*)cvGetSeqElem( faces, i );

            // Find the dimensions of the face,and scale it if necessary
            pt1.x = r->x*scale;
            pt2.x = (r->x+r->width)*scale;
            pt1.y = r->y*scale;
            pt2.y = (r->y+r->height)*scale;

            // Draw the rectangle in the input image
            cvRectangle( img, pt1, pt2, CV_RGB(255,0,0), 3, 8, 0 );
        }
    }

    // Show the image in the window named "result"
//    cvShowImage( "result", img );

    QImage result = IplImage2QImage(img);

    // Release the temp image created.
    cvReleaseImage( &temp );

    qDebug() << "<<" << __PRETTY_FUNCTION__;
    return result;
}
