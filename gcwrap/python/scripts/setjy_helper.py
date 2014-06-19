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

    def setSolarObjectJy(self,field,spw,scalebychan, timerange,observation, scan, intent, useephemdir, usescratch=False):
	"""
	Set flux density of a solar system object using Bryan Butler's new
	python model calculation code.
        A single time stamp (first time stamp of MS after selections are applied) is
        currently used per execution. For flux observation done in a long time span
        may need to run multiple setjy with selections by time range (or scans). 
	"""
	#retval = True 
        output = {}
	cleanupcomps = True # leave generated cl files 

        #from taskinit import * 
        from taskinit import gentools 
        qa = casac.quanta()
 

	(myms, mytb, mycl, myme, mycb) = gentools(['ms','tb','cl','me', 'cb'])
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
        sel['scanintent']=intent

	measframes=['REST','LSRK','LSRD','BARY','GEO','TOPO','GALACTO','LGROUP','CMB']
	myms.open(self.vis)
	myms.msselect(sel,False)
	scansummary=myms.getscansummary()
	nscan=len(scansummary.keys())
        selectedids=myms.msselectedindices()
	fieldids=selectedids['field']
        obsids=selectedids['observationid']
	myms.close()
	  
        mytb.open(self.vis+'/OBSERVATION')
        if len(obsids)==0:
          getrow=0
        else:
          getrow=obsids[0]
        observatory=mytb.getcell('TELESCOPE_NAME',getrow)
        mytb.close()
 
	mytb.open(self.vis+'/FIELD')
	if len(fieldids)==0:
	  fieldids = range(mytb.nrows())
        # frame reference for field position
        phdir_info=mytb.getcolkeyword("PHASE_DIR","MEASINFO")
        if phdir_info.has_key('Ref'):
          fieldref=phdir_info['Ref']
        elif phdir_info.has_key('TabRefTypes'):
          colnames=mytb.colnames()
          for col in colnames:
            if col=='PhaseDir_Ref':
                fieldrefind=mytb.getcell(col,fieldids[0])
                fieldref=phdir_info['TabRefTypes'][fieldrefind]
        else:
          fieldref=''
	srcnames={}
        fielddirs={}
        ftimes={}
	for fid in fieldids:
	  #srcnames.append(mytb.getcell('NAME',int(fid)))
	  srcnames[fid]=(mytb.getcell('NAME',int(fid)))
          fielddirs[fid]=(mytb.getcell('PHASE_DIR',int(fid)))
          ftimes[fid]=(mytb.getcell('TIME',int(fid)))
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
	  #if not inparams.has_key(srcnames[fid]):
          #  inparams[srcnames[fid]]={}
	  if not inparams.has_key(fid):
            inparams[fid]={}
            inparams[fid]['fieldname']=srcnames[fid]

	  #tc = (trange['time'][0]+trange['time'][1])/2. #in sec. 
          # use first timestamp to be consistent with the ALMA Control
          # old setjy (Butler-JPL-Horizons 2010) seems to be using
          # time in FIELD... but here is first selected time in main table
          tc = trange['time'][0] #in sec.
	  #if inparams[srcnames[fid]].has_key('mjd'):
          #  inparams[srcnames[fid]]['mjds'][0].append([myme.epoch('utc',qa.quantity(tc,'s'))['m0']['value']])
          #else:
          #  inparams[srcnames[fid]]['mjds']=[myme.epoch('utc',qa.quantity(tc,'s'))['m0']['value']]
	  if inparams[fid].has_key('mjd'):
            inparams[fid]['mjds'][0].append([myme.epoch('utc',qa.quantity(tc,'s'))['m0']['value']])
          else:
            inparams[fid]['mjds']=[myme.epoch('utc',qa.quantity(tc,'s'))['m0']['value']]
          # somehow it gives you duplicated ids .... so need to uniquify
	  selspws= list(set(myms.msselectedindices()['spw']))
          # make sure it is int rather than numpy.int32, etc.
          selspws = [int(ispw) for ispw in selspws]
	  #inparams[srcnames[fid]]['spwids']= selspws if len(selspws)!=0 else range(nspw) 
	  inparams[fid]['spwids']= selspws if len(selspws)!=0 else range(nspw) 

	  #create a list of freq ranges with selected spws
	  # should worry about freq order???
	  freqlist=[]
          freqlistraw=[]
	  framelist=[]
	  #for idx in inparams[srcnames[fid]]['spwids']:
	  for idx in inparams[fid]['spwids']:
	    freqs=freqcol['r'+str(idx+1)]
	    freqws=freqwcol['r'+str(idx+1)]
	    fmeasref=fmeasrefcol[idx]
	 
	    #if scalebychan=T, this has to be min, max determined from
	    # chan_freq(channel center)+/- chan width.
	    if scalebychan:
              # pack into list of list of list (freqlist[nf][nspw])
	      perspwfreqlist=[]
	      for nf in range(len(freqs)):
		fl = freqs[nf][0]-freqws[nf][0]/2.
		fh = freqs[nf][0]+freqws[nf][0]/2.
		perspwfreqlist.append([fl,fh])
	      freqlist.append(perspwfreqlist)
	    else:
	      if (len(freqs)==1): 
		fl = freqs[0][0]-freqws[0][0]/2.
		fh = freqs[0][0]+freqws[0][0]/2.
		freqlist.append([float(fl),float(fh)])
	      else:
		freqlist.append([float(min(freqs)),float(max(freqs))])   
	 
	    framelist.append(measframes[fmeasref])
            freqlistraw.append(freqs.transpose()[0].tolist()) # data chanfreq list
	  #inparams[srcnames[fid]]['freqlist']=freqlist
	  #inparams[srcnames[fid]]['freqlistraw']=freqlistraw
	  #inparams[srcnames[fid]]['framelist']=framelist
	  #inparams[srcnames[fid]]['reffreqs']=reffreqs

	  inparams[fid]['freqlist']=freqlist
	  inparams[fid]['freqlistraw']=freqlistraw
	  inparams[fid]['framelist']=framelist
	  inparams[fid]['reffreqs']=reffreqs
          myms.close()

             
	# call Bryan's code
	# errcode: list of list - inner list - each values for range of freqs
	# flluxes: list of list 
	# fluxerrs:
	# size: [majoraxis, minoraxis, pa]
	# direction: direction for each time stamp
        # 
	#import solar_system_setjy as ss_setjy
	import solar_system_setjy as SSsetjy 
	retdict={} # for returning flux densities?
        ss_setjy=SSsetjy.solar_system_setjy()
	#for src in srcnames:
	for vfid in validfids:
          src=srcnames[vfid]
	  #mjds=inparams[src]['mjds']
	  mjds=inparams[vfid]['mjds']
	  fluxes=[]
          # call solar_system_fd() per spw (for scalebychan freqlist has an extra dimention)
          #nspwused=len(inparams[src]['freqlist'])
          nspwused=len(inparams[vfid]['freqlist'])

          # warning for many channels but it is really depends on the source
          if scalebychan:
            maxnf=0
            for ispw in range(nspwused):
              nf = len(inparams[src]['freqlist'][ispw])
              maxnf = max(nf,maxnf)
            if maxnf >= 3840 and src.upper()!="MARS": # mars shoulde be ok
              self._casalog.post("Processing %s spw(s) and at least some of them are a few 1000 channels or more. This may take \
                            many minutes (>3min per spw for 3840 channels) in some cases. Please be patient." % nspwused,"WARN")
              
	  for i in range(nspwused): # corresponds to n spw
	    if type(freqlist[0][0])==list:
	      #infreqs=inparams[src]['freqlist'][i]
	      infreqs=inparams[vfid]['freqlist'][i]
	    else:
	      #infreqs=[inparams[src]['freqlist'][i]]
	      infreqs=[inparams[vfid]['freqlist'][i]]
            self._casalog.post("Calling solar_system_fd: %s(%s) for spw%s freqs=%s" % (src, vfid,i,freqlist[i]),'DEBUG1')
	    (errcodes, subfluxes, fluxerrs, sizes, dirs)=\
               ss_setjy.solar_system_fd(source_name=src, MJDs=mjds, frequencies=infreqs, observatory=observatory, casalog=self._casalog)
            # for old code
	    #(errcodes, subfluxes, fluxerrs, sizes, dirs)=\
            #   ss_setjy.solar_system_fd(source_name=src, MJDs=mjds, frequencies=infreqs, casalog=self._casalog)
          # for nf freq ranges, nt mjds
          # errcodes[nf][nt], subfluxes[nf][nt], fluxerrs[nf][nt], sizes[nt],  dirs[nt]
            self._casalog.post("+++++ solar_system_fd() returned values +++++", 'DEBUG1')
            self._casalog.post(" fluxes(fds)=%s" % subfluxes, 'DEBUG1')
            self._casalog.post(" sizes=%s" % sizes, 'DEBUG1')
            self._casalog.post(" directions=%s\n" % dirs, 'DEBUG1')

          # packed fluxes for all spws 
	    fluxes.append(subfluxes)    
          # fluxes has fluxesi[nspw][nf][nt]

	  # ------------------------------------------------------------------------
	  # For testing with hardcoded values without calling solar_system_fd()...
	  #errcodes=[[0,0,0,0,0]]
	  #fluxes=[[26.40653147,65.23839313,65.23382757,65.80638802,69.33396562]]
	  #fluxerrs=[[0.0,0.0,0.0,0.0,0.0]]
	  #sizes=[[3.6228991032674371,3.6228991032674371,0.0]]
	  #dirs=[{'m0': {'unit': 'rad', 'value': 0.0}, 'm1': {'unit': 'rad', 'value': 0.0}, 'refer': 'J2000', 'type': 'direction'}]
	  # ------------------------------------------------------------------------
          # local params for selected src
	  #framelist=inparams[src]['framelist']
	  #freqlist=inparams[src]['freqlist']
	  #freqlistraw=inparams[src]['freqlistraw']
	  #reffreqs=inparams[src]['reffreqs']
	  #spwids=inparams[src]['spwids']

	  framelist=inparams[vfid]['framelist']
	  freqlist=inparams[vfid]['freqlist']
	  freqlistraw=inparams[vfid]['freqlistraw']
	  reffreqs=inparams[vfid]['reffreqs']
	  spwids=inparams[vfid]['spwids']

	  clrecs=odict.odict()
	  labels = []
	  # loop for over for multiple directions (=multiple  MJDs) for a given src
	  for i in range(len(dirs)):  # this is currently only length of 1 since no multiple timestamps were used
	    # check errcode - error code would be there per flux per time (dir)  
	    reterr=testerrs(errcodes[i],src) 
	    if reterr == 2: 
	      continue
	  
	    #dirstring = [dirs[i]['refer'],qa.tos(dirs[i]['m0']),qa.tos(dirs[i]['m1'])]
            if useephemdir:
	        dirstring = [dirs[i]['refer'],qa.tos(dirs[i]['m0']),qa.tos(dirs[i]['m1'])]
            else:
                #dirstring = [fieldref, str(fielddirs[fieldids[0]][0][0])+'rad', str(fielddirs[fieldids[0]][1][0])+'rad']
                # extract field direction of first id of the selected field ids
                #dirstring = [fieldref, "%.18frad" % (fielddirs[fieldids[0]][0][0]), "%.18frad" % (fielddirs[fieldids[0]][1][0])]
                dirstring = [fieldref, "%.18frad" % (fielddirs[vfid][0][0]), "%.18frad" % (fielddirs[vfid][1][0])]
            #print "dirstring=",dirstring

	    # setup componentlists
	    # need to set per dir
	    # if scalebychan=F, len(freqs) corresponds to nspw selected
            # Need to put in for-loop to create cl for each time stamp? or scan?

	    #clpath='/tmp/'
	    clpath='./'
            self.clnamelist=[]
            # construct cl name (multiple spws in one componentlist table)
            selreffreqs = [reffreqs[indx] for indx in spwids]
            minfreq = min(selreffreqs)
            maxfreq = max(selreffreqs)
	    freqlabel = '%.3f-%.3fGHz' % (minfreq/1.e9, maxfreq/1.e9)
	    #tmlabel = '%.1fd' % (tc/86400.)
	    mjd=inparams[vfid]['mjds'][0]
	    tmlabel = '%.1fd' % (mjd)
	    #clabel = src+'_spw'+str(spwids[j])+'_'+freqlabel+'_'+tmlabel
	    clabel0 = src+'_'+freqlabel+'_'+tmlabel
	    clname = clpath+clabel0+'.cl'
            #debug
            self._casalog.post("Create componentlist: %s for vfid=%s" % (clname,vfid),'DEBUG1')
	      
	    if (os.path.exists(clname)):
              shutil.rmtree(clname)

            iiflux=[]
            iinfreq=[]
            # for logging/output dict - pack each freq list for each spw
            freqsforlog=[]
	    for j in range(len(freqlist)): # loop over nspw
	      freqlabel = '%.3fGHz' % (reffreqs[int(spwids[j])]/1.e9)
              clabel=src+'_spw'+str(spwids[j])+'_'+freqlabel+'_'+tmlabel
	      #print "addcomponent...for spw=",spwids[j]," i=",i," flux=",fluxes[j][i]
	      if scalebychan:
		index= 2.0
		sptype = 'spectral index'
	      else:
		index= 0.0
		sptype = 'constant'
              # adjust to change in returned flux shape. An extra [] now seems to be gone. 2012-09-27
              iflux=fluxes[j][i][0]
              #print "addcomponent with flux=%s at frequency=%s" %\
              #                    (iflux,str(reffreqs[int(spwids[j])]/1.e9)+'GHz')

              # i - time stamps = 0 for now, j = a freq range
              infreq=freqlist[j][0][0] if type(freqlist[j][0])==list else freqlist[j][0]

              # for a single CL with multple spws
              if j ==0: infreq0=infreq 

              # --- version for 'multi-spws in a single comp single CL'
              # accumulate all frequencies from all spws to a list
              #if not scalebychan:
                #freqlist[j] for j spw should contain a list with two freqs (min and max of
                # of the spw and the same fluxes should be set for the freqs so that cl.setspectrum
                # with tabular would not interpolate inside each spw 
                ##iinfreq.extend(freqlist[j])
              #  nch = len(freqlistraw[j])
              #  iinfreq.extend(freqlistraw[j])
                # assigning the same flux to the two freqs
                ##iiflux.extend([iflux,iflux])
              #  for ich in range(nch):
              #    iiflux.extend([iflux])
              #else:
              #  iiflux.extend(fluxes[j][i])
	      #  if type(freqlist[j][0])==list and len(freqlist[j][0])>1:
	      #	  for fr in freqlist[j]:
	      #	    iinfreq.append((fr[1]+fr[0])/2)
	      #  else:
              #    if type(freqlist[j])==list:
	      #	    iinfreq.append((freqlist[j][1]+freqlist[j][0])/2)
              #    else:
	      #      iinfreq.append(freqlist[j])
              #  freqsforlog.append(iinfreq)
              # -----------------------------------------------------------------------------------
              
              self._casalog.post("addcomponent with flux=%s at frequency=%s" %\
              #                    (fluxes[j][i][0],str(reffreqs[int(spwids[j])]/1.e9)+'GHz'), 'INFO1')
                                  (iflux,str(reffreqs[int(spwids[j])]/1.e9)+'GHz'), 'INFO1')
            
              #  
	      mycl.addcomponent(iflux,fluxunit='Jy', polarization="Stokes", dir=dirstring,
              		 shape='disk', majoraxis=str(sizes[i][0])+'arcsec', minoraxis=str(sizes[i][1])+'arcsec', 
                        positionangle=str(sizes[i][2])+'deg', freq=[framelist[0],str(infreq)+'Hz'], 
	      		 spectrumtype=sptype, index=index, label=clabel)

              if scalebychan:
                # use tabular form to set flux for each channel
                
                # This may be redundant 
                if type(fluxes[j][i]) ==list and len(fluxes[j][i])> 1:
                  if type(freqlist[j][0])==list and len(freqlist[j][0])>1:
                    freqs=[]
                    for fr in freqlist[j]:
                      freqs.append((fr[1]+fr[0])/2)
                    clind = mycl.length() - 1
                    mycl.setspectrum(which=clind, type='tabular', tabularfreqs=freqs, tabularflux=fluxes[j][0],
	                             tabularframe=framelist[j])
            ### === per spw process end here

          # - version that put all spws into a component -
          # set a single component freq set at the the lower edge of first spw 
	  #mycl.addcomponent(iiflux[0],fluxunit='Jy', polarization="Stokes", dir=dirstring,
	  #		 shape='disk', majoraxis=str(sizes[i][0])+'arcsec', minoraxis=str(sizes[i][1])+'arcsec', 
	  #	         positionangle=str(sizes[i][2])+'deg', freq=[framelist[0],str(infreq0)+'Hz'], 
	  #		 spectrumtype=sptype, index=index, label=clabel)
          # if it's list of fluxes try to put in tabular form
	  #if type(fluxes[j][i]) ==list and len(fluxes[j][i])> 1:
	#  if len(iiflux)> 1:
	        #print "framelist[j]=",framelist[j]
	        #if type(freqlist[j][0])==list and len(freqlist[j][0])>1:
		#  freqs=[]
		#  for fr in freqlist[j]:
		#    freqs.append((fr[1]+fr[0])/2)
		#else:
		#  freqs=freqlist[j]
                #clind = mycl.length() - 1
	#	mycl.setspectrum(which=clind, type='tabular', tabularfreqs=freqs, tabularflux=fluxes[j][0],
			   #tabularframe=framelist[j])

        #    mycl.setspectrum(which=clind, type='tabular', tabularfreqs=iinfreq, tabularflux=iiflux,
	#                     tabularframe=framelist[0])
	      #mycl.rename(clname)
	      #put in a record for log output
	      #clrecs[clabel] = mycl.torecord()
        
            clrecs[clabel0] = mycl.torecord()
            clrecs[clabel0]['allfluxinfo']=fluxes
            clrecs[clabel0]['allfreqinfo']=freqsforlog
            clrecs[clabel0]['spwids']=spwids
	    mycl.rename(clname) #  - A CL per field
            mycl.close(False) # False for not to send a warning message
	    mycl.done()

              # if scratch=F check if the virtual model already exist
              # for the field if it is clear it.
            #  if j==0 and (not usescratch): 
              #  mytb.open(self.vis, nomodify=False)
              #  kwds = mytb.getkeywords()
              #  modelkwd='definedmodel_field_'+str(vfid)
              #  if kwds.has_key(modelkwd):
              #    clmodname=kwds[modelkwd]
              #    mytb.removekeyword(clmodname)
              #    mytb.removekeyword(modelkwd)
              #  mytb.close()
             
            mycb.open(self.vis,addcorr=False,addmodel=False)
            mycb.delmod(otf=True,field=str(vfid),spw=spwids, scr=False)
            mycb.close()

	      # finally, put the componentlist as model
              #tqlstr=''
              #if intent!='':
              #   tqlstr='any(STATE_ID=='+str(stateids.tolist())+')'
            # Currently still need to do per spw (see the comment below...)
            # assuming CL contains nrow = nspw components
            mycl.open(clname)
            clinrec = mycl.torecord()
            mycl.close(False)
            mycl.done()
            ncomp = clinrec['nelements']
            if ncomp != len(spwids):
               raise Exception, "Inconsistency in generated componentlist...Please submit a bug report."
            for icomp in range(ncomp): 
	      #self.im.selectvis(spw=spwids[icomp],field=field,observation=observation,time=timerange,intent=intent)
	      self.im.selectvis(spw=spwids[icomp],field=vfid,observation=observation,time=timerange,intent=intent)
              newclinrec = {}
              newclinrec['nelements']=1
              newclinrec['component0']=clinrec['component'+str(icomp)]
              mycl.fromrecord(newclinrec)
              if os.access("/tmp",os.W_OK):
                tmpclpath = "/tmp/"
              elif os.access("./",os.W_OK):
                tmpclpath = "./"
              tmpclpath=tmpclpath+"_tmp_setjyCLfile"
              if os.path.exists(tmpclpath):
                 shutil.rmtree(tmpclpath)
              mycl.rename(tmpclpath)
              mycl.done()
	      self.im.ft(complist=tmpclpath)

            # --------------------------------------------------------------------------------------------
            # Currently this does not work properly for some case. Need ft can handle multi-component CL
            # with the way to map which component apply to which spw
            # Unable when such functionality is implemented in im.ft()
	    #self.im.selectvis(spw=spwids,field=field,observation=observation,time=timerange,intent=intent)
	    #self.im.ft(complist=clname) <= e.g. im.ft(comprec) where comrec = {'spw-mapping':[...],comprecs}..
            # --------------------------------------------------------------------------------------------

              #debug: set locally saved 2010-version component list instead
              #cl2010='mod_setjy_spw0_Titan_230.543GHz55674.1d.cl'
              #print "Using complist=",cl2010
	      #self.im.ft(complist=cl2010)

	      #if cleanupcomps:          
              # 		  shutil.rmtree(clname)
              #self.clnamelist.append(clname)
            self.clnamelist.append(clname)

	  msg="Using channel dependent " if scalebychan else "Using spw dependent "
       
	  self._casalog.post(msg+" flux densities")
	  #self._reportoLog(clrecs,self._casalog)
	  self._reportoLog2(clrecs,self._casalog)
	  #self._updateHistory(clrecs,self.vis)
	  self._updateHistory2(clrecs,self.vis)
          # dictionary of dictionary for each field
          retdict[vfid]=clrecs 
        # ==== end of for loop over fields		 
        #output=self._makeRetFluxDict(retdict)
        output=self._makeRetFluxDict2(retdict)
	#return retval
	return output

    def getclnamelist(self):
        return self.clnamelist

    def _reportoLog(self,clrecs,casalog):
	"""
	send model parameters to log
	"""
	#print "clrecs=", clrecs
	for ky in clrecs.keys():
	    # expect only one component for each
            nelm = clrecs[ky]['nelements']
            for icomp in range(nelm):
	      comp = clrecs[ky]['component'+str(icomp)]
              complab = comp['label']
	      #srcn = ky.split('_')[0]
	      #ispw = ky.split('_')[1]
	      srcn = complab.split('_')[0]
	      ispw = complab.split('_')[1]
              if icomp==0:
                msg=" direction set in the componentlist: RA=%s rad, Dec%s rad" %\
                   (float('%.6g' % comp['shape']['direction']['m0']['value']),
                    float('%.6g' % comp['shape']['direction']['m1']['value']))
                casalog.post(msg,'INFO2')

              msg=" %s: %s Flux:[I=%s,Q=%s,U=%s,V=%s] +/- [I=%s,Q=%s,U=%s,V=%s] Jy" %\
                 (srcn, ispw, float('%.5g' % comp['flux']['value'][0]),
                  float('%.5g' % comp['flux']['value'][1]),
                  float('%.5g' % comp['flux']['value'][2]),
                  float('%.5g' % comp['flux']['value'][3]),
                  float('%.5g' % comp['flux']['error'][0]),
                  float('%.5g' % comp['flux']['error'][1]),
                  float('%.5g' % comp['flux']['error'][2]),
                  float('%.5g' % comp['flux']['error'][3]))
	      casalog.post(msg, 'INFO')

    def _reportoLog2(self,clrecs,casalog):
        """
        send model parameters to log
        (This version handles for a single componentlist with possible multiple componet)
        Assume componentlist has a name src_spw_xxxx or src_spw
        """
        nelm = -1
        ncl = len(clrecs.keys())
        if ncl == 1:
            clname = clrecs.keys()[0] 
            comprec = clrecs[clname]
            if comprec.has_key('nelements'):
                nelm = comprec['nelements']
        
        if ncl != 1 or nelm == -1:
            casalog.post("Cannot process the generated componentlist, possibly a bug.\
                         Please submit a bug report", "SEVERE")
       
        for icomp in range(nelm):
            comp = comprec['component'+str(icomp)]
            complab = comp['label']
            srcn = complab.split('_')[0]
            if icomp==0:
                msg=" direction set in the componentlist: RA=%s rad, Dec%s rad" %\
                     (float('%.6g' % comp['shape']['direction']['m0']['value']),
                      float('%.6g' % comp['shape']['direction']['m1']['value']))
                casalog.post(msg,'INFO2')
            if nelm==1:
                for i in range(len(comprec['spwids'])):
                    ispw=comprec['spwids'][i]
                    iflux = comprec['allfluxinfo'][i][0][0]
                    # currently only I flux is reported and no flux error is reported
                    msg=" %s: spw%s Flux:[I=%s,Q=%s,U=%s,V=%s] +/- [I=%s,Q=%s,U=%s,V=%s] Jy" %\
                         (srcn, ispw, float('%.5g' % iflux),0.0,0.0,0.0,
                          0.0,0.0,0.0,0.0)
                    casalog.post(msg, 'INFO')
            else:
                ispw = complab.split('_')[1]
                msg=" %s: %s Flux:[I=%s,Q=%s,U=%s,V=%s] +/- [I=%s,Q=%s,U=%s,V=%s] Jy" %\
                     (srcn, ispw, float('%.5g' % comp['flux']['value'][0]),
                      float('%.5g' % comp['flux']['value'][1]),
                      float('%.5g' % comp['flux']['value'][2]),
                      float('%.5g' % comp['flux']['value'][3]),
                      float('%.5g' % comp['flux']['error'][0]),
                      float('%.5g' % comp['flux']['error'][1]),
                      float('%.5g' % comp['flux']['error'][2]),
                      float('%.5g' % comp['flux']['error'][3]))
                casalog.post(msg, 'INFO')

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

    def _updateHistory2(self,clrecs,vis):
        """
        Update history table when setSolarObjectJy is run
        (mulitple comopents in one version)
        """
        #
        from taskinit import gentools
        (mytb,) = gentools(['tb'])
        mytb.open(vis+'/HISTORY',nomodify=False)
        nrow = mytb.nrows()
        lasttime=mytb.getcol('TIME')[nrow-1]
        rown=nrow
        #
        clname = clrecs.keys()[0]

        #for ky in clrecs.keys():
        srcn = clname.split('_')[0]
        comprec = clrecs[clname]
        nelm = comprec['nelements']
        for icomp in range(nelm): 
            comp = comprec['component'+str(icomp)]
            complab = comp['label']
            origin='setjy'
            priority='INFO'
            time=lasttime
            appl='setjy'
            emptystrarr=numpy.array([''])
            if nelm==1:
              for i in range(len(comprec['spwids'])):
                mytb.addrows(1)
                ispw=comprec['spwids'][i]
                iflux = comprec['allfluxinfo'][i][0][0]
                msg = (" %s: spw %s Flux:[I=%s,Q=%s,U=%s,V=%s] +/- [I=%s,Q=%s,U=%s,V=%s] Jy" %
                       (srcn, ispw, iflux, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0))
                mytb.putcell('APP_PARAMS', rown, [''])
                mytb.putcell('CLI_COMMAND',rown, [''])
                mytb.putcell('APPLICATION',rown, appl)
                mytb.putcell('MESSAGE',rown, msg)
                mytb.putcell('OBSERVATION_ID',rown, -1)
                mytb.putcell('ORIGIN',rown,origin)
                mytb.putcell('PRIORITY',rown,priority)
                mytb.putcell('TIME',rown, time)
                rown += 1
              
            else: 
                ispw = complab.split('_')[1]
                mytb.addrows(1)
                msg = (" %s: spw %s Flux:[I=%s,Q=%s,U=%s,V=%s] +/- [I=%s,Q=%s,U=%s,V=%s] Jy" %
                       (srcn, ispw, comp['flux']['value'][0], comp['flux']['value'][1],
                        comp['flux']['value'][2], comp['flux']['value'][3],
                        comp['flux']['error'][0], comp['flux']['error'][1],comp['flux']['error'][2],
                        comp['flux']['error'][3]))

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

    def _makeRetFluxDict(self, flxdict):
        """
        re-arrange the calculated flux density info for returned dict. 
        """
        # flxdict should contains a ditionary per field
        retflxdict={}
        for fid in flxdict.keys():
            comp=flxdict[fid]
            tmpdict={}
            for ky in comp.keys():
	        srcn = ky.split('_')[0]
	        ispw = ky.split('_')[1].strip('spw')
                
                tmpdict[ispw]={}
                tmpdict[ispw]['fluxd']=comp[ky]['component0']['flux']['value']
                #tmpdict[ispw]['fluxderr']=comp[ky]['component0']['flux']['error']
            tmpdict['fieldName']=srcn
            retflxdict[str(fid)]=tmpdict
        retflxdict['format']=\
          '{field Id: {spw Id: {fluxd:[I,Q,U,V] in %s}, \'fieldName\':field name}}' % \
          comp[ky]['component0']['flux']['unit']
        return retflxdict

    def _makeRetFluxDict2(self, flxdict):
        """
        re-arrange the calculated flux density info for returned dict.
        (for each field id as key, single dict)
        """
        # flxdict should contains a ditionary per field
        retflxdict={}
        for fid in flxdict.keys():
            clrecs=flxdict[fid]
            clname=clrecs.keys()[0]
	    srcn = clname.split('_')[0]
            comprec=clrecs[clname]
            nelm = comprec['nelements']
            tmpdict={}
            for icomp in range(nelm): 
                comp = comprec['component'+str(icomp)]
                complab = comp['label']
                if nelm==1:
                    for i in range(len(comprec['spwids'])):
                      ispw = str(comprec['spwids'][i])
                      iflux = comprec['allfluxinfo'][i][0][0]
                      tmpdict[ispw]={}
                      tmpdict[ispw]['fluxd']=[iflux,0.0,0.0,0.0]
                else:
                    ispw = complab.split('_')[1].strip('spw')
                    tmpdict[ispw]={}
                    #tmpdict[ispw]['fluxd']=comp[ky]['component0']['flux']['value']
                    tmpdict[ispw]['fluxd']=comp['flux']['value']
            tmpdict['fieldName']=srcn
            retflxdict[str(fid)]=tmpdict
        retflxdict['format']=\
          '{field Id: {spw Id: {fluxd:[I,Q,U,V] in %s}, \'fieldName\':field name}}' % \
          comprec['component0']['flux']['unit']
        return retflxdict


def testerrs(errcode,srcname):
    """
    Check error codes from solar_system_fd
    return = 0 all ok
    return = 1 partly ok
    return = 2 all bad - should not proceed to set component
    """
    from taskinit import casalog 
    errcount = 0
    if type(errcode)!=list: 
      errcode=[errcode]
    for ec in errcode:
      if ec != 0:
        errcount += 1
      if ec == 1:
         casalog.post("The model for %s is not supported" % srcname, 'WARN')
      elif ec == 2:
         casalog.post("Unsupported frequency range",'WARN')
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
