########################################################################3
#  task_immath.py
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
# Shannon Jaeger, University of Calgary  (image math) 
# Takeshi Nakazato, National Radio Astronomy Obaservatory (polarization)
# </author>
#
# <prerequisite>
# </prerequisite>
#
# <etymology>
# immath stands for image mathematics
# </etymology>
#
# <synopsis>
#    This task evaluates mathematical expressions involving existing
#    image files. The results of the calculations are stored in the 
#    designated output file.  Options are available to specify mathematical 
#    expression directly or pre-defined expression for calculation of 
#    spectral index image, and polarization intensity and position angle 
#    images are available. The image file names imbedded in the expression or
#    specified in the imagename parameter for the pre-defined calculations may
#    be CASA images or FITS images.
#
#
#    NOTE: Index values for axes start at 0 for the box and chans
#          parameters, but at 1 when used with the indexin function
#          in expression. Use the imhead task to see the range of
#          values for each axes.
#    
#
#    Keyword arguments:
#    outfile -- The file where the results of the image calculations 
#                are stored.  Overwriting an existing outfile is not permitted.
#            Default: none;  Example: outfile='results.im'
#    mode -- mode for mathematical operation
#            Default: evalexpr
#            Options: 'evalexpr' : evalulate a mathematical expression defined in 'expr' 
#                     'spix' : spectalindex image 
#                     'pola' : polarization position angle image 
#                     'poli' : polarization intesity image 
#           mode expandable parameters
#            expr -- (for mode='evalexpr') A mathematical expression, with image file names.
#              Image file names MUST be enclosed in double quotes (&quot;)
#              Default: none 
#              Examples:
#                 Make an image that is image1.im - image2.im
#                   expr=' (&quot;image1.im&quot; - &quot;image2.im&quot; )'
#                 Clip an image below a value (0.5 in this case)
#                   expr = ' iif(&quot;image1.im&quot;>=0.5, &quot;image1.im&quot;, 0.0) '
#                         Note: iif (a, b, c)   a is the boolian expression
#                                               b is the value if true
#                                               c is the value if false
#                 Take the rms value of two images
#                   expr = ' sqrt(&quot;image1.im&quot; * &quot;image1.im&quot; + &quot;image2.im&quot; * &quot;image2.im&quot;) '
#                         Note: No exponentiaion available?
#                 Build an image pixel by pixel from the minimum of (image2.im, 2*image1.im)
#                   expr='min(&quot;image2.im&quot;,2*max(&quot;image1.im&quot;))'
#            imagename -- (for mode='spix','pola','poli') input image names        
#              Default: none;
#              Examples: mode='spix'; imagename=['image1.im','image2.im'] will calculate 
#                       an image of log(S1/S2)/log(f1/f2), where S1 and S2 are fluxes and 
#                       f1 and f2 are frequencies
#                       mode='pola'; imagename=['imageQ.im','imageU.im'] will calculate 
#                       an image of polarization angle distribution, where imageQ.im and 
#                       imageU.im are Stokes Q and U images, respectively. Calculate 0.5*arctan(U/Q).
#                       mode='poli'; imagename=['imageQ.im','imageU.im','imageV.im'] will calculate
#                       total polarization intensity image, where imageQ.im, imageU.im, imageV.im
#                       are Stokes Q, U, and V images, respectively.
#            sigma - (for mode='poli') standard deviation of noise of Stokes images with unit such as
#                    Jy/beam to correct for bias 
#              Default: '0.0Jy/beam' (= no debiasing)
#    mask -- Name of mask applied to each image in the calculation
#            Default '' means no mask;  Example: mask='orion.mask'.  
#    region -- File path to an ImageRegion file.
#            An ImageRegion file can be created with the CASA
#            viewer's region manager.  Typically ImageRegion files
#            will have the suffix '.rgn'.  If a region file is given
#            then the box, chans, and stokes selections whill be 
#            ignored.
#            Default: none
#            Example: region='myimage.im.rgn'
#    box --  A box region on the directional plane
#            Only pixel values acceptable at this time.
#            Default: none (whole 2-D plane);  Example: box='10,10,50,50'
#    chans -- channel numbers, velocity, and/or frequency
#            Only channel numbers acceptable at this time.
#            Default: none (all);  Example: chans='3~20'   
#    stokes -- Stokes parameters to image, may or may not be separated
#            by commas but best if you use commas.
#            Default: none (all); Example: stokes='IQUV';
#            Options: 'I','Q','U','V','RR','RL','LR','LL','XX','YX','XY','YY', ... 
#
#    Available functions in the <i>expr</i> and <i>mask</i> paramters:
#    pi(), e(), sin(), sinh(), asinh(), cos(), cosh(), tan(), tanh(),
#    atan(), exp(), log(), log10(), pow(), sqrt(), complex(), conj()
#    real(), imag(), abs(), arg(), phase(), aplitude(), min(), max()
#    round(), isgn(), floor(), ceil(), rebin(), spectralindex(), pa(), 
#    iif(), indexin(), replace(), ...
#
#    For a full description of the allowed syntax see the 
#    Lattice Expression Language (LEL) documentation on the at:
#    http://aips2.nrao.edu/docs/notes/223/223.html
#
#    NOTE: where indexing and axis numbering are used in the above
#    functions they are 1-based, ie. numbering starts at 1.
#
# </synopsis> 
#
# <example>
# <srcblock>
# </srcblock
#
# </example>
#
# <motivation>
# To provide a user-friendly task interface to imagecalc and ???
# as well as an more user-friendling math syntax then what is
# provided by the CASA Lattice Exprssion Language.
# </motivation>
#
# <todo>
#  Crystal wanted different masks for different inputs
#  but unlikely that its really needed.
#
#  Add an "overwrite" output file parameter
#
#  Add polygon and circle region selection 
# </todo>
########################################################################3


