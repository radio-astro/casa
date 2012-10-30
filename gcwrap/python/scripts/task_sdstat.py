import os
import sys
from taskinit import *
from get_user import get_user

import asap as sd
from asap import _to_list
from asap.scantable import is_scantable
import pylab as pl
import sdutil

def sdstat(infile, antenna, fluxunit, telescopeparm, specunit, restfreq, frame, doppler, scanlist, field, iflist, pollist, masklist, invertmask, interactive, outfile, format, overwrite):


        casalog.origin('sdstat')

        ###
        ### Now the actual task code
        ###
        restorer = None
        verbsave=sd.rcParams['verbose']
        try:
            sdutil.assert_infile_exists(infile)

            #load the data without averaging
            s = sd.scantable(infile,average=False,antenna=antenna)

            # collect data to restore
            restorer = sdutil.scantable_restore_factory(s,
                                                        infile,
                                                        fluxunit,
                                                        specunit,
                                                        frame,
                                                        doppler,
                                                        restfreq)

            #Apply the selection
            sel = sdutil.get_selector(scanlist, iflist, pollist,
                                      field)
            s.set_selection(sel)
            del sel

            stmp = sdutil.set_fluxunit(s, fluxunit, telescopeparm, False)

            if stmp:
                    # Restore flux unit in original table before deleting
                    restorer.restore()
                    del restorer
                    restorer = None
                    del s
                    s = stmp
                    del stmp


            # set default spectral axis unit
            sdutil.set_spectral_unit(s, specunit)

            # set restfrequency
            sdutil.set_restfreq(s, restfreq)

            # reset frame and doppler if needed
            sdutil.set_freqframe(s, frame)

            sdutil.set_doppler(s, doppler)

            # If outfile is set, sd.rcParams['verbose'] must be True
            if ( len(outfile) > 0 ):
                    if ( not os.path.exists(outfile) or overwrite ):
                            sd.rcParams['verbose']=True

            (retValue,resultstats) = calcstatistics(s, specunit, fluxunit, masklist, invertmask, interactive, format)

            # Output to terminal if outfile is not empty
            savestatistics(resultstats, outfile, overwrite)

            return retValue
    
            # DONE
            
        except Exception, instance:
                sdutil.process_exception(instance)
                raise Exception, instance
                return
        finally:
                # restore rcParams
                sd.rcParams['verbose']=verbsave
                try:
                        # Restore header information in the table
                        if restorer is not None:
                                restorer.restore()
                                del restorer

                        # Final clean up
                        del s
                except:
                        pass
                casalog.post('')

def calcstatistics(s, specunit, fluxunit, masklist, invertmask, interactive, stformat):
    # Warning for multi-IF data
    if len(s.getifnos()) > 1:
        #print '\nWarning - The scantable contains multiple IF data.'
        #print '          Note the same mask(s) are applied to all IFs based on CHANNELS.'
        #print '          Baseline ranges may be incorrect for all but IF=%d.\n' % (s.getif(0))
        casalog.post( 'The scantable contains multiple IF data.', priority='WARN' )
        casalog.post( 'Note the same mask(s) are applied to all IFs based on CHANNELS.', priority='WARN' )
        casalog.post( 'Baseline ranges may be incorrect for all but IF=%d.\n' % (s.getif(0)), priority='WARN' )


    # Get mask
    msk = get_mask(s, interactive, masklist, invertmask)

    # Get statistics values
    formstr = get_formatter(stformat)
    statsdict = get_stats(s, msk, formstr)

    resultstats = statsdict.pop('statslist')

    # Get unit labels
    (abclbl,intlbl,xunit,intunit) = get_unit_labels(s, specunit, fluxunit)

    # Equivalent width and integrated intensities
    (eqw,integratef) = get_eqw_and_integf(s, statsdict)

    if sd.rcParams['verbose']:
            # Print equivalent width
            out = get_statstext(s, 'eqw', abclbl, eqw, formstr)
            resultstats += out

            # Print integrated flux
            outp = get_statstext(s, 'Integrated intensity', intlbl, integratef, formstr)
            resultstats += outp

            # to logger
            casalog.post(out[:-2]+outp)

    # reshape statsdict for return
    for k in ['min','max']:
        statsdict['%s_abscissa'%(k)] = qa.quantity(statsdict.pop('%s_abc'%(k)),xunit)
    for (k,v,u) in [('eqw',eqw,xunit),('totint',integratef,intunit)]:
        statsdict[k] = qa.quantity(v,u)

    return (statsdict,resultstats)


def savestatistics(stats, outfile, overwrite):
    if ( len(outfile) > 0 ):
        if ( not os.path.exists(outfile) or overwrite ):
            #sys.stdout=open( outfile,'w' )
            f=open(outfile,'w')
            f.write(stats)
            f.close()
            #sd.rcParams['verbose']=verbsave
            #bbb=True
        else:
            #print '\nFile '+outfile+' already exists.\nStatistics results are not written into the file.\n'
            casalog.post( 'File '+outfile+' already exists.\nStatistics results are not written into the file.', priority = 'WARN' )


def get_formatter(format):
    format=format.replace(' ','')
    formstr=format
    if len(format)==0:
        casalog.post("Invalid format string. Using the default 3.3f.")
        formstr='3.3f'
    return formstr

