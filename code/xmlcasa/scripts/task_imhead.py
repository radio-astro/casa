########################################################################3
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
# <prerequisite>
# <ul>
#   <li> <linkto class="imregion.py:description">imregion</linkto> 
# </ul>
# </prerequisite>
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
# # The following code lists the keyword/value pairs found in
# # the header of the CASA image file ngc133.clean.image.  The information
# # is stored in Python variable hdr_info in a Python dictionary.
# # The information is also listed in the CASA logger.  The observation
# # date is #printed out from the hdr_info variable.
# hdr_info = imhead( 'ngc4826.clean.image', 'list' )
# #print "Observation Date: ", hdr_info['date-obs']
# 
# # The following exmple displays the CASA images history in the CASA logger.
# imhead( 'ngc4826.clean.image', 'history' )
#
# # The following example adds a new, user-defined keyword to the
# # CASA image ngc4826.clean.image
# imhead( 'ngc4826.clean.image', 'add', 'observer 1', 'Joe Smith'  )
# imhead( 'ngc4826.clean.image', 'add', 'observer 2', 'Daniel Boulanger'  )
#
# # The following example changes the name of the observer keyword,
# # OBSERVE, to ALMA
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

def imhead(imagename=None,mode=None,hdkey=None,hdvalue=None,hdtype=None,hdcomment=None):
    # Some debugging info.
    casalog.origin('imhead')
    casalog.post( "parameter imagename: "+imagename, 'DEBUG1' )
    casalog.post( "parameter mode:      "+mode, 'DEBUG1')
    casalog.post( "parameter hdkey:     "+hdkey, 'DEBUG1')
    casalog.post( "parameter hdvalue:   "+str(hdvalue), 'DEBUG1')
    casalog.post( "parameter hdtype:    "+hdtype, 'DEBUG1')
    casalog.post( "parameter hdcomment: "+hdcomment, 'DEBUG1')
    

    # Initialization stuff, If we are here the user has
    # specified an imagename and mode and we don't need to do
    # checks on them.  The CASA task infrustructure will
    # have done them.  But to make the script insensitive we'll
    # make everything lower case.
    mode      = mode.lower()
    hdkey     = hdkey.lower()
    #if ( isinstance( hdvalue, str ) ):
    #hdvalue   = hdvalue.lower()
    hdtype    = hdtype.lower()
    hdcomment = hdcomment.lower()
	
    #############################################################
    #                 HISTORY Mode
    #############################################################
    # History mode, List the history information for the
    # CASA image file.
    try:
	if (mode.startswith('his') ):
	    ia.open(imagename)
	    ia.history()
	    ia.done()
	    #print "History information is listed in logger"
	    return ''
    except Exception, instance:
	casalog.post( str('*** Error ***')+str(instance), 'SEVERE' )        
	return

    
    #############################################################
    #                 Summary Mode
    #############################################################
    # Summary mode, likely to become obsolete.  When
    # Image Analysis summary output looks like the task
    # output
    try:
	if (mode.startswith('sum') ):
	    ia.open(imagename)
	    ia.summary()
	    ia.done()
	    #print "Summary information is listed in logger"
	    return ''
    except Exception, instance:
	casalog.post( str('*** Error ***')+str(instance), 'SEVERE' )
	return

		

    #############################################################
    # Some variables used to keep track of header iformation
    # as follows:
    #    hd_keys:     Full list of header keys
    #    hd_values:   The values associated with the keywords
    #    hd_type:     
    #    hd_comment:
    #
    #    axes:        List of lists with axis information.
    #                 Order is dir1, dir2, spec, stokes
    #
    # NOTE: There is extra work done here for put, add, get,
    #       and del modes.  As we read the full header into
    #       local variables.  However, it makes the code
    #       cleaner to read.
    #
    #############################################################
    axes=[]
    try:
	axes=getimaxes(imagename)	
    except Exception, instance:
	casalog.post( str('*** Error ***')+str(instance), 'SEVERE' )
	return

    tbkeys   = [ 'imtype', 'object' ]
    csyskeys = [ 'date-obs', 'equinox', 'observer', 'projection',
		 'restfreq', 'reffreqtype', 'telescope' ]
    imkeys   = [ 'beammajor', 'beamminor', 'beampa', 'bunit', 'masks' ]
    crdkeys  = [ 'ctype', 'crpix', 'crval', 'cdelt', 'cunit']
    statkeys = [ 'datamin', 'datamax', 'minpos', 'minpixpos', 'maxpos', 'maxpixpos'  ]
	       
    hd_keys = tbkeys + csyskeys + imkeys + statkeys
    for key in crdkeys:
	for axis in range( len( axes ) ):
	    hd_keys.append( key + str(axis+1) )
	    
    hd_values    = {}
    hd_types     = {}
    hd_units     = {}
    hd_comments  = {}

    # For each header keyword initialize the dictionaries
    # as follows:
    #   value     "Not Known"
    #   type:     "Not Known"
    #   unit:     ""
    #   comment:  ""
    not_known = " Not Known "
    for hd_field in hd_keys:
	hd_values[hd_field] = not_known
	hd_types[hd_field] = not_known
	hd_units[hd_field] = ""
	hd_comments[hd_field] = ""
	
    ##print " HEADER KEYS: ", hd_keys
    ##print " HEADER VALUES: ", hd_values

    #############################################################
    #            Read the header contents.
    #
    # Needed for get and list mode only.
    #
    # TODO or consider.  Some of the coordsys() functions take
    # a format type.  It might save us a lot of time and trouble
    # if we retrieve the information as a string.  For example
    #   csys.refernecevalue( format='s')
    #############################################################
    

    ## Read the information from the TABLE keywords
    tbColumns = {}
    try:
	tb.open(imagename)
	tbColumns = tb.getcolkeywords()
	tb.close()
    except:
	casalog.post( str('*** Error *** Unable to open image file ')+imagename, 'SEVERE' )
	return

    # Now update our header dictionary.
    if ( tbColumns.has_key( 'imageinfo' ) and       
	 tbColumns['imageinfo'].has_key( 'objectname' ) ):
	hd_values['object']   =  tbColumns['imageinfo']['objectname']
	hd_types['object']    =  "string"
	hd_units['object']    =  ""
	hd_comments['object'] =  ""
	
    if ( tbColumns.has_key( 'imageinfo' ) and 
	 tbColumns['imageinfo'].has_key( 'imagetype' ) ):
	hd_values['imtype']   = tbColumns['imageinfo']['imagetype']
	hd_types['imtype']    =  "string"
	hd_units['imtype']    =  ""
	hd_comments['imtype'] =  ""

    # Use ia.summary to gather some of the header information
    #     ia.stats for the min and max
    #     ia.coordsys() to get a coordsys object to retrieve
    #     the coordinate information.
    #     Also find the units the data is stored in, for storing
    hd_dict   = {}
    stats     = {}
    csys      = None
    misc_info = {}
    data_unit = ""
    try :
	ia.open(imagename)
	hd_dict   = ia.summary(list=False)['header']
	stats     = ia.statistics(verbose=False, list=False)
	csys      = ia.coordsys()
	misc_info = ia.miscinfo()
	data_unit = ia.brightnessunit()
	ia.done()
    except Exception, instance:
	casalog.post( str('*** Error *** Unable to open image file ')+imagename, 'SEVERE' )
        casalog.post( str('              Python error: ')+str(instance),'SEVERE' )
	return

    
    # Add the Statistical information as follows:
    #    DATAMIN, DATAMAX, MINPIX, MINPIXPOS, MAXPOS, MAXPIXPOS

    # Store the MIN and MAX values.
    if ( stats.has_key('min' ) ):
	hd_values['datamin']      = stats['min'][0]
	hd_types['datamin']       = "double"
	hd_units['datamin']       = data_unit
	hd_comments['datamin']    = ""
    if ( stats.has_key('minposf' ) ):
	hd_values['minpos']      = stats['minposf']
	hd_types['minpos']       = "list"
	hd_units['minpos']       = ''
	hd_comments['minpos']    = ""
    if ( stats.has_key('minpos' ) ):
	hd_values['minpixpos']      = stats['minpos']
	hd_types['minpixpos']       = "list"
	hd_units['minpixpos']       = "pixels"
	hd_comments['minpixpos']    = ""		    	    

    if ( stats.has_key('msx' ) ):    
	hd_values['datamax']      = stats['max'][0]
	hd_types['datamax']       = "double"
	hd_units['datamax']       = data_unit
	hd_comments['datamax']    = ""
    if ( stats.has_key('maxposf' ) ):
	hd_values['maxpos']      = stats['maxposf']
	hd_types['maxpos']       = "list"
	hd_units['maxpos']       = ''
	hd_comments['maxpos']    = ""
    if ( stats.has_key('maxpos' ) ):
	hd_values['maxpixpos']      = stats['maxpos']
	hd_types['maxpixpos']       = "list"
	hd_units['maxpixpos']       = "pixels"
	hd_comments['maxpixpos']    = ""

	    
    # Find some of the general information from COORD SYS object:
    #     OBSERVER, TELESCOPE, RESTFREQUENCY, PROJECTION, EPOCH, and
    #     EQUINOX  
    try:
	hd_values['observer']   = csys.observer()
	hd_types['observer']    = "string"
	hd_units['observer']    = ""
	hd_comments['observer'] = ""
    except:
	no_op = 'noop'
	
    try:
	hd_values['telescope']   = csys.telescope()
	hd_types['telescope']    = "string"
	hd_units['telesceope']   = ""
	hd_comments['telescope'] = ""
    except:
	no_op = 'noop'

    try:
	tmp=csys.restfrequency()
	if ( tmp.has_key('value') ):
	    hd_values['restfreq']  = list( tmp['value'] )
	hd_types['restfreq']  = 'list'
	if ( tmp.has_key('unit') ):
	    hd_units['restfreq'] = tmp['unit']
	hd_comments['restfreq'] = ""
    except:
	no_op = 'noop'

    try:
        # Expected value is a dictionary with two keys:
        #    parameters: the parameters to the projection
        #    type:       Typeof projection (value we want)
	tmp=csys.projection()
	if ( tmp.has_key( 'type' ) ):	    
	    hd_values['projection']=tmp['type']
	hd_types['projection']='string'
        hd_units['projection']=''
	if ( tmp.has_key( 'parameters' ) ):
            hd_comments['projection']=str(list(tmp['parameters']))
    except:
	no_op = 'noop'

    try:
	hd_values['equinox']    = csys.referencecode(type='direction')
	hd_types['equinox']     = 'string'
	hd_units['equinox']     = ''
	hd_comments['equinox']  = ''
    except:
	no_op = 'noop'

    try:
	hd_values['reffreqtype']    = csys.referencecode(type='spectral')
	hd_types['reffreqtype']     = 'string'
	hd_units['reffreqtype']     = ''
	hd_comments['reffreqtype']  = ''
    except:
	no_op = 'noop'	    

    try:
	tmp=csys.epoch()
	if ( tmp.has_key( 'm0' ) and 
	     tmp['m0'].has_key( 'value' ) and
	     tmp['m0']['value'] > 0 ):
	    hd_values['date-obs'] = qa.time(tmp['m0'],form='ymd')
	    hd_units['data-obs']  ='ymd'
	hd_types['date-obs']      ='string'
	hd_comments['date-obs']   = ''
    except:
	no_op='noop'

    # Set some more of the standard header keys from the
    # header dictionary abtained from ia.summary().  The
    # fiels that will be set are:
    #     BEAMMAJOR, BEAMMINOR, BEAMPA, BUNIT, MASKS, CRVALx, CRPIXx,
    #     CDETLx,  CTYPEx, CUNITx
    if ( hd_dict.has_key( 'unit' ) ):
	hd_values['bunit']   = hd_dict['unit']
	hd_types['bunit']    = 'string'
	hd_units['bunit']    = ''
	hd_comments['bunit'] = ''

    if ( hd_dict.has_key( 'restoringbeam' ) and hd_dict['restoringbeam'].has_key( 'restoringbeam' ) ):
	tmp = hd_dict['restoringbeam']['restoringbeam']
	if ( tmp.has_key( 'major' ) ):
	    tmp2 = tmp['major']
	    if ( tmp2.has_key( 'value' ) ):
		hd_values['beammajor'] = tmp2['value']
	    if ( tmp2.has_key( 'unit' ) ):
		hd_units['beammajor'] = str(tmp2['unit'])
	    hd_types['beammajor']    = 'double'
	    hd_comments['beammajor'] = ''
	    
	if ( tmp.has_key( 'minor' ) ):
	    tmp2 = tmp['minor']
	    if ( tmp2.has_key( 'value' ) ):
		hd_values['beamminor'] = tmp2['value']
	    if ( tmp2.has_key( 'unit' ) ):
		hd_units['beamminor'] = str(tmp2['unit'])
	    hd_types['beamminor']    = 'double'
	    hd_comments['beamminor'] = ''

	if ( tmp.has_key( 'positionangle' ) ):
	    tmp2 = tmp['positionangle']
	    if( tmp2.has_key( 'value' ) ):
		hd_values['beampa'] = tmp2['value']
	    if ( tmp2.has_key( 'unit' ) ):
		hd_units['beampa'] = str(tmp2['unit'])
	    hd_types['beampa']    = 'double'
	    hd_comments['beampa'] = ''

    if ( hd_dict.has_key( 'masks' ) and len( hd_dict['masks'] ) > 0 ):
	hd_values['masks']   = hd_dict['masks'][0]
	hd_types['masks']    = 'string'
	hd_units['masks']    = ''
	hd_comments['masks'] = ''

	    
    # The COORDINATE keywords
    stokes = 'Not Known'
    if ( isinstance( axes[3][0], int ) ):
	stokes = csys.stokes()
    #print "GETTING COORD INFO"
    hd_coordtypes=[]
    for i in range(hd_dict['ndim']):
	hd_values['ctype'+str(i+1)]=    hd_dict['axisnames'][i]
	hd_values['crpix'+str(i+1)]=     hd_dict['refpix'][i]
	if ( (hd_dict['axisnames'][i]).lower() == 'stokes' ):
	    hd_values['crval'+str(i+1)]= stokes
	else:
	    hd_values['crval'+str(i+1)]=str(hd_dict['refval'][i])

	hd_values['cdelt'+str(i+1)]=str(hd_dict['incr'][i])
	hd_values['cunit'+str(i+1)]=hd_dict['axisunits'][i]
    #print "HD_VALUES ctype3: ", hd_values['ctype3']

    # Add the miscellaneous info/keywords
    # TODO add some some smarts to figure out the type
    #      of the keyword, and maybe unit.
    for new_key in misc_info.keys() :
	hd_values[new_key]   = misc_info[new_key]
	hd_types[new_key]    = ''
	hd_units[new_key]    = ''
	hd_comments[new_key] = ''

    # Find all of the *user defined* keywords, Python sets
    # support differences but lists don't this is why we use
    # sets here.
    user_keys          = list( set(hd_values.keys()) - set(hd_keys ) )
    casalog.post( 'List of user defined keywords found are: '+str(user_keys), 'DEBUG2' )
    casalog.post( str( hd_values ), 'DEBUG2' )
    casalog.post( str( hd_types ), 'DEBUG2' )
    casalog.post( str( hd_units ), 'DEBUG2' )
    casalog.post( str( hd_comments ), 'DEBUG2' )
	
    #print "DONE GETTING HDR INFO"
    #############################################################
    #                     FITS MODE
    #
    # Just #print out all the information we just gathered.
    #############################################################
    if (mode=='fits'):
	casalog.post( 'Sorry this mode is not available yet.', 'WARN' )
	return
	
		    
    #############################################################
    #                     List MODE
    #
    # Just #print out all the information we just gathered.
    #############################################################
    if (mode=='list'):
	try:
	    casalog.post( 'Available header items to modify:' )
	    casalog.post( 'General --' )
	    #casalog.post( '' )

	    user_key_count = 0
	    for field in hd_keys+user_keys:
		if ( field == 'ctype1' ):
		    casalog.post( 'axes --' )
		elif ( field == 'crpix1' ):
		    casalog.post( 'crpix --' )
		elif ( field == 'crval1' ):
		    casalog.post( 'crval --' )			    
		elif ( field == 'cdelt1' ):
		    casalog.post( 'cdelt --' )
		elif ( field == 'cunit1' ):
		    casalog.post( 'units --' )
			
		elif ( field.startswith( 'cdelt' ) or field.startswith('crval' ) ):
		    index=int(field[5:])
		    printVal=str(hd_values[field])+str(hd_values['cunit'+str(index)])
		    if ( hd_values['cunit'+str(index)]== 'rad' ):
			printVal=qa.formxxx(printVal, 'dms')+str("deg.min.sec")
		    casalog.post( str( '        -- ' )+field+str(': ')+printVal )
		elif ( field.startswith( 'cunit' ) ):
		    # Output the displayed unit, not the stored unit value.
		    outUnit=hd_values[field]
		    if ( outUnit == 'rad' ):
			outUnit='deg.min.sec'
		    casalog.post( str( '        -- ' )+field+str(': ')+outUnit )
		elif ( hd_keys.count( field ) < 1 ):
		    # This is a user defined keyword
		    if ( user_key_count < 1 ):
			casalog.post( 'User Defined --' )
		    user_key_count = user_key_count + 1
		    casalog.post( str( '        -- ' )+field+str(': ')+str(hd_values[field]))
		else:
		    casalog.post( str( '        -- ' )+field+str(': ')+str(hd_values[field]))
	    if ( csys!=None ):
		csys.done()
		del csys
	    #print "List information displayed in the logger"
	    return hd_values
	except Exception, instance:
	    casalog.post( str('*** Error ***')+str(instance), 'SEVERE' )
            casalog.post( str('              Python error: ')+str(instance),'SEVERE' )
	    return

	
    #############################################################
    #                     add MODE
    #
    # Add a new keyword to the image table.
    #
    #############################################################
    key_list = hd_keys + user_keys
    casalog.post( 'All of the header keys: '+str(key_list), 'DEBUG2' )
    
    if ( mode=='add' and ( key_list.count( hdkey ) > 0 and str( hd_values[ hdkey ] ) != not_known ) ):
	# The Keyword is already in the header, we are
	# switching to put mode.
	casalog.post( hdkey+str( ' is already in the header, switching to "put" mode.'), 'WARN' )
	mode = 'put'

    #print "MODE IS: ", mode
    if ( mode == 'add' ):
	if ( hdkey in tbkeys ):
            #print "ADDING A TABLE KEY"
	    try:	
		# We are dealing with having to add to the table columns.
		tb.open(imagename, nomodify=False)
                #print "COLUMNS KEYS: ", tbColumns.keys()
                #print "image info KEYS: ", tbColumns['imageinfo'].keys()
		#keys = tb.getkeywords()
                #print "HDKEY: ", hdkey
                #print "HDVALUE: ", hdvalue
		if ( hdkey=='imtype' ):
		    tbColumns['imageinfo']['imagetype']=hdvalue
                    #print "ADDED IMTYPE: ", hdvalue, "  ", tbColumns['imageinfo']['imagetype']
		else:
		    tbColumns['imageinfo']['objectname']=hdvalue
                    #print "ADDED IMTYPE: ", hdvalue, "  ", tbColumns['imageinfo']['imagetype']
		tb.putkeywords( tbColumns )
		tb.close()
		casalog.post( hdkey+" keyword has been ADDED to "+imagename+"'s header", "NORMAL" )
		return
            except Exception, instance:
		casalog.post( '*** Error *** Unable to add keyword '+hdkey+' to image file '+imagename, 'SEVERE' )
                casalog.post( str('              Python error: ')+str(instance),'SEVERE' )
		return
            
	elif( hdkey in statkeys ):
	    # This is a statistical value, these are generated by
	    # ia.statistics and not actually in the header.
	    casalog.post( hdkey+str( ' is not part of the header information, but generated by the image\nstatistics function. Therefore there are no values to place in the header.' ), 'WARN' )
	    return

	elif ( hdkey in crdkeys ):
	    # We need to use a coordinate system function to add
	    # this keyword, in fact we need to add a coordinate
	    # axis.
	    #
	    # TODO
	    return

	elif ( hdkey  in csyskeys or hdkey  in imkeys ):
	    # We need to use a coordinate system function to add
	    # this keyword.  Since these keywords are known by
	    # by the coordsys tool, it is sufficent to change
	    # to "put" mode and update the field.
            #print "HERE IN ADD"
            #print "VALUE IS: ", hdvalue
	    if ( hdkey == 'masks' ):
		casalog.post( 'imhead does not add masks to images, this is to complex a task\n use the "makemask" task to add masks', 'WARN' )
		return
	    casalog.post( hdkey+' switching to "put" mode to add the '+hdkey+' to the header.', 'WARN' )
	    mode = 'put'

	else:
	    # Handle a User defined keyword
	    try:
                # First step is to make sure we have the given
                # value in the expected data type, if there is
                # one.  The default is the string stored in hd_types
                # if there isn't a value then str is used.
		value = hdvalue
                #print 'hdtype IS: ', type(hdtype)
                #print 'hdtype IS: ', hdtype
                keytype='str'
                if ( type(hdtype)!=None and len( hdtype ) > 0 ):
                    keytype=hdtype
                elif ( hd_types.has_key( hdkey) and len( hd_types[hdkey] ) > 0 ):
                    keytype=hd_types[hdkey]
                elif ( isinstance( hdvalue, str ) ):
                    keytype = 'str'
                elif ( isinstance( hdvalue, int ) ):
                    keytype = 'int'
                elif ( isinstance( hdvalue, float ) ):
                    keytype = 'float'
                elif ( isinstance( hdvalue, complex ) ):
                    keytype = 'complex'                    
                elif ( isinstance( hdvalue, list ) ):                    
                    keytype = 'list'
                elif ( isinstance( hdvalue, dict ) ):
                    keytype = 'dict'
                    
                if ( keytype=='double' ):
                    keytype='float'
                if ( keytype=='string' ):
                    keytype='str'
                #print "KEY TYPE: ", keytype
                
                #print "VALUE TYPE IS: ", type(value)
                #TODO -- Add a check to see if we really need
                #        to do the type conversion.
                #print "eval( ",keytype, "(",str( value ),")"
                if ( isinstance( value, str ) ):
                    value = eval( keytype+'("'+value+'")' )
                else:
                    value = eval( keytype+'("'+str(value)+'")' )
                
                #print "VALUE IS: ", value
		misc_info[hdkey] = value
		ia.open(imagename)
		ia.setmiscinfo( misc_info )
		ia.done()
		casalog.post( hdkey+" keyword has been ADDED to "+imagename+"'s header with value "+str(value), "NORMAL" )		
		return
	    except Exception, instance:
		casalog.post( '*** Error *** Unable to add keyword '+hdkey+' to image file '+imagename, 'SEVERE' )
                casalog.post( str('              Python error: ')+str(instance),'SEVERE' )                
		return	    

	
    #############################################################
    #                     Del MODE
    #
    # Remove a keyword to the image table.
    #############################################################
    if ( mode=='del' and ( key_list.count( hdkey ) < 0 \
			   or  hd_values[ hdkey ] == not_known ) ):
	# The Keyword is already absent there is nothing to do.
	casalog.post( hdkey+str( ' is already absent in the header, therfore nothing to delete.', 'WARN' ) )
	return
    if ( mode == 'del' ):    
	if ( hdkey in tbkeys ):
	    try:
		# We are dealing with having to add to the table columns.
		tb.open(imagename, nomodify=False)
		key=''
		if ( hdkey=='object' ):
                    tbColumns['imageinfo'].has_key('objectname' ) 
		    tbColumns['imageinfo'].pop( 'objectname' )
		else:
                    #print "REMOVING IMAGE TYPE FROM HDR"
                    tbColumns['imageinfo'].has_key('imagetype' ) 
		    tbColumns['imageinfo'].pop( 'imagetype' )
                #print "Keys in TABLES: ", tbColumns.keys()
                #print "KEYS in IMAGE INFO: ", tbColumns['imageinfo'].keys()
		tb.putcolkeywords(value=tbColumns)
		tb.flush()
		tb.done()
		casalog.post( hdkey+" keyword has been DELETED from "+imagename+"'s header", "NORMAL" )
		return
	    except Exception, instance:
		casalog.post( '*** Error *** Unable delete keyword '+hdkey+' from image file '+imagename, 'SEVERE' )
                casalog.post( str('              Python error: ')+str(instance),'SEVERE' )
		return

	elif( hdkey in statkeys ):
	    # This is a statistical value, these are generated by
	    # ia.statistics and not actually in the header.
	    casalog.post( hdkey+str( ' is not part of the header information, but generated by the image\nstatistics function. Therefore there are no values to be removed from the header.' ), 'WARN' )
	    return

	elif ( hdkey in imkeys or hdkey  in csyskeys ):
	    # We need to use an image analysis function to add this
	    # keyword.
	    try:
		if ( hdkey.startswith( 'beam' ) ):
                    # We don't actually remove the beam information, we
                    # instead set it to the default information.
		    ia.open( imagename )
		    ia.setrestoringbeam( remove = True )
		    ia.done()
		    casalog.post( 'The restoring beam has been removed from '+imagename, 'NORMAL' )
		    return
		elif ( hdkey.startswith( 'mask' ) ):
		    ia.open( imagename )
		    ia.maskhandler( op='delete', name=hdvalue )
		    ia.done()
		    casalog.post( 'Mask '+hdvalue+' has been removed from '+imagename, 'NORMAL' )
		else:
		    casalog.post( hdkey+str( ' can not be removed, setting value to ')+not_known+str(' instead.'), 'WARN' );
		    mode='put'
		    # TODO set hdvalue to a sensible default?
		    #hdvalue = not_known
		    hdvalue = ''
	    except Exception, instance:
                casalog.post( '*** Error *** Unable delete keyword '+hdkey+' from image file '+imagename, 'SEVERE' )
                casalog.post( str('              Python error: ')+str(instance),'SEVERE' )                    
                return


	elif ( hdkey in crdkeys ):
	    # We need to use a coordinate system function to add
	    # this keyword, in fact we need to add a coordinat
	    # axis.
	    #
	    # TODO
	    no_op = 'noop'
	    return
		
	else:
	    try:	    
		# Handle a User defined keyword
		value = hdvalue
                if ( misc_info.has_key( hdkey ) ):
                    junk = misc_info.pop( hdkey )
		ia.open(imagename)
		ia.setmiscinfo( misc_info )
		ia.done()
		casalog.post( hdkey+" keyword has been ADDED to "+imagename+"'s header with value "+str(value), "NORMAL" )		
		return
	    except Exception, instance:
		casalog.post( '*** Error *** Unable delete keyword '+hdkey+' from image file '+imagename, 'SEVERE' )
                casalog.post( str('              Python error: ')+str(instance),'SEVERE' )
		return
	    

    #############################################################
    #                     get MODE
    #
    # Just #print out the requested information. Note that we
    # acquired all of the header details, which is not very
    # efficient for get, but it does make the code cleaner!
    # Getting the min/max values can take some time to
    # calculate, and it might be worthwhile doing this step 
    # only if we are getting them.
    #############################################################
    #print "IN KEYLIST: ", key_list.count( hdkey )
    #print "IN VALUE LIST: ", hd_values.has_key( hdkey )
    if ( mode=='get' and ( key_list.count( hdkey ) < 1 or str( hd_values[ hdkey ] ) == not_known ) ):
	# The Keyword is not in the header, nothing to GET!!!
	casalog.post( hdkey+str( ' is NOT in the header unable to "get" its value.'), 'SEVERE'  )
	return
    
    if ( mode=='get' ):
	retValue = ''
	msg      = ''
	#print "TYPE:  ", hd_types[hdkey]
	#print "VALUE: ", hd_values[hdkey]
	if ( len( str( hd_types[hdkey] ) ) < 1 or str(hd_types[hdkey]) == not_known ):
	    retValue = str( hd_values[hdkey] )
        elif( str(  hd_types[hdkey] ) == 'list' ):
	    retValue = hd_values[hdkey] 
	else:
	    keytype = str( hd_types[hdkey] )
	    if ( keytype=='double' ):
		keytype='float'
	    if ( keytype=='string' ):
		keytype='str'		
	    #print keytype+'("'+str( hd_values[hdkey] )+'")'
	    retValue = eval( keytype+'("'+str( hd_values[hdkey] )+'")' )
            
	retValue={ 'unit': hd_units[hdkey], 'value': retValue }
        #print "RETURNING: ", retValue
	msg = 'Value of Header Key '+hdkey+' is:'+str(retValue)
	if ( len( hd_units ) > 0 ):
	    msg = msg + ' '+str( hd_units[hdkey] )

	casalog.post( msg, 'NORMAL' )
	return retValue
	

    #############################################################
    #                     put MODE
    #
    # If we made it here the user is putting something into
    # the header.
    #############################################################
    casalog.post( "Putting (changing): "+hdkey+" to  "+str(hdvalue), 'DEBUG2' )
    if ( hdkey in tbkeys ):
	try:
	    # 'object' and 'imtype' are only the ones that we use the
	    # table tool to add/del/put/get/...
	    # for, may need to expand this to allow other keywords.
	    #print 'Mary had a little Lamb'
	    tb.open(imagename,nomodify=False)
	    #print 'Little Lamb -- ', hdkey
	    if ( hdkey == 'object' ):
		tbColumns['imageinfo']['objectname'] = hdvalue
	    elif ( hdkey == 'imtype' ):
		tbColumns['imageinfo']['imagetype'] = hdvalue
	    #print 'Little Lamb'
	    tb.putcolkeywords(value=tbColumns)
	    #print "Who's fleece was as white as snow."
	    tb.flush()
	    tb.done()
	    casalog.post( hdkey+" keyword has been UPDATED in "+imagename+"'s header", "NORMAL" )
	    return
	except Except, instance:
	    casalog.post( '*** Error *** Unable update keyword '+hdkey+' from image file '+imagename, 'SEVERE' )
	    casalog.post( '             Python exception is: Unable update keyword '+hdkey+' from image file '+imagename, 'SEVERE' )
	    return
	
    elif( hdkey in statkeys ):	
	# This is a statistical value, these are generated by
	# ia.statistics and not actually in the header.
	casalog.post( hdkey+str( ' is not part of the header information, but generated by the image\nstatistics function. Therefore, '+hdkey+' can not be changed.' ), 'WARN' )
	return hdvalue
    
    elif ( hdkey in imkeys ):
	# Header values that can be changed through the image analysis too.
	try:
	    # These are field that can be set with the image
	    # analysis tool
	    ia.open(imagename)		
	    if ( hdkey == 'bunit' ):
		ia.setbrightnessunit(hdvalue)

	    elif ( hdkey == 'masks' ):
		# We only set the default mask to the first mask in
		# the list.
		# TODO delete any masks that aren't in the list.
		if ( type( hdvalue, list ) ):
		    ia.maskhanderler( op='set', name=hdvalue[0] )
		else:
		    ia.maskhanderler( op='set', name=hdvalue )		

	    elif ( hdkey.startswith( 'beam' ) ):
                # Get orignal values.  Note that since there are
                # expected header fields we assume that they exist
                # in our dictionary of header values.
                #
                # TODO IF NOT A LIST BUT A STRING CHECK FOR
                # UNITS
                #print "HERE IN BEAM PUTTING"
                #if ( hd_values.has_key( 'beammajor' ) ):
                    #print "MAJOR ", hd_values['beammajor']
                #else:
                    #print "MAJOR NOT KNOWN"

                major={'unit':'arcsec', 'value':1}
                if ( str( hd_values['beammajor'] ) != not_known ):
                    major = { 'value': hd_values['beammajor'], 'unit': hd_units['beammajor'] }
                #print "MAJOR", major
                
                minor={'unit':'arcsec', 'value':1}
                if ( str( hd_values['beamminor'] ) != not_known ):
                    minor = { 'value': hd_values['beamminor'], 'unit': hd_units['beamminor'] }

                pa={'unit':'deg', 'value':0}
                if ( str( hd_values['beampa'] ) != not_known ):
                    pa = { 'value': hd_values['beampa'], 'unit': hd_units['beampa'] }
                #print "HDVALUE: ", hdvalue    
                if ( hdkey=='beammajor' ) :
                    major = _imhead_strip_units( hdvalue, hd_values['beammajor'], hd_units['beammajor'] )
                    major['value'] = float( major['value'] )
                    #print "MAJOR After Parsing: ", major
                elif ( hdkey=='beamminor' ) :
                    minor = _imhead_strip_units( hdvalue, hd_values['beamminor'], hd_units['beamminor'] )
                    minor['value'] = float( minor['value'] )
                elif ( hdkey=='beampa' ) :
                    pa = _imhead_strip_units( hdvalue, hd_values['beampa'], hd_units['beampa'] )
                    pa['value'] = float( pa['value'] )
                else:
                    casalog.post( '*** Error *** Unrecognized beam keyword '+hdkey, 'SEVERE' )
                    return
                #print "MAJOR: ", major
                #print "MINOR: ", minor
                #print "PA:    ", pa
                ia.setrestoringbeam( beam = {'major': major, 'minor':minor, 'positionangle': pa}, log=True )
                
            ia.done()
	    casalog.post( hdkey+" keyword has been UPDATED to "+imagename+"'s header", "NORMAL" )						
	    return hdvalue
	except Exception, instance:
	    casalog.post( '*** Error *** Unable update keyword '+hdkey+' from image file '+imagename+'\n'+str(instance), 'SEVERE' )
            casalog.post( str('              Python error: ')+str(instance),'SEVERE' )
	    return 

    elif (hdkey  in csyskeys ):
	# Header values that can be changed through the coordsys tool
	try:
	    ia.open(imagename)
	    csys=ia.coordsys()
	    if ( hdkey=='date-obs' ):
		if ( hdvalue=='Not Known' or hdvalue=='UNKNOWN' ):
		    hdvalue=0
		tmp=me.epoch( v0=str(hdvalue) )
		csys.setepoch( tmp )
	    elif ( hdkey=='equinox' ):
		csys.setreferencecode( type='direction', value=str(hdvalue) )
	    elif ( hdkey=='observer' ):
		csys.setobserver( str( hdvalue ) )
	    elif( hdkey=='projection' ):
		csys.setprojection( hdvalue )
	    elif ( hdkey=='telescope' ):
		csys.settelescope( str(hdvalue) )
	    elif ( hdkey=='restfreq' ):
                # TODO handle a list of rest frequencies
		#print "NEW REST FREQUENCY: ", hdvalue, "  ", type(hdvalue)

                #print "HDVALUE: ", hdvalue
                if ( isinstance( hdvalue, list ) ):
                    no_op='noop'  # Nothing to change here
                elif ( not isinstance( hdvalue, str ) ):
                    hdvalue = str( hdvalue )
                else:
                    hdvalue = hdvalue.split( ',' )
                # Loop through the list of values, adding each
                # one separately.

                #print "LIST: ", hdvalue
                if ( isinstance( hdvalue, str ) ):
                    num_freq = 1
                else:
                    num_freq = len( hdvalue )
                #print "NUMBER OF FREQ: ", num_freq
                
                for i in range( num_freq ):
                    if ( not isinstance( hdvalue[i], str ) ):
                        current = str( hdvalue[i] )
                    else:
                        current = hdvalue[i]
                    #print "CURRENT: ", current

                    # Remove any units from the string, if
                    # there are any.
                    parsed_input = \
                          _imhead_strip_units( current, 0.0, hd_units[hdkey] )
                    parsed_input['value'] = float( parsed_input['value'] )
                    
                    #print "Setting rest frequency to: ",parsed_input
                    if ( i < 1 ):
                        csys.setrestfrequency(parsed_input)
                    else:
                        csys.setrestfrequency(parsed_input, append=True)
	    else:
		casalog.post( str('*** Error *** Unrecognized hdkey ')+str(hdkey), 'SEVERE' )
		return
			
	    # Now store the values!
            #print "SETTING CSYS"
	    ia.setcoordsys(csys=csys.torecord())
            #print "DONE"
	    ia.done()
	    csys.done()
	    del csys
	    casalog.post( hdkey+" keyword has been UPDATED to "+imagename+"'s header", "NORMAL" )
	    return hdvalue	    
	except Exception, instance:
	    if ( ia.isopen() ):
		ia.done()
	    if ( csys!=None ):
		csys.done()
		del csys	    
	    casalog.post( '*** Error *** Unable to UPDATE keyword '+hdkey+' in image file '+imagename, 'SEVERE' )
            casalog.post( str('              Python error: ')+str(instance),'SEVERE' )
            return 
	
    elif ( hdkey[0:5] in crdkeys ):
        #print "PROCESSING COORDINATE KEYWORD"
	# Coordinate axes information, changed through the coordsys tool
	try:
	    # Open the file and obtain a coordsys tool
	    ia.open(imagename)
	    csys = ia.coordsys()
		
	    # Find which axis is being modified from the field name
	    # (hdkey). Note, that internally we use 0-based
	    # indexes but the user input will be 1-based, we
	    # convert by subtracting 1. Also note that all of our
	    # fields are 5 character long.
	    fieldRoot=hdkey[0:5]
	    index=-1 
	    if ( len( hdkey ) > 5 ):
		index=(int(hdkey[5:])) - 1

	    # cdelt, crpix, and cval all return a record, the values
	    # are store in and *array* in the 'numeric' field.  But
	    # we need to input a list, this makes things a bit messy!
	    if (hdkey.startswith('cdelt')):
		# We need 
		if ( index < 0 ):
		    csys.setincrement( hdvalue )
		else:
		    # We grab the reference values as quantities, 
		    # which allows users to specify a wider variety
		    # of units.  We are going to make an assumption
		    # that if there are no units at the end that the
		    # user gave the value in radians, but checking if
		    # the last character of the string is a number.
		    values=csys.increment( format='q')
		    units =csys.units()
			
		    if ( isinstance( hdvalue, str ) ):
			hdvalue=hdvalue.strip()
			
			# Remove any '.' and ':' from the string
			# if all that remains is a number then the
			# user didn't give any units.
			tmpVal=hdvalue.replace('.', '' )
			tmpVal=tmpVal.replace(':', '')
			if ( tmpVal.isdigit() ):
			    if ( len(units[index]) > 0 ):
				hdvalue=hdvalue+units[index]
		    elif ( isinstance( hdvalue, int )
			   or isinstance( hdvalue, float ) ):
			if ( len(units[index]) > 0 ):
			    hdvalue = qa.convert( str(hdvalue)+units[index], units[index] )
			else:
			    hdvalue = str(hdvalue)
			    
		    # reference axies are 1-based, but python
		    # lists are 0-based -- yikes!
		    values['quantity']['*'+str(index+1)]=qa.convert( hdvalue, units[index] )
		    csys.setincrement( values )
			
	    elif (hdkey.startswith('crpix')):
		if ( index < 0 ):
		    csys.setreferencepixel(hdvalue)
		else:
		    values=csys.referencepixel()['numeric']
		    values=[values[0],values[1],values[2],values[3]]
		    values[index]=float(hdvalue)
		    csys.setreferencepixel(values)
			
	    elif (hdkey.startswith('crval')):
		# Because setreferencevalue has issues 
		# with setting the stokes value, if
		# the stokes axis has changed we use the
		# setstokes function instead.
		if ( isinstance( axes[3][0], int ) ):
		    stokesIndex = axes[3][0]
		    origvalue = csys.referencevalue('s')['string'][stokesIndex]
		else:
		    origvalue = 'Not Known'

		if ( index < 0 ):
		    newvalue = hdvalue[stokesIndex]
		    if ( origvalue != newvalue ):
			hdvalue[stokesIndex]=origvalue
			newvalue = hdvalue[stokesIndex]
			csys.setreferencevalue(hdvalue)
			csys.setstokes( newvalue )
		else:
		    # We grab the reference values as quantities, 
		    # which allows users to specify a wider variety
		    # of units.  We are going to make an assumption
		    # that if there are no units at the end that the
		    # user gave the value in radians, but checking if
		    # the last character of the string is a number.
		    values=csys.referencevalue( format='q')
		    units =csys.units()

		    if ( isinstance( hdvalue, str ) ):
			hdvalue=hdvalue.strip()
			
			# Remove any '.' and ':' from the string
			# if all that remains is a number then the
			# user didn't give any units.
			tmpVal=hdvalue.replace('.', '' )
			tmpVal=tmpVal.replace(':', '')
			if ( tmpVal.isdigit() ):
			    if ( index < len(units) and len(units[index]) > 0 ):
				hdvalue=hdvalue+units[index]
			    else:
				hdvalue=hdvalue
		    elif ( isinstance( hdvalue, int )
			   or isinstance( hdvalue, float ) ):
			if ( index < len(units) and len(units[index]) > 0 ):
			    hdvalue = qa.convert( str(hdvalue)+units[index], units[index] )
			else:
			    hdvalue = str(hdvalue)

		    # reference axies are 1-based, but python
		    # lists are 0-based -- yikes!
		    # 
		    # We also need to deal with stokes changes
		    # very carefully, and use csys.setstokes()
		    #
		    # TODO We need to deal with adding stokes
		    # directional, and spectral values with
		    # the appropriate csys.set?? methods.
		    if ( index != axes[3][0] ):
			if ( index < len(units) and len(units[index]) > 0 ):
			    values['quantity']['*'+str(index+1)]=qa.convert( hdvalue, units[index] )
			else:
			    values['quantity']['*'+str(index+1)]=hdvalue
			csys.setreferencevalue(values)
		    else:
			origStokes = csys.referencevalue('s')
			origStokes = origStokes['string'][index]
			if ( origStokes != hdvalue ):
			    csys.setstokes( hdvalue )
			    
            elif (hdkey.startswith('ctype')):
                #print "INDEX: ", index
		if ( index < 0 ):
		    csys.setnames(hdvalue)
		else:
		    values=csys.names()
		    values[index]=str(hdvalue)
                    #print "SETTING COORD NAMES TO: ", values
		    csys.setnames(values)
                    
	    elif (hdkey.startswith('cunit')):
		if ( index < 0 ):
		    csys.setunits(hdvalue)
		else:
		    values=csys.units()
		    values[index]=str(hdvalue)
		    csys.setunits(values)

	    # Store the changed values.
	    if ( csys != None ):
		ia.setcoordsys(csys=csys.torecord())
		csys.done()
		del csys
		
	    ia.done()
	    casalog.post( hdkey+" keyword has been UPDATED in "+imagename+"'s header", "NORMAL" )
	    return hdvalue
	except Exception, instance:
	    casalog.post( str('*** Error ***')+str(instance), 'SEVERE' )
	    return
	
    else:
	# User defined keywords, changed with the image analsys too's
	# "miscinfo" function.
	# TODO Add units and comments
	try:
            # First step is to make sure we have the given
            # value in the expected data type, if there is
            # one.  The default is the string stored in hd_types
            # if there isn't a value then str is used.
            value = hdvalue
            keytype='str'
            if ( type(hdtype)!=None and len( hdtype ) > 0 ):
                keytype=hdtype
            elif ( hd_types.has_key( hdkey) and len( hd_types[hdkey] ) > 0 ):
                keytype=hd_types[hdkey]
            elif ( isinstance( hdvalue, str ) ):
                keytype = 'str'
            elif ( isinstance( hdvalue, int ) ):
                keytype = 'int'
            elif ( isinstance( hdvalue, float ) ):
                keytype = 'float'
            elif ( isinstance( hdvalue, complex ) ):
                keytype = 'complex'                    
            elif ( isinstance( hdvalue, list ) ):                    
                keytype = 'list'
            elif ( isinstance( hdvalue, dict ) ):
                keytype = 'dict'
                    
            if ( keytype=='double' ):
                keytype='float'
            if ( keytype=='string' ):
                keytype='str'
            
            #TODO -- Add a check to see if we really need
            #        to do the type conversion.
            #print "eval( ",keytype, "(",str( value ),")"
            if ( isinstance( value, str ) ):
                value = eval( keytype+'("'+value+'")' )
            else:
                value = eval( keytype+'("'+str(value)+'")' )

	    misc_info[hdkey] = value
	    ia.open(imagename)
	    ia.setmiscinfo( misc_info )
	    ia.done()
	    casalog.post( hdkey+" keyword has been ADDED to "+imagename+"'s header with value "+str(value), "NORMAL" )		
	    return
	except Exception, instance:
	    casalog.post( '*** Error *** Unable to change keyword '+hdkey+" in "+imagename+" to "+str(hdvalue), "SEVERE" )
            casalog.post( str('              Python error: ')+str(instance),'SEVERE' )
	    return
    
