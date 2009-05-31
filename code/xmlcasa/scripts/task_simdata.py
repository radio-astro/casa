# (partial) TODO list - also search for XXX in the code
# Find timerange when at least one of the inputs is up, limit the
#       observing time to it, and warn or abort (printing the valid
#       timerange) depending on how badly the user missed it.
# look for antenna list in default repository also
# allow user to report statistics in Jy/bm instead of Jy/arcsec
# Pad input image to a composite number of pixels on a side.  I don't
#       know if it would speed things up much, but it would suppress a warning.
#       Large primes are surprisingly common.



import os
from clean import clean
from taskinit import *
from simutil import *
import pylab as pl
import pdb


def simdata(modelimage=None, ignorecoord=None, inbright=None, complist=None, antennalist=None, checkinputs=None, project=None, refdate=None, totaltime=None, integration=None, startfreq=None, chanwidth=None, nchan=None, direction=None, pointingspacing=None, relmargin=None, cell=None, imsize=None, niter=None, threshold=None, psfmode=None, weighting=None, robust=None, uvtaper=None, outertaper=None, innertaper=None, noise=None, npixels=None, stokes=None, noise_thermal=None, t_amb=None, tau0=None, fidelity=None, display=None, verbose=False, async=None):




# helper function to plot an image (optionally), and calculate its statistics
# we could move this to the utility object

    def statim(image,plot=True,incell=None):
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
            pix=ia.summary()['header']['incr']
            toJypix=toJyarcsec*abs(pix[0]*pix[1])*206265.0**2
        elif imunit == 'Jy/pixel':
            pix=ia.summary()['header']['incr']
            toJyarcsec=1./abs(pix[0]*pix[1])/206265.0**2
            toJypix=1.
        else:
            msg("WARN: don't know image units for %s" % image,origin="statim")
            toJyarcsec=1.
            toJypix=1.
        stats=ia.statistics(robust=True)
        #im_max=stats['max']*toJyarcsec
        #im_min=stats['min']*toJyarcsec
        im_max=stats['max']*toJypix
        im_min=stats['min']*toJypix
        imsize=ia.shape()[0:2]
        reg1=rg.box([0,0],[imsize[0]*.25,imsize[1]*.25])
        stats=ia.statistics(region=reg1)
        #im_rms=stats['rms']*toJyarcsec
        im_rms=stats['rms']*toJypix
        if len(im_rms)==0: im_rms=0.
        data_array=ia.getchunk([-1,-1,1,1],[-1,-1,1,1],[1],[],True,True,False)
        data_array=pl.array(data_array)
        tdata_array=pl.transpose(data_array)
        ttrans_array=tdata_array.tolist()
        ttrans_array.reverse()
        if (plot):
            csys=ia.coordsys()
            pixsize= csys.increment()["numeric"][0:2]*180/pl.pi*3600 # to arcsec
            if incell != None:
                if type(incell)=='str':
                    incell=qa.quantity(incell)
                pixsize=qa.convert(incell,'arcsec')['value']+pl.zeros(2)
            #if verbose: msg("pixel size= %s" % pixsize,origin="statim")
            xextent=imsize[0]*abs(pixsize[0])*0.5
            xextent=[xextent,-xextent]
            yextent=imsize[1]*abs(pixsize[1])*0.5
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





    ############ main function ############################################


    casalog.origin('simdata')
    if verbose: casalog.filter(level="DEBUG2")

    # this is the output desired bandwidth:
    bandwidth=qa.mul(qa.quantity(nchan),qa.quantity(chanwidth))

    # create the utility object:
    util=simutil(direction,startfreq=qa.quantity(startfreq),
                 bandwidth=bandwidth)
    if verbose: util.verbose=True
    msg=util.msg
    
    if((not os.path.exists(modelimage)) and (not os.path.exists(complist))):
        msg("ERROR -- No sky input found.  At least one of modelimage or complist must be set.",color="31")
        return
    
    try:

        nbands = 1;    
        fband  = 'band'+startfreq
        msfile=project+'.ms'


        ##################################################################
        # read antenna file:

        stnx, stny, stnz, stnd, nant, telescopename = util.readantenna(antennalist)
        antnames=[]
        for k in range(0,nant): antnames.append('A%02d'%k)
        aveant=stnd.mean()

        # (set back to simdata after calls to util -
        #   there must be an automatic way to do this)
        casalog.origin('simdata')





        ##################################################################
        # set output shape:

        if imsize.__len__()==1:
            imsize=[imsize,imsize]
        out_size=imsize
            
        out_nstk=stokes.__len__()
        out_shape=[out_size[0],out_size[1],out_nstk,nchan]



        ##################################################################
        # if needed, make a model image from the input clean component list
        # (make it the desired 4d output shape)
        # visibilities will be calculated from the component list - this
        # image is just for display and fidelity

        if (modelimage == ''):
            
            if cell=="incell":
                msg("ERROR: You can't use the input header for the pixel size if you don't have an input header!",color="31")
                return False
            else:
                out_cell=qa.convert(cell,'arcsec')

            if verbose: msg("creating an image from your clean components",origin="setup model")
            components_only=True
            modelimage=project+'.ccmodel'
            ia.fromshape(modelimage,out_shape)
            cs=ia.coordsys()
            # use output direction to create model image:
            epoch,ra,dec=util.direction_splitter(direction)
            cs.setunits(['rad','rad','','Hz'])
            # use output cell size to create model image:
            # this is okay - if none of the clean components fit in the output
            # image, then the simulated image will be blank.  user error.
            cell_rad=qa.convert(qa.quantity(out_cell),"rad")['value']
            cs.setincrement([-cell_rad,cell_rad],'direction')
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
            if (complist != ''):
                msg("WARN: I will use clean components and model image to calculate visibilities,",color="31",origin="setup model")
                msg("         but the difference image won't have the clean components in it",color="31",origin="setup model")
        





        ##################################################################
        # open original model image as in_ia and keep open for now

        in_ia=ia.newimagefromfile(modelimage)
    
        # model image size and coordsys:
        in_shape=in_ia.shape()
        in_csys=in_ia.coordsys()

        # if the user wants to use the input pixel size for the output, then
        # get that from the header:                
        if ignorecoord:
            if cell=="incell":
                msg("ERROR: You can't use the input header for the pixel size and also ignore the input header information!",color="31")
                return False
            else:
                in_cell=qa.convert(cell,'arcsec')
        else:
            # model image pixel sizes
            increments=in_csys.increment(type="direction")['numeric']
            incellx=abs(increments[0])
            incelly=increments[1]
    
            # warn if input pixels are not square
            if (abs(incellx)-abs(incelly))/abs(incellx) > 0.001 and not ignorecoord:
                msg("input pixels are not square!",color="31",origin="setup model")
                if verbose:
                    msg("using cell = %s (not %s)" % (incellx,incelly),origin="setup model")
                else:
                    msg("using cell = %s" % incellx,origin="setup model")
            incellx=qa.quantity(incellx,'rad')
            incelly=qa.quantity(incelly,'rad')
            in_cell=qa.convert(incellx,'arcsec')
            

        if cell=="incell":                
            out_cell=in_cell
        else:
            out_cell=qa.convert(cell,'arcsec')

        if verbose:
            msg("input model image shape= %s" % in_shape,origin="setup model")
            msg("input model pixel size = %8.2e arcsec" % in_cell['value'],origin="setup model")







        ##################################################################
        # set imcenter to the center of the mosaic
        # in clean, we also set to phase center; cleaning with no pointing
        # at phase center causes issues

        if type(direction) == list:
            # manually specified pointings            
            nfld = direction.__len__()
            if nfld > 1 :
                pointings = direction
                if verbose: msg("You are inputing the precise pointings in 'direction' - if you want me to fill the mosaic, give a single direction")
            else:
                # calculate pointings for the user 
                nfld, pointings = util.calc_pointings(pointingspacing,out_size,out_cell,direction,relmargin)
                #if verbose: msg("Calculated %i pointings in an output image of %s pixels by %s" % (nfld,str(out_size),str(pointingspacing)))
        else:
            # calculate pointings for the user 
            nfld, pointings = util.calc_pointings(pointingspacing,out_size,out_cell,direction,relmargin)
            #if verbose: msg("Calculated %i pointings in an output image of %s pixels by %s" % (nfld,str(out_size),str(pointingspacing)))
            
                

        # find imcenter=average central point, and centralpointing
        # (to be used for phase center)
        imcenter , offsets = util.average_direction(pointings)

        minoff=1e10
        central=-1
        
        for i in range(nfld):
            o=pl.sqrt(offsets[0,i]**2+offsets[1,i]*2)
            if o<minoff:
                minoff=o
                central=i
        centralpointing=pointings[central]
            
        # (set back to simdata after calls to util -
        #   there must be an automatic way to do this)
        casalog.origin('simdata')

        if nfld==1:
            imagermode=''
            msg("phase center = " + centralpointing)
        else:
            imagermode="mosaic"
            msg("mosaic center = " + imcenter + "; phase center = " + centralpointing)
            if verbose: 
                for dir in pointings:
                    msg("   "+dir)


                

        ##################################################################
        # fit modelimage into a 4 coordinate image defined by the parameters
        # we will have one because we made one from clean components above

        # deal with brightness scaling here since we may close in_ia below.
        if (inbright=="unchanged") or (inbright=="default"):
            scalefactor=1.
        else:
            stats=in_ia.statistics()
            highvalue=stats['max']
            scalefactor=float(inbright)/highvalue.max()

        if verbose: msg("scaling image brightness by a factor of %f" % scalefactor,origin="setup model")
            


        # truncate model image name to craete new images in current dir:
        (modelimage_path,modelimage_local)=os.path.split(os.path.normpath(modelimage))
        modelimage_local=modelimage_local.strip()
        if modelimage_local.endswith(".fits"): modelimage_local=modelimage_local.replace(".fits","")
        if modelimage_local.endswith(".FITS"): modelimage_local=modelimage_local.replace(".FITS","")
        if modelimage_local.endswith(".fit"): modelimage_local=modelimage_local.replace(".fit","")






        # check shape characteristics of the input; add degenerate axes as needed:
        # does input model have direction/spectral/stokes data?
        in_dir=in_csys.findcoordinate("direction")
        in_spc=in_csys.findcoordinate("spectral")
        in_stk=in_csys.findcoordinate("stokes")


        if verbose: msg("rearranging input data (may take some time for large cubes)")
        arr=in_ia.getchunk()
        axmap=[-1,-1,-1,-1]
        axassigned=[-1,-1,-1,-1]

        in_nax=arr.shape.__len__()
        if in_nax<2:
            msg("ERROR: Your input model has fewer than 2 dimensions.  Can't proceed",color="31")
            return False

        # we have at least two axes:

        # set model_refdir and model_cell according to ignorecoord
        # do we want to set model_refdir to imcenter or centralpointing?  I think
        # centralpointing, since that's where the output image will get made
        if ignorecoord:
            epoch, ra, dec = util.direction_splitter(centralpointing)
            if verbose: msg("setting model image direction to ra="+qa.angle(qa.div(ra,"15"))+" dec="+qa.angle(dec),origin="setup model")
            #model_refdir=[qa.convert(ra,'rad')['value'],qa.convert(dec,'rad')['value']]
            model_refdir='J2000 '+qa.formxxx(ra,format='hms')+" "+qa.formxxx(dec,format='dms')
            model_cell=out_cell # in arcsec
            axmap[0]=0 # direction in first two pixel axes
            axmap[1]=1
            axassigned[0]=0  # coordinate corresponding to first 2 pixel axes
            axassigned[1]=0
            
        else:            
            if not in_dir['return']:
                msg("ERROR: You don't have direction coordinates that I can understand, so either edit the header or set ignorecoord=True",color="31")
                return False            
            ra,dec = in_csys.referencevalue(type="direction")['numeric']
            model_refdir= in_csys.referencecode(type="direction")+" "+qa.formxxx(str(ra)+"rad",format='hms')+" "+qa.formxxx(str(dec)+"rad",format='dms')
            if in_dir['pixel'].__len__() != 2:
                msg("ERROR: I can't understand your direction coordinates, so either edit the header or set ignorecoord=True",color="31")
                return False            
            dirax=in_dir['pixel']
            axmap[0]=dirax[0]
            axmap[1]=dirax[1]                    
            model_cell=in_cell # in arcsec
            axassigned[dirax[0]]=0
            axassigned[dirax[1]]=0
            if verbose: msg("Direction coordinate (%i,%i) parsed" % (axmap[0],axmap[1]),origin="setup model")

        # if we only have 2d to start with:
        if in_nax==2:            
            # then we can't make a cube (at least, it would be boring)
            if nchan>1: 
                msg("WARN: you are trying to create a cube from a flat image; I can't do that (yet) so am going to make a flat image",color="31",origin="setup model")
                nchan=1
            # add an extra axis to be Spectral:
            arr=arr.reshape([arr.shape[0],arr.shape[1],1])
            in_shape=arr.shape
            in_nax=in_shape.__len__() # which should be 3
            if verbose: msg("Adding dummy spectral axis",origin="setup model")

        # we have at least 3 axes, either by design or by addition:
        if ignorecoord:
            add_spectral_coord=True
            extra_axis=2
        else:
            if in_spc['return']:
                if type(in_spc['pixel']) == int :
                    foo=in_spc['pixel']
                else:
                    foo=in_spc['pixel'][0]
                    msg("WARN: you seem to have two spectral axes",color="31")
                if arr.shape[foo]>1 and nchan==1:
                    if verbose: msg("WARN: You will be flattening your spectral dimension",origin="setup model")
                axmap[3]=foo
                axassigned[foo]=3
                model_restfreq=in_csys.restfrequency()
                in_startpix=in_csys.referencepixel(type="spectral")['numeric'][0]
                model_step=in_csys.increment(type="spectral")['numeric'][0]
                model_start=in_csys.referencevalue(type="spectral")['numeric'][0]-in_startpix*model_step
                model_step=str(model_step)+in_csys.units(type="spectral")
                model_start=str(model_start)+in_csys.units(type="spectral")
                add_spectral_coord=False
                if verbose: msg("Spectral Coordinate %i parsed" % axmap[3],origin="setup model")                
            else:
                # we're not ignoreing coord, but we have at least one extra axis
                # if we have a valid stokes axis, but not a valid spectral axis:
                if in_stk['return']:
                    axassigned[in_stk['pixel']]=2
                    axmap[2]=in_stk['pixel']
                    # AND, if we only had 3 axes, need to add dummy spectral:
                    if in_nax<4:
                        # then we can't make a cube (at least, it would be boring)
                        if nchan>1: 
                            msg("WARN: you are trying to create a cube from a flat image; I can't do that (yet) so am going to make a flat image",color="31",origin="setup model")
                            nchan=1
                        # add an extra axis to be Spectral:
                        arr=arr.reshape([arr.shape[0],arr.shape[1],arr.shape[2],1])
                        in_shape=arr.shape
                        in_nax=in_shape.__len__() # which should be 4
                        if verbose: msg("Adding dummy spectral axis",origin="setup model")
                        
                # find first unused axis - probably at end, but just in case its not:
                i=0
                extra_axis=-1
                while extra_axis<0 and i<4:
                    if axassigned[i]<0: extra_axis=i
                    i+=1
                if extra_axis<0:                    
                    msg("ERROR: I can't find an unused axis to make Spectral [%i %i %i %i] " % (axassigned[0],axassigned[1],axassigned[2],axassigned[3]),color="31",origin="setup model")
                    return False
                add_spectral_coord=True
                
        if add_spectral_coord:
            axmap[3]=extra_axis
            axassigned[extra_axis]=3
            if nchan>arr.shape[extra_axis]:
                nchan=arr.shape[extra_axis]
                msg("WARN: you are asking for more channels than you have - truncating output cube to %i channels (%s each)" % (nchan,str(chanwidth)),color="31",origin="setup model")
            if arr.shape[extra_axis]>nchan:
                # actually subsample someday:
                msg("WARN: you are asking for fewer channels (%i) than the input cube - increasing nchan to %i (%s each) " % (nchan,arr.shape[extra_axis],str(chanwidth)),color="31",origin="setup model")
                nchan=arr.shape[extra_axis]
            if arr.shape[extra_axis]>1:
                model_restfreq=startfreq
                model_start=startfreq
                model_step=chanwidth
            else:
                model_restfreq=startfreq
                model_start=startfreq            
                model_step=bandwidth
            if verbose: msg("Adding Spectral Coordinate",origin="setup model")



        # if we only have three axes, add one to be Stokes:
        if in_nax==3:
            arr=arr.reshape([arr.shape[0],arr.shape[1],arr.shape[2],1])
            in_shape=arr.shape
            in_nax=in_shape.__len__() # which should be 4
            add_stokes_coord=True
            extra_axis=3
            if verbose: msg("Adding dummy Stokes axis",origin="setup model")
            
        # we have at least 3 axes, either by design or by addition:
        if ignorecoord:
            add_stokes_coord=True
            extra_axis=3
        else:
            if in_stk['return']:
                model_stokes=in_csys.stokes()
                foo=model_stokes[0]
                for i in range(model_stokes.__len__()-1):
                    foo=foo+model_stokes[i+1]
                model_stokes=foo
                if type(in_stk['pixel']) == int:
                    foo=in_stk['pixel']
                else:
                    foo=in_stk['pixel'][0]
                    msg("WARN: you seem to have two stokes axes",color="31")                
                axmap[2]=foo
                axassigned[foo]=2
                if in_shape[foo]>4:
                    msg("ERROR: You appear to have more than 4 Stokes components - please edit your header and/or parameters",color="31")
                    return False                        
                add_stokes_coord=False
                if verbose: msg("Stokes Coordinate %i parsed" % axmap[2],origin="setup model")
            else:
                # find the unused axis:
                i=0
                extra_axis=-1
                while extra_axis<0 and i<4:
                    if axassigned[i]<0: extra_axis=i
                    i+=1
                if extra_axis<0:
                    msg("ERROR: I can't find an unused axis to make Stokes [%i %i %i %i] " % (axassigned[0],axassigned[1],axassigned[2],axassigned[3]),color="31",origin="setup model")
                    return False
                add_stokes_coord=True
                            

        if add_stokes_coord:
            axmap[2]=extra_axis
            axassigned[extra_axis]=2
            if arr.shape[extra_axis]>4:
                msg("ERROR: you have %i Stokes parameters in your potential Stokes axis %i.  something is wrong." % (arr.shape[extra_axis],extra_axis))
                return False
            if verbose: msg("Adding Stokes Coordinate",origin="setup model")
            if arr.shape[extra_axis]==4:                    
                model_stokes="IQUV"
            if arr.shape[extra_axis]==3:                    
                model_stokes="IQV"
                msg("WARN: setting IQV Stokes parameters from the 4th axis of you model.  If that's not what you want, then edit the header",origin="setup model")
            if arr.shape[extra_axis]==2:                    
                model_stokes="IQ"
                msg("WARN: setting IQ Stokes parameters from the 4th axis of you model.  If that's not what you want, then edit the header",origin="setup model")
            if arr.shape[extra_axis]<=1:                    
                model_stokes="I"



        if verbose: msg("axis map for model image = %i %i %i %i" %
            (axmap[0],axmap[1],axmap[2],axmap[3]),origin="setup model")








        ##################################################################
        # check inputs - need to add atmospheric window, better display of
        # where the actual observation block lies on the ephemeris window
        
        if checkinputs=="yes" or checkinputs=="only":
            currfignum=0
            pl.figure(currfignum)
            pl.ion()
            pl.clf()
            pl.subplot(121)
            model_min,model_max, model_rms = statim(modelimage,plot=display,incell=in_cell)
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
            pl.plot(range(2),'o')
            ax=pl.gca()
            l=ax.get_xticklabels()
            pl.setp(l,fontsize="x-small")
            l=ax.get_yticklabels()
            pl.setp(l,fontsize="x-small")
            pl.xlabel("coming soon",fontsize="x-small")
            pl.subplots_adjust(left=0.05,right=0.98,bottom=0.08,top=0.96,hspace=0.2,wspace=0.2)
            
            if checkinputs=="only":
                return
            else:
                if display==True:
                    pl.figure(currfignum+1)
                    pl.clf()
        else:
            model_min,model_max, model_rms = statim(modelimage,plot=False,incell=in_cell)





        ##################################################################
        # reset output shape it case nchan has changed

        out_shape=[out_size[0],out_size[1],out_nstk,nchan]

        if verbose:
            msg("simulated image desired shape= %s" % out_shape,origin="setup model")
            msg("simulated image desired pixel size = %8.2e arcsec" % out_cell['value'],origin="setup model")




        ##################################################################
        # set up observatory, feeds, etc
        # (has to be here since we may have changed nchan)

        if verbose:
            msg("preparing empty measurement set")

        sm.open(msfile)
        posobs=me.observatory(telescopename)
        diam=stnd;
        sm.setconfig(telescopename=telescopename, x=stnx, y=stny, z=stnz, 
                 dishdiameter=diam.tolist(), 
                 mount=['alt-az'], antname=antnames,
                 coordsystem='global', referencelocation=posobs)
        sm.setspwindow(spwname=fband, freq=startfreq, deltafreq=chanwidth, 
                   freqresolution=chanwidth, nchannels=nchan, 
                   stokes='XX YY')
        sm.setfeed(mode='perfect X Y',pol=[''])
        sm.setlimits(shadowlimit=0.01, elevationlimit='10deg')
        sm.setauto(0.0)
        for k in range(0,nfld):
            src=project+'_%d'%k
            sm.setfield(sourcename=src, sourcedirection=pointings[k],
                    calcode="C", distance='0m')
        reftime = me.epoch('TAI', refdate)
        sm.settimes(integrationtime=integration, usehourangle=True, 
                referencetime=reftime)
        for k in range(0,nfld) :
            src=project+'_%d'%k
            sttime=-qa.convert(qa.quantity(totaltime),'s')['value']/2.0+qa.convert(qa.quantity(totaltime),'s')['value']/nfld*k
            endtime=-qa.convert(qa.quantity(totaltime),'s')['value']/2.0+qa.convert(qa.quantity(totaltime),'s')['value']/nfld*(k+1)
            # this only creates blank uv entries
            sm.observe(sourcename=src, spwname=fband,
                   starttime=qa.quantity(sttime, "s"),
                   stoptime=qa.quantity(endtime, "s"));
        sm.setdata(fieldid=range(0,nfld))
        sm.setvp()

        msg("done setting up observations (blank visibilities)")




            
        # now, make a blank 4-dimensional image in the imager tool, 
        # with the same shape as input, and same stokes params as input,
        # but output _cell_ sizes and channel widths
        # this sets up the image for processing inside of the simulated ms,
        # and also gives us a template coordinate system
        modelimage4d=project+"."+modelimage_local+'.coord'        
        im.open(msfile)    
        im.selectvis(field=range(nfld), spw=0)

        # XXX should we center this at imcenter or at centralpointing?
        # the user may be surprised if the center of their image moves. 
        
        # XXXX if the input cube is in VEL, we need to actually
        # make sure it is getting importfitted to turn into freq -
        # if it started as fits, then it got transformed on ia.open,
        # but if it started as an image, then it won't get converted.

        if verbose: msg("im.defineimage(nx="+str(in_shape[axmap[0]])+", ny="+str(in_shape[axmap[1]])+
                       ", cellx="+str(model_cell)+", celly="+str(model_cell)+", stokes='"+str(model_stokes)+
                       "', phasecenter='"+str(model_refdir)+"', nchan="+str(in_shape[axmap[3]])+ 
                       ", mode=\"FREQ-LSRK\",start='"+str(model_start)+"',step="+str(model_step)+
                       ", restfreq='"+str(model_restfreq)+"')")


        im.defineimage(nx=in_shape[axmap[0]], ny=in_shape[axmap[1]],
                       cellx=model_cell,celly=model_cell, stokes=model_stokes,
                       phasecenter=model_refdir, nchan=in_shape[axmap[3]], 
                       mode="FREQ-LSRK",start=model_start,step=model_step,
                       restfreq=model_restfreq)

        im.make(modelimage4d)
        im.close()
        ia.open(modelimage4d)
        modelcsys=ia.coordsys()
        modelsize=ia.shape()



        # first assure that the ia image created by Imager has the expected
        # order - Kumar has enough hardcoded that it is unlikely to be otherwise.
        expected=['Direction', 'Direction', 'Stokes', 'Spectral']
        if modelcsys.axiscoordinatetypes() != expected:
            msg("ERROR: internal error with coordinate axis order created by Imager",color="31")
            msg(modelcsys.axiscoordinatetypes().__str__(),color="31")
            return False

        # more checks:
        foo=pl.array(modelsize)
        if not (pl.array(arr.shape) == pl.array(foo.take(axmap).tolist())).all():
            msg("ERROR: internal error: I'm confused about the shape if your model data cube",color="31")
            msg("have "+foo.take(axmap).__str__()+", want "+in_shape.__str__(),color="31")
            return False

        for ax in range(4):
            if axmap[ax] != ax:
                if verbose: msg("swapping input axes %i with %i" % (ax,axmap[ax]),origin="setup model")
                arr=arr.swapaxes(ax,axmap[ax])                        
                tmp=axmap[ax]
                axmap[ax]=ax
                axmap[tmp]=tmp                


        # there's got to be a better way to remove NaNs: :)
        for i0 in range(arr.shape[0]):
            for i1 in range(arr.shape[1]):
                for i2 in range(arr.shape[2]):
                    for i3 in range(arr.shape[3]):
                        foo=arr[i0,i1,i2,i3]
                        if foo!=foo: arr[i0,i1,i2,i3]=0.0

        if verbose: msg("model array minmax= %e %e" % (arr.min(),arr.max()),origin="setup model")
        ia.putchunk(arr*scalefactor)                    
        ia.close()

        in_ia.close()

        # coord image should now have correct Coordsys and shape


