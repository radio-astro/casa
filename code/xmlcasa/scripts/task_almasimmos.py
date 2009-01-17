# for the love of god don't use tabs in python code!
#
# revisions
# date   author change
# 20071030 rr    change mosaic center choices onto hex grid
#                It would also be nice to check whether those directions are
#                 1. within the modelimage or hull of components,
#                 2. above the elevation limit
#                before producing any visibilities.
# 20080417 rr+ri fix xmlization, remove rmode parameter
# 20080505 ri thermal noise, clearer xmlized parameters
# 20080529 ri switch to newclean and other xml nastiness fixed.
# 20080601 ri at least partially fix mom0 maps to prevent crashing on simulated cubes.

# TODO: Find timerange when at least one of the inputs is up, limit the
#       observing time to it, and warn or abort (printing the valid
#       timerange) depending on how badly the user missed it.
# TODO: midpoint freq of band instead of startfreq
# TODO: look for antenna list in default repository also
# TODO: allow user to report statistics in J/ybm instead of Jy/arcsec



import os
from clean import clean_imp as clean
from taskinit import *
import pylab as pl

import pdb

def almasimmos(modelimage=None, incell=None, inbright=None, complist=None, antennalist=None, project=None, refdate=None, totaltime=None, integration=None, startfreq=None, chanwidth=None, nchan=None, direction=None, pointingspacing=None, relmargin=None, cell=None, imsize=None, niter=None, threshold=None, psfmode=None, weighting=None, robust=None, uvtaper=None, outertaper=None, innertaper=None, noise=None, npixels=None, stokes=None, noise_thermal=None, t_amb=None, tau0=None, fidelity=None, display=None, async=None):

    ##################some helper functions.
    ##################keeping them inside almasimmos name space
    ##################as nobody needs to know about them
    def msg(s):
        clr="\x1b[32m"
        bw="\x1b[0m"
        print clr+"[simmos] "+bw+s
        casalog.post("")
        casalog.post(s)

    def mul_scal_qa(scal, q):
        """
        Returns the product of a dimensionless number scal and quantity q.
        """
        return qa.quantity(scal * q['value'], q['unit'])
    
    def calc_pointings(direction, spacing, imsize, cell, relmargin = 0.5):
        """
        If direction is a list, simply returns direction and the number of
        pointings in it.
        
        Otherwise, returns a hexagonally packed list of pointings separated by
        spacing and fitting inside an image specified by direction, imsize and
        cell, and the number of pointings.  The hexagonal packing starts with a
        horizontal row centered on direction, and the other rows alternate
        being horizontally offset by a half spacing.  All of the pointings will
        be within a rectangle relmargin * spacing smaller than the image on all
        sides.
        """
        if type(direction) == list:
            if len(direction) >1:
                return len(direction), direction
            else:
                direction=direction[0]
    
        epoch, centx, centy = direction_splitter(direction)

        spacing  = qa.quantity(spacing)
        yspacing = mul_scal_qa(0.866025404, spacing)
    
        if type(cell) == list:
            cellx, celly = map(qa.quantity, cell)
        else:
            cellx = qa.quantity(cell)
            celly = cellx
        
        ysize = mul_scal_qa(imsize[1], celly)
        nrows = 1+ int(pl.floor(qa.convert(qa.div(ysize, yspacing), '')['value']
                                - 2.309401077 * relmargin))

        xsize = mul_scal_qa(imsize[0], cellx)
        availcols = 1 + qa.convert(qa.div(xsize, spacing),
                                   '')['value'] - 2.0 * relmargin
        ncols = int(pl.floor(availcols))

        # By making the even rows shifted spacing/2 ahead, and possibly shorter,
        # the top and bottom rows (nrows odd), are guaranteed to be short.
        if availcols - ncols >= 0.5:                                # O O O
            evencols = ncols                                    #  O O O
            ncolstomin = 0.5 * (ncols - 0.5)
        else:
            evencols = ncols - 1                                #  O O 
            ncolstomin = 0.5 * (ncols - 1)                      # O O O
        pointings = []

        # Start from the top because in the Southern hemisphere it sets first.
        y = qa.add(centy, mul_scal_qa(0.5 * (nrows - 1), yspacing))
        for row in xrange(0, nrows):         # xrange stops early.
            xspacing = mul_scal_qa(1.0 / pl.cos(qa.convert(y, 'rad')['value']),
                                   spacing)
            ystr = qa.formxxx(y, format='dms')
        
            if row % 2:                             # Odd
                xmin = qa.sub(centx, mul_scal_qa(ncolstomin, xspacing))
                stopcolp1 = ncols
            else:                                   # Even (including 0)
                xmin = qa.sub(centx, mul_scal_qa(ncolstomin - 0.5,
                                                 xspacing))
                stopcolp1 = evencols
            for col in xrange(0, stopcolp1):        # xrange stops early.
                x = qa.formxxx(qa.add(xmin, mul_scal_qa(col, xspacing)),
                               format='hms')
                pointings.append("%s%s %s" % (epoch, x, ystr))
            y = qa.sub(y, yspacing)
        ####could not fit pointings then single pointing
        if(len(pointings)==0):
            pointings.append(direction)
        msg("Using %i generated pointings:" % len(pointings))
        return len(pointings), pointings

    def average_direction(directions):
        """
        Returns the average of directions as a string, and relative offsets
        """
        epoch0, x, y = direction_splitter(directions[0])
        i = 1
        avgx = 0.0
        avgy = 0.0
        for drn in directions:
            epoch, x, y = direction_splitter(drn)
            x = x['value']
            y = y['value']
            if epoch != epoch0:                     # Paranoia
                print "[simmos] WARN: precession not handled by average_direction()"
            x = wrapang(x, avgx, 360.0)
            avgx += (x - avgx) / i
            avgy += (y - avgy) / i
            i += 1
        offsets=pl.zeros([2,i-1])
        i=0
        for drn in directions:
            epoch, x, y = direction_splitter(drn)
            x = x['value']
            y = y['value']
            x = wrapang(x, avgx, 360.0)
            offsets[:,i]=[x-avgx,y-avgy]
            i+=1
        avgx = qa.toangle('%fdeg' % avgx)
        avgy = qa.toangle('%fdeg' % avgy)
        avgx = qa.formxxx(avgx, format='hms')
        avgy = qa.formxxx(avgy, format='dms')
        return "%s%s %s" % (epoch0, avgx, avgy), offsets

    def direction_splitter(direction):
        """
        Given a direction, return its epoch, x, and y parts.  Epoch will be ''
        if absent, or '%s ' % epoch if present.  x and y will be angle qa's in
        degrees.
        """
        if type(direction) == list:
            direction=direction[0]                
        dirl = direction.split()
        if len(dirl) == 3:
            epoch = dirl[0] + ' '
        else:
            epoch = ''
        x, y = map(qa.toangle, dirl[-2:])
        return epoch, qa.convert(x, 'deg'), qa.convert(y, 'deg')

    def wrapang(ang, target, period = 360.0):
        """
        Returns ang wrapped so that it is within +-period/2 of target.
        """
        dang       = ang - target
        period     = pl.absolute(period)
        halfperiod = 0.5 * period
        if pl.absolute(dang) > halfperiod:
            nwraps = pl.floor(0.5 + float(dang) / period)
            ang -= nwraps * period
        return ang

    def readantenna(antab=None):
    ###Helper function to read 4 columns text antenna table X, Y, Z, Diam
        f=open(antab)
        line= '  '
        stnx=[]
        stny=[]
        stnz=[]
        stnd=[]
        nant=0
        line='    '
        while (len(line)>0):
            try: 
                line=f.readline()
                if (line.find('#')!=0):
                ###ignoring line that has less than 4 elements
                    if(len(line.split()) >3):
                        splitline=line.split()
                        stnx.append(float(splitline[0]))
                        stny.append(float(splitline[1]))
                        stnz.append(float(splitline[2]))
                        stnd.append(float(splitline[3]))
                        nant+=1                 
            except:
                break

        f.close()
        return (pl.array(stnx), pl.array(stny), pl.array(stnz), pl.array(stnd), nant)

