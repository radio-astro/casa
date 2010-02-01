import os
import sys
import re
from taskinit import *

import asap as sd
import pylab as pl


def sdmath(expr, varlist, fluxunit, telescopeparm, specunit, frame, doppler, scanlist, field, iflist, pollist, outfile, outform, overwrite):

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

            sel = sd.selector()

            #Select scans
            if (type(scanlist) == list ):
              scans = scanlist
            else:
              scans = [scanlist]

            if ( len(scans) > 0 ):
              sel.set_scans(scans)
            #Select source
            if (field != '' ):
              sel.set_name(field)

            #Select IFs
            if (type(iflist) == list):
              ifs = iflist
            else:
              ifs = [iflist]

            if (len(ifs) > 0 ):
              sel.set_ifs(ifs)

            #Select Polarizations
            if (type(pollist) == list):
              pols = pollist
            else:
              pols = [pollist]

            if(len(pols) > 0 ):
              sel.set_polarisations(pols)

            scandic={} 
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
                  scandic[skey]=sd.scantable(filenames[i])
                  # apply set_fluxunit, selection
                  # if fluxunit is not given, use first spetral data's flux unit 
                  if fluxunit=='':
                     fluxunit=scandic[skey].get_fluxunit()
                  scandic[skey].set_fluxunit(fluxunit)
                  if frame!='':
                     scandic[skey].set_freqframe(frame)
                  if ( doppler != '' ):
                     ddoppler=doppler.upper()
                     scandic[skey].set_doppler(ddoppler)
                  try:
                     #Apply the selection
                     scandic[skey].set_selection(sel)
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
