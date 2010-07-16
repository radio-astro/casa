
##########################################################################
# task_imcollapse.py
#
# Copyright (C) 2008, 2009, 2010
# Associated Universities, Inc. Washington DC, USA.
#
# This script is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation,
# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning AIPS++ should be adressed as follows:
#        Internet email: aips2-request@nrao.edu.
#        Postal address: AIPS++ Project Office
#                        National Radio Astronomy Observatory
#                        520 Edgemont Road
#                        Charlottesville, VA 22903-2475 USA
#
# <author>
# Dave Mehringer
# </author>
#
# <summary>
# Task to collapse an image along a specified axis,
# computing a specified aggregate function of pixels along that axis
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed>
#
# <prerequisite>
# <ul>
#
# </ul>
# </prerequisite>
#
# <etymology>
# imtrans => im(age) collapse
# </etymology>
#
# <synopsis>
# imtrans collapses an image along a specified axis. It is built on top of ia.collapse()
# </synopsis> 
#
# <example>
# collapsed_image_tool = imcollapse(imagename="myim.im", outfile="collapsed.im", axis=2, function="variance", wantreturn=true)
#
# </example>
#
# <motivation>
# To make users happy (https://bugs.aoc.nrao.edu/browse/CAS-1222)
# and associated casacore class is prereq for specfit work
# </motivation>
#

###########################################################################
from taskinit import *

def imcollapse(
    imagename=None, function=None, axis=None, outfile=None, box=None,
    region=None, chans=None, stokes=None, mask=None, wantreturn=None,
    overwrite=None
):
    casalog.origin('imcollapse')
    retval = None
    myia = iatool.create()
    try:
        if (not myia.open(imagename)):
            raise Exception, "Cannot create image analysis tool using " + imagename
        ia_tool = myia.collapse(
            function, axis, outfile, region, box, chans, stokes, mask, overwrite
        )
        if wantreturn:
            retval = ia_tool
        else:
            ia_tool.done()
    except Exception, instance:
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
    if (myia):
        myia.done()
    return retval
