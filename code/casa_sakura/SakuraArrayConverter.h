#ifndef _CASA_SAKURA_ARRAY_CONVERTER_H_
#define _CASA_SAKURA_ARRAY_CONVERTER_H_

#include <iostream>
#include <string>
//#include <libsakura/sakura.h>
#include <casa/aipstype.h>

// AIPS++
#include <casa/aips.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>

using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN

class SakuraArrayConverter {
protected:

public:
	template <class T, class S> static void ConvertCASAToSakura(uInt const index_in,
			uInt const index_out, const T* pin,S *output_data){
	        output_data[index_out] = pin[index_in];
	}
	template <class S, class T> static void ConvertSakuraToCASA(uInt const index_in,
			uInt const index_out, S const *input_data,T* pout){
		    pout[index_out] = input_data[index_in];
	}
private:
};

template<> void SakuraArrayConverter::ConvertCASAToSakura<Complex, float>(uInt const index_in,
		uInt const index_out, const Complex* pin,float *output_data);
template<> void SakuraArrayConverter::ConvertSakuraToCASA<float,Complex>(uInt const index_in,
		uInt const index_out, float const *input_data,Complex* pout);

} //# NAMESPACE CASA - END

#endif /* _CASA_SINGLEDISH_DUMMY_H_ */
