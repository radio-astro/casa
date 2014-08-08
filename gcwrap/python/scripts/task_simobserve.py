from taskinit import *
from simutil import *
import os
import re
import pylab as pl
import pdb

def simobserve(
    project=None, 
    skymodel=None, inbright=None, indirection=None, incell=None, 
    incenter=None, inwidth=None, # innchan=None,
    complist=None, compwidth=None,
    setpointings=None,
    ptgfile=None, integration=None, direction=None, mapsize=None, 
    maptype=None, pointingspacing=None, caldirection=None, calflux=None, 
    # observe=None,
    obsmode=None, 
    refdate=None, hourangle=None, 
    totaltime=None, antennalist=None, 
    sdantlist=None,
    sdant=None,
    thermalnoise=None,
    user_pwv=None, t_ground=None, t_sky=None, tau0=None, seed=None,
    leakage=None,
    graphics=None,
    verbose=None, 
    overwrite=None
    ):

    # Collect a list of parameter values to save inputs
    in_params =  locals()

    import re

    try:

        #########################
        # some hardcoded variables
        pbcoeff = 1.13 ##  PB defined as pbcoeff*lambda/d
        nyquist = 0.5/pbcoeff ## Nyquist spacing = PB*nyquist

        gridratio_int = 1./pl.sqrt(3) # times lambda/d
        gridratio_tp  = 1./3

        relmargin = .5  # number of PB between edge of model and ptg centers
        scanlength = 1  # number of integrations per scan






        # RI TODO for inbright=unchanged, need to scale input image to jy/pix
        # according to actual units in the input image

        casalog.origin('simobserve')
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
        from simutil import is_array_type

        # it was requested to make the user interface "observe" for what 
        # is sm.observe and sm.predict.
        # interally the code is clearer if we stick with predict so
        predict = obsmode.startswith('i') or obsmode.startswith('s')
        if predict:
            uvmode = obsmode.startswith('i')
            if not uvmode: antennalist = sdantlist
        elif sdantlist != "":
            if antennalist == "":
                uvmode = False
                antennalist = sdantlist
            else:
                #uvmode = True
                #msg("Both antennalist and sdantlist are defined. sdantlist will be ignored",priority="warn")
                msg("Both antennalist and sdantlist are defined. Define one of them.",priority="error")
                return False
        else:
            uvmode = True
        #    uvmode = (sdant < 0) #when flexible default values come available


        # put output in directory called "project"
        fileroot = project
        if not os.path.exists(fileroot):
            os.mkdir(fileroot)


        # filename parsing of cfg file here so that the project filenames 
        # can contain the cfg
        repodir = os.getenv("CASAPATH").split(' ')[0] + "/data/alma/simmos/"

        # convert "alma;0.4arcsec" to an actual configuration
        # can only be done after reading skymodel, so here, we just string parse
        if str.upper(antennalist[0:4]) == "ALMA":
            foo=antennalist.replace(';','_')
        elif len(antennalist) > 0:
            foo=antennalist
        else:
            msg("The name of antenna list (antennalist/sdantlist) is not specified",priority="error")

        if foo:
            foo=foo.replace(".cfg","")
            sfoo=foo.split('/')
            if len(sfoo)>1:
                foo=sfoo[-1]
            project=project+"."+foo


        if not overwrite:
            if (predict and uvmode and os.path.exists(fileroot+"/"+project+".ms")):
                msg(fileroot+"/"+project+".ms exists but overwrite=F",priority="error")
                return False
            if (predict and (not uvmode) and os.path.exists(fileroot+"/"+project+".sd.ms")):
                msg(fileroot+"/"+project+".sd.ms exists but overwrite=F",priority="error")
                return False


        saveinputs = myf['saveinputs']
        # something broken in saveinputs
        in_params['antennalist']=''+in_params['antennalist']+''
        saveinputs('simobserve',fileroot+"/"+project+".simobserve.last",
                   myparams=in_params)


        if is_array_type(skymodel):
            skymodel = skymodel[0]
        skymodel = skymodel.replace('$project',project)

        if is_array_type(complist):
            complist = complist[0]

        if((not os.path.exists(skymodel)) and (not os.path.exists(complist))):
            if len(skymodel)>0:
                msg("Your skymodel '"+skymodel+"' could not be found.",priority="warn")
            if len(complist)>0:
                msg("Your complist '"+complist+"' could not be found.",priority="warn")
            if len(skymodel)==0 and len(complist)==0:
                msg("At least one of skymodel or complist must be set.",priority="error")
                
            else:
                msg("No sky input found.  At least one of skymodel or complist must exist.",priority="error")


        ### WORKAROUND for wrong flux in COMP TP simulations
        if (obsmode.startswith("s") and os.path.exists(complist)):
            msg("Single dish simulation has a flux recovery issue when using a components list as an input.\nPlease generate compskymodel image first by obsmode='' and use the image as the skymodel input.\nSorry for the inconvenience.", priority="error")
            return False
        ### End of WORKAROUND

        grscreen = False
        grfile = False
        if graphics == "both":
            grscreen = True
            grfile = True
        if graphics == "screen":
            grscreen = True
        if graphics == "file":
            grfile = True

        ##################################################################
        # set up skymodel image

        if os.path.exists(skymodel):
            components_only = False
            # create a new skymodel called skymodel, or if its already there, called newmodel
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
            returnpars = util.modifymodel(skymodel,newmodel,
                                          inbright,indirection,incell,
                                          incenter,inwidth,innchan,
                                          flatimage=False)
            if not returnpars:
                return False

            (model_refdir,model_cell,model_size,
             model_nchan,model_center,model_width,
             model_stokes) = returnpars

            modelflat = fileroot + "/" + project + ".skymodel.flat"
            if os.path.exists(modelflat) and (not predict):
                # if we're not predicting, then we want to use the previously
                # created modelflat, because it may have components added 
                msg("flat sky model "+modelflat+" exists, predict not requested",priority="warn")
                msg(" working from existing model image - please delete it if you wish to overwrite.",priority="warn")
            else:
                # create and add components into modelflat with util.flatimage()
                util.flatimage(newmodel,complist=complist,verbose=verbose)
                # we want the skymodel.flat image to be called that no matter what 
                # the skymodel image is called, since that's what used in analysis
                if modelflat != newmodel+".flat":
                    if os.path.exists(modelflat):
                        shutil.rmtree(modelflat)
                    shutil.move(newmodel+".flat",modelflat)

            casalog.origin('simobserve')

            # set startfeq and bandwidth in util object after modifymodel
            bandwidth = qa.mul(qa.quantity(model_nchan),qa.quantity(model_width))
            util.bandwidth = bandwidth

        else:
            components_only = True
            # calculate model parameters from the component list:

            compdirs = []
            cl.done()
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
            model_stokes = "I"

            cmax = 0.0014 # ~5 arcsec
            for i in range(coffs.shape[1]):
                xc = pl.absolute(coffs[0,i])  # offsets in deg
                yc = pl.absolute(coffs[1,i])
                if xc > cmax:
                    cmax = xc
                if yc > cmax:
                    cmax = yc

            model_size = ["%fdeg" % (3*cmax), "%fdeg" % (3*cmax)]


        # for cases either if there is a skymodel or if there are only components,
        # if the user has not input a map size (for setpointings), then use model_size
        if len(mapsize) == 0:
            mapsize = model_size
            if verbose: msg("setting map size to "+str(model_size),origin='simobserve')
        else:
             if is_array_type(mapsize):
                 if len(mapsize[0]) == 0:
                     mapsize = model_size
                     if verbose: msg("setting map size to "+str(model_size),origin="simobserve")

        if components_only:
            if is_array_type(mapsize):
                map_asec = qa.convert(mapsize[0],"arcsec")['value']
            else:
                map_asec = qa.convert(mapsize,"arcsec")['value']
            if is_array_type(model_size):
                mod_asec = qa.convert(model_size[0],"arcsec")['value']
            else:
                mod_asec = qa.convert(model_size,"arcsec")['value']
            if map_asec>mod_asec:
                model_size=["%farcsec" % map_asec,"%farcsec" % map_asec]




        ##################################################################
        # read antenna file here to get Primary Beam, and estimate psfsize
        aveant = -1
        stnx = []  # for later, to know if we read an array in or not
        pb = 0. # primary beam

        # convert "alma;0.4arcsec" to an actual configuration
        if str.upper(antennalist[0:4]) == "ALMA":

            resparsed=False
            # test for cycle 1
            q = re.compile('.*CYCLE.?1.?;(.*)')
            qq = q.match(antennalist.upper())
            if qq:
                z = qq.groups()
                tail=z[0]
                tail=tail.lower()
                if util.isquantity(tail,halt=False):
                    resl = qa.convert(tail,"arcsec")['value']
                    if os.path.exists(repodir):
                        confnum = (1.044-6.733*pl.log10(resl*qa.convert(model_center,"GHz")['value']/345.))
                        confnum = max(1,min(6,confnum))
                        conf = str(int(round(confnum)))
                        antennalist = repodir + "alma.cycle1." + conf + ".cfg"
                        msg("converted resolution to antennalist "+antennalist)
                        resparsed=True
                    else:
                        msg("failed to find antenna configuration repository at "+repodir,priority="error")
            if not resparsed:
                q = re.compile('.*CYCLE.?2.?;(.*)')
                qq = q.match(antennalist.upper())
                if qq:
                    z = qq.groups()
                    tail=z[0]
                    tail=tail.lower()
                    if util.isquantity(tail,halt=False):
                        resl = qa.convert(tail,"arcsec")['value']
                        if os.path.exists(repodir):
                            confnum = 10.**(0.91-0.74*(resl*qa.convert(model_center,"GHz")['value']/345.))
                            confnum = max(1,min(7,confnum))
                            conf = str(int(round(confnum)))
                            antennalist = repodir + "alma.cycle2." + conf + ".cfg"
                            msg("converted resolution to antennalist "+antennalist)
                            resparsed=True
                        else:
                            msg("failed to find antenna configuration repository at "+repodir,priority="error")

            if not resparsed: # assume FS
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
                        msg("failed to find antenna configuration repository at "+repodir,priority="error")


        # Search order is fileroot/ -> specified path -> repository
        if len(antennalist) > 0:
            if os.path.exists(fileroot+"/"+antennalist):
                antennalist = fileroot + "/" + antennalist
            elif not os.path.exists(antennalist) and \
                     os.path.exists(repodir+antennalist):
                antennalist = repodir + antennalist
            # Now make sure the antennalist exists
            if not os.path.exists(antennalist):
                util.msg("Couldn't find antennalist: %s" % antennalist, priority="error")
                return False
        elif predict or components_only:
            # antennalist is required when predicting or components only
            util.msg("Must specify antennalist", priority="error")

        # Read antennalist
        if os.path.exists(antennalist):
            stnx, stny, stnz, stnd, padnames, nant, telescopename = util.readantenna(antennalist)
            if len(stnx) == 1:
                if predict and uvmode:
                    # observe="int" but antennalist is SD
                    util.msg("antennalist contains only 1 antenna", priority="error")
                uvmode = False
            antnames = []
            if not uvmode: #Single-dish
                # KS TODO: what if not predicting but SD with multi-Ants
                # in antennalist (e.g., aca.tp)? In that case, PB on plots and
                # pointingspacing="??PB" will not be correct for heterogeneous list.
                if sdant > nant-1 or sdant < -nant:
                    msg("antenna index %d is out of range. setting sdant=0"%sdant,priority="warn")
                    sdant = 0
                stnx = [stnx[sdant]]
                stny = [stny[sdant]]
                stnz = [stnz[sdant]]
                stnd = pl.array(stnd[sdant])
                padnames = [padnames[sdant]]
                nant = 1

            # (set back to simdata - there must be an automatic way to do this)
            casalog.origin('simobserve')

            for k in xrange(0,nant): antnames.append('A%02d'%k)
            aveant = stnd.mean()
            # TODO use max ant = min PB instead?
            pb = pbcoeff*0.29979/qa.convert(qa.quantity(model_center),'GHz')['value']/aveant*3600.*180/pl.pi # arcsec

            # PSF size
            if uvmode:
                # approx beam, to compare with model_cell:
                psfsize = util.approxBeam(antennalist,qa.convert(qa.quantity(model_center),'GHz')['value'])
            else: # Single-dish
                psfsize = pb
                # check for model size
                if not components_only:
                    minsize = min(qa.convert(model_size[0],'arcsec')['value'],\
                                  qa.convert(model_size[1],'arcsec')['value'])
                    if minsize < 2.5*pb:
                        msg("skymodel should be larger than 2.5*primary beam. Your skymodel: %.3f arcsec < %.3f arcsec: 2.5*primary beam" % (minsize, 2.5*pb),priority="error")
                    del minsize
        else:
            msg("Can't find antennalist",priority="error")
            return False


        # now we have an estimate of the psf from the antenna configuration, 
        # so we can guess a model_cell for the case of component-only 
        # simulation, 
        if components_only:
            # first set based on psfsize:
            # needs high subsampling because small shifts in placement of 
            # components lead to large changes in the difference image.
            model_cell = [ str(psfsize/20)+"arcsec", str(psfsize/20)+"arcsec" ]
            
            # XXX if the user has set direction should we center the compskymodel there?
            # if len(direction)>0: model_refdir = direction

        # and can create a compskymodel image (tmp) and 
        # skymodel.flat which is what is needed for analysis.

        if components_only:
            newmodel = fileroot + "/" + project + ".compskymodel"
            needmodel=True

            modimsize=int((qa.convert(model_size[0],"arcsec")['value'])/(qa.convert(model_cell[0],"arcsec")['value']))
