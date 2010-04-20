import os
from taskinit import *
from flagdata_cli import flagdata_cli as flagdata

def importevla(asdm=None, vis=None, singledish=None, antenna=None, corr_mode=None, srt=None, time_sampling=None, ocorr_mode=None, compression=None, asis=None, wvr_corrected_data=None, verbose=None, overwrite=None, showversion=None, applyflags=None, tbuff=None):
	""" Convert a Science Data Model (SDM) dataset into a CASA Measurement Set (MS)
	This version is under development and is geared to handling EVLA specific flag and
	system files, and is otherwise identical to importasdm.
	
	Keyword arguments:
	asdm -- Name of input SDM file (directory)
		default: none; example: asdm='ExecBlock3'

	"""
	#Python script
	try:
                casalog.origin('importevla')
		viso = ''
                if singledish:
                   ocorr_mode = 'ao'
                   corr_mode = 'ao'
                   casalog.post('corr_mode and ocorr_mode is forcibly set to ao.')
                   if compression:
                      casalog.post('compression=True has no effect for single-dish format.')
		if(len(vis) > 0) :
		   viso = vis
                   if singledish:
                      viso = vis.rstrip('/') + '.importevla.tmp.ms'
		else :
		   viso = asdm + '.ms'
		   vis = asdm
                   if singledish:
                      viso = asdm.rstrip('/') + '.importevla.tmp.ms'
                      vis = asdm.rstrip('/') + '.asap'
		execute_string='asdm2MS  --icm \"' +corr_mode + '\" --isrt \"' + srt+ '\" --its \"' + time_sampling+ '\" --ocm \"' + ocorr_mode + '\" --wvr-corrected-data \"' + wvr_corrected_data + '\" --asis \"' + asis + '\" --logfile \"' +casalog.logfile() +'\"'
		if(compression) :
		   execute_string= execute_string +' --compression'
		if(verbose) :
		   execute_string= execute_string +' --verbose'
		if(showversion) :
		   execute_string= execute_string +' --revision'
		if not overwrite and os.path.exists(viso) :
		   raise Exception, "You have specified and existing ms and have indicated you do not wish to overwrite it"
		execute_string = execute_string + ' ' + asdm + ' ' + viso
		casalog.post('Running the asdm2MS standalone invoked as:')
		#print execute_string
		casalog.post(execute_string)
        	os.system(execute_string)
		if compression :
                   #viso = viso + '.compressed'
                   viso = visover.rstrip('.ms') + '.compressed.ms'
                ok=fg.open(viso);
                ok=fg.saveflagversion('Original',comment='Original flags at import into CASA',merge='save')
                ok=fg.done();
                if singledish:
                   casalog.post('temporary MS file: %s'%viso)
                   casalog.post('        ASAP file: %s'%vis)
                   casalog.post(' selected antenna: %s'%antenna)
                   try:
                      from asap import scantable
                      s = scantable(viso,average=False,getpt=True,antenna=antenna)
                      s.save(vis,format='ASAP',overwrite=overwrite)
                      # remove intermediate products
                      if os.path.exists(viso):
                         os.system('rm -rf %s'%viso)
                         os.system('rm -rf %s.flagversions'%viso)
                   except Exception, instance:
                      if os.path.exists(viso):
                         os.system('rm -rf %s'%viso)
                         os.system('rm -rf %s.flagversions'%viso)  
                      if type(instance) == ImportError and (str(instance)).find('asap') != -1:
                         casalog.post(str(instance),'SEVERE')
                         casalog.post('You should build ASAP to be able to create single-dish data.','SEVERE')
                      else:
                         raise Exception, instance
		# Begin EVLA specific code here
		if os.access(asdm+'/Flag.xml',os.F_OK):
		   if os.access(asdm+'/Antenna.xml',os.F_OK):
		      print "  Found Antenna.xml in SDM, copying to MS"
		      casalog.post("Found Antenna.xml in SDM, copying to MS")
		      os.system('cp -rf '+asdm+'/Antenna.xml '+viso+'/')
	           else:
		      raise Exception, "Failed to find Antenna.xml in SDM"
	           #
		   print "  Found Flag.xml in SDM, copying to MS"
		   casalog.post("Found Flag.xml in SDM, copying to MS")
		   os.system('cp -rf '+asdm+'/Flag.xml '+viso+'/')
		   if applyflags:
		      # apply flags to data
		      # NOTE: should not call a task from within a task but
		      # will do so here, otherwise would have to steal code
		      # from flagdata to vectorize the flagging commands
		      try:
		         from xml.dom import minidom
		      except Exception, instance:
		         if os.path.exists(viso):
                            os.system('rm -rf %s'%viso)
			    os.system('rm -rf %s.flagversions'%viso)  
			 raise Exception, instance
			 print "*** Error *** failed to load xml.dom.minidom:\n", instance
		      xmlants = minidom.parse(viso+'/Antenna.xml')
		      antdict = {}
		      rowlist = xmlants.getElementsByTagName("row")
		      for rownode in rowlist:
		         rowname = rownode.getElementsByTagName("name")
			 ant = str(rowname[0].childNodes[0].nodeValue)
			 rowid = rownode.getElementsByTagName("antennaId")
			 id = str(rowid[0].childNodes[0].nodeValue)
			 antdict[id] = ant
		      print '  Found ',rowlist.length,' antennas in Antenna.xml'
		      casalog.post('Found %s antennas in Antenna.xml'%str(rowlist.length) )
		      #
		      # now read Flag.xml into dictionary also and make a list
		      xmlflags = minidom.parse(viso+'/Flag.xml')
		      flagdict = {}
		      flaglist = []
		      flagants = {}
		      rowlist = xmlflags.getElementsByTagName("row")
		      print '  Padding timerange by %s seconds'%str(tbuff)
		      casalog.post('Padding timerange by %s seconds'%str(tbuff) )
		      for rownode in rowlist:
		         rowfid = rownode.getElementsByTagName("flagId")
			 fid = str(rowfid[0].childNodes[0].nodeValue)
			 flagdict[fid] = {}
			 flaglist.append(fid)
			 rowid = rownode.getElementsByTagName("antennaId")
			 id = str(rowid[0].childNodes[0].nodeValue)
			 # start and end times in mjd ns
			 rowstart = rownode.getElementsByTagName("startTime")
			 start = int(rowstart[0].childNodes[0].nodeValue)
			 startmjd = (float(start)*1.0E-9 - tbuff)/86400.0
			 t = qa.quantity(startmjd,'d')
			 starttime = qa.time(t,form="ymd")
			 rowend = rownode.getElementsByTagName("endTime")
			 end = int(rowend[0].childNodes[0].nodeValue)
			 endmjd = (float(end)*1.0E-9 + tbuff)/86400.0
			 t = qa.quantity(endmjd,'d')
			 endtime = qa.time(t,form="ymd")
			 # Construct antenna name and timerange strings also
			 antname = antdict[id]
			 flagdict[fid]['antname'] = antname
			 cmd = starttime+'~'+endtime
			 flagdict[fid]['timerange'] = cmd
			 # Also build a per antenna string
			 if flagants.has_key(antname):
			    flagants[antname] += ','+cmd
		         else:
			    flagants[antname] = cmd
		      print '  Found ',rowlist.length,' flags in Flag.xml'
		      casalog.post('Found %s flags in Flag.xml'%str(rowlist.length) )
		      #
		      flags = {}
		      flags['antenna'] = []
		      flags['timerange'] = []
		      if rowlist.length > 0:
		         #
			 # Now flag the data
			 #
			 keylist = flagants.keys()
			 keylist.sort()
			 antenna = []
			 timerange = []
			 for ant in keylist:
			    if ocorr_mode=='co':
			       antenna.append(ant)
			    else:
			       # append &&* to catch auto-correlations
			       antenna.append(ant+'&&*')
			    timerange.append(flagants[ant])
			 # save the params in a dictionary
			 flags['antenna'] = antenna
			 flags['timerange'] = timerange
			 print 'Flagging data using Flag.xml entries'
			 flagdata(vis=viso,mode='manualflag',antenna=antenna,timerange=timerange)
		      else:
		         print 'No flagging found in Flag.xml'
			 casalog.post('No flagging found in Flag.xml')
		   else:
		      print 'Warning: will not be applying flags (applyflags=False)'
		      casalog.post('Will not be applying flags (applyflags=False)')
		else:
		   print 'No Flag.xml in SDM'
		   casalog.post('No Flag.xml in SDM')

	except Exception, instance:
		print '*** Error ***',instance