############End of helper functions

############Begin of main function ############################################

    casalog.origin('almasimmos')
   
    if((not os.path.exists(modelimage)) and (not os.path.exists(complist))):
        print "[simmos] ERROR -- No sky input found.  At least one of modelimage or complist"
        print "              must be set."
        return

    try:
        # set up pointings

        nfld, pointings = calc_pointings(direction, pointingspacing,
                                         imsize, cell, relmargin)
        ave , off = average_direction(pointings)
        nbands = 1;    
        fband  = 'band'+startfreq

      
        msfile=project+'.ms'
        
        if (modelimage == ''):
            if incell=="header":
                print "\x1b[31m[simmos] Error: you want to use the model image header but haven't specified a model image\x1b[0m"
                return

            # 20090106 devel: should we be using incell since there is not
            # an image?
            msg("creating an image from your clean components")
            modelimage=project+'.model.im'
            ia.fromshape(modelimage,[imsize[0],imsize[1],1,nchan])
            ia.setbrightnessunit("Jy/pixel")
            cs=ia.coordsys()
            epoch,ra,dec=direction_splitter(direction)
            cs.setunits(['rad','rad','','Hz'])
            cs.setincrement([incell,incell],'direction')
            cs.setreferencevalue([qa.convert(ra,'rad')['value'],qa.convert(dec,'rad')['value']],'direction')
            cs.setreferencevalue(startfreq,'spectral')
            ia.setcoordsys(cs.torecord())
            cl.open(complist)
            ia.modify(cl.torecord(),subtract=False)
            cl.done()
            ia.done() # to make sure its saved to disk at the start
            

        # open model image as ia tool "model"; leave open
        modelia=ia.newimagefromfile(modelimage)

        # for people who have put in an array, we should print an error.
        if type(incell) == 'list':
            incell=incell[0]
        # optionally use image header for cell size
        if incell=='header':                
            incellx,incelly=modelia.coordsys().increment()['numeric']
            incellx=qa.quantity(incellx,'rad')
            incelly=qa.quantity(incelly,'rad')
            # warn if incells are not square 
            if not incellx == incelly:
                clrstr="\x1b[31m" #red
                print clrstr+"Input pixels are not square!\x1b[0m"
                print clrstr+("Using incell=incellx=%s\x1b[0m" % incellx)
            incell=qa.convert(incellx,'arcsec')
        else:
            incellx=incell
            incelly=incell

        # now that incell is set by the user or has been retrieved from the 
        # image header, we can use it for the output cell also
        if cell=="incell":
            cell=incell



        print "imsize=",imsize


        # set up observatory

