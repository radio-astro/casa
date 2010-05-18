# Single dish observation simulator
# KS todo TP simulation for more than one antenae. what happens? 
import os
import asap as sd
from taskinit import * 
from simutil import *

#def sdsim(modelimage, ignorecoord, inbright, antennalist, antenna,  project, refdate, integration, startfreq, chanwidth, nchan, direction, pointingspacing, relmargin, cell, imsize, stokes, noise_thermal, t_atm, tau0, verbose):


def sdsim(
    project=None, 
#setup=None, 
#    complist=None, 
    modelimage=None, inbright=None, ignorecoord=None,
    # nchan=None, # removed - possible complist issues
    startfreq=None, chanwidth=None,
    refdate=None, totaltime=None, integration=None, 
    scanlength=None, # will be removed
    direction=None, pointingspacing=None, mosaicsize=None, # plotfield=None,
#    caldirection=None, calflux=None,
    checkinputs=None, # will be removed
#predict=None, 
    antennalist=None, 
    antenna=None,
#    ptgfile=None, plotuv=None, plotconfig=None,
#process=None, 
    noise_thermal=None, 
#    noise_mode=None, #will be removed
#    user_pwv=None, t_ground=None,
    t_sky=None, tau0=None, 
#    cross_pol=None,
#image=None, 
#    cleanmode=None,
    cell=None, imsize=None, #threshold=None, niter=None, 
#    weighting=None, outertaper=None, 
    stokes=None, 
#    psfmode=None, weighting=None, robust=None, uvtaper=None, outertaper=None, innertaper=None, noise=None, npixels=None, stokes=None, # will be removed
#    plotimage=None, cleanresidual=None, 
#analyze=None, 
#    imagename=None, originalsky=None, convolvedsky=None, difference=None, 
    fidelity=None, 
    display=None, # will be removed
#    plotpsf=None
    verbose=None, async=False
    ):


    casalog.origin('sdsim')
    if verbose: casalog.filter(level="DEBUG2")

    try:

        # create the utility object:
        #util=simutil(direction,startfreq=qa.quantity(startfreq),
        #             verbose=verbose)
        util=simutil(startfreq=qa.quantity(startfreq),verbose=verbose)
        msg=util.msg

        # file check 
        if not os.path.exists(modelimage):
            msg("modelimage '%s' is not found." % modelimage,priority="error")
            
        ##################################################################
        # determine where the observation will occur:
        #nfld, pointings, etime = util.calc_pointings(pointingspacing,mosaicsize,direction)
        nfld, pointings, etime = _calc_lattice_pointings(util,pointingspacing,mosaicsize,direction)

        # find imcenter - phase center
        imcenter , offsets = util.average_direction(pointings)
        util.direction=imcenter
        epoch, ra, dec = util.direction_splitter(imcenter)

        #########################################################
        # input cell size (only used in setup if ignorecoord=T)

        if cell=="incell":
            if ignorecoord:
                msg("You can't use the input header for the pixel size if you don't have an input header!",priority="error")
                return False
            else:
                in_cell=qa.quantity('0arcsec')
                in_cell=[in_cell,in_cell]
        else:
            if type(cell) == list:
                in_cell =  map(qa.convert,cell,['arcsec','arcsec'])
            else:
                in_cell = qa.convert(cell,'arcsec')            
                in_cell = [in_cell,in_cell]
        

        #####
        # simulate from components here?

        ###########################################################
        # convert original model image to 4d shape:        

        # truncate model image name to craete new images in current dir:
        (modelimage_path,modelimage_local) = os.path.split(os.path.normpath(modelimage))
        modelimage_local=modelimage_local.strip()
        if modelimage_local.endswith(".fits"):
            modelimage_local=modelimage_local.replace(".fits","")
        if modelimage_local.endswith(".FITS"):
            modelimage_local=modelimage_local.replace(".FITS","")
        if modelimage_local.endswith(".fit"):
            modelimage_local=modelimage_local.replace(".fit","")

        # recast into 4d form
        modelimage4d=project+"."+modelimage_local+'.coord'
        # need this filename whether or not we create the output image
        modelregrid=project+"."+modelimage_local+".flat"
        # modelflat should be the moment zero of that
        modelflat=project+"."+modelimage_local+".flat0"

        (ra,dec,model_cell,nchan,startfreq,chanwidth,
         model_stokes) = util.image4d(modelimage,modelimage4d,
                                      inbright,ignorecoord,
                                      ra,dec,in_cell,startfreq,chanwidth,
                                      flatimage=modelflat)
        # nchan freq etc used in predict
        casalog.origin('simdata')

        # out_cell will get used in clean
        # in_cell was defined above from cell, and not overridden
        if cell=="incell":
            if ignorecoord:
                print "you should not be here"
            else:
                out_cell=model_cell
        else:
            out_cell=in_cell
        
        # set startfeq and bandwidtg in util object after treating model image
        bandwidth=qa.mul(qa.quantity(nchan),qa.quantity(chanwidth))
        util.bandwidth=bandwidth


        ##################################################################
        # read antenna file:

        stnx, stny, stnz, stnd, padnames, nant, telescopename = util.readantenna(antennalist)
        
        ant=' '.join(' '.join(''.join(antenna.split()).split(',')).split(';'))
        if ant=='' and nant==1: ant='0'
        if ant=='' or len(ant.split(' ')) != 1 or ant.find('&') != -1:
            raise ValueError, 'Select a antenna number.'
        ant=int(ant)
        if (ant > nant): raise ValueError, 'Invalid antenna ID.'
        
        stnx=[stnx[ant]]
        stny=[stny[ant]]
        stnz=[stnz[ant]]
        stnd=[stnd[ant]]
        padnames=[padnames[ant]]
        antnames=[telescopename]
        if nant > 1: antnames=[telescopename+('%02dTP'%ant)]
        nant = 1
        aveant=stnd

        # (set back to simdata - there must be an automatic way to do this)
        casalog.origin('simdata')


        # move this to "image" section?
        ## For single dish simulation
        imagermode=''
        ftmachine="sd"
        msg("map center = " + imcenter)
        if verbose: 
            for dir in pointings:
                msg("   "+dir)


        ##################################################################
        # check inputs - need to add atmospheric window, better display of
        # where the actual observation block lies on the ephemeris window

        if checkinputs=="yes" or checkinputs=="only":
            currfignum=0
            pl.figure(currfignum)
            pl.ion()
            pl.clf()
            pl.subplot(121)
            model_min,model_max, model_rms = util.statim(modelflat,plot=True,incell=model_cell)
            lims=pl.xlim(),pl.ylim()
            #tt=pl.array(range(25))*pl.pi/12
            pb=1.2*0.3/qa.convert(qa.quantity(startfreq),'GHz')['value']/aveant[0]*3600.*180/pl.pi
            #if max(max(lims)) > pb/2:
            #    plotcolor='w'
            #else:
            #    plotcolor='k'
            #for i in range(offsets.shape[1]):
            #    pl.plot(pl.cos(tt)*pb/2+offsets[0,i]*3600,pl.sin(tt)*pb/2+offsets[1,i]*3600,plotcolor)
            pl.plot(offsets[0]*3600.,offsets[1]*3600.,'w+',markeredgewidth=1)#markersize=pb/8
            if max(max(lims)) > pb/2:
                plotpb(pb,pl.gca(),lims=lims)
            else:
                from matplotlib.patches import Circle
                for i in range(offsets.shape[1]):
                    pl.gca().add_artist(Circle(
                        (offsets[0,i]*3600,+offsets[1,i]*3600),
                        radius=pb/2.,edgecolor='k',fill=False,
                        label='beam',transform=pl.gca().transData))
            xlim=max(abs(pl.array(lims[0])))
            ylim=max(abs(pl.array(lims[1])))
            # show entire pb: (statim doesn't by default)
            pl.xlim([max([xlim,pb/2]),min([-xlim,-pb/2])])
            pl.ylim([min([-ylim,-pb/2]),max([ylim,pb/2])])
            pl.text(0,max([ylim,pb/2])*1.2,"regridded model:",horizontalalignment='center')
            # ephemeris:
            #pl.subplot(222)
            pl.subplot(122)
            util.ephemeris(refdate)  # util already knows the direction from above
            
            #pl.subplot(224)
            #util.plotants(stnx, stny, stnz, stnd, padnames)
            #ax=pl.gca()
            #l=ax.get_xticklabels()
            #pl.setp(l,fontsize="x-small")
            #l=ax.get_yticklabels()
            #pl.setp(l,fontsize="x-small")
            #pl.xlabel(telescopename,fontsize="x-small")
            
            pl.subplots_adjust(left=0.05,right=0.98,bottom=0.09,top=0.95,hspace=0.2,wspace=0.2)
            
            if checkinputs=="only":
                msg("Stopping after checking inputs as requested",priority="warn")
                return
            else:
                if display==True:
                    pl.figure(currfignum+1)
                    pl.clf()
        else:
