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
#    checkinputs=None, # will be removed
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
    cell=None, imsize=None, threshold=None, niter=None, 
#    weighting=None, outertaper=None, 
     stokes=None, 
#    psfmode=None, weighting=None, robust=None, uvtaper=None, outertaper=None, innertaper=None, noise=None, npixels=None, stokes=None, # will be removed
#    plotimage=None, cleanresidual=None, 
#analyze=None, 
#    imagename=None, originalsky=None, convolvedsky=None, difference=None, 
#    fidelity=None, 
#    display=None, # will be removed
#    plotpsf=None
    verbose=None, async=False
    ):


    casalog.origin('sdsim')
    if verbose: casalog.filter(level="DEBUG2")

    try:

        # create the utility object:
        util=simutil(direction,startfreq=qa.quantity(startfreq),
                     verbose=verbose)
        msg=util.msg

        # file check 
        if not os.path.exists(modelimage):
            msg("modelimage '%s' is not found." % modelimage,priority="error")
            
        ##################################################################
        # determine where the observation will occur:
        nfld, pointings, etime = util.calc_pointings(pointingspacing,mosaicsize,direction)

        # find imcenter - phase center
        imcenter , offsets = util.average_direction(pointings)        
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
            if type(cell) == type([]):
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
        if nant > 1: antnames=[telescopename+('SD%02d'%ant)]
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
        sm.setoptions(gridfunction='pb', ftmachine=ftmachine, location=posobs, cache=100000)
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
        
        # modify SOURCE table information for ASAP
        #nsrc=1
        tb.open(tablename=msfile+'/FIELD',nomodify=True)
        srcids=set(tb.getcol(columnname="SOURCE_ID"))
        times=tb.getcol(columnname="TIME")
        tb.close()

        nsrc=len(srcids)
        if nsrc > 1: raise Error, "More than 1 source!!!"
        print 'opnening SOURCE subtable to modify'
        tb.open(tablename=msfile+'/SOURCE',nomodify=False)
        tb.addrows(nrow=nsrc)
        print nsrc,'rows are added to SOURCE subtable'
        tb.putcol(columnname='SOURCE_ID',value=range(nsrc),startrow=0,nrow=nsrc,rowincr=1)
        #this need to be modified
        tb.putcol(columnname='TIME',value=list(times[0:nsrc]),startrow=0,nrow=nsrc,rowincr=1)
        tb.putcell(columnname='DIRECTION',rownr=0,thevalue=[0.,0.])
        rec0=tb.getvarcol(columnname='DIRECTION',startrow=0,nrow=1)
        if nsrc > 1: tb.putvarcol(columnname='DIRECTION',value=rec0,startrow=0,nrow=nsrc,rowincr=1)
        #tb.putcol(columnname='PROPER_MOTION',value=[[0.,0.]]*nsrc,startrow=0,nrow=nsrc,rowincr=1)
        tb.putvarcol(columnname='PROPER_MOTION',value=rec0,startrow=0,nrow=nsrc,rowincr=1)
        tb.putcol(columnname='CALIBRATION_GROUP',value=[0]*nsrc,startrow=0,nrow=nsrc,rowincr=1)
        tb.putcol(columnname='INTERVAL',value=[-1.]*nsrc,startrow=0,nrow=nsrc,rowincr=1)
        tb.putcol(columnname='NAME',value=[' ']*nsrc,startrow=0,nrow=nsrc,rowincr=1)
        tb.putcol(columnname='NUM_LINES',value=[0]*nsrc,startrow=0,nrow=nsrc,rowincr=1)
        tb.putcol(columnname='SPECTRAL_WINDOW_ID',value=[-1]*nsrc,startrow=0,nrow=nsrc,rowincr=1)
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
        outimage=project+'.im'
        im.makeimage(type='singledish',image=outimage)
        im.close()

        msg('generation of image ' + outimage + ' complete.')

    except TypeError, e:
        print 'TypeError: ', e
        return
    except ValueError, e:
        print 'OptionError: ', e
        return
    except Exception, instance:
        print '***Error***', instance
        return
