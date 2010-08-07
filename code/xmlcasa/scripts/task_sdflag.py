import os
from taskinit import *

import asap as sd
import pylab as pl
from numpy import ma, array, logical_not, logical_and

def sdflag(sdfile, antenna, scanlist, field, iflist, pollist, maskflag, flagrow, clip, clipminmax, clipoutside, flagmode, outfile, outform, overwrite, plotlevel):

        casalog.origin('sdflag')

        try:
            myp=None
            if sdfile=='':
                raise Exception, 'sdfile is undefined'

            filename = os.path.expandvars(sdfile)
            filename = os.path.expanduser(filename)
            if not os.path.exists(filename):
                s = "File '%s' not found." % (filename)
                raise Exception, s

	    if (outfile != ''):
	        project = outfile
	    else:
	        project = sdfile.rstrip('/')
	        if not overwrite:
		    project = project + '_f'

            outfilename = os.path.expandvars(project)
            outfilename = os.path.expanduser(outfilename)
            if not overwrite and os.path.exists(outfilename):
		    s = "Output file '%s' exist." % (outfilename)
                    raise Exception, s

            s = sd.scantable(sdfile,average=False,antenna=antenna)


            #check the format of the infile
            if isinstance(sdfile, str):
              if os.path.isdir(filename) and os.path.exists(filename+'/table.info'):
                if os.path.exists(filename+'/table.f1'):
                  format = 'MS2'
                else:
                  format = 'ASAP'
              else:
                  format = 'SDFITS'


            if ( abs(plotlevel) > 1 ):
                    casalog.post( "Initial Scantable:" )
                    casalog.post( s._summary() )
                    casalog.post( '--------------------------------------------------------------------------------' )

            # Default file name
            #if ( outfile == '' ):
            #        project = sdfile + '_f'
            #elif ( outfile == 'none' ):
            #        project = sdfile 
            #        outform = format
            #        overwrite=True
            #else:
            #        project = outfile

            # get telescope name
            #'ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43' (Tid), 'CEDUNA', and 'HOBART'
            #Currently only channel based flag is allowed
            # so make sure the input data is channel 
            unit_in=s.get_unit()
            if unit_in!='channel':
                s.set_unit('channel')

            antennaname = s.get_antennaname()

            # Select scan and field
            sel = sd.selector()


            # Set up scanlist
            if ( type(scanlist) == list ):
                    # is a list
                    scans = scanlist
            else:
                    # is a single int, make into list
                    scans = [ scanlist ]
            # Now select them
            if ( len(scans) > 0 ):
                    sel.set_scans(scans)

            # Select source names
            if ( field != '' ):
                    sel.set_name(field)
                    # NOTE: currently can only select one
                    # set of names this way, will probably
                    # need to do a set_query eventually

            # Select IFs
            if ( type(iflist) == list ):
                    # is a list
                    ifs = iflist
            else:
                    # is a single int, make into list
                    ifs = [ iflist ]
            if ( len(ifs) > 0 ):
                    # Do any IF selection
                    sel.set_ifs(ifs)

            # Select Pol
            if ( type(pollist) == list ):
                    pols = pollist
            else:
                    pols = [ pollist ] 
            if ( len(pols) > 0 ):
                    sel.set_polarisations(pols)

            try:
                #Apply the selection
                s.set_selection(sel)
            except Exception, instance:
                casalog.post( str(instance), priority = 'ERROR' )
                return

            # flag mode
	    flgmode = flagmode.lower()
            if (flgmode == 'flag'):
                    unflag = False
            elif (flgmode == 'unflag'):
                    unflag = True
            else:
                    raise Exception, 'unexpected flagmode'

            nr=s.nrow()

	    if clip:
		    casalog.post("Number of spectra to be flagged: %d" % (nr) )
		    casalog.post("Applying clipping...")
	    else:
		    if (len(flagrow) == 0):
                            casalog.post( "Number of spectra to be flagged: %d" % (nr) )
                            casalog.post( "Applying channel flagging..." )
	            else:
		            casalog.post( "Number of rows to be flagged: %d" % (len(flagrow)) )
		            casalog.post( "Applying row flagging..." )
		    

	    dthres = uthres = None
	    if isinstance(clipminmax, list):
		    if (len(clipminmax) == 2):
			    dthres = min(clipminmax)
			    uthres = max(clipminmax)
			    
	    
	    if (len(flagrow) == 0) and (not clip):
	            #channel flag
		    if (len(maskflag) == 0):
			    raise Exception, 'maskflag is undefined'
		    masks = s.create_mask(maskflag)

            #for row in range(ns):
            if ( abs(plotlevel) > 0 ):

                    #sc=s.copy()
		    # Plot final spectrum
		    if nr < 3:
			    nrow=1
			    ncol=nr
		    elif nr < 5:
			    nrow=2
			    ncol=2
		    elif nr < 7:
			    nrow=2
			    ncol=3
		    elif nr < 10:
			    nrow=3
			    ncol=3
		    else:
			    nrow=4
			    ncol=4
		    
		    casalog.post( "nrow,ncol= %d,%d" % (nrow, ncol) )
		    if nr >16:
			    casalog.post( "Only first 16 spectra is plotted.", priority = 'WARN' )

                    if not myp or myp.is_dead:
                        if sd.rcParams['plotter.gui']:
                            from asap.asaplotgui import asaplotgui as asaplot
                        else:
                            from asap.asaplot import asaplot
                    myp = asaplot()
                    myp.hold()
                    myp.clear()
                    myp.set_panels(nrow,ncol)
                    colours = ["green","red","#dddddd","#777777"]
                    if nr <17:
                        rowlist=range(nr)
                    else:
                        rowlist=range(16)
                    for row in rowlist:
                        myp.subplot(row)
                        myp.palette(0,colours)
                        if row==rowlist[0]:
                          myp.set_line(label='Spectrum')
                        else:
                          myp.set_line(label='spec')
                        x = s._getabcissa(row)
                        y = s._getspectrum(row)
			
			if s._getflagrow(row):
				oldmskarr = array([False]*(s.nchan()))
			else:
				oldmskarr = array(s._getmask(row))
				
			if clip:
			  if (uthres != None) and (dthres != None) and (uthres > dthres):
			    masks = array(s._getclipmask(row, uthres, dthres, clipoutside, unflag))
			  else:
			    masks = [False]*(s.nchan())
			elif (len(flagrow) > 0):
			  found = False
			  for i in range(0, len(flagrow)):
			    if (row == flagrow[i]):
			      found = True
			      break
			  masks = [found and not(unflag)]*(s.nchan())
			#marr = array(masks)
			
			marr = logical_not(array(masks))
			allmsk = logical_and(marr,oldmskarr)
                        ym = ma.masked_array(y,mask=logical_not(allmsk))
                        myp.plot(x,ym)
                        myp.palette(2)
                        if row==rowlist[0]:
                          myp.set_line(label='previously flagged data')
                        else:
                          myp.set_line(label='prev')
                        #oldmskarr = logical_not(oldmskarr)
                        ym = ma.masked_array(y,mask=oldmskarr)
                        myp.plot(x,ym)
                        myp.palette(1)
                        if row==rowlist[0]:
                          myp.set_line(label='current flag masks')
                        else:
                          myp.set_line(label='flag')
                        ym = ma.masked_array(y,mask=marr)
                        myp.plot(x,ym)
                        xlim=[min(x),max(x)]
                        myp.axes.set_xlim(xlim)
                        myp.release()

                    #Apply flag
                    ans=raw_input("Apply %s (y/n)?: " % flgmode)
                    if ans.upper() == 'Y':
		        if (clip):
				if (uthres != None) and (dthres != None) and (uthres > dthres):
					s.clip(uthres, dthres, clipoutside, unflag)
			else:
				if (len(flagrow) == 0):
					s.flag(masks,unflag)
				else:
					s.flag_row(flagrow, unflag)
                        params={}
                        if ( vars()['pols'] == [] ):
                                params['pols']=list(s.getpolnos())
                        else:
                                params['pols']=vars()['pols']
                        if ( vars()['ifs'] == [] ):
                                params['ifs']=list(s.getifnos())
                        else:
                                params['ifs']=vars()['ifs']
                        if ( vars()['scans'] == [] ):
                                params['scans']=list(s.getscannos())
                        else:
                                params['scans']=vars()['scans']
                        params['mode']=vars()['flagmode']
                        params['maskflag']=vars()['maskflag']
                        #print "input parameters:\n", params
                        s._add_history( "sdflag", params ) 
                    else:
                        return
                    #Plot the result
                    #print "Showing only the first spectrum..."
                    casalog.post( "Showing only the first spectrum..." )
                    row=rowlist[0]
                    if not myp or myp.is_dead:
                        if sd.rcParams['plotter.gui']:
                            from asap.asaplotgui import asaplotgui as asaplot
                        else:
                            from asap.asaplot import asaplot
                    myp = asaplot()
                    myp.hold()
                    myp.clear()
                    myp.palette(0,colours)
                    myp.set_panels()
                    myp.set_line(label="Spectrum after %s" % flgmode+'ging')
                    x = s._getabcissa(row)
                    y = s._getspectrum(row)
                    allmskarr=array(s._getmask(row))
                    #y = ma.masked_array(y,mask=marr)
                    ym = ma.masked_array(y,mask=logical_not(allmskarr))
                    myp.plot(x,ym)
                    myp.palette(2)
                    myp.set_line(label="Flagged")
                    #allmsk = logical_and(marr,oldmskarr)
                    #y = ma.masked_array(y,mask=logical_not(allmsk))
                    ym = ma.masked_array(y,mask=allmskarr)
                    myp.plot(x,ym)
                    myp.axes.set_xlim(xlim)
                    if ( plotlevel < 0 ):
                            # Hardcopy - currently no way w/o screen display first
                            pltfile=project+'_flag.eps'
                            myp.save(pltfile)
                    myp.release()
            else:
                    ans=raw_input("Apply %s (y/n)?: " % flgmode)
                    if ans.upper() == 'Y':
			    if clip:
				    if (uthres != None) and (dthres != None) and (uthres > dthres):
					    s.clip(uthres, dthres, clipoutside, unflag)
			    else:
				    if (len(flagrow) == 0):
					    s.flag(masks,unflag)
				    else:
					    s.flag_row(flagrow, unflag)
                            params={}
                            if ( vars()['pols'] == [] ):
                                    params['pols']=list(s.getpolnos())
                            else:
                                    params['pols']=vars()['pols']
                            if ( vars()['ifs'] == [] ):
                                    params['ifs']=list(s.getifnos())
                            else:
                                    params['ifs']=vars()['ifs']
                            if ( vars()['scans'] == [] ):
                                    params['scans']=list(s.getscannos())
                            else:
                                    params['scans']=vars()['scans']
                            params['mode']=vars()['flagmode']
                            params['maskflag']=vars()['maskflag']
                            #print "input parameters:\n", params
                            s._add_history( "sdflag", params )
                    else:
                            return

           # Now save the spectrum and write out final ms
            if ( (outform == 'ASCII') or (outform == 'ascii') ):
                    outform = 'ASCII'
                    spefile = project + '_'
            elif ( (outform == 'ASAP') or (outform == 'asap') ):
                    outform = 'ASAP'
                    spefile = project
            elif ( (outform == 'SDFITS') or (outform == 'sdfits') ):
                    outform = 'SDFITS'
                    spefile = project
            elif ( (outform == 'MS') or (outform == 'ms') or (outform == 'MS2') or (outform == 'ms2') ):
                    outform = 'MS2'
                    spefile = project
            else:
                    outform = 'ASAP'
                    spefile = project

            if overwrite and os.path.exists(outfilename):
                    os.system('rm -rf %s' % outfilename)
            
            #put back original spectral unit
            s.set_unit(unit_in) 
            sel.reset()
            s.set_selection(sel)
            s.save(spefile,outform,overwrite)
	    
            if outform!='ASCII':
                    casalog.post( "Wrote output "+outform+" file "+spefile )

            del s, sel

        except Exception, instance:
                casalog.post( str(instance), priority = 'ERROR' )
        finally:
                casalog.post('')
