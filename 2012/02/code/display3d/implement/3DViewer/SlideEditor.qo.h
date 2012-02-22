
#ifndef SLIDE_EDITOR_H
#define SLIDE_EDITOR_H

#include <graphics/X11/X_enter.h>
#include <QtCore>
#include <QtGui/QLineEdit>
#include <QtGui/QSlider>
#include <QtGui/QVBoxLayout>
#include <graphics/X11/X_exit.h>

namespace casa {

class SlideEditor : public QWidget
{
   Q_OBJECT

public:
   SlideEditor(QString name,
               double min = 0.,
               double max = 1.,
               double pos = 0.,
               int stepSize = 16,
               int pageStep = 10,
               QWidget *parent = 0);
   ~SlideEditor(){}

public slots:
   void display2(int);
   void display2();

   void setMinimum(double); 
   void setMaximum(double); 
   void setPosition(double);

   void reset(double, double, double);

signals:

   //objName can be obtained from sender(), but 
   //that violates object oriented programming 
   void itemValueChanged(QString& value);
   
   //explicitly send name 
   void itemValueChanged(QString& name, QString& value);

   void itemValueChanged(int);

private:
   QLineEdit *lineEdit;
   QSlider *slider;
   QVBoxLayout *vboxLayout;

   int stepSize;
   QString itemName;
   QString itemValue;
};

}
#endif
