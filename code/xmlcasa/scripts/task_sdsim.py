# Single dish observation simulator
# KS todo TP simulation for more than one antenae. what happens? 
import os
import asap as sd
from taskinit import * 
from simutil import *

#def sdsim(modelimage, modifymodel, refdirection, refpixel, incell, inbright, antennalist, antenna,  project, refdate, integration, startfreq, chanwidth, nchan, direction, pointingspacing, relmargin, cell, imsize, stokes, noise_thermal, t_atm, tau0, verbose):
def sdsim(modelimage, ignorecoord, inbright, antennalist, antenna,  project, refdate, integration, startfreq, chanwidth, nchan, direction, pointingspacing, relmargin, cell, imsize, stokes, noise_thermal, t_atm, tau0, verbose):

    casalog.origin('sdsim')
    if verbose: casalog.filter(level="DEBUG2")

    try:

        # this is the output desired bandwidth:
        bandwidth=qa.mul(qa.quantity(nchan), qa.quantity(chanwidth))

        # create the utility object:
        util=simutil(direction,startfreq=qa.quantity(startfreq),
                     bandwidth=bandwidth, verbose=verbose)
        msg=util.msg

        # file check 
        if not os.path.exists(modelimage):
            msg("modelimage '%s' is not found." % modelimage,priority="error")

        nbands = 1
        fband = 'band'+startfreq
        msfile = project+'.ms'


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

        # (set back to simdata after calls to util -
        #   there must be an automatic way to do this)
        casalog.origin('simdata')

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

            # direction is always a list of strings (defined by .xml)
            if len(direction) == 1: direction=direction[0]
            if type(direction) == str:
                # Assume direction as a filename and read lines if it exists
                filename=os.path.expanduser(os.path.expandvars(direction))
                if os.path.exists(filename):
                    msg('Reading direction information from the file, %s' % filename)
                    n, direction, time = util.read_pointings(filename)

            nfld, pointings, etime = util.calc_pointings(pointingspacing,out_size,direction,relmargin)
                
            # find imcenter=average central point, and centralpointing
            # (to be used for phase center)
            imcenter , offsets = util.average_direction(pointings)

            ## commenting out because sd obs don't need centralpointing
#             minoff=1e10
#             central=-1
        
#             for i in range(nfld):
#                 o=pl.sqrt(offsets[0,i]**2+offsets[1,i]**2)
#                 if o<minoff:
#                     minoff=o
#                     central=i
#             centralpointing=pointings[central]
            
            #epoch, ra, dec = util.direction_splitter(centralpointing)
            epoch, ra, dec = util.direction_splitter(imcenter)
            

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

#         if type(direction) == list:
#             # manually specified pointings            
#             nfld = direction.__len__()
#             if nfld > 1 :
#                 pointings = direction
#                 if verbose: msg("You are inputing the precise pointings in 'direction' - if you want me to fill the mosaic, give a single direction")
#             else:
#                 # calculate pointings for the user 
#                 nfld, pointings, etime = util.calc_pointings(pointingspacing,out_size,direction,relmargin)
#         else:
#             # check here if "direction" is a filename, and load it in that case
#             # util.read_pointings(filename)
#             # calculate pointings for the user 
#             nfld, pointings, etime = util.calc_pointings(pointingspacing,out_size,direction,relmargin)

        # direction is always a list of strings (defined by .xml)
        if len(direction) == 1: direction=direction[0]
        if type(direction) == str:
            # Assume direction as a filename and read lines if it exists
            filename=os.path.expanduser(os.path.expandvars(direction))
            if os.path.exists(filename):
                msg('Reading direction information from the file, %s' % filename)
                n, direction, time = util.read_pointings(filename)

        nfld, pointings, etime = util.calc_pointings(pointingspacing,out_size,direction,relmargin)
            
                

        # find imcenter=average central point or direction (for single dish)
        # (to be used for phase center)
        if type(direction) == list:
            imcenter , offsets = util.average_direction(pointings)
        else:
            imcenter = direction

        ## commenting out this part because sd obs don't need centralpointing
#         minoff=1e10
#         central=-1
        
#         for i in range(nfld):
#             o=pl.sqrt(offsets[0,i]**2+offsets[1,i]**2)
#             if o<minoff:
#                 minoff=o
#                 central=i
#         centralpointing=pointings[central]
            
        # (set back to simdata after calls to util)
        casalog.origin('simdata')

#         if nfld==1:
#             imagermode=''
#             ftmachine="ft"
#             msg("phase center = " + centralpointing)
#         else:
#             imagermode="mosaic"
#             ftmachine="mosaic"
#             msg("mosaic center = " + imcenter + "; phase center = " + centralpointing)
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
        sm.setoptions(gridfunction='pb', ftmachine='sd', location=posobs, cache=100000)
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
            sm.setapply(type='TOPAC',opacity=tau0);  # opac corruption
            # SimACohCalc needs 2-temp formula not just t_atm
            #sm.setnoise(mode="calculate",table=noisymsroot,
            #            antefficiency=eta_a,correfficiency=eta_q,
            #            spillefficiency=eta_s,tau=tau0,trx=t_rx,
            #            tatmos=t_atm,tcmb=t_cmb)
            sm.oldsetnoise(mode="calculate",table=noisymsroot,
                           antefficiency=eta_a,correfficiency=eta_q,
                           spillefficiency=eta_s,tau=tau0,trx=t_rx,
                           tatmos=t_atm,tcmb=t_cmb)
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
        im.setoptions(ftmachine='sd',gridfunction='pb')
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
