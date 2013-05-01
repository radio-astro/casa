#ifndef _casac_conversions_python_h__
#define _casac_conversions_python_h__
#include <Python.h>
//#include <complexobject.h>
#include <stdcasa/record.h>
//#include <casac/BoolAry.h>
//#include <casac/IntAry.h>
//#include <casac/DoubleAry.h>
//#include <casac/ComplexAry.h>
//#include <casac/StringAry.h>
//#include <WX/Utils/value.h>
#include <string>
#include <tools/casaswig_types.h>
//#include <casac/complex.h>

namespace casac {

//int convert_idl_complex_from_python_complex(PyObject *obj, void *s);
//PyObject *convert_idl_complex_to_python_complex(const casac::complex &from);


//int python_BoolAry_check(PyObject *obj);
//int convert_idl_BoolAry_from_python_BoolAry(PyObject *obj, void *s);
//PyObject *convert_idl_BoolAry_to_python_BoolAry( const BoolAry &from);

//int python_IntAry_check(PyObject *obj);
//int convert_idl_IntAry_from_python_IntAry(PyObject *obj, void *s);
//PyObject *convert_idl_IntAry_to_python_IntAry( const IntAry &from);

//int python_DoubleAry_check(PyObject *obj);
//int convert_idl_DoubleAry_from_python_DoubleAry(PyObject *obj, void *s);
//PyObject *convert_idl_DoubleAry_to_python_DoubleAry( const DoubleAry &from);

//int python_ComplexAry_check(PyObject *obj);
//int convert_idl_ComplexAry_from_python_ComplexAry(PyObject *obj, void *s);
//PyObject *convert_idl_ComplexAry_to_python_ComplexAry( const ComplexAry &from);

//int python_StringAry_check(PyObject *obj);
//int convert_idl_StringAry_from_python_StringAry(PyObject *obj, void *s);
//PyObject *convert_idl_StringAry_to_python_StringAry( const StringAry &from);


variant pyobj2variant(PyObject*,bool throw_error=false);
void pyobj2variant(PyObject*,variant&);
PyObject *variant2pyobj(const variant &val);
PyObject *record2pydict(const record &val);
int convert_record_value_from_python_dict(PyObject *obj, void *s);
//PyObject *convert_record_value_to_python_dict( WX::Utils::SmartPtr<WX::Utils::Value> ptr );
int convert_variant_value_from_python_obj(PyObject *obj, void *s);
//PyObject *convert_variant_value_to_python_obj( WX::Utils::SmartPtr<WX::Utils::Value> ptr );

//WX::Utils::Value *initialize_python_record( );
//WX::Utils::Value *initialize_python_record( const std::string & );
//WX::Utils::Value *initialize_python_variant( );
//WX::Utils::Value *initialize_python_variant( const std::string &a);

int is_intvec_compatible_numpy_array( PyObject *obj );
int convert_intvec_from_compatible_numpy_array( PyObject *obj, void *s );


int pyarray_check(PyObject *obj);

#define AMAP_ARRAY(TYPE) \
PyObject *map_array_numpy(const std::vector<TYPE> &vec, const std::vector<int> &shape);         \
PyObject *map_vector_numpy(const std::vector<TYPE> &vec);                                       \
inline PyObject *map_array( const std::vector<TYPE> &vec, const std::vector<int> &shape ) {     \
                            return map_array_numpy(vec, shape);                                 \
}                                                                                               \
                                                                                                \
inline PyObject *map_vector( const std::vector<TYPE> &vec ) {                                   \
                             return map_vector_numpy( vec );                                    \
}                                                                                               \
void numpy2vector( PyObject *obj, std::vector<TYPE > &vec, std::vector<int> &shape );           \
int pylist2vector( PyObject *array, std::vector<TYPE> &vec, std::vector<int> &shape, int stride = 1, int offset = 0 );


AMAP_ARRAY(std::string)
AMAP_ARRAY(bool)
AMAP_ARRAY(int)
AMAP_ARRAY(long long)
AMAP_ARRAY(double)
AMAP_ARRAY(casac::complex)
AMAP_ARRAY(std::complex<double>)

}

#endif
