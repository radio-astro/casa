import os
import sys
import re
from taskinit import *

import asap as sd
from asap import _to_list
from asap.scantable import is_scantable
import pylab as pl
import sdutil

def sdmath(expr, varlist, antenna, fluxunit, telescopeparm, specunit, frame, doppler, scanlist, field, iflist, pollist, outfile, outform, overwrite):

        casalog.origin('sdmath')


        ###
        ### Now the actual task code
        ###
        try:
            if expr=='':
                raise Exception, 'expr is undefined'

            sdutil.assert_outfile_canoverwrite_or_nonexistent(outfile, outform, overwrite)
        
            # Force return new table (WORKAROUND for bugs in STMath)
            oldInsitu = sd.rcParams['insitu']
            if sd.rcParams['scantable.storage'] == 'disk':
                    sd.rcParams['insitu'] = False
            
            # do operation
            tmpout = dooperation(expr, varlist, antenna, scanlist, iflist, pollist, field, fluxunit, telescopeparm)

##             # vars() or locals() must be called to update local symbol table to remove
##             # unwanted scantable instances from that symbol table 
##             vars()

            # set specunit to output data
            sdutil.set_spectral_unit(tmpout,specunit)

            # set frame to output data
            sdutil.set_freqframe(tmpout,frame)

            # set doppler to output data
            sdutil.set_doppler(tmpout,doppler)

            sdutil.save(tmpout, outfile, outform, overwrite)

        except Exception, instance:
                sdutil.process_exception(instance)
                raise Exception, instance
                return
        finally:
                # Put back insitu (WORKAROUND for bugs in STMath)
                try:
                        sd.rcParams['insitu'] = oldInsitu
                except:
                        pass
                casalog.post('')


def dooperation(expr, varlist, antenna, scanlist, iflist, pollist, field, fluxunit, telescopeparm):
    for key in varlist.keys():
            regex = re.compile( key )
            if isinstance( varlist[key], str ):
                    expr = regex.sub( '\"%s\"' % varlist[key], expr )
            else:
                    expr = regex.sub( "varlist['%s']" % key, expr )

    filenames = _sdmath_parse(expr)
    # default flux unit
    fluxunit_now = fluxunit

    # A scantable selection
    sel = sdutil.get_selector(in_scans=scanlist,
                              in_ifs=iflist,
                              in_pols=pollist,
                              in_field=field)

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
          thisscan=sd.scantable(filenames[i],average=False,antenna=antenna)

          # Apply the selection
          thisscan.set_selection(sel)
          if fluxunit_now == '':
             fluxunit_now=thisscan.get_fluxunit()
          # copy scantable since convert_flux overwrites spectral data
          if sd.rcParams['scantable.storage']=='disk':
                  casalog.post('copy data to keep original one')
                  s = thisscan.copy()
          else:
                  s = thisscan
          sdutil.set_fluxunit(s, fluxunit, telescopeparm,True)
          scandic[skey] = s
          del s, thisscan
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

    del scandic
    
    # set flux unit
    if tmpout.get_fluxunit() != fluxunit_now:
            tmpout.set_fluxunit(fluxunit_now)

    return tmpout


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

