
#include <graphics/X11/X_enter.h>
#include <QtGui>
#include <QtOpenGL>
#include <math.h>
#include <QGLWidget>
#include <QFile>
#include <graphics/X11/X_exit.h>

#include <display/DisplayDatas/DisplayData.h>
#include <display/DisplayDatas/MSAsRaster.h>
#include <images/Images/ImageInterface.h>
#include <display/DisplayDatas/LatticeAsRaster.h>
#include <display/DisplayDatas/LatticeAsContour.h>
#include <display/DisplayDatas/LatticeAsVector.h>
#include <display/DisplayDatas/LatticeAsMarker.h>
#include <display/DisplayDatas/SkyCatOverlayDD.h>
#include <casa/OS/Path.h>
#include <images/Images/PagedImage.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/ImageInfo.h>
#include <images/Images/ImageRegrid.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayEvents/WCMotionEvent.h>
#include <casa/iostream.h>
#include <casa/iomanip.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <images/Images/SubImage.h>
#include <images/Images/ImageStatistics.h>
#include <images/Regions/WCBox.h>
#include <images/Regions/WCPolygon.h>
#include <images/Regions/WCIntersection.h>
#include <images/Regions/ImageRegion.h>
#include <casa/OS/File.h>
#include <display/Display/Colormap.h>
#include <display/Display/ColormapDefinition.h>
#include <casa/System/Aipsrc.h>
#include <display/DisplayDatas/WedgeDD.h>
#include <casa/Exceptions/Error.h>
#include <images/Images/ImageExpr.h>
#include <images/Images/ImageExprParse.h>
#include <scimath/Mathematics/Interpolate2D.h>
#include <lattices/LatticeMath/LattStatsSpecialize.h>

#include <display3d/3DViewer/3DController.qo.h>
#include <display3d/3DViewer/3DDisplayData.qo.h>


namespace casa {

DisplayData3::DisplayData3(const String& fn) :
im_(0), cim_(0) 
{
   fileCube.resize(0, 0, 0);
   switch(ImageOpener::imageType(fn)) {

      case ImageOpener::AIPSPP: {

         if(imagePixelType(fn) == TpFloat) {
           im_ = new PagedImage<Float>
                     (fn, TableLock::AutoNoReadLocking);
         }
         else if(imagePixelType(fn) == TpComplex) {
           cim_ = new PagedImage<Complex>
                     (fn, TableLock::AutoNoReadLocking);
         }
         else  {
           throw AipsError("Only Float and Complex CASA "
                 "images are supported at present.");
         }
         break;  
      }

      case ImageOpener::FITS: {
         im_ = new FITSImage(fn);
         break;  
      }

      case ImageOpener::MIRIAD: {
         im_ = new MIRIADImage(fn);
         break;  
      }

      default: {

         File f(fn);
         if (!f.exists()) {
            throw AipsError("Image file not found.");
         }

         QString path(fn.chars());
         QFile file(path);

         Int dim = 0;
         if (file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);
            Vector<Float> x(2); 
            Vector<Float> y(2);
            Vector<Float> z(2);
            Float min = 0.0;
            x = 0;
            y = 0;
            z = 0;
            while (!in.atEnd()) {
               QString line = in.readLine();
               if (!line.isNull() && !line.isEmpty()) {
                  QStringList coords =
                     line.trimmed().split(QRegExp("\\s+"));
                  if (coords.size() == 1) {
                     Float a = coords[0].toFloat();
                     z(0) = fmin(z(0), a); 
                     z(1) = fmax(z(1), a); 
                  }
                  if (coords.size() == 3) {
                     Float b = coords[0].toFloat();
                     Float c = coords[1].toFloat();
                     x(0) = fmin(x(0), b); 
                     x(1) = fmax(x(1), b); 
                     y(0) = fmin(y(0), c); 
                     y(1) = fmax(y(1), c); 
                     Float d = coords[2].toFloat();
                     min = fmin(min, d);
                  }
               }
            }
            //cout << "x=" << x << " y=" << y << " z=" << z << endl;
            IPosition pos(3, (Int)(x(1) - x(0) + 1), (Int)(y(1) - y(0) + 1),
                          (Int)(z(1) - z(0) + 1));
            fileCube.resize(pos);
            fileCube = min;
            //cout << "pos=" << pos << endl;
            dim = pos(0) * pos(1) * pos(2);

            file.reset();
            while (!in.atEnd()) {
               QString line = in.readLine();
               if (!line.isNull() && !line.isEmpty()) {
                  QStringList coords =
                     line.trimmed().split(QRegExp("\\s+"));
                  int current = 0;
                  if (coords.size() == 1) {
                     Float a = coords[0].toFloat();
                     current = (int) a; 
                  }
                  if (coords.size() == 3) {
                     Float b = coords[0].toFloat();
                     Float c = coords[1].toFloat();
                     Float d = coords[2].toFloat();
                     fileCube((int)(b - x(0)), (int)(c - y(0)), 
                              (int)(current - z(0))) = d; 
                     //cout << b << " " << c << " " << d << endl;
                  }
               }
            }
 
         }
         
         if (!dim)
         throw 
            AipsError("The image format must be of casa, FITS or MIRIAD.");
      }
   }