#os.listdir(os.getenv("CASAPATH").split(' ')[0]+"/data/alma/")
        stnx, stny, stnz, stnd, nant=readantenna(antennalist);
        antnames=[]
        for k in range(0,nant): antnames.append('A%02d'%k)

        sm.open(msfile)
        posalma=me.observatory('ALMA')
        ax=sum(stnx)/nant;
        x= stnx-ax; 
        ay=sum(stny)/nant;
        y= stny-ay;
        az=sum(stnz)/nant;
        z= stnz-az;
        diam=stnd;
        sm.setconfig(telescopename='ALMA', x=x.tolist(), 
                 y=y.tolist(), z=z.tolist(), 
                 dishdiameter=diam.tolist(), 
                 mount=['alt-az'], antname=antnames,
                 coordsystem='local', referencelocation=posalma);
        sm.setspwindow(spwname=fband, freq=startfreq, deltafreq=chanwidth, 
                   freqresolution=chanwidth, nchannels=nchan, 
                   stokes='XX YY');
        sm.setfeed(mode='perfect X Y',pol=['']);
        sm.setlimits(shadowlimit=0.01, elevationlimit='10deg');
        sm.setauto(0.0);
        for k in range(0,nfld):
            src=project+'_%d'%k
            sm.setfield(sourcename=src, sourcedirection=pointings[k],
                    calcode="C", distance='0m')
        reftime = me.epoch('TAI', refdate);
        sm.settimes(integrationtime=integration, usehourangle=True, 
                referencetime=reftime);
        for k in range(0,nfld) :
            src=project+'_%d'%k
            sttime=-qa.convert(qa.quantity(totaltime),'s')['value']/2.0+qa.convert(qa.quantity(totaltime),'s')['value']/nfld*k
            endtime=-qa.convert(qa.quantity(totaltime),'s')['value']/2.0+qa.convert(qa.quantity(totaltime),'s')['value']/nfld*(k+1)
            # remember, this just creates blank uv entries
            sm.observe(sourcename=src, spwname=fband,
                   starttime=qa.quantity(sttime, "s"),
                   stoptime=qa.quantity(endtime, "s"));
        sm.setdata(fieldid=range(0,nfld))
        sm.setvp()
        if type(direction) == list:
            imcenter , foo = average_direction(direction)
            msg("Using phasecenter " + imcenter)
        else:
            imcenter = direction

        msg("done setting up observations")