#            model_min,model_max, model_rms = util.statim(modelimage,plot=False,incell=model_cell)
            model_min,model_max, model_rms = util.statim(modelflat,plot=False,incell=in_cell)

        casalog.origin('simdata')


        ##################################################################
        # set up observatory, feeds, etc:
        # (has to be here since we may have changed nchan)
        
        if verbose:
            msg("preparing empty measurement set",priority="warn")

        nbands = 1
        fband = 'band'+startfreq
        msfile = project+'.ms'

        sm.open(msfile)
        posobs=me.observatory(telescopename)
        diam=stnd;
        if not isinstance(diam, list): diam=diam.tolist()
        # Add rows to ANTENNA table
        #sm.setconfig(telescopename=telescopename, x=stnx, y=stny, z=stnz, 
        #             dishdiameter=diam, 
        #             mount=['alt-az'], antname=antnames,
        #             coordsystem='global', referencelocation=posobs)
        sm.setconfig(telescopename=telescopename, x=stnx, y=stny, z=stnz, 
                     dishdiameter=diam, 
                     mount=['alt-az'], antname=antnames,padname=padnames,
                     coordsystem='global', referencelocation=posobs)
        #
        sm.setspwindow(spwname=fband, freq=startfreq, deltafreq=chanwidth, 
                       freqresolution=chanwidth, nchannels=nchan, 
                       #stokes=stokes)
                       stokes='XX YY')
        # Add rows to FEED table
        sm.setfeed(mode='perfect X Y',pol=[''])
        sm.setlimits(shadowlimit=0.01, elevationlimit='10deg')
        # auto-correlation should be unity for single dish obs.
        sm.setauto(1.0)
        for k in range(0,nfld):
            src=project+'_%d'%k
            sm.setfield(sourcename=src, sourcedirection=pointings[k],
                        calcode="C", distance='0m')
        reftime = me.epoch('TAI', refdate)
        sm.settimes(integrationtime=integration, usehourangle=True, 
                    referencetime=reftime)
        totaltime=qa.mul(qa.quantity(integration),qa.quantity(nfld))
        totalsec=qa.convert(qa.quantity(totaltime),'s')['value']
        scansec=qa.convert(qa.quantity(integration),'s')['value']
        for k in range(0,nfld) :
            sttime=-totalsec/2.0+scansec*k
            endtime=sttime+scansec
            src=project+'_%d'%k
            # this only creates blank uv entries
            sm.observe(sourcename=src, spwname=fband,
                       starttime=qa.quantity(sttime, "s"),
                       stoptime=qa.quantity(endtime, "s"));
        sm.setdata(fieldid=range(0,nfld))
        sm.setvp()

        msg("done setting up observations (blank visibilities)")
        if verbose:
            sm.summary()
        

        ##################################################################
        # do actual calculation of visibilities from the model image:
        
        # image should now have 4 axes and same size as output
        # XXX at least in xy (check channels later)        
        msg("predicting from "+modelimage4d,priority="warn")
        #sm.setoptions(gridfunction='pb', ftmachine='sd', cache=100000)
        #sm.setoptions(gridfunction='pb', ftmachine='sd', location=posobs, cache=100000)
        sm.setoptions(gridfunction='pb', ftmachine=ftmachine, location=posobs)
        sm.predict(imagename=[modelimage4d])
        sm.done()
        sm.close()

        # modify STATE table information for ASAP
        # Ugly part!! need improvement. 
        nstate=1
        tb.open(tablename=msfile+'/STATE',nomodify=False)
        tb.addrows(nrow=nstate)
        tb.putcol(columnname='CAL',value=[0.]*nstate,startrow=0,nrow=nstate,rowincr=1)
        tb.putcol(columnname='FLAG_ROW',value=[False]*nstate,startrow=0,nrow=nstate,rowincr=1)
        tb.putcol(columnname='LOAD',value=[0.]*nstate,startrow=0,nrow=nstate,rowincr=1)
        tb.putcol(columnname='REF',value=[False]*nstate,startrow=0,nrow=nstate,rowincr=1)
        tb.putcol(columnname='SIG',value=[True]*nstate,startrow=0,nrow=nstate,rowincr=1)
        tb.putcol(columnname='SUB_SCAN',value=[0]*nstate,startrow=0,nrow=nstate,rowincr=1)
        tb.flush()
        tb.close()
            
        tb.open(tablename=msfile,nomodify=False)
        tb.putcol(columnname='STATE_ID',value=[0]*nfld,startrow=0,nrow=nfld,rowincr=1)
        tb.flush()
        tb.close()
        
        msg('generation of measurement set ' + msfile + ' complete.')


        ###################################################################
        # noisify

        noise_any=False
            
        if noise_thermal:
            if not (util.telescopename == 'ALMA' or util.telescopename == 'ACA'):
                msg("thermal noise only works properly for ALMA/ACA",priority="warn",origin="noise")
                
            noise_any=True

            noisymsfile = project + ".noisy.ms"
            noisymsroot = project + ".noisy"
            msg('adding thermal noise to ' + noisymsfile,origin="noise",priority="warn")

            eta_p, eta_s, eta_b, eta_t, eta_q, t_rx = util.noisetemp()

            # antenna efficiency
            eta_a = eta_p * eta_s * eta_b * eta_t
            if verbose: 
                msg('antenna efficiency    = ' + str(eta_a),origin="noise")
                msg('spillover efficiency  = ' + str(eta_s),origin="noise")
                msg('correlator efficiency = ' + str(eta_q),origin="noise")
 
            # Cosmic background radiation temperature in K. 
            t_cmb = 2.275

            if os.path.exists(noisymsfile):
                cu.removetable(noisymsfile)
            os.system("cp -r "+msfile+" "+noisymsfile)
            # RI todo check for and copy flagversions file as well
            
            sm.openfromms(noisymsfile);    # an existing MS
            sm.setdata(fieldid=range(0,nfld))
            #sm.setapply(type='TOPAC',opacity=tau0);  # opac corruption
            # SimACohCalc needs 2-temp formula not just t_atm
            #sm.setnoise(mode="calculate",table=noisymsroot,
            #            antefficiency=eta_a,correfficiency=eta_q,
            #            spillefficiency=eta_s,tau=tau0,trx=t_rx,
            #            tatmos=t_atm,tcmb=t_cmb)
            sm.oldsetnoise(mode="calculate",table=noisymsroot,
                           antefficiency=eta_a,correfficiency=eta_q,
                           spillefficiency=eta_s,tau=tau0,trx=t_rx,
                           tatmos=t_sky,tcmb=t_cmb)
            # use ANoise version
            #sm.setnoise2(mode="calculate",table=noisymsroot,
            #             antefficiency=eta_a,correfficiency=eta_q,
            #             spillefficiency=eta_s,tau=tau0,trx=t_rx,
            #             tatmos=t_atm,tground=t_ground,tcmb=t_cmb)
            sm.corrupt();
            sm.done();

            if verbose: msg("done corrupting with thermal noise",origin="noise")


        ##################################################################
        # Imaging            

        if noise_any:
            mstoimage = noisymsfile
        else:
            mstoimage = msfile

        msg('creating image from generated ms: '+mstoimage)
        #im.open(msfile)
        im.open(mstoimage)
        im.selectvis(nchan=nchan,start=0,step=1,spw=0)
        im.defineimage(mode='channel',nx=imsize[0],ny=imsize[1],cellx=cell,celly=cell,phasecenter=imcenter,nchan=nchan,start=0,step=1,spw=0)
        #im.setoptions(ftmachine='sd',gridfunction='pb')
        im.setoptions(ftmachine=ftmachine,gridfunction='pb')
        outimage=project+'.image'
        image=project
        im.makeimage(type='singledish',image=outimage)
        im.close()

        # For single dish: manually set the primary beam
        ia.open(outimage)
        beam=ia.restoringbeam()
        if len(beam) == 0:
            msg('setting primary beam information to image.')
            pb=1.2*0.3/qa.convert(qa.quantity(startfreq),'GHz')['value']/aveant[0]*3600.*180/pl.pi
            beam['major'] = beam['minor'] = qa.quantity(pb,'arcsec')
            beam['positionangle'] = qa.quantity(0.0,'deg')
            msg('Primary beam: '+str(beam['major']))
            ia.setrestoringbeam(beam=beam)
        ia.done()
        del beam

        msg('generation of image ' + outimage + ' complete.')

        # flat output -- needed even if fidelity is not calculated
        if ftmachine=='sd':
            ia.open(image+".image")
            outimsize=ia.shape()
            outimcsys=ia.coordsys()
            ia.done()
            outspectax=outimcsys.findcoordinate('spectral')['pixel']
            outnchan=outimsize[outspectax]
            outstokesax=outimcsys.findcoordinate('stokes')['pixel']
            outnstokes=outimsize[outstokesax]

            outflat=image+".flat"
            if outnchan>1:
                if verbose: msg("creating moment zero output image",origin="analysis")
                ia.open(image+".image")
                ia.moments(moments=[-1],outfile=outflat,overwrite=True)
                ia.done()
            else:
                if verbose: msg("removing degenerate output image axes",origin="analysis")
                # just remove degenerate axes from image
                ia.newimagefromimage(infile=image+".image",outfile=outflat,dropdeg=True,overwrite=True)
                # seems no way to just drop the spectral and keep the stokes. 
                if outnstokes<=1:
                    os.rename(outflat,outflat+".tmp")
                    ia.open(outflat+".tmp")
                    ia.adddegaxes(outfile=outflat,stokes='I',overwrite=True)
                    ia.done()
                    shutil.rmtree(outflat+".tmp")
            if outnstokes>1:
                os.rename(outflat,outflat+".tmp")
                po.open(outflat+".tmp")
                foo=po.stokesi(outfile=outflat,stokes='I')
                foo.done()
                po.done()
                shutil.rmtree(outflat+".tmp")
            # be sure to get outflatcoordsys from outflat
            ia.open(outflat)
            outflatcoordsys=ia.coordsys()
            outflatshape=ia.shape()
            ia.done()            
    
            # regrid flat input to flat output shape, for convolution, etc
            ia.open(modelflat)
            ia.regrid(outfile=modelregrid+'.tmp', overwrite=True,
                             csys=outflatcoordsys.torecord(),shape=outflatshape)
            # im.regrid assumes a surface brightness, or more accurately doesnt
            # pay attention to units at all, so we now have to scale 
            # by the pixel size to have the right values in jy/pixel, 
            # which is what the immath assumes below.            
            factor  = (qa.convert(out_cell[0],"arcsec")['value'])  
            factor *= (qa.convert(out_cell[1],"arcsec")['value']) 
            factor /= (qa.convert(model_cell[0],"arcsec")['value']) 
            factor /= (qa.convert(model_cell[1],"arcsec")['value']) 

            imrr = ia.imagecalc(modelregrid, 
                                "'%s'*%g" % (modelregrid+'.tmp',factor), 
                                overwrite = True)
            shutil.rmtree(modelregrid+".tmp")
            if verbose:
                msg("scaling model by pixel area ratio %g" % factor)

            # add clean components and model image; 
            # it'll be convolved to restored beam in the fidelity calc below
            # components are in jy/pix so should be added to the scaled iamge
            #if (os.path.exists(complist)):
            #    cl.open(complist)
            #    imrr.modify(cl.torecord(),subtract=False)
            #    cl.done()

            imrr.done()
    

    
            ia.done()
            outflatcoordsys.done()
            del outflatcoordsys
            del imrr
    
            
            shutil.rmtree(modelflat)  # no need to keep this
                    
            msg("done inverting and cleaning")

            # get beam from output clean image
            if verbose: msg("getting beam from "+image+".image",origin="analysis")
            ia.open(image+".image")
            beam=ia.restoringbeam()
            ia.done()

        #####################################################################
        # fidelity  

        if os.path.exists(modelimage) and fidelity == True: 
            # from math import sqrt
            
            # Convolve model with beam.
            convolved = project + '.convolved.im'
            ia.open(modelregrid)
            ia.convolve2d(convolved,major=beam['major'],minor=beam['minor'],
                          pa=beam['positionangle'],overwrite=True)

            # imagecalc is interpreting outflat in Jy/pix - 
            # the diff is v. negative.
            bmarea=beam['major']['value']*beam['minor']['value']*1.1331 #arcsec2
            bmarea=bmarea/(out_cell[0]['value']*out_cell[1]['value']) # bm area in pix
            msg("primary beam area in output pixels = %f" % bmarea)
            # Make difference image.
            difference = project + '.diff.im'
            ia.imagecalc(difference, "'%s' - ('%s'/%g)" % (convolved, outflat,bmarea), overwrite = True)
            ia.done()
            # Get rms of difference image.
            ia.open(difference)
            diffstats = ia.statistics(robust = True)
            ia.done()
            maxdiff=diffstats['medabsdevmed']            
            if maxdiff!=maxdiff: maxdiff=0.
            if type(maxdiff)!=type(0.):
                if maxdiff.__len__()>0: 
                    maxdiff=maxdiff[0]
                else:
                    maxdiff=0.
            # Make fidelity image.
            absdiff = project + '.absdiff.im'
            ia.imagecalc(absdiff, "max(abs('%s'), %f)" % (difference,
                                                          maxdiff/pl.sqrt(2.0)), overwrite = True)
            fidelityim = project + '.fidelity.im'
            ia.imagecalc(fidelityim, "abs('%s') / '%s'" % (convolved, absdiff), overwrite = True)
            ia.done()

            msg("fidelity image calculated",origin="analysis")

        else:
            bmarea=1.

            # clean up moment zero maps if we don't need them anymore
