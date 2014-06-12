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
	template<class T> static void ConvertComplexMatrixToSakura(
			uInt const num_pol, uInt const num_data,
			Matrix<Complex> &input_complex_matrix, T output_data[]) {
		Bool deleteIn;
		const uInt max_pol = input_complex_matrix.nrow();
		const Complex* pin = input_complex_matrix.getStorage(deleteIn);
		for (uInt i = 0; i < num_data; ++i) {
			output_data[i] = casa::real(pin[max_pol * i + num_pol]);
		}
		input_complex_matrix.freeStorage(pin, deleteIn);
	}
	template<class T> static void ConvertMatrixToSakura(uInt const num_pol,
			uInt const num_data, Matrix<T> &input_matrix, T output_data[]) {
		Bool deleteIn;
		const uInt max_pol = input_matrix.nrow();
		const T* pin = input_matrix.getStorage(deleteIn);
		for (uInt i = 0; i < num_data; ++i) {
			output_data[i] = pin[max_pol * i + num_pol];
		}
		input_matrix.freeStorage(pin, deleteIn);
	}
	template<class T> static void ConvertSakuraToComplexMatrix(
			uInt const num_pol, uInt const num_data, T const input_data[],
			Matrix<Complex> &output_complex_matrix) {
		Bool deleteOut;
		const uInt max_pol = output_complex_matrix.nrow();
		Complex* pout = output_complex_matrix.getStorage(deleteOut);
		for (uInt i = 0; i < num_data; ++i) {
			casa::real(pout[max_pol * i + num_pol]) = input_data[i];
			casa::imag(pout[max_pol * i + num_pol]) = 0.0;
		}
		output_complex_matrix.putStorage(pout, deleteOut);
	}
	template<class T> static void ConvertSakuraToMatrix(uInt const num_pol,
			uInt const num_data, T const input_data[],
			Matrix<T> &output_matrix) {
		Bool deleteOut;
		const uInt max_pol = output_matrix.nrow();
		T* pout = output_matrix.getStorage(deleteOut);
		for (uInt i = 0; i < num_data; ++i) {
			pout[max_pol * i + num_pol] = input_data[i];
		}
		output_matrix.putStorage(pout, deleteOut);
	}
private:
};

} //# NAMESPACE CASA - END

#endif /* _CASA_SINGLEDISH_DUMMY_H_ */
