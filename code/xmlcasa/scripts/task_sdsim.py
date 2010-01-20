# Single dish observation simulator 
import os
import asap as sd
from taskinit import * 
from simutil import *

def sdsim(modelimage, modifymodel, refdirection, refpixel, incell, inbright, antennalist, antenna,  project, refdate, integration, startfreq, chanwidth, nchan, direction, pointingspacing, relmargin, cell, imsize, stokes, noise_thermal, t_atm, tau0, verbose):
#def sdsim(modelimage, modifymodel, refdirection, refpixel, incell, inbright, antennalist, antenna,  project, refdate, integration, startfreq, chanwidth, nchan, direction, pointingspacing, relmargin, cell, imsize, stokes, noise_thermal, t_atm, t_ground, tau0, verbose):

    casalog.origin('sdsim')

    ### Start mod: 2010/01/20 kana ###
    #def msg(s, origin=None, color=None):
    #    # ansi color codes:
    #    # Foreground colors
    #    # 30    Black
    #    # 31    Red
    #    # 32    Green
    #    # 33    Yellow
    #    # 34    Blue
    #    # 35    Magenta
    #    # 36    Cyan
    #    # 37    White
    #    if color==None:
    #        clr="\x1b[32m"
    #    else:
    #        clr="\x1b["+color+"m"
    #    bw="\x1b[0m"
    #    if origin==None:
    #        origin="simutil"
    #    print clr+"["+origin+"] "+bw+s
    #    casalog.post("")
    #    casalog.post(s)
    ### End mod ######################

    try:
        ### Start mod: 2010/01/20 kana ###
        # set up simulation utility
        bandwidth=qa.mul(qa.quantity(nchan), qa.quantity(chanwidth))
        util=simutil(direction,startfreq=qa.quantity(startfreq), bandwidth=bandwidth, verbose=verbose)
        msg=util.msg
        ### End mod ######################
        # file check 
        if not os.path.exists(modelimage):
            msg("modelimage '%s' is not found." % modelimage,priority="error")

        try:
            msfile = project+'.ms'

            ### Start mod: 2010/01/20 kana ###
            ##################################################################
            # convert original model image to 4d shape:

            out_cell=cell
            # if cell="incell", this will be changed to an
            # actual value by simutil.image4d below


            #if not ignorecoord:
            if not modifymodel:
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

#                 if type(direction) == list:
#                     # manually specified pointings            
#                     nfld = direction.__len__()
#                     if nfld > 1 :
#                         pointings = direction
#                         if verbose: msg("You are inputing the precise pointings in 'direction' - if you want me to fill the mosaic, give a single direction")
#                     else:
#                         # calculate pointings for the user 
#                         nfld, pointings, etime = util.calc_pointings(pointingspacing,out_size,direction,relmargin)
#                 else:
#                     # check here if "direction" is a filename, and load it in that case
#                     # util.read_pointings(filename)
#                     # calculate pointings for the user 
#                     nfld, pointings, etime = util.calc_pointings(pointingspacing,out_size,direction,relmargin)

                # direction is always a list of strings (defined by .xml)
                if len(direction) == 1: direction=direction[0]
                if type(direction) == str:
                    # Assume direction as a filename and read lines if it exists
                    filename=os.path.expanduser(os.path.expandvars(direction))
                    if os.path.exists(filename):
                        msg('Reading direction information from the file, %s' % filename)
                        n, direction, time = util.read_pointings(filename)

#                out_size = [qa.mul(imsize[0], cell),qa.mul(imsize[1], cell)]
                #nfld, pointings = util.calc_pointings(pointingspacing,imsize,cell,direction,relmargin)
                nfld, pointings, etime = util.calc_pointings(pointingspacing,out_size,direction,relmargin)
                
                # find imcenter=average central point, and centralpointing
                # (to be used for phase center)
                imcenter , offsets = util.average_direction(pointings)

                ## commenting out because sd obs don't need centralpointing
#                 minoff=1e10
#                 central=-1
        
#                 for i in range(nfld):
#                     o=pl.sqrt(offsets[0,i]**2+offsets[1,i]**2)
#                     if o<minoff:
#                         minoff=o
#                         central=i
#                 centralpointing=pointings[central]
            
                #epoch, ra, dec = util.direction_splitter(centralpointing)
                if refdirection=="direction": 
                    epoch, ra, dec = util.direction_splitter(imcenter)
                else:
                    epoch, ra, dec = util.direction_splitter(refdirection)
            

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
                                      #inbright,ignorecoord,
                                      inbright,modifymodel,
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

