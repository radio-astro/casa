import os
from taskinit import *

import sdutil
import asap as sd
from asap.scantable import is_scantable,is_ms
from asap.flagplotter import flagplotter
import pylab as pl
from numpy import ma, array, logical_not, logical_and

def sdflag(infile, antenna, specunit, restfreq, frame, doppler, scanlist, field, iflist, pollist, maskflag, flagrow, clip, clipminmax, clipoutside, flagmode, interactive, showflagged, outfile, outform, overwrite, plotlevel):

        casalog.origin('sdflag')

        try:
            myp=None
            sdutil.assert_infile_exists(infile)
            filename = sdutil.get_abspath(infile)

	    if (outfile != ''):
	        project = outfile.rstrip('/')
	    else:
	        project = infile.rstrip('/')
	        if not overwrite:
		    project = project + '_f'

            sdutil.assert_outfile_canoverwrite_or_nonexistent(project,outform,overwrite)

            sorg = sd.scantable(infile,average=False,antenna=antenna)

	    # Copy the original data (CAS-3987)
	    if (sd.rcParams['scantable.storage'] == 'disk') and (project != infile.rstrip('/')):
		    s = sorg.copy()
	    else:
		    s = sorg
	    del sorg

	    # set restfreq
	    modified_molid = False
	    if (specunit == 'km/s'):
		    if (restfreq == '') and (len(s.get_restfreqs()[0]) == 0):
			    mesg = "Restfreq must be given."
			    raise Exception, mesg
		    elif (len(str(restfreq)) > 0):
			    molids = s._getmolidcol_list()
			    s.set_restfreqs(sdutil.normalise_restfreq(restfreq))
			    modified_molid = True

            #check the format of the infile
            if isinstance(infile, str):
                    if is_scantable(filename):
                            format = 'ASAP'
                    elif is_ms(filename):
                            format = 'MS2'
                    else:
                            format = 'SDFITS'

	    # Check the formats of infile and outfile are identical when overwrite=True.
	    # (CAS-3096). If not, print warning message and exit.
	    outformat = outform.upper()
	    if (outformat == 'MS'): outformat = 'MS2'
	    if overwrite and (project == infile.rstrip('/')) and (outformat != format):
		    msg = "The input and output data format must be identical when "
		    msg += "their names are identical and overwrite=True. "
		    msg += "%s and %s given for input and output, respectively." % (format, outformat)
	            raise Exception, msg
	    
            # Do at least one
	    docmdflag = True
	    if (len(flagrow) == 0) and (len(maskflag) == 0) and (not clip):
                    if not interactive:
			    raise Exception, 'No flag operation specified.'
		    # interactive flagging only
                    docmdflag = False

            print_summary(s, plotlevel)

 	    # set default spectral axis unit
            unit_in=s.get_unit()
            sdutil.set_spectral_unit(s, specunit)
            
	    # reset frame and doppler if needed
            sdutil.set_freqframe(s, frame)
            sdutil.set_doppler(s, doppler)

            # data selection
            sel = sdutil.get_selector(in_scans=scanlist,
                                      in_ifs=iflist,
                                      in_pols=pollist,
                                      in_field=field)
            s.set_selection(sel)
            del sel

            # flag mode
            if not flagmode.lower() in ['flag','unflag']:
                raise Exception, 'unexpected flagmode'
            unflag = (flagmode.lower() == 'unflag')
                    

            if docmdflag:
                    doflag(s, flagmode, maskflag, clip, clipminmax, clipoutside, flagrow, unflag, plotlevel)

            anyflag = docmdflag
            if interactive:
                    interactiveflag = dointeractiveflag(s,showflagged)
                    anyflag = (anyflag or interactiveflag)

            if not anyflag:
                    del s
                    raise Exception, 'No flag operation. Finish without saving'

            posterior_plot(s, project, unflag, plotlevel)

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

	    # Commented out on 19 Apr 2012. (CAS-3986)
            #if overwrite and os.path.exists(outfilename):
            #        os.system('rm -rf %s' % outfilename)
            
            #put back original spectral unit
            s.set_unit(unit_in) 
            s.set_selection()

	    #restore the original moleculeID column
	    if modified_molid:
		    s._setmolidcol_list(molids)
	    
            s.save(spefile,outform,overwrite)
	    
            if outform!='ASCII':
                    casalog.post( "Wrote output "+outform+" file "+spefile )

            del s

        except Exception, instance:
                sdutil.process_exception(instance)
		raise Exception, instance
        finally:
                casalog.post('')

