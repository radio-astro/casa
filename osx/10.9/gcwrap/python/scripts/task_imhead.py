# -*- coding: utf-8 -*-
#######################################################################3
#  task_imhead.py
#
#
# Copyright (C) 2008
# Associated Universities, Inc. Washington DC, USA.
#
# This library is free software; you can redistribute it and/or modify it
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
# Shannon Jaeger (University of Calgary)
# </author>
#
# <summary>
# CASA task for reading/writing/listing the CASA Image header
# contents
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <etymology>
# imhead stands for image header
# </etymology>
#
# <synopsis>
# task_imhead.py is a Python script providing an easy to use task
# for adding, removing, listing and updating the contents of a CASA
# image.  This task is very useful for fixing mistakes made in the
# importing of FITS files into CASA images, as well as seeing what
# checking the header to see what type of data is in the image file.
#
# NOTE: This task does not edit FITS files, but FITS files may
#       be created with exportuvfits task
#
# </synopsis>
#
# <example>
# <srcblock>
## The following code lists the keyword/value pairs found in
## the header of the CASA image file ngc133.clean.image.  The information
## is stored in Python variable hdr_info in a Python dictionary.
## The information is also listed in the CASA logger.  The observation
## date is #printed out from the hdr_info variable.
# hdr_info = imhead( 'ngc4826.clean.image', 'list' )
##print "Observation Date: ", hdr_info['date-obs']
#
## The following exmple displays the CASA images history in the CASA logger.
# imhead( 'ngc4826.clean.image', 'history' )
#
## The following example adds a new, user-defined keyword to the
## CASA image ngc4826.clean.image
# imhead( 'ngc4826.clean.image', 'add', 'observer 1', 'Joe Smith'  )
# imhead( 'ngc4826.clean.image', 'add', 'observer 2', 'Daniel Boulanger'  )
#
## The following example changes the name of the observer keyword,
## OBSERVE, to ALMA
# imhead( 'ngc4826.clean.image', 'put', 'telescope', 'ALMA' )
# </srblock>
# </example>
#
# <motivation>
# To provide headering modification and reading tools to the CASA users.
# </motivation>
#
# <todo>
# </todo>

import numpy
import os

from taskinit import *

def imhead(
    imagename, mode, hdkey, hdvalue, verbose
):
    if mode.startswith('h') or mode.startswith('s'):
        myia = iatool()
        try:
            myia.open(imagename)
            if mode.startswith('h'):
                myia.history()
            elif mode.startswith('s'):
                myia.summary(verbose=verbose)
            return True
        except Exception, instance:
            casalog.post(str('*** Error *** ') + str(instance), 'SEVERE')
            raise
        finally:
            myia.done()
    if (
        mode.startswith('a') or mode.startswith('d')
        or mode.startswith('g') or mode.startswith('l')
        or mode.startswith('p')
    ):
        myimd = imdtool()
        try:
            myimd.open(imagename)
            if mode.startswith('a'):
                return myimd.add(hdkey, hdvalue)
            elif mode.startswith('d'):
                return myimd.remove(hdkey, hdvalue)
            elif mode.startswith('g'):
                return myimd.get(hdkey)
            elif mode.startswith('l'):
                return myimd.list(True)
            elif mode.startswith('p'):
                return myimd.set(hdkey, hdvalue)
        except Exception, instance:
            casalog.post(str('*** Error *** ') + str(instance), 'SEVERE')
            return False
        finally:
            myimd.done()
            
        casalog.post('Unknown imhead mode ' + str(mode), 'SEVERE')
        return False
                
        