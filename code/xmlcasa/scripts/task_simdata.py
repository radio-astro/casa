import os
from clean import clean
from taskinit import *
from simutil import *
import pylab as pl
import pdb

def simdata(
    project=None, 
#setup=None, 
    complist=None, modelimage=None, inbright=None, ignorecoord=None,
    # nchan=None, # removed - possible complist issues
    startfreq=None, chanwidth=None,
    refdate=None, totaltime=None, integration=None, 
    scanlength=None, # will be removed
    direction=None, pointingspacing=None, mosaicsize=None, # plotfield=None,
    caldirection=None, calflux=None,
    checkinputs=None, # will be removed
#predict=None, 
    antennalist=None, 
#    ptgfile=None, plotuv=None, plotconfig=None,
#process=None, 
    noise_thermal=None, 
    noise_mode=None, #will be removed
    user_pwv=None, t_ground=None, t_sky=None, tau0=None, 
#    cross_pol=None,
#image=None, 
#    cleanmode=None,
    cell=None, imsize=None, threshold=None, niter=None, 
    # weighting=None, outertaper=None, stokes=None, 
    psfmode=None, weighting=None, robust=None, uvtaper=None, outertaper=None, innertaper=None, noise=None, npixels=None, stokes=None, # will be removed
#    plotimage=None, cleanresidual=None, 
#analyze=None, 
#    imagename=None, originalsky=None, convolvedsky=None, difference=None, 
    fidelity=None, 
    display=None, # will be removed
#    plotpsf=None
    verbose=None, async=False
    ):


    # RI TODO for inbright=unchanged, need to scale input image to jy/pix


    casalog.origin('simdata')
    if verbose: casalog.filter(level="DEBUG2")

    # create the utility object:
    util=simutil(direction)
    if verbose: util.verbose=True
    msg=util.msg
    
    if((not os.path.exists(modelimage)) and (not os.path.exists(complist))):
        msg("No sky input found.  At least one of modelimage or complist must be set.",priority="error")
        return
    
    try:

        ##################################################################
        # determine where the observation will occur:
        nfld, pointings, etime = util.calc_pointings(pointingspacing,mosaicsize,direction)

        # find imcenter - phase center
        imcenter , offsets = util.average_direction(pointings)        
        epoch, ra, dec = util.direction_splitter(imcenter)

        # for clean
        msg("phase center = " + imcenter)
        if nfld==1:
            imagermode=''
            ftmachine="ft"        
        else:
            imagermode="mosaic"
            ftmachine="ft"   # FOR CARMA/heterogeneous, has to be mosaic
            if verbose: 
                for dir in pointings:
                    msg("   "+dir)

        ##################################################################
        # calibrator
        calfluxjy=qa.convert(calflux,'Jy')['value']
        # stupid XML:
        if type(caldirection)==type([]): caldirection=caldirection[0]
        if len(caldirection)<4: caldirection=""
        if calfluxjy > 0 and caldirection != "":            
            docalibrator=True
            cl.done()
            cl.addcomponent(flux=calfluxjy,dir=caldirection,label="phase calibrator")
            # in principle, need reference freq - defaults to 0!
            cl.rename(project+'.cal.cclist')
            cl.done()
        else:
            docalibrator=False

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
            if type(cell) == type([]):
                in_cell =  map(qa.convert,cell,['arcsec','arcsec'])
            else:
                in_cell = qa.convert(cell,'arcsec')            
                in_cell = [in_cell,in_cell]
        

        #####################
        # create image from components - this is used in checkinputs, 
        # and analysis, but not really for much else

        if (modelimage == ''):
            # if we are going to create an image we need a shape:
            if imsize.__len__()==1:
                imsize=[imsize,imsize]            
            out_nstk=stokes.__len__()
            # RI TODO ***** need to find nchan from components?!!!
            nchan=1
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
            cell_rad=qa.convert(qa.quantity(in_cell),"rad")['value']
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
        # read antenna file (goes to predict in simdata2)

        stnx, stny, stnz, stnd, padnames, nant, telescopename = util.readantenna(antennalist)
        if stnx==False:
            return
        antnames=[]
        for k in range(0,nant): antnames.append('A%02d'%k)
        aveant=stnd.mean()

        # (set back to simdata - there must be an automatic way to do this)
        casalog.origin('simdata')



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
            ax=pl.gca()
            l=ax.get_xticklabels()
            pl.setp(l,fontsize="x-small")
            l=ax.get_yticklabels()
            pl.setp(l,fontsize="x-small")
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
#            model_min,model_max, model_rms = util.statim(modelimage,plot=False,incell=model_cell)
            model_min,model_max, model_rms = util.statim(modelflat,plot=False,incell=in_cell)

        casalog.origin('simdata')



        ##################################################################
        # set up observatory, feeds, etc
        # (has to be here since we may have changed nchan)
        
        if verbose:
            msg("preparing empty measurement set",origin="simdata",priority="warn")

        nbands = 1;    
        fband  = 'band'+startfreq
        msfile=project+'.ms'

        sm.open(msfile)
        posobs=me.observatory(telescopename)
        diam=stnd;
        sm.setconfig(telescopename=telescopename, x=stnx, y=stny, z=stnz, 
                 dishdiameter=diam.tolist(), 
                 mount=['alt-az'], antname=antnames, padname=padnames, 
                 coordsystem='global', referencelocation=posobs)
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
                    calcode="OBJ", distance='0m')
            if k==0:
                sourcefieldlist=src
            else:
                sourcefieldlist=sourcefieldlist+','+src
        if docalibrator:
            sm.setfield(sourcename="phase calibrator", sourcedirection=caldirection,calcode='C',distance='0m')
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

        if nscan<nfld:
            msg("Only %i pointings of %i in the mosaic will be observed - check mosaic setup and exposure time parameters!" % (nscan,nfld),priority="error")
            return
        
        
        for k in range(0,nscan) :
            sttime=-totalsec/2.0+scansec*k
            endtime=sttime+scansec
            src=project+'_%d'%kfld
            # this only creates blank uv entries
            sm.observe(sourcename=src, spwname=fband,
                   starttime=qa.quantity(sttime, "s"),
                   stoptime=qa.quantity(endtime, "s"));
            kfld=kfld+1
            if kfld==nfld: 
                if docalibrator:
                    sttime=-totalsec/2.0+scansec*k
                    endtime=sttime+scansec
                    sm.observe(sourcename="phase calibrator", spwname=fband,
                               starttime=qa.quantity(sttime, "s"),
                               stoptime=qa.quantity(endtime, "s"));
                kfld=kfld+1                
            if kfld > nfld: kfld=0
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

            sm.done() # simulator hangs onto previous versions of the MS
            if os.path.exists(noisymsfile):
                shutil.rmtree(noisymsfile)                
            shutil.copytree(msfile,noisymsfile)
            if sm.name()!='':
                msg("table persistence error on %s" % sm.name(),priority="error")
                return

            sm.openfromms(noisymsfile)    # an existing MS
            sm.setdata()                # currently defaults to fld=0,spw=0
