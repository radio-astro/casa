import os
from taskinit import *
from simutil import *
import pylab as pl
import pdb

def simdata(
    project=None, 
    modifymodel=None,
    skymodel=None, inbright=None, indirection=None, incell=None, 
    incenter=None, inwidth=None, # innchan=None,
    setpointings=None,
    ptgfile=None, integration=None, direction=None, mapsize=None, 
    maptype=None, pointingspacing=None, caldirection=None, calflux=None, 
    predict=None, 
    refdate=None, reftime=None, complist=None, compwidth=None,
    totaltime=None, antennalist=None, 
    sdantlist=None, sdant=None,
    thermalnoise=None,
    user_pwv=None, t_ground=None, t_sky=None, tau0=None, leakage=None,
    image=None,
    vis=None, modelimage=None, cell=None, imsize=None, niter=None, threshold=None,
    weighting=None, mask=None, outertaper=None, stokes=None,     
    analyze=None, 
    showarray=None, showuv=None, showpsf=None, showmodel=None, 
    showconvolved=None, showclean=None, showresidual=None, showdifference=None, 
    showfidelity=None,
    graphics=None,
    verbose=None, 
    overwrite=None,
    async=False):


    # RI TODO for inbright=unchanged, need to scale input image to jy/pix

    casalog.origin('simdata')
    if verbose: casalog.filter(level="DEBUG2")

    a = inspect.stack()
    stacklevel = 0
    for k in range(len(a)):
        if (string.find(a[k][1], 'ipython console') > 0):
            stacklevel = k
    myf = sys._getframe(stacklevel).f_globals
     
    # create the utility object:
    util = simutil(direction)  # this is the dir of the observation - could be ""
    if verbose: util.verbose = True
    msg = util.msg

    # put output in directory called "project"
    fileroot = project
    if os.path.exists(fileroot):
#        if overwrite:
#            shutil.rmtree(fileroot)
#            os.mkdir(fileroot)
#        else:
        if not overwrite:
# this will break re-analysis of existing ms..
#            timestr = time.strftime("%y%b%d_%H:%M:%S",time.gmtime(os.path.getctime(fileroot)))
#            if os.path.exists(fileroot+"."+timestr):
#                 msg("project exists and overwrite=F",priority="error")
#                 return False
#            else:
#                shutil.move(fileroot,fileroot+"."+timestr)
            #                os.mkdir(fileroot)
            if (predict and os.path.exists(fileroot+"/"+project+".ms")):
                msg(fileroot+"/"+project+".ms exists but overwrite=F",priority="error")
                return False
            if (image and os.path.exists(fileroot+"/"+project+".model")):
                msg(fileroot+"/"+project+".model, image, and other imaging products exist but overwrite=F",priority="error")
                return False
            if (image and os.path.exists(fileroot+"/"+project+".fidelity")):
                msg(fileroot+"/"+project+".fidelity and other analysis products exist but overwrite=F",priority="error")
                return False
    else:
        os.mkdir(fileroot)


    saveinputs = myf['saveinputs']
    saveinputs('simdata',fileroot+"/"+project+".simdata.last")



    # some hardcoded variables that may be reintroduced in future development
    relmargin = .5  # number of PB between edge of model and pointing centers
    scanlength = 1  # number of integrations per scan

    # as of 20100507, newfig will delete the previous one

    
    if type(skymodel) == type([]):
        skymodel = skymodel[0]
    skymodel = skymodel.replace('$project',project)

    # check for people re-imaging only, in case they've turned modifymodel 
    # off without changing their imagename to project.skymodel
    if image and not predict and not modifymodel:
        if len(skymodel) < 9 or skymodel[-8:] != "skymodel":
            if os.path.exists(skymodel):
                msg("When imaging from a previously predicted project, you should either set modifymodel=T with the same settings as before, or set modifymodel=F and skymodel=$project.skymodel.",priority="warn")
                msg("Proceeding with "+project+".skymodel instead of "+skymodel,priority="warn")
                skymodel = fileroot + "/" + project + ".skymodel"


    if((not os.path.exists(skymodel)) and (not os.path.exists(complist))):
        msg("No sky input found.  At least one of skymodel or complist must be set.",priority="error")
        return False

    if((not os.path.exists(skymodel)) and (os.path.exists(complist))):
        msg("No skymodel found. Simulating from components only is new and still considered an 'expert' feature.",priority="warn")
#        if analyze:
#            msg("In particular, fidelity image may be misleading due to division by small values",priority="warn")

    # handle '$project' in modelimage
    modelimage = modelimage.replace('$project',project)

    grscreen = False
    grfile = False
    if graphics == "both":
        grscreen = True
        grfile = True
    if graphics == "screen":
        grscreen = True
    if graphics == "file":
        grfile = True
    
    try:
#    if True:

        if type(complist) == type([]):
            complist = complist[0]

        ##################################################################
        # set up skymodelimage

