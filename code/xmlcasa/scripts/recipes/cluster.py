from IPython.kernel import client
PARALLEL_CASAPY_ROOT_DIR = "/home/casa-dev-02/casa_ccore/linux_64b/";
PARALLEL_CONFIGFILES_DIR = "/users/sbhatnag/Scripts/ClusterConfigFiles/";
PARALLEL_WORK_DIR        = "/sanjay/PTEST/";
PARALLEL_BASE_ROOT_DIR   = "/home/casa-dev-";
#
#===============================================================
# Make up a name containting the node number info.
# (This function has the policy used to identify pieces of the
#  distributed database)
#
def mkname(node,rootdir,workdir,basename,ext,regex=false):
    format=str('%2.2d');
    nodenumber=(format%node);
    if regex:
        imgnodenumber="*";
    else:
        imgnodenumber=nodenumber;
    targetname=rootdir+nodenumber+workdir+basename+imgnodenumber+ext;
    return targetname;
#
#===============================================================
# Return a string as <var>+<op>+"<value>".  Often required
# functionality when set up remote variables.
#
def rcmd(var,op,value):
    tt=var+"=\""+value+"\"";
    tt=var+op+"\""+value+"\"";
    return tt;
#
#===============================================================
# Set up and return the MultiEngineClient.  This is required
# when doing any communication of the nodes.
#
def pinit(message="Hello CASA Cluster"):
    casalog.post("Setting up the connection to the remote nodes...",origin="PDeconv::pinit");
    mec = client.MultiEngineClient();
    ids=mec.get_ids();
    print "Connected to IDs ",ids;
#        tt='print '+"'"+message+"'";
    mec.activate();
#	mec.execute(tt);
    return mec;
#
#===============================================================
# Start remote, non-interactive casapy
#
#def setpath(rmec,root='/home/casa-dev-01/hye/gnuactive/linux_64b'):
def startcasapy(rmec,root=PARALLEL_CASAPY_ROOT_DIR):
    casalog.post("Starting remote casapys...",origin="PDeconv::startcasapy");
    rmec.execute("import sys");
    rmec.execute(rcmd("root","=",root));
    cmd="sys.path.append("+rcmd("root","+","/python/2.5/")+")";
    rmec.execute(cmd);
    cmd = "execfile("+rcmd("root","+","/python/2.5/casa_in_py.py")+")";
    rmec.execute(cmd);
    cmd = "import os,shutil";
    rmec.execute(cmd);
#
#===============================================================
# Make a continuum image from the images on various nodes.
# (This is a work around a bug in image analysis tool (CAS-1229))
#
def mkContResImgWorkaround(rmec,nodes,iatool,
                           rootdir=PARALLEL_BASE_ROOT_DIR,
                           workdir=PARALLEL_WORK_DIR,
                           imagebasename="ptest.cont.im.",
                           extension=".residual",
                           outimagename="dirtyavg.im"):
    n=len(nodes);
    imagenames=[];
    refval=[];
    for i in range(n):
        imagename=mkname(nodes[i],rootdir,workdir,imagebasename,extension);
        iatool.open(imagename);
        shp=iatool.shape();
        tt="tempimage."+str(nodes[i])+".cont"
        iatool.rebin(outfile=tt,bin=[1,1,1,shp[3]],overwrite=true);
        iatool.open(tt);
        cs=iatool.coordsys();
        refval.append(ia.coordsys().referencevalue(type='spectral')['numeric'][0]);
        iatool.close();
#    print refval;
    refval0=sum(refval)/len(refval);
    width=100*refval[len(refval)-1]-refval[0];
    tmpimagename=outimagename+".sum";
    cmd = "rm -rf "+tmpimagename;
    os.system(cmd);
    cmd = "cp -r tempimage."+str(nodes[0])+".cont "+outimagename;
    os.system(cmd);
    iatool.open(outimagename);
    d0=iatool.getchunk();
    d0=d0*0;
#    iatool.open(outimagename);
    cs=iatool.coordsys();
    cs.setincrement(value=width,type='spectral');
    cs.setreferencepixel(value=0,type='spectral');
    cs.setreferencevalue(value=refval0,type='spectral');
    iatool.setcoordsys(cs.torecord());
    iatool.close()
    
