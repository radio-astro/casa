//# FFTServer.cc: A class with methods for Fast Fourier Transforms
//# Copyright (C) 1994,1995,1996,1997,1998,1999,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: FFTServer.tcc 20253 2008-02-23 15:15:00Z gervandiepen $

#include <scimath/Mathematics/FFTServer.h>
#include <scimath/Mathematics/NumericTraits.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/VectorIter.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicMath/Math.h>
#include <casa/Utilities/Assert.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  
template<class T, class S> FFTServer<T,S>::
  FFTServer()
  :itsSize(0),
  itsBuffer(0),
  itsWorkIn(NULL),
  itsWorkOut(NULL),
  itsWorkC2C(NULL)
{
}
  
template<class T, class S> FFTServer<T,S>::
FFTServer(const IPosition & fftSize, 
	  const FFTEnums::TransformType transformType)
  :
    itsBuffer(0),
    itsWorkIn(NULL),
    itsWorkOut(NULL),
    itsWorkC2C(NULL)
{
  itsTransformType = transformType;
  resize(fftSize, transformType);
}

template<class T, class S> FFTServer<T,S>::
FFTServer(const FFTServer<T,S> & other)
  :
  itsBuffer(other.itsBuffer.nelements()),
  itsWorkIn(NULL),
  itsWorkOut(NULL),
  itsWorkC2C(NULL)
{
  itsTransformType = other.itsTransformType;
  resize(other.itsSize, other.itsTransformType);
}


template<class T, class S> FFTServer<T,S>::
~FFTServer() 
{
    if (itsWorkIn) delete[] itsWorkIn;
    if (itsWorkOut) delete[] itsWorkOut;
    if (itsWorkC2C) delete[] itsWorkC2C;
}

template<class T, class S> FFTServer<T,S> & FFTServer<T,S>::
operator=(const FFTServer<T,S> & other) {
  if (this != &other) {

      itsBuffer.resize(other.itsBuffer.nelements());
      itsFFTW = other.itsFFTW;
      itsTransformType = other.itsTransformType;
      resize(other.itsSize, other.itsTransformType);
  }

  return *this;
}


template<class T, class S> void FFTServer<T,S>::
resize(const IPosition & fftSize,
       const FFTEnums::TransformType transformType) 
{
     DebugAssert(fftSize.nelements() > 0, AipsError);
     DebugAssert(fftSize.product() > 0, AipsError);
     
     if (transformType != itsTransformType ||
	 itsSize.nelements() != fftSize.nelements() || 
	 fftSize != itsSize) {

       itsTransformType = transformType;
            
	itsSize.resize(fftSize.nelements(), False); //otherwise assignment won't work!
	itsSize = fftSize;
	if (itsWorkIn) {
	    delete[] itsWorkIn;
	}
	itsWorkIn = new T[itsSize.product()];
	
	if (itsWorkOut) {
	    delete[] itsWorkOut;
	}

	itsWorkOut = new S[((itsSize(0)/2+1) * itsSize.product())/itsSize(0)];

	if (itsWorkC2C) {
	    delete[] itsWorkC2C;
	}
	itsWorkC2C = new S[itsSize.product()];

	IPosition transpose(itsSize);
	transpose(0) = itsSize(itsSize.nelements()-1);
	transpose(itsSize.nelements()-1) = itsSize(0);

	if (itsTransformType == FFTEnums::REALTOCOMPLEX) {
	  itsFFTW.plan_r2c(transpose, itsWorkIn, itsWorkOut);
	}
	else if (itsTransformType == FFTEnums::COMPLEXTOREAL) {
	  itsFFTW.plan_c2r(transpose, itsWorkOut, itsWorkIn);
	}
	else if (itsTransformType == FFTEnums::COMPLEX) {
	  itsFFTW.plan_c2c_forward(transpose, itsWorkC2C);
	}
	else if (itsTransformType == FFTEnums::INVCOMPLEX) {
	  itsFFTW.plan_c2c_backward(transpose, itsWorkC2C);
	}
	else {
	  AlwaysAssert(False, AipsError);
	}
    }
}

template<class T, class S> void FFTServer<T,S>::
fft(Array<S> & cResult, Array<T> & rData, const Bool constInput) {
  if (constInput) {
    Array<T> rCopy = rData.copy();
    flip(rCopy,True,False);
    fft0(cResult, rCopy, False);
  }
  else {
    flip(rData,True,False);
    fft0(cResult, rData, False);
  }
  flip(cResult,False,True);
}

template<class T, class S> void FFTServer<T,S>::
fft(Array<S> & cResult, const Array<T> & rData) 
{
 fft(cResult, (Array<T> &) rData, True);
}