#         # check for people re-imaging only, in case they've turned modifymodel 
#         # off without changing their imagename to project.skymodel
#         if image and not predict and not modifymodel:
#             if len(skymodel) < 9 or skymodel[-8:] != "skymodel":
#                 if os.path.exists(skymodel):
#                     msg("When imaging from a previously predicted project, you should either set modifymodel=T with the same settings as before, or set modifymodel=F and skymodel=$project.skymodel.",priority="warn")
#                     msg("Proceeding with "+project+".skymodel instead of "+skymodel,priority="warn")
#                     skymodel = fileroot + "/" + project + ".skymodel"
#                 else:
#                     msg("When imaging from a previously predicted project, you should either set modifymodel=T with the same settings as before, or set modifymodel=F and skymodel=$project.skymodel.",priority="error")
#                     return False

        if os.path.exists(skymodel):
            components_only = False
            # if the skymodel is okay, work from it directly
            if util.is4d(skymodel) and os.path.isdir(skymodel) and not modifymodel:
                newmodel = skymodel
            else:
                # otherwise create $newmodel
                default_model = project + ".skymodel"
                if skymodel == default_model:
                    newmodel = fileroot + "/" + project + ".newmodel"
                else:
                    newmodel = fileroot + "/" + default_model
                if os.path.exists(newmodel):
                    if overwrite:
                        shutil.rmtree(newmodel)
                    else:
                        msg(newmodel+" exists -- please delete it, change skymodel, or set overwrite=T",priority="error")
                        return False

            # modifymodel just collects info if skymodel==newmodel
            innchan = -1
            (model_refdir,model_cell,model_size,
             model_nchan,model_center,model_width,
             model_stokes) = util.modifymodel(skymodel,
             newmodel,modifymodel,inbright,indirection,incell,
             incenter,inwidth,innchan,
             flatimage=False) 

            modelflat = newmodel + ".flat"
            if os.path.exists(modelflat) and (not predict) and analyze:
                # if we're not predicting, then we want to use the previously
                # created modelflat, because it may have components added 
                msg("flat sky model "+modelflat+" exists, predict not requested",priority="warn")
                msg(" working from existing model image - please delete it if you wish to overwrite.",priority="warn")
            else:
                # create and add components into modelflat with util.flatimage()
                util.flatimage(newmodel,complist=complist,verbose=verbose)

            casalog.origin('simdata')

            # set startfeq and bandwidth in util object after modifymodel
            bandwidth = qa.mul(qa.quantity(model_nchan),qa.quantity(model_width))
            util.bandwidth = bandwidth

        else:
            # if there are only components, modifymodel=T doesn't 
            # make sense
            if modifymodel:
                msg("can't find model image "+skymodel+" to modify",priority="error")
                return False
            components_only = True
            # if only components, the pointings 
            # can be displayed on blank sky, with symbols at the locations 
            # of components, but if analysis is going to be peformed, 
            # TODO create a sky model image here ?

            # we need model_refdir below for calibrator
            compdirs = []
            cl.open(complist)

            for i in range(cl.length()):
                compdirs.append(util.dir_m2s(cl.getrefdir(i)))

            model_refdir, coffs = util.average_direction(compdirs)
            model_center = cl.getspectrum(0)['frequency']['m0']
            # components don't yet support spectrum
            if util.isquantity(compwidth,halt=False):
                model_width = compwidth                
            else:
                model_width = "2GHz"
                msg("component-only simulation, compwidth unset: setting bandwidth to 2GHz",priority="warn")

            model_nchan = 1

            cmax = 0.0014 # ~5 arcsec
            for i in range(coffs.shape[1]):
                xc = pl.absolute(coffs[0,i])  # offsets in deg
                yc = pl.absolute(coffs[1,i])
                if xc > cmax:
                    cmax = xc
                if yc > cmax:
                    cmax = yc

            #model_size=qa.quantity(2*cmax,'deg'),qa.quantity(2*cmax,'deg')
            model_size = ["%fdeg" % (2*cmax), "%fdeg" % (2*cmax)]
            model_cell = ["0.1arcsec", "0.1arcsec"]

            
            if type(cell) == type([]):
                if len(cell) > 0:
                    cell0 = cell[0]
                else:
                    cell0 = ""
            else:
                cell0 = cell

            if len(cell0) > 0:
                model_cell = [cell0,cell0]


        if len(mapsize) == 0:
            mapsize = model_size
            if verbose: msg("setting map size to "+str(model_size))
        else:
             if type(mapsize) == type([]):
                 if len(mapsize[0]) == 0:
                     mapsize = model_size
                     if verbose: msg("setting map size to "+str(model_size))




        ##################################################################
        # read antenna file here to get Primary Beam
        predict_uv = False
        predict_sd = False
        sd_only = False
        aveant = -1
        stnx = []  # for later, to know if we read an array in or not
        repodir = os.getenv("CASAPATH").split(' ')[0] + "/data/alma/simmos/"

        # experimental: alma;0.4arcsec  allowed string
        if str.upper(antennalist[0:4]) == "ALMA":
            tail = antennalist[5:]
            if util.isquantity(tail,halt=False):
                resl = qa.convert(tail,"arcsec")['value']                
                if os.path.exists(repodir):
                    confnum = (2.867-pl.log10(resl*1000*qa.convert(model_center,"GHz")['value']/672.))/0.0721
                    confnum = max(1,min(28,confnum))
                    conf = str(int(round(confnum)))
                    if len(conf) < 2: conf = '0' + conf
                    antennalist = repodir + "alma.out" + conf + ".cfg"
                    msg("converted resolution to antennalist "+antennalist)
                else:
                    msg("failed to find antenna configuration repository at "+repodir,priority="warn")

        pb = 0. # primary beam

        # Search order is fileroot/ -> specified path -> repository
        if len(antennalist) > 0:
            if os.path.exists(fileroot+"/"+antennalist):
                antennalist = fileroot + "/" + antennalist
            elif not os.path.exists(antennalist) and \
                     os.path.exists(repodir+antennalist):
                antennalist = repodir + antennalist

        if os.path.exists(antennalist):
            stnx, stny, stnz, stnd, padnames, nant, telescopename = util.readantenna(antennalist)
            antnames = []
            for k in xrange(0,nant): antnames.append('A%02d'%k)
            aveant = stnd.mean()
            # TODO use max ant = min PB instead?  
            # (set back to simdata - there must be an automatic way to do this)
            casalog.origin('simdata')
            predict_uv = True
            pb = 1.2*0.3/qa.convert(qa.quantity(model_center),'GHz')['value']/aveant*3600.*180/pl.pi # arcsec


        # Search order is fileroot/ -> specified path -> repository
        if len(sdantlist) > 0:
            if os.path.exists(fileroot+"/"+sdantlist):
                sdantlist = fileroot + "/" + sdantlist
            elif not os.path.exists(sdantlist) and \
                     os.path.exists(repodir+sdantlist):
                sdantlist = repodir + sdantlist

        if os.path.exists(sdantlist):
            tpx, tpy, tpz, tpd, tp_padnames, tp_nant, tp_telescopename = util.readantenna(sdantlist)
            tp_antnames = []
            #for k in range(0,tp_nant): tp_antnames.append('TP%02d'%k)
            #select an antenna from thelist
            if sdant > tp_nant-1:
                msg("antenna index %d is out of range. setting sdant=0"%sdant,priority="warn")
                sdant = 0
            tp_antnames.append('TP%02d'%sdant)
            tpx = [tpx[sdant]]
            tpy = [tpy[sdant]]
            tpz = [tpz[sdant]]
            tpd = pl.array(tpd[sdant])
            tp_padnames = [tp_padnames[sdant]]
            tp_nant = 1
            tp_aveant = tpd.mean()
            casalog.origin('simdata')
            predict_sd = True
            if not predict_uv:
                aveant = tp_aveant
                msg("Only single-dish observation is predicted",priority="info")
                sd_only = True
            # check for image size (need to be > 2*pb)
            if not components_only:
                pb2 = 2.*1.2*0.3/qa.convert(qa.quantity(model_center),'GHz')['value']/tp_aveant*3600.*180/pl.pi
                minsize = min(qa.convert(model_size[0],'arcsec')['value'],\
                              qa.convert(model_size[1],'arcsec')['value'])
                if pb == 0:
                    pb = 0.5*pb2 #arcsec
                if minsize < pb2:
                    msg("skymodel should be larger than 2*primary beam. Your skymodel: %.3f arcsec < %.3f arcsec: 2*primary beam" % (minsize, pb2),priority="error")
                    del minsize,pb2
                    return False            
                del minsize,pb2






        ##################################################################
        # set up pointings
        dir = model_refdir
        dir0 = dir
        if type(direction) == type([]):
            if len(direction) > 0:
                if util.isdirection(direction[0],halt=False):
                    dir = direction
                    dir0 = direction[0]
        else:
            if util.isdirection(direction,halt=False):
                dir = direction
                dir0 = dir
        util.direction = dir0

        if setpointings:
            import re
            if verbose: util.msg("calculating map pointings centered at "+str(dir0))

            if len(pointingspacing) < 1:
                pointingspacing = "0.5PB"
            q = re.compile('(\d+.?\d+)\s*PB')
            qq = q.match(pointingspacing.upper())
            if qq:
                z = qq.groups()
                if pb <= 0:
                    util.msg("Can't calculate pointingspacing in terms of primary beam because neither antennalist nor sdantlist exist",priority="error")
                    return False
                pointingspacing = "%farcsec" % (float(z[0])*pb)
                # todo make more robust to nonconforming z[0] strings
            pointings = util.calc_pointings2(pointingspacing,mapsize,maptype=maptype, direction=dir)
            nfld=len(pointings)
            etime = qa.convert(qa.quantity(integration),"s")['value']
            ptgfile = fileroot + "/" + project + ".ptg.txt"
        else:
            if type(ptgfile) == type([]):
                ptgfile = ptgfile[0]
#            ptgfile = ptgfile.replace('$project',fileroot+"/"+project)
            ptgfile = ptgfile.replace('$project',project)
            if not os.path.exists(fileroot+"/"+ptgfile):
                if os.path.exists(ptgfile):
                    shutil.copyfile(ptgfile,fileroot+"/"+ptgfile)
                else:
                    util.msg("Can't find pointing file "+ptgfile,priority="error")
                    return False
            ptgfile = fileroot + "/" + ptgfile
            nfld, pointings, etime = util.read_pointings(ptgfile)
            # a string of different integrations doesn't work below yet