#    print "Averaging...";
    for i in range(n):
        tt="tempimage."+str(nodes[i])+".cont";
        iatool.open(tt);
        dd=ia.getchunk();
        d0=dd+d0;
        iatool.close();
        imagenames.append(tt);
    iatool.open(outimagename);
    iatool.putchunk(d0/n);
    iatool.close();
#
#===============================================================
# Gather operation
# (keep this serial for now - but make this parallel (btree averager))
# (This is not in use yet because of an bug in ia tool related to
#  Tabulated Axis in the image)
#
def mkContResImgNotInUse(rmec,nodes,iatool,
                         rootdir=PARALLEL_BASE_ROOT_DIR,
                         workdir=PARALLEL_WORK_DIR,
                         imagebasename="ptest.cont.im.",
                         extension=".residual",
                         outimagename="dirtyavg.im"):
    n=len(nodes);
    imagenames=[];
    for i in range(n):
        imagename=mkname(nodes[i],rootdir,workdir,imagebasename,extension);
        imagenames.append(imagename);
    print imagenames;
    tmpimagename=outimagename+".sum";
    iatool.imageconcat(outfile=tmpimagename,infiles=imagenames,overwrite=true,relax=true);
    iatool.open(tmpimagename);
    shp=iatool.shape();
    print "Averaging ",shp[3]," channels of '",tmpimagename,"'";
    iatool.rebin(outfile=outimagename,bin=[1,1,1,shp[3]],overwrite=true);
    iatool.close();
#
#===============================================================
#
def mksyscmdstr(cmd):
    return "os.system(\""+cmd+"\")";
#
#===============================================================
# Scatter operation
# (keep this serial for now - but make this non-block/parallel)
#
def cpmodelimg(rmec,nodes,modelimage,
               rootdir=PARALLEL_BASE_ROOT_DIR,
               workdir=PARALLEL_WORK_DIR,
               imagebasename="ptest.model."):
    n=len(nodes);
    for i in range(n):
        toimagename=mkname(nodes[i],rootdir,workdir,imagebasename,".image");
        cmd = "shutil.copytree('"+modelimage+"','"+toimagename+"')";
        print cmd;
#        mec.execute(cmd,i);
#
#===============================================================
#
def rmimage(mec,imagename,doit=false):
    cmd = "rm -rf "+imagename;
    cmd = "os.rmdir("+imagename+")";
#    cmd = mksyscmdstr(cmd);
#
#===============================================================
# Make only residual images using distributed database.
# This is used as a the parallel-major cycle of a parallel
# deconvolution run.
#
def mkResOnly(rmec,nodes,
              configfile=PARALLEL_CONFIGFILES_DIR+"clean_mfs_major.last",
              rootdir=PARALLEL_BASE_ROOT_DIR,
              workdir=PARALLEL_WORK_DIR,
              msbasename="pevla2_1h.16.ms-",
              imagebasename="ptest.cont.im."):
    # Just set up imager parameter to stop making PSFs
    # Fireup imager to do "makeimage("residual")
    n=len(nodes);
#     for i in range(n):
#         imgname=mkname(nodes[i],rootdir,workdir,imagebasename,".residual");
#         rmimage(imgname);
    setUpClean(rmec,1,0,nodes,configfile,rootdir,workdir,msbasename,imagebasename)
#
#===============================================================
#
def setModelImgCoordsys(modelimage, sourceimage,iatool=ia):
    iatool.open(sourceimage);
    cs=iatool.coordsys();
    iatool.close();
    iatool.open(modelimage);
    iatool.setcoordsys(cs.torecord());
    iatool.close();
#
#===============================================================
# Accumulate model image from each minor cycle iteration
#
def accumulateModel(iModel, modelimage,iatool=ia):
    #
    # Accumulate the model image by hand too!
    #
    if (os.path.exists(modelimage)):
        iatool.open(iModel);      d0=iatool.getchunk();
        iatool.open(modelimage);  d1=iatool.getchunk();
        d1 = d1 + d0;
        iatool.putchunk(d1);
        iatool.close();
    else:
        cmd = "cp -r " + iModel + " " + modelimage;
        os.system(cmd);
    #
    # Remove the incremental model
    #
    cmd = "rm -rf " + iModel;
    os.system(cmd);
