#ifndef _casac_conversions_python_h__
#define _casac_conversions_python_h__
#include <casac/complex.h>
#include <Python.h>
#include <complexobject.h>
#include <xmlcasa/record.h>
#include <casac/BoolAry.h>
#include <casac/IntAry.h>
#include <casac/DoubleAry.h>
#include <casac/ComplexAry.h>
#include <casac/StringAry.h>
#include <WX/Utils/value.h>
#include <string>

namespace casac {

int convert_idl_complex_from_python_complex(PyObject *obj, void *s);
PyObject *convert_idl_complex_to_python_complex(const casac::complex &from);


int python_BoolAry_check(PyObject *obj);
int convert_idl_BoolAry_from_python_BoolAry(PyObject *obj, void *s);
PyObject *convert_idl_BoolAry_to_python_BoolAry( const BoolAry &from);

int python_IntAry_check(PyObject *obj);
int convert_idl_IntAry_from_python_IntAry(PyObject *obj, void *s);
PyObject *convert_idl_IntAry_to_python_IntAry( const IntAry &from);

int python_DoubleAry_check(PyObject *obj);
int convert_idl_DoubleAry_from_python_DoubleAry(PyObject *obj, void *s);
PyObject *convert_idl_DoubleAry_to_python_DoubleAry( const DoubleAry &from);

int python_ComplexAry_check(PyObject *obj);
int convert_idl_ComplexAry_from_python_ComplexAry(PyObject *obj, void *s);
PyObject *convert_idl_ComplexAry_to_python_ComplexAry( const ComplexAry &from);

int python_StringAry_check(PyObject *obj);
int convert_idl_StringAry_from_python_StringAry(PyObject *obj, void *s);
PyObject *convert_idl_StringAry_to_python_StringAry( const StringAry &from);


variant pyobj2variant(PyObject*,bool throw_error=false);
PyObject *variant2pyobj(const variant &val);
int convert_record_value_from_python_dict(PyObject *obj, void *s);
PyObject *convert_record_value_to_python_dict( WX::Utils::SmartPtr<WX::Utils::Value> ptr );
int convert_variant_value_from_python_obj(PyObject *obj, void *s);
PyObject *convert_variant_value_to_python_obj( WX::Utils::SmartPtr<WX::Utils::Value> ptr );

WX::Utils::Value *initialize_python_record( );
WX::Utils::Value *initialize_python_record( const std::string & );
WX::Utils::Value *initialize_python_variant( );
WX::Utils::Value *initialize_python_variant( const std::string &a);

}

#endif
