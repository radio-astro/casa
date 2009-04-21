/****************************************************************************
** Meta object code from reading C++ file 'QtProgressWidget.qo.h'
**
** Created: Mon Apr 20 17:15:15 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.0-beta1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "casaqt/QtUtilities/QtProgressWidget.qo.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtProgressWidget.qo.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.0-beta1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_casa__QtProgressWidget[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      24,   23,   23,   23, 0x05,
      46,   23,   23,   23, 0x05,
      63,   23,   23,   23, 0x05,
      81,   23,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
     113,   99,   23,   23, 0x0a,
     140,  133,   23,   23, 0x0a,
     168,  159,   23,   23, 0x0a,
     202,  186,   23,   23, 0x0a,
     228,   23,   23,   23, 0x0a,
     239,   23,   23,   23, 0x08,
     258,  252,   23,   23, 0x08,
     276,   23,   23,   23, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_casa__QtProgressWidget[] = {
    "casa::QtProgressWidget\0\0backgroundRequested()\0"
    "pauseRequested()\0resumeRequested()\0"
    "cancelRequested()\0operationName\0"
    "initialize(QString)\0status\0"
    "setStatus(QString)\0progress\0"
    "setProgress(uint)\0progress,status\0"
    "setProgress(uint,QString)\0finalize()\0"
    "background()\0pause\0pauseResume(bool)\0"
    "cancel()\0"
};

const QMetaObject casa::QtProgressWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_casa__QtProgressWidget,
      qt_meta_data_casa__QtProgressWidget, 0 }
};

const QMetaObject *casa::QtProgressWidget::metaObject() const
{
    return &staticMetaObject;
}

void *casa::QtProgressWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_casa__QtProgressWidget))
	return static_cast<void*>(const_cast< QtProgressWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int casa::QtProgressWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: backgroundRequested(); break;
        case 1: pauseRequested(); break;
        case 2: resumeRequested(); break;
        case 3: cancelRequested(); break;
        case 4: initialize((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: setStatus((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: setProgress((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 7: setProgress((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 8: finalize(); break;
        case 9: background(); break;
        case 10: pauseResume((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: cancel(); break;
        }
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void casa::QtProgressWidget::backgroundRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void casa::QtProgressWidget::pauseRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void casa::QtProgressWidget::resumeRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void casa::QtProgressWidget::cancelRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}
QT_END_MOC_NAMESPACE