import os
import shutil
from taskinit import *
from imregion import *

def immath(imagename, mode, outfile, expr, varnames, sigma, mask, \
           region, box, chans, stokes ):
    # Tell CASA who will be reporting
    casalog.origin('immath')
    retValue = False
    # NOTE: This step likely be eliminated
    # 
    # Remove any old tmp files that may be left from
    # a previous run of immath
    tb.clearlocks()    
    tmpFilePrefix='_immath_tmp'
    try:
        _immath_cleanup( tmpFilePrefix )
    except Exception, e:
        casalog.post( 'Unable to cleanup working directory '+os.getcwd()+'\n'+str(e), 'SEVERE' )
        raise Exception, str(e)


    # Keep track of which LEL functions are upper-case and
    # which are lower-case.  For some reason LEL was written
    # to be case sensitive!
    #
    # The values stored are in the opposite case of what we want, 
    # because we want to search for the invalid case.
    #
    # Note these were found in the various LEL*Enums.h files.
    lower_case=[ 'SIN(', 'SINH(', 'ASIN(', 'COS(', 'COSH(', 'ACOS(', 'TAN(', \
                 'TANH(', 'ATAN(', 'ATAN2(', 'EXP(', 'LOG(', 'LOG10(', 'POW(',\
                 'SQRT(', 'ROUND(', 'SIGN(', 'CEIL(', 'FLOOR(', 'ABS(', \
                 'ARG(', 'REAL(', 'IMAG(', 'CONJ(', 'COMPLEX(', 'FMOD(' \
                 'MIN(', 'MAX(', 'MIN1D(', 'MAX1D(', 'MEAN1D(', 'MEDIAN1D('\
                 'FRACTILE1D(', 'FRACTILERANGE1D(', 'SUM(', 'NELEM(', 'ALL('\
                 'ANY(', 'IIF(', 'REPLACE(', 'LENGTH(', 'INDEXIN(', 'NDIM' ]
    upper_case=[ ]
    
    # First check to see if the output file exists.  If it
    # does then we abort.  CASA doesn't allow files to be
    # over-written, just a policy.
    if ( len( outfile ) < 1 ):
        outfile = 'immath_results.im'
        casalog.post( "The outfile paramter is empty, consequently the" \
                      +" resultant image will be\nsaved on disk in file, " \
                      + outfile, 'WARN' )
    if ( len( outfile ) > 0 and os.path.exists( outfile ) ):
        raise Exception, 'Output file, '+outfile+\
              ' exists. immath can not proceed, please\n'+\
              'remove it or change the output file name.'
    
    # Find the list of filenames in the expression
    # also do a quick check to see if all of the files
    # exist
    #
    # TODO add a size check to make sure all images are the
    # same size.  Is this
    tmpfilenames=''
    if mode=='evalexpr':
        tmpfilenames=_immath_parse( expr )
        filenames=imagename
    else:
        filenames=imagename
    if ( isinstance( filenames, str ) ):
        filenames= [ filenames ]
    casalog.post( 'List of input files is: '+str(filenames), 'DEBUG1' )
    #for i in range( len(filenames) ):
    #        if ( not os.path.exists(filenames[i]) ):
    #            raise Exception, 'Image data set not found - please verify '+filenames[i]

    # Construct the variable name list.  We append to the list the
    # default variable names if the user hasn't supplied a full suite.
    if ( not isinstance( varnames, list ) ):
        name0=varnames
        varnames=[]
        if ( len(name0 ) > 0 ):
            varnames.append( name0 )
    nfile=max(len(filenames),len(tmpfilenames))
    #for i in range( len(varnames), len(filenames) ):
    for i in range( len(varnames), nfile ):
        varnames.append( 'IM'+str(i) )
    casalog.post( 'Variable name list is: '+str(varnames), 'DEBUG1' )

    #file existance check
    ignoreimagename=False
    if mode=='evalexpr':
        varnamesSet=set(varnames)
        count = 0
        for imname in tmpfilenames:
            # check if it is one of varnames, if not check the files in expr exist 
            if(not varnamesSet.issuperset(imname)):
               if( not os.path.exists(imname)):
                   raise Exception, 'Image data set not found - please verify '+imname
               else:
                   count=count+1            
        if len(tmpfilenames)==count:
            ignoreimagename=True
            filenames=tmpfilenames
        #if(tmpfilenames==['']): 
        #    for i in range(len(varnames)):
        #       if(expr.count(varnames[i])==0):
        #           casalog.post('Variable name '+varnames[i]+' not found in the expression.','WARN')
    if not ignoreimagename:
        for i in range( len(filenames) ):
            if ( not os.path.exists(filenames[i]) ):
                raise Exception, 'Image data set not found - please verify '+filenames[i]

       
    # Remove spaces from the expression.
    expr=expr.replace( ' ', '' )

    # Construct expressions for the spectral and polarization modes.
    # These are common, repetitive calculations that are handled for
    # the user.
    if mode=='spix':
        # calculate a spectral index distribution image
        if len(filenames) != 2:
            raise Exception, 'Requires two images at different frequencies'
        #expr = 'spectralindex("%s","%s")' % (filenames[0],filenames[1])

        expr = 'spectralindex('+varnames[0]+', '+varnames[1]+')'
    elif mode=='pola':
        # calculate a polarization position angle image
        if len(filenames) < 2:
            raise Exception, 'Requires separate Stokes Q and U images'
        else:
            if len(filenames) >2:
                casalog.post( "More than two images. Take first two and ignore the rest. " ,'WARN' );
            stkslist=__check_stokes(filenames)
            Uimage=Qimage=''
            for i in range(2):
                if type(stkslist[i])==list:
                           raise Exception, 'Cannot handle %s, a multi-Stokes image.' % filenames[i]
                else:
                    if stkslist[i]=='U':
                        #Uimage=filenames[i]
                        Uimage=varnames[i]
                    if stkslist[i]=='Q':
                        #Qimage=filenames[i]
                        Qimage=varnames[i]
            if len(Uimage)<1 or len(Qimage)<1:
                missing = []
                if len(Qimage)<1: missing.append('Q')
                if len(Uimage)<1: missing.append('U')
                raise Exception, 'Missing Stokes %s image(s)' % missing
            expr = 'pa(%s,%s)' % (Uimage,Qimage)
    elif mode=='poli':
        # calculate a polarization intensity image
        # if 3 files total pol intensity image
        # if 2 files given linear pol intensity image
        isQim = False
        isUim = False
        isVim = False
        isLPol = False
        isTPol = False
        stkslist=__check_stokes(filenames)
        if len(filenames) !=3 and len(filenames)!=2:
            raise Exception, 'Requires at two or three Stokes images'
        if len(filenames)==3:
            for i in range(len(stkslist)):
                if type(stkslist[i])==list:
                    raise Exception, 'Cannot handle %s, a multi-Stokes image.' % filenames[i]
                else:
                    if stkslist[i]=='Q':isQim=True
                    if stkslist[i]=='U':isUim=True
                    if stkslist[i]=='V':isVim=True
            if not (isUim and isQim and isVim):
                missing = []
                if not isQim: missing.append('Q')
                if not isUim: missing.append('U')
                if not isVim: missing.append('V')
                raise Exception, 'Missing Stokes %s image(s)' % missing
            #expr='sqrt("%s"*"%s"+"%s"*"%s"+"%s"*"%s"' % (filenames[0],filenames[0], filenames[1],filenames[1],filenames[2],filenames[2])
            expr='sqrt('+varnames[0]+'*'+varnames[0]\
                  +'+'+varnames[1]+'*'+varnames[1]\
                  +'+'+varnames[2]+'*'+varnames[2]
            isTPol = True
        elif len(filenames) ==2:
            for i in range(len(stkslist)):
                if type(stkslist[i])==list:
                    raise Exception, 'Cannot handle %s, a multi-Stokes image.' % filenames[i]
                else:
                    if stkslist[i]=='Q':isQim=True
                    if stkslist[i]=='U':isUim=True
            if not (isUim and isQim):
                missing = []
                if not isQim: missing.append('Q')
                if not isUim: missing.append('U')
                raise Exception, 'Missing Stokes %s image(s)' % missing
            #expr='sqrt("%s"*"%s"+"%s"*"%s"' % (filenames[0],filenames[0], filenames[1],filenames[1])
            expr='sqrt('+varnames[0]+'*'+varnames[0]\
                  +'+'+varnames[1]+'*'+varnames[1]
            isLPol = True
        
        sigsq=0

        if len(sigma)>0:
            qsigma=qa.quantity(sigma)
            if qa.getvalue(qsigma) >0:
                sigmaunit=qa.getunit(qsigma)
                try:
                    ia.open(filenames[0])
                    iunit=ia.brightnessunit()
                    ia.done()
                except:
                    raise Exception, 'Unable to get brightness unit from image file '+filenames[0]
                if sigmaunit!=iunit:
                    newsigma=qa.convert(qsigma,iunit)
                else:
                    newsigma=sigma
                    sigsq=(qa.getvalue(newsigma))**2
                expr+='-%s' % sigsq
            expr+=')'
        # reset stokes selection for pola and poli
    if (mode=='pola' or mode=='poli') and len(stokes)!=0:
        casalog.post( "Ignoring stokes parameters selection." ,'WARN' );
        stokes=''

    # PUT TRY BLOCK AROUND THIS PART
    #
    # NEED TO DO EXPRESSION FOR EXPR MODE

            
    # If the user didn't give any region or mask information
    # then just evaluated the expression with the filenames in it.

    if ( len(box)<1 and len(chans)<1 and len(stokes)<1 and len(region)<1 and len(mask)<1):
        expr = _immath_expr_from_varnames(expr, varnames, filenames)
        casalog.post( 'Will evaluate expression: '+expr, 'DEBUG1' )
        try:
            ia.imagecalc(pixels=expr, outfile=outfile)

            # need to modify stokes type of output image for pol. intensity image
            if ( mode =="poli" ):
            	print (inspect.getlineno(inspect.currentframe()))
            	ia.open(outfile)
                csys=ia.coordsys()
                if isTPol:
                    csys.setstokes('Ptotal')
                elif isLPol:
                    csys.setstokes('Plinear')
                ia.setcoordsys(csys.torecord())
                ia.done()
            ia.done()
            return True
        except Exception, error:
            try:
                ia.done()
            except:
                pass
            casalog.post( 'Unable to do mathematical expression: '\
                  +expr+'\n'+str(error), 'SEVERE' )
            return False

            
    # If we've made it here we need to apply masks or extract
    # regions from the images before doing the calculations first.
    # Warning if user has given a region file plus other region
    # selection information

    if ( len(region)>1 ):
        if ( len(box)>1 or len(chans)>1 or len(stokes)>1 ):
            casalog.post( "Ignoring region selection\ninformation"\
                          " the in box, chans, and stokes parameters."\
                          " Using region information\nin file: "\
                          + region, 'WARN' );
                
    # For each file in the list of files, creat a subimage
    # of it and store it.  We need to do this only if the user
    # specified a region or mask information.
    subImages=[]
    casalog.post( "SUBIMAGES: "+str(subImages), 'DEBUG2' )
    for i in range( 0,len( filenames ) ):
        casalog.post( 'Creating tmp image for file '+str(i), 'DEBUG2')
        # Get the region information
        # NOTE, we can't do this outside of the loop because
        #       imregion uses the coordsys of the image to
        #       create an appropriate region for that image.
        reg={}
        if ( len(region)>1 ):        
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
                    reg=rg.fromtabletorecord( filenames[i], region, False )
                else:
                    reg=rg.fromtabletorecord( reg_names[0], reg_names[1], False )
        else: 
            reg=imregion( filenames[i], chans, stokes, box, '', '' )
        casalog.post( 'Region record is: '+str(reg), 'DEBUG2' )
        if ( reg == {} ):
            casalog.post( 'Failed to create specified image region.', 'SEVERE' )
            return False

            
        # TODO see if there are issues when NO region given by user
        try:
            ia.open( filenames[i] )
            tmpFile=tmpFilePrefix+str(i)
            ia.subimage( region=reg, mask=mask, outfile=tmpFile )
            subImages.append( tmpFile )
            ia.done()
            casalog.post( 'Created temporary image '+tmpFile+' from '+\
                          filenames[i], 'DEBUG1' )
        except Exception, e:
            try:
                ia.done()
            except:
                pass
            casalog.post( 'Exception caught is: '+str(e), 'DEBUG2' )
            casalog.post( 'Unable to apply region to file: '\
                  +filenames[i]\
                  +'.\nUsed region: '+str(reg), 'DEBUG2' )
            #raise Exception, 'Unable to apply region to file: '\
            #      +filenames[i]
            casalog.post( 'Unable to apply region to file: '\
                          +filenames[i], 'SEVERE' )
            return False
    # Make sure no problems happened

    if ( len(filenames) != len(subImages) ) :
        #raise Exception, 'Unable to create subimages for all image names given'
        casalog.post( 'Unable to create subimages for all image names given',\
                      'SEVERE' )
        return False
    
    # Put the subimage names into the expression
    try:
            expr = _immath_expr_from_varnames(expr, varnames, subImages)
    except Exception, e:
        casalog.post( 'Unable to construct pixel expression aborting immath.'
                      'SEVERE' )
        casalog.post( 'Exception occured during expression construction\n'\
                     +str(e), 'DEBUG2' )
    casalog.post( 'Will evaluate expression of subimages: '+expr, 'DEBUG1' )

    try:
        # Do the calculation
        ia.imagecalc(pixels=expr, outfile=outfile )

                
        # modify stokes type for polarization intensity image
        if (  mode=="poli" ):                
            csys=retValue.coordsys()
            if isTPol:
                csys.setstokes('Ptotal')
            elif isLPol:
                csys.setstokes('Plinear')
            ia.setcoordsys(csys.torecord())

        #cleanup
        ia.done()                
        _immath_cleanup( tmpFilePrefix )
                    
        return True
    except Exception, error:
        try:
            ia.done()
            _immath_cleanup( tmpFilePrefix )
        except:
            pass
        casalog.post( "Exception caught was: "+str(error), 'DEBUG2')
        #raise Exception, 'Unable to evaluate math expression: '\
        #      +expr+' on file(s) '+str(filenames)
        casalog.post( 'Unable to evaluate math expression: '\
                      +expr+' on file(s) '+str(filenames), 'SEVERE' )
        return False
        

    # Remove any temporary files
    try: 
        _immath_cleanup( tmpFilePrefix )
    except:
        casalog.post( 'immath was unable to cleanup temporary files','SEVERE' )
        return False
    return True

