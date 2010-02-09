# (partial) TODO list 
# RI todo Find timerange when at least one of the inputs is up, limit the
#       observing time to it, and warn or abort (printing the valid
#       timerange) depending on how badly the user missed it.
# RI todo look for antenna list in default repository also
# RI todo allow user to report statistics in Jy/bm instead of Jy/pixel
# RI todo Pad input image to a composite number of pixels on a side. 



import os
from clean import clean
from taskinit import *
from simutil import *
import pylab as pl
import pdb


def simdata2(modelimage=None, ignorecoord=None, inbright=None, incell=None, complist=None, antennalist=None, plotfield=None, predict=None, ptgfile=None, project=None, refdate=None, totaltime=None, integration=None, scanlength=None, startfreq=None, chanwidth=None, nchan=None, direction=None, pointingspacing=None, relmargin=None, cell=None, imsize=None, niter=None, threshold=None, psfmode=None, weighting=None, robust=None, uvtaper=None, outertaper=None, innertaper=None, noise=None, npixels=None, stokes=None, noise_thermal=None, noise_mode=None, user_pwv=None, t_ground=None, t_sky=None, tau0=None, fidelity=None, display=None, verbose=False, async=None):


    casalog.origin('simdata')
    if verbose: casalog.filter(level="DEBUG2")

    # this is the output desired bandwidth:
    bandwidth=qa.mul(qa.quantity(nchan),qa.quantity(chanwidth))

    # create the utility object:
    util=simutil(direction,startfreq=qa.quantity(startfreq),
                 bandwidth=bandwidth)
    if verbose: util.verbose=True
    msg=util.msg
    
    msg("this task is a mockup for testing",priority="error")
    return False

    if((not os.path.exists(modelimage)) and (not os.path.exists(complist))):
        msg("No sky input found.  At least one of modelimage or complist must be set.",priority="error")
        return
    
    try:

        nbands = 1;    
        fband  = 'band'+startfreq
        msfile=project+'.ms'

        ##################################################################
        # read antenna file:

        stnx, stny, stnz, stnd, padnames, nant, telescopename = util.readantenna(antennalist)
        if stnx==False:
            return
        antnames=[]
        for k in range(0,nant): antnames.append('A%02d'%k)
        aveant=stnd.mean()

        # (set back to simdata after calls to util -
        #   there must be an automatic way to do this)
        casalog.origin('simdata')






        ##################################################################
        # if needed, make a model image from the input clean component list
        # (make it the desired 4d output shape)
        # visibilities will be calculated from the component list - this
        # image is just for display and fidelity

        if (modelimage == ''):
            
            if cell=="incell":
                msg("You can't use the input header for the pixel size if you don't have an input header!",priority="error")
                return False
            else:
                out_cell=qa.convert(cell,'arcsec')
                
            # if we are going to create an image we need a shape:
            if imsize.__len__()==1:
                imsize=[imsize,imsize]            
            out_nstk=stokes.__len__()
            out_shape=[imsize[0],imsize[1],out_nstk,nchan]
            
            if verbose: msg("creating an image from your clean components",origin="setup model")
            components_only=True
            modelimage=project+'.ccmodel'
            ia.fromshape(modelimage,out_shape,overwrite=True)
            cs=ia.coordsys()
            # use output direction to create model image:
            epoch,ra,dec=util.direction_splitter(direction)
            cs.setunits(['rad','rad','','Hz'])
            # use output cell size to create model image:
            # this is okay - if none of the clean components fit in the output
            # image, then the simulated image will be blank.  user error.
            cell_rad=qa.convert(qa.quantity(out_cell),"rad")['value']
            cs.setincrement([cell_rad,cell_rad],'direction')
            #cs.setlineartransform('direction',pl.array([[-1,0],[0,1]]))
            cs.setreferencevalue([qa.convert(ra,'rad')['value']
                                   ,qa.convert(dec,'rad')['value']],type="direction")
            cs.setreferencevalue(startfreq,'spectral')
            ia.setcoordsys(cs.torecord())
            cl.open(complist)
            ia.setbrightnessunit("Jy/pixel")
            ia.modify(cl.torecord(),subtract=False)
            cl.done()
            ia.done() # to make sure its saved to disk at the start

        else:  # we have a model image already
            components_only=False





        ##################################################################
        # convert original model image to 4d shape:

        out_cell=cell
        # if cell="incell", this will be changed to an
        # actual value by simutil.image4d below


        if not ignorecoord:
            # we are going to use the input coordinate system, so we
            # don't calculate pointings until we know the input pixel
            # size.
            # image4d will return in_cell and out_cell
            in_cell='0arcsec'
            ra=qa.quantity("0.deg")
            dec=qa.quantity("0.deg")
        else:
            # if we're ignoreing coord, then we need to calculate where
            # the pointings will be now, to move the model image there
            
            if cell=="incell":
                msg("You can't use the input header for the pixel size and also ignore the input header information!",priority="error")
                return False
            else:
                in_cell=qa.convert(cell,'arcsec')

            if type(out_cell) == list:
                cellx, celly = map(qa.quantity, out_cell)
            else:
                cellx = qa.quantity(out_cell)
                celly = cellx

            out_size = [qa.mul(imsize[0], cellx),qa.mul(imsize[1], celly)]

            if type(direction) == list:
                # manually specified pointings            
                nfld = direction.__len__()
                if nfld > 1 :
                    pointings = direction
                    if verbose: msg("You are inputing the precise pointings in 'direction' - if you want me to fill the mosaic, give a single direction",priority="warn")
                else:
                    # calculate pointings for the user 
                    nfld, pointings, etime = util.calc_pointings(pointingspacing,out_size,direction,relmargin)
            else:
                # check here if "direction" is a filename, and load it in that case
                # util.read_pointings(filename)
                # calculate pointings for the user 
                nfld, pointings, etime = util.calc_pointings(pointingspacing,out_size,direction,relmargin)
            
                
            # find imcenter=average central point, and centralpointing
            # (to be used for phase center)
            imcenter , offsets = util.average_direction(pointings)

            minoff=1e10
            central=-1
        
            for i in range(nfld):
                o=pl.sqrt(offsets[0,i]**2+offsets[1,i]**2)
                if o<minoff:
                    minoff=o
                    central=i
            centralpointing=pointings[central]
            
            epoch, ra, dec = util.direction_splitter(centralpointing)
            

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

        # this will get set by image4d if required
        out_nstk=1

        # *** ra is expected and returned in angular quantity/dict
        (ra,dec,in_cell,out_cell,
         nchan,startfreq,chanwidth,bandwidth,
         out_nstk) = util.image4d(modelimage,modelimage4d,
                                  inbright,ignorecoord,
                                  ra,dec,
                                  in_cell,out_cell,
                                  nchan,startfreq,chanwidth,bandwidth,
                                  out_nstk,
                                  flatimage=modelflat)
        
        # (set back to simdata after calls to util)
        casalog.origin('simdata')

        if verbose:
            out_shape=[imsize[0],imsize[1],out_nstk,nchan]
            msg("simulated image desired shape= %s" % out_shape,origin="setup model")
            msg("simulated image desired center= %f %f" % (qa.convert(ra,"deg")['value'],qa.convert(dec,"deg")['value']),origin="setup model")




        ##################################################################
        # set imcenter to the center of the mosaic
        # in clean, we also set to phase center; cleaning with no pointing
        # at phase center causes issues

        if type(out_cell) == list:
            cellx, celly = map(qa.quantity, out_cell)
        else:
            cellx = qa.quantity(out_cell)
            celly = cellx
        
        out_size = [qa.mul(imsize[0], cellx),qa.mul(imsize[1], celly)]

        if type(direction) == list:
            # manually specified pointings            
            nfld = direction.__len__()
            if nfld > 1 :
                pointings = direction
                if verbose: msg("You are inputing the precise pointings in 'direction' - if you want me to fill the mosaic, give a single direction",priority="warn")
            else:
                # calculate pointings for the user 
                nfld, pointings, etime = util.calc_pointings(pointingspacing,out_size,direction,relmargin)
        else:
            # check here if "direction" is a filename, and load it in that case
            # util.read_pointings(filename)
            # calculate pointings for the user 
            nfld, pointings, etime = util.calc_pointings(pointingspacing,out_size,direction,relmargin)
            
                

        # find imcenter=average central point, and centralpointing
        # (to be used for phase center)
        imcenter , offsets = util.average_direction(pointings)

        minoff=1e10
        central=-1
        
        for i in range(nfld):
            o=pl.sqrt(offsets[0,i]**2+offsets[1,i]**2)
            if o<minoff:
                minoff=o
                central=i
        centralpointing=pointings[central]
            
        # (set back to simdata after calls to util)
        casalog.origin('simdata')

        if nfld==1:
            imagermode=''
            ftmachine="ft"
            msg("phase center = " + centralpointing)
        else:
            imagermode="mosaic"
            ftmachine="mosaic"
            msg("mosaic center = " + imcenter + "; phase center = " + centralpointing)
            if verbose: 
                for dir in pointings:
                    msg("   "+dir)



        ##################################################################
        # check inputs - need to add atmospheric window, better display of
        # where the actual observation block lies on the ephemeris window
        # RI todo this should use modelflat
        
        if checkinputs=="yes" or checkinputs=="only":
            currfignum=0
            pl.figure(currfignum)
            pl.ion()
            pl.clf()
            pl.subplot(121)
            model_min,model_max, model_rms = util.statim(modelimage,plot=display,incell=in_cell)
            lims=pl.xlim(),pl.ylim()
            tt=pl.array(range(25))*pl.pi/12
            pb=1.2*0.3/qa.convert(qa.quantity(startfreq),'GHz')['value']/aveant*3600.*180/pl.pi
            if max(max(lims)) > pb/2:
                plotcolor='w'
            else:
                plotcolor='k'
            for i in range(offsets.shape[1]):
                pl.plot(pl.cos(tt)*pb/2+offsets[0,i]*3600,pl.sin(tt)*pb/2+offsets[1,i]*3600,plotcolor)
            xlim=max(abs(pl.array(lims[0])))
            ylim=max(abs(pl.array(lims[1])))
            # show entire pb: (statim doesn't by default)
            pl.xlim([max([xlim,pb/2]),min([-xlim,-pb/2])])
            pl.ylim([min([-ylim,-pb/2]),max([ylim,pb/2])])
            pl.text(0,max([ylim,pb/2])*1.2,"regridded model:",horizontalalignment='center')
            # ephemeris:
            pl.subplot(222)
            util.ephemeris(refdate)  # util already knows the direction from above
            
            pl.subplot(224)
            util.plotants(stnx, stny, stnz, stnd, padnames)
