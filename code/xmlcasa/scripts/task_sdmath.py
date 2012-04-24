import os
import sys
import re
from taskinit import *

import asap as sd
from asap import _to_list
from asap.scantable import is_scantable
import pylab as pl


def sdmath(expr, varlist, antenna, fluxunit, telescopeparm, specunit, frame, doppler, scanlist, field, iflist, pollist, outfile, outform, overwrite):

        casalog.origin('sdmath')


        ###
        ### Now the actual task code
        ###
        try:
            if expr=='':
                raise Exception, 'expr is undefined'
        
            for key in varlist.keys():
                    regex = re.compile( key )
                    if isinstance( varlist[key], str ):
                            expr = regex.sub( '\"%s\"' % varlist[key], expr )
                    else:
                            expr = regex.sub( "varlist['%s']" % key, expr )

            filenames = _sdmath_parse(expr)

            # Force return new table (WORKAROUND for bugs in STMath)
            oldInsitu = sd.rcParams['insitu']
            if sd.rcParams['scantable.storage'] == 'disk':
                    sd.rcParams['insitu'] = False
            
            # A scantable selection
            # Scan selection
            scans = _to_list(scanlist,int) or []

            # IF selection
            ifs = _to_list(iflist,int) or []

            # Select polarizations
            pols = _to_list(pollist,int) or []

            # Actual selection
            sel = sd.selector(scans=scans, ifs=ifs, pols=pols)

            # Select source names
            if ( field != '' ):
                    sel.set_name(field)
                    # NOTE: currently can only select one
                    # set of names this way, will probably
                    # need to do a set_query eventually


            scandic={}
            # Lists of header info to restore
            restorebase = sd.rcParams['scantable.storage'] == 'disk' and \
                          ( (fluxunit != '') or (specunit != '') or \
                            (frame != '') or (doppler != '') )
            restorescans = []
            spunitlist = []
            flunitlist = []
            framelist = []
            dopplerlist = []

            for i in range(len(filenames)):
               skey='s'+str(i)
               isfactor = None
               # file type check
               if os.path.isdir( filenames[i] ):
                  isfactor = False
               else:
                  f = open( filenames[i] )
                  line = f.readline().rstrip('\n')
                  f.close()
                  del f
                  try:
                     isfactor = True
                     vtmp = float( line[0] )
                     del line
                  except ValueError, e:
                     isfactor = False
                     del line
               if not isfactor:
                  # scantable
                  scandic[skey]=sd.scantable(filenames[i],average=False,antenna=antenna)
                  thisscan = scandic[skey]
                  # back up header info
                  if restorebase and is_scantable(filenames[i]):
                          restorescans.append(thisscan)
                          spunitlist.append(thisscan.get_unit())
                          flunitlist.append(thisscan.get_fluxunit())
                          coord = thisscan._getcoordinfo()
                          framelist.append(coord[1])
                          dopplerlist.append(coord[2])
                          del coord
                  # apply set_fluxunit, selection
                  # if fluxunit is not given, use first spetral data's flux unit 
                  if fluxunit=='':
                     fluxunit=thisscan.get_fluxunit()
                  thisscan.set_fluxunit(fluxunit)
                  if frame!='':
                     thisscan.set_freqframe(frame)
                  if ( doppler != '' ):
                     ddoppler=doppler.upper()
                     thisscan.set_doppler(ddoppler)
                  try:
                     #Apply the selection
                     thisscan.set_selection(sel)
                  except Exception, instance:
                     #print '***Error***',instance
                     #print 'No output written.'
                     casalog.post( str(instance), priority = 'ERROR' )
                     casalog.post( 'No output written.', priority = 'ERROR' )
                     return
               elif isfactor:
                  # variable
                  f = open( filenames[i] )
                  lines = f.readlines()
                  f.close()
                  del f
                  for lin in range( len(lines) ):
                     lines[lin] = lines[lin].rstrip('\n')
                     lines[lin] = lines[lin].split()
                     for ljn in range( len(lines[lin]) ):
                        lines[lin][ljn] = float( lines[lin][ljn] )
                  scandic[skey] = lines
               #regex=re.compile('[\',\"]')
               regex=re.compile('[\',\"]%s[\',\"]' % filenames[i])
               #expr=regex.sub('',expr)
               expr=regex.sub("scandic['%s']" % skey ,expr)
            expr="tmpout="+expr
            exec(expr)

            # Restore header information in the table
            for i in range(len(restorescans)):
                    thisscan = restorescans[i]
                    thisscan.set_fluxunit(flunitlist[i])
                    thisscan.set_unit(spunitlist[i])
                    thisscan.set_doppler(dopplerlist[i])
                    thisscan.set_freqframe(framelist[i])
            casalog.post( "Restored header information of input tables" )
            # Final clean up
            del restorescans, scandic, thisscan
            # vars() or locals() must be called to update local symbol table to remove
            # unwanted scantable instances from that symbol table 
            vars()

            outform=outform.upper()
            if (outform == 'MS'):
               outform = 'MS2'
            elif (outform==''):
               outform = 'ASAP'

            tmpout.save(outfile, outform, overwrite) 

        except Exception, instance:
                #print '***Error***',instance
                casalog.post( str(instance), priority = 'ERROR' )
                return
        finally:
                try:
                        # Restore header information in the table
                        for i in range(len(restorescans)):
                                        thisscan = restorescans[i]
                                        thisscan.set_fluxunit(flunitlist[i])
                                        thisscan.set_unit(spunitlist[i])
                                        thisscan.set_doppler(dopplerlist[i])
                                        thisscan.set_freqframe(framelist[i])
                        casalog.post( "Restored header information of input tables" )
                        # Final clean up
                        del restorescans, scandic
                except:
                        pass
                # Put back insitu (WORKAROUND for bugs in STMath)
                try:
                        sd.rcParams['insitu'] = oldInsitu
                except:
                        pass
                casalog.post('')


def _sdmath_parse( expr='' ):
        
        import re
        retValue=[]
        #p=re.compile(r'[\',\"]\w+[\',\"]')
        #p=re.compile(r'[\',\"]\w+[\.,\-,\w+]*[\',\"]')
        p=re.compile(r'(?!varlist\[)[\',\"]\w+[\.,\-,/,\w+]*[\',\"](?!\])')
        fnames=p.findall(expr)
        p=re.compile('[\',\"]')
        for fname in fnames:
           fnamestr=p.sub('',fname)
           retValue.append(fnamestr)

        return retValue
