#include <iostream>

//#include <libsakura/sakura.h>
//#include <libsakura/config.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Utilities/Assert.h>
#include <casa/Arrays/ArrayMath.h>

#include <ms/MSSel/MSSelectionTools.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisSetUtil.h>

#include <casa_sakura/SakuraUtils.h>
#include <singledish/SingleDish/SDMSManager.h>


#define _ORIGIN LogOrigin("SDMSManager", __func__, WHERE)

namespace casa {

SDMSManager::SDMSManager()
{
}

// SDMSManager &SDMSManager::operator=(SDMSManager const &other)
// {
//   return *this;
// }

SDMSManager::~SDMSManager()
{
  LogIO os(_ORIGIN);
}

// -----------------------------------------------------------------------
// Fill output MS with data from an input VisBuffer
// -----------------------------------------------------------------------
void SDMSManager::fillCubeToOutputMs(vi::VisBuffer2 *vb,Cube<Float> const &data_cube, Cube<Bool> const &flag_cube)
{
	setupBufferTransformations(vb);

	// make sure the shape of cube matches the number of rows to add.
	AlwaysAssert(data_cube.nplane()==nRowsToAdd_p, AipsError);

	if (not bufferMode_p)
	{
		// Create RowRef object to fill new rows
		uInt currentRows = outputMs_p->nrow();
		RefRows rowRef( currentRows, currentRows + nRowsToAdd_p/nspws_p - 1);

		// Add new rows to output MS
		outputMs_p->addRow(nRowsToAdd_p,False);

		// Fill new rows
		weightSpectrumFlatFilled_p = False;
		weightSpectrumFromSigmaFilled_p = False;
		fillWeightCols(vb,rowRef);
		fillCubeToDataCols(vb,rowRef,data_cube, flag_cube);
		fillIdCols(vb,rowRef);
	}

    return;
}

void SDMSManager::fillCubeToOutputMs(vi::VisBuffer2 *vb,Cube<Float> const &data_cube)
{
  Cube<Bool> const *flag_cube=NULL;
  fillCubeToOutputMs(vb, data_cube, *flag_cube);
}

// ----------------------------------------------------------------------------------------
// Fill main (data) columns which have to be combined together to produce bigger SPWs
// ----------------------------------------------------------------------------------------
void SDMSManager::fillCubeToDataCols(vi::VisBuffer2 *vb,RefRows &rowRef,Cube<Float> const &data_cube, Cube<Bool> const &flag_cube)
{
	ArrayColumn<Bool> *outputFlagCol=NULL;
	for (dataColMap::iterator iter = dataColMap_p.begin();iter != dataColMap_p.end();iter++)
	{
		// Get applicable *_SPECTRUM (copy constructor uses reference semantics)
		// If channel average or combine, otherwise no need to copy
		const Cube<Float> applicableSpectrum = getApplicableSpectrum(vb,iter->first);

		// Apply transformations
		switch (iter->first)
		{
			case MS::DATA:
			{
				if (mainColumn_p == MS::DATA)
				{
					outputFlagCol = &(outputMsCols_p->flag());
				}
				else
				{
					outputFlagCol = NULL;
				}
				Cube<Complex> cdata_cube(data_cube.shape());
				convertArray(cdata_cube,data_cube);
				transformCubeOfData(vb,rowRef,cdata_cube,outputMsCols_p->data(), outputFlagCol,applicableSpectrum);

				break;
			}
			case MS::CORRECTED_DATA:
			{
				if (mainColumn_p == MS::CORRECTED_DATA)
				{
					outputFlagCol = &(outputMsCols_p->flag());
				}
				else
				{
					outputFlagCol = NULL;
				}

				Cube<Complex> cdata_cube(data_cube.shape());
				convertArray(cdata_cube,data_cube);
				if (iter->second == MS::DATA)
				{
					transformCubeOfData(vb,rowRef,cdata_cube,outputMsCols_p->data(), outputFlagCol,applicableSpectrum);
				}
				else
				{
					transformCubeOfData(vb,rowRef,cdata_cube,outputMsCols_p->correctedData(), outputFlagCol,applicableSpectrum);
				}

				break;
			}
			case MS::MODEL_DATA:
			{
				if (mainColumn_p == MS::MODEL_DATA)
				{
					outputFlagCol = &(outputMsCols_p->flag());
				}
				else
				{
					outputFlagCol = NULL;
				}

				if (iter->second == MS::DATA)
				{
					transformCubeOfData(vb,rowRef,vb->visCubeModel(),outputMsCols_p->data(), outputFlagCol,applicableSpectrum);
				}
				else
				{
					transformCubeOfData(vb,rowRef,vb->visCubeModel(),outputMsCols_p->modelData(), outputFlagCol,applicableSpectrum);
				}
				break;
			}
			case MS::FLOAT_DATA:
			{
				if (mainColumn_p == MS::FLOAT_DATA)
				{
					outputFlagCol = &(outputMsCols_p->flag());
				}
				else
				{
					outputFlagCol = NULL;
				}

				transformCubeOfData(vb,rowRef,data_cube,outputMsCols_p->floatData(), outputFlagCol,applicableSpectrum);

				break;
			}
			case MS::LAG_DATA:
			{
				// jagonzal: TODO
				break;
			}
			default:
			{
				// jagonzal: TODO
				break;
			}
		}
		//KS: THIS PART ASSUMES INROW==OUTROW
		if (outputFlagCol != NULL && &flag_cube != NULL)
		  {
		    writeCube(flag_cube,*outputFlagCol,rowRef);
		  }
	}

    // Special case for flag category
    if (inputFlagCategoryAvailable_p)
    {
    	if (spectrumReshape_p)
    	{
    		IPosition transformedCubeShape = getShape(); //[nC,nF,nR]
    		IPosition inputFlagCategoryShape = vb->flagCategory().shape(); // [nC,nF,nCategories,nR]
    		IPosition flagCategoryShape(4,	inputFlagCategoryShape(1),
    										transformedCubeShape(2),
    										inputFlagCategoryShape(2),
    										transformedCubeShape(2));
    		Array<Bool> flagCategory(flagCategoryShape,False);

        	outputMsCols_p->flagCategory().putColumnCells(rowRef, flagCategory);
    	}
    	else
    	{
        	outputMsCols_p->flagCategory().putColumnCells(rowRef, vb->flagCategory());
    	}
    }

	return;
}

// -----------------------------------------------------------------------
// Method to determine sort columns order
// -----------------------------------------------------------------------
void SDMSManager::setSortColumns(Block<Int> sortColumns,
				 bool addDefaultSortCols)
{
  size_t const num_in = sortColumns.nelements();
  LogIO os(_ORIGIN);
  os << "Setting user sort columns with " << num_in << " elements" << LogIO::POST;
  if (addDefaultSortCols) {
    Block<Int> defaultCols(6);
    defaultCols[0] = MS::OBSERVATION_ID;
    defaultCols[1] = MS::ARRAY_ID;
    // Assuming auto-correlation here
    defaultCols[2] = MS::ANTENNA1;
    defaultCols[3] = MS::FEED1;
    defaultCols[4] = MS::DATA_DESC_ID;
    defaultCols[5] = MS::TIME;
    os << "Adding default sort columns to user sort column" << LogIO::POST;
    if (num_in > 0){
      size_t num_elem(num_in);
      sortColumns.resize(num_in+defaultCols.nelements(),false,true);
      for (size_t i=0; i < defaultCols.nelements(); ++i){
	bool addcol = true;
	if (getBlockId(sortColumns,defaultCols[i]) > -1)
	  addcol = false; // the columns is in sortColumns
	if (addcol) {
	  sortColumns[num_elem] = defaultCols[i];
	  ++num_elem;
	}
      }
      // shrink block
      sortColumns.resize(num_elem, True, True);
      userSortCols_ = sortColumns;
    } else {
      userSortCols_ = defaultCols;
    }
  } else { // do not add
    if (num_in > 0)
      userSortCols_ = sortColumns;
    else
      userSortCols_ = Block<Int>();
  }
  os << "Defined user sort columns with " << userSortCols_.nelements() << " elements" << LogIO::POST;
  // regenerate iterator if necessary
  if (visibilityIterator_p != NULL) {
    os << "Regenerating iterator" << LogIO::POST;
    setIterationApproach();
    generateIterator();
  }
  return;
}

int SDMSManager::getBlockId(Block<Int> const &data, Int const value){
  for (size_t i=0; i < data.nelements(); ++i){
    if (data[i] == value) return (int) i;
  }
  return -1;
}

void SDMSManager::setIterationApproach()
{
  if (userSortCols_.nelements()==0){
    sortColumns_p = Block<Int>();
    return;
  }
  // User column is set.
  uInt nSortColumns = userSortCols_.nelements();
  Block<Int> removeCols(3);
  uInt nRemoveCols = 0 ;
  logger_p.origin(_ORIGIN);
  if (timespan_p.contains("scan") && (getBlockId(userSortCols_, MS::SCAN_NUMBER)>-1)) {
    logger_p << LogIO::WARN << "Combining data through scans for time average."
	     << "Removing SCAN_NUMBER from user sort list." << LogIO::POST;    
    removeCols[nRemoveCols] = MS::SCAN_NUMBER;
    nRemoveCols += 1;
  }
  if (timespan_p.contains("state") && (getBlockId(userSortCols_, MS::STATE_ID)>-1)) {
    logger_p << LogIO::WARN << "Combining data through scans for time average."
	     <<  "Removing STATE_ID form user sort list." << LogIO::POST;
    removeCols[nRemoveCols] = MS::STATE_ID;
    nRemoveCols += 1;
  }
  if (combinespws_p && (getBlockId(userSortCols_, MS::DATA_DESC_ID)>-1) ) {
    logger_p << LogIO::WARN << "Combining data from selected spectral windows."
	     << "Removing DATA_DESC_ID from user sort list" << LogIO::POST;
    removeCols[nRemoveCols] = MS::DATA_DESC_ID;
    nRemoveCols += 1;
  }
  nSortColumns -= nRemoveCols;
  sortColumns_p = Block<Int>(nSortColumns);
  uInt sortColumnIndex = 0;

  for (size_t i=0; i < userSortCols_.nelements(); ++i){
    bool addcol = true;
    for (size_t j = 0 ; j < nRemoveCols; ++j) {
	if (getBlockId(userSortCols_,removeCols[i]) > -1)
	  addcol = false; // the columns is in removeColumns
    }
    if (addcol) {
      sortColumns_p[sortColumnIndex] = userSortCols_[i];
      ++sortColumnIndex;
    }
  }
  ostringstream oss;
  for (size_t i=0;i<sortColumns_p.nelements(); ++i)
    oss << sortColumns_p[i] << ", ";
  logger_p << LogIO::DEBUG1 << "Final Sort order: "
	   << oss.str() << LogIO::POST;

  return;
}

Record SDMSManager::getSelRec(string const &spw)
{
  MeasurementSet myms = MeasurementSet(inpMsName_p);
  return myms.msseltoindex(toCasaString(spw));
}

}  // End of casa namespace.