#            modimsize=max([modimsize,32])
            newepoch,newlat,newlon = util.direction_splitter(model_refdir)

            if os.path.exists(newmodel):
                if overwrite:
                    shutil.rmtree(newmodel)
                else:
                    needmodel=False
                    ia.open(newmodel)
                    oldshape=ia.shape()
                    if len(oldshape) != 2:
                        needmodel=True
                    else:
                        if oldshape[0] != modimsize or oldshape[1]==modimsize:
                            needmodel=True
                    oldcs=ia.coordsys()
                    ia.done()
                    olddir = (oldcs.referencevalue())['numeric']
                    if ( olddir[0] != qa.convert(newlat,oldcs.units()[0])['value'] or
                         olddir[1] != qa.convert(newlon,oldcs.units()[1])['value'] or
                         newepoch != oldcs.referencecode() ):
                        needmodel=True
                    oldcs.done()
                    del oldcs, olddir
                    if needmodel:
                        msg(newmodel+" exists and is inconsistent with required size="+str(modimsize)+" and direction. Please set overwrite=True",priority="error")
                        return False

            if needmodel:
                csmodel = ia.newimagefromshape(newmodel,[modimsize,modimsize,1,1])
                modelcsys = csmodel.coordsys()
                modelshape = csmodel.shape()
                cell0_rad=qa.convert(model_cell[0],'rad')['value']
                cell1_rad=qa.convert(model_cell[1],'rad')['value']
                modelcsys.setdirection(refpix=[modimsize/2,modimsize/2],
                                       refval=[qa.tos(newlat),qa.tos(newlon)],
                                       refcode=newepoch)
                modelcsys.setincrement([-cell0_rad,cell1_rad],'direction')
                modelcsys.setreferencevalue(type="spectral",value=qa.tos(model_center))
                modelcsys.setrestfrequency(qa.tos(model_center))
                modelcsys.setincrement(type="spectral",value=compwidth)
                csmodel.setcoordsys(modelcsys.torecord())
                modelcsys.done()
                cl.open(complist)
                csmodel.setbrightnessunit("Jy/pixel")
                csmodel.modify(cl.torecord(),subtract=False)
                cl.done()
                csmodel.done()
                # as noted, compskymodel doesn't need to exist, only skymodel.flat
                # flatimage adds in components if complist!=None
                #util.flatimage(newmodel,complist=complist,verbose=verbose)
                util.flatimage(newmodel,verbose=verbose)
                modelflat = fileroot + "/" + project + ".compskymodel.flat"