#
#===============================================================
#
def setUpFlagger(rmec,nodes,mode="quack",
                 configfile=PARALLEL_CONFIGFILES_DIR+"pquack.last",
                 rootdir=PARALLEL_BASE_ROOT_DIR,
                 workdir=PARALLEL_WORK_DIR,
                 msbasename="pevla2_1h.ms-"):
    #
    # Load the defaults from a user defined configfile
    #
    rmec.execute(rcmd("pquackconfig","=", configfile));
    cmd="execfile(pquackconfig)";
    rmec.execute(cmd);
    #
    # Override the parameters if required
    #
    if (mode != ""):
        cmd="mode="+"'"+mode+"'";
        rmec.execute(cmd);
    for i in range(len(nodes)):
        msname=mkname(nodes[i],rootdir,workdir,msbasename,"");
        mec.execute(rcmd("vis","=",msname),i);
#
#===============================================================
#
def setUpGaincal(rmec,nodes, caltable='s0_0.gcal',
                 field='0', spw='0', solint='10s',
                 configfile=PARALLEL_CONFIGFILES_DIR+"pgaincal.last",
                 rootdir=PARALLEL_BASE_ROOT_DIR,
                 workdir=PARALLEL_WORK_DIR,
                 msbasename="pevla2_1h.ms-"):
    #
    # Load the defaults from a user defined configfile
    #
    rmec.execute(rcmd("pgaincalconfig","=",configfile));
    cmd="execfile(pgaincalconfig)";
    rmec.execute(cmd);
    #
    # Override the parameters if required
    #
    if (caltable != ""):
        rmec.execute(rcmd("caltable", "=", caltable));
    if (field != ""):
        rmec.execute(rcmd("field",    "=", field));
    if (spw != ""):
        rmec.execute(rcmd("spw",      "=", spw));
    if (solint != ""):
        rmec.execute(rcmd("solint",   "=", solint));
    #
    # This is the "distributed" parameter (i.e., different at each node)
    #
    for i in range(len(nodes)):
        msname=mkname(nodes[i],rootdir,workdir,msbasename,"");
        mec.execute(rcmd("vis","=",msname),i);
#
#===============================================================
#
def setUpBandpass(rmec,nodes,caltable='BJones.tab', field='0',
                  spw='0', solint='inf', gaintable='',
                  interp=['nearest'],
                  configfile=PARALLEL_CONFIGFILES_DIR+"pbandpass.last",
                  rootdir=PARALLEL_BASE_ROOT_DIR,
                  workdir=PARALLEL_WORK_DIR,
                  msbasename="pevla2_1h.ms-"):
    #
    # Load the defaults from a user defined configfile
    #
    rmec.execute(rcmd("pbandpassconfig","=",configfile));
    cmd="execfile(pbandpassconfig)";
    rmec.execute(cmd);
    #
    # Override the parameters if required
    #
    if (caltable != ""):
        rmec.execute(rcmd("caltable", "=", caltable));
    if (field != ""):
        rmec.execute(rcmd("field",    "=", field));
    if (spw != ""):
        rmec.execute(rcmd("spw",      "=", spw));
    if (solint != ""):
        rmec.execute(rcmd("solint",   "=", solint));
    if ( gaintable != ""):
        rmec.execute(rcmd("gaintable","=", gaintable));
    #
    # This is the "distributed" parameter (i.e., different at each node)
    #
    for i in range(len(nodes)):
        msname=mkname(nodes[i],rootdir,workdir,msbasename,"");
        mec.execute(rcmd("vis","=",msname),i);
#
#===============================================================
#
def setUpApplycal(rmec,field="", spw="", gaintable=[''],
                  interp=[''],
                  rootdir=PARALLEL_BASE_ROOT_DIR,
                  workdir=PARALLEL_WORK_DIR,
                  msbasename="pevla2_1h.ms-",
                  imagebasename="ptest.im."):
    #
    # Load the defaults from a user defined configfile
    #
    rmec.execute(rcmd("papplycalconfig","=",configfile));
    cmd="execfile(papplycalconfig)";
    rmec.execute(cmd);
    #
    # Override the parameters if required
    #
    if (field != ""):
        rmec.execute(rcmd("field", "=", field));
    if (spw != ""):
        rmec.execute(rcmd("spw", "=", spw));
    #
    # set gaintable - this is a list of strings
    # set interp    - this is a list of strings
    #
    #
    # This is the "distributed" parameter (i.e., different at each node)
    #
    for i in range(len(nodes)):
        msname=mkname(nodes[i],rootdir,workdir,msbasename,"");
        mec.execute(rcmd("vis","=",msname),i);
