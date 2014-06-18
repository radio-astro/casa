#ifndef _CASA_SAKURA_ARRAY_CONVERTER_H_
#define _CASA_SAKURA_ARRAY_CONVERTER_H_

#include <iostream>
#include <string>
#include <stdexcept>
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
	template<class T, class S> static void ConvertCASAToSakura(
			uInt const pol_index, uInt const num_data,
			const Matrix<T> &input_matrix, S *output_data) {
		uInt const max_pol = input_matrix.nrow();
		if (!(pol_index < max_pol)) {
			throw std::invalid_argument(
					"pol_index must less than input_matrix.nrow()");
		}
		if (!(num_data == input_matrix.ncolumn())) {
			throw std::invalid_argument(
					"num_data must equal to input_matrix.ncolumn()");
		}
		Bool deleteIn;
		const T* pin = input_matrix.getStorage(deleteIn);
		for (uInt i = 0; i < num_data; ++i) {
			CASAToSakura(max_pol * i + pol_index, i, pin, output_data);
		}
		input_matrix.freeStorage(pin, deleteIn);
	}
	template<class S, class T> static void ConvertSakuraToCASA(
			uInt const pol_index, uInt const num_data, S const *input_data,
			Matrix<T> &output_matrix) {
		uInt const max_pol = output_matrix.nrow();
		if (!(pol_index < max_pol)) {
			throw std::invalid_argument(
					"pol_index must less than input_matrix.nrow()");
		}
		if (!(num_data == output_matrix.ncolumn())) {
			throw std::invalid_argument(
					"num_data must equal to output_matrix.ncolumn()");
		}
		Bool deleteOut;
		T* pout = output_matrix.getStorage(deleteOut);
		for (uInt i = 0; i < num_data; ++i) {
			SakuraToCASA(i, max_pol * i + pol_index, input_data, pout);
		}
		output_matrix.putStorage(pout, deleteOut);
	}

private:
	template<class T, class S> static void CASAToSakura(uInt const index_in,
			uInt const index_out, const T* pin, S *output_data) {
		output_data[index_out] = pin[index_in];
	}
	template<class S, class T> static void SakuraToCASA(uInt const index_in,
			uInt const index_out, S const *input_data, T* pout) {
		pout[index_out] = input_data[index_in];
	}
};

template<> void SakuraArrayConverter::CASAToSakura<Complex, float>(
		uInt const index_in, uInt const index_out, const Complex* pin,
		float *output_data);
template<> void SakuraArrayConverter::SakuraToCASA<float, Complex>(
		uInt const index_in, uInt const index_out, float const *input_data,
		Complex* pout);

} //# NAMESPACE CASA - END

#endif /* _CASA_SINGLEDISH_DUMMY_H_ */