template<class T, class S> void FFTServer<T,S>::
fft(Array<T> & rResult, Array<S> & cData, const Bool constInput) {
  if (constInput) {
    Array<S> cCopy = cData.copy();
    flip(cCopy, True, True);
    fft0(rResult, cCopy, False);
  }
  else {
    flip(cData, True, True);
    fft0(rResult, cData, False);
  }
  flip(rResult, False, False);
}

template<class T, class S> void FFTServer<T,S>::
fft(Array<T> & rResult, const Array<S> & cData) {
  fft(rResult, (Array<S> &) cData, True);
}

template<class T, class S> void FFTServer<T,S>::
fft(Array<S> & cValues, const Bool toFrequency) {
  
  /*  Int doFlip;
  doFlip = !phaseRotate((Matrix<S> &)cValues, True);
  if (doFlip) flip(cValues, True, False);
  
  //if (doFlip) cerr << "...Flipping" << endl;
  
  fft0(cValues, toFrequency);
  
  if(doFlip) {
    flip(cValues, False, False);
  }
  else{
    doFlip = !phaseRotate((Matrix<S> &)cValues, False);
    //    if (doFlip)  flip(cValues, False, False);
  }
  */
  // backed out above as it is inconsitent between imager and tFFTserver
  
  flip(cValues, True, False);
  fft0(cValues, toFrequency);
  flip(cValues, False, False);
}

template<class T, class S> void FFTServer<T,S>::
fft(Array<S> & cResult, const Array<S> & cData, const Bool toFrequency) 
{
  if (cResult.nelements() != 0) {
    AlwaysAssert(cResult.conform(cData), AipsError);
  }
  else
    cResult.resize(cData.shape());
  cResult = cData;
  fft(cResult, toFrequency);
}

template<class T, class S> void FFTServer<T,S>::
fft0(Array<S> & cResult, Array<T> & rData, const Bool constInput) {

  // The constInput argument is not used as the input Array is never changed by
  // this function. But is put into the interface in case this function changes
  // in the future and to maintain a consistant interface with the other fft
  // functions in this class. To suppress the compiler warning I'll use it
  // here.
  if (constInput) {
  }

  const IPosition shape = rData.shape();
  // Ensure the output Array is the required size
  IPosition resultShape = shape;
  resultShape(0) = (shape(0)+2)/2;

  if (cResult.nelements() != 0) {
    AlwaysAssert(resultShape.isEqual(cResult.shape()), AipsError);
  }
  else {
    cResult.resize(resultShape);
  }
  // Early exit if the Array is all zero;
  if (allNearAbs(rData, T(0), NumericTraits<T>::minimum)) {
    cResult = S(0);
    return;
  }
  // Initialise the work arrays
  if (!shape.isEqual(itsSize) || itsTransformType != FFTEnums::REALTOCOMPLEX)
    resize(shape, FFTEnums::REALTOCOMPLEX);

  // get a pointer to the array holding the result
  Bool resultIsAcopy;
  Bool dataIsAcopy;

  IPosition fftwShape(resultShape);
  fftwShape(0) = resultShape(resultShape.nelements() - 1);
  fftwShape(resultShape.nelements() - 1) = resultShape(0);

  Array<S> transposedResult(fftwShape);

  const T * dataPtr = rData.getStorage(dataIsAcopy);

  objcopy(itsWorkIn, dataPtr, itsSize.product());

  itsFFTW.r2c(itsSize, itsWorkIn, itsWorkOut);

  S *cPtr = cResult.getStorage(resultIsAcopy);

  objcopy(cPtr, itsWorkOut, 
	  ((itsSize(0)/2+1) * itsSize.product())/itsSize(0));
  cResult.putStorage(cPtr, resultIsAcopy);

  return;

}
  
template<class T, class S> void FFTServer<T,S>::
fft0(Array<S> & cResult, const Array<T> & rData)
{
  fft0(cResult, (Array<T> &) rData, True);
}

