import os
import sys
import re
from taskinit import *

import asap as sd
import pylab as pl


def sdmath(expr, fluxunit, telescopeparm, specunit, frame, doppler, scanlist, field, iflist, pollist, outfile, outform, overwrite):

        casalog.origin('sdmath')


        ###
        ### Now the actual task code
        ###
        try:
            if expr=='':
                raise Exception, 'expr is undefined'
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
                   print '***Error***',instance
                   print 'No output written.'
                   return

               
               #regex=re.compile('[\',\"]')
               regex=re.compile('[\',\"]%s[\',\"]' % filenames[i])
               #expr=regex.sub('',expr)
               expr=regex.sub("scandic['%s']" % skey ,expr)
            expr="tmpout="+expr 
            exec(expr)
            tmpout.save(outfile, outform, overwrite) 

        except Exception, instance:
                print '***Error***',instance
                return
        finally:
                casalog.post('')


def _sdmath_parse( expr='' ):
        
        import re
        retValue=[]
        #p=re.compile(r'[\',\"]\w+[\',\"]')
        p=re.compile(r'[\',\"]\w+[\.,\-,\w+]*[\',\"]')
        fnames=p.findall(expr)
        p=re.compile('[\',\"]')
        for fname in fnames:
           fnamestr=p.sub('',fname)
           retValue.append(fnamestr)

        return retValue
