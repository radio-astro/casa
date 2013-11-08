#include <display/Display/PixelCanvas.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <display/Display/DisplayEnums.h>


namespace casa { //# NAMESPACE CASA - BEGIN

	Matrix<Float> PixelCanvas::getMarker(const Display::Marker& marker,
	                                     const Float& pixelHeight) {

		Matrix<Float> unScaled(0,0);

		switch(marker) {

			// Triangles were 0.433

		case Display::Triangle:
			unScaled.resize(3,2);
			unScaled(0,0) = 0;
			unScaled(0,1) = 0.4330127;
			unScaled(1,0) = 0.5 ;
			unScaled(1,1) = -0.4330127;
			unScaled(2,0) = -0.5 ;
			unScaled(2,1) = -0.4330127;
			break;

		case Display::InvertedTriangle:
			unScaled.resize(3,2);
			unScaled(0,0) = 0;
			unScaled(0,1) = -0.4330127;
			unScaled(1,0) = 0.5 ;
			unScaled(1,1) = 0.4330127;
			unScaled(2,0) = -0.5 ;
			unScaled(2,1) = 0.4330127;
			break;


		case Display::Square:
			unScaled.resize(4,2);

			unScaled(0,0) = -0.5;
			unScaled(0,1) =  -0.5;
			unScaled(1,0) = -0.5;
			unScaled(1,1) =  0.5;
			unScaled(2,0) = 0.5;
			unScaled(2,1) =  0.5;
			unScaled(3,0) = 0.5;
			unScaled(3,1) =  -0.5;
			break;

		case Display::Diamond:
			unScaled.resize(4,2);

			unScaled(0,0) = -0.5;
			unScaled(0,1) = 0;
			unScaled(1,0) = 0;
			unScaled(1,1) = 0.5;
			unScaled(2,0) = 0.5;
			unScaled(2,1) = 0;
			unScaled(3,0) = 0;
			unScaled(3,1) = -0.5;
			break;

		case Display::FilledDiamond:
			unScaled.resize(4,2);

			unScaled(0,0) = -0.5;
			unScaled(0,1) = 0;
			unScaled(1,0) = 0;
			unScaled(1,1) = 0.5;
			unScaled(2,0) = 0.5;
			unScaled(2,1) = 0;
			unScaled(3,0) = 0;
			unScaled(3,1) = -0.5;
			break;

		case Display::Cross:
			unScaled.resize(5,2);

			unScaled(0,0) = -0.5  ;
			unScaled(0,1) = 0;
			unScaled(1,0) =  0.5  ;
			unScaled(1,1) = 0;
			unScaled(2,0) =  0 ;
			unScaled(2,1) = 0;
			unScaled(3,0) =  0 ;
			unScaled(3,1) = 0.5;
			unScaled(4,0) =  0 ;
			unScaled(4,1) = -0.5;
			break;

		case Display::X:
			unScaled.resize(5,2);
			unScaled(0,0) = -.5  ;
			unScaled(0,1) = -.5;
			unScaled(1,0) = .5   ;
			unScaled(1,1) = .5;
			unScaled(2,0) = 0    ;
			unScaled(2,1) = 0;
			unScaled(3,0) = -.5  ;
			unScaled(3,1) = .5;
			unScaled(4,0) = .5   ;
			unScaled(4,1) = -.5;
			break;

		case Display::Circle:
			break;

		case Display::FilledCircle:
			break;

		case Display::FilledSquare:
			unScaled.resize(4,2);

			unScaled(0,0) = -0.5;
			unScaled(0,1) =  -0.5;
			unScaled(1,0) = -0.5;
			unScaled(1,1) =  0.5;
			unScaled(2,0) = 0.5;
			unScaled(2,1) =  0.5;
			unScaled(3,0) = 0.5;
			unScaled(3,1) =  -0.5;
			break;

		case Display::FilledTriangle:
			unScaled.resize(3,2);
			unScaled(0,0) = 0;
			unScaled(0,1) = 0.4330127 ;
			unScaled(1,0) = 0.5 ;
			unScaled(1,1) = -0.4330127 ;
			unScaled(2,0) = -0.5 ;
			unScaled(2,1) = -0.4330127 ;
			break;

		case Display::FilledInvertedTriangle:
			unScaled.resize(3,2);
			unScaled(0,0) = 0;
			unScaled(0,1) = -0.4330127;
			unScaled(1,0) = 0.5 ;
			unScaled(1,1) = 0.4330127;
			unScaled(2,0) = -0.5 ;
			unScaled(2,1) = 0.4330127;
			break;

		case Display::CircleAndCross:
			unScaled.resize(5,2);

			unScaled(0,0) = -0.5  ;
			unScaled(0,1) = 0;
			unScaled(1,0) =  0.5  ;
			unScaled(1,1) = 0;
			unScaled(2,0) =  0 ;
			unScaled(2,1) = 0;
			unScaled(3,0) =  0 ;
			unScaled(3,1) = 0.5;
			unScaled(4,0) =  0 ;
			unScaled(4,1) = -0.5;
			break;

		case Display::CircleAndX:
			unScaled.resize(5,2);
			// was 0.5
			unScaled(0,0) = -0.35355339  ;
			unScaled(0,1) = -0.35355339;
			unScaled(1,0) = 0.35355339   ;
			unScaled(1,1) = 0.35355339;
			unScaled(2,0) = 0            ;
			unScaled(2,1) = 0;
			unScaled(3,0) = -0.35355339  ;
			unScaled(3,1) = 0.35355339;
			unScaled(4,0) = 0.35355339   ;
			unScaled(4,1) = -0.35355339;
			break;

		default:
			unScaled.resize(2,2);
			unScaled(0,0) = -.5  ;
			unScaled(0,1) = -.5;
			unScaled(1,0) = .5   ;
			unScaled(1,1) = .5;
			break;
		}
		return  (unScaled * pixelHeight);
	}







} //# NAMESPACE CASA - END