template<class T, class S> void FFTServer<T,S>::
fft0(Array<T> & rResult, Array<S> & cData, const Bool constInput) 
{
  Array<S> cCopy;
  if (constInput)
    cCopy = cData;
  else
    cCopy.reference(cData);

  const IPosition cShape = cCopy.shape();
  const IPosition rShape = determineShape(rResult.shape(), cCopy);
  rResult.resize(rShape);

  // Early exit if the Array is all zero;
  if (allNearAbs(cData, S(0), NumericTraits<S>::minimum)) {
    rResult = T(0);
    return;
  }
  // resize the server if necessary
  if (!rShape.isEqual(itsSize) || itsTransformType != FFTEnums::COMPLEXTOREAL)
    resize(rShape, FFTEnums::COMPLEXTOREAL);

  Bool dataIsAcopy;
  
  const S * complexPtr = cData.getStorage(dataIsAcopy);

  objcopy(itsWorkOut, complexPtr, 
	  ((itsSize(0)/2+1) * itsSize.product())/itsSize(0));
  
  itsFFTW.c2r(itsSize, itsWorkOut, itsWorkIn);

  for (int i = 0; i < itsSize.product(); i++) {
      itsWorkIn[i] /= 1.0*itsSize.product();
  }

  Bool resultIsAcopy;
  T *rPtr = rResult.getStorage(resultIsAcopy);

  objcopy(rPtr, itsWorkIn, itsSize.product());
  rResult.putStorage(rPtr, resultIsAcopy);

  return;
}

template<class T, class S> void FFTServer<T,S>::
fft0(Array<T> & rResult, const Array<S> & cData) 
{
  fft0(rResult, (Array<S> &) cData, True);
}


template<class T, class S> void FFTServer<T,S>::
fft0(Array<S> & cValues, const Bool toFrequency) 
{
  // Early exit if the Array is all zero;
  if (allNearAbs(cValues, S(0), NumericTraits<S>::minimum)){
    return;
  }
  // resize the server if necessary
  const IPosition shape = cValues.shape();

  if (toFrequency) {
    if (!shape.isEqual(itsSize) || itsTransformType != FFTEnums::COMPLEX) {
      resize(shape, FFTEnums::COMPLEX);
    }
  }
  else {
    if (!shape.isEqual(itsSize) || itsTransformType != FFTEnums::INVCOMPLEX) {
      resize(shape, FFTEnums::INVCOMPLEX);
    }
  }

  Bool valuesIsAcopy;
  
  S * complexPtr = cValues.getStorage(valuesIsAcopy);

  objcopy(itsWorkC2C, complexPtr, itsSize.product());

  itsFFTW.c2c(itsSize, itsWorkC2C, toFrequency);

  if (!toFrequency)
      for (int i = 0; i < itsSize.product(); i++) {
	  itsWorkC2C[i] /= 1.0*itsSize.product();
      }

  objcopy(complexPtr, itsWorkC2C, itsSize.product());
  cValues.putStorage(complexPtr, valuesIsAcopy);

  return;

}


template<class T, class S> void FFTServer<T,S>::
fft0(Array<S> & cResult, const Array<S> & cData, const Bool toFrequency) 
{
  if (cResult.nelements() != 0) {
    AlwaysAssert(cResult.conform(cData), AipsError);
  }
  else
    cResult.resize(cData.shape());
  cResult = cData;
  fft0(cResult, toFrequency);
}


template<class T, class S> IPosition FFTServer<T,S>::
determineShape(const IPosition & rShape, const Array<S> & cData)
{
  const IPosition cShape=cData.shape();
  const uInt cDim = cShape.nelements();
  DebugAssert(cDim > 0, AipsError);
  // If rShape is non-zero then it must match one of the two possible shapes
  if (rShape.product() != 0) {
    DebugAssert(cDim == rShape.nelements(), AipsError);
    IPosition reqShape(cShape);
    reqShape(0) = 2*cShape(0)-2;
    if (reqShape.isEqual(rShape))
      return reqShape;
    reqShape(0) += 1;
    if (reqShape.isEqual(rShape))
      return reqShape;
    throw(AipsError("FFTServer<T,S>::determineShape() -"
		    " output array has the wrong shape"));
  }
  // Scan the imaginary components of the last samples on the first axis in
  // the cData to see if there are any non-zero terms. If so the output array
  // must be odd length in its first axis.
  {
    VectorIterator<S> iter((Array<S> &) cData);
    uInt lastElem = cShape(0)-1;
    while (!iter.pastEnd()) {
      if (!near(iter.vector()(lastElem).imag(), (T)0.0)) {
	IPosition oddLength(cShape);
	oddLength(0) = cShape(0)*2-1;
	return oddLength;
      }
      iter.next();
    }
  }
  // See if the FFTServer size can be used to guess the output Array size;
  if (itsSize.nelements() == cDim) {
    Bool match = True;
    for (uInt i = 1; i < cDim; i++)
      if (itsSize(i) != cShape(i))
	match = False;
    if (match == True && 
	((itsSize(0) == 2*cShape(0) - 2) || (itsSize(0) == 2*cShape(0) - 1)))
      return itsSize;
  }
  IPosition defShape(cShape);
  defShape(0) = 2*cShape(0) - 2;

  return defShape;
}

