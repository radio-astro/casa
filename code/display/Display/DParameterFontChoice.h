//# DParameterFontChoice.h: class to store and retrieve font selection
//# Copyright (C) 2000,2002
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
//# $Id:

#ifndef TRIALDISPLAY_DPARAMETERFONTCHOICE_H
#define TRIALDISPLAY_DPARAMETERFONTCHOICE_H

#include <display/Display/DParameterChoice.h>
#include <casa/Arrays/Vector.h>
namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of DParameterChoice to store font selection parameters.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" test="" demos="">
// </reviewed>

// <prerequisite>
// <li> <linkto class=DParameterChoice>DParameterChoice</linkto>
// </prerequisite>

// <etymology>
// DParameterFontChoice is an implementation of a DisplayParameter
// providing a choice parameter type tuned to selecting fonts.
// </etymology>

// <motivation>
// It is common to have the user select a font for a particular
// element of the display, hence this class.
// </motivation>

// <synopsis>
// This class is basically a placeholder for several different
// representations of fonts. Currently avaiable fonts are hard-coded
// into this class. The class holds three representations - Their 'name',
// X-windows font description and post script font name. This class can
// be used in conjustion with <linkto class=DLFont>DLFont</linkto>
// to set the font on a number of different pixel canvases.
// </synopsis>

// <thrown>
// <li> None.
// </thrown>

// <todo asof="2002/05/13">
// <li> Nothing known.
// </todo>

	class DParameterFontChoice : public DParameterChoice {

	public:

		// Constructor takes the name of the parameter, a short
		// description and some help text.  The context of the
		// parameter, and whether it is editable, can also be
		// given.
		DParameterFontChoice(const String name, const String description,
		                     const String help, const String context = "");

		// Copy constructor using copy semantics.
		DParameterFontChoice(const DParameterFontChoice &other);

		// Destructor.
		virtual ~DParameterFontChoice();

		// Copy assignment.
		DParameterFontChoice &operator=(const DParameterFontChoice &other);

	protected:

		// (Required) default constructor.
		DParameterFontChoice();

	private:

	};


} //# NAMESPACE CASA - END

#endif