#
#===============================================================
#
def setUpClean(rmec,nchan,niter=1000, nodes=[1,2,3,4],
               configfile=PARALLEL_CONFIGFILES_DIR+"clean.last",
               rootdir=PARALLEL_BASE_ROOT_DIR,
               workdir=PARALLEL_WORK_DIR,
               msbasename="pevla2_1h.ms-",
               imagebasename="ptest.im."):
    #
    # Load the defaults from a user defined configfile
    #
    rmec.execute(rcmd("pcleanconfig","=",configfile));
    cmd="execfile(pcleanconfig)";
    rmec.execute(cmd);
    #
    # Override the parameters if required
    #
    if (nchan > 0):
        cmd="nchan="+str(nchan);
        rmec.execute(cmd);
    if (niter > -1):
        cmd="niter="+str(niter);
        rmec.execute(cmd);
    for i in range(len(nodes)):
        msname=mkname(nodes[i],rootdir,workdir,msbasename,"");
        imgname=mkname(nodes[i],rootdir,workdir,imagebasename,"");
#         print "Setting vis=",msname;
        mec.execute(rcmd("vis","=",msname),i);
        mec.execute(rcmd("imagename","=",imgname),i);
#
#===============================================================
# Do a spectral line deconvolution.
def pspectralline(rmec,nodes,nchan,niter=1000,
                  msbasename="pevla2_1h.16.ms-",
                  imagebasename="ptest.cont.im.",
                  configfile=PARALLEL_CONFIGFILES_DIR+"clean.last",
                  rootdir=PARALLEL_BASE_ROOT_DIR,
                  workdir=PARALLEL_WORK_DIR):
    setUpClean(rmec,nchan,niter,nodes,configfile,
               rootdir,workdir,msbasename,
               imagebasename);
    cmd="go('clean')"
    mec.execute(cmd);
#
#===============================================================
# Do a continuum deconvolution.
def pcontinuum(mec,iatool,deconvtool,nodes,
               minoriter=1000,minorpermajor=200,majoriter=5,
               majorconfigfile=PARALLEL_CONFIGFILES_DIR+"clean_mfs_major.last",
               minorconfigfile="",
               msbasename="pevla2_1h.16.ms-",
               imagebasename="ptest.cont.im.",
               rootdir=PARALLEL_BASE_ROOT_DIR,
               workdir=PARALLEL_WORK_DIR):
    n = len(nodes);
    dirtyimagename     = "dirtyavg.im.sum";
    avgpsfname         = "psfavg.im.sum";
    modelimagename     = "avgcomp.im";
    tmpModelImageName  = "tmp.avgcomp.im";
    minorIterRemaining = minoriter;
    minorIterPerMajor  = minorpermajor;
    casalog.origin("PDeconv::pcontinuum");
    for major in range(majoriter):
        #
        #--------------------Parallel Major Cycle------------------
        #
        casalog.post("====================================================================");
        mesg="Making residual images for major cycle no. " + str(major) + " (the parallel operation)";
        casalog.post(mesg);
        mkResOnly(mec,nodes,configfile=majorconfigfile,rootdir=rootdir,workdir=workdir,
                  msbasename=msbasename,imagebasename=imagebasename);
        cmd="go('clean')"
        mec.execute(cmd);
        #
        #--------------------Gather residuals----------------------
        #
        casalog.post("Making continuum residual image (the gather operation)");
        casalog.post("   Making average residual image...");
        mkContResImgWorkaround(mec,nodes,iatool,imagebasename=imagebasename,
                               outimagename=dirtyimagename);
        #
        # The PSF needs to be computed only once...being lazy, for now I am
        # letting it compute in every major cycle.
        #
        casalog.post("   Making average PSF image...");
        mkContResImgWorkaround(mec,nodes,iatool,imagebasename=imagebasename,
                               extension=".psf",outimagename=avgpsfname);
        #
        #--------------------Serial minor cycle--------------------
        #
        casalog.post("Doing the minor cycle (the serial operation)");
        if (minorIterRemaining > minorIterPerMajor):
            minorIterRemaining = minorIterPerMajor;
        if (minorIterRemaining > 0):
            casalog.post("Doing "+str(minorIterRemaining)+ " minor cycle iterations...");
            deconvtool.open(dirtyimagename, avgpsfname);
            deconvtool.clarkclean(niter=minorIterRemaining,model=tmpModelImageName);
            accumulateModel(tmpModelImageName,modelimagename,iatool);
        minorIterRemaining = minoriter - (major+1)*minorIterPerMajor;
        #
        #--------------------Scatter/update model------------------
        #
        casalog.post("Scattering the model image");
        for i in range(n):
            remotemodelimage=mkname(nodes[i],rootdir,workdir,imagebasename,".model");
            cmd = "rm -rf " + remotemodelimage;
            os.system(cmd);
            cmd = "cp -r " + modelimagename + " " + remotemodelimage;
            os.system(cmd);
            remoteresidualimage=mkname(nodes[i],rootdir,workdir,imagebasename,".residual");
            setModelImgCoordsys(remotemodelimage, remoteresidualimage);
