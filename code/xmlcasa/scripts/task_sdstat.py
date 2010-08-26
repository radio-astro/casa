import os
import sys
from taskinit import *
#from casa_in_py import get_user
from get_user import get_user

import asap as sd
import pylab as pl

def sdstat(sdfile, antenna, fluxunit, telescopeparm, specunit, frame, doppler, scanlist, field, iflist, pollist, masklist, invertmask, interactive, statfile, format, overwrite):


        casalog.origin('sdstat')


        ###
        ### Now the actual task code
        ###
        retValue={}
        ## if os.environ.has_key( 'USER' ):
        ##     usr = os.environ['USER']
        ## else:
        ##     import commands
        ##     usr = commands.getoutput( 'whoami' )
        usr = get_user()
        tmpfile = '/tmp/tmp_'+usr+'_casapy_asap_scantable_stats'
        resultstats = []
        try:
            if sdfile=='':
                raise Exception, 'sdfile is undefined'

            filename = os.path.expandvars(sdfile)
            filename = os.path.expanduser(filename)
            if not os.path.exists(filename):
                s = "File '%s' not found." % (filename)
                raise Exception, s

            #load the data without averaging
            s=sd.scantable(sdfile,average=False,antenna=antenna)

            # get telescope name
            #'ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43' (Tid), 'CEDUNA', and 'HOBART'
            antennaname = s.get_antennaname()

            # determine current fluxunit
            fluxunit_now = s.get_fluxunit()
            if ( antennaname == 'GBT'):
                    if (fluxunit_now == ''):
                           #print "no fluxunit in the data. Set to Kelvin."
                           casalog.post( "no fluxunit in the data. Set to Kelvin." )
                           s.set_fluxunit('K')
                           fluxunit_now = s.get_fluxunit()

            #print "Current fluxunit = "+fluxunit_now
            casalog.post( "Current fluxunit = "+fluxunit_now )

            # set flux unit string (be more permissive than ASAP)
            if ( fluxunit == 'k' ):
                    fluxunit = 'K'
            elif ( fluxunit == 'JY' or fluxunit == 'jy' ):
                    fluxunit = 'Jy'

            # fix the fluxunit if necessary
            # No way to query scantable to find what telescope is
            # so rely on user input

            if ( telescopeparm == 'FIX' or telescopeparm == 'fix' ):
                            if ( fluxunit != '' ):
                                    if ( fluxunit == fluxunit_now ):
                                            #print "No need to change default fluxunits"
                                            casalog.post( "No need to change default fluxunits" )
                                    else:
                                            s.set_fluxunit(fluxunit)
                                            #print "Reset default fluxunit to "+fluxunit
                                            casalog.post( "Reset default fluxunit to "+fluxunit )
                                            fluxunit_now = s.get_fluxunit()
                            else:
                                    #print "Warning - no fluxunit for set_fluxunit"
                                    casalog.post( "no fluxunit for set_fluxunit", priority = 'WARN' )

            elif ( fluxunit=='' or fluxunit==fluxunit_now ):
                    #print "No need to convert fluxunits"
                    casalog.post( "No need to convert fluxunits" )
            elif ( type(telescopeparm) == list ):
                    # User input telescope params
                    if ( len(telescopeparm) > 1 ):
                            D = telescopeparm[0]
                            eta = telescopeparm[1]
                            #print "Use phys.diam D = %5.1f m" % (D)
                            #print "Use ap.eff. eta = %5.3f " % (eta)
                            casalog.post( "Use phys.diam D = %5.1f m" % (D) )
                            casalog.post( "Use ap.eff. eta = %5.3f " % (eta) )
                            s.convert_flux(eta=eta,d=D)
                    elif ( len(telescopeparm) > 0 ):
                            jypk = telescopeparm[0]
                            #print "Use gain = %6.4f Jy/K " % (jypk)
                            casalog.post( "Use gain = %6.4f Jy/K " % (jypk) )
                            s.convert_flux(jyperk=jypk)
                    else:
                            #print "Empty telescopeparm list"
                            casalog.post( "Empty telescopeparm list" )

            elif (telescopeparm==''):
                    if ( antennaname == 'GBT'):
                            # needs eventually to be in ASAP source code
                            #print "Convert fluxunit to "+fluxunit
                            casalog.post( "Convert fluxunit to "+fluxunit )
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
                            rf = s.get_restfreqs()[0][0]*1.0e-9 # GHz
                            eta = eta_0*pl.exp(-0.001757*(eps*rf)**2)
                            #print "Calculated ap.eff. eta = %5.3f " % (eta)
                            #print "At rest frequency %5.3f GHz" % (rf)
                            casalog.post( "Calculated ap.eff. eta = %5.3f " % (eta) )
                            casalog.post( "At rest frequency %5.3f GHz" % (rf) )
                            D = 104.9 # 100m x 110m
                            #print "Assume phys.diam D = %5.1f m" % (D)
                            casalog.post( "Assume phys.diam D = %5.1f m" % (D) )
                            s.convert_flux(eta=eta,d=D)

                            #print "Successfully converted fluxunit to "+fluxunit
                            casalog.post( "Successfully converted fluxunit to "+fluxunit )
                    elif ( antennaname in ['AT','ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43', 'CEDUNA', 'HOBART'] ):
                            s.convert_flux()

                    else:
                            # Unknown telescope type
                            #print "Unknown telescope - cannot convert"
                            casalog.post( "Unknown telescope - cannot convert", priority = 'WARN' )


            # set default spectral axis unit
            if ( specunit != '' ):
                    s.set_unit(specunit)

            # reset frame and doppler if needed
            if ( frame != '' ):
                    s.set_freqframe(frame)
            else:
                    #print 'Using current frequency frame'
                    casalog.post( 'Using current frequency frame' )

            if ( doppler != '' ):
                    if ( doppler == 'radio' ):
                            ddoppler = 'RADIO'
                    elif ( doppler == 'optical' ):
                            ddoppler = 'OPTICAL'
                    elif ( doppler == 'z' ):
                            ddoppler = 'Z'
                    else:
                            ddoppler = doppler

                    s.set_doppler(ddoppler)
            else:
                    #print 'Using current doppler convention'
                    casalog.post( 'Using current doppler convention' )

            # Prepare a selection
            sel=sd.selector()

            # Scan selection
            if ( type(scanlist) == list ):
                    # is a list
                    scans = scanlist
            else:
                    # is a single int, make into list
                    scans = [ scanlist ]
            if ( len(scans) > 0 ):
                    sel.set_scans(scans)

            # Select source names
            if ( field != '' ):
                    sel.set_name(field)
                    # NOTE: currently can only select one
                    # set of names this way, will probably
                    # need to do a set_query eventually

            # IF selection
            if ( type(iflist) == list ):
                    # is a list
                    ifs = iflist
            else:
                    # is a single int, make into list
                    ifs = [ iflist ]
            if ( len(ifs) > 0 ):
                    # Do any IF selection
                    sel.set_ifs(ifs)

            # polarization selection
            if (type(pollist) == list):
              pols = pollist
            else:
              pols = [pollist]

            if(len(pols) > 0 ):
              sel.set_polarisations(pols)


            try:
                #Apply the selection
                s.set_selection(sel)
                del sel
            except Exception, instance:
                #print '***Error***',instance
                casalog.post( str(instance), priority = 'ERROR' )
                return

	    # Warning for multi-IF data
	    if len(s.getifnos()) > 1:
		#print '\nWarning - The scantable contains multiple IF data.'
		#print '          Note the same mask(s) are applied to all IFs based on CHANNELS.'
		#print '          Baseline ranges may be incorrect for all but IF=%d.\n' % (s.getif(0))
		casalog.post( 'The scantable contains multiple IF data.', priority='WARN' )
		casalog.post( 'Note the same mask(s) are applied to all IFs based on CHANNELS.', priority='WARN' )
		casalog.post( 'Baseline ranges may be incorrect for all but IF=%d.\n' % (s.getif(0)), priority='WARN' )

            # If statfile is set, sd.rcParams['verbose'] must be True
            verbsave=sd.rcParams['verbose']
            if ( len(statfile) > 0 ):
                    if ( not os.path.exists(statfile) or overwrite ):
                            sd.rcParams['verbose']=True

	    ### Start mod: 2009/09/03 kana ###
	    format=format.replace(' ','')
	    formstr=format
	    if len(format)==0:
		casalog.post("Invalid format string. Using the default 3.3f.")
		formstr='3.3f'
	    ### End mod ######################
            # Interactive mask
            if interactive:
                    # Interactive masking
		    new_mask=sd.interactivemask(scan=s)
		    if (len(masklist) > 0):
		            new_mask.set_basemask(masklist=masklist,invert=False)

		    new_mask.select_mask(once=False,showmask=True)
		    # Wait for user to finish mask selection
		    finish=raw_input("Press return to calculate statistics.\n")
		    new_mask.finish_selection()
			    
		    # Get final mask list
		    msk=new_mask.get_mask()
		    del new_mask
		    msks=s.get_masklist(msk)
		    if len(msks) < 1:
			    #print 'No channel is selected. Exit without calculation.'
                            raise Exception, 'No channel is selected. Exit without calculation.'
			    #return
		    lbl=s.get_unit()
                    #print 'final mask list ('+lbl+') =',msks
                    casalog.post( 'final mask list ('+lbl+') = '+str(msks) )

                    # Get statistic values
                    maxl=s.stats('max',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    minl=s.stats('min',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
		    maxabcl=s.stats('max_abc',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
		    minabcl=s.stats('min_abc',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    suml=s.stats('sum',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    meanl=s.stats('mean',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    medianl=s.stats('median',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    rmsl=s.stats('rms',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    stdvl=s.stats('stddev',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
		    del msk, msks

            # set the mask region
            elif ( len(masklist) > 0):
                    msk=s.create_mask(masklist,invert=invertmask)
		    msks=s.get_masklist(msk)
		    if len(msks) < 1:
			    del msk, msks
			    #print 'Selected mask lists are out of range. Exit without calculation.'
                            raise Exception, 'Selected mask lists are out of range. Exit without calculation.'
			    #return

                    maxl=s.stats('max',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    minl=s.stats('min',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
		    maxabcl=s.stats('max_abc',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
		    minabcl=s.stats('min_abc',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    suml=s.stats('sum',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    meanl=s.stats('mean',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    medianl=s.stats('median',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    rmsl=s.stats('rms',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    stdvl=s.stats('stddev',msk,formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    del msk, msks             
            else:
                    # Full region
                    #print 'Using full region'
                    casalog.post( 'Using full region' )

                    maxl=s.stats('max',form=formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    minl=s.stats('min',form=formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
		    maxabcl=s.stats('max_abc',form=formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
		    minabcl=s.stats('min_abc',form=formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    suml=s.stats('sum',form=formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    meanl=s.stats('mean',form=formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    medianl=s.stats('median',form=formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    rmsl=s.stats('rms',form=formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()
                    stdvl=s.stats('stddev',form=formstr)
                    if sd.rcParams['verbose']:
                            f=open(tmpfile,'r')
                            resultstats+=f.readlines()
                            f.close()


            # Put into output dictionary
	    # Get unit labels
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

            ns = len(maxl)
            if ( ns > 1 ):
                    # User selected multiple scans,ifs
                    # put into lists
                    retValue['rms']=list(rmsl)
                    retValue['stddev']=list(stdvl)
                    retValue['max']=list(maxl)
		    retValue['max_abscissa']=qa.quantity(list(maxabcl),xunit)
                    retValue['min']=list(minl)
		    retValue['min_abscissa']=qa.quantity(list(minabcl),xunit)
                    retValue['sum']=list(suml)
                    retValue['median']=list(medianl)
                    retValue['mean']=list(meanl)
                    
                    # Construct equivalent width (=sum/max)
                    eqw=[]

		    integratef=[]
		    outp = ''
                    out = ''
                    for i in range(ns):
			    #Get bin width
			    abcissa, lbl = s.get_abcissa(rowno=i)
			    dabc=abs(abcissa[-1] - abcissa[0])/float(len(abcissa)-1)

                            if ( maxl[i] != 0.0 or minl[i] != 0.0 ):
                                    if ( abs(maxl[i]) >= abs(minl[i]) ):
                                            eqw = eqw + [ suml[i]/maxl[i]*dabc ]
                                    else:
                                            eqw = eqw + [ suml[i]/minl[i]*dabc ]
                            else:
                                    eqw = eqw + [0.0]

                            out += 'Scan[%d] (%s) ' % (s.getscan(i), s._getsourcename(i))
                            out += 'Time[%s]:\n' % (s._gettime(i))
                            if s.nbeam(-1) > 1: out +=  ' Beam[%d] ' % (s.getbeam(i))
                            if s.nif(-1) > 1: out +=  ' IF[%d] ' % (s.getif(i))
                            if s.npol(-1) > 1: out +=  ' Pol[%d] ' % (s.getpol(i))
			    #out += '= %3.3f\n' % (eqw[i])
			    out += ('= %'+formstr) % (eqw[i]) + '\n'
                            out +=  "--------------------------------------------------\n "
			    # Construct integrated flux
			    integratef = integratef +[suml[i]*dabc]
			    outp += 'Scan[%d] (%s) ' % (s.getscan(i), s._getsourcename(i))
			    outp += 'Time[%s]:\n' % (s._gettime(i))
			    if s.nbeam(-1) > 1: outp +=  ' Beam[%d] ' % (s.getbeam(i))
			    if s.nif(-1) > 1: outp +=  ' IF[%d] ' % (s.getif(i))
			    if s.npol(-1) > 1: outp +=  ' Pol[%d] ' % (s.getpol(i))
			    #outp += '= %3.3f\n' % (integratef[i])
			    outp += ('= %'+formstr) % (integratef[i])+'\n'
			    outp +=  "--------------------------------------------------\n "

                    if sd.rcParams['verbose']:
                            # Print equivalent width
                            f = open(tmpfile,'w')
                            print >> f, "--------------------------------------------------"
                            print >> f, " ", "eqw [",abclbl,"]"
                            print >> f, "--------------------------------------------------"
                            print >> f, out
                            #print >> f, ''
                            f.close()
                            f = open(tmpfile,'r')
                            rlines = f.readlines()
                            f.close()
                            resultstats += rlines
                            for rl in rlines:
                                    casalog.post( rl )

                            # Print integrated flux
                            f = open(tmpfile,'w')
                            print >> f,  "--------------------------------------------------"
                            print >> f,  " ", "Integrated intensity [", intlbl, "]"
                            print >> f, "--------------------------------------------------"
                            print >> f, outp
                            #print >> f, ''
                            f.close()
                            f = open(tmpfile,'r')
                            rlines = f.readlines()
                            f.close()
                            resultstats += rlines
                            for rl in rlines:
                                    casalog.post( rl )

		    retValue['eqw']=qa.quantity(eqw,xunit)
		    retValue['totint']=qa.quantity(integratef,intunit)
                    
            else:
                    # Single scantable only
                    # put into scalars
                    retValue['rms']=rmsl[0]
                    retValue['stddev']=stdvl[0]
                    retValue['max']=maxl[0]
		    retValue['max_abscissa']=qa.quantity(maxabcl[0],xunit)
                    retValue['min']=minl[0]
		    retValue['max_abscissa']=qa.quantity(minabcl[0],xunit)
                    retValue['sum']=suml[0]
                    retValue['median']=medianl[0]
                    retValue['mean']=meanl[0]

		    outp = ''
		    abcissa, lbl = s.get_abcissa(rowno=0)
		    dabc=abs(abcissa[-1] - abcissa[0])/float(len(abcissa)-1)

                    # Construct equivalent width (=sum/max)
                    if ( maxl[0] != 0.0 or minl[0] != 0.0 ):
                            if ( abs(maxl[0]) >= abs(minl[0]) ):
                                    eqw = suml[0]/maxl[0]*dabc
                            else:
                                    eqw = suml[0]/minl[0]*dabc
                    else:
                            eqw = 0.0
                    out = ''
                    out += 'Scan[%d] (%s) ' % (s.getscan(0), s._getsourcename(0))
                    out += 'Time[%s]:\n' % (s._gettime(0))
                    if s.nbeam(-1) > 1: out +=  ' Beam[%d] ' % (s.getbeam(0))
                    if s.nif(-1) > 1: out +=  ' IF[%d] ' % (s.getif(0))
                    if s.npol(-1) > 1: out +=  ' Pol[%d] ' % (s.getpol(0))
		    #out += '= %3.3f\n' % (eqw)
		    out += ('= %'+formstr) % (eqw) + '\n'
                    out +=  "--------------------------------------------------\n "
                    
		    # Construct integrated flux
		    integratef = suml[0]*dabc
		    outp += 'Scan[%d] (%s) ' % (s.getscan(0), s._getsourcename(0))
		    outp += 'Time[%s]:\n' % (s._gettime(0))
		    if s.nbeam(-1) > 1: outp +=  ' Beam[%d] ' % (s.getbeam(0))
		    if s.nif(-1) > 1: outp +=  ' IF[%d] ' % (s.getif(0))
		    if s.npol(-1) > 1: outp +=  ' Pol[%d] ' % (s.getpol(0))
		    #outp += '= %3.3f\n' % (integratef)
		    outp += ('= %'+formstr) % (integratef) + '\n'
		    outp +=  "--------------------------------------------------\n "

                    if sd.rcParams['verbose']:
                            # Print equivalent width
                            f = open(tmpfile,'w')
                            print >> f,  "--------------------------------------------------"
                            print >> f, " ", "eqw [",abclbl,"]"
                            print >> f, "--------------------------------------------------"
                            print >> f, out
                            f.close()
                            f = open(tmpfile,'r')
                            rlines = f.readlines()
                            f.close()
                            resultstats += rlines
                            for rl in rlines:
                                    casalog.post( rl )
                            # Print integrated flux
                            f = open(tmpfile,'w')
                            print >> f, "--------------------------------------------------"
                            print >> f, " ", "Integrated intensity [", intlbl, "]"
                            print >> f, "--------------------------------------------------"
                            print >> f, outp
                            f = open(tmpfile,'r')
                            rlines = f.readlines()
                            f.close()
                            resultstats += rlines
                            for rl in rlines:
                                    casalog.post( rl )
                            
		    retValue['eqw']=qa.quantity(eqw,xunit)
		    retValue['totint']=qa.quantity(integratef,intunit)

                    
            # Output to terminal if statfile is not empty
            if ( len(statfile) > 0 ):
                    if ( not os.path.exists(statfile) or overwrite ):
                            #sys.stdout=open( statfile,'w' )
                            f=open(statfile,'w')
                            for xx in resultstats:
                                    f.write( xx )
                            f.close()
                            sd.rcParams['verbose']=verbsave
                            #bbb=True
                    else:
                            #print '\nFile '+statfile+' already exists.\nStatistics results are not written into the file.\n'
                            casalog.post( 'File '+statfile+' already exists.\nStatistics results are not written into the file.', priority = 'WARN' )

            # Final clean up
            del s

            #return retValue
            return retValue
    
            # DONE
            
        except Exception, instance:
                #print '***Error***',instance
                casalog.post( str(instance), priority = 'ERROR' )
                return
        finally:
                casalog.post('')