##################################################################
### fit modelimage into a 4 coordinate image defined by the parameters
### (ignoorecoord has been removed for now, for better or worse)
### the area to predict really doesn't have to be the same size as the output
### so we just have to set the input pixel size, and then create an output 
### model image that's padded or trimmed as ness.

        if (modelimage != ''):
            # modelia is open to the original input model from above
            insize=modelia.shape()

            # first make a blank 4-dimensional image w/the right cell size
            modelimage4d=project+"."+modelimage+'.coord'
            im.open(msfile)    
            im.selectvis(field=range(nfld), spw=0)
            im.defineimage(nx=insize[0], ny=insize[1], cellx=incellx, celly=incelly, phasecenter=imcenter)
            im.make(modelimage4d)
            im.close()
            im.done()

            # scale to input Jy/arcsec
            inbrightpix=inbright*(qa.quantity(incell)['value'])**2            
            stats=modelia.statistics()
            highvalue=stats['max']
            scalefactor=inbrightpix/highvalue

#            pdb.set_trace()

            # arr is the chunk of the input image
            arr=modelia.getchunk()*scalefactor;
            blc=[0,0,0,0]
            trc=[0,0,1,1]
            naxis=len(arr.shape);
            modelia.done();


            # now open rescaled output image
            modelia.open(modelimage4d)            
            # and overwrite the input shapes with the 4d shapes
            insize=modelia.shape()
            incoordsys=modelia.coordsys()

            # arr2 is the chunk of the resized input image
            arr2=modelia.getchunk()

            if(arr2.shape[0] >= arr.shape[0] and arr2.shape[1] >= arr.shape[1]):
                arrin=arr
                arrout=arr2
                reversi=False
            elif(arr2.shape[0] < arr.shape[0] and arr2.shape[1] < arr.shape[1]):
                arrin=arr2
                arrout=arr
                reversi=True
            blc[0]=(arrout.shape[0]-arrin.shape[0])/2;
            blc[1]=(arrout.shape[1]-arrin.shape[1])/2;
            trc[0]=blc[0]+arrin.shape[0];
            trc[1]=blc[1]+arrin.shape[1];
            if(naxis==2):
                if(not reversi):
                    arrout[blc[0]:trc[0], blc[1]:trc[1], 0, 0]=arrin
                else:
                    arrin[:,:,0,0]=arrout[blc[0]:trc[0], blc[1]:trc[1]]
            if(naxis==3):
                if(arrout.shape[2]==arrin.shape[2]):
                    blc[2]=0
                    trc[2]=arrin.shape[2]
                if(not reversi):
                    arrout[blc[0]:trc[0], blc[1]:trc[1], blc[2]:trc[2], 0]=arrin
                else:
                    arrin[:,:,:,0]=arrout[blc[0]:trc[0], blc[1]:trc[1], :]
            if(naxis==4):
# input images with more than 1 channel need more testing!
                if((arrout.shape[3]==arrin.shape[3]) and
                   (arrout.shape[2]==arrin.shape[2])):
                    blc[3]=0
                    trc[3]=arrin.shape[3]
                    blc[2]=0
                    trc[2]=arrin.shape[2]
                if(not reversi):
                    arrout[blc[0]:trc[0], blc[1]:trc[1], blc[2]:trc[2], blc[3]:trc[3]]=arrin
                else:
                    arrin=arrout[blc[0]:trc[0], blc[1]:trc[1], : :]


            modelia.putchunk(arr2)
            modelia.done()

# image should now have 4 axes and same size as output
# at least in xy (check channels later)        
            msg("predicting from "+modelimage4d)
            sm.predict(imagename=[modelimage4d], complist=complist)
            
        else:   # we're doing just components
            sm.predict(complist=complist)
            
        sm.done()
        sm.close()
        
        msg('generation of measurement set ' + msfile + ' complete.')