#        if shrinkspectral:
#            # we had more channels in the cube than requested in the output image.







            
        ##################################################################
        # do actual calculation of visibilities from the model image:

        if not components_only:
            # right now this is ok - if we only had components,
            # we have created modelimage4d from them but if
            # we had components and model image they are not yet combined
            msg("predicting from "+modelimage4d)
            if arr.nbytes > 5e7:
                msg("WARN: your model is large - this may take a while")
            sm.predict(imagename=[modelimage4d],complist=complist)
        else:   # if we're doing only components
            msg("predicting from "+complist)
            sm.predict(complist=complist)

        sm.done()
        
        msg('generation of measurement set ' + msfile + ' complete')






        ######################################################################
        # noisify

        noise_any=False
    
        if noise_thermal:
            if not (util.telescopename == 'ALMA' or util.telescopename == 'ACA'):
                msg("WARN: thermal noise only works properly for ALMA/ACA",color="31",origin="noise")
                
            noise_any=True

            noisymsfile = project + ".noisy.ms"
            msg('adding thermal noise to ' + noisymsfile,origin="noise")

            eta_p, eta_s, eta_b, eta_t, eta_q, t_rx = util.noisetemp()

            # antenna efficiency
            eta_a = eta_p * eta_s * eta_b * eta_t
            if verbose: msg('antenna efficiency = ' + str(eta_a),origin="noise")
 
            # Ambient surface radiation temperature in K. 
            # FOR NOW, T_atm = T_ground = T_amb = parameter, also tau0=parameter
            t_ground = t_amb
            # Atmospheric radiation temperature in K. 
            t_atm = t_amb

            # Cosmic background radiation temperature in K. 
            t_cmb = 2.275

            if os.path.exists(noisymsfile):
                cu.removetable(noisymsfile)
            os.system("cp -r "+msfile+" "+noisymsfile)
            # XXX check for and copy flagversions file as well
            
            sm.openfromms(noisymsfile);    # an existing MS
            sm.setdata();                # currently defaults to fld=0,spw=0
            # type = B BPOLY G GSPLINE D P T TOPAC GAINCURVE
            sm.setapply(type='TOPAC',opacity=tau0);  # arrange opac corruption
            # NewMSSimulator needs 2-temp formula not just t_atm
            sm.setnoise(spillefficiency=eta_s,correfficiency=eta_q,
                        antefficiency=eta_a,trx=t_rx,
                        tau=tau0,tatmos=t_atm,tcmb=t_cmb,
                        mode="calculate")
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
        # clean if desired, WILL use noisy image for further calculation if present!
        # do we want to invert the noiseless one anyway for comparison?

        if noise_any:
            mstoimage = noisymsfile
        else:
            mstoimage = msfile

        if fidelity == True and psfmode == "none":
            msg("WARN: You can't calculate fidelity without imaging, so change psfmode if you want a fidelity image calculated.",origin="deconvolve")
            fidelity=False

        if display == True and psfmode == "none":
            msg("WARN: Without creating an image, there's very little to display, so I'm turning off display.  Change psfmode if you want to make an image.",origin="deconvolve")
            display=False

        if psfmode != "none": 
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

        #if verbose:
        # print clean inputs no matter what, so user can use them.
        msg("clean inputs:",origin="deconvolve")
        msg("clean(vis='"+mstoimage+"',imagename='"+image+"',field='',spw='',selectdata=False,timerange='',uvrange='',antenna='',scan='',mode='channel',niter="+str(niter)+",gain=0.1,threshold='"+str(threshold)+"',psfmode='"+psfmode+"',imagermode='"+imagermode+"',ftmachine='mosaic',mosweight=False,scaletype='SAULT',multiscale=[],negcomponent=-1,interactive=False,mask=[],nchan="+str(nchan)+",start=0,width=1,imsize="+str(imsize)+",cell='"+str(cell)+"',phasecenter='"+str(centralpointing)+"',restfreq='',stokes='"+stokes+"',weighting='"+weighting+"',robust="+str(robust)+",uvtaper="+str(uvtaper)+",outertaper="+str(outertaper)+",innertaper="+str(innertaper)+",modelimage='',restoringbeam=[''],pbcor=False,minpb=0.1,noise="+str(noise)+",npixels="+str(npixels)+",npercycle=100,cyclefactor=1.5,cyclespeedup=-1)")

        if psfmode != "none": 
            if int(casalog.version().split()[2].split('.')[1]) <= 3:
                # XXX 2.3.1 syntax:
                clean(vis=mstoimage,imagename=image,field='',spw='',selectdata=False,timerange='',uvrange='',antenna='',scan='',mode="channel",niter=niter,gain=0.1,threshold=threshold,psfmode=psfmode,imagermode=imagermode,ftmachine="mosaic",mosweight=False,scaletype="SAULT",multiscale=[],negcomponent=-1,interactive=False,mask=[],nchan=nchan,start=0,width=1,imsize=imsize,cell=cell,phasecenter=centralpointing,restfreq='',stokes=stokes,weighting=weighting,robust=robust,uvtaper=uvtaper,outertaper=outertaper,innertaper=innertaper,modelimage='',restoringbeam=[''],pbcor=False,minpb=0.1,noise=noise,npixels=npixels,npercycle=100,cyclefactor=1.5,cyclespeedup=-1)
                # XXX 2.4.0 syntax:
            else:
                clean(vis=mstoimage, imagename=image, field='', spw='', selectdata=False,
                      timerange='', uvrange='', antenna='', scan='', mode="channel",
                      interpolation='nearest', niter=niter, gain=0.1, threshold=threshold,
                      psfmode=psfmode, imagermode=imagermode, ftmachine="mosaic",
                      mosweight=False, scaletype="SAULT", multiscale=[],
                      negcomponent=-1, interactive=False, mask=[], nchan=nchan,
                      start=0, width=1, imsize=imsize, cell=cell, phasecenter=centralpointing,
                      restfreq='', stokes=stokes, weighting=weighting, robust=robust,
                      uvtaper=uvtaper, outertaper=outertaper, innertaper=innertaper,
                      modelimage='', restoringbeam=[''], pbcor=False, minpb=0.1,
                      noise=noise, npixels=npixels, npercycle=100,
                      cyclefactor=1.5, cyclespeedup=-1)
        else:
            image=project




        # need this filename whether or not we create the output image
        modelregrid=project+"."+modelimage_local+".flat"



        #####################################################################
        # prepare for diff and fidelity and display by making a moment 0 image


        inspectax=modelcsys.findcoordinate('spectral')['pixel']
        innchan=modelsize[inspectax]
        # modelimage4d is the .coord version w/spectral axis.

        stokesax=modelcsys.findcoordinate('stokes')['pixel']
        innstokes=modelsize[stokesax]

        # modelflat should be the moment zero of that
        modelflat=project+"."+modelimage_local+".flat0" 
        if innchan>1:
            if verbose: msg("creating moment zero input image",origin="analysis")
            # actually run ia.moments
            ia.open(modelimage4d)
            ia.moments(moments=[-1],outfile=modelflat,overwrite=True)
            ia.done()
        else:            
            # just remove degenerate axes from modelimage4d
            ia.newimagefromimage(infile=modelimage4d,outfile=modelflat,dropdeg=True,overwrite=True)
            if innstokes<=1:
                os.system("mv "+modelflat+" "+modelflat+".tmp")
                ia.open(modelflat+".tmp")
                ia.adddegaxes(outfile=modelflat,stokes='I',overwrite=True)
                ia.done()
                os.system("rm -rf "+modelflat+".tmp")
        if innstokes>1:
            os.system("mv "+modelflat+" "+modelflat+".tmp")
            po.open(modelflat+".tmp")
            foo=po.stokesi(outfile=modelflat,stokes='I')
            foo.done()
            po.done()
            os.system("rm -rf "+modelflat+".tmp")


        # flat output -- needed even if fidelity is not calculated
        if psfmode!="none" and (fidelity == True or display == True):
            outflat=image+".flat"
            if nchan>1:
                # image is either .clean or .dirty
                if verbose: msg("creating moment zero output image",origin="analysis")
                ia.open(image+".image")
                ia.moments(moments=[-1],outfile=outflat,overwrite=True)
                ia.done()
            else:
                if verbose: msg("flattening output image to "+outflat,origin="analysis")
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
            ia.open(modelflat)
            imrr = ia.regrid(outfile=modelregrid, overwrite=True,
                             csys=outflatcoordsys.torecord(),shape=outflatshape)
            imrr.done()
    
            # XXX when we get to using both clean components and model image,
            # we should add them together here if not before.
    
            ia.done()
            outflatcoordsys.done()
            del outflatcoordsys
            del imrr
    
            
            os.system("rm -rf "+modelflat)  # no need to keep this
    
                
            msg("done inverting and cleaning")








        #####################################################################
        # fidelity  

        if os.path.exists(modelimage) and (fidelity == True or display == True):
            # get beam from output clean image
            if verbose: msg("getting beam from "+image+".image",origin="analysis")
            ia.open(image+".image")
            beam=ia.restoringbeam()
            ia.done()

        if os.path.exists(modelimage) and fidelity == True: 
            # from math import sqrt
            
            # Convolve model with beam.
            convolved = project + '.convolved'
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
#                os.system("rm -rf "+outflat) 







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
        if psfmode != "none":
            sim_min,sim_max,sim_rms = statim(outflat,plot=display)

        # plot model image if exists
        if os.path.exists(modelimage) and display==True:
            if fidelity == True:
                pl.subplot(231)
            else:
                pl.subplot(221)
            # modelregrid might not exist if there's no display or fidelity
            if os.path.exists(modelregrid):
                model_min,model_max, model_rms = statim(modelregrid,plot=display)
                xlim=max(pl.xlim())
                ylim=max(pl.ylim())
            if display==True:
                tt=pl.array(range(25))*pl.pi/12
                pb=1.2*0.3/qa.convert(qa.quantity(startfreq),'GHz')['value']/aveant*3600.*180/pl.pi
                # XXX change this to use tb.open(ms/POINTINGS)/direction
                # and them make it a helper function
                for i in range(offsets.shape[1]):
                    pl.plot(pl.cos(tt)*pb/2+offsets[0,i]*3600,pl.sin(tt)*pb/2+offsets[1,i]*3600,'w')
                pl.xlim([xlim,-xlim])
                pl.ylim([-ylim,ylim])

            
            # fidelity image would only exist if there's a model image
            if modelimage != '' and fidelity == True:
                if display: pl.subplot(233)
                statim(project+".diff.im",plot=display)
                if display: pl.subplot(234)
                fidel_min, fidel_max, fidel_rms = statim(project+".fidelity.im",plot=display)

        if display:
            tb.open(mstoimage)
            # XXX use rob's FFT of the PB instead
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

            if modelimage != '' and psfmode != "none":   
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
                from matplotlib.font_manager import fontManager, FontProperties
                font= FontProperties(size='x-small');
                pl.legend(("bmaj=%7.1e" % beam['major']['value'],"bmin=%7.1e" % beam['minor']['value']),prop=font)                
                ia.done()

                
            pl.subplots_adjust(left=0.05,right=0.98,bottom=0.08,top=0.96,hspace=0.1,wspace=0.2)
        
        # if not displaying still print stats:
        if psfmode != "none":
            msg('Simulation rms: '+str(sim_rms),origin="analysis")
            msg('Simulation max: '+str(sim_max),origin="analysis")
        
        if display == True or fidelity == True:
            msg('Model rms: '+str(model_rms),origin="analysis")
            msg('Model max: '+str(model_max),origin="analysis")
            msg('Beam bmaj: '+str(beam['major']['value'])+' bmin: '+str(beam['minor']['value'])+' bpa: '+str(beam['positionangle']['value']),origin="analysis")






    except TypeError, e:
        msg("task_simdata -- TypeError: %s" % e,color="31")
        return
    except ValueError, e:
        print "task_simdata -- OptionError: ", e
        return
    except Exception, instance:
        print '***Error***',instance
        return
