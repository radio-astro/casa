from asap import rcParams
from asap import _n_bools, mask_and, mask_or
from asap.scantable import scantable

class interactivemask:
	"""
	The class for interactive mask selection.

	Example:
	   new_mask=interactivemask()
	   new_mask.select_mask(scan,masklist=[[0,10],[90,100]],invert=False)
	   mask=new_mask.get_mask()
	   
	Modify mask region by selecting a region on a plot with mouse. 
	"""

	def __init__(self):
		"""
		Create a interactive masking object
		"""
		self.scan=None
		self.rect={}
		self.xold=None
		self.yold=None
		self.xdataold=None
		self.ydataold=None
		self.mask=None
		self._polygons=[]
		self._p=None
	
	def select_mask(self,scan,masklist=[],invert=False):
		"""
		Do interactive mask selection.
		Calculate initial channel mask based on the parameters and modify
		it interactively by adding/deleting regions with mouse drawing.
		When finish modifying, press <Return> to calculate the final mask. 
                
		Parameters:
		    scan:      a scantable
		    masklist:  [[min, max], [min2, max2], ...]
		               A list of pairs of start/end points (inclusive) 
                               specifying the regions to be masked
		    invert:    optional argument. If specified as True,
		               return an inverted mask, i.e. the regions
			       specified are excluded

		Interactive region selection is available only when GUI plotter
		is active. When GUI plotter is disabled, this method only
		calculates a initial channel mask. 
		"""
		# Verify input parameters
		if not isinstance(scan, scantable):
			msg = 'Input is not a scantable'
			raise TypeError(msg)
		if not (isinstance(masklist, list) or isinstance(masklist, tuple)) \
		   or not isinstance(invert, bool):
			msg = 'Invalid mask definition'
			raise TypeError(msg)
		
		self.scan=scan

		# Create initial mask
		if ( len(masklist) > 0 ):
			self.mask=self.scan.create_mask(masklist,invert=invert)
                else:
			self.mask=_n_bools(self.scan.nchan(),True)

		# Return if GUI is not active
		if not rcParams['plotter.gui']:
			print 'GUI plotter is disabled.\n'
			print 'Exit interactive mode.'
			return

                # Plot selected spectra
		if not self._p or self._p.is_dead:
			from asap.asapplotter import asapplotter
			plotter=asapplotter()
		        self._p = plotter._plotter
                plotter.plot(self.scan)
		for panel in self._p.subplots:
			xmin, xmax = panel['axes'].get_xlim()
			marg = 0.05*(xmax-xmin)
			panel['axes'].set_xlim(xmin-marg, xmax+marg)
		self._p.show()
                self._plot_mask()

		print ''
		print 'Selected regions are shaded with yellow. (gray: projections)'
		print 'Now you can modify the selection.'
		print 'Draw rectangles with Left-mouse to add the regions,'
		print 'or with Right-mouse to exclude the regions.'
		
		cid = None
		cid = self._p.canvas.mpl_connect('button_press_event', self._region_start)
		finish=raw_input('Press return to calculate statistics.\n')
		if cid is not None:
			self._p.canvas.mpl_disconnect(cid)

		# Finish the plot
		self._p.unmap()
		self._p = None
		del plotter

	def _region_start(self,event):
		# Do not fire event when in zooming/panning mode
		mode = self._p.figmgr.toolbar.mode
		if not mode =='':
			return
		# Return if selected point is out of panel
		if event.inaxes == None: return
		# Select mask/unmask region with mask
		height = self._p.canvas.figure.bbox.height()
		self.rect = {'button': event.button, 'axes': event.inaxes,
			     'fig': None, 'height': height,
			     'x': event.x, 'y': height - event.y,
			     'world': [event.xdata, event.ydata,
				       event.xdata, event.ydata],
			     'pixel': [event.x, height - event.y,
				       event.x, height -event.y]}
		self._p.register('motion_notify', self._region_draw)
		self._p.register('button_release', self._region_end)

	def _region_draw(self,event):
		self._p.canvas._tkcanvas.delete(self.rect['fig'])
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
			
		self.rect['fig'] = self._p.canvas._tkcanvas.create_rectangle(
			self.rect['x'], self.rect['y'],
			xnow, self.rect['height'] - ynow)

	def _region_end(self,event):
		height = self._p.canvas.figure.bbox.height()
		self._p.register('motion_notify', None)
		self._p.register('button_release', None)
		
		self._p.canvas._tkcanvas.delete(self.rect['fig'])
                
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
		self.rect['pixel'][2:4] = [xend, height - yend]
		self._update_mask()

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
		self._plot_mask()

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
			npanel=len(self._p.subplots)
			j=-1
			for iloop in range(len(msks)*npanel):
				i = iloop % len(msks)
				if  i == 0 : j += 1
				if len(ifs) > 1:
					for k in xrange(len(ifs)-1):
						self._polygons.append(self._p.subplots[j]['axes'].axvspan(projs[k][i][0],projs[k][i][1],facecolor='#aaaaaa'))			
				self._polygons.append(self._p.subplots[j]['axes'].axvspan(msks[i][0],msks[i][1],facecolor='yellow'))
		self._p.canvas.draw()

	def get_mask(self):
		"""
		Get the interactively selected channel mask.
		Returns:
		    A list of channel mask. 
		"""
		return self.mask