######################################################################
# noisify_alma ; start with Apr 2008 Knee version (see helper functions)

        noise_any=False
    
        if noise_thermal:
            noise_any=True

            import numpy
            from math import exp

            noisymsfile = project + ".noisy.ms"
            msg('adding thermal noise to ' + noisymsfile)

            diam=12.
            # spillover efficiency.    
            eta_s = 0.96 
            # correlator quantization efficiency.    
            eta_q = 0.95
            # blockage efficiency.    
            eta_b = 1.-(1./diam)**2
            # taper efficiency.    
            eta_t = 0.86

            # Ruze phase efficiency.    
            epsilon = 25.  # micron surface rms

            startfreq_ghz=qa.convertfreq(qa.quantity(startfreq),'GHz')
            eta_p = exp(-(4.0*3.1415926535*epsilon*startfreq_ghz.get("value")/2.99792458e5)**2)
            msg('ruze phase efficiency = ' + str(eta_p))

            # antenna efficiency
            eta_a = eta_p*eta_s*eta_b*eta_t
            msg('antenna efficiency = ' + str(eta_a))
 
            # Ambient surface radiation temperature in K. 
            # FOR NOW, T_atm = T_ground = T_amb
            t_ground = t_amb
            # Atmospheric radiation temperature in K. 
            t_atm = t_amb

            # Cosmic background radiation temperature in K. 
            t_cmb = 2.275
            # Receiver radiation temperature in K. 
            # ALMA-40.00.00.00-001-A-SPE.pdf
            freq0=[35,75,110,145,185,230,345,409,175,230]
            trx0=[17,30,37,51,65,83,147,196,175,230]
            t_rx = numpy.interp([startfreq_ghz.get("value")],freq0,trx0)[0]

            os.system("cp -r "+msfile+" "+noisymsfile)
            # TODO check for and copy flagversions file as well
            
            sm.openfromms(noisymsfile);    # an existing MS
            sm.setdata();                # currently defaults to fld=0,spw=0
            # type = B BPOLY G GSPLINE D P T TOPAC GAINCURVE
            sm.setapply(type='TOPAC',opacity=tau0);  # arrange opac corruption
            # TODO VERIFY that this is just an attenuation!
            sm.setnoise(spillefficiency=eta_s,correfficiency=eta_q,
                        antefficiency=eta_a,
                        tau=tau0,tatmos=t_amb,tcmb=t_cmb,
                        mode="calculate")
            sm.corrupt();
            sm.done();

            msg("done corrupting with thermal noise")
            
#TEST
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
# clean if desired, WILL use noisy image for further calculation if present!
# do we want to invert the noiseless one anyway for comparison?

        if noise_any:
#            imgroot = project + '.noisy'
# 20080525 shorter filenames
            imgroot = project 
            mstoimage = noisymsfile
        else:
            imgroot = project
            mstoimage = msfile

        if fidelity == True and psfmode == "none":
#            msg("You can't calculate fidelity without imaging, so I'm making you a dirty image")
#            psfmode="clark"
#            niter=0
            msg("You can't calculate fidelity without imaging, so change psfmode if you want a fidelity image calculated.")
            fidelity=False

        if display == True and psfmode == "none":
            msg("Without creating an image, there's very little to display, so I'm turning off display.  Change psfmode if you want to make an image.")
            display=False

        if psfmode != "none": 
            if niter == 0:
                image=imgroot+'.dirty'
                msg("inverting to "+image)
            else:
                image=imgroot+'.clean'
                msg("cleaning to "+image)
        
            if uvtaper == False:
                outertaper=[]
                innertaper=[]
                
            clean(vis=msfile,imagename=image,field='',spw='',selectdata=False,timerange='',uvrange='',antenna='',scan='',mode="channel",niter=niter,gain=0.1,threshold=threshold,psfmode=psfmode,imagermode="mosaic",ftmachine="mosaic",mosweight=False,scaletype="SAULT",multiscale=[],negcomponent=-1,interactive=False,mask=[],nchan=nchan,start=0,width=1,imsize=imsize,cell=cell,phasecenter=imcenter,restfreq='',stokes=stokes,weighting=weighting,robust=robust,uvtaper=uvtaper,outertaper=outertaper,innertaper=innertaper,modelimage='',restoringbeam=[''],pbcor=False,minpb=0.1,noise=noise,npixels=npixels,npercycle=100,cyclefactor=1.5,cyclespeedup=-1)
        else:
            image=imgroot