template<class T, class S> void FFTServer<T,S>::
flip(Array<S> & cData, const Bool toZero, const Bool isHermitian) 
{
  const IPosition shape = cData.shape();
  const uInt ndim = shape.nelements();
  const uInt nElements = shape.product();
  if (nElements == 1) {
    return;
  }
  AlwaysAssert(nElements != 0, AipsError);
  {
    Int buffLen = itsBuffer.nelements();
    for (uInt i = 0; i < ndim; i++)
      buffLen = max(buffLen, shape(i));
    itsBuffer.resize(buffLen, False, False);
  }
  Bool dataIsAcopy;
  S * dataPtr = cData.getStorage(dataIsAcopy);
  S * buffPtr = itsBuffer.storage();
  S * rowPtr = 0;
  S * rowPtr2 = 0;
  S * rowPtr2o = 0;
  uInt rowLen, rowLen2, rowLen2o;
  uInt nFlips;
  uInt stride = 1;
  uInt r;
  uInt n=0;
  if (isHermitian) {
    n = 1;
    stride = shape(0);
  }
  for (; n < ndim; n++) {
    rowLen = shape(n);
    if (rowLen > 1) {
      rowLen2 = rowLen/2;
      rowLen2o = (rowLen+1)/2;
      nFlips = nElements/rowLen;
      rowPtr = dataPtr;
      r = 0;
      while (r < nFlips) {
	rowPtr2 = rowPtr + stride * rowLen2;
	rowPtr2o = rowPtr + stride * rowLen2o;
	if (toZero) {
	  objcopy(buffPtr, rowPtr2, rowLen2o, 1u, stride);
	  objcopy(rowPtr2o, rowPtr, rowLen2, stride, stride);
	  objcopy(rowPtr, buffPtr, rowLen2o, stride, 1u);
	}
	else {
	  objcopy(buffPtr, rowPtr, rowLen2o, 1u, stride);
	  objcopy(rowPtr, rowPtr2o, rowLen2, stride, stride);
	  objcopy(rowPtr2, buffPtr, rowLen2o, stride, 1u);
	}
	r++;
	rowPtr++;
	if (r%stride == 0)
	  rowPtr += stride*(rowLen-1);
      }
      stride *= rowLen;
    }
  }
  cData.putStorage(dataPtr, dataIsAcopy);
}

template<class T, class S> void FFTServer<T,S>::
flip(Array<T> & rData, const Bool toZero, const Bool isHermitian) 
{
  const IPosition shape = rData.shape();
  const uInt ndim = shape.nelements();
  const uInt nElements = shape.product();
  if (nElements == 1) {
    return;
  }
  AlwaysAssert(nElements != 0, AipsError);
  {
    Int buffLen = itsBuffer.nelements();
    for (uInt i = 0; i < ndim; i++) {
      buffLen = max(buffLen, (shape(i)+1)/2);
    }
    itsBuffer.resize(buffLen, False, False);
  }
  Bool dataIsAcopy;
  T * dataPtr = rData.getStorage(dataIsAcopy);
  T * buffPtr = (T *) itsBuffer.storage();
  T * rowPtr = 0;
  T * rowPtr2 = 0;
  T * rowPtr2o = 0;
  uInt rowLen, rowLen2, rowLen2o;
  uInt nFlips;
  uInt stride = 1;
  uInt r;
  uInt n=0;
  if (isHermitian) {
    n = 1;
    stride = shape(0);
  }
  for (; n < ndim; n++) {
    rowLen = shape(n);
    if (rowLen > 1) {
      rowLen2 = rowLen/2;
      rowLen2o = (rowLen+1)/2;
      nFlips = nElements/rowLen;
      rowPtr = dataPtr;
      r = 0;
      while (r < nFlips) {
	rowPtr2 = rowPtr + stride * rowLen2;
	rowPtr2o = rowPtr + stride * rowLen2o;
	if (toZero) {
	  objcopy(buffPtr, rowPtr2, rowLen2o, 1u, stride);
	  objcopy(rowPtr2o, rowPtr, rowLen2, stride, stride);
	  objcopy(rowPtr, buffPtr, rowLen2o, stride, 1u);
	}
	else {
	  objcopy(buffPtr, rowPtr, rowLen2o, 1u, stride);
	  objcopy(rowPtr, rowPtr2o, rowLen2, stride, stride);
	  objcopy(rowPtr2, buffPtr, rowLen2o, stride, 1u);
	}
	r++;
	rowPtr++;
	if (r%stride == 0)
	  rowPtr += stride*(rowLen-1);
      }
      stride *= rowLen;
    }
  }
  rData.putStorage(dataPtr, dataIsAcopy);
}

} //# NAMESPACE CASA - END