def get_mask(s, interactive, masklist, invertmask):
    msk = None
    if interactive:
        # Interactive masking
        msk = sdutil.get_interactive_mask(s, masklist, False)
        msks = s.get_masklist(msk)
        if len(msks) < 1:
            #print 'No channel is selected. Exit without calculation.'
            raise Exception, 'No channel is selected. Exit without calculation.'
            #return
        lbl=s.get_unit()
        #print 'final mask list ('+lbl+') =',msks
        casalog.post( 'final mask list ('+lbl+') = '+str(msks) )
        
        del msks

    # set the mask region
    elif ( len(masklist) > 0):
        msk=s.create_mask(masklist,invert=invertmask)
        msks=s.get_masklist(msk)
        if len(msks) < 1:
            del msk, msks
            #print 'Selected mask lists are out of range. Exit without calculation.'
            raise Exception, 'Selected mask lists are out of range. Exit without calculation.'
            #return
        del msks
    else:
        # Full region
        #print 'Using full region'
        casalog.post( 'Using full region' )
    return msk

def get_stats(s, msk=None, formstr='3.3f'):
    usr = get_user()
    tmpfile = '/tmp/tmp_'+usr+'_casapy_asap_scantable_stats'
    #statslist = []
    statslist = ''
    d = {}
    statsname = ['max', 'min', 'max_abc', 'min_abc',
                 'sum', 'mean', 'median', 'rms',
                 'stddev']
    for name in statsname:
        v = s.stats(name,msk,formstr)
        d[name] = list(v) if len(v) > 1 else v[0]
        if sd.rcParams['verbose']:
            statslist += get_text_from_file(tmpfile)
    d['statslist'] = statslist
    return d

def get_unit_labels(s, specunit, fluxunit):
    if specunit != '': abclbl = specunit
    else: abclbl = s.get_unit()
    if fluxunit != '': ordlbl = fluxunit
    else: ordlbl = s.get_fluxunit()
    intlbl = ordlbl +' * '+abclbl
    # Check units
    if abclbl == 'channel' and not qa.check(abclbl):
        qa.define('channel','1 _')
    if qa.check(abclbl):
        xunit = abclbl
    else:
        #print "Undefined unit: '"+abclbl+"'...ignored"
        casalog.post( "Undefined unit: '"+abclbl+"'...ignored", priority = 'WARN' )
        xunit = '_'
    if qa.check(ordlbl):
        intunit = ordlbl+'.'+abclbl
    else:
        #print "Undifined unit: '"+ordlbl+"'...ignored"
        casalog.post( "Undifined unit: '"+ordlbl+"'...ignored", priority = 'WARN' )
        intunit = '_.'+abclbl
    return (abclbl,intlbl,xunit,intunit)

def get_eqw_and_integf(s, statsdict):
    eqw = None
    integratef = None
    if isinstance(statsdict['max'],list):
        # User selected multiple scans,ifs
        ns = len(statsdict['max'])
        eqw=[]
        integratef=[]
        for i in range(ns):
            #Get bin width
            abcissa, lbl = s.get_abcissa(rowno=i)
            dabc=abs(abcissa[-1] - abcissa[0])/float(len(abcissa)-1)
            # Construct equivalent width (=sum/max)
            eqw = eqw + [get_eqw(statsdict['max'][i],
                                 statsdict['min'][i],
                                 statsdict['sum'][i],
                                 dabc)]
            # Construct integrated flux
            integratef = integratef + [get_integf(statsdict['sum'][i], dabc)]

    else:
        # Single scantable only
        abcissa, lbl = s.get_abcissa(rowno=0)
        dabc=abs(abcissa[-1] - abcissa[0])/float(len(abcissa)-1)
        
        # Construct equivalent width (=sum/max)
        eqw = get_eqw(statsdict['max'],
                      statsdict['min'],
                      statsdict['sum'],
                      dabc)

        # Construct integrated flux
        integratef = get_integf(statsdict['sum'], dabc)
    return (eqw,integratef)

def get_eqw(maxl, minl, suml, dabc):
    eqw = 0.0
    if ( maxl != 0.0 or minl != 0.0 ):
        if ( abs(maxl) >= abs(minl) ):
            eqw = suml/maxl*dabc
        else:
            eqw = suml/minl*dabc
    return eqw
    
def get_integf(suml, dabc):
    return suml * dabc

def get_statstext(s, title, label, val, formstr):
    sep = "--------------------------------------------------"
    head = string.join([sep,string.join([" ","%s ["%(title),label,"]"]," "),sep],'\n')
    tail = ''
    out = head + '\n'
    if isinstance(val,list):
        ns = len(val)
        for i in xrange(ns):
            out += get_statstr(s, i, val[i], formstr, sep)
    else:
        out += get_statstr(s, 0, val, formstr, sep)
    out += '\n%s'%(tail)
    return out

def get_statstr(s, i, val, formstr, separator):
    out = ''
    out += 'Scan[%d] (%s) ' % (s.getscan(i), s._getsourcename(i))
    out += 'Time[%s]:\n' % (s._gettime(i))
    if s.nbeam(-1) > 1: out +=  ' Beam[%d] ' % (s.getbeam(i))
    if s.nif(-1) > 1: out +=  ' IF[%d] ' % (s.getif(i))
    if s.npol(-1) > 1: out +=  ' Pol[%d] ' % (s.getpol(i))
    out += ('= %'+formstr) % (val) + '\n'
    out +=  "%s\n "%(separator)
    return out

def get_text_from_file(filename):
    f = open(filename,'r')
    rlines = f.readlines()
    f.close()
    return string.join(rlines,'')