# wtf?  __temp_model2
# OSError: (66, 'Directory not empty')

# need this filename whether or not we create the image
        modelregrid=project+"."+modelimage+".regrid"

        if fidelity == True or display == True:
        # now deal with the input image, and make sure we have flat/mom0
        # versions of both to compare and calculate fidelity.
        # we'll need the flat image for display whether or not we have 
        # fidelity.  

            inspectax=incoordsys.findcoordinate('spectral')['pixel']
            innchan=insize[inspectax]
            # modelimage4d is the .coord version w/spectral axis.
            # modelflat should be the moment zero of that
            modelflat=project+"."+modelimage+".mom0" 
            if innchan>1:
                msg("creating moment zero input image")
                # actually run ia.moments
                ia.open(modelimage4d)
                ia.moments(moments=[-1],outfile=modelflat,overwrite=True)
                ia.done()
            else:
                # just remove degenerate axes from modelimage4d
                ia.newimagefromimage(infile=modelimage4d,outfile=modelflat,dropdeg=True,overwrite=True)
                # why can't I just remove spectral and leave stokes?
                os.system("mv "+modelflat+" "+modelflat+".tmp")
                ia.open(modelflat+".tmp")
                ia.adddegaxes(outfile=modelflat,stokes='I',overwrite=True)
                ia.done()
                os.system("rm -rf "+modelflat+".tmp")

            # flat output
            outflat=image+".mom0"
        # should we really be calling this mom0, if we use the mean, or mom-1?
            if nchan>1:
                # image is either .clean or .dirty
                msg("creating moment zero output image")                
                ia.open(image+".image")
                ia.moments(moments=[-1],outfile=outflat,overwrite=True)
                ia.done()
            else:            
                # just remove degenerate axes from image
                ia.newimagefromimage(infile=image+".image",outfile=outflat,dropdeg=True,overwrite=True)
                # seems to not be a way to just drop the spectral and keep the stokes.  sigh.
                os.system("mv "+outflat+" "+outflat+".tmp")
                ia.open(outflat+".tmp")
                ia.adddegaxes(outfile=outflat,stokes='I',overwrite=True)
                ia.done()
                os.system("rm -rf "+outflat+".tmp")
            
            # be sure to get outflatcoordsys from outflat
            ia.open(outflat)
            outflatcoordsys=ia.coordsys()
            outflatshape=ia.shape()
            ia.done()

            # regrid flat input to flat output shape, for convolution, etc
            modelia.open(modelflat)
            imrr = modelia.regrid(outfile=modelregrid, overwrite=True,
                                  csys=outflatcoordsys.torecord(),shape=outflatshape)
            imrr.done()

# should we also add the cclean image?  can we make an image from the cclist?
# create it and then modelia.immath or some such

            modelia.done()
            outflatcoordsys.done()
            del outflatcoordsys
            del imrr

            if innchan==1:
                os.system("rm -rf "+modelflat)  # no need to keep this


        if psfmode != "none":
            msg("done inverting and cleaning")



#####################################################################
# fidelity  start with Knee version again

#        if os.path.exists(modelimage) and display == True and psfmode != "none":
        if os.path.exists(modelimage) and (fidelity == True or display == True):             # get beam from output clean image
            ia.open(image+".image")
            beam=ia.restoringbeam()
            ia.done()

        if os.path.exists(modelimage) and fidelity == True: 
            from math import sqrt
            
            # Convolve model with beam.
            convolved = project + '.convolved.im'
            modelia.open(modelregrid)
            modelia.convolve2d(convolved,major=beam['major'],minor=beam['minor'],
                          pa=beam['positionangle'],overwrite=True)

            # Make difference image.
            difference = imgroot + '.diff.im'
            modelia.imagecalc(difference, "'%s' - '%s'" % (convolved, outflat), overwrite = True)
            modelia.done()
            # Get rms of difference image.
            ia.open(difference)
            diffstats = ia.statistics(robust = True)
            ia.done()
            # Make fidelity image.
            absdiff = imgroot + '.absdiff.im'
            ia.imagecalc(absdiff, "max(abs('%s'), %f)" % (difference,
                                                          diffstats['medabsdevmed']/sqrt(2.0)), overwrite = True)
            fidelityim = imgroot + '.fidelity.im'
            ia.imagecalc(fidelityim, "abs('%s') / '%s'" % (convolved, absdiff), overwrite = True)
            ia.done()

            msg("fidelity image calculated")