#                modelflat = fileroot + "/" + project + ".skymodel.flat"
#                if modelflat != newmodel+".flat":
#                    if os.path.exists(modelflat):
#                        shutil.rmtree(modelflat)
#                    shutil.move(newmodel+".flat",modelflat)


        # and finally, with model_cell set either from an actual skymodel,
        # or from the antenna configuration in components_only case,
        # we can check for the user that the psf is likely to be sampled enough:
        cell_asec=qa.convert(model_cell[0],'arcsec')['value']
        if psfsize < cell_asec:
            msg("Sky model cell of "+str(cell_asec)+" asec is very large compared to highest resolution "+str(psfsize)+" asec - this will lead to blank or erroneous output. (Did you set incell?)",priority="error")
            shutil.rmtree(modelflat)
            return False
        if psfsize < 2*cell_asec:
            msg("Sky model cell of "+str(cell_asec)+" asec is large compared to highest resolution "+str(psfsize)+" asec. (Did you set incell?)",priority="warn")

        # set this for future minimum image size
        minimsize = 8* int(psfsize/cell_asec)



        ##################################################################
        # set up pointings
        dir = model_refdir
        dir0 = dir
        if is_array_type(direction):
            if len(direction) > 0:
                if util.isdirection(direction[0],halt=False):
                    dir = direction
                    dir0 = direction[0]
        else:
            if util.isdirection(direction,halt=False):
                dir = direction
                dir0 = dir
        util.direction = dir0

        # if the integration time is a real time quantity
        # test for weird units
        if not util.isquantity(integration):
            msg("integration time "+integration+" does not appear to represent a time interval (use 's','min'; not 'sec','m')",priority="error")
            return False

        if qa.quantity(integration)['unit'] != '':
            if not qa.compare(integration,"1s"):
                msg("integration time "+integration+" does not appear to represent a time interval ('s','min'; not 'sec','m')",priority="error")
                return False
            intsec = qa.convert(qa.quantity(integration),"s")['value']
        else:
            if len(integration)>0:
                intsec = float(integration)
                msg("interpreting integration time parameter as "+str(intsec)+"s",priority="warn")
            else:
                intsec = 0
        integration="%fs" %intsec


        if setpointings:
            util.msg("calculating map pointings centered at "+str(dir0),origin='simobserve')

            if len(pointingspacing) < 1:
                if uvmode:
                    # ALMA OT uses lambda/d/sqrt(3)
                    pointingspacing = "%fPB" % (gridratio_int/pbcoeff) 
                else:
                    pointingspacing = "%fPB" % (gridratio_tp/pbcoeff) 
            if str.upper(pointingspacing)=="NYQUIST":
                pointingspacing="%fPB" % nyquist
            q = re.compile('(\d+.?\d+)\s*PB')
            qq = q.match(pointingspacing.upper())
            if qq:
                z = qq.groups()
                if pb <= 0:
                    util.msg("Can't calculate pointingspacing in terms of primary beam because antennalist is not specified",priority="error")
                    return False
                pointingspacing = "%farcsec" % (float(z[0])*pb)
                # todo make more robust to nonconforming z[0] strings

            if verbose:
                msg("pointing spacing in mosaic = "+pointingspacing,origin='simobserve')
            pointings = util.calc_pointings2(pointingspacing,mapsize,maptype=maptype, direction=dir, beam=pb)
            nfld=len(pointings)
            etime = qa.convert(qa.mul(qa.quantity(integration),scanlength),"s")['value']
            # etime is an array of scan lengths - here they're all the same.
            etime = etime + pl.zeros(nfld)
            # totaltime might not allow all fields to be observed, or it might
            # repeat
            ptgfile = fileroot + "/" + project + ".ptg.txt"
        else:
            if is_array_type(ptgfile):
                ptgfile = ptgfile[0]
            ptgfile = ptgfile.replace('$project',project)
            # precedence to ptg file outside the project dir
            if os.path.exists(ptgfile):
                shutil.copyfile(ptgfile,fileroot+"/"+project + ".ptg.txt")
                ptgfile = fileroot + "/" + project + ".ptg.txt"
            else:
                if os.path.exists(fileroot+"/"+ptgfile):
                    ptgfile = fileroot + "/" + ptgfile
                else:
                    util.msg("Can't find pointing file "+ptgfile,priority="error")
                    return False

            nfld, pointings, etime = util.read_pointings(ptgfile)
            if max(etime) <= 0:
                # integration is a string in input params
                etime = intsec
                # make etime into an array
                etime = etime + pl.zeros(nfld)
            # etimes determine stop/start i.e. the scan
            # if a longer etime is in the file, it'll do multiple integrations
            # per scan
            # expects that the cal is separate, and this is just one round of the mosaic
            # furthermore, the cal will use _integration_ from the inputs, and that
            # needs to be less than the min etime:
            if min(etime) < intsec:
                integration = str(min(etime))+"s"
                msg("Setting integration to "+integration+" to match the shortest time in the pointing file.",priority="warn")
                intsec = min(etime)


        # find imcenter - phase center
        imcenter , offsets = util.median_direction(pointings)     
        epoch, ra, dec = util.direction_splitter(imcenter)

        # model is centered at model_refdir, and has model_size;
        mepoch, mra, mdec = util.direction_splitter(model_refdir)
        # ra/mra should be in degrees, but just in case
        ra=qa.convert(ra,'deg')
        mra=qa.convert(mra,'deg')
        dec=qa.convert(dec,'deg')
        mdec=qa.convert(mdec,'deg')

        # observation near ra=0:
        if abs(mra['value'])<10 or abs(mra['value']-360.)<10 or abs(ra['value'])<10 or abs(mra['value']-360.)<10:
            nearzero=True
        else:
            nearzero=False

        # fix wraps
        if nearzero: # put break at 180
           if ra['value'] >= 180.:
               ra['value'] = ra['value'] - 360.
           if mra['value'] >= 180.:
               mra['value'] = mra['value'] - 360.
        else:
           if ra['value'] >= 360.:
               ra['value'] = ra['value'] - 360.
           if mra['value'] >= 360.:
               mra['value'] = mra['value'] - 360.

        # shift is the offset from the model to imcenter
        shift = [ (qa.convert(ra,'deg')['value'] -
                   qa.convert(mra,'deg')['value'])*pl.cos(qa.convert(dec,'rad')['value'] ),
                  (qa.convert(dec,'deg')['value'] - qa.convert(mdec,'deg')['value']) ]
        if verbose: 
            msg("pointings are shifted relative to the model by %g,%g arcsec" % (shift[0]*3600,shift[1]*3600),origin='simobserve')

        xmax = qa.convert(model_size[0],'deg')['value']*0.5
        ymax = qa.convert(model_size[1],'deg')['value']*0.5
        overlap = False
        # wrapang in median_direction should make offsets always small, not >360
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
            util.write_pointings(ptgfile,pointings,etime.tolist())

        msg("center = "+imcenter,origin='simobserve')
        if nfld > 1 and verbose:
            for idir in range(min(len(pointings),20)):
                msg("   "+pointings[idir],origin='simobserve')
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
        if is_array_type(caldirection): caldirection = caldirection[0]
        if len(caldirection) < 4: caldirection = ""
        if calfluxjy > 0 and caldirection != "" and uvmode:
            docalibrator = True
            if os.path.exists(fileroot+"/"+project+'.cal.cclist'):
                shutil.rmtree(fileroot+"/"+project+'.cal.cclist')
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
        if grfile:
            file = fileroot + "/" + project + ".skymodel.png"
        else:
            file = ""
    
        if grscreen or grfile:
            util.newfig(show=grscreen)