#             if type(direction) == list:
#                 # manually specified pointings            
#                 nfld = direction.__len__()
#                 if nfld > 1 :
#                     pointings = direction
#                     if verbose: msg("You are inputing the precise pointings in 'direction' - if you want me to fill the mosaic, give a single direction")
#                 else:
#                     # calculate pointings for the user 
#                     nfld, pointings, etime = util.calc_pointings(pointingspacing,out_size,direction,relmargin)
#             else:
#                 # check here if "direction" is a filename, and load it in that case
#                 # util.read_pointings(filename)
#                 # calculate pointings for the user 
#                 nfld, pointings, etime = util.calc_pointings(pointingspacing,out_size,direction,relmargin)

            # direction is always a list of strings (defined by .xml)
            if len(direction) == 1: direction=direction[0]
            if type(direction) == str:
                # Assume direction as a filename and read lines if it exists
                filename=os.path.expanduser(os.path.expandvars(direction))
                if os.path.exists(filename):
                    msg('Reading direction information from the file, %s' % filename)
                    n, direction, time = util.read_pointings(filename)

#            out_size = [qa.mul(imsize[0], cell),qa.mul(imsize[1], cell)]
            #nfld, pointings = util.calc_pointings(pointingspacing,imsize,cell,direction,relmargin)
            nfld, pointings, etime = util.calc_pointings(pointingspacing,out_size,direction,relmargin)
            
                

            # find imcenter=average central point, and centralpointing
            # (to be used for phase center)
            imcenter , offsets = util.average_direction(pointings)

            ## commenting out this part because sd obs don't need centralpointing
#             minoff=1e10
#             central=-1
        
#             for i in range(nfld):
#                 o=pl.sqrt(offsets[0,i]**2+offsets[1,i]**2)
#                 if o<minoff:
#                     minoff=o
#                     central=i
#             centralpointing=pointings[central]
            
            # (set back to simdata after calls to util)
            casalog.origin('simdata')

#             if nfld==1:
#                 imagermode=''
#                 ftmachine="ft"
#                 msg("phase center = " + centralpointing)
#             else:
#                 imagermode="mosaic"
#                 ftmachine="mosaic"
#                 msg("mosaic center = " + imcenter + "; phase center = " + centralpointing)
            ## For single dish simulation
            imagermode=''
            ftmachine="sd"
            msg("map center = " + imcenter)
            if verbose: 
                for dir in pointings:
                    msg("   "+dir)
            ### End mod ######################




            ### Start mod: 2010/01/20 kana ###
#             # open model image as ia tool "model"; leave open
#             modelia=ia.newimagefromfile(modelimage)
            
#             # Setup cell sizes of input and output image.
#             # This seems to be needed before calculating position list. 
#             # for people who have put in an array, we should print an error.
#             if type(incell) == 'list':
#                 incell=incell[0]
#             # optionally use image header for cell size
#             if incell=='header':                
#                 #incellx,incelly=modelia.coordsys().increment()['numeric']
#                 modelcs=modelia.coordsys()
#                 csincs=modelcs.increment()['numeric']
#                 incellx=csincs[modelcs.findcoordinate("direction")['pixel'][0]]
#                 incelly=csincs[modelcs.findcoordinate("direction")['pixel'][1]]
#                 incellx=qa.quantity(incellx,'rad')
#                 incelly=qa.quantity(incelly,'rad')
#                 # warn if incells are not square 
#                 if not incellx == incelly:
#                     msg("Input pixels are not square!",color="31")
#                     msg("Using incell=incellx=%s" % incellx,color="31")
#                 incell=qa.convert(incellx,'arcsec')
#             else:
#                 incellx=incell
#                 incelly=incell

#             # now that incell is set by the user or has been retrieved from the 
#             # image header, we can use it for the output cell also
#             if cell=="incell":
#                 cell=incell
            ### End mod ######################

            # set up simulation utility
            ### Start mod: 2010/01/20 kana ###
            #bandwidth=qa.mul(qa.quantity(nchan), qa.quantity(chanwidth))
            #util=simutil(direction,startfreq=qa.quantity(startfreq), bandwidth=bandwidth, verbose=verbose)
            ### End mod ######################

            # set up a list of pointings (imsize*cell)
            # If direction is not a list, only hexagonal gridding is currently available
            ### Start mod: 2010/01/20 ###