# clean up moment zero maps if we don't need them anymore
            if nchan==1:
                os.system("rm -rf "+outflat)  # no need to keep this


 #####################################################################
# display and stats;        
        # we want stats etc calculated whether we do display or not
        def statim(image,plot=True):
# make this global later
#            statunits=
            ia.open(image)
            imunit=ia.summary()['header']['unit']            
            if imunit == 'Jy/beam':
                # stupid for dirty image:
                if len(ia.restoringbeam())>0:
                    bm=ia.summary()['header']['restoringbeam']['restoringbeam']
                    toJyarcsec=1./(qa.convert(bm['major'],'arcsec')['value']*
                                   qa.convert(bm['minor'],'arcsec')['value']*pl.pi/4)
                else:
                    toJyarcsec=1.
            elif imunit == 'Jy/pixel':
                pix=ia.summary()['header']['incr']
                toJyarcsec=1./abs(pix[0]*pix[1])/206265.0**2
            else:
                print "WARN: don't know image units for %s" % image
            stats=ia.statistics(robust=True)
            im_max=stats['max']*toJyarcsec
            im_min=stats['min']*toJyarcsec
            imsize=ia.shape()[0:2]
            reg1=rg.box([0,0],[imsize[0]*.25,imsize[1]*.25])
            stats=ia.statistics(region=reg1)
            im_rms=stats['rms']*toJyarcsec
            if len(im_rms)==0: im_rms=0.
            data_array=ia.getchunk([-1,-1,1,1],[-1,-1,1,1],[1],[],True,True,False)
            data_array=pl.array(data_array)
            tdata_array=pl.transpose(data_array)
            ttrans_array=tdata_array.tolist()
            ttrans_array.reverse()
            if (plot):
                csys=ia.coordsys()
                pixsize= csys.increment()["numeric"][0:2]*180/pl.pi*3600 # to arcsec
                xextent=imsize[0]*pixsize[0]*0.5
                xextent=[xextent,-xextent]
                yextent=imsize[1]*pixsize[1]*0.5
                yextent=[-yextent,yextent]
                # remove top .5% of pixels:
                nbin=200
                imhist=ia.histograms(cumu=True,nbins=nbin)['histout']
                ii=nbin-1
                highcounts=imhist['counts'][ii]
                while imhist['counts'][ii]>0.995*highcounts and ii>0: 
                    ii=ii-1
                highvalue=imhist['values'][ii]
                #
                pl.imshow(ttrans_array,interpolation='bilinear',cmap=pl.cm.jet,extent=xextent+yextent,vmax=highvalue)
                ax=pl.gca()
                l=ax.get_xticklabels()
                pl.setp(l,fontsize="x-small")
                l=ax.get_yticklabels()
                pl.setp(l,fontsize="x-small")
                pl.title(image,fontsize="x-small")
                from matplotlib.font_manager import fontManager, FontProperties
                font= FontProperties(size='x-small');
                pl.legend(("min=%7.1e" % im_min,"max=%7.1e" % im_max,"RMS=%7.1e" % im_rms),pad=0.15,prop=font)
            ia.done()
            return im_min,im_max,im_rms

        # plot whichever image we have - preference is for noisy and cleaned
        if display:
            pl.ion()
            pl.clf()
            if fidelity == True:
                pl.subplot(232)
            else:
                pl.subplot(222)
        if psfmode != "none":
            sim_min,sim_max,sim_rms = statim(image+".image",plot=display)

        # plot model image if exists
        if os.path.exists(modelimage):
            if fidelity == True:
                pl.subplot(231)
            else:
                pl.subplot(221)
            # modelregrid might not exist if there's no display or fidelity
            if os.path.exists(modelregrid):
                model_min,model_max, model_rms = statim(modelregrid,plot=display)
            foo=pl.xlim(),pl.ylim()
            if (display):
                tt=pl.array(range(25))*pl.pi/12
                pb=1.2*0.3/qa.convert(qa.quantity(startfreq),'GHz')['value']/12.*3600.*180/pl.pi
                for i in range(off.shape[1]):
                    pl.plot(pl.cos(tt)*pb/2+off[0,i]*3600,pl.sin(tt)*pb/2+off[1,i]*3600,'w')
                pl.xlim(max([foo[0],pb/2]))
                pl.ylim(max([foo[1],pb/2]))
        # else:
        #    Maybe a restored complist should be presented, but that
        #    would be so clean-centric.
            
            # fidelity image would only exist if there's a model image
            if modelimage != '' and fidelity == True:
                pl.subplot(233)
                statim(imgroot+".diff.im",plot=display)
                pl.subplot(234)
                fidel_min, fidel_max, fidel_rms = statim(imgroot+".fidelity.im",plot=display)

        if (display):
            tb.open(mstoimage)
            foo=tb.getcol("UVW")
            tb.done()
            if fidelity == True:
                pl.subplot(235)
            else:
                pl.subplot(223)
            pl.box()
            maxbase=max([max(foo[0,]),max(foo[1,])])  # in m
            klam_m=300/qa.convert(qa.quantity(startfreq),'GHz')['value']
            pl.plot(foo[0,]/klam_m,foo[1,]/klam_m,',')
            pl.plot(-foo[0,]/klam_m,-foo[1,]/klam_m,',')
            ax=pl.gca()
            ax.yaxis.LABELPAD=-4
            l=ax.get_xticklabels()
            pl.setp(l,fontsize="x-small")
            l=ax.get_yticklabels()
            pl.setp(l,fontsize="x-small")
            pl.xlabel('u[klambda]',fontsize='x-small')
            pl.ylabel('v[klambda]',fontsize='x-small')
            pl.axis('equal')

            if modelimage != '' and psfmode != "none":   
                if fidelity == True:
                    pl.subplot(236)
                else:
                    pl.subplot(224)