#            integration = [str(s)+"s" for s in etime]
            integration = str(etime[0]) + "s"
            util.msg("Using integration time "+integration+" for all pointings.",priority="warn")

            if max(etime) <= 0:
                etime = qa.convert(qa.quantity(integration),"s")['value']
            # expects that the cal is separate, and this is just one round of the mosaic

        # find imcenter - phase center
        imcenter , offsets = util.average_direction(pointings)        
        epoch, ra, dec = util.direction_splitter(imcenter)

        # model is centered at model_refdir, and has model_size; this is the offset in 
        # angular arcsec from the model center to the imcenter:        
        mepoch, mra, mdec = util.direction_splitter(model_refdir)
        if ra['value'] >= 359.999:
            ra['value'] = ra['value'] - 360.
        if mra['value'] >= 359.999:
            mra['value'] = mra['value'] - 360.
        shift = [ (qa.convert(ra,'deg')['value'] - 
                   qa.convert(mra,'deg')['value'])/pl.cos(qa.convert(mdec,'rad')['value'] ), 
                  (qa.convert(dec,'deg')['value'] - qa.convert(mdec,'deg')['value']) ]
        if verbose: 
            msg("pointings are shifted relative to the model by %g,%g arcsec" % (shift[0]*3600,shift[1]*3600))
        xmax = qa.convert(model_size[0],'deg')['value']
        ymax = qa.convert(model_size[1],'deg')['value']
        overlap = False        
        for i in xrange(offsets.shape[1]):
            xc = pl.absolute(offsets[0,i]+shift[0])  # offsets and shift are in degrees
            yc = pl.absolute(offsets[1,i]+shift[1])
            if xc < xmax and yc < ymax:
                overlap = True
                break

        if setpointings:
            if os.path.exists(ptgfile):
                if overwrite:
                    os.remove(ptgfile)
                else:
                    util.msg("pointing file "+ptgfile+" already exists and user does not want to overwrite",priority="error")
                    return False
            util.write_pointings(ptgfile,pointings,etime)

        msg("phase center = "+imcenter)
        if nfld > 1 and verbose:
            for idir in range(min(len(pointings),20)):
                msg("   "+pointings[idir])
            if nfld >= 20:
                msg("   (printing only first 20 - see pointing file for full list)")
            
 
        if not overlap:
            msg("No overlap between model and pointings",priority="error")
            return False



        ##################################################################
        # calibrator is not explicitly contained in the pointing file
        # but interleaved with etime=intergration
        util.isquantity(calflux)
        calfluxjy = qa.convert(calflux,'Jy')['value']
        # XML returns a list even for a string:
        if type(caldirection) == type([]): caldirection = caldirection[0]
        if len(caldirection) < 4: caldirection = ""
        if calfluxjy > 0 and caldirection != "":            
            docalibrator = True
            util.isdirection(caldirection)
            cl.done()
            cl.addcomponent(flux=calfluxjy,dir=caldirection,label="phase calibrator")
            # set reference freq to center freq of model
            cl.rename(fileroot+"/"+project+'.cal.cclist')
            cl.done()
        else:
            docalibrator = False





        ##################################################################
        # create one figure for model and pointings - need antenna diam 
        # to determine primary beam
        #if modifymodel or setpointings:
        if True: 
            if grfile:
                file = fileroot + "/" + project + ".skymodel.png"
            else:
                file = ""                            
    
            if grscreen or grfile:
                util.newfig(show=grscreen)

                if components_only:
                    pl.plot()
                    # TODO add symbols at locations of components
                    pl.plot(coffs[0,]*3600,coffs[1,]*3600,'o',c="#dddd66")
                    pl.axis("equal")

                else:
                    discard = util.statim(modelflat,plot=True,incell=model_cell)
                lims = pl.xlim(),pl.ylim()
                if pb <= 0 and verbose:
                    msg("unknown primary beam size for plot",priority="warn")
                if max(max(lims)) > pb:
                    plotcolor = 'w'
                else:
                    plotcolor = 'k'

                #if offsets.shape[1] > 16 or pb <= 0 or pb > pl.absolute(max(max(lims))):
                if offsets.shape[1] > 16 or pb <= 0:
                    lims = pl.xlim(),pl.ylim()
                    pl.plot((offsets[0]+shift[0])*3600.,(offsets[1]+shift[1])*3600.,
                            plotcolor+'+',markeredgewidth=1)
                    #if pb > 0 and pl.absolute(lims[0][0]) > pb:
                    if pb > 0:
                        plotpb(pb,pl.gca(),lims=lims,color=plotcolor)
                else:
                    from matplotlib.patches import Circle
                    for i in xrange(offsets.shape[1]):
                        pl.gca().add_artist(Circle(
                            ((offsets[0,i]+shift[0])*3600,
                             (offsets[1,i]+shift[1])*3600),
                            radius=pb/2.,edgecolor=plotcolor,fill=False,
                            label='beam',transform=pl.gca().transData,clip_on=True))

                xlim = max(abs(pl.array(lims[0])))
                ylim = max(abs(pl.array(lims[1])))
                # show entire pb: (statim doesn't by default)
                pl.xlim([max([xlim,pb/2]),min([-xlim,-pb/2])])
                pl.ylim([min([-ylim,-pb/2]),max([ylim,pb/2])])            
                pl.xlabel("resized model sky",fontsize="x-small")
                util.endfig(show=grscreen,filename=file)
    







        ##################################################################
        # set up observatory, feeds, etc        
        quickpsf_current = False

        msfile = fileroot + "/" + project + '.ms'
        sdmsfile = fileroot + "/" + project + '.sd.ms'
        sd_any = False
        if predict:
            if not(predict_uv or predict_sd):
                util.msg("must specify at least one of antennalist, sdantlist",priority="error")
                return False
            # TODO check for frequency overlap here - if zero stop
            # position overlap already checked above in pointing section

            if verbose:
                msg("preparing empty measurement set",origin="simdata",priority="warn")
            else:
                msg("preparing empty measurement set",origin="simdata")

            nbands = 1;    
            fband  = 'band' + qa.tos(model_center,prec=1)

            ############################################
            # predict interferometry observation
            if predict_uv: 
                if os.path.exists(msfile):
                    if not overwrite:
                        util.msg("measurement set "+msfile+" already exists and user does not wish to overwrite",priority="error")
                        return False                
                sm.open(msfile)
                posobs = me.observatory(telescopename)
                diam = stnd;
                # WARNING: sm.setspwindow is not consistent with clean::center
                #model_start=qa.sub(model_center,qa.mul(model_width,0.5*model_nchan))
                # but the "start" is the center of the first channel:
                model_start = qa.sub(model_center,qa.mul(model_width,0.5*(model_nchan-1)))

                sm.setconfig(telescopename=telescopename, x=stnx, y=stny, z=stnz, 
                             dishdiameter=diam.tolist(), 
                             mount=['alt-az'], antname=antnames, padname=padnames, 
                             coordsystem='global', referencelocation=posobs)
                if str.upper(telescopename).find('VLA') > 0:
                    sm.setspwindow(spwname=fband, freq=qa.tos(model_start), 
                                   deltafreq=qa.tos(model_width), 
                                   freqresolution=qa.tos(model_width), 
                                   nchannels=model_nchan, 
                                   stokes='RR LL')
                    sm.setfeed(mode='perfect R L',pol=[''])
                else:            
                    sm.setspwindow(spwname=fband, freq=qa.tos(model_start), 
                                   deltafreq=qa.tos(model_width), 
                                   freqresolution=qa.tos(model_width), 
                                   nchannels=model_nchan, 
                                   stokes='XX YY')
                    sm.setfeed(mode='perfect X Y',pol=[''])

                if verbose: msg(" spectral window set at %s" % qa.tos(model_center))
                sm.setlimits(shadowlimit=0.01, elevationlimit='10deg')
                sm.setauto(0.0)
                for k in xrange(0,nfld):
                    src = project + '_%d' % k
                    sm.setfield(sourcename=src, sourcedirection=pointings[k],
                                calcode="OBJ", distance='0m')
                    if k == 0:
                        sourcefieldlist = src
                    else:
                        sourcefieldlist = sourcefieldlist + ',' + src
                if docalibrator:
                    sm.setfield(sourcename="phase calibrator", 
                                sourcedirection=caldirection,calcode='C',
                                distance='0m')

                # what if someone has the old style refdate with time
                # included?  CATCH XXX 
  
                q = re.compile('(\d*/\d+/\d+)([/:\d]*)')
                qq = q.match(refdate)
                if not qq:
                    msg("Invalid reference date "+refdate,priority="error")
                    return
                else:
                    z = qq.groups()
                    refdate=z[0]
                    if len(z)>1:
                        msg("Discarding time part of refdate, "+z[1]+", in favor of reftime parameter = "+reftime)
 
                if reftime=="transit":
                    refdate=refdate+"/00:00:00"
                    usehourangle=True
                else:
                    refdate=refdate+"/"+reftime
                    usehourangle=False

                mereftime = me.epoch('TAI', refdate)
                sm.settimes(integrationtime=integration, usehourangle=usehourangle, 
                            referencetime=mereftime)
                totalsec = qa.convert(qa.quantity(totaltime),'s')['value']
                scantime = qa.mul(qa.quantity(integration),str(scanlength))
                scansec = qa.convert(qa.quantity(scantime),'s')['value']
                nscan = int(totalsec/scansec)
                kfld = 0
                # RI todo progress meter for simdata Sim::observe

                if nscan < nfld:
                    msg("Only %i pointings of %i in the mosaic will be observed - check mosaic setup and exposure time parameters!" % (nscan,nfld),priority="error")
                    return
        
                # sm.observemany
                observemany = True
                if observemany:
                    srces = []
                    starttimes = []
                    stoptimes = []
                    dirs = []

                for k in xrange(0,nscan) :
                    if usehourangle:
                        sttime = -totalsec/2.0 + scansec*k
                    else:
                        sttime = scansec*k # leave start at the reftime
                    endtime = sttime + scansec
                    src = project + '_%d' % kfld
                    if observemany:
                        srces.append(src)
                        starttimes.append(str(sttime)+"s")
                        stoptimes.append(str(endtime)+"s")
                        dirs.append(pointings[kfld])
                    else:
                    # this only creates blank uv entries
                        sm.observe(sourcename=src, spwname=fband,
                                   starttime=qa.quantity(sttime, "s"),
                                   stoptime=qa.quantity(endtime, "s"),project=project);
                    kfld = kfld + 1
                    if kfld == nfld: 
                        if docalibrator:
                            if usehourangle:
                                sttime = -totalsec/2.0 + scansec*k
                            else:
                                sttime = scansec*k
                            endtime = sttime + scansec
                            if observemany:
                                # need to observe cal singly to get new row in obs table, so 
                                # first observemany the on-source pointing(s)
                                sm.observemany(sourcenames=srces,spwname=fband,starttimes=starttimes,stoptimes=stoptimes,project=project)
                                # and clear the list
                                srces = []
                                starttimes = []
                                stoptimes = []
                                dirs = []
 #                               srces.append(src)
 #                               starttimes.append(str(sttime)+"s")
 #                               stoptimes.append(str(endtime)+"s")
 #                               dirs.append(caldirection)
 #                           else:
                            sm.observe(sourcename="phase calibrator", spwname=fband,
                                       starttime=qa.quantity(sttime, "s"),
                                       stoptime=qa.quantity(endtime, "s"),
                                       state_obs_mode="CALIBRATE_PHASE.ON_SOURCE",state_sig=True,
                                       project=project);
                        kfld = kfld + 1
                    if kfld > nfld: kfld = 0
                # if directions is unset, NewMSSimulator::observemany 

                # looks up the direction in the field table.
                if observemany and (not docalibrator):
                    sm.observemany(sourcenames=srces,spwname=fband,starttimes=starttimes,stoptimes=stoptimes,project=project)

                sm.setdata(fieldid=range(0,nfld))
                sm.setvp()

                msg("done setting up observations (blank visibilities)")
                if verbose: sm.summary()

                # do actual calculation of visibilities:

                if not components_only:                
                    if len(complist) > 1:
                        if verbose:
                            msg("predicting from "+newmodel+" and "+complist,priority="warn")
                        else:
                            msg("predicting from "+newmodel+" and "+complist)
                    else:
                        if verbose:
                            msg("predicting from "+newmodel,priority="warn")
                        else:
                            msg("predicting from "+newmodel)
                    sm.predict(imagename=newmodel,complist=complist)
                else:   # if we're doing only components
                    if verbose:
                        msg("predicting from "+complist,priority="warn")
                    else:
                        msg("predicting from "+complist)
                    sm.predict(complist=complist)
            
                sm.done()
                msg('generation of measurement set '+msfile+' complete')


            ##################################################################
            # predict single dish observation
            if predict_sd:
                if os.path.exists(sdmsfile):
                    if not overwrite:
                        util.msg("measurement set "+sdmsfile+" already exists and user does not wish to overwrite",priority="error")
                        return False
                sd_any = True

                sm.open(sdmsfile)
                posobs = me.observatory(tp_telescopename)
                diam = tpd
                # WARNING: sm.setspwindow is not consistent with clean::center
                model_start = qa.sub(model_center,qa.mul(model_width,0.5*(model_nchan-1)))

                sm.setconfig(telescopename=tp_telescopename, x=tpx, y=tpy, z=tpz, 
                             dishdiameter=diam.tolist(),
                             mount=['alt-az'], antname=tp_antnames, padname=tp_padnames, 
                             coordsystem='global', referencelocation=posobs)
                sm.setspwindow(spwname=fband, freq=qa.tos(model_start), 
                               deltafreq=qa.tos(model_width), 
                               freqresolution=qa.tos(model_width), 
                               nchannels=model_nchan, 
                               stokes='XX YY')
                sm.setfeed(mode='perfect X Y',pol=[''])

                if verbose: msg(" spectral window set at %s" % qa.tos(model_center))
                sm.setlimits(shadowlimit=0.01, elevationlimit='10deg')
                # auto-correlation should be unity for single dish obs.
                sm.setauto(1.0)
                for k in xrange(0,nfld):
                    src = project + '_%d' % k
                    sm.setfield(sourcename=src, sourcedirection=pointings[k],
                                calcode="OBJ", distance='0m')
                    if k == 0:
                        sourcefieldlist = src
                    else:
                        sourcefieldlist = sourcefieldlist + ',' + src
                if docalibrator:
                    msg("calibration is not supported for SD observation...skipped")
                #    sm.setfield(sourcename="phase calibrator", 
                #                sourcedirection=caldirection,calcode='C',
                #                distance='0m')
                mereftime = me.epoch('TAI', refdate)
                sm.settimes(integrationtime=integration, usehourangle=True, 
                            referencetime=mereftime)
                totalsec = qa.convert(qa.quantity(totaltime),'s')['value']
                scantime = qa.mul(qa.quantity(integration),str(scanlength))
                scansec = qa.convert(qa.quantity(scantime),'s')['value']
                nscan = int(totalsec/scansec)
                kfld = 0
                # RI todo progress meter for simdata Sim::observe

                if nscan < nfld:
                    msg("Only %i pointings of %i in the mosaic will be observed - check mosaic setup and exposure time parameters!" % (nscan,nfld),priority="error")
                    return
        
                # sm.observemany
                observemany = True
                #if observemany:
                srces = []
                starttimes = []
                stoptimes = []
                dirs = []

                for k in xrange(0,nscan) :
                    if usehourangle:
                        sttime = -totalsec/2.0 + scansec*k
                    else:
                        sttime = scansec*k
                    endtime = sttime + scansec
                    src = project + '_%d' % kfld
                    #if observemany:
                    srces.append(src)
                    starttimes.append(str(sttime)+"s")
                    stoptimes.append(str(endtime)+"s")
                    dirs.append(pointings[kfld])
                    #else:
                    ## this only creates blank uv entries
                    #    sm.observe(sourcename=src, spwname=fband,
                    #               starttime=qa.quantity(sttime, "s"),
                    #               stoptime=qa.quantity(endtime, "s"));
                    kfld = kfld + 1
                    if predict_uv and docalibrator and kfld == nfld:
                        # calibration obs is disabled for SD but add a gap to synchronize with interferometer
                        #if docalibrator:
                        #    sttime = -totalsec/2.0 + scansec*k
                        #    endtime = sttime + scansec
                        #    if observemany:
                        #        srces.append(src)
                        #        starttimes.append(str(sttime)+"s")
                        #        stoptimes.append(str(endtime)+"s")
                        #        dirs.append(caldirection)
                        #    else:
                        #        sm.observe(sourcename="phase calibrator", spwname=fband,
                        #                   starttime=qa.quantity(sttime, "s"),
                        #                   stoptime=qa.quantity(endtime, "s"));
                        kfld = kfld + 1
                    if kfld > nfld-1: kfld = 0
                # if directions is unset, NewMSSimulator::observemany 
                # looks up the direction in the field table.
                #if observemany:
                sm.observemany(sourcenames=srces,spwname=fband,starttimes=starttimes,stoptimes=stoptimes)

                sm.setdata(fieldid=range(0,nfld))
                sm.setvp()

                msg("done setting up observations (blank visibilities)")
                if verbose:
                    sm.summary()

                #######################################################
                # do actual calculation of visibilities:

                sm.setoptions(gridfunction='pb', ftmachine="sd", location=posobs)
                if not components_only:                
                    if len(complist) > 1:
                        msg("predicting from "+newmodel+" and "+complist,priority="warn")
                    else:
                        msg("predicting from "+newmodel,priority="warn")
                    sm.predict(imagename=newmodel,complist=complist)
                else:   # if we're doing only components
                    msg("predicting from "+complist,priority="warn")
                    sm.predict(complist=complist)
            
                sm.done()
                
                msg('generation of measurement set ' + sdmsfile + ' complete')


            ############################################
            # create figure 
            if grfile:            
                file = fileroot + "/" + project + ".predict.png"
            else:
                file = ""
            if predict_uv:
                multi = [2,2,1]
            else:
                multi = 0

            if (grscreen or grfile):
                util.newfig(multi=multi,show=grscreen)
                if predict_uv:
                    util.ephemeris(refdate,direction=util.direction,telescope=telescopename,ms=msfile,usehourangle=usehourangle)
                if predict_sd:
                    util.ephemeris(refdate,direction=util.direction,telescope=tp_telescopename,ms=sdmsfile,usehourangle=usehourangle)
                casalog.origin('simdata')
                if predict_uv:
                    util.nextfig()
                    util.plotants(stnx, stny, stnz, stnd, padnames)
                    if predict_sd:
                        msg("The total power antenna will not be shown on the array configuration")
                    
                    # uv coverage
                    util.nextfig()
                    tb.open(msfile)  
                    rawdata = tb.getcol("UVW")
                    tb.done()
                    pl.box()
                    maxbase = max([max(rawdata[0,]),max(rawdata[1,])])  # in m
                    klam_m = 300/qa.convert(model_center,'GHz')['value']
                    pl.plot(rawdata[0,]/klam_m,rawdata[1,]/klam_m,'b,')
                    pl.plot(-rawdata[0,]/klam_m,-rawdata[1,]/klam_m,'b,')
                    ax = pl.gca()
                    ax.yaxis.LABELPAD = -4
                    pl.xlabel('u[klambda]',fontsize='x-small')
                    pl.ylabel('v[klambda]',fontsize='x-small')
                    pl.axis('equal')
                    # Add single dish (zero-spacing)
                    if predict_sd:
                        pl.plot([0.],[0.],'r,')

                    # show dirty beam from observed uv coverage
                    util.nextfig()
                    im.open(msfile)  
                    # TODO spectral parms
                    if not image:
                        msg("using default model cell "+qa.tos(model_cell[0])+" for PSF calculation",priority="warn")
                    # defineim needs to be larger than synth beam
                    pixsize = (qa.convert(qa.quantity(model_cell[0]),'arcsec')['value'])
                    psfsize = 200.*klam_m/maxbase/pixsize
                    if psfsize < 32:
                        psfsize = 32
                    im.defineimage(cellx=qa.tos(model_cell[0]),nx=int(psfsize*8))
                    #im.makeimage(type='psf',image=project+".quick.psf")
                    if os.path.exists(fileroot+"/"+project+".quick.psf"):
                        shutil.rmtree(fileroot+"/"+project+".quick.psf")
                    im.approximatepsf(psf=fileroot+"/"+project+".quick.psf")
                    quickpsf_current = True
                    beam = im.fitpsf(psf=fileroot+"/"+project+".quick.psf")
                    im.done()
                    ia.open(fileroot+"/"+project+".quick.psf")
                    beamcs = ia.coordsys()
                    beam_array = ia.getchunk(axes=[beamcs.findcoordinate("spectral")['pixel'],beamcs.findcoordinate("stokes")['pixel']],dropdeg=True)
                    nn = beam_array.shape
                    xextent = nn[0]*pixsize*0.5
                    xextent = [xextent,-xextent]
                    yextent = nn[1]*pixsize*0.5
                    yextent = [-yextent,yextent]
                    flipped_array = beam_array.transpose()
                    ttrans_array = flipped_array.tolist()
                    ttrans_array.reverse()
                    pl.imshow(ttrans_array,interpolation='bilinear',cmap=pl.cm.jet,extent=xextent+yextent,origin="bottom")
                    pl.title(project+".quick.psf",fontsize="x-small")
                    b = qa.convert(beam['bmaj'],'arcsec')['value']
                    pl.xlim([-3*b,3*b])
                    pl.ylim([-3*b,3*b])
                    ax = pl.gca()
                    pl.text(0.05,0.95,"bmaj=%7.1e\nbmin=%7.1e" % (beam['bmaj']['value'],beam['bmin']['value']),transform = ax.transAxes,bbox=dict(facecolor='white', alpha=0.7),size="x-small",verticalalignment="top")
                    ia.done()
                util.endfig(show=grscreen,filename=file)



        else:
            # if not predicting this time, but are imageing or analyzing, 
            # get telescopename from ms
            # KS - telescopename seems not used in image and analyze
            # RI - possibly indirectly, through the util() object 4/8/11; 
            if (image or analyze) and os.path.exists(fileroot+"/"+project+'.ms'):
                tb.open(fileroot+"/"+project+".ms/OBSERVATION")
                n = tb.getcol("TELESCOPE_NAME")
                telescopename = n[0]
                util.telescopename = telescopename
                # todo add check that entire column is the same
                tb.done()

        ######################################################################
        # noisify

        noise_any = False
        msroot = fileroot + "/" + project  # if leakage, can just copy from this project
    
        if thermalnoise != "":
            knowntelescopes = ["ALMA", "ACA", "SMA", "EVLA", "VLA"]
            
            noise_any = True

            noisymsroot = msroot + ".noisy"
 
            # Cosmic background radiation temperature in K. 
            t_cmb = 2.725


            # check for interferometric ms:
            if os.path.exists(msroot+".ms"):
                if verbose:
                    msg('copying '+msroot+'.ms to ' + 
                        noisymsroot+'.ms and adding thermal noise',
                        origin="noise",priority="warn")
                else:
                    msg('copying '+msroot+'.ms to ' + 
                        noisymsroot+'.ms and adding thermal noise',
                        origin="noise")

                if os.path.exists(noisymsroot+".ms"):
                    shutil.rmtree(noisymsroot+".ms")                
                shutil.copytree(msfile,noisymsroot+".ms")
                if sm.name() != '':
                    msg("table persistence error on %s" % sm.name(),priority="error")
                    return

                #if sd_only:
                #    msg("sd_only set to False since you have "+msroot+".ms",priority="warn")
                #    sd_only=False
                
                # if not predicted this time, get telescopename from ms
                if not predict:
                    tb.open(noisymsroot+".ms/OBSERVATION")
                    n = tb.getcol("TELESCOPE_NAME")
                    telescopename = n[0]
                    # todo add check that entire column is the same
                    tb.done()
                    msg("telescopename read from "+noisymsroot+".ms: "+telescopename)

                if telescopename not in knowntelescopes:
                    msg("thermal noise only works properly for ALMA/ACA or EVLA",origin="noise",priority="warn")
                eta_p, eta_s, eta_b, eta_t, eta_q, t_rx = util.noisetemp(telescope=telescopename,freq=model_center)

                # antenna efficiency
                eta_a = eta_p * eta_s * eta_b * eta_t
                if verbose: 
                    msg('antenna efficiency    = '+str(eta_a), origin="noise")
                    msg('spillover efficiency  = '+str(eta_s), origin="noise")
                    msg('correlator efficiency = '+str(eta_q), origin="noise")

                sm.openfromms(noisymsroot+".ms")    # an existing MS
                sm.setdata(fieldid=[]) # force to get all fields
                if thermalnoise == "tsys-manual":
                    if verbose:
                        msg("sm.setnoise(spillefficiency="+str(eta_s)+
                            ",correfficiency="+str(eta_q)+",antefficiency="+str(eta_a)+
                            ",trx="+str(t_rx)+",tau="+str(tau0)+
                            ",tatmos="+str(t_sky)+",tground="+str(t_ground)+
                            ",tcmb="+str(t_cmb)+",mode='tsys-manual')");
                        msg("** this may be slow if your MS is finely sampled in time ** ",priority="warn")
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
                            ",pground='560mbar',altitude='5000m',waterheight='200m',relhum=20,pwv="+str(user_pwv)+"mm)");
                        msg("** this may be slow if your MS is finely sampled in time ** ",priority="warn")
                    sm.setnoise(spillefficiency=eta_s,correfficiency=eta_q,
                                antefficiency=eta_a,trx=t_rx,
                                tground=t_ground,tcmb=t_cmb,pwv=str(user_pwv)+"mm",
                                mode="tsys-atm",table=noisymsroot)
                    # don't set table, that way it won't save to disk
                    #                        mode="calculate",table=noisymsroot)
                sm.corrupt();
                sm.done();

            # now TP ms:
            # KS note: You want to noisify it if SD is predicted this time
            # (predict=predict_sd=T => sd_any=T) or not predicted but 
            # sdantlist is specified (predict=F, predict_sd=T).
            if (predict_sd or sd_any) and os.path.exists(sdmsfile):
                #msg('copying '+msroot+'.sd.ms to ' +
                msg('copying '+sdmsfile+' to ' + 
                    noisymsroot+'.sd.ms and adding thermal noise',
                    origin="noise",priority="warn")
                
                if os.path.exists(noisymsroot+".sd.ms"):
                    shutil.rmtree(noisymsroot+".sd.ms")
                shutil.copytree(sdmsfile,noisymsroot+".sd.ms")
                if sm.name() != '':
                    msg("table persistence error on %s" % sm.name(),priority="error")
                    return

                # if not predicted this time, get telescopename from ms
                if not predict:
                    tb.open(noisymsroot+".sd.ms/OBSERVATION")
                    n = tb.getcol("TELESCOPE_NAME")
                    tp_telescopename = n[0]
                    # todo add check that entire column is the same
                    tb.done()
                    msg("telescopename read from "+noisymsroot+".sd.ms: "+tp_telescopename)

                if tp_telescopename not in knowntelescopes:
                    msg("thermal noise only works properly for ALMA/ACA or EVLA",origin="noise",priority="warn")
                eta_p, eta_s, eta_b, eta_t, eta_q, t_rx = util.noisetemp(telescope=tp_telescopename,freq=model_center)

                # antenna efficiency
                eta_a = eta_p * eta_s * eta_b * eta_t
                if verbose: 
                    msg('antenna efficiency    = ' + str(eta_a),origin="noise")
                    msg('spillover efficiency  = ' + str(eta_s),origin="noise")
                    msg('correlator efficiency = ' + str(eta_q),origin="noise")

                sm.openfromms(noisymsroot+".sd.ms")    # an existing MS
                sm.setdata(fieldid=[]) # force to get all fields
                if thermalnoise == "tsys-manual":
                    if verbose:
                        msg("sm.[old]setnoise(spillefficiency="+str(eta_s)+
                            ",correfficiency="+str(eta_q)+",antefficiency="+str(eta_a)+
                            ",trx="+str(t_rx)+",tau="+str(tau0)+
                            ",tatmos="+str(t_sky)+
                            ",tcmb="+str(t_cmb));
                    sm.oldsetnoise(spillefficiency=eta_s,correfficiency=eta_q,
                                   antefficiency=eta_a,trx=t_rx,
                                   tau=tau0,tatmos=t_sky,tcmb=t_cmb,
                                   mode="calculate")
                else:
                    msg("Can't corrupt SD data using ATM library - please use tsys-manual",priority="error")
                    return False
                sm.corrupt();
                sm.done();
                # update TP ms name for the following steps
                sdmsfile = noisymsroot + ".sd.ms"
                sd_any = True

            msroot = noisymsroot
            if verbose: msg("done corrupting with thermal noise",origin="noise")


        if leakage > 0:
            noise_any = True
            if msroot == fileroot+"/"+project:
                noisymsroot = fileroot + "/" + project + ".noisy"
            else:
                noisymsroot = fileroot + "/" + project + ".noisier"
            if os.path.exists(msroot+".sd.ms"):
                msg("Can't corrupt SD data with polarization leakage",priority="warn")
            if os.path.exists(msfile):
                msg('copying '+msfile+' to ' + 
                    noisymsroot+'.ms and adding polarization leakage',
                    origin="noise",priority="warn")
                if os.path.exists(noisymsroot+".ms"):
                    shutil.rmtree(noisymsroot+".ms")                
                shutil.copytree(msfile,noisymsroot+".ms")
                if sm.name() != '':
                    msg("table persistence error on %s" % sm.name(),priority="error")
                    return

                sm.openfromms(noisymsroot+".ms")    # an existing MS
                sm.setdata(fieldid=[]) # force to get all fields
                sm.setleakage(amplitude=leakage,table=noisymsroot+".cal")
                sm.corrupt();
                sm.done();

                

            



        #####################################################################
        # clean if desired, use noisy image for further calculation if present
        # todo suggest a cell size from psf?

        # make sure cell is defined
        if type(cell) == type([]):
            if len(cell) > 0:
                cell0 = cell[0]
            else:
                cell0 = ""
        else:
            cell0 = cell
        if len(cell0) <= 0:
            cell = model_cell
        if type(cell) == type([]):
            if len(cell) == 1:
                cell = [cell[0],cell[0]]
        else:
            cell = [cell,cell]

        # cells are positive by convention
        cell = [qa.abs(cell[0]),qa.abs(cell[1])]

        # and imsize
        if type(imsize) == type([]):
            if len(imsize) > 0:
                imsize0 = imsize[0]
            else:
                imsize0 = -1
        else:
            imsize0 = imsize
        if imsize0 <= 0:
            imsize = [int(pl.ceil(qa.convert(qa.div(model_size[0],cell[0]),"")['value'])),
                      int(pl.ceil(qa.convert(qa.div(model_size[1],cell[1]),"")['value']))]

            


        #####################################################################
        if image:
            # if you neither predict nor noisify this time and already
            # have modelimage generated, set the name to tpimage
            #if not tpset and os.path.exists(modelimage):
            #    # should be CASA image so far. 
            #    tpimage=modelimage # KS-don't think this is necessary
            #    tpset=True

            # parse ms parameter and check for existance;
            # if noise_any
            #     mstoimage = noisymsfile
            # else:
            #     mstoimage = msfile
            mslist = vis.split(',')
            mstoimage = []
            tpmstoimage = None
            for ms0 in mslist:
                if not len(ms0): continue
                # if noisy ms was created, check for defaults:
                if (ms0 == "$project.ms" or ms0 == "$project.sd.ms") and noise_any:
                    msg("you are requesting to image $project[.sd].ms, but have created a corrupted $project.noisy[.sd].ms",priority="error");
                    msg("If you want to image the corrupted visibilites, you need to set vis=$project.noisy[.sd].ms in the image subtask",priority="error");

                ms1 = ms0.replace('$project',project)
                # MSes in fileroot/ have priority
                if os.path.exists(fileroot+"/"+ms1):
                    ms1 = fileroot + "/" + ms1
                if os.path.exists(ms1):
                    # check if the ms is tp data or not.
                    #if util.ismstp(ms1,halt=False) and tpset:
                    if util.ismstp(ms1,halt=False):
                        # if any SD operation in previous steps, check for vis
                        if sd_any and ms1 != sdmsfile:
                            msg("inconsistent vis name. you have generated a total power MS "+sdmsfile+", but are requesting vis="+ms1+" for imaging",priority="error")
                            return False
                        tpmstoimage = ms1
                        msg("Found a total power measurement set, %s." % ms1)
                    else:
                        mstoimage.append(ms1)
                        msg("Found a synthesis measurement set, %s." % ms1)
                else:
                    if verbose:
                        msg("measurement set "+ms1+" not found -- removing from imaging list",priority="warn")

                    else:
                        msg("measurement set "+ms1+" not found -- removing from imaging list")

            if not tpmstoimage and sd_any and os.path.exists(sdmsfile):
                msg("you have generated a total power MS "+sdmsfile+", but not specified it in 'vis' for imaging",priority="warn")
                msg("assuming you want to image it and creating a total power image",priority="warn")
                tpmstoimage = sdmsfile

            if len(mstoimage) == 0:
                if tpmstoimage:
                    sd_only = True
                else:
                    msg("no measurement sets found to image",priority="warn")
                    image = False
            else:
                sd_only = False

            # Do single dish imaging first if tpmstoimage exists.
            if tpmstoimage and os.path.exists(tpmstoimage):
                msg('creating image from generated ms: '+tpmstoimage)
                if len(mstoimage):
                    tpimage = project + '.sd.image'
                else:
                    tpimage = project + '.image'
                tpimage = fileroot + "/" + tpimage
                #if sd_only:
                #    # for analysis step -> do it later
                #    msfile=tpmstoimage
                #else:
                # check for modelimage
                if len(mstoimage):
                    if len(modelimage) and tpimage != modelimage and \
                           tpimage != fileroot+"/"+modelimage:
                        msg("modelimage parameter set to "+modelimage+" but also creating a new total power image "+tpimage,priority="warn")
                        msg("assuming you know what you want, and using modelimage="+modelimage+" in deconvolution",priority="warn")
                    else:
                        # This forces to use TP image as a model for clean
                        if len(modelimage) <= 0:
                            msg("you are generating total power image "+tpimage+". this is used as a model image for clean",priority="warn")
                        modelimage = tpimage
                
                # format image size properly
                sdimsize = imsize
                if not isinstance(imsize,list):
                    sdimsize = [imsize,imsize]
                elif len(imsize) == 1:
                    sdimsize = [imsize[0],imsize[0]]

                im.open(tpmstoimage)
                im.selectvis(nchan=model_nchan,start=0,step=1,spw=0)
                im.defineimage(mode='channel',nx=sdimsize[0],ny=sdimsize[1],cellx=cell[0],celly=cell[1],phasecenter=model_refdir,nchan=model_nchan,start=0,step=1,spw=0)
                #im.setoptions(ftmachine='sd',gridfunction='pb')
                im.setoptions(ftmachine='sd',gridfunction='pb')
                im.makeimage(type='singledish',image=tpimage)
                im.close()
                del sdimsize

                # For single dish: manually set the primary beam
                ia.open(tpimage)
                beam = ia.restoringbeam()
                if len(beam) == 0:
                    msg('setting primary beam information to image.')
                    # !! aveant will only be set if modifymodel or setpointingsm and in 
                    # any case it will the the aveant of the INTERFM array - we want the SD
                    tb.open(tpmstoimage+"/ANTENNA")
                    diams = tb.getcol("DISH_DIAMETER")
                    tb.done()
                    aveant = pl.mean(diams)
                    # model_center should be set even if we didn't predict this execution
                    pb = 1.2*0.3/qa.convert(qa.quantity(model_center),'GHz')['value']/aveant*3600.*180/pl.pi
                    beam['major'] = beam['minor'] = qa.quantity(pb,'arcsec')
                    beam['positionangle'] = qa.quantity(0.0,'deg')
                    msg('Primary beam: '+str(beam['major']))
                    ia.setrestoringbeam(beam=beam)
                ia.done()
                del beam

                msg('generation of total power image '+tpimage+' complete.')
                # update TP ms name the for following steps
                sdmsfile = tpmstoimage
                sd_any = True
                
                # End of single dish imaging part

        outflat_current = False
        convsky_current = False
        beam_current = False
        imagename = fileroot + "/" + project

        if image and len(mstoimage) > 0:
            if not predict:
                # get nfld, sourcefieldlist, from (interfm) ms if it was not just created
                tb.open(mstoimage[0]+"/SOURCE")
                code = tb.getcol("CODE")
                sourcefieldlist = pl.where(code=='OBJ')[0]
                nfld = len(sourcefieldlist)
                tb.done()
                msfile = mstoimage[0]

            # set cleanmode automatically (for interfm)
            if nfld == 1:
                cleanmode = "csclean"
            else:
                cleanmode = "mosaic"

            if not docalibrator:
                sourcefieldlist = ""  # sourcefieldlist should be ok, but this is safer
            
            # clean insists on using an existing model if its present
            if os.path.exists(imagename+".image"): shutil.rmtree(imagename+".image")
            if os.path.exists(imagename+".model"): shutil.rmtree(imagename+".model")

            # An image in fileroot/ has priority
            if len(modelimage) > 0 and os.path.exists(fileroot+"/"+modelimage):
                modelimage = fileroot + "/" + modelimage
                msg("Found modelimage, %s." % modelimage)

            # use imcenter instead of model_refdir
            util.image(mstoimage,imagename,
                       cleanmode,cell,imsize,imcenter,
                       niter,threshold,weighting,
                       outertaper,stokes,sourcefieldlist=sourcefieldlist,
                       modelimage=modelimage,mask=mask)

            # create imagename.flat and imagename.residual.flat:
            util.flatimage(imagename+".image",verbose=verbose)
            util.flatimage(imagename+".residual",verbose=verbose)
            outflat_current = True

            msg("done inverting and cleaning")
            if not type(cell) == type([]):
                cell = [cell,cell]
            if len(cell) <= 1:
                cell = [qa.quantity(cell[0]),qa.quantity(cell[0])]
            else:
                cell = [qa.quantity(cell[0]),qa.quantity(cell[1])]
            cell = [qa.abs(cell[0]),qa.abs(cell[0])]

            # get beam from output clean image
            if verbose: msg("getting beam from "+imagename+".image",origin="analysis")
            ia.open(imagename+".image")
            beam = ia.restoringbeam()
            beam_current = True
            ia.done()
            # model has units of Jy/pix - calculate beam area from clean image
            # (even if we are not plotting graphics)
            bmarea = beam['major']['value']*beam['minor']['value']*1.1331 #arcsec2
            bmarea = bmarea/(cell[0]['value']*cell[1]['value']) # bm area in pix
            msg("synthesized beam area in output pixels = %f" % bmarea)


            # show model, convolved model, clean image, and residual 
            if grfile:            
                file = fileroot + "/" + project + ".image.png"
            else:
                file = ""

        # create fake model from components for analysis
        if components_only and (image or analyze):
            newmodel = fileroot + "/" + project + ".compskymodel"
            if not os.path.exists(imagename+".image"):
                msg("must image before analyzing",priority="error")
                return False
            ia.imagecalc(pixels="'"+imagename+".image' * 0",outfile=newmodel,overwrite=True)
            ia.open(newmodel)
            cl.open(complist)
            ia.setbrightnessunit("Jy/pixel")
            ia.modify(cl.torecord(),subtract=False)
            cl.done()
            modelcsys = ia.coordsys()
            modelshape = ia.shape()

            modelflat = fileroot + "/" + project + ".compskymodel.flat"

            # TODO should be able to simplify degen axes code using new
            # image anal tools.
            inspectax = modelcsys.findcoordinate('spectral')['pixel']
            innchan = modelshape[inspectax]
            
            stokesax = modelcsys.findcoordinate('stokes')['pixel']
            innstokes = modelshape[stokesax]

            if innchan > 1:
                # actually run ia.moments
                ia.moments(moments=[-1],outfile=modelflat,overwrite=True)
                ia.done()
            else:   
                ia.done()

                # just remove degenerate axes from modelimage4d
                ia.newimagefromimage(infile=newmodel,outfile=modelflat,dropdeg=True,overwrite=True)
                if innstokes <= 1:
                    os.rename(modelflat,modelflat+".tmp")
                    ia.open(modelflat+".tmp")
                    ia.adddegaxes(outfile=modelflat,stokes='I',overwrite=True)
                    ia.done()
                    shutil.rmtree(modelflat+".tmp")
            if innstokes > 1:
                os.rename(modelflat,modelflat+".tmp")
                po.open(modelflat+".tmp")
                foo = po.stokesi(outfile=modelflat,stokes='I')
                foo.done()
                po.done()
                shutil.rmtree(modelflat+".tmp")


        if image and len(mstoimage) > 0:
            if grscreen or grfile:
                util.newfig(multi=[2,2,1],show=grscreen)

                # create regridded and convolved sky model image
                util.convimage(modelflat,imagename+".image.flat")
                convsky_current = True # don't remake this for analysis in this run

                disprange = []  # passing empty list causes return of disprange

                # original sky regridded to output pixels but not convolved with beam
                discard = util.statim(modelflat+".regrid",disprange=disprange)
                util.nextfig()

                # disprange from skymodel.regrid is in Jy/pix, but convolved im is in Jy/bm
                # bmarea is in units of output image pixels
                # unless we simulated from components in which case things 
                # are off
                if components_only:
                    disprange = []
                else:
                    disprange = [disprange[0]*bmarea,disprange[1]*bmarea]

                # convolved sky model - units of Jy/bm
                discard = util.statim(modelflat+".regrid.conv",disprange=disprange)                
                util.nextfig()
                
                # clean image - also in Jy/beam
                # although because of DC offset, better to reset disprange
                disprange = []
                discard = util.statim(imagename+".image.flat",disprange=disprange)
                util.nextfig()

                # clean residual image - Jy/bm
                discard = util.statim(imagename+".residual.flat",disprange=disprange)
                util.endfig(show=grscreen,filename=file)
        



        #####################################################################
        # analysis

        if analyze:
            if not os.path.exists(newmodel):
                msg("skymodel "+str(newmodel)+" not found",priority="warn")
                if not os.path.exists(complist):
                    return False
                else:
                    msg("If you have a sky model image, please set the skymodel parameter.",priority="warn")

            modelim = newmodel

            if not os.path.exists(modelim):
                msg("sky model image "+str(modelim)+" not found",priority="error")
                return False

            # so we should have modelim and modelim.flat created above, 
            # whether modifymodel is true or not.

            if not image:
                if not os.path.exists(imagename+".image"):
                    msg("you must image before analyzing.",priority="error")
                    return False

                # get beam from output clean image
                if verbose: msg("getting beam from "+imagename+".image",origin="analysis")
                ia.open(imagename+".image")
                beam = ia.restoringbeam()
                beam_current = True
                ia.done()
                # model has units of Jy/pix - calculate beam area from clean image
                # (even if we are not plotting graphics)
                bmarea = beam['major']['value']*beam['minor']['value']*1.1331 #arcsec2
                bmarea = bmarea/(cell[0]['value']*cell[1]['value']) # bm area in pix
                msg("synthesized beam area in output pixels = %f" % bmarea)


            # what about the output image?
            outim = imagename + ".image"
            if not os.path.exists(outim):
                msg("output image"+str(outim)+" not found",priority="warn")
                msg("you may need to run simdata.image, or if you deconvolved manually, rename your output to "+outim,priority="error")
                return False

            # flat output:?  if the user manually cleaned, this may not exist
            outflat = imagename + ".image.flat"
            if (not outflat_current) or (not os.path.exists(outflat)):
                # create imagename.flat and imagename.residual.flat
                if not image:
                    # get cell from outim
                    cell = util.cellsize(outim)
                util.flatimage(imagename+".image",verbose=verbose)
                if os.path.exists(imagename+".residual"):
                    util.flatimage(imagename+".residual",verbose=verbose)
                else:
                    if showresidual:
                        msg(imagename+".residual not found -- residual will not be plotted",priority="warn")
                    showresidual = False
                outflat_current = True
                
            # regridded and convolved input:?
            if not convsky_current:
                util.convimage(modelim+".flat",imagename+".image.flat")
                convsky_current = True
            
            # now should have all the flat, convolved etc even if didn't run "image" 

            # make difference image.
            # immath does Jy/bm if image but only if ia.setbrightnessunit("Jy/beam") in convimage()
            convolved = modelim + ".flat.regrid.conv"
            difference = imagename + '.diff'
            ia.imagecalc(difference, "'%s' - '%s'" % (convolved, outflat), overwrite=True)
            
            # get rms of difference image for fidelity calculation
            ia.open(difference)
            diffstats = ia.statistics(robust=True, verbose=False,list=False)
            maxdiff = diffstats['medabsdevmed']            
            if maxdiff != maxdiff: maxdiff = 0.
            if type(maxdiff) != type(0.):
                if maxdiff.__len__() > 0: 
                    maxdiff = maxdiff[0]
                else:
                    maxdiff = 0.
            # Make fidelity image.
            absdiff = imagename + '.absdiff'
            ia.imagecalc(absdiff, "max(abs('%s'), %f)" % (difference,
                                                          maxdiff/pl.sqrt(2.0)), overwrite=True)
            fidelityim = imagename + '.fidelity'
            ia.imagecalc(fidelityim, "abs('%s') / '%s'" % (convolved, absdiff), overwrite=True)
            msg("fidelity image calculated",origin="analysis")

            # scalar fidelity
            absconv = imagename + '.absconv'
            ia.imagecalc(absconv, "abs('%s')" % convolved, overwrite=True)
            ia.done()
            
            ia.open(absconv)
            modelstats = ia.statistics(robust=True, verbose=False,list=False)
            maxmodel = modelstats['max']            
            if maxmodel != maxmodel: maxmodel = 0.
            if type(maxmodel) != type(0.):
                if maxmodel.__len__() > 0: 
                    maxmodel = maxmodel[0]
                else:
                    maxmodel = 0.
            ia.done()
            scalarfidel = maxmodel/maxdiff
            msg("fidelity range (max model / rms difference) = "+str(scalarfidel),origin="analysis")


            # now, what does the user want to actually display?
            if len(stnx) <= 0:
                # array configuration is read from antenna list.
                # no use for SD only
                if showarray: msg("input data is not an array -- the array will not be plotted",priority="warn")
                showarray = False
            # need MS for showuv and showpsf
            if not (predict or image):
                msfile = filerot + "/" + project + ".ms"
            if sd_only and os.path.exists(sdmsfile):
                # use TP ms for UV plot if only SD sim, i.e.,
                # image=sd_only=T or (image=F=predict_uv and predict_sd=T)
                msfile = sdmsfile
            # psf is not available for SD only sim
            #if sd_only or util.ismstp(msfile,halt=False):
            if util.ismstp(msfile,halt=False):
                if showpsf: msg("single dish simulation -- psf will not be plotted",priority='warn')
                showpsf = False

            # if the order in the task input changes, change it here too
            figs = [showarray,showuv,showpsf,showmodel,showconvolved,showclean,showresidual,showdifference,showfidelity]
            nfig = figs.count(True)
            if nfig > 6:
                msg("only displaying first 6 selected panels in graphic output",priority="warn")
            if nfig <= 0:
                return True
            if nfig < 4:
                multi = [1,nfig,1]
            else:
                if nfig == 4:
                    multi = [2,2,1]
                else:
                    multi = [2,3,1]
                    
            if grfile:            
                file = fileroot + "/" + project + ".analysis.png"
            else:
                file = ""

            if grscreen or grfile:
                util.newfig(multi=multi,show=grscreen)

                # if order in task parameters changes, change here too
                if showarray:
                    util.plotants(stnx, stny, stnz, stnd, padnames)
                    if predict_sd:
                        msg("The total power antenna will not be shown on the array configuration")
                    util.nextfig()

                if showuv:
                    tb.open(msfile)
                    rawdata = tb.getcol("UVW")
                    tb.done()
                    pl.box()
                    maxbase = max([max(rawdata[0,]),max(rawdata[1,])])  # in m
                    klam_m = 300/qa.convert(model_center,'GHz')['value']
                    pl.plot(rawdata[0,]/klam_m,rawdata[1,]/klam_m,'b,')
                    pl.plot(-rawdata[0,]/klam_m,-rawdata[1,]/klam_m,'b,')
                    ax = pl.gca()
                    ax.yaxis.LABELPAD = -4
                    pl.xlabel('u[klambda]',fontsize='x-small')
                    pl.ylabel('v[klambda]',fontsize='x-small')
                    pl.axis('equal')
                    # Add zero-spacing (single dish) if not yet plotted
                    if predict_sd and not util.ismstp(msfile,halt=False):
                        pl.plot([0.],[0.],'r,')
                    util.nextfig()

                if showpsf:
                    pixsize = (qa.convert(qa.quantity(model_cell[0]),'arcsec')['value'])
                    if image: 
                        psfim = imagename + ".psf"
                    else:
                        psfim = project + ".quick.psf"
                        if not quickpsf_current:
                            im.open(msfile)  
                            # TODO spectral parms
                            # defineim needs to be larger than synth beam
                            psfsize = 200.*klam_m/maxbase/pixsize
                            if psfsize < 32: psfsize = 32
                            im.defineimage(cellx=qa.tos(model_cell[0]),nx=psfsize*8)
                            if os.path.exists(psfim):
                                shutil.rmtree(psfim)
                            im.approximatepsf(psf=psfim)
                            # beam is set above (even in "analyze" only)
                            # note that if image, beam has fields 'major' whereas if not, it 
                            # has fields like 'bmaj'.  
                            # beam=im.fitpsf(psf=psfim)  
                            im.done()
                    ia.open(psfim)            
                    beamcs = ia.coordsys()
                    beam_array = ia.getchunk(axes=[beamcs.findcoordinate("spectral")['pixel'],beamcs.findcoordinate("stokes")['pixel']],dropdeg=True)
                    nn = beam_array.shape
                    xextent = nn[0]*pixsize*0.5
                    xextent = [xextent,-xextent]
                    yextent = nn[1]*pixsize*0.5
                    yextent = [-yextent,yextent]
                    flipped_array = beam_array.transpose()
                    ttrans_array = flipped_array.tolist()
                    ttrans_array.reverse()
                    pl.imshow(ttrans_array,interpolation='bilinear',cmap=pl.cm.jet,extent=xextent+yextent,origin="bottom")
                    pl.title(psfim,fontsize="x-small")
                    b = qa.convert(beam['major'],'arcsec')['value']
                    pl.xlim([-3*b,3*b])
                    pl.ylim([-3*b,3*b])
                    ax = pl.gca()
                    pl.text(0.05,0.95,"bmaj=%7.1e\nbmin=%7.1e" % (beam['major']['value'],beam['minor']['value']),transform = ax.transAxes,bbox=dict(facecolor='white', alpha=0.7),size="x-small",verticalalignment="top")
                    ia.done()
                    util.nextfig()

                disprange = []  # first plot will define range
                if showmodel:
                    discard = util.statim(modelflat+".regrid",incell=cell,disprange=disprange)
                    util.nextfig()

                if showconvolved:
                    discard = util.statim(modelflat+".regrid.conv")
                    # if disprange gets set here, it'll be Jy/bm
                    util.nextfig()
                
                if showclean:
                    # own scaling because of DC/zero spacing offset
                    discard = util.statim(imagename+".image.flat")
                    util.nextfig()

                if showresidual:
                    # it gets its own scaling
                    discard = util.statim(imagename+".residual.flat")
                    util.nextfig()

                if showdifference:
                    # it gets its own scaling.
                    discard = util.statim(imagename+".diff")
                    util.nextfig()

                if showfidelity:
                    # it gets its own scaling.
                    discard = util.statim(imagename+".fidelity")
                    util.nextfig()

                util.endfig(show=grscreen,filename=file)
            else:
                sim_min,sim_max,sim_rms = util.statim(imagename+".image.flat",plot=False)
                # if not displaying still print stats:
                # 20100505 ia.stats changed to return Jy/bm:
                msg('Simulation rms: '+str(sim_rms/bmarea)+" Jy/pix = "+
                    str(sim_rms)+" Jy/bm",origin="analysis")
                msg('Simulation max: '+str(sim_max/bmarea)+" Jy/pix = "+
                    str(sim_max)+" Jy/bm",origin="analysis")
                #msg('Simulation rms: '+str(sim_rms)+" Jy/pix = "+
                #    str(sim_rms*bmarea)+" Jy/bm",origin="analysis")
                #msg('Simulation max: '+str(sim_max)+" Jy/pix = "+
                #    str(sim_max*bmarea)+" Jy/bm",origin="analysis")
                msg('Beam bmaj: '+str(beam['major']['value'])+' bmin: '+str(beam['minor']['value'])+' bpa: '+str(beam['positionangle']['value']),origin="analysis")



        # cleanup - delete newmodel, newmodel.flat etc
        if os.path.exists(modelflat):
            shutil.rmtree(modelflat)  
        if os.path.exists(modelflat+".regrid"):
            shutil.rmtree(modelflat+".regrid")  
