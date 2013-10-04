
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
# Task correct an image for primary beam attenuation.
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
# impbcor => im(age) pb(primary beam) cor(rect)
# </etymology>
#
# <synopsis>
# impbcor corrects and image for primary beam attenuation. It is built on top of ia.pbcor()
# </synopsis> 
#
# <example>
# pbcorrected_image_tool = impbcor(imagename="myim.im", pbimage="mypb.im", outfile="corrected.im", wantreturn=true)
#
# </example>
#
# <motivation>
# https://bugs.aoc.nrao.edu/browse/CAS-3256
# </motivation>
#

###########################################################################
from taskinit import *

def impbcor(
    imagename=None, pbimage=None, outfile=None, overwrite=None,
    box=None, region=None, chans=None, stokes=None, mask=None,
    mode=None, cutoff=None,  stretch=None
):
    casalog.origin('impbcor')
    myia = iatool()
    try:
        if (not myia.open(imagename)):
            raise Exception, "Cannot create image analysis tool using " + imagename
        if (len(outfile) == 0):
            raise Exception, "outfile must be specified"
        ia_tool = myia.pbcor(
            pbimage=pbimage, outfile=outfile, overwrite=overwrite,
            box=box, region=region, chans=chans, stokes=stokes,
            mask=mask, mode=mode, cutoff=cutoff, stretch=stretch
        )
        ia_tool.done()
        return True
    except Exception, instance:
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
        raise
    finally:
        if (myia):
            myia.done()
        