#            pl.plot(range(2),'o')
            ax=pl.gca()
            l=ax.get_xticklabels()
            pl.setp(l,fontsize="x-small")
            l=ax.get_yticklabels()
            pl.setp(l,fontsize="x-small")
#            pl.xlabel("coming soon",fontsize="x-small")
            pl.xlabel(telescopename,fontsize="x-small")
            
            pl.subplots_adjust(left=0.05,right=0.98,bottom=0.09,top=0.95,hspace=0.2,wspace=0.2)
            
            if checkinputs=="only":
                msg("Stopping after checking inputs as requested",priority="warn")
                return
            else:
                if display==True:
                    pl.figure(currfignum+1)
                    pl.clf()
        else:
            model_min,model_max, model_rms = util.statim(modelimage,plot=False,incell=in_cell)




        ##################################################################
        # set up observatory, feeds, etc
        # (has to be here since we may have changed nchan)
        
        if verbose:
            msg("preparing empty measurement set",priority="warn")

        sm.open(msfile)
        posobs=me.observatory(telescopename)
        diam=stnd;
        sm.setconfig(telescopename=telescopename, x=stnx, y=stny, z=stnz, 
                 dishdiameter=diam.tolist(), 
                 mount=['alt-az'], antname=antnames, padname=padnames, 
                 coordsystem='global', referencelocation=posobs)
        # sm.setspwindow now expects startfreq as argument
        # midfreq=qa.add(qa.quantity(startfreq),qa.div(bandwidth,qa.quantity("2")))
        if str.upper(telescopename).find('VLA')>0:
            sm.setspwindow(spwname=fband, freq=startfreq, deltafreq=chanwidth, 
                           freqresolution=chanwidth, nchannels=nchan, 
                           stokes='RR LL')
            sm.setfeed(mode='perfect R L',pol=[''])
        else:            
            sm.setspwindow(spwname=fband, freq=startfreq, deltafreq=chanwidth, 
                           freqresolution=chanwidth, nchannels=nchan, 
                           stokes='XX YY')
            sm.setfeed(mode='perfect X Y',pol=[''])
            
        if verbose: msg(" spectral window set at %s" % str(startfreq))
        sm.setlimits(shadowlimit=0.01, elevationlimit='10deg')
        sm.setauto(0.0)
        for k in range(0,nfld):
            src=project+'_%d'%k
            sm.setfield(sourcename=src, sourcedirection=pointings[k],
                    calcode="C", distance='0m')
        reftime = me.epoch('TAI', refdate)
        sm.settimes(integrationtime=integration, usehourangle=True, 
                referencetime=reftime)
        totalsec=qa.convert(qa.quantity(totaltime),'s')['value']
        scantime=qa.mul(qa.quantity(integration),str(scanlength))
        scansec=qa.convert(qa.quantity(scantime),'s')['value']
        nscan=int(totalsec/scansec)
        kfld=0
        # RI todo progress meter for simdata Sim::observe
        # print "calculating ";
        
        for k in range(0,nscan) :
            sttime=-totalsec/2.0+scansec*k
            endtime=sttime+scansec
            src=project+'_%d'%kfld