   IPosition pos = fileCube.shape();
   //cout << "pos=" << pos;
   Int dim = pos(0) * pos(1) * pos(2);
   if (im_ == 0 && cim_ == 0 && dim == 0) {
      throw AipsError("Couldn't create image.");  
   }
}


DisplayData3::~DisplayData3()
{
   //should not need to care about this because when it 
   //reach to this point, the program has completed
   if (im_) {
      delete im_;
      im_ = 0;
   }
   if (cim_) {
      delete cim_;
      cim_ = 0;
   }
}

void DisplayData3::getCube(Cube<Float>& data, int& pol)
{
   //if read text file, the fileCube size will not be 0
   //simply give that cube out
   IPosition pos = fileCube.shape();
   //cout << "pos=" << pos;
   Int len = pos(0) * pos(1) * pos(2);
   if (len) {
      data = fileCube;
      return;
   }

   //the input is not a text file
   //is it a valid image?
   if (cim_ && !im_) {
      cout << "complex image cube is not supported" << endl;
      return;
   }
   if (!im_) {
      cout << "could not open the image" << endl;
      return;
   }

   //cout << "  name: "<< im_->name() << endl;
   //cout << "  type: "<< im_->imageType() << endl;
   //cout << " paged: "<< im_->isPaged() << endl;
   //cout << " shape: "<< im_->shape() << endl;
   //cout << "    ok: "<< im_->ok()<< endl;
   //cout << " nAxis: "<< im_->coordinates().nWorldAxes() << endl;
   //cout << " coord: "<< im_->coordinates().type() << endl;
   //for (uInt i = 0; i < im_->coordinates().nCoordinates(); i++) {
   //   cout << " coord(" << i << ")=" 
   //        << im_->coordinates().showType(i) << endl;
   //}

   Float dataMin;
   Float dataMax;
   Vector<Float> range(2);
   LattStatsSpecialize::minMax(dataMin, dataMax, im_, range, True, True);
   //cout << "dataMin=" << dataMin << " dataMax=" << dataMax
   //     << " range=" << range << endl;

   getAxes();
   //cout << "axis=" << axes << endl;
   //cout << "type=" << tAxes << endl;
   //cout << "  ip=" << ip << endl;

   Int size = axes.nelements();

   if (size < 2) {
      cout << "No pixels in the image" << endl;
      return;
   }

   if (axes(0) < 0 || axes(1) < 0) {
      cout << "No pixels in the image" << endl;
      return;
   }

   Int xx = ip(axes(0));
   Int yy = ip(axes(1));

   if (xx < 1 || yy < 1) {
      cout << "No pixels in the image" << endl;
      return;
   }

   Int zz = 1;
   if (size > 2 && axes(2) > 0) {
     zz = ip(axes(2));
   }

   Int scale = (Int)(300 / sqrt(xx * xx + yy * yy)) + 1 ;
   //cout << " scale=" << scale << endl;

   Int outNX = scale * xx;
   Int outNY = scale * yy;
   data.resize(outNX, outNY, zz);

   Matrix<Float> out(outNX, outNY); 
   Matrix<Float> in(xx, yy); 
   Interpolate2D interp(Interpolate2D::LINEAR);
   //Interpolate2D interp(Interpolate2D::CUBIC);
   Double incrX = (xx - 1.0) / (outNX - 1.0);
   Double incrY = (yy - 1.0) / (outNY - 1.0);

   for (Int chn = 0; chn < zz; chn++) {
     IPosition start(size);
     IPosition end(size);
     IPosition stride(size);
     getStartEnd(start, end, stride, chn, pol);
     //cout << "start:" << start << " end:" << end << endl;
     if (scale < 2) {
       data.xyPlane(chn) = 
             im_->getSlice(start, end, stride, True); 
     }
     else {
       out = 0; 
       in = im_->getSlice(start, end, stride, True); 

       Vector<Double> inloc(2);
       /*
       for (Int i = 0, inloc[0] = 0; i < outNX; i++, inloc[0] += incrX) {
          for (Int j = 0, inloc[1] = 0; j < outNY; j++, inloc[1] += incrY) {
             interp.interp(out(i, j), inloc, in);
             //cout << "i=" << i << " j=" << j
             //     << " inloc[0]=" << inloc[0] 
             //     << " inloc[1]=" << inloc[1] << endl;
          }
       }
       */
       for (Int i = 0; i < outNX; i++) {
          for (Int j = 0; j < outNY; j++) {
             inloc[0] = i * incrX;
             inloc[1] = j * incrY;
             interp.interp(out(i, j), inloc, in);
             //cout << "i=" << i << " j=" << j
             //     << " inloc[0]=" << inloc[0] 
             //     << " inloc[1]=" << inloc[1] << endl;
          }
       }
       data.xyPlane(chn) = out; 
     }
   }
   //cout << " done pack" << endl;

}

