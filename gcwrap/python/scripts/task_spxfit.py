
##########################################################################
# task_spxfit.py
#
# Copyright (C) 2008, 2009
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
# Fit spectral index like functions.
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
# spxfit => sp(ectral) (inde)x fit(ter)
# but in general it can be used for any image axis
# </etymology>
#
# <synopsis>
# spxfit fits models to 1-d profiles. It is built on top of ia.fitprofile()
# </synopsis> 
#
# <example>
#
# </example>
#
# <motivation>
# To make users happy, cf https://bugs.aoc.nrao.edu/browse/CAS-3116
# </motivation>
#

###########################################################################
from taskinit import *

def spxfit(
	imagename, box, region, chans,
	stokes, axis, mask, minpts, multifit, plpest,
	plpfix, plpdiv, plpsol, plperr,
	model, residual, wantreturn,
	stretch, logresults, logfile, append,
    sigma=None, outsigma=None
):
    casalog.origin('spxfit')
    retval = None
    myia = iatool()
    try:
        if (not myia.open(imagename)):
            raise Exception, "Cannot create image analysis tool using " + imagename
        retval = myia.fitprofile(
			box=box, region=region, chans=chans,
			stokes=stokes, axis=axis, mask=mask,
			minpts=minpts, ngauss=0,
			multifit=multifit, plpest=plpest, plpfix=plpfix,
			plpdiv=plpdiv,  model=model,
			residual=residual, 
			stretch=stretch, logresults=logresults,
			plpsol=plpsol, plperr=plperr, logfile=logfile,
			append=append, 
			sigma=sigma, outsigma=outsigma
		)
    except Exception, instance:
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
        retval = None
    myia.done()
    if (wantreturn):
    	return retval
    else:
    	if (retval):
    	   del retval
    	return None



