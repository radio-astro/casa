
##########################################################################
# task_slsearch.py
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
# Task to search a spectral line table
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
# slsearch => s(pectral) l(ine) search
# </etymology>
#
# <synopsis>
# slsearch searches a spectral line table. It is built on sl.search().
# </synopsis> 
#
# <example>
# newsl = slsearch(table="mysplatlist.tbl")
#
# </example>
#
# <motivation>
# To allow splatalogue spectral line lists to be searchable in CASA.
# </motivation>
#
###########################################################################
from taskinit import *

def slsearch(
    table=None, outfile=None, freqrange=None,
    species=None, reconly=None, chemnames=None,
    qns=None, intensity=None, smu2=None,
    loga=None, el=None, eu=None, rrlinclude=None,
    rrlonly=None, verbose=None, logfile=None, 
    append=None
):
    casalog.origin('slsearch')
    newsl = None
    mysl = sltool()
    try:
        mysl.open(table)
        newsl = mysl.search(
            outfile=outfile, freqrange=freqrange,
            species=species, reconly=reconly,
            chemnames=chemnames, qns=qns,
            intensity=intensity, smu2=smu2,
            loga=loga, el=el, eu=eu,
            rrlinclude=rrlinclude, rrlonly=rrlonly,
            verbose=verbose, logfile=logfile,
            append=append
        )
            
        if (not newsl):
            raise Exception, "Exception when running sl.search()"
        return True
    except Exception, instance:
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
        raise
    finally:
        if mysl:
            mysl.done()
        if newsl:
            newsl.done()
