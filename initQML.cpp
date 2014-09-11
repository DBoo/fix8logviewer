#include <InitQML.h>
#include <resourceimageprovider.h>
#include <QDebug>
#include <QGraphicsObject>
#include <QDeclarativeContext>
#include <QDeclarativeImageProvider>
#include <QDeclarativeEngine>


InitQML::InitQML(QWidget *MainW,QWidget *parent) :
  QDeclarativeView(parent),mainW(MainW)
{
  resetDone = false;
  QPalette pal = palette();

  pal.setColor(QPalette::Window,QColor("black"));
  // setAutoFillBackground(true);
  pal.setColor( QPalette::Background, Qt::transparent);
  setPalette(pal);
  rip = new ResourceImageProvider(QDeclarativeImageProvider::Pixmap);
  rip->mainW = mainW;
  engine()->addImageProvider("background",rip); 
  setSource(QUrl("qrc:/qml/init.qml"));
  QDeclarativeContext *context = rootContext();
  context->setContextProperty("backgroundColor", 
			      QPalette().color(QPalette::Window));


  QGraphicsObject *object = this->rootObject();
  
  QObject *rect = object->findChild<QObject*>("imageRect");
  /*
  if (!rect) 
    qDebug() <<"Image Rect not found !!!!!!" << __FILE__ << __LINE__;
  else {
    rect->setProperty("src",);
  */
}
void InitQML::fadeIn()
{
     QGraphicsObject *object = this->rootObject();
     if (object) {
        qDebug() << "\tFADE IN";
        object->setProperty("state", "fadein");
    }
}
void InitQML::fadeOut()
{
  QGraphicsObject *object = rootObject();


  if (object) {
    QObject *rect = object->findChild<QObject*>("rect");
    if (rect) {
      rect->setProperty("state", "fadeout");
    }
    else
      qWarning() << "Item not found..." << __FILE__;

    QObject *bgImage = object->findChild<QObject*>("bgPixmap");
    if (bgImage) {
      bgImage->setProperty("source","image://background/snapshot.png");
    }
    else
      qWarning() << "ERROR COULD NOT FIND BG PIXMAP " << __FILE__ << __LINE__;
  }


}
void InitQML::clear()
{
  QGraphicsObject *object = rootObject();
  if (object)
    object->setProperty("state", "hide");
}

/*******************************************/
void InitQML::setText(TextType tt,QString str)
{
  QString textStr;
  QGraphicsObject *object = rootObject();
  if (!object) {
    qWarning() << "qml root object not found" << __FILE__ << __LINE__ ;
    return;
  }
  switch (tt) {
  case UpperText:
    textStr = "textTop";
    break;
  case MiddleText:
    textStr = "textMiddle";
    break;
  case LowerText:
    textStr = "textBottom";
    break;
  }
  QObject *rect = object->findChild<QObject*>(textStr);
  
  if (rect)  {
    rect->setProperty("text",str);
  }
  else
    qWarning() << "QML OBJECT NOT FOUND " << __FILE__ << __LINE__;
  
}
/*******************************************/
void InitQML::resetImage()
{
  QGraphicsObject *object = rootObject();
  QObject *bgImage = object->findChild<QObject*>("bgPixmap");
  rip->resetDone = true;
  if (bgImage) {
    bgImage->setProperty("source","image://background/snapshot.png");
  }
  else
    qWarning() << "ERROR COULD NOT FIND BG PIXMAP " << __FILE__ << __LINE__;

}
