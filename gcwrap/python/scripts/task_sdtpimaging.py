# sd task for total power raster scan imaging
import time
import shutil
import numpy
import pylab as pl

from taskinit import casalog, gentools

import asap as sd
import sdutil

interactive=False

# This is a task version of the script originally made for reducing ATF raster scan data 
# in total power mode. Still experimental...
# 
@sdutil.sdtask_decorator
def sdtpimaging(infile, calmode, masklist, blpoly, backup, flaglist, antenna, spw, stokes, createimage, outfile, imsize, cell, phasecenter, ephemsrcname, pointingcolumn, gridfunction, plotlevel):
    # NEED to include spw, src? name for movingsource param. in function argument
    with sdutil.sdtask_manager(sdtpimaging_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()


class sdtpimaging_worker(sdutil.sdtask_template_imaging):
    # list of ephemeris objects that CASA supports
    ephemobjs = set(['MERCURY','VENUS','MARS','JUPITER','SATURN','URANUS','NEPTUNE','PLUTO','SUN','MOON'])

    # define stokestypes (only up to 'YL') based on enumerated stokes type in CASA
    stokestypes=['undef','I','Q','U','V','RR','RL','LR','LL','XX','XY','YX','YY','RX','RY','LX','LY','XR','XL','YR','YL']
    def __init__(self, **kwargs):
        super(sdtpimaging_worker,self).__init__(**kwargs)

    def parameter_check(self):
        # use FLOAT_DATA if exists
        self.open_table(self.infile)
        colnames = self.table.colnames()
        if any(col=='FLOAT_DATA' for col in colnames):
            self.datacol = 'FLOAT_DATA'
            casalog.post( "FLOAT_DATA exist" )
        else:
            self.datacol = 'DATA'
            casalog.post( "No FLOAT_DATA, DATA is used" )
        self.close_table()

        # masklist must be specified for baseline subtraction
        self.dobaseline = self.calmode.lower() == 'baseline'
        if self.dobaseline and (self.masklist is None \
                                or len(self.masklist) == 0):
            msg='Please specify a region to be fitted in masklist'
            raise Exception, msg

        # outfile must be specified for imaging
        if self.createimage and self.outfile == '':
            msg='Please specify out image name'
            raise Exception, msg

        # flag operation
        self.doflag = len(self.flaglist) > 0

        # plot
        self.doplot = abs(self.plotlevel) > 0
        
    def compile(self):
        # common parameters
        self.__compile_common()
        
        # for baseline calibration
        if self.dobaseline:
            self.__compile_for_baseline()

        # for imaging
        if self.createimage:
            self.__compile_for_imaging()

    def execute(self):        
        if self.dobaseline:
            self.__backup()
            self.__execute_baseline()
        elif self.doplot:
            self.__plot(subplot=111, title=True, ylabel=self.datacol)

        if self.doflag:
            self.__execute_flag()

        if self.createimage:
            self.__execute_imaging()

    def __backup(self):
        # back up original file if backup == True
        casalog.post('%s column in %s will be overwritten.'%(self.datacol,self.infile))
        if self.backup:
            backupname = self.infile.rstrip('/')+'.sdtpimaging.bak.'+time.asctime(time.gmtime()).replace(' ','_')
            casalog.post('The task will create a back up named %s'%backupname)
            self.open_table(self.infile)
            backuptb = self.table.copy( backupname, deep=True, valuecopy=True )
            if backuptb:
                backuptb.close()
            self.close_table()

    def __execute_baseline(self):
        #print "calibration begins..."
        #select the totalpower data for antenna1,2=1 (DV01, vertex antenna)
        # use generic pylab
        if type(self.antid) == list and len(self.antid) > 2:
            #print "Assume the antenna selection is all..."
            casalog.post( "Assume the antenna selection is all..." )
        self.open_table(self.infile)
        ndat = 0
        pl.ioff()
        # determine the size of processing data
        subtb = self.table.query('any(ANTENNA1==%s && ANTENNA2==%s)' % (self.antid,self.antid))
        nr = subtb.nrows()
        #nr = len(data)*len(data[0])
        #ndatcol = numpy.zeros((npol,nr),dtype=numpy.float)
        ndatcol = subtb.getcol(self.datacol)
        (l,m,n) = ndatcol.shape
        # m (= nchan) should be 1 since the task is specifically designed for total power data
        ndatcol.reshape(l,n) 
        for np in range(len(self.selcorrtypeind)):
            pl.figure(np+1)
            pl.clf()
            pl.subplot(311)
            #pl.title(infile+' Ant:'+str(antlist.values()))
            pl.ylabel(self.datacol,fontsize='smaller')
            symbols=['b.','c.']

            #print "Arranging data by scans..."
            casalog.post( "Arranging data by scans..." )

            data = []
            flag = []
            ndat0 = 0
            ndat = 0
            if self.selnpol == 1: np = self.selpol
            pl.title(self.infile.rstrip('/')+' Ant:'+self.antnameused+' '+self.corrtypestr[np])
            casalog.post( "select %s data..." % self.corrtypestr[np] )
            for i in xrange(self.nscan):
                idata = []
                iflag = []
                #for j in xrange(nsubscan):
                for j in xrange(self.nsubscan[i]):
                    # may be need to iterate on each antenna 
                    # identify 'scan' by STATE ID
                    #selsubtb=tb.query('any(ANTENNA1==%s && ANTENNA2==%s) && SCAN_NUMBER==%s && STATE_ID==%s' % (antid,antid,scans[i],subscans[j]))
                    selsubtb = self.table.query('any(ANTENNA1==%s && ANTENNA2==%s) && SCAN_NUMBER==%s && STATE_ID==%s' % (self.antid,self.antid,self.scans[i],self.subscans[i][j]))
                    datcol = selsubtb.getcol(self.datacol)
                    flgcol = selsubtb.getcol("FLAG") # only channel flag is needed to be hanled in MS
                    selsubtb.close()
                    if self.npol > 1 and self.selnpol == 1:
                        #casalog.post( "select %s data..." % corrtypestr[selpol] )
                        rdatcol = datcol[self.selpol].real
                        flagcell = flgcol[self.selpol]
                    else:
                        rdatcol = datcol[self.selcorrtypeind[np]].real
                        flagcell = flgcol[self.selpol]
                    (m,n) = rdatcol.shape
                    if flagcell.shape != (m,n):
                        raise Exception, "Data conformation error. Shape of FLAG and %s differs." % self.datacol
                    rdatcol = rdatcol.reshape(n)
                    flagcell = flagcell.reshape(n)
                    #data.append(rdatcol)
                    idata.append(rdatcol)
                    iflag.append(flagcell)
                    ndat0 = ndat
                    ndat += len(rdatcol)
                data.append(idata)
                flag.append(iflag)
            if abs(self.plotlevel) > 0:
                pl.xlim(0,ndat)
                t1 = pl.getp(pl.gca(), 'xticklabels')
                t2 = pl.getp(pl.gca(), 'yticklabels')
                pl.setp((t1,t2), fontsize='smaller')
                mdat = 0
                for i in xrange(self.nscan):
                    #for j in xrange(nsubscan):
                    for j in xrange(self.nsubscan[i]):
                        mdat0 = mdat
                        mdat += len(data[i][j])
                        masked_data = numpy.ma.masked_array(data[i][j], flag[i][j])
                        #pl.plot(xrange(mdat0,mdat),data[i][j],symbols[subscans[j]%2])
                        #pl.plot(xrange(mdat0,mdat), data[i][j], symbols[self.subscans[i][j]%2])
                        pl.plot(xrange(mdat0,mdat), masked_data, symbols[self.subscans[i][j]%2])
                        ax = pl.gca()
                        if i == 1: 
                            leg = ax.legend(('even subscan no.', 'odd subscan no.'), numpoints=1, handletextsep=0.01) 
                            for t in leg.get_texts():
                                t.set_fontsize('small')
                pl.ion()
                pl.plot()
                #pl.draw()
            ### Calibration ############################################
            # Do a zero-th order calibration by subtracting a baseline #
            # from each scan to take out atmospheric effect.          #
            # The baseline fitting range specified by masklist must be #
            # given and is the same for all the scans.                 # 
            ############################################################
            f = sd.fitter()
            f.set_function(lpoly = self.blpoly)
            #print "Processing %s %s scans" % (corrtypestr[np], nscan)
            casalog.post( "Processing %s %s scans" % (self.corrtypestr[np], self.nscan) )
            cdat = numpy.array([])
            pl.subplot(312)
            pl.ioff()
            for i in xrange(self.nscan):
                casalog.post( "Processing Scan#=%s" % i )
                #for j in xrange(nsubscan):
                for j in xrange(self.nsubscan[i]):
                    masks = numpy.zeros(len(data[i][j]), dtype=numpy.int)
                    if self.left_mask >= len(data[i][j]) \
                           or self.right_mask >= len(data[i][j]):
                        msg = "Too large mask. All data will be used for baseline subtraction.\n The fitting may not be correct since it might confuse signal component as a background..."
                        casalog.post(msg, "WARN")
                    else:
                        msg = "Subtracting baselines, set masks for fitting at row ranges: [0,%s] and [%s,%s] " % (self.left_mask-1, len(masks)-self.right_mask, len(masks)-1)
                        casalog.post(msg, "INFO")
                    #Use edge channels for fitting excluding flagged integrations.
                    #masks[:self.left_mask] = True
                    #masks[-self.right_mask:] = True
                    masks[:self.left_mask] = (flag[i][j][:self.left_mask]==False)
                    masks[-self.right_mask:] = (flag[i][j][-self.right_mask:]==False)
                    x = xrange(len(data[i][j]))
                    if self.plotlevel > 1:
                        pl.cla()
                        pl.ioff()
                        pl.title('scan %s subscan %s'%(self.scans[i],self.subscans[i][j]))
                        #pl.plot(x, data[i][j], 'b')
                        pl.plot(x, numpy.ma.masked_array(data[i][j], flag[i][j]), 'b')
                    f.set_data(x, data[i][j], mask=masks)
                    f.fit()
                    #f.plot(residual=True, components=[1], plotparms=True)
                    fitd=f.get_fit()
                    data[i][j] = data[i][j]-fitd
                    if self.plotlevel > 1:
                        pl.ion()
                        pl.plot(xrange(len(fitd)), fitd, 'r-')
                        pl.draw()
                        pl.cla()
                        pl.ioff()
                        pl.title('scan %s subscan %s'%(self.scans[i], self.subscans[i][j]))
                        pl.ion()
                        #pl.plot(x, data[i][j], 'g')
                        pl.plot(x, numpy.ma.masked_array(data[i][j], flag[i][j]), 'g')
                        pl.draw()
                        if interactive: check=raw_input('Hit return for Next\n')
                    cdat = numpy.concatenate([cdat,data[i][j]])
            ndatcol[np] = cdat
            del data
        subtb.close()
        self.close_table()
        self.open_table(self.infile, nomodify=False)
        # put the corrected data to CORRECTED_DATA column in MS
        # assuming the data for the vertex are stored in every other
        # row starting from 2nd row for the vertex antenna....
        # For the alcatel startrow=0, for len(antid) >1,
        # it assumes all the data to be corrected. 
        if type(self.antid) == int:
            startrow = self.antid
            rowincr = self.nant
        else:
            startrow = 0
            rowincr = 1
        casalog.post( "Storing the corrected data to %s column in the MS..."%self.datacol )
        casalog.post( "Note that %s will be overwritten"%self.datacol )
        cdatm = ndatcol.reshape(self.npol,1,len(cdat))
        self.table.putcol(self.datacol, cdatm, startrow=startrow, rowincr=rowincr)
        self.close_table()
        # calibration end
        #
        # plot corrected data
        if abs(self.plotlevel) > 0:
            casalog.post( "plotting corrected data..." )
            self.__plot(subplot=313, title=False, ylabel='CORRECTED_DATA')

    def __execute_flag(self):
        #flag scans
        self.open_table(self.infile)
        subtb=self.table.query('any(ANTENNA1==%s && ANTENNA2==%s)' % (self.antid,self.antid))
        fdatcol = subtb.getcol('FLAG')
        (l,m,n) = fdatcol.shape
        # assume m = nchan = 1
        fdatcol.reshape(l,n)
        casalog.post( "Flag processing..." )
        flagscanlist=[]
        if type(self.antid) == int:
            startrow = self.antid
            rowincr = self.nant
        else:
            startrow = 0
            rowincr = 1
        subtb.close()
        self.close_table()
        self.open_table(self.infile, nomodify=False)
        for flag in self.flaglist:
            if type(flag) == list:
                flagscanlist.extend(range(flag[0],(flag[1]+1)))
            if type(flag) == int:
                flagscanlist.append(flag)
        flagscanset = set(flagscanlist) 
        for np in range(len(self.selcorrtypeind)):
            fdata=[]
            fdatac = numpy.array([])
            for i in xrange(self.nscan):
                #fdata=[]
                #for j in xrange(nsubscan):
                for j in xrange(self.nsubscan[i]):
                    #subtb=tb.query('any(ANTENNA1==%s && ANTENNA2==%s) && SCAN_NUMBER==%s && STATE_ID==%s' % (antid,antid,scans[i],subscans[j]))
                    subtb=self.table.query('any(ANTENNA1==%s && ANTENNA2==%s) && SCAN_NUMBER==%s && STATE_ID==%s' % (self.antid,self.antid,self.scans[i],self.subscans[i][j]))
                    fcolall = subtb.getcol('FLAG')
                    subtb.close()
                    if self.npol > 1 and self.selnpol == 1:
                        fcol=fcolall[self.selpol]
                    else:
                        fcol=fcolall[self.selcorrtypeind[np]]
                    (m,n) = fcol.shape
                    fcoln = fcol.reshape(n)
                    if i in flagscanset:
                        flgs = numpy.ones(n,dtype=numpy.bool)
                        fcoln = flgs 
                    #fdata.append(fcoln)
                    fdatac = numpy.concatenate([fdatac,fcoln])
            fdatcol[np] = fdatac
            #flagc=tb.getcol('FLAG')
        #subtb.close()
        fdatacm = fdatcol.reshape(self.npol,1,len(fdatac))
        self.table.putcol('FLAG', fdatacm, startrow=startrow, rowincr=rowincr)
        #print "Scans flagged: %s" % list(flagscanset)
        casalog.post( "Scans flagged: %s" % list(flagscanset) )
        self.close_table()

    def __execute_imaging(self):
        #if pointingcolumn.upper()=="OFFSET":
        #   pointingcolumn="POINTING_OFFSET"
        #print "pointingcolumn=",pointingcolumn
        #print "Imaging...."
        casalog.post( "pointingcolumn used: %s" % self.pointingcolumn )
        casalog.post( "Imaging...." )
        self.open_imager(self.infile)
        self.imager.selectvis(field=0, spw=self.spw, baseline=self.antsel)
        #self.imager.selectvis(vis=infile, field=0, spw=spw, baseline=antsel)
        #self.imager.defineimage(nx=nx, ny=ny, cellx=cellx, celly=celly,  phasecenter=phasecenter, spw=0, stokes=stokes, movingsource=ephemsrcname)
        self.imager.defineimage(nx=self.nx, ny=self.ny, cellx=self.cellx, celly=self.celly, phasecenter=self.phasecenter, spw=self.spw, stokes=self.stokes, movingsource=self.ephemsrcname)
        #self.imager.defineimage(nx=nx, ny=ny, cellx=cellx, celly=celly,  phasecenter=phasecenter, spw=0, stokes=stokes, movingsource=ephemsrcname,restfreq=1.14e11)
        self.imager.setoptions(ftmachine='sd', gridfunction=self.gridfunction)
        #self.imager.setsdoptions(convsupport=5)
        self.imager.setsdoptions(pointingcolumntouse=self.pointingcolumn, convsupport=-1, truncate=-1, gwidth=-1, jwidth=-1)
        self.imager.makeimage(type='singledish-observed', image=self.outfile)
        #self.imager.makeimage(type='singledish', image=self.outfile)
        # create temporal weight image for masking
        weightfile_temp = self.outfile+".weight."+str(time.time()).replace('.', '_')
        self.imager.makeimage(type='coverage', image=weightfile_temp)
        self.close_imager()
        # Handle mask
        my_ia = gentools(['ia'])[0]
        my_ia.open(self.outfile)
        my_ia.calcmask("'%s'>%f" % (weightfile_temp,0.0), asdefault=True)
        my_ia.close()
        shutil.rmtree(weightfile_temp)

    def __compile_common(self):
        # antenna parameter handling
        #print "antenna parameter handling..."
        self.open_table(self.antenna_table)
        antnames = self.table.getcol('NAME')
        self.nant = self.table.nrows()
        self.close_table()
        self.antid=[]
        if self.antenna == "": # assume all antennas
            self.antenna = str(range(self.nant)).lstrip('[').rstrip(']')
            self.antsel = self.antenna.replace(',','&&& ; ') + '&&&'
        elif self.antenna.find('&')==-1 and self.antenna.find(';')==-1:
            self.antsel = self.antenna + '&&&'
        else:
            self.antsel = self.antenna
        antennav = self.antenna.split(',')
        for i in range(len(antennav)):
            for j in xrange(self.nant):
                if antennav[i].strip() in [antnames[j], str(j)]:
                    self.antid.append(j)
        if len(self.antid)==1: 
            self.antid=self.antid[0] 
        elif len(self.antid)==0:
            msg='No matching antenna ID or name in the data, please check antenna parameter'
            raise Exception, msg
        self.antnameused = ''
        if type(self.antid) == int:
            self.antnameused = antnames[self.antid]
        else: 
            for i in self.antid:
                self.antnameused += antnames[i]+' ' 

        # check if the data contain multiple polarizations
        self.open_table(self.infile)
        ddids = numpy.unique(self.table.getcol('DATA_DESC_ID'))
        self.close_table()
        self.open_table(self.data_desc_table)
        polids = self.table.getcol('POLARIZATION_ID').take(ddids)
        spwids = self.table.getcol('SPECTRAL_WINDOW_ID').take(ddids)
        self.close_table()
        idx=[]
        casalog.post('spw=%s, spwids=%s'%(self.spw,spwids),'INFO')
        for id in xrange(len(spwids)):
            if spwids[id] == self.spw:
                idx.append(id)
        if len(idx) == 0:
            msg='No matching spw ID in the data, please check spw parameter'
            raise Exception, msg
        
        polid = polids[idx[0]]
        self.open_table(self.polarization_table)
        # only use first line of POLARIZATION table
        self.npol = self.table.getcell('NUM_CORR', polid)
        corrtype = self.table.getcell('CORR_TYPE', polid)
        self.corrtypestr = ([self.stokestypes[corrtype[i]] for i in range(self.npol)])
        
        #print corrtypestr
        #print "no. of polarization: %s" % npol
        casalog.post( "no. of polarization: %s" % self.npol )
        self.close_table()
        
        selpol = 0
        selnpol = 0
        selcorrtypeind=[]
        if self.npol>1:
            if self.stokes=='':
                self.stokes='I'
            if self.stokes.upper()=='I':
                if set(self.corrtypestr).issuperset(set(("XX","YY"))):
                    for ct in self.corrtypestr:
                        if ct == 'XX' or ct == 'YY':
                            selcorrtypeind.append(self.corrtypestr.index(ct))
                    #print "selected corr type indices: %s" % selcorrtypeind
                    casalog.post( "selected corr type indices: %s" % selcorrtypeind ) 
                    #print "selected (%s,%s) data" % corrtypestr
                elif set(self.corrtypestr).issuperset(set(("RR","LL"))):
                    for ct in self.corrtypestr:
                        if ct == 'RR' or ct == 'LL':
                            selcorrtypeind.append(ct.index(ct))
                    #print "selected corr type indices: %s" % selcorrtypeind
                    casalog.post( "selected corr type indices: %s" % selcorrtypeind )
                    #print "selected (%s,%s) data" % corrtypestr
                else:
                    #print "Cannot get Stokes I with the input (%s,%s) data" % corrtypestr
                    casalog.post( "Cannot get Stokes I with the input (%s,%s) data" % self.corrtypestr, priority='WARN' )
                selnpol = len(selcorrtypeind)
            else:            
                if len(self.stokes) <= 2:
                    for i in range(self.npol):
                        if self.stokes.upper()==self.corrtypestr[i]:
                            selcorrtypeind.append(i)
                            selpol = selcorrtypeind[0]
                            #print "selected %s data" % corrtypestr[i]
                            casalog.post( "selected %s data" % self.corrtypestr[i] )
                    selnpol=1 
                else:
                    # try to identify multiple corrtypes in stokes parm.
                    for i in range(len(self.stokes)):
                        ns = 2*i
                        for i in range(self.npol):
                            if self.stokes.upper()[ns:ns+2]==self.corrtypestr[i]:
                                selcorrtypeind.append(i)
                    selnpol=len(selcorrtypeind)
        else:
            #single pol data
            selpol=0
            selnpol=1
            selcorrtypeind.append(0)
            if self.stokes=='': self.stokes=self.corrtypestr[0]
            else:
                if self.stokes != self.corrtypestr[0]:
                    msg='stokes=%s specified but the data contains only %s' % (self.stokes, self.corrtypestr)
                    raise Exception, msg
        self.selpol = selpol
        self.selnpol = selnpol
        self.selcorrtypeind = selcorrtypeind

        # get number of scans and subscans
        self.open_table(self.infile)
        self.scans = numpy.unique(self.table.getcol('SCAN_NUMBER'))
        self.nscan = len(self.scans)
        casalog.post('There are %s scans in the data.'%self.nscan)
        # 2011/2/27 TN
        # number of subscans can be variable in each scan
##         subscans = numpy.unique(self.table.getcol('STATE_ID'))
##         nsubscan = len(subscans)
        self.subscans = []
        self.nsubscan = []
        for iscan in xrange(self.nscan):
            subtb = self.table.query( 'SCAN_NUMBER==%s'%(self.scans[iscan]) )
            self.subscans.append( numpy.unique(subtb.getcol('STATE_ID')) )
            subtb.close()
            self.nsubscan.append( len(self.subscans[iscan]) )
            casalog.post('There are %s subscans in scan %s'%(self.nsubscan[iscan],self.scans[iscan]))
        self.close_table()

    def __compile_for_baseline(self):
        # compile masklist
        casalog.post( "masklist...." )
        if type(self.masklist) == int:
            self.masklist = [self.masklist]
        if len(self.masklist) > 1:
            self.left_mask = self.masklist[0]
            self.right_mask = self.masklist[1]
        elif len(self.masklist) == 1:
            self.left_mask = self.right_mask = self.masklist[0]

    def __compile_for_imaging(self):
        # pointing data column to use
        #if pointingcolumn.upper()=="OFFSET":
        #    pointingcolumn="POINTING_OFFSET"
        # check ephemsrcname and set a proper source name if not given
        if self.ephemsrcname == '':
            # assume single source/field
            # try determine if it is a known ephemeris source 
            self.open_table(self.source_table)
            src = self.table.getcol('NAME')
            self.close_table()
            src = src[0]
            if src.upper() in self.ephemobjs:
                self.ephemsrcname = src

        # imsize
        (self.nx, self.ny) = sdutil.get_nx_ny(self.imsize)

        # cell size
        (self.cellx, self.celly) = sdutil.get_cellx_celly(self.cell,
                                                          unit='arcmin')
        if self.cellx == '' or self.celly == '':
            msg='Unrecognized quantity for cell'
            raise Exception, msg
            
        if self.phasecenter =='' and len(self.ephemsrcname)>0:
            # if phasecenter is undefined and if it is a moving source, 
            # the phasecneter is set to the position of the moving source
            # at the time of the first data in the POINTING table.  
            if self.pointingcolumn.upper() =='POINTING_OFFSET':
                self.phasecenter='AZELGEO 0d0m 0d0m'
            else:
                self.open_table(self.observation_table)
                telname = self.table.getcol('TELESCOPE_NAME')
                self.close_table()
                self.open_table(self.pointing_table)
                t = self.table.getcol('TIME')
                self.close_table()
                me.doframe(me.observatory(telname[0]))
                me.doframe(me.epoch('utc',str(t[0])+'s'))
                self.phasecenter = me.measure(me.direction(self.ephemsrcname),'AZELGEO')

    def __plot(self, subplot=111, title=None, ylabel=None):
        self.open_table(self.infile)
        subt = self.table.query('any(ANTENNA1==%s && ANTENNA2==%s)' % (self.antid, self.antid))
        datcol = subt.getcol(self.datacol)
        flgcol = subt.getcol("FLAG")  # only channel flag is needed to be hanled in MS
        (l,m,n) = datcol.shape
        if flgcol.shape != (l,m,n):
            raise Exception, "Data conformation error. Shape of FLAG and %s differs." % self.datacol
        nrow = subt.nrows()
        subt.close()
        self.close_table()

        pl.ioff()
        if not ylabel:
            ylabel = self.datacol
        for np in range(self.selnpol):
            pl.ioff()
            pl.figure(np+1)
            pl.subplot(subplot)
            pl.cla()
            ip = self.selpol if self.selnpol == 1 else np
            if title:
                pl.title(self.infile+' Ant:'+ self.antnameused+' '+self.corrtypestr[ip])
            datcol2 = datcol[ip].reshape(n)
            flgcol2 = flgcol[ip].reshape(n)
            if self.plotlevel > 0:
                pl.ion()
            #pl.plot(range(len(datcol2)), datcol2, 'g.')
            pl.plot(range(len(datcol2)), numpy.ma.masked_array(datcol2, flgcol2), 'g.')
            pl.xlim(0, nrow)
            t1 = pl.getp(pl.gca(), 'xticklabels')
            t2 = pl.getp(pl.gca(), 'yticklabels')
            pl.setp((t1,t2), fontsize='smaller')
            pl.ylabel(ylabel, fontsize='smaller')
            pl.xlabel('row #')
            pl.draw()
            if self.plotlevel < 0:
                outplfile=self.infile.rstrip('/')+'_scans_'+self.corrtypestr[self.selcorrtypeind[np]]+'.eps'
                pl.savefig(outplfile,format='eps')
                casalog.post( "Plot  was written to %s" % outplfile )