#            if nchan==1:
#                sh.rmtree(outflat) 





        #####################################################################
        # display and statistics:

        # plot whichever image we have - preference is for noisy and cleaned
        if display==True:
            pl.ion()
            pl.clf()
            if fidelity == True:
                pl.subplot(222) #pl.subplot(232)
            else:
                pl.subplot(122) #pl.subplot(222)
        if ftmachine=='sd':
            max_cleanim=[] # mutable so can be returned
            sim_min,sim_max,sim_rms = util.statim(outflat,plot=display,disprange=max_cleanim)
            # max_cleanim returned in outflat units i.e. Jy/bm > jy/pix
            max_cleanim[0]/=bmarea
        # plot model image if exists
        if os.path.exists(modelimage) and display==True:
            if fidelity == True:
                pl.subplot(221) #pl.subplot(231)
            else:
                pl.subplot(121) #pl.subplot(221)
            # modelregrid might not exist if there's no display or fidelity
            if os.path.exists(modelregrid):
                model_min,model_max, model_rms = util.statim(modelregrid,plot=display)
                xlim=max(pl.xlim())
                ylim=max(pl.ylim())
            if display==True:
                #tt=pl.array(range(25))*pl.pi/12
                #pb=1.2*0.3/qa.convert(qa.quantity(startfreq),'GHz')['value']/aveant*3600.*180/pl.pi
                ## RI todo change this to use tb.open(ms/POINTINGS)/direction
                ## and them make it a helper function
                #for i in range(offsets.shape[1]):
                #    pl.plot(pl.cos(tt)*pb/2+offsets[0,i]*3600,pl.sin(tt)*pb/2+offsets[1,i]*3600,'w')
                pl.plot(offsets[0]*3600.,offsets[1]*3600.,'w+',markeredgewidth=1)
                pl.xlim([xlim,-xlim])
                pl.ylim([-ylim,ylim])
                plotpb(pb,pl.gca())

            
            # fidelity image would only exist if there's a model image
            if modelimage != '' and fidelity == True:
                if display: pl.subplot(223)  #pl.subplot(233)                
                util.statim(project+".diff.im",plot=display,disprange=max_cleanim)
                if display: pl.subplot(224) #pl.subplot(234)
                fidel_min, fidel_max, fidel_rms = util.statim(project+".fidelity.im",plot=display)

        if display:
#             tb.open(mstoimage)
#             # RI todo use rob's FFT of the PB instead
#             rawdata=tb.getcol("UVW")
#             tb.done()
#             if fidelity == True:
#                 pl.subplot(235)
#             else:
#                 pl.subplot(223)
#             pl.box()
#             maxbase=max([max(rawdata[0,]),max(rawdata[1,])])  # in m
#             klam_m=300/qa.convert(qa.quantity(startfreq),'GHz')['value']
#             pl.plot(rawdata[0,]/klam_m,rawdata[1,]/klam_m,'b,')
#             pl.plot(-rawdata[0,]/klam_m,-rawdata[1,]/klam_m,'b,')
#             ax=pl.gca()
#             ax.yaxis.LABELPAD=-4
#             l=ax.get_xticklabels()
#             pl.setp(l,fontsize="x-small")
#             l=ax.get_yticklabels()
#             pl.setp(l,fontsize="x-small")
#             pl.xlabel('u[klambda]',fontsize='x-small')
#             pl.ylabel('v[klambda]',fontsize='x-small')
#             pl.axis('equal')

#             if modelimage != '' and doclean:   
#                 if fidelity == True:
#                     pl.subplot(236)
#                 else:
#                     pl.subplot(224)
#                 ia.open(image+".psf")
#                 beamcs=ia.coordsys()
#                 beam_array=ia.getchunk(axes=[beamcs.findcoordinate("spectral")['pixel'],beamcs.findcoordinate("stokes")['pixel']],dropdeg=True)
#                 if type(cell)==type([]):
#                     pixsize=(qa.convert(qa.quantity(cell[0]),'arcsec')['value'])
#                 else:
#                     pixsize=(qa.convert(qa.quantity(cell),'arcsec')['value'])
#                 xextent=imsize[0]*pixsize*0.5
#                 xextent=[xextent,-xextent]
#                 yextent=imsize[1]*pixsize*0.5
#                 yextent=[-yextent,yextent]
#                 flipped_array=beam_array.transpose()
#                 ttrans_array=flipped_array.tolist()
#                 ttrans_array.reverse()
#                 pl.imshow(ttrans_array,interpolation='bilinear',cmap=pl.cm.jet,extent=xextent+yextent,origin="bottom")
#                 pl.title(image+".psf",fontsize="x-small")
#                 b=qa.convert(beam['major'],'arcsec')['value']
#                 pl.xlim([-3*b,3*b])
#                 pl.ylim([-3*b,3*b])
#                 ax=pl.gca()
#                 l=ax.get_xticklabels()
#                 pl.setp(l,fontsize="x-small")
#                 l=ax.get_yticklabels()
#                 pl.setp(l,fontsize="x-small")
#                 #from matplotlib.font_manager import fontManager, FontProperties
#                 #font= FontProperties(size='x-small');
#                 #pl.legend(("bmaj=%7.1e" % beam['major']['value'],"bmin=%7.1e" % beam['minor']['value']),prop=font)                
#                 pl.text(0.05,0.95,"bmaj=%7.1e\nbmin=%7.1e" % (beam['major']['value'],beam['minor']['value']),transform = ax.transAxes,bbox=dict(facecolor='white', alpha=0.7),size="x-small",verticalalignment="top")
#                 ia.done()

                
            pl.subplots_adjust(left=0.05,right=0.98,bottom=0.08,top=0.96,hspace=0.1,wspace=0.2)
        
        # if not displaying still print stats:
        if ftmachine=='sd':
            bmarea=beam['major']['value']*beam['minor']['value']*1.1331 #arcsec2
            bmarea=bmarea/(out_cell[0]['value']*out_cell[1]['value']) # bm area in pix
            msg('Simulation rms: '+str(sim_rms)+" Jy/pix = "+
                str(sim_rms*bmarea)+" Jy/bm",origin="analysis")
            msg('Simulation max: '+str(sim_max)+" Jy/pix = "+
                str(sim_max*bmarea)+" Jy/bm",origin="analysis")
        
        if display == True or fidelity == True:
            msg('Model rms: '+str(model_rms)+" Jy/pix",origin="analysis")
            msg('Model max: '+str(model_max)+" Jy/pix",origin="analysis")
            msg('Beam bmaj: '+str(beam['major']['value'])+' bmin: '+str(beam['minor']['value'])+' bpa: '+str(beam['positionangle']['value']),origin="analysis")


    except TypeError, e:
        print 'TypeError: ', e
        return
    except ValueError, e:
        print 'OptionError: ', e
        return
    except Exception, instance:
        print '***Error***', instance
        return