void DisplayData3::getDefaultCube(Cube<Float>& data)
{
   const Int xlen = 50;
   const Int ylen = 40;
   const Int zlen = 30;
   data.resize(xlen, ylen, zlen);
   for (Int i = 0; i < xlen; i++){
     for (Int j = 0; j < ylen; j++){
       for (Int k = 0; k < zlen; k++){
         data(i, j, k) = i * 10000 + j * 100 + k;
       }
     }
   }
}


void DisplayData3::getStartEnd(IPosition& start, IPosition& end,
                               IPosition& stride, 
                               Int spec, Int pol) {
   Int size = tAxes.nelements(); 
   for (Int k = 0; k < size; k++) {
      if (tAxes(k) == "Spectral") {
         Int sp = (spec < ip(k)) ? spec : ip(k) - 1;
         sp = (sp > -1) ? sp : 0;
         start(k) = sp;
         end(k) = 1; 
         stride(k) = 1;
      }
      if (tAxes(k) == "Stokes") {
         Int st = (pol < ip(k)) ? pol : ip(k) - 1;
         st = (st > -1) ? st : 0;
         start(k) = st;
         end(k) = 1; 
         stride(k) = 1;
      }
      if (tAxes(k) == "Direction") {
         start(k) = 0;
         end(k) = ip(k); 
         stride(k) = 1;
      }
   }
}

