from asap import rcParams
from asap import _n_bools, mask_and, mask_or
from asap.scantable import scantable

class interactivemask:
	"""
	The class for interactive mask selection.

	Example:
	   my_mask=interactivemask(plotter,scan)
	   my_mask.set_basemask(masklist=[[0,10],[90,100]],invert=False)
	   # Do interactive mask selection
	   my_mask.select_mask()
	   finish=raw_input('Press return to finish selection.\n')
	   my_mask.finish_selection(callback=func)
	   mask=my_mask.get_mask()
	   
	Modify mask region by selecting a region on a plot with mouse. 
	"""

	def __init__(self,plotter=None, scan=None):
		"""
		Create a interactive masking object.
		Either or both 'plotter' or/and 'scan' should be defined. 

		Parameters:
		   plotter: an ASAP plotter object for interactive selection
		   scan: a scantable to create a mask interactively
		"""
		# Return if GUI is not active
		if not rcParams['plotter.gui']:
			print 'GUI plotter is disabled.\n'
			print 'Exit interactive mode.'
			return
		# Verify input parameters
		if scan is None and plotter is None:
			msg = "Either scantable or plotter should be defined."
			raise TypeError(msg)

		self.scan=None
		self.p=None
		self.newplot=False
		if scan and isinstance(scan, scantable):
			self.scan=scan
		from asap.asapplotter import asapplotter
		if plotter and isinstance(plotter,asapplotter):
			self.p = plotter
			if self.scan is None and isinstance(self.p._data,scantable):
				self.scan=self.p._data
		if self.scan is None:
			msg = "Invalid scantable."
			raise TypeError(msg)

		self.mask=_n_bools(self.scan.nchan(),True)
		self.callback=None
		self.event=None
		self.once=False
		self.showmask=True
		self.rect={}
		self.xold=None
		self.yold=None
		self.xdataold=None
		self.ydataold=None
		self._polygons=[]
	

	def set_basemask(self,masklist=[],invert=False):
		"""
		Set initial channel mask.
		
		Parameters:
		    masklist:  [[min, max], [min2, max2], ...]
		               A list of pairs of start/end points (inclusive) 
                               specifying the regions to be masked
		    invert:    optional argument. If specified as True,
		               return an inverted mask, i.e. the regions
			       specified are excluded
		You can reset the mask selection by running this method with
		the default parameters. 
		"""
		# Verify input parameters
		if not (isinstance(masklist, list) or isinstance(masklist, tuple)) \
		   or not isinstance(invert, bool):
			msg = 'Invalid mask definition'
			raise TypeError(msg)

		# Create base mask
		if ( len(masklist) > 0 ):
			self.mask=self.scan.create_mask(masklist,invert=invert)
                elif invert==True:
			self.mask=_n_bools(self.scan.nchan(),False)
		else:
			self.mask=_n_bools(self.scan.nchan(),True)


	def set_startevent(self,event):
		"""
		Inherit an event from the parent function.
		
		Parameters:
		    event: 'button_press_event' object to be inherited to
		           start interactive region selection . 
		"""
		from matplotlib.backend_bases import MouseEvent
		if isinstance(event,MouseEvent) and event.name=='button_press_event': 
			self.event=event
		else:
			msg="Invalid event."
			raise TypeError(msg)	

	def set_callback(self,callback):
		"""
		Set callback function to run when finish_selection() is executed. 
		    callback: The post processing function to run after
		              the mask selections are completed.
			      This will be overwritten if callback is defined in
			      finish_selection(callback=func)
		"""
		self.callback=callback

	def select_mask(self,once=False,showmask=True):
		"""
		Do interactive mask selection.
		Modify masks interactively by adding/deleting regions with
		mouse drawing.(left-button: mask; right-button: UNmask) 
		Note that the interactive region selection is available only 
		when GUI plotter is active. 

		Parameters:
		    once:     If specified as True, you can modify masks only
		              once. Else if False, you can modify them repeatedly. 
		    showmask: If specified as True, the masked regions are plotted
		              on the plotter.
			      Note this parameter is valid only when once=True.
			      Otherwise, maskes are forced to be plotted for reference. 
		"""
		# Return if GUI is not active
		if not rcParams['plotter.gui']:
			print 'GUI plotter is disabled.\n'
			print 'Exit interactive mode.'
			return

		self.once = once
		if self.once:
			self.showmask=showmask
		else:
			if not showmask: print 'Warning: showmask spcification is ignored. Mask regions are plotted anyway.'
			self.showmask=True

		#if not self.p._plotter or self.p._plotter.is_dead:
		if not self.p or self.p._plotter.is_dead:
			print 'A new ASAP plotter will be loaded'
			from asap.asapplotter import asapplotter
			self.p=asapplotter()
			self.newplot=True

                # Plot selected spectra if needed
		if self.scan != self.p._data:
			# Need replot
			self.p.plot(self.scan)
			# disable casa toolbar
			if self.p._plotter.figmgr.casabar:  self.p._plotter.figmgr.casabar.disable_button()
			for panel in self.p._plotter.subplots:
				xmin, xmax = panel['axes'].get_xlim()
				marg = 0.05*abs(xmax-xmin)
				panel['axes'].set_xlim(xmin-marg, xmax+marg)
				if rcParams['plotter.ganged']: break
			self.p._plotter.show()

		# Plot initial mask region
		#if self.showmask or not self.once:
		if self.showmask:
			self._plot_mask()
			print ''
			print 'Selected regions are shaded with yellow. (gray: projections)'
			print 'Now you can modify the selection.'
			print 'Draw rectangles with Left-mouse to add the regions,'
			print 'or with Right-mouse to exclude the regions.'


		if self.event != None:
			self._region_start(self.event)
		else: 
			self.p._plotter.register('button_press',None)
			self.p._plotter.register('button_press',self._region_start)


	def _region_start(self,event):
		# Do not fire event when in zooming/panning mode
		mode = self.p._plotter.figmgr.toolbar.mode
		if not mode =='':
			return
		# Return if selected point is out of panel
		if event.inaxes == None: return
		# Select mask/unmask region with mask
		self.rect = {'button': event.button, 'axes': event.inaxes,
			     'x': event.x, 'y': event.y,
			     'world': [event.xdata, event.ydata,
				       event.xdata, event.ydata],
			     'pixel': [event.x, event.y,
				       event.x, event.y]}
		### Start mod: 2009/08/17 kana ###
		#self._default_motion('stop')
		### End mod ######################
		self.p._plotter.register('motion_notify', self._region_draw)
		self.p._plotter.register('button_release', self._region_end)

	def _region_draw(self,event):
		sameaxes=(event.inaxes == self.rect['axes'])
		if sameaxes: 
			xnow=event.x
			ynow=event.y
			self.xold=xnow
			self.yold=ynow
			self.xdataold=event.xdata
			self.ydataold=event.ydata
		else:
			xnow=self.xold
			ynow=self.yold

		self.p._plotter.figmgr.toolbar.draw_rubberband(event, xnow, ynow, self.rect['x'], self.rect['y'])
		### Start mod: 2009/08/17 kana ###
		#self.p._plotter.figmgr.toolbar.mouse_move(event)
		### End mod ######################

	def _region_end(self,event):
		self.p._plotter.register('motion_notify', None)
		### Start mod: 2009/08/17 kana ###
		#self._default_motion('start')
		### End mod ######################
		self.p._plotter.register('button_release', None)

		self.p._plotter.figmgr.toolbar.release(event)
                
		if event.inaxes == self.rect['axes']: 
			xend=event.x
			yend=event.y
			xdataend=event.xdata
			ydataend=event.ydata
		else:
			xend=self.xold
			yend=self.yold
			xdataend=self.xdataold
			ydataend=self.ydataold
			
		self.rect['world'][2:4] = [xdataend, ydataend]
		self.rect['pixel'][2:4] = [xend, yend]
		self._update_mask()
		# Clear up region selection
		self.rect={}
		self.xold=None
		self.yold=None
		self.xdataold=None
		self.ydataold=None
		if self.once: self.finish_selection(callback=self.callback)

	def _update_mask(self):
		# Min and Max for new mask
		xstart=self.rect['world'][0]
		xend=self.rect['world'][2]
		if xstart <= xend: newlist=[xstart,xend]
		else: newlist=[xend,xstart]
		# Mask or unmask
		invmask=None
		if self.rect['button'] == 1:
			invmask=False
			mflg='Mask'
		elif self.rect['button'] == 3:
			invmask=True
			mflg='UNmask'
		print mflg+': ',newlist
		newmask=self.scan.create_mask(newlist,invert=invmask)
		# Logic operation to update mask
		if invmask:
			self.mask=mask_and(self.mask,newmask)
		else:
			self.mask=mask_or(self.mask,newmask)
		# Plot masked regions
		#if self.showmask or not self.once: self._plot_mask()
		if self.showmask: self._plot_mask()

	# Plot masked regions
	def _plot_mask(self):
                msks = []
                msks = self.scan.get_masklist(self.mask,row=0)
		# Get projection masks for multi-IF
		ifs=self.scan.getifnos()
		projs = []
		if len(ifs) > 1:
			row0if=self.scan.getif(0)
			for ifno in ifs:
				if ifno == row0if: continue
				for row in xrange(self.scan.nrow()):
					if self.scan.getif(row) == ifno:
						projs.append(self.scan.get_masklist(self.mask,row=row))
						break
                if len(self._polygons)>0:
			# Remove old polygons
			for polygon in self._polygons: polygon.remove()
			self._polygons=[]
		# Plot new polygons
		if len(msks) > 0:
			npanel=len(self.p._plotter.subplots)
			j=-1
			for iloop in range(len(msks)*npanel):
				i = iloop % len(msks)
				if  i == 0 : j += 1
				if len(ifs) > 1:
					for k in xrange(len(ifs)-1):
						self._polygons.append(self.p._plotter.subplots[j]['axes'].axvspan(projs[k][i][0],projs[k][i][1],facecolor='#aaaaaa'))			
				self._polygons.append(self.p._plotter.subplots[j]['axes'].axvspan(msks[i][0],msks[i][1],facecolor='yellow'))
		self.p._plotter.canvas.draw()

	### Start mod: 2009/08/17 kana ###
	def _default_motion(self,action):
		if not isinstance(action,str):
			print "WARN: Either 'stop' or 'start' is valid."
			return
		
		canvas=self.p._plotter.canvas
		toolbar=self.p._plotter.figmgr.toolbar
		if action == 'stop':
			if toolbar._idDrag:
				print "WARN: No default event. Nothing to be done."
				return
			canvas.mpl_disconnect(toolbar._idDrag)
			print "Disconnecting default motion event ", toolbar._idDrag
		elif action == 'start':
			if toolbar._idDrag:
				print "WARN: Default event already exists. Disconnecting the event."
				canvas.mpl_disconnect(toolbar._idDrag)
			toolbar._idDrag=canvas.mpl_connect('motion_notify_event',toolbar.mouse_move)
			print "Connecting default motion event ", toolbar._idDrag			
		else:
			print "WARN: Either 'stop' or 'start' is valid."

	### End mod ######################
		
	def finish_selection(self, callback=None):
		"""
		Execute callback function, reset or close plotter window as
		necessary.

		Parameters:
		    callback: The post processing function to run after
		              the mask selections are completed.
			      Specifying the callback function here will overwrite
			      the one set by set_callback(func)
		
		Note this function is automatically called at the end of
		select_mask() if once=True. 
		"""
		if callback: self.callback=callback
		if self.callback: self.callback()
		if not self.event: self.p._plotter.register('button_press',None)
		# Finish the plot
		if not self.newplot:
			self.clear_polygon()
		else: 
			self.p._plotter.unmap()
			self.p._plotter = None
			del self.p
			self.p=None
			self._polygons=[]


	def clear_polygon(self):
		"""
		Erase masks plots from the plotter. 
		"""
                if len(self._polygons)>0:
			# Remove old polygons
			for polygon in self._polygons: polygon.remove()
			self.p._plotter.show()
			self._polygons=[]


	def get_mask(self):
		"""
		Get the interactively selected channel mask.
		Returns:
		    A list of channel mask. 
		"""
		return self.mask


