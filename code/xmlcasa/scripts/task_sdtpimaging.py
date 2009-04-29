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
def sdtpimaging(sdfile, calmode, masklist, blpoly, flaglist, antenna, createimage, imagename, imsize, cell, phasecenter, ephemsrcname, plotlevel):
       # NEED to include spw, src? name for movingsource param. in function argument
       # put implementation here....
        casalog.origin('sdtpimaging')

        try:
            ### Parameter checking #############################################
            # checks performed     					       # 	
            # sdfile - must be an MS                                           # 
            # if calmode=baseline, masklist must exist, do calib               # 
            # antenna mapped to antenna ID/name in the MS                      #
            #                                                                  # 
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
                        print "FLOAT_DATA exist"
                    else:
                        print "No FLOAT_DATA, DATA is used"
                    if any(col=='CORRECTED_DATA' for col in colnames):
                        calibrated = True
                        print "Calibrated data seems to exist"
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

            #print "ephemsrcname handling..."
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

            if createimage is True and imagename=='':
               msg='Please specify out image name'
	       raise Exception, msg

            if len(imsize) == 1:
	       nx=imsize[0]
	       ny=imsize[0]
	    else:
	       nx=imsize[0]
	       ny=imsize[1]

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

            # get number of scans 
            tb.open(sdfile+'/STATE')
            # assume each SUB_SCAN id represents each raster 'scan'
            nscan=tb.nrows()
            if calmode=='none':
                print "There are %s scans in the data." % nscan
       	    tb.close()

            #print "calibration begins..."
            #select the totalpower data for antenna1,2=1 (DV01, vertex antenna)
            # use generic pylab
            datalab='DATA'
            if calmode=='baseline':
                if type(antid)==list and len(antid)>2:
                    print "Assume the antenna selection is all..." 
                #if calibrated:
	        #    docalib=raw_input('Do you want to proceed with calibration?(\'y\' or \'n\' will terminate the task, then you can re-run with calmode=\'none\')\n')
                #    if docalib=='n':
                #        return 
                data=[]
                tb.open(sdfile)
                ndat=0
                if fdataexist: 
                    datalab='FLOAT_DATA'
                else:
                    datalab='DATA'
                if abs(plotlevel) > 0:
                    if plotlevel< 0:
                       pl.ioff()
                    else:
                       pl.ion()
		    pl.clf()
		    pl.subplot(311)
		    #pl.title(sdfile+' Ant:'+str(antlist.values()))
		    pl.title(sdfile+' Ant:'+antnameused)
		    pl.ylabel(datalab,fontsize='smaller')
		    symbols=['b.','c.']
                 
                print "Arranging data by scans..."
		for i in range(nscan):
		# may be need to iterate on each antenna 
		# identify 'scan' by STATE ID
		    subtb=tb.query('any(ANTENNA1==%s && ANTENNA2==%s) && STATE_ID==%s' % (antid,antid, i))
		    datcol=subtb.getcol(datalab)
		    rdatcol=datcol.real
		    (l,m,n)=datcol.shape
	            rdatcol=rdatcol.reshape(n)
		    data.append(rdatcol)
		    ndat0 = ndat
		    ndat += len(rdatcol)
		    #pl.plot(range(ndat0,ndat),rdatcol,'k.')
		    if abs(plotlevel) > 0: 
                        pl.plot(range(ndat0,ndat),rdatcol,symbols[i%2])
                        ax=pl.gca()
                        #if i==0:
                          #leg=ax.legend(['odd scan'],numpoints=1,handletextsep=0.01) 
                          #for t in leg.get_texts():
                          #    t.set_fontsize('small')
                          #pl.draw()
                        if i==1: 
                          leg=ax.legend(('even scan no.', 'odd scan no.'),numpoints=1,handletextsep=0.01) 
                          for t in leg.get_texts():
                              t.set_fontsize('small')
                          pl.draw()
                if abs(plotlevel) > 0:
		    pl.xlim(0,ndat)
		    t1=pl.getp(pl.gca(),'xticklabels')
		    t2=pl.getp(pl.gca(),'yticklabels')
		    pl.setp((t1,t2),fontsize='smaller')
		    subtb.close()
		tb.close()

		### Calibration ############################################
		# Do a zero-th order calibration by subtracting a baseline #
		# from each scan to take out atmospheric effect.          #
		# The baseline fitting range specified by masklist must be #
		# given and is the same for all the scans.                 # 
		############################################################
		masks=numpy.zeros(len(data[0]),dtype=numpy.int)
		msg = "Subtracting baselines, set masks for fitting at row ranges: [0,%s] and [%s,%s] " % (lmask-1, len(masks)-rmask, len(masks)-1)
                casalog.post(msg, "INFO")
		masks[:lmask]=True
		masks[-rmask:]=True
            
		f=sd.fitter()
		f.set_function(lpoly=blpoly)
		print "Processing %s scans" % nscan
		for i in range(nscan):
		    print "Processing Scan#=", i
		    x=range(len(data[i]))
		    if abs(plotlevel) > 1:
		        pl.subplot(312)
			pl.cla()
			pl.plot(x,data[i],'b')
		    f.set_data(x,data[i],mask=masks)
		    f.fit()
		    #f.plot(residual=True, components=[1], plotparms=True)
		    fitd=f.get_fit()
		    data[i]=data[i]-fitd
		    if abs(plotlevel) > 1:
			pl.plot(range(len(fitd)),fitd,'r-')
			pl.cla()
			pl.plot(x,data[i],'g')
			pl.title('scan %s' % i)
			if interactive: check=raw_input('Hit return for Next\n')
		    cdat=numpy.concatenate(([datarr for datarr in data]))
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
                     
	        print "Storing the corrected data to CORRECTED_DATA column in the MS..."
	        cdatm=cdat.reshape(1,1,len(cdat))
                cdato=tb.getcol('CORRECTED_DATA')
	        tb.putcol('CORRECTED_DATA', cdatm, startrow=startrow, rowincr=rowincr)
                tb.close() 
                # calibration end
		#
		# plot corrected data
		tb.open(sdfile)
		subt=tb.query('any(ANTENNA1==%s && ANTENNA2==%s)' % (antid, antid))
		cdatcol=subt.getcol('CORRECTED_DATA')
		(l,m,n)=cdatcol.shape
		cdatcol2=cdatcol.reshape(n)
                if abs(plotlevel) >0:
		    pl.subplot(313)
		    pl.plot(range(len(cdatcol2)),cdatcol2, 'g.')
		    pl.xlim(0,ndat)
		    t1=pl.getp(pl.gca(),'xticklabels')
		    t2=pl.getp(pl.gca(),'yticklabels')
		    pl.setp((t1,t2),fontsize='smaller')
		    pl.ylabel('CORRECTED_DATA',fontsize='smaller')
		    #pl.text(len(cdatcol2)*1.01,cdatcol2.min(),'[row #]',fontsize='smaller')
		    pl.xlabel('[row #]',fontsize='smaller')
		tb.close()
                if plotlevel < 0:
                   outplfile=sdfile+'_scans.eps'
                   pl.savefig(outplfile,format='eps')
                   print "Plot was written to %s" % outplfile 
            else: # no calib, if requested plot raw/calibrated data 
                if abs(plotlevel) > 0:
		    tb.open(sdfile)
		    subt=tb.query('any(ANTENNA1==%s && ANTENNA2==%s)' % (antid, antid))
                    nrow=subt.nrows()
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
                    pl.figure(1)
		    pl.clf()
		    pl.title(sdfile+' Ant:'+ antnameused)
		    pl.subplot(111)
		    (l,m,n)=datcol.shape
                    #To avoid error reported as CAS-1312
                    #--2009/4/24 Takeshi Nakazato
                    #datcol2=datcol.reshape(n)
                    datcol2=datcol[0].reshape(n)
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
                    #if plotlevel>0:
                    #   pl.ion()
                    pl.draw()
                    if plotlevel<0:
                       outplfile=sdfile+'_scans.eps'
                       pl.savefig(outplfile,format='eps')
                       print "Plot  was written to %s" % outplfile 
                   
	
            #flag scans
            if len(flaglist) > 0:
                print "Flag processing..."
                fdata=[]
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
	        for i in range(nscan):
		    subtb=tb.query('any(ANTENNA1==%s && ANTENNA2==%s) && STATE_ID==%s' % (antid,antid, i))
		    fcol=subtb.getcol('FLAG')
                    (l,m,n) = fcol.shape
                    fcoln=fcol.reshape(n)
                    if i in flagscanset:
                        flgs=numpy.ones(n,dtype=numpy.bool)
                        fcoln=flgs 
                    fdata.append(fcoln)
                fdatac=numpy.concatenate(([datarr for datarr in fdata]))
                flagc=tb.getcol('FLAG')
	        fdatacm=fdatac.reshape(1,1,len(fdatac))
	        tb.putcol('FLAG', fdatacm, startrow=startrow, rowincr=rowincr)
                # need flip True -> 0
                iw=numpy.array(-fdatacm.reshape(1,len(fdatac)), dtype=numpy.int64)
	        tb.putcol('IMAGING_WEIGHT', iw, startrow=startrow, rowincr=rowincr)
                print "Scans flagged: %s" % list(flagscanset)
                tb.close()

                      	
            ### imaging ###############
            # field id 0 is assumed   #
            ###########################
            if createimage:
                print "Imaging...."
                im.open(sdfile)
                im.selectvis(field=0, spw='', baseline=antenna)
                im.defineimage(nx=nx, ny=ny, cellx=cellx, celly=celly,  phasecenter=phasecenter, spw=0, movingsource=ephemsrcname)
                #print "im.defineimage(nx=%s, ny=%s, cellx=%s, celly=%s, phasecenter=%s, spw=0, movingsource='moon') " % (str(imsize[0]), str(imsize[1]), cell[0], cell[1], phasecenter)
                im.setoptions(ftmachine='sd')
                #im.setsdoptions(convsupport=5)
                im.makeimage(type='singledish', image=imagename)
                im.close()

        except Exception, instance:
            print '***Error***',instance
            return



