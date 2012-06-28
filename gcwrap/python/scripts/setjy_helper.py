# setjy helper functions
from casac import casac
import os
import sys
import shutil
import odict
import numpy

class ss_setjy_helper:
    def __init__(self,imtool, vis, casalog=None):
	self.im = imtool
	self.vis = vis
        if not casalog:
          casalog = casac.logsink()
        self._casalog = casalog

    def setSolarObjectJy(self,field,spw,scalebychan, timerange,observation, scan):
	"""
	set flux density of a solar system object using Bryan Butler's new
	python model calculation code.
	"""
	retval = True 
	cleanupcomps = False # leave genenerated cl files 

        #from taskinit import * 
        from taskinit import gentools 
        qa = casac.quanta()
 

	(myms, mytb, mycl, myme) = gentools(['ms','tb','cl','me'])
	# prepare parameters need to pass to the Bryan's code
	# make ms selections
	# get source name
	# get time ranges
	# get spwused and get frequency ranges

	sel={}
	sel['field']=field
	sel['spw']=spw
	sel['timerange']=timerange
	sel['observation']=str(observation)
	sel['scan']=scan

	
	measframes=['REST','LSRK','LSRD','BARY','GEO','TOPO','GALACTO','LGROUP','CMB']
	myms.open(self.vis)
	myms.msselect(sel,False)
	scansummary=myms.getscansummary()
	nscan=len(scansummary.keys())
	fieldids=myms.msselectedindices()['field']
	myms.close()
	  
	mytb.open(self.vis+'/FIELD')
	if len(fieldids)==0:
	  fieldids = range(mytb.nrows())
	#srcnames=[]
	srcnames={}
	for fid in fieldids:
	  #srcnames.append(mytb.getcell('NAME',int(fid)))
	  srcnames[fid]=(mytb.getcell('NAME',int(fid)))
	mytb.close() 
	# need to get a list of time
	# but for now for test just get a time centroid of the all scans
	# get time range
	# Also, this needs to be done per source

	# gather freq info from Spw table
	mytb.open(self.vis+'/SPECTRAL_WINDOW')
	nspw = mytb.nrows()
	freqcol=mytb.getvarcol('CHAN_FREQ')
	freqwcol=mytb.getvarcol('CHAN_WIDTH')
	fmeasrefcol=mytb.getcol('MEAS_FREQ_REF')
	reffreqs=mytb.getcol('REF_FREQUENCY')
	mytb.close()

	# store all parameters need to call solar_system_fd for all sources
	inparams={}
        validfids = [] # keep track of valid fid that has data (after selection) 
        # if same source name ....need handle this...
	for fid in fieldids:
	  sel['field']=str(fid)
	  myms.open(self.vis)
          #reset the selection
	  try:
            status=myms.msselect(sel)
          except:
            #skip this field
            continue 
          if not status:
            continue 

          validfids.append(fid)
	  trange=myms.range('time')
	  if not inparams.has_key(srcnames[fid]):
            inparams[srcnames[fid]]={}

	  tc = (trange['time'][0]+trange['time'][1])/2. #in sec. 
	  if inparams[srcnames[fid]].has_key('mjd'):
            inparams[srcnames[fid]]['mjds'][0].append([myme.epoch('utc',qa.quantity(tc,'s'))['m0']['value']])
          else:
            inparams[srcnames[fid]]['mjds']=[myme.epoch('utc',qa.quantity(tc,'s'))['m0']['value']]
          # somehow it gives you duplicated ids .... so need to uniquify
	  selspws= list(set(myms.msselectedindices()['spw']))
          # make sure it is int rather than numpy.int32, etc.
          selspws = [int(ispw) for ispw in selspws]
	  inparams[srcnames[fid]]['spwids']= selspws if len(selspws)!=0 else range(nspw) 
	  #create a list of freq ranges with selected spws
	  # should worry about freq order???
	  freqlist=[]
	  framelist=[]
	  for idx in inparams[srcnames[fid]]['spwids']:
	    freqs=freqcol['r'+str(idx+1)]
	    freqws=freqwcol['r'+str(idx+1)]
	    fmeasref=fmeasrefcol[idx]
	 
	    #if scalebychan=T, this has to be min, max determined from
	    # chan_freq(channel center)+/- chan width.
	    if scalebychan:
	      perspwfreqlist=[]
	      for nf in range(len(freqs)):
		fl = freqs[nf][0]-freqws[nf][0]/2.
		fh = freqs[nf][0]+freqws[nf][0]/2.
		perspwfreqlist.append([fl,fh])
	      freqlist.append(perspwfreqlist)
	      #  freqlist.append([float(fl),float(fh)])
	    else:
	      #print "freqs=",freqs
	      if (len(freqs)==1): 
		fl = freqs[0][0]-freqws[0][0]/2.
		fh = freqs[0][0]+freqws[0][0]/2.
		freqlist.append([float(fl),float(fh)])
	      else:
		freqlist.append([float(min(freqs)),float(max(freqs))])   
	 
	    framelist.append(measframes[fmeasref])
	  #print "freqlist=",freqlist
	  inparams[srcnames[fid]]['freqlist']=freqlist
	  inparams[srcnames[fid]]['framelist']=framelist
	  inparams[srcnames[fid]]['reffreqs']=reffreqs

	# call Bryan's code
	# errcode: list of list - inner list - each values for range of freqs
	# flluxes: list of list 
	# fluxerrs:
	# size: [majoraxis, minoraxis, pa]
	# direction: direction for each time stamp
	import solar_system_setjy as ss_setjy
	retdict={}
	#for src in srcnames:
	for vfid in validfids:
          src=srcnames[vfid]
	  #print "srcnames=", src 
	  #print "mjd=",inparams[src]['mjds'] 
	  #print "freq=",inparams[src]['freqlist']
	  mjds=inparams[src]['mjds']
	  fluxes=[]
	  for i in range(len(freqlist)): # corresponds to n spw
	    if type(freqlist[0][0])==list:
	      infreqs=inparams[src]['freqlist'][i]
	    else:
	      infreqs=[inparams[src]['freqlist'][i]]
            self._casalog.post("Calling solar_system_fd: %s for spw%s freqs=%s" % (src, i,freqlist[i]),'INFO2')
	    (errcodes, subfluxes, fluxerrs, sizes, dirs)=\
               ss_setjy.solar_system_fd(source_name=src, MJDs=mjds, frequencies=infreqs, casalog=self._casalog)
            self._casalog.post("ss_fd returns fluxes=%s" % subfluxes, 'INFO2')
	    fluxes.append(subfluxes)    
            #print "fluxes=",fluxes 

	  # ------------------------------------------------------------------------
	  # For testing with hardcoded values without calling solar_system_fd()...
	  #errcodes=[[0,0,0,0,0]]
	  #fluxes=[[26.40653147,65.23839313,65.23382757,65.80638802,69.33396562]]
	  #fluxerrs=[[0.0,0.0,0.0,0.0,0.0]]
	  #sizes=[[3.6228991032674371,3.6228991032674371,0.0]]
	  #dirs=[{'m0': {'unit': 'rad', 'value': 0.0}, 'm1': {'unit': 'rad', 'value': 0.0}, 'refer': 'J2000', 'type': 'direction'}]
	  # ------------------------------------------------------------------------
	  framelist=inparams[src]['framelist']
	  freqlist=inparams[src]['freqlist']
	  reffreqs=inparams[src]['reffreqs']
	  spwids=inparams[src]['spwids']

	  clrecs=odict.odict()
	  labels = []
	  # loop for multiple sources
	  for i in range(len(dirs)):
	   
	    # check errcode - error code would be there per flux per time (dir)  
	    reterr=testerrs(errcodes[i],src) 
	    if reterr == 2: 
	      continue
	  
	    dirstring = [dirs[i]['refer'],qa.tos(dirs[i]['m0']),qa.tos(dirs[i]['m1'])]
	    # setup componentlists
	    # need to set per dir
	    # if scalebychan=F, len(freqs) corresponds to nspw selected

	    clpath='/tmp/'
	    #clpath='./'
	    for j in range(len(freqlist)):
	      freqlabel = '%.3fGHz' % (reffreqs[int(spwids[j])]/1.e9)
	      tmlabel = '%.1fd' % (tc/86400.)
	      clabel = src+'_spw'+str(spwids[j])+'_'+freqlabel+'_'+tmlabel
	      clname = clpath+clabel+'.cl'
	      #clname = clpath+inlabel+'_dir'+'.cl' 
	      
	      if(os.path.exists(clname)):
		shutil.rmtree(clname)
	      #print "addcomponent...for j=",j," flux=",fluxes[j][i]
	      if scalebychan:
		index= 2.0
		sptype = 'spectral index'
	      else:
		index= 0.0
		sptype = 'constant'
              self._casalog.post("addcomponent with flux=%s at frequency=%s" %\
                                  (fluxes[j][i][0],str(reffreqs[int(spwids[j])]/1.e9)+'GHz'), 'INFO1')
	      #mycl.addcomponent(flux=fluxes[j][i][0],fluxunit='Jy', polarization="Stokes", dir=dirs[i],
	      mycl.addcomponent(flux=fluxes[j][i][0],fluxunit='Jy', polarization="Stokes", dir=dirstring,
			 shape='disk', majoraxis=str(sizes[i][0])+'arcsec', minoraxis=str(sizes[i][1])+'arcsec', 
			 positionangle=str(sizes[i][2])+'deg', freq=[framelist[j],str(reffreqs[int(spwids[j])])+'Hz'], 
			 spectrumtype=sptype, index=index, label=clabel)
        
              # if it's list of fluxes try to put in tabular form
	      if type(fluxes[j][i]) ==list and len(fluxes[j][i])> 1:
		#print "freqlist[j]=",freqlist[j]
		#print "fluxes[j][0]=",fluxes[j][0]
	        #print "framelist[j]=",framelist[j]
		if type(freqlist[j][0])==list and len(freqlist[j][0])>1:
		  freqs=[]
		  for fr in freqlist[j]:
		    freqs.append((fr[1]+fr[0])/2)
		else:
		  freqs=freqlist[j]
                clind = mycl.length() - 1
		mycl.setspectrum(which=clind, type='tabular', tabularfreqs=freqs, tabularflux=fluxes[j][0],
	#	mycl.setspectrum(which=i, type='tabular', tabularfreqs=freqs, tabularflux=fluxes[j][0],
			   tabularframe=framelist[j])
	      mycl.rename(clname)
	      #put in a record for log output
	      clrecs[clabel] = mycl.torecord()
	      mycl.done()

	      # finally, put the componentlist as model
	      self.im.selectvis(spw=spwids[j],field=field,observation=observation,time=timerange)
	      self.im.ft(complist=clname)
	      if cleanupcomps:          
		  shutil.rmtree(clname)
		 
	  msg="Using channel dependent " if scalebychan else "Using spw dependent "
       
	  self._casalog.post(msg+" flux densities")
	  self._reportoLog(clrecs,self._casalog)
	  self._updateHistory(clrecs,self.vis)
	return retval

    def _reportoLog(self,clrecs,casalog):
	"""
	send model parameters to log
	"""
	#print "clrecs=", clrecs
	for ky in clrecs.keys():
	    # expect only one component for each
	    comp = clrecs[ky]['component0']
	    srcn = ky.split('_')[0]
	    ispw = ky.split('_')[1]
	    casalog.post(" %s: %s Flux:[I=%s,Q=%s,U=%s,V=%s] +/- [I=%s,Q=%s,U=%s,V=%s] Jy" % 
		(srcn, ispw, float('%.5g' % comp['flux']['value'][0]), 
		 float('%.5g' % comp['flux']['value'][1]),
		 float('%.5g' % comp['flux']['value'][2]), 
		 float('%.5g' % comp['flux']['value'][3]), 
		 float('%.5g' % comp['flux']['error'][0]), 
		 float('%.5g' % comp['flux']['error'][1]),
		 float('%.5g' % comp['flux']['error'][2]),
		 float('%.5g' % comp['flux']['error'][3])))

    def _updateHistory(self,clrecs,vis):
	"""
	Update history table when setSolarObjectJy is run
	"""
	# 
        from taskinit import gentools 
	(mytb,) = gentools(['tb'])
	mytb.open(vis+'/HISTORY',nomodify=False)
	nrow = mytb.nrows()
	lasttime=mytb.getcol('TIME')[nrow-1]
	rown=nrow
	#
	for ky in clrecs.keys():
	    # expect only one component for each
	    comp = clrecs[ky]['component0']
	    srcn = ky.split('_')[0]
	    ispw = ky.split('_')[1]
	    mytb.addrows(1)
	    appl='setjy' 
	    msg = (" %s: spw %s Flux:[I=%s,Q=%s,U=%s,V=%s] +/- [I=%s,Q=%s,U=%s,V=%s] Jy" %  
		(srcn, ispw, comp['flux']['value'][0], comp['flux']['value'][1], 
		 comp['flux']['value'][2], comp['flux']['value'][3],
		 comp['flux']['error'][0], comp['flux']['error'][1],comp['flux']['error'][2],
		 comp['flux']['error'][3]))

	    origin='setjy'
	    priority='INFO'
	    time=lasttime
            emptystrarr=numpy.array([''])
	    mytb.putcell('APP_PARAMS', rown, [''])
	    mytb.putcell('CLI_COMMAND',rown, [''])
	    mytb.putcell('APPLICATION',rown, appl)
	    mytb.putcell('MESSAGE',rown, msg)
	    mytb.putcell('OBSERVATION_ID',rown, -1)
	    mytb.putcell('ORIGIN',rown,origin)
	    mytb.putcell('PRIORITY',rown,priority)
	    mytb.putcell('TIME',rown, time)
	    rown += 1
	mytb.close()

def testerrs(errcode,srcname):
    """
    Check error codes from solar_system_fd
    return = 0 all ok
    return = 1 partly ok
    return = 2 all bad - should not proceed to set component
    """
    from taskinit import casalog 
    errcount = 0
    for ec in errcode:
      if ec != 0:
        errcount += 1
      if ec == 1:
         casalog.post("The model for %s is not supported" % srcname, 'WARN')
      elif ec == 2:
         casalog.post("Unsupported freuquency range",'WARN')
      elif ec == 3:
         casalog.post("Tb model not found",'WARN')
      elif ec == 4:
         casalog.post("The ephemeris table is not found or the time is out of range",'WARN')
    if errcount == len(errcode):
      return 2
    if errcount != 0:
      return 1
    else:
      return 0
