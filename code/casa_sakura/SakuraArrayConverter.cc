#include <iostream>

#include <libsakura/sakura.h>
//#include <libsakura/config.h>

#include <casa_sakura/SakuraUtils.h>
#include <casa_sakura/SakuraArrayConverter.h>

// AIPS++
#include <casa/aips.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
namespace casa {

template<> void SakuraArrayConverter::CASAToSakura<Complex, float>(
		uInt const index_in, uInt const index_out, const Complex* pin,
		float *output_data) {
	output_data[index_out] = casa::real(pin[index_in]);
}

template<> void SakuraArrayConverter::SakuraToCASA<float, Complex>(
		uInt const index_in, uInt const index_out, float const *input_data,
		Complex* pout) {
	casa::real(pout[index_out]) = input_data[index_in];
	casa::imag(pout[index_out]) = 0.0;
}
}
// End of casa namespace.
