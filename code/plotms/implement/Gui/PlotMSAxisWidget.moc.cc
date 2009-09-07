/****************************************************************************
** Meta object code from reading C++ file 'PlotMSAxisWidget.qo.h'
**
** Created: Sun Sep 6 19:55:00 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "plotms/Gui/PlotMSAxisWidget.qo.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PlotMSAxisWidget.qo.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_casa__PlotMSAxisWidget[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      30,   24,   23,   23, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_casa__PlotMSAxisWidget[] = {
    "casa::PlotMSAxisWidget\0\0value\0"
    "axisChanged(QString)\0"
};

const QMetaObject casa::PlotMSAxisWidget::staticMetaObject = {
    { &QtEditingWidget::staticMetaObject, qt_meta_stringdata_casa__PlotMSAxisWidget,
      qt_meta_data_casa__PlotMSAxisWidget, 0 }
};

const QMetaObject *casa::PlotMSAxisWidget::metaObject() const
{
    return &staticMetaObject;
}

void *casa::PlotMSAxisWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_casa__PlotMSAxisWidget))
        return static_cast<void*>(const_cast< PlotMSAxisWidget*>(this));
    if (!strcmp(_clname, "Ui::AxisWidget"))
        return static_cast< Ui::AxisWidget*>(const_cast< PlotMSAxisWidget*>(this));
    return QtEditingWidget::qt_metacast(_clname);
}

int casa::PlotMSAxisWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtEditingWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: axisChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