#                ia.open(imgroot+".convolved.im")
#                #beam=ia.restoringbeam()
#                ia.done()
                ia.open(image+".psf")
                beam_array=ia.getchunk([-1,-1,1,1],[-1,-1,1,1],[1],[],True,True,False)
                pixsize=(qa.convert(qa.quantity(cell),'arcsec')['value'])
                xextent=imsize[0]*pixsize*0.5
                xextent=[xextent,-xextent]
                yextent=imsize[1]*pixsize*0.5
                yextent=[-yextent,yextent]
                pl.imshow(beam_array,interpolation='bilinear',cmap=pl.cm.jet,extent=xextent+yextent)
                pl.title(image+".psf",fontsize="x-small")
                b=qa.convert(beam['major'],'arcsec')['value']
                pl.xlim([-3*b,3*b])
                pl.ylim([-3*b,3*b])
                ax=pl.gca()
                l=ax.get_xticklabels()
                pl.setp(l,fontsize="x-small")
                l=ax.get_yticklabels()
                pl.setp(l,fontsize="x-small")
                from matplotlib.font_manager import fontManager, FontProperties
                font= FontProperties(size='x-small');
                pl.legend(("bmaj=%7.1e" % beam['major']['value'],"bmin=%7.1e" % beam['minor']['value']),prop=font)                
                ia.done()

                
            pl.subplots_adjust(left=0.05,right=0.98,bottom=0.08,top=0.96,hspace=0.1,wspace=0.2)
        
        # if not displaying still print stats:
        if psfmode != "none":
            print 'Simulation rms: '+str(sim_rms)
            print 'Simulation max: '+str(sim_max)
            casalog.post('Simulation rms: '+str(sim_rms))
            casalog.post('Simulation max: '+str(sim_max))
        
#        if os.path.exists(modelregrid):
        if display == True or fidelity == True:
            print 'Model rms: '+str(model_rms)
            print 'Model max: '+str(model_max)
            casalog.post('Model rms: '+str(model_rms))
            casalog.post('Model max: '+str(model_max))
            print 'Beam bmaj: '+str(beam['major']['value'])+' bmin: '+str(beam['minor']['value'])+' bpa: '+str(beam['positionangle']['value'])
            casalog.post('Beam bmaj: '+str(beam['major']['value'])+' bmin: '+str(beam['minor']['value'])+' bpa: '+str(beam['positionangle']['value']))
            

    except TypeError, e:
        print "task_almasimmos -- TypeError: ", e
        return
    except ValueError, e:
        print "task_almasimmos -- OptionError: ", e
        return
    except Exception, instance:
        print '***Error***',instance
        return
