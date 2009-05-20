import os
from taskinit import *
from imregion import *

def immath(outfile=None,mode=None,expr=None,imagename=None,sigma=None,mask=None,region=None,box=None,chans=None,stokes=None):
#def immath(outfile=None,expr=None,mask=None,region=None,box=None,chans=None,stokes=None):
#def immath(outfile=None,expr=None):
	"""Performs mathematical operations on image files
        """

	# Remove any old tmp files that may be left from
	# a previous run of immath
	tmpFilePrefix='_immath_tmp'
	_immath_cleanup( tmpFilePrefix )

	
	# First check to see if the output file exists.  If it
	# does then we abort.  CASA doesn't allow files to be
	# over-written, just a policy.
	if ( len( outfile ) > 0 and os.path.exists( outfile ) ):
		raise Exception, 'Output file, '+outfile+\
		      ' exists. immath can not proceed, please\n'+\
		      'remove it or change the output file name.'
	elif ( len( outfile ) < 1 ):
		casalog.post( "The outfile paramter is empty, consequently the" \
                      +" resultant image will NOT be\nsaved on disk," \
                      +" but an image tool (ia) will be returned and if the" \
                      +" returned value\nis saved then you can used in" \
                      +" the same way the image tool (ia). can", 'WARN' )
	
	subImages=[]
	try :
	    # Find the list of filenames in the expression
	    # also do a quick check to see if all of the files
	    # exist
	    # TODO add a size check to make sure all images are the
	    # same size.
	    casalog.origin('immath')
            if mode=='evalexpr':
                filenames=_immath_parse( expr )
            else:
                filenames=imagename
            for i in range( len(filenames) ):
                if ( not os.path.exists(filenames[i]) ):
                    raise Exception, 'Image data set not found - please verify '+filenames[i]
            if mode=='spix':
            # calculate a spectral index distribution image
                if len(filenames) != 2:
                    raise Exception, 'Requires two images at different frequencies'
                expr = 'spectralindex("%s","%s")' % (filenames[0],filenames[1])
            elif mode=='pola':
            # calculate a polarization position angle image
                if len(filenames) < 2:
                    raise Exception, 'Requires separate Stokes Q and U images'
                else:
                    if len(filenames) >2:
                        casalog.post( "More than two images. Take first two and ignore the rest. " ,'WARN' );
                    stkslist=__check_stokes(filenames)
                    isQim = False
                    isUim = False
                    for i in range(2):
                       if type(stkslist[i])==list:
                           raise Exception, 'Cannot handle %s, a multi-Stokes image.' % filenames[i]
                       else:
                           if stkslist[i]=='U':
                               Uimage=filenames[i]
                               isUim = True
                           if stkslist[i]=='Q':
                               Qimage=filenames[i]
                               isQim = True
                    if not (isUim and isQim):
                        missing = []
                        if not isQim: missing.append('Q')
                        if not isUim: missing.append('U')
                        raise Exception, 'Missing Stokes %s image(s)' % missing
                expr = 'pa("%s","%s")' % (Uimage,Qimage)
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
                if len(filenames) ==3:
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
                    expr='sqrt("%s"*"%s"+"%s"*"%s"+"%s"*"%s"' % (filenames[0],filenames[0], filenames[1],filenames[1],filenames[2],filenames[2])
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
                    expr='sqrt("%s"*"%s"+"%s"*"%s"' % (filenames[0],filenames[0], filenames[1],filenames[1])
                    isLPol = True
                else:
                    raise Exception, 'Requires at least two Stokes images'
                sigsq=0
                if len(sigma)>0:
                    qsigma=qa.quantity(sigma)
                    if qa.getvalue(qsigma) >0:
                        sigmaunit=qa.getunit(qsigma)
                        ia.open(filenames[0])
                        iunit=ia.brightnessunit()
                        ia.close()
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



	    # If the user didn't give any region or mask information
	    # then just evaluated the expression with the filenames in it.
	    if ( len(box)<1 and len(chans)<1 and len(stokes)<1 and len(region)<1 and len(mask)<1):
		ia.imagecalc(pixels=expr, outfile=outfile, overwrite=True)
		ia.close()
                # need to modify stokes type of output image for pol. intensity image
                if mode=="poli":
                    ia.open(outfile)
                    csys=ia.coordsys()
                    if isTPol:
                        csys.setstokes('Ptotal')
                    elif isLPol:
                        csys.setstokes('Plinear')
                    ia.setcoordsys(csys.torecord())
                    ia.close()

		return True


	    # If we've made it here we need to apply masks or extract
	    # regions from the images before doing the calculations first.
	    # Warning if user has given a region file plus other region
	    # selection information
	    if ( len(region)>1 ):
		if ( len(box)<1 or len(chans)<1 or len(stokes)<1 ):
		    casalog.post( "Ignoring region selection\ninformation"\
				  " the in box, chans, and stokes parameters."\
				  " Using region information\nin file: "\
				  + region, 'WARN' );
	    
	    # For each file in the list of files, creat a subimage
	    # of it and store it.  We need to do this only if the user
	    # specified a region or mask information.
	    for i in range( len(filenames) ):
		# Get the region information
		# NOTE, we can't do this outside of the loop because
		#       imregion uses the coordsys of the image to
		#       create an appropriate region for that image.
		reg={}
		if ( len(region)>1 ):
		    reg=rg.fromfiletorecord( region );
		else:
		    reg=imregion( filenames[i], chans, stokes, box, '', '' )


		# TODO see if there are issues when NO region given by user
		ia.open( filenames[i] )
		tmpFile=tmpFilePrefix+str(i)
		ia.subimage( region=reg, mask=mask, outfile=tmpFile )
		subImages.append( tmpFile )
		ia.done()

	    # Make sure no problems happened
	    if ( len(filenames) != len(subImages) ) :
		raise Exception, 'Unable to create subimages for all image names given'
			 
	    # Replace the filenames in the expression with the subimage
	    # object, if we made subimages.  Otherwise don't change the
	    # expression
	    for i in range( len(filenames) ):
		expr=expr.replace(filenames[i], subImages[i])

	    # Do the calculation
	    retValue=ia.imagecalc(pixels=expr, outfile=outfile, overwrite=True)

	    #cleanup, including removeal of all tempfiles
	    ia.done()

            # modify stokes type for polarization intensity image
            if mode=="poli":
                ia.open(outfile)
                csys=ia.coordsys()
                if isTPol:
                    csys.setstokes('Ptotal')
                elif isLPol:
                    csys.setstokes('Plinear')
                ia.setcoordsys(csys.torecord())
                ia.close()

	    # Remove any temporary files
	    _immath_cleanup( tmpFilePrefix )

	except Exception, instance:
	    # cleanup, including removeal of all tempfiles
	    _immath_cleanup( tmpFilePrefix )
	    
	    casalog.post( str('*** Error ***')+str(instance), 'SEVERE' )
	    return False
	
	# 
        return True

def _immath_cleanup( filePrefix ):
    # Remove any old tmp files that may be left from
    # a previous run of immath
    fileList=os.listdir( os.getcwd() )
    for file in fileList:
	if ( file.startswith( filePrefix ) ):
	    recursivermdir( file )


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

