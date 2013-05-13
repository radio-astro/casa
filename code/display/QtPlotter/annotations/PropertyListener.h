/*
 * PropertyListener.h
 *
 *  Created on: Sep 28, 2012
 *      Author: slovelan
 */

#ifndef PROPERTYLISTENER_H_
#define PROPERTYLISTENER_H_

namespace casa {

	class PropertyListener {
	public:
		PropertyListener();
		virtual void propertiesChanged() = 0;
		virtual ~PropertyListener();
	};

} /* namespace casa */
#endif /* PROPERTYLISTENER_H_ */