# remove after we fix the scaling algorithm for the images
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
            if max(max(lims)) > pb and not components_only:
                plotcolor = 'w'
            else:
                plotcolor = 'k'

            #if offsets.shape[1] > 16 or pb <= 0 or pb > pl.absolute(max(max(lims))):
            if offsets.shape[1] > 19 or pb <= 0:
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

        if uvmode:
            msfile = fileroot + "/" + project + '.ms'
        else:
            msfile = fileroot + "/" + project + '.sd.ms'

        if predict:
            # TODO check for frequency overlap here - if zero stop
            # position overlap already checked above in pointing section

            message = "preparing empty measurement set"
            if verbose:
                msg(" ",priority="info")
                msg(message,origin="simobserve",priority="warn")
            else:
                msg(message,origin="simobserve")

            nbands = 1;
            fband  = util.bandname(qa.convert(model_center, 'GHz')['value'])

            ############################################
            # predict observation

            # if someone has the old style refdate with the included, discard
            q = re.compile('(\d*/\d+/\d+)([/:\d]*)')
            qq = q.match(refdate)
            if not qq:
                msg("Invalid reference date "+refdate,priority="error")
                return False
            else:
                z = qq.groups()
                refdate=z[0]
                if len(z)>1:
                    if len(z[1])>1:
                        msg("Discarding time part of refdate, '"+z[1]+"', in favor of hourangle parameter = "+hourangle,origin='simobserve')

            if hourangle=="transit":
                haoffset=0.0
            else:                
                haoffset="no"
                # is this a time quantity?
                if qa.isquantity(hourangle):
                    qha=qa.convert(hourangle,"s")
                    if qa.compare(qha,"s"):
                        haoffset=qa.convert(qha,'s')['value']
                elif qa.isquantity(hourangle+"h"):
                    if qa.compare(hourangle+"h","s"):
                        haoffset=qa.convert(qa.quantity(hourangle+"h"),'s')['value']
            if haoffset=="no":
                msg("Cannot interpret your hourangle parameter "+hourangle+" as a time quantity e.g. '5h', 30min'",origin="simobserve",priority="error")
            else:
                msg("You desire an hour angle of "+str(haoffset/3600.)+" hours",origin="simobserve")                    

            refdate=refdate+"/00:00:00"
            usehourangle=True


            intsec = qa.convert(qa.quantity(integration),"s")['value']

            # totaltime as an integer for # times through the mosaic:
            if not util.isquantity(totaltime,halt=False):
                msg("totaltime "+totaltime+" does not appear to represent a time interval (use 's','min','h'; not 'sec','m','hr')",priority="error")
                return False

            if qa.quantity(totaltime)['value'] < 0.:
                # casapy crashes for negative totaltime
                msg("Negative totaltime is not allowed.",priority="error")
                return False
            if qa.quantity(totaltime)['unit'] == '':
                # assume it means number of maps, or # repetitions.
                totalsec = sum(etime)
                if docalibrator:
                    totalsec = totalsec + intsec # cal gets one int-time
                totalsec = float(totaltime) * totalsec
                msg("Total observing time = "+str(totalsec)+"s.",priority="warn")
            else:
                if not qa.compare(totaltime,"1s"):
                    msg("totaltime "+totaltime+" does not appear to represent a time interval (use 's','min','h'; not 'sec','m','hr')",priority="error")
                    return False
                totalsec = qa.convert(qa.quantity(totaltime),'s')['value']

            if os.path.exists(msfile) and not overwrite: #redundant check?
                util.msg("measurement set "+msfile+" already exists and user does not wish to overwrite",priority="error")
                return False
            sm.open(msfile)
            posobs = me.observatory(telescopename)
            diam = stnd;
            # WARNING: sm.setspwindow is not consistent with clean::center
            #model_start=qa.sub(model_center,qa.mul(model_width,0.5*model_nchan))
            # but the "start" is the center of the first channel:
            model_start = qa.sub(model_center,qa.mul(model_width,0.5*(model_nchan-1)))

            mounttype = 'alt-az'
            if telescopename in ['DRAO', 'WSRT']:
                mounttype = 'EQUATORIAL'
            # Should ASKAP be BIZARRE or something else?  It may be effectively equatorial.

            sm.setconfig(telescopename=telescopename, x=stnx, y=stny, z=stnz,
                         dishdiameter=diam.tolist(),
                         mount=[mounttype], antname=antnames, padname=padnames, 
                         coordsystem='global', referencelocation=posobs)
            if str.upper(telescopename).find('VLA') > 0:
                sm.setspwindow(spwname=fband, freq=qa.tos(model_start),
                               deltafreq=qa.tos(model_width),
                               freqresolution=qa.tos(model_width),
                               nchannels=model_nchan, refcode="LSRK",
                               stokes='RR LL')
                sm.setfeed(mode='perfect R L',pol=[''])
            else:
                sm.setspwindow(spwname=fband, freq=qa.tos(model_start),
                               deltafreq=qa.tos(model_width),
                               freqresolution=qa.tos(model_width), 
                               nchannels=model_nchan, refcode="LSRK",
                               stokes='XX YY')
                sm.setfeed(mode='perfect X Y',pol=[''])

            if verbose: msg(" spectral window set at %s" % qa.tos(model_center),origin='simobserve')
            sm.setlimits(shadowlimit=0.01, elevationlimit='10deg')
            if uvmode:
                sm.setauto(0.0)
            else: #Single-dish
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
                sm.setfield(sourcename="phase calibrator", 
                            sourcedirection=caldirection,calcode='C',
                            distance='0m')

            mereftime = me.epoch('TAI', refdate)
            # integration is a scalar quantity, etime is a vector of seconds
            sm.settimes(integrationtime=integration, usehourangle=usehourangle, 
                        referencetime=mereftime)

            # time required to observe all planned scanes in etime array:
            totalscansec = sum(etime)
            kfld = 0

            if totalsec < totalscansec:
                msg("Not all pointings in the mosaic will be observed - check mosaic setup and exposure time parameters!",priority="warn")
                ###
                #print "you need at least %16.12e sec but you have %16.12e sec (%f < %f = %s)" % (totalscansec, totalsec, totalsec, totalscansec, str(totalsec<totalscansec))
                ###

            # sm.observemany
            srces = []
            starttimes = []
            stoptimes = []
            dirs = []

            if usehourangle:
                sttime = -totalsec/2.0
            else:
                sttime = 0. # leave start at the reftime
            sttime=sttime+haoffset
            scanstart=sttime

            # can before sources
            if docalibrator:
                endtime = sttime + qa.convert(integration,'s')['value']
                sm.observe(sourcename="phase calibrator", spwname=fband,
                           starttime=qa.quantity(sttime, "s"),
                           stoptime=qa.quantity(endtime, "s"),
                           state_obs_mode="CALIBRATE_PHASE.ON_SOURCE",state_sig=True,
                           project=project);
                sttime = endtime

            while (sttime-scanstart) < totalsec: # the last scan could exceed totaltime
                endtime = sttime + etime[kfld]
                src = project + '_%d' % kfld
                srces.append(src)
                starttimes.append(str(sttime)+"s")
                stoptimes.append(str(endtime)+"s")
                dirs.append(pointings[kfld])
                kfld = kfld + 1
                # advance start time - XX someday slew goes here
                sttime = endtime

                if kfld == nfld:
                    if docalibrator:
                        endtime = sttime + qa.convert(integration,'s')['value'] 

                        # need to observe cal singly to get new row in obs table, so 
                        # first observemany the on-source pointing(s)
                        sm.observemany(sourcenames=srces,spwname=fband,starttimes=starttimes,stoptimes=stoptimes,project=project)
                        # and clear the list
                        srces = []
                        starttimes = []
                        stoptimes = []
                        dirs = []
                        sm.observe(sourcename="phase calibrator", spwname=fband,
                                   starttime=qa.quantity(sttime, "s"),
                                   stoptime=qa.quantity(endtime, "s"),
                                   state_obs_mode="CALIBRATE_PHASE.ON_SOURCE",state_sig=True,
                                   project=project);
                    kfld = kfld + 1
                    sttime = endtime