#        for k in range(0,nfld) :
#            sttime=-qa.convert(qa.quantity(totaltime),'s')['value']/2.0+qa.convert(qa.quantity(totaltime),'s')['value']/nfld*k
#            endtime=-qa.convert(qa.quantity(totaltime),'s')['value']/2.0+qa.convert(qa.quantity(totaltime),'s')['value']/nfld*(k+1)
#            src=project+'_%d'%k
            # this only creates blank uv entries
            sm.observe(sourcename=src, spwname=fband,
                   starttime=qa.quantity(sttime, "s"),
                   stoptime=qa.quantity(endtime, "s"));
            kfld=kfld+1
            if kfld==nfld: kfld=0
        sm.setdata(fieldid=range(0,nfld))
        sm.setvp()

        msg("done setting up observations (blank visibilities)")
        if verbose:
            sm.summary()


            
        ##################################################################
        # do actual calculation of visibilities from the model image:

        if not components_only:
            # if we only have components,
            # we have created modelimage4d from them but if
            # we have components and model image they are not yet combined
            if len(complist)>1:
                msg("predicting from "+modelimage4d+" and "+complist,priority="warn")
            else:
                msg("predicting from "+modelimage4d,priority="warn")
            sm.predict(imagename=[modelimage4d],complist=complist)
        else:   # if we're doing only components
            msg("predicting from "+complist,priority="warn")
            sm.predict(complist=complist)

        sm.done()
        
        msg('generation of measurement set ' + msfile + ' complete')






        ######################################################################
        # noisify

        noise_any=False
    
        if noise_thermal:
            if not (util.telescopename == 'ALMA' or util.telescopename == 'ACA'):
                msg("thermal noise only works properly for ALMA/ACA",origin="noise",priority="warn")
                
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
            sm.setdata();                # currently defaults to fld=0,spw=0
            # type = B BPOLY G GSPLINE D P T TOPAC GAINCURVE
            # visibilities referenced to above atmosphere 
            # sm.setapply(type='TOPAC',opacity=tau0);  # opac corruption