def doflag(s, flagmode, maskflag, clip, clipminmax, clipoutside, flagrow, unflag, plotlevel):
    nr = s.nrow()
    if clip:
            casalog.post("Number of spectra to be flagged: %d" % (nr) )
            casalog.post("Applying clipping...")
            if len(flagrow) > 0 or len(maskflag) > 0:
                    casalog.post("flagrow and maskflag will be ignored",priority = 'WARN')
    elif len(flagrow) == 0:
            # Channel based flag
            casalog.post( "Number of spectra to be flagged: %d" % (nr) )
            casalog.post( "Applying channel flagging..." )
    else:
            # Row flagging
            casalog.post( "Number of rows to be flagged: %d" % (len(flagrow)) )
            casalog.post( "Applying row flagging..." )
            if len(maskflag) > 0:
                    casalog.post("maskflag will be ignored",priority = 'WARN')
                    
    if (len(maskflag) > 0):
        masks = s.create_mask(maskflag)
    else:
        masks = [False for i in xrange(s.nchan())]
        
    doclip = False
    dthres = uthres = None
    if isinstance(clipminmax, list):
        if (len(clipminmax) == 2):
            dthres = min(clipminmax)
            uthres = max(clipminmax)
            doclip = (uthres > dthres)
            
    ans = prior_plot(s, clip, doclip, uthres, dthres, clipoutside, flagrow, unflag, masks, plotlevel)

    if ans.upper() == 'Y':
            if clip and doclip:
                    s.clip(uthres, dthres, clipoutside, unflag)
            elif (len(flagrow) == 0):
                    s.flag(mask=masks,unflag=unflag)
            else:
                    s.flag_row(flagrow, unflag)

            params={}
            sel = s.get_selection()
            scans = sel.get_scans()
            ifs = sel.get_ifs()
            pols = sel.get_pols()
            params['pols'] = pols if len(pols)>0 else list(s.getpolnos())
            params['ifs'] = ifs if len(ifs)>0 else list(s.getifnos())
            params['scans'] = scans if len(scans)>0 else list(s.getscannos())
            params['mode']=flagmode
            params['maskflag']=maskflag
            #print "input parameters:\n", params
            s._add_history( "sdflag", params ) 

def dointeractiveflag(s, showflagged):
    from matplotlib import rc as rcp
    rcp('lines', linewidth=1)
    guiflagger = flagplotter(visible=True)
    #guiflagger.set_legend(loc=1,refresh=False)
    guiflagger.set_showflagged(showflagged)
    guiflagger.plot(s)
    finish=raw_input("Press enter to finish interactive flagging:")
    guiflagger._plotter.unmap()
    anyflag = guiflagger._ismodified
    guiflagger._plotter = None
    del guiflagger
    return anyflag

def print_summary(s, plotlevel):
    if ( abs(plotlevel) > 1 ):
        casalog.post( "Initial Scantable:" )
        #casalog.post( s._summary() )
        s._summary()

