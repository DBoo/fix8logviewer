#ifndef InitQML_H
#define InitQML_H
class ResourceImageProvider;
#include <QDeclarativeView>

class InitQML : public QDeclarativeView
{
  Q_OBJECT
    public:
  explicit InitQML(QWidget *mainW,QWidget *parent = 0);
  void resetImage();
  typedef enum {UpperText,MiddleText,LowerText} TextType;
public:
  QWidget *mainW;
  void fadeIn();
  void fadeOut();
  void clear();
  void setText(TextType,QString);
  bool resetDone;
  ResourceImageProvider* rip;
};
#endif 
