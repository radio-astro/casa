#include <iostream>

#include <libsakura/sakura.h>
//#include <libsakura/config.h>
#include <tr1/memory>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>

#include <casa_sakura/SakuraUtils.h>
#include <casa_sakura/SakuraArrayConverter.h>

// AIPS++
#include <casa/aips.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Containers/RecordField.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/Block.h>
#include <casa/Quanta/MVTime.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSPointing.h>

#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TableRow.h>

#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <measures/TableMeasures/ScalarQuantColumn.h>
#include <measures/TableMeasures/ArrayQuantColumn.h>

#define _ORIGIN LogOrigin("Sakura", __func__, WHERE)

namespace casa {

void SakuraArrayConverter::ConvertComplexMatrixToSakura(uInt const num_pol,
		uInt const num_data, Matrix<Complex> &input_complex_matrix,
		float output_data[]) {
	try {
		Bool deleteIn;
		const uInt max_pol = input_complex_matrix.nrow();
		const Complex* pin = input_complex_matrix.getStorage(deleteIn);
		for (uInt i = 0; i < num_data; ++i) {
			output_data[i] = casa::real(pin[max_pol * i + num_pol]);
		}
		input_complex_matrix.freeStorage(pin,deleteIn);
	} catch (AipsError x) {
		LogIO logger(LogOrigin("SakuraArrayConverter", __func__, WHERE));
		logger << _ORIGIN;
		logger << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
		<< LogIO::POST;
		RETHROW(x);
	}
}

void SakuraArrayConverter::ConvertFloatMatrixToSakura(uInt const num_pol,
		uInt const num_data, Matrix<Float> &input_float_matrix,
		float output_data[]) {
	try {
		for (uInt i = 0; i < num_data; ++i) {
			output_data[i] = input_float_matrix.row(num_pol)[i];
		}
	} catch (AipsError x) {
		LogIO logger(LogOrigin("SakuraArrayConverter", __func__, WHERE));
		logger << _ORIGIN;
		logger << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
		<< LogIO::POST;
		RETHROW(x);
	}
}

void SakuraArrayConverter::ConvertSakuraToComplexMatrix(uInt const num_pol,
		uInt const num_data, float const input_data[],
		Matrix<Complex> &output_complex_matrix) {
	try {
		Bool deleteOut;
		const uInt max_pol = output_complex_matrix.nrow();
		Complex* pout = output_complex_matrix.getStorage(deleteOut);
		for (uInt i = 0; i < num_data; ++i) {
			casa::real(pout[max_pol * i + num_pol]) = input_data[i];
			casa::imag(pout[max_pol * i + num_pol]) = 0.0;
		}
		output_complex_matrix.putStorage(pout, deleteOut);
	} catch (AipsError x) {
		LogIO logger(LogOrigin("SakuraArrayConverter", __func__, WHERE));
		logger << _ORIGIN;
		logger << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
		<< LogIO::POST;
		RETHROW(x);
	}
}

void SakuraArrayConverter::ConvertSakuraToFloatMatrix(uInt const num_pol,
		uInt const num_data, float const input_data[],
		Matrix<Float> &output_float_matrix) {
	try {
		Bool deleteOut;
		const uInt max_pol = output_float_matrix.nrow();
		Float* pout = output_float_matrix.getStorage(deleteOut);
		for (uInt i = 0; i < num_data; ++i) {
			pout[max_pol * i + num_pol] = input_data[i];
		}
		output_float_matrix.putStorage(pout, deleteOut);
	} catch (AipsError x) {
		LogIO logger(LogOrigin("SakuraArrayConverter", __func__, WHERE));
		logger << _ORIGIN;
		logger << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
		<< LogIO::POST;
		RETHROW(x);
	}
}

}
 // End of casa namespace.
