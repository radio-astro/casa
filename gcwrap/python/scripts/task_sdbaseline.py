import os
from taskinit import *

import sdutil
import asap as sd
from asap._asap import Scantable
from asap import _to_list
from asap.scantable import is_scantable
import math
import pylab as pl

def sdbaseline(infile, antenna, fluxunit, telescopeparm, specunit, restfreq, frame, doppler, scanlist, field, iflist, pollist, tau, masklist, maskmode, thresh, avg_limit, edge, blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn, rejwn, clipthresh, clipniter, verify, verbose, bloutput, blformat, showprogress, minnrow, outfile, outform, overwrite, plotlevel):
	
	casalog.origin('sdbaseline')

	try:
		sdutil.assert_infile_exists(infile)

		project = sdutil.get_default_outfile_name(infile, outfile, "_bs")
		sdutil.assert_outfile_canoverwrite_or_nonexistent(project, outform, overwrite)

		sorg = sd.scantable(infile, average=False, antenna=antenna)

		prior_plot(sorg, plotlevel)
		
		# check if the data contains spectra
		if (sorg.nchan()==1):
			s = "Cannot process the input data. It contains only single channel data."
			raise Exception, s
                
                sel = sdutil.get_selector(scanlist, iflist, pollist, field)
                sorg.set_selection(sel)
                del sel

		# Copy scantable when usign disk storage not to modify
		# the original table.
		if is_scantable(infile) and \
                       sd.rcParams['scantable.storage'] == 'disk':
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


		
                # convert flux
                sdutil.set_fluxunit(s, fluxunit, telescopeparm, insitu=True)
                
		#unit_in = s.get_unit()
		# set default spectral axis unit
                sdutil.set_spectral_unit(s, specunit)

		# reset frame and doppler if needed
                sdutil.set_freqframe(s, frame)
		sdutil.set_doppler(s, doppler)
		
		
		scanns = s.getscannos()
		sn=list(scanns)
		casalog.post( "Number of scans to be processed: %d" % (len(sn)) )
		
		# Warning for multi-IF data
		#if ( len(s.getifnos()) > 1 and not maskmode == 'auto' ):
		if ( len(s.getifnos()) > 1 and isinstance(masklist,list) and not maskmode == 'auto' ):
			casalog.post( 'The scantable contains multiple IF data.', priority = 'WARN' )
			casalog.post( 'Note the same mask(s) are applied to all IFs based on CHANNELS.', priority = 'WARN' )
			casalog.post( 'Baseline ranges may be incorrect for all but IF=%d.' % (s.getif(0)), priority = 'WARN' )
		
		# do opacity (atmospheric optical depth) correction
                sdutil.doopacity(s, tau)

		if (order < 0):
			casalog.post('Negative order of baseline polynomial given. Exit without baselining.', priority = 'WARN')
			return
                blfile = init_blfile(s, infile, project, masklist, maskmode, thresh, avg_limit, edge, blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn, rejwn, clipthresh, clipniter, bloutput, blformat)

                dobaseline(s, blfile, masklist, maskmode, thresh, avg_limit, edge, blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn, rejwn, clipthresh, clipniter, verify, verbose, blformat, showprogress, minnrow)

		# Plot final spectrum
                posterior_plot(s, project, plotlevel)

		#restore the original spectral axis unit
		#s.set_unit(unit_in)
		#restore the original moleculeID column
		if modified_molid:
			s._setmolidcol_list(molids)
                        
		# Now save the spectrum and write out final ms
		sdutil.save(s, project, outform, overwrite)
		
                del s
	
	except Exception, instance:
                sdutil.process_exception(instance)
		raise Exception, instance