#        if os.path.exists(modelflat+".regrid.conv"):
#            shutil.rmtree(modelflat+".regrid.conv")  
        if os.path.exists(imagename+".image.flat"):
            shutil.rmtree(imagename+".image.flat")  
        if os.path.exists(imagename+".residual.flat"):
            shutil.rmtree(imagename+".residual.flat")  
        if os.path.exists(imagename+".flux"):
            shutil.rmtree(imagename+".flux")  
        absdiff = imagename + '.absdiff'        
        if os.path.exists(absdiff):
            shutil.rmtree(absdiff)   
        absconv = imagename + '.absconv'        
        if os.path.exists(absconv):
            shutil.rmtree(absconv)  
#        if os.path.exists(imagename+".diff"):
#            shutil.rmtree(imagename+".diff")  
#        if os.path.exists(fileroot+"/"+project+".noisy.T.cal"):
#            shutil.rmtree(fileroot+"/"+project+".noisy.T.cal")  
        if os.path.exists(imagename+".quick.psf") and os.path.exists(imagename+".psf"):
            shutil.rmtree(imagename+".quick.psf")  


    except TypeError, e:
        msg("task_simdata -- TypeError: %s" % e,priority="error")
        return
    except ValueError, e:
        print "task_simdata -- OptionError: ", e
        return
    except Exception, instance:
        print '***Error***',instance
        return