#            sm.oldsetnoise(spillefficiency=eta_s,correfficiency=eta_q,
#                        antefficiency=eta_a,trx=t_rx,
#                        tau=tau0,tatmos=t_sky,tcmb=t_cmb,
#                        mode="calculate")
# use ANoise version
            if noise_mode=="tsys-manual":
                if verbose:
                    msg("sm.setnoise(spillefficiency="+str(eta_s)+
                        ",correfficiency="+str(eta_q)+",antefficiency="+str(eta_a)+
                        ",trx="+str(t_rx)+",tau="+str(tau0)+
                        ",tatmos="+str(t_sky)+",tground="+str(t_ground)+
                        ",tcmb="+str(t_cmb)+",mode='tsys-manual')");
                    msg("** this may take a few minutes, but will be faster in the next CASA release",priority="warn")
                sm.setnoise(spillefficiency=eta_s,correfficiency=eta_q,
                            antefficiency=eta_a,trx=t_rx,
                            tau=tau0,tatmos=t_sky,tground=t_ground,tcmb=t_cmb,
                            mode="tsys-manual")
            else:
                if verbose:
                    msg("sm.setnoise(spillefficiency="+str(eta_s)+
                        ",correfficiency="+str(eta_q)+",antefficiency="+str(eta_a)+
                        ",trx="+str(t_rx)+",tground="+str(t_ground)+
                        ",tcmb="+str(t_cmb)+",mode='tsys-atm'"+
                        ",pground='650mbar',altitude='5000m',waterheight='2km',relhum=20,pwv="+str(user_pwv)+"mm)");
                    msg("** this may take a few minutes, but will be faster in the next CASA release",priority="warn")
                sm.setnoise(spillefficiency=eta_s,correfficiency=eta_q,
                            antefficiency=eta_a,trx=t_rx,
                            tground=t_ground,tcmb=t_cmb,pwv=str(user_pwv)+"mm",
                            mode="tsys-atm")
            # don't set table, that way it won't save to disk