##### Helper functions to calculate lattice gridding
def _calc_lattice_pointings(util, spacing, imsize, direction=None, relmargin=0.33):
    if type(imsize) != type([]):
        imsize=[imsize,imsize]
    if len(imsize) <2:
        imsize=[imsize[0],imsize[0]]
    if direction==None:
        # if no direction is specified, use the object's direction
        direction=util.direction
    else:
        # if one is specified, use that to set the object's direction
        # do we really want to do this?
        util.direction=direction

    # direction is always a list of strings (defined by .xml)
    if len(direction) >1:
        if util.verbose: util.msg("You are inputing the precise pointings in 'direction' - if you want me to calculate a mosaic, give a single direction",priority="warn")
        return len(direction), direction, [0.]*len(direction) #etime at end
    else: direction=direction[0]

    # now its either a filename or a single direction:
    # do we need this string check?  maybe it could be a quantity?
    #if type(direction) == str:
    # Assume direction as a filename and read lines if it exists
    filename=os.path.expanduser(os.path.expandvars(direction))
    if os.path.exists(filename):
        util.msg('Reading direction information from the file, %s' % filename)
        return util.read_pointings(filename)
        
    epoch, centx, centy = util.direction_splitter()

    spacing  = qa.quantity(spacing)
    yspacing = spacing
    
    xsize=qa.quantity(imsize[0])
    ### for lattice
    ysize=qa.quantity(imsize[1])

    ### for lattice
    nrows = 1+ int(pl.floor(qa.convert(qa.div(ysize, yspacing), '')['value']
                                - 2.0 * relmargin))

    availcols = 1 + qa.convert(qa.div(xsize, spacing), '')['value'] \
                - 2.0 * relmargin
    ncols = int(pl.floor(availcols))


    ### for lattice
    ncolstomin = 0.5 * (ncols - 1)                           # O O O O
    pointings = []                                           # O O O O

    # Start from the top because in the Southern hemisphere it sets first.
    y = qa.add(centy, qa.mul(0.5 * (nrows - 1), yspacing))
    for row in xrange(0, nrows):         # xrange stops early.
        xspacing = qa.mul(1.0 / pl.cos(qa.convert(y, 'rad')['value']),spacing)
        ystr = qa.formxxx(y, format='dms')

        ### for lattice
        xmin = qa.sub(centx, qa.mul(ncolstomin, xspacing))
        stopcolp1 = ncols
        
        for col in xrange(0, stopcolp1):        # xrange stops early.
            x = qa.formxxx(qa.add(xmin, qa.mul(col, xspacing)),
                           format='hms')
            pointings.append("%s%s %s" % (epoch, x, ystr))
        y = qa.sub(y, yspacing)
    ####could not fit pointings then single pointing
    if(len(pointings)==0):
        pointings.append(direction)
    util.msg("using %i generated pointing(s)" % len(pointings))
    util.pointings=pointings
    return len(pointings), pointings, [0.]*len(pointings)