##### Helper functions to plot primary beam
def plotpb(pb,axes,lims=None,color='k'):
    # This beam is automatically scaled when you zoom in/out but
    # not anchored in plot area. We'll wait for Matplotlib 0.99
    # for that function. 
    #major=major
    #minor=minor
    #rangle=rangle
    #bwidth=max(major*pl.cos(rangle),minor*pl.sin(rangle))*1.1
    #bheight=max(major*pl.sin(rangle),minor*pl.cos(rangle))*1.1
    from matplotlib.patches import Rectangle, Circle #,Ellipse
    try:
        from matplotlib.offsetbox import AnchoredOffsetbox, AuxTransformBox
        box = AuxTransformBox(axes.transData)
        box.set_alpha(0.7)
        circ = Circle((pb,pb),radius=pb/2.,color=color,fill=False,\
                      label='primary beam',linewidth=2.0)
        box.add_artist(circ)
        pblegend = AnchoredOffsetbox(loc=3,pad=0.2,borderpad=0.,\
                                     child=box,prop=None,frameon=False)#,frameon=True)
        pblegend.set_alpha(0.7)
        axes.add_artist(pblegend)
    except:
        print "Using old matplotlib substituting with circle"
        # work around for old matplotlib
        boxsize = pb*1.1
        if not lims: lims = axes.get_xlim(),axes.get_ylim()
        incx = 1
        incy = 1
        if axes.xaxis_inverted(): incx = -1
        if axes.yaxis_inverted(): incy = -1
        #ecx = lims[0][0] + bwidth/2.*incx
        #ecy = lims[1][0] + bheight/2.*incy
        ccx = lims[0][0] + boxsize/2.*incx
        ccy = lims[1][0] + boxsize/2.*incy
    
        #box = Rectangle((lims[0][0],lims[1][0]),incx*bwidth,incy*bheight,
        box = Rectangle((lims[0][0],lims[1][0]),incx*boxsize,incy*boxsize,
                        alpha=0.7,facecolor='w',
                        transform=axes.transData) #Axes
        #beam = Ellipse((ecx,ecy),major,minor,angle=rangle,
        beam = Circle((ccx,ccy), radius=pb/2.,
                      edgecolor='k',fill=False,
                      label='beam',transform=axes.transData)
        #props = {'pad': 3, 'edgecolor': 'k', 'linewidth':2, 'facecolor': 'w', 'alpha': 0.5}
        #pl.matplotlib.patches.bbox_artist(beam,axes.figure.canvas.get_renderer(),props=props)
        axes.add_artist(box)
        axes.add_artist(beam)