def _immath_cleanup( filePrefix ):
    # Remove any old tmp files that may be left from
    # a previous run of immath
    fileList=os.listdir( os.getcwd() )
    for file in fileList:
        if ( file.startswith( filePrefix ) ):
            #recursivermdir( file )
            shutil.rmtree( file )


def _immath_parse( expr='' ):
        retValue=[]
        
        # Find out if the names are surrounded by single or double quotes
        quote=''
        if ( expr.find('"') > -1 ):
            quote='"'
        if ( expr.find("'") > -1 ):
            quote="'"

        current=expr;
        while( current.count(quote) > 1 ):
            start = current.index(quote)+1
            end   = current[start:].index(quote)+start
            if ( retValue.count( current[start:end] ) > 0 ) :
                # We already have this file name so we won't add it
                # to the list again.  This saves us work and disk space.
                current=current[end+1:]
                continue;
            
            retValue.append( current[start:end] )
            current=current[end+1:]

        return retValue

# check stokes type for the list of images
def __check_stokes(images):
        retValue=[]
        for fname in images:
            ia.open(fname)
            csys=ia.coordsys()
            stks=csys.stokes()
            ia.close()
            retValue.append(stks)
        return retValue
       
# it is important to sort the varnames in reverse order before doing
# the substitutions to assure the substitution set is performed correctly
# CAS-1678
def _immath_expr_from_varnames(expr, varnames, filenames):
        tmpfiles = {}
        for i in range(len(filenames)):
                tmpfiles[varnames[i]] = filenames[i]
        tmpvars = tmpfiles.keys()

        tmpvars.sort()
        tmpvars.reverse()

        for varname in tmpvars:
                expr = expr.replace(varname, '"' + tmpfiles[varname] + '"')
        return(expr)


