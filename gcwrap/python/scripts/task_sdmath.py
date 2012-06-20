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

            # default flux unit
            fluxunit_now = fluxunit

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
                  try:
                     #Apply the selection
                     thisscan.set_selection(sel)
                  except Exception, instance:
                     #print '***Error***',instance
                     #print 'No output written.'
                     casalog.post( str(instance), priority = 'ERROR' )
                     casalog.post( 'No output written.', priority = 'ERROR' )
                     return
                  if fluxunit_now == '':
                     fluxunit_now=thisscan.get_fluxunit()
                  scandic[skey] = _convert_flux( thisscan,
                                                 fluxunit_now,
                                                 telescopeparm )
                  del thisscan
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
            # vars() or locals() must be called to update local symbol table to remove
            # unwanted scantable instances from that symbol table 
            vars()

            # set specunit to output data
            if len(specunit) > 0:
                    tmpout.set_unit(specunit)

            # set frame to output data
            if len(frame) > 0:
                    tmpout.set_freqframe(frame)

            # set doppler to output data
            if len(doppler) > 0:
                    tmpout.set_doppler(doppler)

            # set flux unit
            if tmpout.get_fluxunit() != fluxunit_now:
                    tmpout.set_fluxunit(fluxunit_now)

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
                        # Final clean up
                        del scandic
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

def _convert_flux( s, flunit, tparam ):
        fl = s.get_fluxunit()

        if fl == flunit or tparam.upper() == 'FIX':
                return s

        # to make sure the operation is done by insitu=False
        insitu = sd.rcParams['insitu']
        sd.rc('',insitu=False)
        
        # copy scantable since convert_flux overwrites spectral data
        if sd.rcParams['scantable.storage'] == 'disk':
                casalog.post('copy data to keep original one')
                out = s.copy()
        else:
                out = s

        antennaname = out.get_antennaname()

        # convert_flux
        if ( type(tparam) == list ):
                # User input telescope params
                if ( len(tparam) > 1 ):
                        D = tparam[0]
                        eta = tparam[1]
                        #print "Use phys.diam D = %5.1f m" % (D)
                        #print "Use ap.eff. eta = %5.3f " % (eta)
                        casalog.post( "Use phys.diam D = %5.1f m" % (D) )
                        casalog.post( "Use ap.eff. eta = %5.3f " % (eta) )
                        out.convert_flux(eta=eta,d=D,insitu=True)
                elif ( len(tparam) > 0 ):
                        jypk = tparam[0]
                        #print "Use gain = %6.4f Jy/K " % (jypk)
                        casalog.post( "Use gain = %6.4f Jy/K " % (jypk) )
                        out.convert_flux(jyperk=jypk,insitu=True)
                else:
                        #print "Empty telescope list"
                        casalog.post( "Empty telescope list" )

        elif ( tparam=='' ):
                if ( antennaname == 'GBT'):
                        # needs eventually to be in ASAP source code
                        #print "Convert fluxunit to "+flunit
                        casalog.post( "Convert fluxunit to "+flunit )
                        # THIS IS THE CHEESY PART
                        # Calculate ap.eff eta at rest freq
                        # Use Ruze law
                        #   eta=eta_0*exp(-(4pi*eps/lambda)**2)
                        # with
                        #print "Using GBT parameters"
                        casalog.post( "Using GBT parameters" )
                        eps = 0.390  # mm
                        eta_0 = 0.71 # at infinite wavelength
                        # Ideally would use a freq in center of
                        # band, but rest freq is what I have
                        rf = out.get_restfreqs()[0][0]*1.0e-9 # GHz
                        eta = eta_0*pl.exp(-0.001757*(eps*rf)**2)
                        #print "Calculated ap.eff. eta = %5.3f " % (eta)
                        #print "At rest frequency %5.3f GHz" % (rf)
                        casalog.post( "Calculated ap.eff. eta = %5.3f " % (eta) )
                        casalog.post( "At rest frequency %5.3f GHz" % (rf) )
                        D = 104.9 # 100m x 110m
                        #print "Assume phys.diam D = %5.1f m" % (D)
                        casalog.post( "Assume phys.diam D = %5.1f m" % (D) )
                        out.convert_flux(eta=eta,d=D,insitu=True)
                        
                        #print "Successfully converted fluxunit to "+flunit
                        casalog.post( "Successfully converted fluxunit to "+flunit )
                elif ( antennaname in ['AT','ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43', 'CEDUNA', 'HOBART']):
                        out.convert_flux(insitu=True)
                        
                else:
                        # Unknown telescope type
                        #print "Unknown telescope - cannot convert"
                        casalog.post( "Unknown telescope - cannot convert", priority = 'WARN' )
        sd.rc('',insitu=insitu)
        return out
