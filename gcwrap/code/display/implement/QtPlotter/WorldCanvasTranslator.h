/*
 * WorldCanvasTranslator.h
 *
 *	Pure interface class used to decrease the coupling between
 *	the QtCanvas and classes needing to plot their data on the
 *	canvas (specifically ProfileFitMarker).
 *  Created on: May 23, 2012
 *      Author: slovelan
 */

#ifndef WORLDCANVASTRANSLATOR_H_
#define WORLDCANVASTRANSLATOR_H_

namespace casa {

class WorldCanvasTranslator {
public:
	WorldCanvasTranslator();
	virtual int getPixelX( double dataX ) const = 0;
	virtual int getPixelY ( double dataY ) const = 0;
	virtual ~WorldCanvasTranslator();
};

} /* namespace casa */
#endif /* WORLDCANVASTRANSLATOR_H_ */