# use ANoise version - deprecated but required for AC / SD
#            sm.oldsetnoise(spillefficiency=eta_s,correfficiency=eta_q,
#                        antefficiency=eta_a,trx=t_rx,
#                        tau=tau0,tatmos=t_sky,tcmb=t_cmb,
#                        mode="calculate")
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
            msg("You can't calculate fidelity without imaging, so change psfmode if you want a fidelity image calculated.",priority="warn")
            fidelity=False

        if display == True and doclean == False:
            msg("Without creating an image, there's very little to display, so I'm turning off display.  Change psfmode if you want to make an image.",priority="warn")
            display=False

        if doclean: 
            if niter == 0:
                image=project+'.dirty'
                msg("inverting to "+image)
            else:
                image=project+'.clean'
                msg("cleaning to "+image)
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
        # and write a clean.last file
        cleanlast=open("clean.last","write")
        cleanlast.write('taskname            = "clean"\n')

        msg("clean inputs:")
        cleanstr="clean(vis='"+mstoimage+"',imagename='"+image+"'"
        #+",field='',spw='',selectdata=False,timerange='',uvrange='',antenna='',scan='',"
        cleanlast.write('vis                 = "'+mstoimage+'"\n')
        cleanlast.write('imagename           = "'+image+'"\n')
        cleanlast.write('outlierfile         = ""\n')
        if docalibrator:
            cleanlast.write('field               = "'+sourcefieldlist+'"\n')
        else:
            cleanlast.write('field               = ""\n')
        cleanlast.write('spw                 = ""\n')
        cleanlast.write('selectdata          = False\n')
        cleanlast.write('timerange           = ""\n')
        cleanlast.write('uvrange             = ""\n')
        cleanlast.write('antenna             = ""\n')
        cleanlast.write('scan                = ""\n')
        if nchan>1:
            cleanstr=cleanstr+",mode='"+cleanmode+"',nchan="+str(nchan)
            cleanlast.write('mode                = "'+cleanmode+'"\n')
            cleanlast.write('nchan               = "'+str(nchan)+'"\n')
        else:
            cleanlast.write('mode                = "mfs"\n')
            cleanlast.write('nchan               = -1\n')
        cleanlast.write('gridmode                = ""\n')
        cleanlast.write('wprojplanes             = 1\n')
        cleanlast.write('facets                  = 1\n')
        cleanlast.write('cfcache                 = "cfcache.dir"\n')
        cleanlast.write('painc                   = 360.0\n')
        cleanlast.write('epjtable                = ""\n')
        cleanlast.write('interpolation           = "nearest"\n')
        cleanstr=cleanstr+",niter="+str(niter)
        cleanlast.write('niter                   = '+str(niter)+'\n')
        cleanlast.write('gain                    = 0.1\n')
        #+",gain=0.1,"
        cleanstr=cleanstr+",threshold='"+str(threshold)+"'"
        cleanlast.write('threshold               = "'+str(threshold)+'"\n')
        if doclean and psfmode!="clark":
            cleanstr=cleanstr+",psfmode='"+psfmode+"'"
        if doclean:
            cleanlast.write('psfmode                 = "'+psfmode+'"\n')
        if imagermode != "":
            cleanstr=cleanstr+",imagermode='"+imagermode+"'"
        cleanlast.write('imagermode              = "'+imagermode+'"\n')
        cleanstr=cleanstr+",ftmachine='"+ftmachine+"'"
        cleanlast.write('ftmachine               = "'+ftmachine+'"\n')
        cleanlast.write('mosweight               = False\n')
        cleanlast.write('scaletype               = "SAULT"\n')
        cleanlast.write('multiscale              = []\n')
        cleanlast.write('negcomponent            = -1\n')
        cleanlast.write('smallscalebias          = 0.6\n')
        cleanlast.write('interactive             = False\n')
        cleanlast.write('mask                    = []\n')
        cleanlast.write('start                   = 0\n')
        cleanlast.write('width                   = 1\n')
        cleanlast.write('outframe                = ""\n')
        cleanlast.write('veltype                 = "radio"\n')
        #",ftmachine='mosaic',mosweight=False,scaletype='SAULT',multiscale=[],negcomponent=-1,interactive=False,mask=[],start=0,width=1,"
        cleanstr=cleanstr+",imsize="+str(imsize)+",cell="+str(map(qa.tos,out_cell))+",phasecenter='"+str(imcenter)+"'"
        cleanlast.write('imsize                  = '+str(imsize)+'\n');
        cleanlast.write('cell                    = '+str(map(qa.tos,out_cell))+'\n');
        cleanlast.write('phasecenter             = "'+str(imcenter)+'"\n');
        cleanlast.write('restfreq                = ""\n');
        #",restfreq=''"
        if stokes != "I":
            cleanstr=cleanstr+",stokes='"+stokes+"'"
        cleanlast.write('stokes                  = "'+stokes+'"\n');
        cleanlast.write('weighting               = "'+weighting+'"\n');
        if weighting != "natural":
            cleanstr=cleanstr+",weighting='"+weighting+"',robust="+str(robust)+",noise='"+str(noise)+"',npixels="+str(npixels)
        cleanlast.write('robust                  = '+str(robust)+'\n');
        cleanlast.write('uvtaper                 = '+str(uvtaper)+'\n');
        if uvtaper:
            cleanstr=cleanstr+",uvtaper="+str(uvtaper)+",outertaper="+str(outertaper)+",innertaper="+str(innertaper)
        cleanlast.write('outertaper              = "'+str(outertaper)+'"\n');
        cleanlast.write('innertaper              = "'+str(innertaper)+'"\n');
        cleanlast.write('modelimage              = ""\n');
        cleanlast.write("restoringbeam           = ['']\n");
        cleanlast.write("pbcor                   = False\n");
        cleanlast.write("minpb                   = 0.1\n");
        cleanlast.write("calready                = True\n");
        cleanlast.write('noise                   = "'+str(noise)+'"\n');
        cleanlast.write('npixels                 = '+str(npixels)+'\n');
        cleanlast.write('npercycle               = 100\n');
        cleanlast.write('cyclefactor             = 1.5\n');
        cleanlast.write('cyclespeedup            = -1\n');
        cleanlast.write('nterms                  = 1\n');
        cleanlast.write('reffreq                 = ""\n');
        cleanlast.write('chaniter                = False\n');
        #+",modelimage='',restoringbeam=[''],pbcor=False,minpb=0.1,"        
        #+",npercycle=100,cyclefactor=1.5,cyclespeedup=-1)")
        cleanstr=cleanstr+")"
        msg(cleanstr,priority="warn")
        cleanlast.write("#"+cleanstr+"\n")
        cleanlast.close()

        if doclean:
            # clean insists on using an existing model if it's present
            if os.path.exists(image+".image"): shutil.rmtree(image+".image")
            if os.path.exists(image+".model"): shutil.rmtree(image+".model")
            clean(vis=mstoimage, imagename=image, mode=cleanmode, nchan=nchan,
                  niter=niter, threshold=threshold, selectdata=False,
                  psfmode=psfmode, imagermode=imagermode, ftmachine=ftmachine, 
                  imsize=imsize, cell=map(qa.tos,out_cell), phasecenter=imcenter,
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

            # imagecalc is interpreting outflat in Jy/pix - 
            # the diff is v. negative.
            # need to convert the clean image into Jy/pix.  
            bmarea=beam['major']['value']*beam['minor']['value']*1.1331 #arcsec2
            bmarea=bmarea/(out_cell[0]['value']*out_cell[1]['value']) # bm area in pix
            msg("synthesized beam area in output pixels = %f" % bmarea)
#            ia.open(outflat)
#            ia.setbrightnessunit("Jy/pixel")
#            ia.image
            # Make difference image.
            difference = project + '.diff.im'
            ia.imagecalc(difference, "'%s' - ('%s'/%g)" % (convolved, outflat,bmarea), overwrite = True)
            # Get rms of difference image.
            ia.open(difference)
            diffstats = ia.statistics(robust = True)
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
                pl.subplot(232)
            else:
                pl.subplot(222)
        if doclean:
            max_cleanim=[] # mutable so can be returned
            sim_min,sim_max,sim_rms = util.statim(outflat,plot=display,disprange=max_cleanim)
            # max_cleanim returned in outflat units i.e. Jy/bm > jy/pix
            max_cleanim[0]/=bmarea
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
                util.statim(project+".diff.im",plot=display,disprange=max_cleanim)
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
                if type(cell)==type([]):
                    pixsize=(qa.convert(qa.quantity(cell[0]),'arcsec')['value'])
                else:
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
        msg("task_simdata -- TypeError: %s" % e,priority="error")
        return
    except ValueError, e:
        print "task_simdata -- OptionError: ", e
        return
    except Exception, instance:
        print '***Error***',instance
        return
