#ifndef _CASA_SAKURA_ARRAY_CONVERTER_H_
#define _CASA_SAKURA_ARRAY_CONVERTER_H_

#include <iostream>
#include <string>
//#include <libsakura/sakura.h>
#include <casa/aipstype.h>
#include <tr1/memory>
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
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>

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
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN

class SakuraArrayConverter {
protected:

public:
	explicit SakuraArrayConverter();
	virtual ~SakuraArrayConverter();
	virtual void ConvertComplexMatrixToSakura(Int const num_pol,
			Int const num_data, Matrix<Complex> &spectrum_matrix,
			float output_data[]);
	virtual void ConvertFloatMatrixToSakura(Int const num_pol,
			Int const num_data, Matrix<Float> &spectrum_matrix,
			float output_data[]);
private:
	SakuraArrayConverter(const SakuraArrayConverter&);
	SakuraArrayConverter& operator=(const SakuraArrayConverter&);
	LogIO logger_;
};

} //# NAMESPACE CASA - END

#endif /* _CASA_SINGLEDISH_DUMMY_H_ */