void DisplayData3::getAxes() { 
   ip = im_->shape();
   uInt size = ip.nelements();
   //cout << "ip.nelements=" << size << endl;
   //cout << "ip=" << ip << endl;
   if (size < 2) {
      cout << "Invalid image file" << endl;
      return;
   }
   size = min(size, im_->coordinates().nWorldAxes());
   axes.resize(size);
   axes = -1;
   tAxes.resize(size);
   tAxes = "";
   for (uInt i = 0; i < size; i++) {
      Int coordno, axisincoord;
      im_->coordinates().findWorldAxis(coordno, axisincoord, i);
      String cType = im_->coordinates().showType(coordno);
      tAxes(i) = cType;
      if (cType == "Spectral") {
         axes(2) = i;
      } 
      if (cType == "Stokes") {
         axes(3) = i;
      }
      if (cType == "Direction") {
         axes(axisincoord) = i;
      }
   }
}


}
//       
//      //------------------------------------------------------
//      //IPosition axes = IPosition::makeAxisPath(im_->ndim());
//      IPosition axes(2);
//      axes(0) = 0;
//      axes(1) = 1;
//      //cout << "axes=" << axes << endl;
//      
//      CoordinateSystem cSysOut = im_->coordinates();
//      
//      Vector<Double> incr = cSysOut.increment().copy();
//      Vector<Double> refp  = cSysOut.referencePixel().copy();
//      Vector<Double> refv  = cSysOut.referenceValue().copy();
//
//      for (uInt i = 0; i < axes.nelements(); i++) {
//         uInt j = axes(i);
//         shapeOut(j) = scale * ip(j);
//         incr(j) = incr(j) / scale;
//         refp(j) = shapeOut(j) / 2.0;  
//      }
//
//      cSysOut.setReferencePixel(refp);
//      cSysOut.setIncrement(incr);
//      
//      ImageRegrid<Float> regridder;
//      pOut = new TempImage<Float>(shapeOut, cSysOut, -1);
//      Interpolate2D::Method emethod = 
//           Interpolate2D::stringToMethod("Linear");
//           //Interpolate2D::stringToMethod("cubic");
//      //regridder.showDebugInfo(3);
//      regridder.regrid(*pOut, emethod, axes, *im_, False, 0, True, True);
//      //------------------------------------------------------
//   }
//   */
//   //cout << "shapeOut=" << shapeOut << endl;
//
//   /*
//   if (size == 4) {
//     int inPol = pol;
//     pol = shapeOut(axes(3));
//     if (pol <= inPol) {
//       inPol = 0; 
//     }
//
//     data.resize(shapeOut(axes(0)), shapeOut(axes(1)), shapeOut(axes(2)));
//
//     for (Int k = 0; k < shapeOut(2); k++) {
//        for (Int j = 0; j < shapeOut(1); j++) {
//           for (Int i = 0; i < shapeOut(0); i++) {
//              data(i, j, k) = pOut->getAt(IPosition(4, i, j, k, inPol));
//           }
//        }
//     }
//   }
//   if (size == 3) {
//     
//     pol = 0;
//     data.resize(axes(0), axes(1), axes(2));
//     //cout << "data.shape()=" << data.shape() << endl;
//
//     Float min = 100000;
//     if (size == 4) {
//       for (Int l = 0; l < ip(3); l++) {
//         for (Int k = 0; k < axes(2); k++) {
//           for (Int j = 0; j < axes(1); j++) {
//             for (Int i = 0; i < axes(0); i++) {
//               min = fmin(min, im_->getAt(IPosition(4, i, j, k, l)));
//             }
//           }
//         }
//       }
//     }
//     if (size == 3) {
//       for (Int k = 0; k < ip(2); k++) {
//         for (Int j = 0; j < ip(1); j++) {
//           for (Int i = 0; i < ip(0); i++) {
//             min = fmin(min, im_->getAt(IPosition(3, i, j, k)));
//           }
//         }
//       }
//     }
//     if (size == 2) {
//       for (Int k = 0; k < ip(1); k++) {
//         for (Int j = 0; j < ip(0); j++) {
//           min = fmin(min, im_->getAt(IPosition(2, j, k)));
//         }
//       }
//     }
//     //cout << "minVal=" << min << endl;
//
//     Interpolate2D interp(Interpolate2D::LINEAR);
//     //Interpolate2D interp(Interpolate2D::CUBIC);
//
//     Int outNX = scale * xx;
//     Int outNY = scale * yy;
//      
//     Double incrX = (xx - 1.0) / (outNX - 1.0);
//     Double incrY = (yy - 1.0) / (outNY - 1.0);
//
//     for (Int k = 0; k < shapeOut(2); k++) {
//
//        Matrix<Float> out(outNX, outNY); 
//        Matrix<Float> in(xx, yy); 
//        out = min; 
//        for (Int j = 0; j < ip(axes(1)); j++) {
//           for (Int i = 0; i < ip(axes(0)); i++) {
//              in(i, j) = im_->getAt(IPosition(3, i, j, k));
//           }
//        }
//
//        Vector<Double> inloc(2);
//        for (Int i = 0, inloc[0] = 0; i < outNX; i++, inloc[0] += incrX) {
//           for (Int j = 0, inloc[1] = 0; j < outNY; j++, inloc[1] += incrY) {
//            interp.interp(out(i, j), inloc, in);
//            //cout << "i=" << i << " j=" << j
//            //     << " inloc[0]=" << inloc[0] 
//            //     << " inloc[1]=" << inloc[1] << endl;
//           }
//        }
//        //cout << "shapeOut=" << shapeOut << endl;
//        //cout << "out.shape()=" << out.shape() << endl;
//       
//        for (Int j = 0; j < shapeOut(1); j++) {
//           for (Int i = 0; i < shapeOut(0); i++) {
//              data(i, j, k) = out(i, j);
//              //pOut->getAt(IPosition(3, i, j, k));
//              //cout << data(i, j, k) << endl;
//           }
//        }
//     }
//   }
//   if (size == 2) {
//     pol = -1;
//     data.resize(shapeOut(0), shapeOut(1), 1);
//     for (Int j = 0; j < shapeOut(1); j++) {
//        for (Int i = 0; i < shapeOut(0); i++) {
//           data(i, j, 0) = pOut->getAt(IPosition(2, i, j));
//        }
//     }
//   }
//   */
//   //if (pOut)
//   //   delete pOut;