def prior_plot(s, clip, doclip, uthres, dthres, clipoutside, flagrow, unflag, defaultmask, plotlevel):
    if ( abs(plotlevel) > 0 ):
        flgmode = 'unflag' if unflag else 'flag'
        nr = s.nrow()
        np = min(nr,16)
        if nr >16:
            casalog.post( "Only first 16 spectra is plotted.", priority = 'WARN' )
        masks16 = []
        if clip:
            for row in xrange(np):
                if doclip:
                    masks16.append(array(s._getclipmask(row, uthres, dthres, clipoutside, unflag)))
                else:
                    nchan = s.nchan(s.getif(row))
                    masks16.append([False]*(nchan))
        elif (len(flagrow) > 0):
            for row in xrange(np):
                nchan = s.nchan(s.getif(row))
                masks16.append([(row in flagrow) and not (unflag)]*nchan)

        myp = init_plotter(plotlevel)

        myp.set_panels(rows=np,cols=0,nplots=np)
        myp.legend(loc=1)
        label0='Spectrum'
        label1='previously flagged data'
        label2='current flag masks'
        for row in xrange(np):
                myp.subplot(row)
                x = s._getabcissa(row)
                y = s._getspectrum(row)
                nchan = len(y)

                if s._getflagrow(row):
                        oldmskarr = array([False]*(nchan))
                else:
                        oldmskarr = array(s._getmask(row))

                masks = masks16[row] if len(masks16)>0 else defaultmask

                marr = logical_not(array(masks))
                allmsk = logical_and(marr,oldmskarr)
                plot_data(myp,x,y,logical_not(allmsk),0,label0)
                plot_data(myp,x,y,oldmskarr,2,label1)
                plot_data(myp,x,y,marr,1,label2)
                xlim=[min(x),max(x)]
                myp.axes.set_xlim(xlim)

                label0='spec'
                label1='prev'
                label2='flag'
        myp.release()
        
        #Apply flag
        if plotlevel > 0 and sd.rcParams['plotter.gui']:
                ans=raw_input("Apply %s (y/N)?: " % flgmode)
        else:
                casalog.post("Applying selected flags")
                ans = 'Y'
    else:
            ans='Y'
    return ans


def posterior_plot(s, project, unflag, plotlevel):
    if ( abs(plotlevel) > 0 ):
        flgmode = 'unflag' if unflag else 'flag'
        #Plot the result
        #print "Showing only the first spectrum..."
        casalog.post( "Showing only the first spectrum..." )
        row=0
        myp = init_plotter()

        myp.set_panels()
        x = s._getabcissa(row)
        y = s._getspectrum(row)
        allmskarr=array(s._getmask(row))
        plot_data(myp,x,y,logical_not(allmskarr),0,"Spectrum after %s" % flgmode+'ging')
        plot_data(myp,x,y,allmskarr,2,"Flagged")
        xlim=[min(x),max(x)]
        myp.axes.set_xlim(xlim)
        if ( plotlevel < 0 ):
            # Hardcopy - currently no way w/o screen display first
            pltfile=project+'_flag.eps'
            myp.save(pltfile)
        myp.release()

def plot_data(myp,x,y,msk,color=0,label=None):
    if label:
        myp.set_line(label=label)
    myp.palette(color)
    ym = ma.masked_array(y,mask=msk)
    myp.plot(x,ym)

def init_plotter(colormap=None,plotlevel=None):
    f=get_frame('sdflag')
    #myp = f.f_locals['myp']
    myp = f.f_globals['myp'] if f.f_globals.has_key('myp') else None
    if not (myp and myp._alive()):
        from matplotlib import rc as rcp
        rcp('lines', linewidth=1)
        from asap.asapplotter import new_asaplot
        visible = (plotlevel > 0) if plotlevel else sd.rcParams['plotter.gui']
        newp = new_asaplot(visible=visible)
        casalog.post('Create new plotter')
    else:
        newp = myp
    newp.palette(0,colormap)
    newp.hold()
    newp.clear()
    f.f_globals['myp'] = newp
    return newp    

def get_frame(name):
    import inspect
    s = inspect.stack()
    stacklevel=-1
    for i in xrange(len(s)):
        if (s[i][3] == 'sdflag'):
            stacklevel = i
            break
    return s[stacklevel][0]