#             print 'Start - pointing calcs'
#             ### Start mod: 2010/01/19 ###
#             # direction is always a list of strings (defined by .xml)
#             if len(direction) == 1: direction=direction[0]
#             if type(direction) == str:
#                 # Assume direction as a filename and read lines if it exists
#                 filename=os.path.expanduser(os.path.expandvars(direction))
#                 if os.path.exists(filename):
#                     msg('Reading direction information from the file, %s' % filename)
#                     n, direction, time = util.read_pointings(filename)

#             out_size = [qa.mul(imsize[0], cell),qa.mul(imsize[1], cell)]
#             #nfld, pointings = util.calc_pointings(pointingspacing,imsize,cell,direction,relmargin)
#             nfld, pointings, etime = util.calc_pointings(pointingspacing,out_size,direction,relmargin)
#             print 'End - pointing calcs'
#             ### End mod #################
#             ave , off = util.average_direction(pointings)
            ### End mod #################
            nbands = 1
            fband = 'band'+startfreq


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

            # set up observatory, feeds, etc:
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
            for k in range(0,nfld) :
                src=project+'_%d'%k
                sttime=-qa.convert(qa.quantity(totaltime),'s')['value']/2.0+qa.convert(qa.quantity(integration),'s')['value']*k
                endtime=-qa.convert(qa.quantity(totaltime),'s')['value']/2.0+qa.convert(qa.quantity(integration),'s')['value']*(k+1)
                # this only creates blank uv entries
                sm.observe(sourcename=src, spwname=fband,
                           starttime=qa.quantity(sttime, "s"),
                           stoptime=qa.quantity(endtime, "s"));
            sm.setdata(fieldid=range(0,nfld))
            sm.setvp()

            # set imcenter to the center of the mosaic;  this is used in clean
            # as the phase center, which could be nonideal if there is no
            # pointing exactly in the middle of the mosaic.
            if type(direction) == list:
                imcenter , discard = util.average_direction(direction)
                msg("Using phasecenter " + imcenter)
                if verbose:
                    print direction
                    for dir in direction:
                        msg("   "+dir)
            else:
                imcenter = direction

            msg("done setting up observations",priority="warn")


            # fit modelimage into a 4 coordinate image defined by the parameters
            
            ### Start mod: 2010/01/20 kana ###
#             # modelia is already open with the original input model from above
#             insize=modelia.shape()

#             # first make a blank 4-dimensional image w/the right cell size
#             modelimage4d=project+"."+modelimage+'.coord'
#             im.open(msfile)    
#             im.selectvis(field=range(nfld), spw=0)
#             # imcenter is the average of the desired pointings
#             # we put the phase center there, rather than at the
#             # reference direction for the input model (refdirection)
#             im.defineimage(nx=insize[0], ny=insize[1], cellx=incellx, celly=incelly, phasecenter=imcenter)
#             im.make(modelimage4d)
#             im.close()
#             im.done()

#             if refdirection!=None:
#                 if verbose: msg("setting model image direction to "+refdirection)
#                 if refdirection!="header":
#                     ia.open(modelimage4d)
#                     modelcs=ia.coordsys()
#                     if refdirection=="direction":
#                         epoch, ra, dec = util.direction_splitter(imcenter)
#                     else:
#                         epoch, ra, dec = util.direction_splitter(refdirection)
#                     ref=[0,0,0,0]
#                     raax=modelcs.findcoordinate("direction")['pixel'][0]
#                     ref[raax]=qa.convert(ra,modelcs.units()[raax])['value']
#                     deax=modelcs.findcoordinate("direction")['pixel'][1]
#                     ref[deax]=qa.convert(dec,modelcs.units()[deax])['value']
#                     spax=modelcs.findcoordinate("spectral")['pixel']
#                     ref[spax]=qa.convert(startfreq,modelcs.units()[spax])['value']
#                     ref[modelcs.findcoordinate("stokes")['pixel']]=1
#                     modelcs.setreferencevalue(ref)
#                     ia.setcoordsys(modelcs.torecord())
#                     ia.done()
#                     if verbose: msg(" ra="+qa.angle(ra)+" dec="+qa.angle(dec))