#                 if kfld > nfld: kfld = 0
                if kfld > nfld-1: kfld = 0
            # if directions is unset, NewMSSimulator::observemany

            # looks up the direction in the field table.
            if not docalibrator:
                sm.observemany(sourcenames=srces,spwname=fband,starttimes=starttimes,stoptimes=stoptimes,project=project)

            sm.setdata(fieldid=range(0,nfld))
            if uvmode or components_only: #Interferometer only
                sm.setvp()

            msg("done setting up observations (blank visibilities)",origin='simobserve')
            if verbose: sm.summary()

            # do actual calculation of visibilities:

            if not uvmode: #Single-dish
                sm.setoptions(gridfunction='pb', ftmachine="sd", location=posobs)

            if not components_only:
                if docalibrator:
                    if len(complist) <=0:
                        complist=fileroot+"/"+project+'.cal.cclist'
                    else:
                        # XXX will 2 cl work?
                        complist=complist+","+fileroot+"/"+project+'.cal.cclist'

                if len(complist) > 1:
                    message = "predicting from "+newmodel+" and "+complist
                    if verbose:
                        msg(" ",priority="info")
                        msg(message,priority="warn",origin="simobserve")
                    else:
                        msg(message,origin="simobserve")
                else:
                    message = "predicting from "+newmodel
                    if verbose:
                        msg(" ",priority="info")
                        msg(message,priority="warn",origin="simobserve")
                    else:
                        msg(message,origin="simobserve")
                sm.predict(imagename=newmodel,complist=complist)
            else:   # if we're doing only components
                # XXX will 2 cl work?
                if docalibrator:
                    complist=complist+","+fileroot+"/"+project+'.cal.cclist'
                if verbose:
                    msg("predicting from "+complist,priority="warn",origin="simobserve")
                else:
                    msg("predicting from "+complist,origin="simobserve")
                sm.predict(complist=complist)

            sm.done()
            msg('generation of measurement set '+msfile+' complete',origin="simobserve")


            ############################################
            # create figure 
            if grfile:
                file = fileroot + "/" + project + ".observe.png"
            else:
                file = ""

            # update psfsize using uv coverage instead of maxbase above
            if os.path.exists(msfile):
                # psfsize was set from the antenna posns before, but uv is better
                tb.open(msfile)
                rawdata = tb.getcol("UVW")
                tb.done()
                # TODO make this use the 90% baseline as in aU.getBaselineStats
                maxbase = max([max(rawdata[0,]),max(rawdata[1,])])  # in m
                if maxbase > 0.:
                    psfsize = 0.3/qa.convert(qa.quantity(model_center),'GHz')['value']/maxbase*3600.*180/pl.pi # lambda/b converted to arcsec
                    if not uvmode:
                        msg("An single observation is requested but CROSS-correlation in "+msfile,priority="error")
                else: #Single-dish (zero-spacing)
                    psfsize = pb
                    if uvmode:
                        msg("An interferometer observation is requested but only AUTO-correlation in "+msfile,priority="error")
                minimsize = 8* int(psfsize/cell_asec)
            else:
                msg("Couldn't find "+msfile,priority="error")

            if uvmode:
                multi = [2,2,1]
            else:
                multi = 0


            if (grscreen or grfile):
                util.newfig(multi=multi,show=grscreen)
                util.ephemeris(refdate,direction=util.direction,telescope=telescopename,ms=msfile,usehourangle=usehourangle)
                casalog.origin('simobserve')
                if uvmode:
                    util.nextfig()
                    util.plotants(stnx, stny, stnz, stnd, padnames)

                    # uv coverage
                    util.nextfig()
                    klam_m = 300/qa.convert(model_center,'GHz')['value']
                    pl.box()
                    pl.plot(rawdata[0,]/klam_m,rawdata[1,]/klam_m,'b,')
                    pl.plot(-rawdata[0,]/klam_m,-rawdata[1,]/klam_m,'b,')
                    ax = pl.gca()
                    ax.yaxis.LABELPAD = -4
                    pl.xlabel('u[klambda]',fontsize='x-small')
                    pl.ylabel('v[klambda]',fontsize='x-small')
                    pl.axis('equal')

                    # show dirty beam from observed uv coverage
                    util.nextfig()
                    im.open(msfile)
                    # TODO spectral parms
                    msg("using default model cell "+qa.tos(model_cell[0])+" for PSF calculation",priority="warn",origin='simobserve')
                    im.defineimage(cellx=qa.tos(model_cell[0]),nx=int(max([minimsize,128])))
                    if os.path.exists(fileroot+"/"+project+".quick.psf"):
                        shutil.rmtree(fileroot+"/"+project+".quick.psf")
                    im.approximatepsf(psf=fileroot+"/"+project+".quick.psf")
                    quickpsf_current = True
                    beam = im.fitpsf(psf=fileroot+"/"+project+".quick.psf")
                    im.done()
                    ia.open(fileroot+"/"+project+".quick.psf")
                    beamcs = ia.coordsys()
                    beam_array = ia.getchunk(axes=[beamcs.findcoordinate("spectral")['pixel'][0],beamcs.findcoordinate("stokes")['pixel'][0]],dropdeg=True)
                    nn = beam_array.shape
                    xextent = nn[0]*cell_asec*0.5
                    xextent = [xextent,-xextent]
                    yextent = nn[1]*cell_asec*0.5
                    yextent = [-yextent,yextent]
                    flipped_array = beam_array.transpose()
                    ttrans_array = flipped_array.tolist()
                    ttrans_array.reverse()
                    pl.imshow(ttrans_array,interpolation='bilinear',cmap=pl.cm.jet,extent=xextent+yextent,origin="bottom")
                    pl.title(project+".quick.psf",fontsize="x-small")
                    b = qa.convert(beam[1],'arcsec')['value']
                    pl.xlim([-3*b,3*b])
                    pl.ylim([-3*b,3*b])
                    ax = pl.gca()
                    pl.text(0.05,0.95,"bmaj=%7.1e\nbmin=%7.1e" % (beam[1]['value'],beam[2]['value']),transform = ax.transAxes,bbox=dict(facecolor='white', alpha=0.7),size="x-small",verticalalignment="top")
                    ia.done()
                util.endfig(show=grscreen,filename=file)

            elif thermalnoise != "" or leakage > 0:
                # Not predicting this time but corrupting. Get obsmode from ms.
                if not os.path.exists(msfile):
                    msg("Couldn't find "+msfile,priority="error")
                uvmode = (not util.ismstp(msfile,halt=False))


        ######################################################################
        # noisify

        noise_any = False
        msroot = fileroot + "/" + project  # if leakage, can just copy from this project

        if thermalnoise != "":
            knowntelescopes = ["ALMASD", "ALMA", "ACA", "SMA", "EVLA", "VLA"]

            noise_any = True

            noisymsroot = msroot + ".noisy"
            if not uvmode: #Single-dish
                msroot += ".sd"
                noisymsroot += ".sd"
 
            # Cosmic background radiation temperature in K.
            t_cmb = 2.725


            # check for interferometric ms:
            if not os.path.exists(msroot+".ms"):
                msg("Couldn't find "+msroot+".ms",priority="error")

            # MS exists
            message = 'copying '+msroot+'.ms to ' + \
                      noisymsroot+'.ms and adding thermal noise'
            if verbose:
                msg(" ",priority="info")
                msg(message,origin="noise",priority="warn")
            else:
                msg(message,origin="noise")

            if os.path.exists(noisymsroot+".ms"):
                shutil.rmtree(noisymsroot+".ms")
            shutil.copytree(msfile,noisymsroot+".ms")
            if sm.name() != '':
                msg("table persistence error on %s" % sm.name(),priority="error")
                return False

            # if not predicted this time, get telescopename from ms
            if not predict:
                tb.open(noisymsroot+".ms/OBSERVATION")
                n = tb.getcol("TELESCOPE_NAME")
                telescopename = n[0]
                # todo add check that entire column is the same
                tb.done()
                msg("telescopename read from "+noisymsroot+".ms: "+telescopename)

            if telescopename not in knowntelescopes:
                msg("thermal noise only works properly for ALMA/ACA, (J)VLA, and SMA",origin="simobserve",priority="warn")
            eta_p, eta_s, eta_b, eta_t, eta_q, t_rx = util.noisetemp(telescope=telescopename,freq=model_center)

            # antenna efficiency
            eta_a = eta_p * eta_s * eta_b * eta_t
            if verbose: 
                msg('antenna efficiency    = '+str(eta_a), origin="simobserve")
                msg('spillover efficiency  = '+str(eta_s), origin="simobserve")
                msg('correlator efficiency = '+str(eta_q), origin="simobserve")
            # sensitivity constant
            scoeff = -1  #Force setting the default value, 1./sqrt(2.0)
            if not uvmode: #Single-dish
                scoeff = 1.0
                if verbose: msg('sensitivity constant = '+str(scoeff), origin="simobserve")

            sm.openfromms(noisymsroot+".ms")    # an existing MS
            sm.setdata(fieldid=[]) # force to get all fields
            sm.setseed(seed)
            if thermalnoise == "tsys-manual":
                if verbose:
                    message = "sm.setnoise(spillefficiency="+str(eta_s)+\
                              ",correfficiency="+str(eta_q)+\
                              ",antefficiency="+str(eta_a)+\
                              ",trx="+str(t_rx)+",tau="+str(tau0)+\
                              ",tatmos="+str(t_sky)+",tground="+str(t_ground)+\
                              ",tcmb="+str(t_cmb)
                    if not uvmode: message += ",senscoeff="+str(scoeff)
                    message += ",mode='tsys-manual')"
                    msg(message);
                    msg("** this may be slow if your MS is finely sampled in time ** ",priority="warn")
                sm.setnoise(spillefficiency=eta_s,correfficiency=eta_q,
                            antefficiency=eta_a,trx=t_rx,
                            tau=tau0,tatmos=t_sky,tground=t_ground,tcmb=t_cmb,
                            mode="tsys-manual",senscoeff=scoeff)
            else:
                if verbose:
                    message = "sm.setnoise(spillefficiency="+str(eta_s)+\
                              ",correfficiency="+str(eta_q)+\
                              ",antefficiency="+str(eta_a)+\
                              ",trx="+str(t_rx)+",tground="+str(t_ground)+\
                              ",tcmb="+str(t_cmb)
                    if not uvmode: message += ",senscoeff="+str(scoeff)
                    message += ",mode='tsys-atm'"+\
                               ",pground='560mbar',altitude='5000m'"+\
                               ",waterheight='2km',relhum=20,pwv="+str(user_pwv)+"mm)"
                    msg(message);
                    msg("** this may be slow if your MS is finely sampled in time ** ",priority="warn")
                sm.setnoise(spillefficiency=eta_s,correfficiency=eta_q,
                            antefficiency=eta_a,trx=t_rx,
                            tground=t_ground,tcmb=t_cmb,pwv=str(user_pwv)+"mm",
                            mode="tsys-atm",table=noisymsroot,senscoeff=scoeff)
                # don't set table, that way it won't save to disk
                #                        mode="calculate",table=noisymsroot)

            sm.corrupt();
            sm.done();


            msroot = noisymsroot
            if verbose: msg("done corrupting with thermal noise",origin="noise")


        if leakage > 0:
            noise_any = True
            # TODO: need to handle SD name when leakage is available
            if msroot == fileroot+"/"+project:
                noisymsroot = fileroot + "/" + project + ".noisy"
            else:
                noisymsroot = fileroot + "/" + project + ".noisier"
            if not uvmode: #Single-dish
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
                    return False

                sm.openfromms(noisymsroot+".ms")    # an existing MS
                sm.setdata(fieldid=[]) # force to get all fields
                sm.setleakage(amplitude=leakage,table=noisymsroot+".cal")
                sm.corrupt();
                sm.done();



        # cleanup - delete newmodel, newmodel.flat etc
