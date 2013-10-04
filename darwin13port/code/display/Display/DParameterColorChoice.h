//# DParameterColorChoice.h: class to store and retrieve color selection
//# Copyright (C) 2000
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
//# $Id$

#ifndef TRIALDISPLAY_DPARAMETERCOLORCHOICE_H
#define TRIALDISPLAY_DPARAMETERCOLORCHOICE_H

#include <display/Display/DParameterChoice.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of DParameterChoice to store color selection parameters.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" test="" demos="">
// </reviewed>

// <prerequisite>
// <li> <linkto class=DParameterChoice>DParameterChoice</linkto>
// </prerequisite>

// <etymology>
// DParameterColorChoice is an implementation of a DisplayParameter
// providing a choice parameter type tuned to selecting colors.
// </etymology>

// <motivation>
// It is common to have the user select a color for a particular
// element of the display, hence this class.
// </motivation>

// <thrown>
// <li> None.
// </thrown>

// <todo asof="2000/01/31">
// <li> Nothing known.
// </todo>

	class DParameterColorChoice : public DParameterChoice {

	public:

		// Constructor takes the name of the parameter, a short
		// description and some help text.  The context of the
		// parameter, and whether it is editable, can also be
		// given.  The default and initial color is always the
		// foreground color.
		DParameterColorChoice(const String name, const String description,
		                      const String help, const String context = "",
		                      const Bool editable = True);

		// Copy constructor using copy semantics.
		DParameterColorChoice(const DParameterColorChoice &other);

		// Destructor.
		virtual ~DParameterColorChoice();

		// Copy assignment.
		DParameterColorChoice &operator=(const DParameterColorChoice &other);

	protected:

		// (Required) default constructor.
		DParameterColorChoice();

	};


} //# NAMESPACE CASA - END

#endif

