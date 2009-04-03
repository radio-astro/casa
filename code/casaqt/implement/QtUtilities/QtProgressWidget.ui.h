/********************************************************************************
** Form generated from reading ui file 'QtProgressWidget.ui'
**
** Created: Fri Apr 3 11:05:10 2009
**      by: Qt User Interface Compiler version 4.4.0-beta1
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef QTPROGRESSWIDGET_H
#define QTPROGRESSWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ProgressWidget
{
public:
    QVBoxLayout *vboxLayout;
    QSpacerItem *spacerItem;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem1;
    QVBoxLayout *vboxLayout1;
    QHBoxLayout *hboxLayout1;
    QLabel *label;
    QLabel *operationLabel;
    QLabel *label2;
    QSpacerItem *spacerItem2;
    QProgressBar *progressBar;
    QLabel *statusLabel;
    QHBoxLayout *hboxLayout2;
    QSpacerItem *spacerItem3;
    QPushButton *backgroundButton;
    QPushButton *pauseButton;
    QPushButton *cancelButton;
    QSpacerItem *spacerItem4;
    QSpacerItem *spacerItem5;

    void setupUi(QWidget *ProgressWidget)
    {
    if (ProgressWidget->objectName().isEmpty())
        ProgressWidget->setObjectName(QString::fromUtf8("ProgressWidget"));
    ProgressWidget->resize(314, 107);
    vboxLayout = new QVBoxLayout(ProgressWidget);
    vboxLayout->setSpacing(0);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    vboxLayout->setContentsMargins(0, 0, 0, 0);
    spacerItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout->addItem(spacerItem);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem1);

    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setSpacing(3);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    hboxLayout1->setContentsMargins(-1, 0, -1, -1);
    label = new QLabel(ProgressWidget);
    label->setObjectName(QString::fromUtf8("label"));

    hboxLayout1->addWidget(label);

    operationLabel = new QLabel(ProgressWidget);
    operationLabel->setObjectName(QString::fromUtf8("operationLabel"));

    hboxLayout1->addWidget(operationLabel);

    label2 = new QLabel(ProgressWidget);
    label2->setObjectName(QString::fromUtf8("label2"));

    hboxLayout1->addWidget(label2);

    spacerItem2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout1->addItem(spacerItem2);


    vboxLayout1->addLayout(hboxLayout1);

    progressBar = new QProgressBar(ProgressWidget);
    progressBar->setObjectName(QString::fromUtf8("progressBar"));
    progressBar->setValue(0);

    vboxLayout1->addWidget(progressBar);

    statusLabel = new QLabel(ProgressWidget);
    statusLabel->setObjectName(QString::fromUtf8("statusLabel"));
    statusLabel->setWordWrap(true);

    vboxLayout1->addWidget(statusLabel);

    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setSpacing(3);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    spacerItem3 = new QSpacerItem(101, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout2->addItem(spacerItem3);

    backgroundButton = new QPushButton(ProgressWidget);
    backgroundButton->setObjectName(QString::fromUtf8("backgroundButton"));
    backgroundButton->setEnabled(true);

    hboxLayout2->addWidget(backgroundButton);

    pauseButton = new QPushButton(ProgressWidget);
    pauseButton->setObjectName(QString::fromUtf8("pauseButton"));
    pauseButton->setEnabled(true);

    hboxLayout2->addWidget(pauseButton);

    cancelButton = new QPushButton(ProgressWidget);
    cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
    cancelButton->setEnabled(true);

    hboxLayout2->addWidget(cancelButton);


    vboxLayout1->addLayout(hboxLayout2);


    hboxLayout->addLayout(vboxLayout1);

    spacerItem4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem4);


    vboxLayout->addLayout(hboxLayout);

    spacerItem5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout->addItem(spacerItem5);


    retranslateUi(ProgressWidget);

    QMetaObject::connectSlotsByName(ProgressWidget);
    } // setupUi

    void retranslateUi(QWidget *ProgressWidget)
    {
    ProgressWidget->setWindowTitle(QApplication::translate("ProgressWidget", "Progress", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("ProgressWidget", "Progress of operation ", 0, QApplication::UnicodeUTF8));
    operationLabel->setText(QApplication::translate("ProgressWidget", "[[operation]]", 0, QApplication::UnicodeUTF8));
    label2->setText(QApplication::translate("ProgressWidget", ":", 0, QApplication::UnicodeUTF8));
    statusLabel->setText(QApplication::translate("ProgressWidget", "[[status]]", 0, QApplication::UnicodeUTF8));
    backgroundButton->setText(QApplication::translate("ProgressWidget", "Background", 0, QApplication::UnicodeUTF8));
    pauseButton->setText(QApplication::translate("ProgressWidget", "Pause", 0, QApplication::UnicodeUTF8));
    cancelButton->setText(QApplication::translate("ProgressWidget", "Cancel", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(ProgressWidget);
    } // retranslateUi

};

namespace Ui {
    class ProgressWidget: public Ui_ProgressWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // QTPROGRESSWIDGET_H