#                        mode="calculate",table=noisymsroot)
            sm.corrupt();
            sm.done();

            if verbose: msg("done corrupting with thermal noise",origin="noise")

            
        # not yet implemented:
        #        noise_phase=True
        #
        #        if noise_phase:
        #            # phase noise
        #            # noise_any=True
        #            noisycalfile = project + ".atmos.cal"
        #            casalog.post('adding phase noise to ' + noisycalfile)
        #            gaincal_defaults()
        #            # make cal file to be modified
        #            gaincal(vis=msfile,caltable=noisycalfile,solint=-1)
            






        #####################################################################
        # clean if desired, use noisy image for further calculation if present
        # do we want to invert the noiseless one anyway for comparison?

        if psfmode.upper() == "NONE" or psfmode == "" :
            doclean=False
        else:
            doclean=True

        if noise_any:
            mstoimage = noisymsfile
        else:
            mstoimage = msfile

        if fidelity == True and doclean == False:
            msg("You can't calculate fidelity without imaging, so change psfmode if you want a fidelity image calculated.",origin="deconvolve",priority="warn")
            fidelity=False

        if display == True and doclean == False:
            msg("Without creating an image, there's very little to display, so I'm turning off display.  Change psfmode if you want to make an image.",origin="deconvolve",priority="warn")
            display=False

        if doclean: 
            if niter == 0:
                image=project+'.dirty'
                msg("inverting to "+image,origin="deconvolve")
            else:
                image=project+'.clean'
                msg("cleaning to "+image,origin="deconvolve")
        else:
            image=project+'.clean'

        if uvtaper == False:
            outertaper=[]
            innertaper=[]

        if nchan==1:
            cleanmode="mfs"
        else:
            cleanmode="channel"
        
        # print clean inputs no matter what, so user can use them.
        msg("clean inputs:",origin="deconvolve")
        cleanstr="clean(vis='"+mstoimage+"',imagename='"+image+"'"
        #+",field='',spw='',selectdata=False,timerange='',uvrange='',antenna='',scan='',"
        if nchan>1:
            cleanstr=cleanstr+",mode='"+cleanmode+"',nchan="+str(nchan)
        cleanstr=cleanstr+",niter="+str(niter)
        #+",gain=0.1,"
        cleanstr=cleanstr+",threshold='"+str(threshold)+"'"
        if doclean and psfmode!="clark":
            cleanstr=cleanstr+",psfmode='"+psfmode+"'"
        if imagermode != "":
            cleanstr=cleanstr+",imagermode='"+imagermode+"'"
        cleanstr=cleanstr+",ftmachine='"+ftmachine+"'"
        #",ftmachine='mosaic',mosweight=False,scaletype='SAULT',multiscale=[],negcomponent=-1,interactive=False,mask=[],start=0,width=1,"
        cleanstr=cleanstr+",imsize="+str(imsize)+",cell='"+str(cell)+"',phasecenter='"+str(centralpointing)+"'"
        #",restfreq=''"
        if stokes != "I":
            cleanstr=cleanstr+",stokes='"+stokes+"'"
        if weighting != "natural":
            cleanstr=cleanstr+",weighting='"+weighting+"',robust="+str(robust)+",noise='"+str(noise)+"',npixels="+str(npixels)
        if uvtaper:
            cleanstr=cleanstr+",uvtaper="+str(uvtaper)+",outertaper="+str(outertaper)+",innertaper="+str(innertaper)
        #+",modelimage='',restoringbeam=[''],pbcor=False,minpb=0.1,"        
        #+",npercycle=100,cyclefactor=1.5,cyclespeedup=-1)")
        cleanstr=cleanstr+")"
        msg(cleanstr,origin="deconvolve")

        if doclean: 
            clean(vis=mstoimage, imagename=image, mode=cleanmode, nchan=nchan,
                  niter=niter, threshold=threshold, selectdata=False,
                  psfmode=psfmode, imagermode=imagermode, ftmachine=ftmachine, 
                  imsize=imsize, cell=cell, phasecenter=centralpointing,
                  stokes=stokes, weighting=weighting, robust=robust,
                  uvtaper=uvtaper,outertaper=outertaper,innertaper=innertaper,
                  noise=noise, npixels=npixels)
        else:
            msg("(not actually cleaning or inverting, as requested by user)")
            image=project







        # flat output -- needed even if fidelity is not calculated
        if doclean:
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
            imrr = ia.regrid(outfile=modelregrid, overwrite=True,
                             csys=outflatcoordsys.torecord(),shape=outflatshape)

            # add clean components and model image; 
            # it'll be convolved to restored beam in the fidelity calc below
            if (os.path.exists(complist)):
                cl.open(complist)
                imrr.modify(cl.torecord(),subtract=False)
                cl.done()

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

            # Make difference image.
            difference = project + '.diff.im'
            ia.imagecalc(difference, "'%s' - '%s'" % (convolved, outflat), overwrite = True)
            ia.done()
            # Get rms of difference image.
            ia.open(difference)
            diffstats = ia.statistics(robust = True)
            ia.done()
            maxdiff=diffstats['medabsdevmed']
            if maxdiff!=maxdiff: maxdiff=0.
            # Make fidelity image.
            absdiff = project + '.absdiff.im'
            ia.imagecalc(absdiff, "max(abs('%s'), %f)" % (difference,
                                                          maxdiff/pl.sqrt(2.0)), overwrite = True)
            fidelityim = project + '.fidelity.im'
            ia.imagecalc(fidelityim, "abs('%s') / '%s'" % (convolved, absdiff), overwrite = True)
            ia.done()

            msg("fidelity image calculated",origin="analysis")

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
                pl.subplot(232)
            else:
                pl.subplot(222)
        if doclean:
            sim_min,sim_max,sim_rms = util.statim(outflat,plot=display)

        # plot model image if exists
        if os.path.exists(modelimage) and display==True:
            if fidelity == True:
                pl.subplot(231)
            else:
                pl.subplot(221)
            # modelregrid might not exist if there's no display or fidelity
            if os.path.exists(modelregrid):
                model_min,model_max, model_rms = util.statim(modelregrid,plot=display)
                xlim=max(pl.xlim())
                ylim=max(pl.ylim())
            if display==True:
                tt=pl.array(range(25))*pl.pi/12
                pb=1.2*0.3/qa.convert(qa.quantity(startfreq),'GHz')['value']/aveant*3600.*180/pl.pi
                # RI todo change this to use tb.open(ms/POINTINGS)/direction
                # and them make it a helper function
                for i in range(offsets.shape[1]):
                    pl.plot(pl.cos(tt)*pb/2+offsets[0,i]*3600,pl.sin(tt)*pb/2+offsets[1,i]*3600,'w')
                pl.xlim([xlim,-xlim])
                pl.ylim([-ylim,ylim])

            
            # fidelity image would only exist if there's a model image
            if modelimage != '' and fidelity == True:
                if display: pl.subplot(233)
                util.statim(project+".diff.im",plot=display)
                if display: pl.subplot(234)
                fidel_min, fidel_max, fidel_rms = util.statim(project+".fidelity.im",plot=display)

        if display:
            tb.open(mstoimage)
            # RI todo use rob's FFT of the PB instead
            rawdata=tb.getcol("UVW")
            tb.done()
            if fidelity == True:
                pl.subplot(235)
            else:
                pl.subplot(223)
            pl.box()
            maxbase=max([max(rawdata[0,]),max(rawdata[1,])])  # in m
            klam_m=300/qa.convert(qa.quantity(startfreq),'GHz')['value']
            pl.plot(rawdata[0,]/klam_m,rawdata[1,]/klam_m,'b,')
            pl.plot(-rawdata[0,]/klam_m,-rawdata[1,]/klam_m,'b,')
            ax=pl.gca()
            ax.yaxis.LABELPAD=-4
            l=ax.get_xticklabels()
            pl.setp(l,fontsize="x-small")
            l=ax.get_yticklabels()
            pl.setp(l,fontsize="x-small")
            pl.xlabel('u[klambda]',fontsize='x-small')
            pl.ylabel('v[klambda]',fontsize='x-small')
            pl.axis('equal')

            if modelimage != '' and doclean:   
                if fidelity == True:
                    pl.subplot(236)
                else:
                    pl.subplot(224)
                ia.open(image+".psf")
                beamcs=ia.coordsys()
                beam_array=ia.getchunk(axes=[beamcs.findcoordinate("spectral")['pixel'],beamcs.findcoordinate("stokes")['pixel']],dropdeg=True)
                pixsize=(qa.convert(qa.quantity(cell),'arcsec')['value'])
                xextent=imsize[0]*pixsize*0.5
                xextent=[xextent,-xextent]
                yextent=imsize[1]*pixsize*0.5
                yextent=[-yextent,yextent]
                flipped_array=beam_array.transpose()
                ttrans_array=flipped_array.tolist()
                ttrans_array.reverse()
                pl.imshow(ttrans_array,interpolation='bilinear',cmap=pl.cm.jet,extent=xextent+yextent,origin="bottom")
                pl.title(image+".psf",fontsize="x-small")
                b=qa.convert(beam['major'],'arcsec')['value']
                pl.xlim([-3*b,3*b])
                pl.ylim([-3*b,3*b])
                ax=pl.gca()
                l=ax.get_xticklabels()
                pl.setp(l,fontsize="x-small")
                l=ax.get_yticklabels()
                pl.setp(l,fontsize="x-small")
                #from matplotlib.font_manager import fontManager, FontProperties
                #font= FontProperties(size='x-small');
                #pl.legend(("bmaj=%7.1e" % beam['major']['value'],"bmin=%7.1e" % beam['minor']['value']),prop=font)                
                pl.text(0.05,0.95,"bmaj=%7.1e\nbmin=%7.1e" % (beam['major']['value'],beam['minor']['value']),transform = ax.transAxes,bbox=dict(facecolor='white', alpha=0.7),size="x-small",verticalalignment="top")
                ia.done()

                
            pl.subplots_adjust(left=0.05,right=0.98,bottom=0.08,top=0.96,hspace=0.1,wspace=0.2)
        
        # if not displaying still print stats:
        if doclean:
            bmarea=beam['major']['value']*beam['minor']['value']*1.1331 #arcsec2
            bmarea=bmarea/(out_cell['value'])**2 # bm area in pix
            msg('Simulation rms: '+str(sim_rms)+" Jy/pix = "+
                str(sim_rms*bmarea)+" Jy/bm",origin="analysis")
            msg('Simulation max: '+str(sim_max)+" Jy/pix = "+
                str(sim_max*bmarea)+" Jy/bm",origin="analysis")
        
        if display == True or fidelity == True:
            msg('Model rms: '+str(model_rms)+" Jy/pix",origin="analysis")
            msg('Model max: '+str(model_max)+" Jy/pix",origin="analysis")
            msg('Beam bmaj: '+str(beam['major']['value'])+' bmin: '+str(beam['minor']['value'])+' bpa: '+str(beam['positionangle']['value']),origin="analysis")






    except TypeError, e:
        msg("task_simdata -- TypeError: %s" % e,priority="error")
        return
    except ValueError, e:
        print "task_simdata -- OptionError: ", e
        return
    except Exception, instance:
        print '***Error***',instance
        return
