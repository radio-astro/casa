//# Copyright (C) 2005
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
//# $Id: $


#include <casa/System/ProgressMeter.h>
#include <casa/BasicSL/String.h>

#include <graphics/X11/X_enter.h>
#include <graphics/X11/X_exit.h>

#include <casa/namespace.h>

int main( int argc, char ** argv )
{
    Double min = 0;
    Double max = 100;

    ProgressMeter big(min, max, "Big Job");
    big._update(1);
    sleep(3);
    big._update(21);

    ProgressMeter bus(min, max, "Busy Job");
    bus.busy();

    ProgressMeter small(min, max, "Small Job");
    for (Int i = (Int)min; i <= (Int)max; i++) {
        small._update(i);
        sleep(1);
        if (i == 20) {
           bus.done();
           big._update(80);
        } 
    }

    big._update((Int)max);

    return 1;
}

