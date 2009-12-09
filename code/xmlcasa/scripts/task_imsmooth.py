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
# <prerequisite>
# <ul>
#   <li> <linkto class="imregion.py:description">imregion</linkto> 
# </ul>
# </prerequisite>
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
from imregion import *


def imsmooth( imagename, kernel, major, minor, region, box, chans, stokes, mask, outfile):
    casalog.origin( 'imsmooth' )
    retValue = False

    # boxcar, tophat and user-defined kernel's are not supported
    # yet.
    if ( not ( kernel.startswith( 'gaus' ) or  kernel.startswith( 'boxc' ) ) ):
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
    
    # Get the region information, if the user has specified
    # a region file it is given higher priority.
    reg={}
    try:
    	if ( len(region)>1 ):
	    if ( len(box)>1 or len(chans)>1 or len(stokes)>1 ):
		casalog.post( "Ignoring region selection\ninformation in" \
			      " the box, chans, and stokes parameters."\
			      " Using region information\nin file: " + region, 'WARN' );
            # Prepend the current working directory if the first
            # character of the region is not '/'
            if os.path.exists( region ):
                    # We have a region file on disk!
                reg=rg.fromfiletorecord( region );
            else:
                # The name given is the name of a region stored
                # with the image.
                # Note that we accept:
                #    'regionname'          -  assumed to be in imagename
                #    'my.image:regionname' - in my.image
                reg_names=region.split(':')
                if ( len( reg_names ) == 1 ):
                    reg=rg.fromtabletorecord( imagename, region, False )
                else:
                    reg=rg.fromtabletorecord( reg_names[0], reg_names[1], False )
	else: 
	    reg=imregion( imagename, chans, stokes, box, '', '' )
    except Exception, instance:
        casalog.post( 'Unable to construct specified region.', 'SEVERE' )
	casalog.post( 'Exception thrown is: '+str(instance), 'NORMAL4' )
        return retValue

    if ( reg == {} ):
        casalog.post( 'Ill-formed region: '+str(reg)+'. can not continue.',
                      'SEVERE' )
        return retValue
    
    casalog.post( 'Smoothing to be done in region: '+str(reg), 'DEBUG2' )

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
            ia.open( imagename )
            casalog.post( "ia.convolve2d( major="+str(major)+", minor="\
                          +str(minor)+", outfile="+outfile+")", 'DEBUG2' )
            #retValue = ia.convolve2d( axes=[0,1], region=reg, major=major, \
            #                          minor=minor, outfile=outfile )
            ia.convolve2d( axes=[0,1], region=reg, major=major, \
                                         minor=minor, outfile=outfile )
            ia.done()
            retValue = True

        elif (kernel.startswith( "boxc" ) ):
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

            ia.open( imagename )
            casalog.post( "ia.sepconvolve( axes=[0,1],"+\
                          "types=['boxcar','boxcar' ],"+\
                          "widths=[ "+str(minor)+", "+str(major)+" ],"+ \
                          "region="+str(reg)+",outfile="+outfile+" )",\
                          'DEBUG2' )
            #retValue = ia.sepconvolve( axes=[0,1], types=['box','box' ],\
            #                           widths=[ minor, major ], \
            #                           region=reg,outfile=outfile )
            ia.sepconvolve( axes=[0,1], types=['box','box' ],\
                                       widths=[ minor, major ], \
                                       region=reg,outfile=outfile )
            ia.done()
            retValue = True
        else:
            casalog.post( 'Unrecognized kernel type: ' + kernel, 'SEVERE' )
            retValue = False
        
    except Exception, instance:
        casalog.post( 'Something has gone wrong with the smoothing. Try, try again, and ye shall suceed', 'SEVERE' )
        casalog.post( 'Exception thrown is: '+str(instance), 'NORMAL4' )
        return False

    
    return retValue
