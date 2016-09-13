#include <graphics/X11/X_enter.h>
#include <graphics/X11/X_exit.h>

#include <math.h>
#include <display3d/3DViewer/SlideEditor.qo.h>

namespace casa {

SlideEditor::SlideEditor(QString name,
                         double min, 
                         double max,
                         double pos,
                         int step,
                         int pageStep,
                         QWidget *parent) {

   setObjectName(name);
   setMinimumSize(QSize(140, 40));

   vboxLayout = new QVBoxLayout(this);
   vboxLayout->setSpacing(0);
   vboxLayout->setMargin(0);
   vboxLayout->setObjectName(
       QString::fromUtf8("slideEidtorLayout"));
   vboxLayout->setSizeConstraint(
       QLayout::SetDefaultConstraint);

   stepSize = step;
   itemValue = QString::number(pos);
   itemName = name;


   slider = new QSlider;
   slider->setObjectName(QString::fromUtf8("slider"));
   slider->setMinimumSize(QSize(140, 0));
   slider->setMaximumSize(QSize(160, 20));
   slider->setBaseSize(QSize(160, 0));
   slider->setOrientation(Qt::Horizontal);
   //slider->setTickPosition(QSlider::TicksAbove);
   slider->setTickPosition(QSlider::NoTicks);
   if (pageStep < 1) {
      pageStep = 1;
   }
   if (stepSize < 1) {
      stepSize = 1;
   }
   if (min > max) {
      double tmp = min;
      min = max;
      max = tmp;
   }
   slider->setMinimum((int)(min * stepSize));
   slider->setMaximum((int)(max * stepSize));
   slider->setSingleStep(stepSize);
   slider->setPageStep(pageStep * stepSize);
   //slider->setSliderPosition((int)(pos * stepSize));
   slider->setValue((int)(pos * stepSize));
   slider->setTickInterval((int)(pos * stepSize));
   connect(slider, SIGNAL(valueChanged(int)), 
                   SIGNAL(itemValueChanged(int)));
   vboxLayout->addWidget(slider);

   lineEdit = new QLineEdit;
   lineEdit->setObjectName(
       QString::fromUtf8("lineEdit"));
   lineEdit->setMinimumSize(QSize(140, 0));
   lineEdit->setMaximumSize(QSize(160, 20));
   lineEdit->setBaseSize(QSize(160, 0));
   connect(lineEdit, SIGNAL(editingFinished()), 
                     SLOT(display2()));
   vboxLayout->addWidget(lineEdit);

   //setText will emit valuseChanged
   //lineEdit->setText(itemValue);
   display2((int)(pos * stepSize));

   //qDebug() << "max=" << max << "min=" << min
   //         << "step=" << stepSize
   //         << "page=" << pageStep;
}

void SlideEditor::reset(double min, double max, double pos) {
   qDebug() << "reset" << min << max << pos;
   slider->setMinimum((int)floor(min * stepSize));
   //qDebug() << "min------" << (int)floor(min * stepSize);
   slider->setMaximum((int)ceil(max * stepSize));
   //qDebug() << "max------" << (int)ceil(max * stepSize);
   //slider->setSliderPosition((int)(pos * stepSize));
   slider->setValue((int)(pos * stepSize));
   //qDebug() << "val------" << (int)(pos * stepSize);
   itemValue = QString::number(pos);
   //lineEdit->setText(itemValue);
   //qDebug() << "text------" << itemValue;
}

void SlideEditor::setMinimum(double a) {
   slider->setMinimum((int)(a * stepSize));
}
   
void SlideEditor::setMaximum(double a) {
   slider->setMaximum((int)(a * stepSize));
}

void SlideEditor::setPosition(double a) {
   slider->setValue((int)(a * stepSize));
}

void SlideEditor::display2() {
    QString entered = lineEdit->text();
    //qDebug() << "editingFinished" << entered;
    bool ok;
    double val = entered.toDouble(&ok);

    if (!ok) {
       lineEdit->setText(itemValue);
       return;
    }

    display2((int)(val * stepSize));
}

void SlideEditor::display2(int value) {
    //slider->blockSignals(true);
    //qDebug() << "value=" << value;
    if (value > slider->maximum() || 
        value < slider->minimum())
        return;

    slider->setSliderPosition (value);
    int range = slider->maximum() - slider->minimum();
    int pos = slider->value() - slider->minimum();

    double val = (double)value / stepSize; 
    //qDebug() << "val:" << val << "itemValue:" << itemValue;
    itemValue.setNum(val);
    lineEdit->setText(itemValue);

    if (range != 0 && 2 * pos > range) {
       lineEdit->setAlignment(Qt::AlignRight);
    }
    else {
       lineEdit->setAlignment(Qt::AlignLeft);
    }

    //qDebug() << "itemValue" << itemValue << stepSize;
    //emit itemValueChanged(itemValue);
    //emit itemValueChanged(itemName, itemValue);
    //emit itemValueChanged(value);
    //slider->blockSignals(false);
}


}
