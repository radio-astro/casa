# sd task for total power raster scan imaging
import os
import numpy
import pylab as pl
import asap as sd
from taskinit import * 

interactive=False

# This is a task version of the script originally made for reducing ATF raster scan data 
# in total power mode. Still experimental...
# 
def sdtpimaging(sdfile, calmode, masklist, blpoly, flaglist, antenna, stokes, createimage, imagename, imsize, cell, phasecenter, ephemsrcname, pointingcolumn, gridfunction, plotlevel):
    # NEED to include spw, src? name for movingsource param. in function argument
    # put implementation here....
    casalog.origin('sdtpimaging')
    try:
        ### Parameter checking #############################################
        # checks performed     					       # 	
        # sdfile - must be an MS                                           # 
        # if calmode=baseline, masklist must exist, do calib               # 
        # antenna mapped to antenna ID/name in the MS                      #
        # stokes                                                           # 
        ####################################################################
        if os.path.isdir(sdfile):
            tb.open(sdfile)
            tbkeys=tb.getkeywords()
            colnames=tb.colnames()
            fdataexist = False
            calibrated = False
            tb.close()
            if any(key=='MS_VERSION' for key in tbkeys):
                if any(col=='FLOAT_DATA' for col in colnames):
                    fdataexist = True
                    #print "FLOAT_DATA exist"
                    casalog.post( "FLOAT_DATA exist" )
                else:
                    #print "No FLOAT_DATA, DATA is used"
                    casalog.post( "No FLOAT_DATA, DATA is used" )
                if any(col=='CORRECTED_DATA' for col in colnames):
                    calibrated = True
                    #print "Calibrated data seems to exist"
                    casalog.post( "Calibrated data seems to exist" )
                elif calmode != 'none':
                    # Workaround: create CORRECTED_DATA using im tool
                    casalog.post( "CORRECTED_DATA doesn't exist, create it..." )
                    im.open(sdfile,usescratch=True)
                    im.close()
            else:
                msg='sdfile must be in MS format'
                raise Exception, msg
                
        else:
            msg='sdfile must be in MS format'
            raise Exception, msg
        sdfile=sdfile.rstrip('/')

        if calmode == 'baseline':
            if masklist is None:
                msg='Please specify a region to be fitted in masklist'
                raise Exception, msg
            		
            #print "masklist...."
            casalog.post( "masklist...." )
            if type(masklist)==int:
                masklist=[masklist]
            if type(masklist)==list or type(masklist)==tuple:
                if len(masklist) >1:
                    lmask=masklist[0]
                    rmask=masklist[1]
                elif len(masklist)==1:
                    lmask=masklist[0]
                    rmask=masklist[0]
                else:
                    msg='Please specify a region to be fitted in masklist'
                    raise Exception, msg
        # antenna parameter handling
        #print "antenna parameter handling..."
        tb.open(sdfile+'/ANTENNA')
        antnames=tb.getcol('NAME')
        nant=tb.nrows()
        tb.close()
        if antenna == "": # assume all antennas
            antenna = str(range(nant))
            antenna = antenna.lstrip('[')
            antenna = antenna.rstrip(']') 
        antlist={}
        for i in range(nant):
            antlist[i]=antnames[i]
        antennav = antenna.split(',')
        antid=[]
        for i in range(len(antennav)):
            for key in antlist:
                if antennav[i].strip()==antlist[key]:
                    antid.append(key)
                elif antennav[i].strip()==str(key):
                    antid.append(key)
        if len(antid)==1: 
            antid=antid[0] 
        elif len(antid)==0:
            msg='No matching antenna ID or name in the data, please check antenna parameter'
            raise Exception, msg

        antnameused=''
        if type(antid)==int:
            antnameused=antnames[antid]
        else: 
            for i in antid:
                antnameused += antnames[i]+' ' 

        # check if the data contain multiple polarizations
        # define stokestypes (only up to 'YL') based on enumerated stokes type in CASA
        stokestypes=['undef','I','Q','U','V','RR','RL','LR','LL','XX','XY','YX','YY','RX','RY','LX','LY','XR','XL','YR','YL']
        tb.open(sdfile)
        ddids=numpy.unique(tb.getcol('DATA_DESC_ID'))
        tb.close()
        tb.open(sdfile+'/DATA_DESCRIPTION')
        polids=tb.getcol('POLARIZATION_ID').take(ddids)
        tb.close()
        tb.open(sdfile+'/POLARIZATION')
        #only use first line of POLARIZATION table
        #npol=tb.getcol('NUM_CORR')[0]
        npol=tb.getcol('NUM_CORR')[polids[0]]
        #corrtype=tb.getcol('CORR_TYPE')
        corrtype=tb.getcell('CORR_TYPE',polids[0])
        #corrtypestr = (stokestypes[corrtype[0][0]],stokestypes[corrtype[1][0]])
        #corrtypestr = ([stokestypes[corrtype[i][0]] for i in range(npol)])
        corrtypestr = ([stokestypes[corrtype[i]] for i in range(npol)])
        
        #print corrtypestr
        #print "no. of polarization: %s" % npol
        casalog.post( "no. of polarization: %s" % npol )
        tb.close()
        selpol = 0
        selnpol = 0
        selcorrtypeind=[]
        if npol>1:
            if stokes=='':
                stokes='I'
            if stokes.upper()=='I':
                if set(corrtypestr).issuperset(set(("XX","YY"))):
                    for ct in corrtypestr:
                        if ct == 'XX' or ct == 'YY':
                            selcorrtypeind.append(corrtypestr.index(ct))
                    #print "selected corr type indices: %s" % selcorrtypeind
                    casalog.post( "selected corr type indices: %s" % selcorrtypeind ) 
                    #print "selected (%s,%s) data" % corrtypestr
                elif set(corrtypestr).issuperset(set(("RR","LL"))):
                    for ct in corrtypestr:
                        if ct == 'RR' or ct == 'LL':
                            selcorrtypeind.append(ct.index(ct))
                    #print "selected corr type indices: %s" % selcorrtypeind
                    casalog.post( "selected corr type indices: %s" % selcorrtypeind )
                    #print "selected (%s,%s) data" % corrtypestr
                else:
                    #print "Cannot get Stokes I with the input (%s,%s) data" % corrtypestr
                    casalog.post( "Cannot get Stokes I with the input (%s,%s) data" % corrtypestr, priority='WARN' )
                selnpol = len(selcorrtypeind)
            else:            
                if len(stokes) <= 2:
                    for i in range(npol):
                        if stokes.upper()==corrtypestr[i]:
                            selcorrtypeind.append(i)
                            selpol = selcorrtypeind[0]
                            #print "selected %s data" % corrtypestr[i]
                            casalog.post( "selected %s data" % corrtypestr[i] )
                    selnpol=1 
                else:
                    # try to identify multiple corrtypes in stokes parm.
                    for i in range(len(stokes)):
                        ns = 2*i
                        for i in range(npol):
                            if stokes.upper()[ns:ns+2]==corrtypestr[i]:
                                selcorrtypeind.append(i)
                    selnpol=len(selcorrtypeind)
        else:
            #single pol data
            selpol=0
            selnpol=1
            selcorrtypeind.append(0)
            if stokes=='':stokes=corrtypestr[0]
            else:
                if stokes != corrtypestr[0]:
                    msg='stokes=%s specified but the data contains only %s' % (stokes, corrtypestr)
                    raise Exception, msg

        #parameter checks/setup for imaging
        if createimage:
            # pointing data column to use
            #if pointingcolumn.upper()=="OFFSET":
            #    pointingcolumn="POINTING_OFFSET"
            # check ephemsrcname and set a proper source name if not given
            if ephemsrcname=='':
                # assume single source/field
                # try determine if it is a known ephemeris source 
                tb.open(sdfile+'/SOURCE')
                src=tb.getcol('NAME')
                tb.close()
                src = src[0]
                ephemobjs = set(['MERCURY','VENUS','MARS','JUPITER','SATURN','URANUS','NEPTUNE','PLUTO','SUN','MOON'])
                if src.upper() in ephemobjs:
                    ephemsrcname=src

            if imagename=='':
                msg='Please specify out image name'
                raise Exception, msg
            # imsize
            if len(imsize) == 1:
                nx=imsize[0]
                ny=imsize[0]
            else:
                nx=imsize[0]
                ny=imsize[1]
            # cell size
            cellx=celly=''
            if type(cell) == int or type(cell) == float:
                cellx=qa.quantity(cell, 'arcmin')
                celly=qa.quantity(cell, 'arcmin')
            elif type(cell) == str:
                if qa.isquantity(cell):
                    cellx=cell
                    celly=cell
                else:
                    msg='Unrecognized quantity for cell'
                    raise Exception, msg
            elif type(cell) == list:
                if len(cell) == 1:
                    if type(cell[0]) == int or type(cell[0]) == float:
                        cellx=qa.quantity(cell[0], 'arcmin')
                        celly=qa.quantity(cell[0], 'arcmin')
                    elif type(cell[0])==str:
                        if qa.isquantity(cell[0]):
                            cellx=cell[0]
                            celly=cell[0]
                        else:
                            msg='Unrecognized quantity for cell'
                            raise Exception, msg
                else:
                    if type(cell[0]) == int or type(cell[0]) == float:
                        cellx=qa.quantity(cell[0], 'arcmin')
                    elif type(cell[0])==str:
                        if qa.isquantity(cell[0]):
                            cellx=cell[0]
                        else:
                            msg='Unrecognized quantity for cell'
                            raise Exception, msg
                    if type(cell[1]) == int or type(cell[1]) == float:
                        celly=qa.quantity(cell[1], 'arcmin')
                    elif type(cell[1])==str:
                        if qa.isquantity(cell[1]):
                            celly=cell[1]
                        else:
                            msg='Unrecognized quantity for cell'
                            raise Exception, msg
            if phasecenter =='' and len(ephemsrcname)>0:
                # if phasecenter is undefined and if it is a moving source, 
                # the phasecneter is set to the position of the moving source
                # at the time of the first data in the POINTING table.  
                if pointingcolumn.upper() =='POINTING_OFFSET':
                    phasecenter='AZELGEO 0d0m 0d0m'
                else:
                    tb.open(sdfile+'/OBSERVATION')
                    telname=tb.getcol('TELESCOPE_NAME')
                    tb.close()
                    tb.open(sdfile+'/POINTING')
                    t = tb.getcol('TIME')
                    tb.close()
                    me.doframe(me.observatory(telname[0]))
                    me.doframe(me.epoch('utc',str(t[0])+'s'))
                    phasecenter = me.measure(me.direction(ephemsrcname),'AZELGEO')
        # get number of scans and subscans
        tb.open(sdfile)
        scans = numpy.unique(tb.getcol('SCAN_NUMBER'))
        nscan = len(scans)
        subscans = numpy.unique(tb.getcol('STATE_ID'))
        nsubscan = len(subscans)
        casalog.post('There are %s scans in the data.'%nscan)
        tb.close()

        #print "calibration begins..."
        #select the totalpower data for antenna1,2=1 (DV01, vertex antenna)
        # use generic pylab
        datalab='DATA'
        if calmode=='baseline':
            if type(antid)==list and len(antid)>2:
                #print "Assume the antenna selection is all..."
                casalog.post( "Assume the antenna selection is all..." )
            #if calibrated:
            #    docalib=raw_input('Do you want to proceed with calibration?(\'y\' or \'n\' will terminate the task, then you can re-run with calmode=\'none\')\n')
            #    if docalib=='n':
            #        return 
            #data=[]
            tb.open(sdfile)
            ndat=0
            if fdataexist: 
                datalab='FLOAT_DATA'
            else:
                datalab='DATA'
            pl.ioff()
            # determine the size of processing data
            subtb=tb.query('any(ANTENNA1==%s && ANTENNA2==%s)' % (antid,antid))
            nr = subtb.nrows()
            #nr = len(data)*len(data[0])
            #ndatcol = numpy.zeros((npol,nr),dtype=numpy.float)
            ndatcol = subtb.getcol('CORRECTED_DATA')
            (l,m,n) = ndatcol.shape
            ndatcol.reshape(l,n)
            for np in range(len(selcorrtypeind)):
                pl.figure(np+1)
                pl.clf()
                pl.subplot(311)
                #pl.title(sdfile+' Ant:'+str(antlist.values()))
                pl.ylabel(datalab,fontsize='smaller')
                symbols=['b.','c.']
                
                #print "Arranging data by scans..."
                casalog.post( "Arranging data by scans..." )
                
                data = []
                ndat0 = 0
                ndat = 0
                if selnpol==1: np = selpol
                pl.title(sdfile+' Ant:'+antnameused+' '+corrtypestr[np])
                casalog.post( "select %s data..." % corrtypestr[np] )
                for i in xrange(nscan):
                    idata = []
                    for j in xrange(nsubscan):
                        # may be need to iterate on each antenna 
                        # identify 'scan' by STATE ID
                        subtb=tb.query('any(ANTENNA1==%s && ANTENNA2==%s) && SCAN_NUMBER==%s && STATE_ID==%s' % (antid,antid,scans[i],subscans[j]))
                        datcol=subtb.getcol(datalab)
                        if npol >1 and selnpol==1:
                            #casalog.post( "select %s data..." % corrtypestr[selpol] )
                            rdatcol=datcol[selpol].real
                        else:
                            rdatcol=datcol[selcorrtypeind[np]].real
                        (m,n)=rdatcol.shape
                        rdatcol=rdatcol.reshape(n)
                        #data.append(rdatcol)
                        idata.append(rdatcol)
                        ndat0 = ndat
                        ndat += len(rdatcol)
                    data.append(idata)
                if abs(plotlevel) > 0:
                    pl.xlim(0,ndat)
                    t1=pl.getp(pl.gca(),'xticklabels')
                    t2=pl.getp(pl.gca(),'yticklabels')
                    pl.setp((t1,t2),fontsize='smaller')
                    mdat = 0
                    for i in xrange(nscan):
                        for j in xrange(nsubscan):
                            mdat0 = mdat
                            mdat += len(data[i][j])
                            if plotlevel > 0 and (i == nscan-1) and (j == nsubscan-1):
                                pl.ion()
                            pl.plot(xrange(mdat0,mdat),data[i][j],symbols[subscans[j]%2])
                            ax=pl.gca()
                            if i==1: 
                                leg=ax.legend(('even subscan no.', 'odd subscan no.'),numpoints=1,handletextsep=0.01) 
                                for t in leg.get_texts():
                                    t.set_fontsize('small')
                    pl.draw()
                ### Calibration ############################################
                # Do a zero-th order calibration by subtracting a baseline #
                # from each scan to take out atmospheric effect.          #
                # The baseline fitting range specified by masklist must be #
                # given and is the same for all the scans.                 # 
                ############################################################
                f=sd.fitter()
                f.set_function(lpoly=blpoly)
                #print "Processing %s %s scans" % (corrtypestr[np], nscan)
                casalog.post( "Processing %s %s scans" % (corrtypestr[np], nscan) )
                cdat = numpy.array([])
                pl.subplot(312)
                pl.ioff()
                for i in xrange(nscan):
                    casalog.post( "Processing Scan#=%s" % i )
                    for j in xrange(nsubscan):
                        masks=numpy.zeros(len(data[i][j]),dtype=numpy.int)
                        msg = "Subtracting baselines, set masks for fitting at row ranges: [0,%s] and [%s,%s] " % (lmask-1, len(masks)-rmask, len(masks)-1)
                        casalog.post(msg, "INFO")
                        masks[:lmask]=True
                        masks[-rmask:]=True
                        x=xrange(len(data[i][j]))
                        if plotlevel > 1:
                            pl.cla()
                            pl.ioff()
                            pl.title('scan %s subscan %s'%(scans[i],subscans[j]))
                            pl.plot(x,data[i][j],'b')
                        f.set_data(x,data[i][j],mask=masks)
                        f.fit()
                        #f.plot(residual=True, components=[1], plotparms=True)
                        fitd=f.get_fit()
                        data[i][j]=data[i][j]-fitd
                        if plotlevel > 1:
                            pl.ion()
                            pl.plot(xrange(len(fitd)),fitd,'r-')
                            pl.draw()
                            pl.cla()
                            pl.ioff()
                            pl.title('scan %s subscan %s'%(scans[i],subscans[j]))
                            pl.ion()
                            pl.plot(x,data[i][j],'g')
                            pl.draw()
                            if interactive: check=raw_input('Hit return for Next\n')
                        cdat=numpy.concatenate([cdat,data[i][j]])
                ndatcol[np]=cdat
                del data
            subtb.close()
            tb.close()
            tb.open(sdfile,nomodify=False)
            # put the corrected data to CORRECTED_DATA column in MS
            # assuming the data for the vertex are stored in every other
            # row starting from 2nd row for the vertex antenna....
            # For the alcatel startrow=0, for len(antid) >1,
            # it assumes all the data to be corrected. 
            if type(antid)==int:
                startrow=antid
                rowincr=nant
            else:
                startrow=0
                rowincr=1
            casalog.post( "Storing the corrected data to CORRECTED_DATA column in the MS..." )
            cdatm=ndatcol.reshape(npol,1,len(cdat))
            tb.putcol('CORRECTED_DATA', cdatm, startrow=startrow, rowincr=rowincr)
            tb.close() 
            # calibration end
            #
            # plot corrected data
            tb.open(sdfile)
            subt=tb.query('any(ANTENNA1==%s && ANTENNA2==%s)' % (antid, antid))
            cdatcol=subt.getcol('CORRECTED_DATA')
            (l,m,n)=cdatcol.shape
            subt.close()
            tb.close()
            casalog.post( "plotting corrected data..." )
            pl.subplot(313)
            pl.ioff()
            for np in range(selnpol):
                pl.figure(np+1)
                if selnpol==1:
                    #pl.title(sdfile+' Ant:'+ antnameused+' '+corrtypestr[selpol])
                    cdatcol2=cdatcol[selpol].reshape(n)
                else:
                    #pl.title(sdfile+' Ant:'+ antnameused+' '+corrtypestr[np])
                    cdatcol2=cdatcol[np].reshape(n)
                    #cdatcol2=cdatcol.reshape(n)
                if abs(plotlevel) > 0:
                    if plotlevel > 0:
                        pl.ion()