#             if refpixel!=None:
#                 if refpixel!="header":
#                     ia.open(modelimage4d)
#                     modelcs=ia.coordsys()
#                     if refpixel=="center":
#                         crpix=pl.array([insize[0]/2,insize[1]/2])
#                     else:
#                         # XXX this is pretty fragile code right now
#                         refpixel=refpixel.replace('[','')
#                         refpixel=refpixel.replace(']','')
#                         crpix=pl.array(refpixel.split(','))
#                     ref=[0,0,0,0]
#                     raax=modelcs.findcoordinate("direction")['pixel'][0]
#                     ref[raax]=float(crpix[0])
#                     deax=modelcs.findcoordinate("direction")['pixel'][1]
#                     ref[deax]=float(crpix[1])
#                     spax=modelcs.findcoordinate("spectral")['pixel']
#                     ref[spax]=0
#                     ref[modelcs.findcoordinate("stokes")['pixel']]=0
#                     if verbose: msg("setting model image ref pixel to %f,%f" % (ref[0],ref[1]))
#                     modelcs.setreferencepixel(ref)
#                     ia.setcoordsys(modelcs.torecord())
#                     ia.done()
                

#             if inbright=="default":
#                 scalefactor=1.
#             else:
#                 # scale to input Jy/arcsec
#                 if type(incell)==str:
#                     incell=qa.quantity(incell)
#                 inbrightpix=float(inbright)*(qa.convert(incell,'arcsec')['value'])**2            
#                 stats=modelia.statistics()
#                 highvalue=stats['max']
#                 scalefactor=inbrightpix/highvalue


#             # arr is the chunk of the input image
#             arr=modelia.getchunk()*scalefactor;
#             blc=[0,0,0,0]
#             trc=[0,0,1,1]
#             naxis=len(arr.shape);
#             modelia.done();


#             # now open rescaled output image
#             modelia.open(modelimage4d)            
#             # and overwrite the input shapes with the 4d shapes
#             insize=modelia.shape()
#             incoordsys=modelia.coordsys()

#             # arr2 is the chunk of the resized input image
#             arr2=modelia.getchunk()

#             if(arr2.shape[0] >= arr.shape[0] and arr2.shape[1] >= arr.shape[1]):
#                 arrin=arr
#                 arrout=arr2
#                 reversi=False
#             elif(arr2.shape[0] < arr.shape[0] and arr2.shape[1] < arr.shape[1]):
#                 arrin=arr2
#                 arrout=arr
#                 reversi=True
#             blc[0]=(arrout.shape[0]-arrin.shape[0])/2;
#             blc[1]=(arrout.shape[1]-arrin.shape[1])/2;
#             trc[0]=blc[0]+arrin.shape[0];
#             trc[1]=blc[1]+arrin.shape[1];
#             if(naxis==2):
#                 if(not reversi):
#                     arrout[blc[0]:trc[0], blc[1]:trc[1], 0, 0]=arrin
#                 else:
#                     arrin[:,:,0,0]=arrout[blc[0]:trc[0], blc[1]:trc[1]]
#             if(naxis==3):
#                 if(arrout.shape[2]==arrin.shape[2]):
#                     blc[2]=0
#                     trc[2]=arrin.shape[2]
#                 if(not reversi):
#                     arrout[blc[0]:trc[0], blc[1]:trc[1], blc[2]:trc[2], 0]=arrin
#                 else:
#                     arrin[:,:,:,0]=arrout[blc[0]:trc[0], blc[1]:trc[1], :]
#             if(naxis==4):
#                 # XXX input images with more than 1 channel need more testing!
#                 if((arrout.shape[3]==arrin.shape[3]) and
#                    (arrout.shape[2]==arrin.shape[2])):
#                     blc[3]=0
#                     trc[3]=arrin.shape[3]
#                     blc[2]=0
#                     trc[2]=arrin.shape[2]
#                 if(not reversi):
#                     arrout[blc[0]:trc[0], blc[1]:trc[1], blc[2]:trc[2], blc[3]:trc[3]]=arrin
#                 else:
#                     arrin=arrout[blc[0]:trc[0], blc[1]:trc[1], : :]


#             modelia.putchunk(arr2)
#             modelia.done()
            ### End mod ######################

            # image should now have 4 axes and same size as output
            # XXX at least in xy (check channels later)        

            
            # do actual calculation of visibilities from the model image:
            if verbose: msg("predicting from "+modelimage4d)
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
                msg('adding thermal noise to ' + noisymsfile,origin="noise")

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
                # XXX check for and copy flagversions file as well
            
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

    except Exception, instance:
        print '***Error***', instance
        return