#        if os.path.exists(modelflat):
#            shutil.rmtree(modelflat)
        if os.path.exists(modelflat+".regrid"):
            shutil.rmtree(modelflat+".regrid")
        if os.path.exists(fileroot+"/"+project+".noisy.T.cal"):
            shutil.rmtree(fileroot+"/"+project+".noisy.T.cal")
        if os.path.exists(fileroot+"/"+project+".noisy.sd.T.cal"):
            shutil.rmtree(fileroot+"/"+project+".noisy.sd.T.cal")

    except TypeError, e:
        finalize_tools()
        #msg("simobserve -- TypeError: %s" % e, priority="error")
        casalog.post("simobserve -- TypeError: %s" % e, priority="ERROR")
        raise TypeError, e
        return False
    except ValueError, e:
        finalize_tools()
        #print "task_simobserve -- OptionError: ", e
        #msg("simobserve -- OptionError: %s" % e, priority="error")
        casalog.post("simobserve -- OptionError: %s" % e, priority="ERROR")
        raise ValueError, e
        return False
    except Exception, instance:
        finalize_tools()
        #print '***Error***',instance
        #msg("simobserve -- Exception: %s" % instance, priority="error")
        casalog.post("simobserve -- Exception: %s" % instance, priority="ERROR")
        raise Exception, instance
        return False
    return True

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

def finalize_tools():
    if ia.isopen(): ia.close()
    sm.close()
    #cl.close()   # crashes casa