##                     pl.ion()
                    pl.plot(xrange(len(cdatcol2)),cdatcol2, 'g.')
                    pl.xlim(0,ndat)
                    t1=pl.getp(pl.gca(),'xticklabels')
                    t2=pl.getp(pl.gca(),'yticklabels')
                    pl.setp((t1,t2),fontsize='smaller')
                    pl.ylabel('CORRECTED_DATA',fontsize='smaller')
                    #pl.text(len(cdatcol2)*1.01,cdatcol2.min(),'[row #]',fontsize='smaller')
                    pl.xlabel('[row #]',fontsize='smaller')
                    pl.draw()
                if plotlevel < 0:
                    outplfile=sdfile+'_scans_'+corrtypestr[selcorrtypeind[np]]+'.eps'
                    pl.savefig(outplfile,format='eps')
                    casalog.post( "Plot was written to %s" % outplfile ) 
        else: # no calib, if requested plot raw/calibrated data 
            if abs(plotlevel) > 0:
                tb.open(sdfile)
                subt=tb.query('any(ANTENNA1==%s && ANTENNA2==%s)' % (antid, antid))
                nrow=subt.nrows()
                for np in range(selnpol):
                    if calibrated:
                        datalab = 'CORRECTED_DATA'
                        datcol=subt.getcol(datalab)
                    else:
                        if fdataexist:
                            datalab='FLOAT_DATA'
                        else:
                            datalab='DATA'
                        datcol=subt.getcol(datalab)
                    tb.close()
                    pl.ioff()
                    pl.figure(np+1)
                    pl.clf()
                    (l,m,n)=datcol.shape
                    if selnpol==1:
                        pl.title(sdfile+' Ant:'+ antnameused+' '+corrtypestr[selpol])
                        datcol2=datcol[selpol].reshape(n)
                    else:
                        pl.title(sdfile+' Ant:'+ antnameused+' '+corrtypestr[np])
                        datcol2=datcol[np].reshape(n)
                    pl.subplot(111)
                    if plotlevel>0:
                        pl.ion()
                    pl.plot(range(len(datcol2)),datcol2, 'g.')
                    pl.xlim(0,nrow)
                    t1=pl.getp(pl.gca(),'xticklabels')
                    t2=pl.getp(pl.gca(),'yticklabels')
                    pl.setp((t1,t2),fontsize='smaller')
                    pl.ylabel(datalab,fontsize='smaller')
                    #pl.text(len(datcol2)*1.01,datcol2.min(),'[row #]',fontsize='smaller')
                    pl.xlabel('row #')
                    pl.draw()
                    if plotlevel<0:
                        outplfile=sdfile+'_scans_'+corrtypestr[selcorrtypeind[np]]+'.eps'
                        pl.savefig(outplfile,format='eps')
                        casalog.post( "Plot  was written to %s" % outplfile )
                   
        #flag scans
        if len(flaglist) > 0:
            tb.open(sdfile)
            subtb=tb.query('any(ANTENNA1==%s && ANTENNA2==%s)' % (antid,antid))
            fdatcol = subtb.getcol('FLAG')
            (l,m,n) = fdatcol.shape
            fdatcol.reshape(l,n)
            casalog.post( "Flag processing..." )
            flagscanlist=[]
            if type(antid) == int:
                startrow=antid
                rowincr=nant
            else:
                startrow=0
                rowincr=1
            tb.open(sdfile, nomodify=False)
            for flag in flaglist:
                if type(flag) == list:
                    flagscanlist.extend(range(flag[0],(flag[1]+1)))
                if type(flag) == int:
                    flagscanlist.append(flag)
            flagscanset=set(flagscanlist) 
            for np in range(len(selcorrtypeind)):
                fdata=[]
                fdatac = numpy.array([])
                for i in xrange(nscan):
                    #fdata=[]
                    for j in xrange(nsubscan):
                        subtb=tb.query('any(ANTENNA1==%s && ANTENNA2==%s) && SCAN_NUMBER==%s && STATE_ID==%s' % (antid,antid,scans[i],subscans[j]))
                        fcolall=subtb.getcol('FLAG')
                        if npol >1 and selnpol==1:
                            fcol=fcolall[selpol]
                        else:
                            fcol=fcolall[selcorrtypeind[np]]
                        (m,n) = fcol.shape
                        fcoln=fcol.reshape(n)
                        if i in flagscanset:
                            flgs=numpy.ones(n,dtype=numpy.bool)
                            fcoln=flgs 
                        #fdata.append(fcoln)
                        fdatac=numpy.concatenate([fdatac,fcoln])
                fdatcol[np]=fdatac
                #flagc=tb.getcol('FLAG')
            subtb.close()
            fdatacm=fdatcol.reshape(npol,1,len(fdatac))
            tb.putcol('FLAG', fdatacm, startrow=startrow, rowincr=rowincr)
            #print "Scans flagged: %s" % list(flagscanset)
            casalog.post( "Scans flagged: %s" % list(flagscanset) )
            tb.close()

                      	
        ### imaging ###############
        # field id 0 is assumed   #
        ###########################
        if createimage:
            #if pointingcolumn.upper()=="OFFSET":
            #   pointingcolumn="POINTING_OFFSET"
            #print "pointingcolumn=",pointingcolumn
            #print "Imaging...."
            casalog.post( "pointingcolumn used: %s" % pointingcolumn )
            casalog.post( "Imaging...." )
            im.open(sdfile)
            im.selectvis(field=0, spw='', baseline=antenna)
            im.defineimage(nx=nx, ny=ny, cellx=cellx, celly=celly,  phasecenter=phasecenter, spw=0, stokes=stokes, movingsource=ephemsrcname)
            #im.defineimage(nx=nx, ny=ny, cellx=cellx, celly=celly,  phasecenter=phasecenter, spw=0, stokes=stokes, movingsource=ephemsrcname,restfreq=1.14e11)
            im.setoptions(ftmachine='sd', gridfunction=gridfunction)
            #im.setsdoptions(convsupport=5)
            im.setsdoptions(pointingcolumntouse=pointingcolumn)
            im.makeimage(type='singledish', image=imagename)
            im.close()

    except Exception, instance:
        #print '***Error***',instance
        casalog.post( str(instance), priority='ERROR' )
        return