#
# NAME:        _imhead_strip_units
#
# AUTHOR:      S. Jaeger
#
# PURPOSE:     To take as input a string which contains a numeric value
#              followed by a unit and separate them.
#
# DESCRIPTION: Take the input string, find the the index of the
#              last numerical character in the string.  We assume
#              this is the point where the number ends and the unit
#              begins and split the input string at this place.
#
#              If there is no value found, then the default value
#              is used.  Similarly for the units.
#
# RETURN:      dictionary of the form:
#                  { 'value': numberFound, 'unit', 'unitfound' }

def _imhead_strip_units( input_number, default_value=0.0, default_unit="" ):
    # Find the place where the units start and the number
    # ends.
    #print "IN STRIP UNITS -- ", input_number

    if ( isinstance( input_number, dict ) ):
        if ( input_number.has_key( 'value' ) ):
            value = input_number[ 'value' ]
        else:
            value = default_value


        if ( input_number.has_key( 'unit' ) ):
            unit = input_number[ 'unit' ]
        else:
            unit = default_unit            
    elif ( isinstance( input_number, str ) ):    
        lastNumber = -1
        for j in range( len(input_number) ):
            if (input_number[j].isdigit() ):
                lastNumber=j
            
        #print "index of last number: ", lastNumber
        if ( lastNumber >= len( input_number )-1 ):
            unit = default_unit
        else:
            unit = input_number[lastNumber+1:]

        if ( lastNumber < 0 ):
            value = default_value
        else:
            value = input_number[0:lastNumber+1]
    else:
        raise Exception, "Unable to parse units from numerical value in input "+str(input_number)
    
    #print "RETURNING: ", { 'value': value, 'unit': unit}
    return { 'value': value, 'unit': unit}
