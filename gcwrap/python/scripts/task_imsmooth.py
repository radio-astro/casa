########################################################################3
#  task_imsmooth.py
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
# <summary>
# CASA task for smoothing an image, by doing Forier-based convolution
# on a CASA image file.
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <author>
# Shannon Jaeger (University of Calgary)
# </author>
#
# <etymology>
# imsmooth stands for image smoothing
# </etymology>
#
# <synopsis>
# task_imsmooth.py is a Python script providing an easy to use task
# for smoothing an image. 
#
# The convolv2d function of the image tool is used to do the work,
# a description of this function can be found at
# http://casa.nrao.edu/docs/casaref/image.convolve2d.html#x27-270001.1.1
# 
# </synopsis> 
#
# <example>
# <srcblock>
# # The following code snippet find the 1-moments, intensity-weighted
# # coordinate, often used for finding velocity fields.
# imsmooth( imagename='myimage', kernel='gaussian', outfile='myimage.smooth' )
#
#
# </example>
#
# <motivation>
# To provide a user-friendly method to smooth images.
# </motivation>
#
# <todo>
# </todo>

import os
import numpy
from taskinit import *

def imsmooth(
    imagename, kernel, major, minor, pa, targetres, region,
    box, chans, stokes, mask, outfile, stretch
):
    casalog.origin( 'imsmooth' )
    retValue = False

    # boxcar, tophat and user-defined kernel's are not supported
    # yet.
    if ( not ( kernel.startswith( 'gaus' ) or  kernel.startswith( 'box' ) ) ):
        casalog.post( 'Our deepest apologies gaussian kernels is the only'
                      +' type supported at this time.', 'SEVERE' )
        return retValue
    

    # First check to see if the output file exists.  If it
    # does then we abort.  CASA doesn't allow files to be
    # over-written, just a policy.
    if ( len( outfile ) < 1 ):
        outfile = 'imsmooth_results.im'
        casalog.post( "The outfile paramter is empty, consequently the" \
                      +" smoothed image will be\nsaved on disk in file, " \
                      + outfile, 'WARN' )
    if ( len( outfile ) > 0 and os.path.exists( outfile ) ):
        casalog.post( 'Output file, '+outfile+\
                      ' exists. imsmooth can not proceed, please\n'+\
                      'remove it or change the output file name.', 'SEVERE' )
        return retValue
    _myia = iatool()
    _myia.open(imagename)
    mycsys = _myia.coordsys()
    reg = rg.frombcs(
        mycsys.torecord(), _myia.shape(), box, chans,
        stokes, "a", region
    )
    _myia.done()

    # If the values given are integers we assume they are given in
    # arcsecs and alter appropriately
    if type( major ) == int:
        major=str(major)+'arcsec'
    if type( minor ) == int:
        minor=str(minor)+'arcsec'                

    try:        
        if ( kernel.startswith( "gaus" ) ):
            # GAUSSIAN KERNEL
            casalog.post( "Calling convolve2d with Gaussian kernel", 'NORMAL3' )
            _myia.open( imagename )
            _myia.convolve2d(
                axes=[0,1], region=reg, major=major,
                minor=minor, pa=pa, outfile=outfile,
                mask=mask, stretch=stretch, targetres=targetres
            )
            _myia.done()
            retValue = True

        elif (kernel.startswith( "box" ) ):
            # BOXCAR KERNEL
            #
            # Until convolve2d supports boxcar we will need to
            # use sepconvolve to do this.
            #
            # BIG NOTE!!!!!
            # According to Gaussian2D documentation the default position
            # angle aligns the major axis along the y-axis, which typically
            # be lat.  So this means that we need to use the major quantity
            # on the y axis (or 1) for sepconvolve.

            _myia.open( imagename )
            casalog.post( "ia.sepconvolve( axes=[0,1],"+\
                          "types=['boxcar','boxcar' ],"+\
                          "widths=[ "+str(minor)+", "+str(major)+" ],"+ \
                          "region="+str(reg)+",outfile="+outfile+" )",\
                          'DEBUG2' )
            #retValue = ia.sepconvolve( axes=[0,1], types=['box','box' ],\
            #                           widths=[ minor, major ], \
            #                           region=reg,outfile=outfile )
            _myia.sepconvolve(
                axes=[0,1], types=['box','box' ],
                widths=[ minor, major ],
                region=reg,outfile=outfile,
                mask=mask, stretch=stretch
            )
            _myia.done()
            retValue = True
        else:
            casalog.post( 'Unrecognized kernel type: ' + kernel, 'SEVERE' )
            retValue = False
        
    except Exception, instance:
        casalog.post( 'Something has gone wrong with the smoothing. Try, try again, and ye shall suceed', 'SEVERE' )
        casalog.post( 'Exception thrown is: '+str(instance), 'SEVERE' )
        return False

    
    return retValue