#         casalog.post("Synchornizing by sleeping for 10s....zzz.z.z...zzz....");
#         os.system("sleep 10");
#
#===============================================================
# A routine used for debugging continuum deconvolution.
# The "dowhat" variable allows doing each step of the major-minor
# cycle iteration at a time.
#
def pcontinuumdebugger(mec,iatool,deconvtool,nodes,
                       minorconfigfile,
                       majorconfigfile,
                       dowhat,
                       rootdir=PARALLEL_BASE_ROOT_DIR,
                       workdir=PARALLEL_WORK_DIR,
                       msbasename="pevla2_1h.16.ms-",
                       imagebasename="ptest.cont.im.",
                       minoriter=100,majoriter=10):
    n = len(nodes);
    dirtyimagename="dirtyavg.im.sum";
    avgpsfname = "psfavg.im.sum";
    modelimagename="avgcomp.im";
    tmpModelImageName="tmp.avgcomp.im";
    minorIterRemaining=minoriter;
    minorIterPerMajor = 100;
    
    major=0;
    if (dowhat == 1):
        print "Making residual images for major cycle no. ", major, "(the scatter operation)";
        mkResOnly(mec,nodes,configfile=majorconfigfile,rootdir=rootdir,workdir=workdir,
                  msbasename=msbasename,imagebasename=imagebasename);
        cmd="go('clean')"
        mec.execute(cmd);

    if (dowhat == 2):
        print "Making continuum dirt image (the gather operation)"
        print "   Making average dirty image...";
        mkContResImgWorkaround(mec,nodes,iatool,imagebasename=imagebasename,
                               outimagename=dirtyimagename);
        print "   Making average PSF image...";
        mkContResImgWorkaround(mec,nodes,iatool,imagebasename=imagebasename,
                               extension=".psf",outimagename=avgpsfname);

    if (dowhat == 3):
        print minorIterRemaining, minorIterPerMajor;
        print "Doing the minor cycle (the serial operation)"
        if (minorIterRemaining > minorIterPerMajor):
            minorIterRemaining = minorIterPerMajor;
        if (minorIterRemaining > 0):
            print "Doing ", minorIterRemaining, " minor cycle iterations...";
            print dirtyimagename, avgpsfname;
            deconvtool.open(dirtyimagename, avgpsfname);
            deconvtool.clarkclean(niter=minorIterRemaining,model=tmpModelImageName);
            deconvtool.done();
            accumulateModel(tmpModelImageName, modelimagename,iatool);
        minorIterRemaining = minoriter - (major+1)*minorIterPerMajor;

    if (dowhat == 4):
        print "Scattering the model image"
        for i in range(n):
            remotemodelimage=mkname(nodes[i],rootdir,workdir,imagebasename,".model");
            cmd = "rm -rf " + remotemodelimage;
            os.system(cmd);
            cmd = "cp -r " + modelimagename + " " + remotemodelimage;
            os.system(cmd);
            remoteresidualimage=mkname(nodes[i],rootdir,workdir,imagebasename,".residual");
            setModelImgCoordsys(remotemodelimage, remoteresidualimage);
#         print "Synchornizing by sleeping for 10s....zzz.z.z...zzz....";
#         os.system("sleep 10");