def init_blfile(s, infile, project, masklist, maskmode, thresh, avg_limit, edge, blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn, rejwn, clipthresh, clipniter, bloutput, blformat):
        if bloutput:
                blfile = project + "_blparam.txt"

                if (blformat.lower() != "csv"):
                        blf = open(blfile, "w")

                        # Header data for saving parameters of baseline fit
                        header =  "Source Table: "+infile+"\n"
                        header += " Output File: "+project+"\n"
                        header += "   Flux Unit: "+s.get_fluxunit()+"\n"
                        header += "    Abscissa: "+s.get_unit()+"\n"
                        header += "    Function: "+blfunc+"\n"
                        if blfunc == 'poly':
                                header += "   Fit order: %d\n"%(order)
                        elif blfunc == 'chebyshev':
                                header += "   Fit order: %d\n"%(order)
                                header += "  clipThresh: %f\n"%(clipthresh)
                                header += "   clipNIter: %d\n"%(clipniter)
                        elif blfunc == 'cspline':
                                header += "      nPiece: %d\n"%(npiece)
                                header += "  clipThresh: %f\n"%(clipthresh)
                                header += "   clipNIter: %d\n"%(clipniter)
                        elif blfunc == 'sinusoid':
                                header += "    applyFFT: "+str(applyfft)+"\n"
                                header += "   fftMethod: "+fftmethod+"\n"
                                header += "   fftThresh: "+str(fftthresh)+"\n"
                                header += "    addWaveN: "+str(addwn)+"\n"
                                header += "    rejWaveN: "+str(rejwn)+"\n"
                                header += "  clipThresh: %f\n"%(clipthresh)
                                header += "   clipNIter: %d\n"%(clipniter)
                        header += "   Mask mode: "+maskmode+"\n"
                        if maskmode == 'auto':
                                header += "   Threshold: %f\n"%(thresh)
                                header += "   avg_limit: %d\n"%(avg_limit)
                                header += "        Edge: "+str(edge)+"\n"
                        elif maskmode == 'list':
                                header += "   Fit Range: "+str(masklist)+"\n"

                        separator = "#"*60 + "\n"

                        blf.write(separator)
                        blf.write(header)
                        blf.write(separator)
                        blf.close()
        else:
                blfile = ""
        return blfile
        

