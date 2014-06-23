##########################################################################
# task_splattotable.py
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
# Task to convert a spectral line list exported from Splatalogue (www.splatalogue.net) to a CASA table
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
# splattotable => splat(alogue) to (CASA) table
# </etymology>
#
# <synopsis>
# splattotable converts a splatalogue spectral line list to a CASA table. It is built on sl.splattotable.
# </synopsis> 
#
# <example>
# newsl = splattotable("mysplatlist.txt", "mynewsl.tbl", true)
#
# </example>
#
# <motivation>
# To allow splatalogue spectral line lists to be searchable in CASA.
# </motivation>
#
###########################################################################
from taskinit import *

def splattotable(filenames=None, table=None):
    casalog.origin('splattotable')
    newsl = None
    mysl = sltool()

    try:
        if (len(table) == 0):
            raise Exception("table must be specified.")
        newsl = mysl.splattotable(filenames=filenames, table=table)
        if (not newsl):
            raise Exception, "Exception when running sl.splattotable"
        return True
    except Exception, instance:
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
        raise
    finally:
        if newsl:
            newsl.done()
        if mysl:
            mysl.done()