##### Helper functions to plot primary beam
def plotpb(pb,axes,lims=None):
    # This beam is automatically scaled when you zoom in/out but
    # not anchored in plot area. We'll wait for Matplotlib 0.99
    # for that function. 
    #major=major
    #minor=minor
    #rangle=rangle
    #bwidth=max(major*pl.cos(rangle),minor*pl.sin(rangle))*1.1
    #bheight=max(major*pl.sin(rangle),minor*pl.cos(rangle))*1.1
    boxsize=pb*1.1
    if not lims: lims=axes.get_xlim(),axes.get_ylim()
    incx=1
    incy=1
    if axes.xaxis_inverted(): incx=-1
    if axes.yaxis_inverted(): incy=-1
    #ecx=lims[0][0]+bwidth/2.*incx
    #ecy=lims[1][0]+bheight/2.*incy
    ccx=lims[0][0]+boxsize/2.*incx
    ccy=lims[1][0]+boxsize/2.*incy
    from matplotlib.patches import Rectangle, Circle #,Ellipse
    #box=Rectangle((lims[0][0],lims[1][0]),incx*bwidth,incy*bheight,
    box=Rectangle((lims[0][0],lims[1][0]),incx*boxsize,incy*boxsize,
                  alpha=0.7,facecolor='w',
                  transform=axes.transData) #Axes
    #beam=Ellipse((ecx,ecy),major,minor,angle=rangle,
    beam=Circle((ccx,ccy), radius=pb/2.,
                 edgecolor='k',fill=False,
                 label='beam',transform=axes.transData)
    #props={'pad': 3, 'edgecolor': 'k', 'linewidth':2, 'facecolor': 'w', 'alpha': 0.5}
    #pl.matplotlib.patches.bbox_artist(beam,axes.figure.canvas.get_renderer(),props=props)
    axes.add_artist(box)
    axes.add_artist(beam)

    