def dobaseline(s, blfile, masklist, maskmode, thresh, avg_limit, edge, blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn, rejwn, clipthresh, clipniter, verify, verbose, blformat, showprogress, minnrow):
        if (order < 0):
                casalog.post('Negative order of baseline polynomial given. Exit without baselining.', priority = 'WARN')
                return

        csvformat = (blformat.lower() == "csv")

        nrow = s.nrow()

        # parse string masklist
        if isinstance(masklist,list):
                maskdict = {'': masklist}
        else:
                maskdict = s.parse_maskexpr(masklist)
        basesel = s.get_selection()
        for sif, lmask in maskdict.iteritems():
                if len(sif) > 0:
                        #s.set_selection(selection=(basesel+sd.selector(ifs=[int(sif)])))
                        sel = sd.selector(basesel)
                        sel.set_ifs([int(sif)])
                        s.set_selection(sel)
                        del sel
                        msg = "Working on IF%s" % (sif)
                        casalog.post(msg)
                        if (maskmode == 'interact'): print "===%s===" % (msg)
                        del msg

                msk = None

                if (maskmode == 'interact'):
                        msk = sdutil.interactive_mask(s, lmask, False, purpose='to baseline spectra')
                        msks = s.get_masklist(msk)
                        if len(msks) < 1:
                                casalog.post( 'No channel is selected. Exit without baselining.', priority = 'WARN' )
                                return

                        casalog.post( 'final mask list ('+s._getabcissalabel()+') ='+str(msks) )
                        #header += "   Fit Range: "+str(msks)+"\n"
                        del msks
                else:
                        # Use baseline mask for regions to INCLUDE in baseline fit
                        # Create mask using list, e.g. masklist=[[500,3500],[5000,7500]]
                        if (len(lmask) > 0): msk = s.create_mask(lmask)


                if (maskmode == 'auto'):
                        if (blfunc == 'poly'):
                                s.auto_poly_baseline(mask=msk,order=order,edge=edge,threshold=thresh,chan_avg_limit=avg_limit,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile,csvformat=csvformat,insitu=True)
                        elif (blfunc == 'chebyshev'):
                                s.auto_chebyshev_baseline(mask=msk,order=order,clipthresh=clipthresh,clipniter=clipniter,edge=edge,threshold=thresh,chan_avg_limit=avg_limit,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile,csvformat=csvformat,insitu=True)
                        elif (blfunc == 'cspline'):
                                s.auto_cspline_baseline(mask=msk,npiece=npiece,clipthresh=clipthresh,clipniter=clipniter,edge=edge,threshold=thresh,chan_avg_limit=avg_limit,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile,csvformat=csvformat,insitu=True)
                        elif (blfunc == 'sinusoid'):
                                s.auto_sinusoid_baseline(mask=msk,applyfft=applyfft,fftmethod=fftmethod,fftthresh=fftthresh,addwn=addwn,rejwn=rejwn,clipthresh=clipthresh,clipniter=clipniter,edge=edge,threshold=thresh,chan_avg_limit=avg_limit,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile,csvformat=csvformat,insitu=True)
                else:
                        if (blfunc == 'poly'):
                                s.poly_baseline(mask=msk,order=order,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile,csvformat=csvformat,insitu=True)
                        elif (blfunc == 'chebyshev'):
                                s.chebyshev_baseline(mask=msk,order=order,clipthresh=clipthresh,clipniter=clipniter,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile,csvformat=csvformat,insitu=True)
                        elif (blfunc == 'cspline'):
                                s.cspline_baseline(mask=msk,npiece=npiece,clipthresh=clipthresh,clipniter=clipniter,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile,csvformat=csvformat,insitu=True)
                        elif (blfunc == 'sinusoid'):
                                s.sinusoid_baseline(mask=msk,applyfft=applyfft,fftmethod=fftmethod,fftthresh=fftthresh,addwn=addwn,rejwn=rejwn,clipthresh=clipthresh,clipniter=clipniter,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile,csvformat=csvformat,insitu=True)

                # the above 14 lines will eventually shrink into the following 2 commands:
                #
                #sbinfo = s.create_sbinfo(blfunc=blfunc,order=order,npiece=npiece,applyfft=applyfft,fftmethod=fftmethod,fftthresh=fftthresh,addwn=addwn,rejwn=rejwn,\
                #                         masklist=masklist,maskmode=maskmode,edge=edge,threshold=threshold,chan_avg_limit=chan_avg_limit,\
                #                         clipthresh=clipthresh,clipniter=clipniter)
                #s.sub_baseline(sbinfo=sbinfo,plot=verify,showprogress=showprogress,minnrow=minnrow,outlog=verbose,blfile=blfile)
                #
                # where
                # sbinfo = {'func':funcinfo, 'mask':maskinfo, 'clip':clipinfo}
                # and
                # funcinfo should be one of the follows:
                #     funcinfo = {'type':'poly', 'params':{'order':order}}
                #     funcinfo = {'type':'cspline', 'params':{'npiece':npiece}}
                #     funcinfo = {'type':'sspline', 'params':{'lambda':lambda}}
                #     funcinfo = {'type':'sinusoid', 'params':{'applyfft':applyfft, 'fftmethod':fftmethod, 'fftthresh':fftthresh, 'addwn':addwn, 'rejwn':rejwn}}
                # maskinfo should be one of the follows:
                #     maskinfo = {'base':masklist, 'aux':{'type':'auto', 'params':{'edge':edge, 'threshold':thresh, 'chan_avg_limit':avg_limit}}}
                #     maskinfo = {'base':masklist, 'aux':{'type':'list'}}
                #     maskinfo = {'base':masklist, 'aux':{'type':'interactive'}}
                # clipinfo should be:
                #     clipinfo = {'clipthresh':clipthresh, 'clipniter':clipniter}

                del msk

                # reset selection
                if len(sif) > 0: s.set_selection(basesel)

        ### END of IF loop
        del basesel

def prior_plot(s, plotlevel):
    if ( abs(plotlevel) > 1 ):
        casalog.post( "Initial Raw Scantable:" )
        s._summary()

def posterior_plot(s, project, plotlevel):
    if ( abs(plotlevel) > 0 ):
        pltfile=project+'_bsspec.eps'
        sdutil.plot_scantable(s, pltfile, plotlevel)
