import sys
import os
import inspect
import string
import time
from taskinit import *
import viewertool


class __msview_class(object):
	"msview() task with local state for created viewer tool"

	def __init__( self ):
		self.local_vi = None
		self.local_ving = None

	def __call__(self, infile=None,displaytype=None,channel=None,zoom=None,outfile=None,outscale=None,outdpi=None,outformat=None,outlandscape=None,gui=None):
		""" The msview will display measurement sets in raster form
		Many display and editing options are available.

		examples of usage:

		msview
		msview "mymeasurementset.ms"
		msview "myrestorefile.rstr"
		
		Executing the msview task will bring up a display panel
		window, which can be resized.  If no data file was specified,
		a Load Data window will also appear.  Click on the desired data
		file and choose the display type; the rendered data should appear
		on the display panel.

		A Data Display Options window will also appear.  It has drop-down
		subsections for related	options, most of which are self-explanatory.
	  
		The state of the msview task -- loaded data and related display
		options -- can be saved in a 'restore' file for later use.
		You can provide the restore filename on the command line or
		select it from the Load Data window.

		See the cookbook for more details on using the msview task.
	
		Keyword arguments:
		infile -- Name of file to visualize
			default: ''
			example: infile='my.ms'
			If no infile is specified the Load Data window
			will appear for selecting data.
		displaytype -- (optional): method of rendering data
			visually (raster, contour, vector or marker).  
			You can also set this parameter to 'lel' and
			provide an lel expression for infile (advanced).
			default: 'raster'

		Note: there is no longer a filetype parameter; typing of
		data files is now done automatically.
	        	example:  msview infile='my.ms'
			obsolete: msview infile='my.ms', filetype='ms'


		"""
		a=inspect.stack()
		stacklevel=0
		for k in range(len(a)):
			if a[k][1] == "<string>" or (string.find(a[k][1], 'ipython console') > 0 or string.find(a[k][1],"casapy.py") > 0):
				stacklevel=k

		myf=sys._getframe(stacklevel).f_globals

		#Python script
		try:
			## vi might not be defined in taskinit if loading
			## directly from python via casa.py...
			vwr = vi
			if type(gui) == bool and gui == False:
				vwr = ving

			if type(vwr.cwd( )) != str:
				vwr = None
		except:
			vwr = None

		if type(vwr) == type(None):
			need_gui = True
			if type(gui) == bool and gui == False:
				need_gui = False

			if need_gui :
				if self.local_vi is not None:
					vwr = self.local_vi
				else:
					vwr = viewertool.viewertool( True, True, (type(myf) == dict and myf.has_key('casa') and type(myf['casa']) == type(os)) )
					self.local_vi = vwr
			else:
				if self.local_ving is not None:
					vwr = self.local_ving
				else:
					vwr = viewertool.viewertool( False, True, (type(myf) == dict and myf.has_key('casa') and type(myf['casa']) == type(os)) )
					self.local_ving = vwr

		if type(vwr) != type(None) :
			##
			## (1) save current *viewer*server* path
			## (2) have viewer() task follow casapy/python's cwd
			try:
				old_path = vwr.cwd( )
			except:
				raise Exception, "msview() failed to get the current working directory"

			try:
				vwr.cwd(os.path.abspath(os.curdir))
			except:
				raise Exception, "msview() failed to change to the new working directory"
				
			data = None
			if type(infile) == str and len(infile) > 0 :
				info = vwr.fileinfo(infile);
				if info['type'] != 'ms' :
					if info['type'] == 'image' :
						raise Exception, "msview() only displays images, try 'imview()'..."
					elif info['type'] == 'nonexistent' :
						raise Exception, "ms (" + infile + ") could not be found..."
					else :
						raise Exception, "unknow error..."

				panel = vwr.panel("viewer")
				if type(displaytype) == str:
					data = vwr.load( infile, displaytype, panel=panel )
				else:
					data = vwr.load( infile, panel=panel )

				if type(channel) == int and channel > 0 :
					vwr.channel(channel,panel=panel)
				if type(zoom) == int and zoom != 1 :
					vwr.zoom(zoom,panel=panel)
				if type(outfile) == str and len(outfile) > 0 :
					scale=1.0
					if type(outscale) == float :
						scale=outscale
					dpi=300
					if type(outdpi) == int :
						dpi=outdpi
					format="jpg"
					if type(outformat) == str :
						format=outformat
					orientation="portrait"
					if type(outlandscape) == bool and outlandscape :
						orientation="landscape"
					vwr.output(outfile,scale=scale,dpi=dpi,format=format,orientation=orientation,panel=panel)
			else:
				panel = vwr.panel("viewer")
				vwr.popup( 'open', panel=panel )


			# it makes no sense to leave a panel open with no way of interacting with it
			if type(gui) == bool and not gui:
				vwr.close(panel)

			## (3) restore original path
			try:
				vwr.cwd(old_path)
			except:
				raise Exception, "msview() failed to restore the old working directory"

		else:
			viewer_path = myf['casa']['helpers']['viewer']   #### set in casapy.py
			args = [ viewer_path ]

			if type(infile) == str:
				if type(displaytype) == str:
					args += [ infile, displaytype ]
				else:
					args += [ infile ]

			if (os.uname()[0]=='Darwin'):
				vwrpid=os.spawnvp( os.P_NOWAIT, viewer_path, args )
			elif (os.uname()[0]=='Linux'):
				vwrpid=os.spawnlp( os.P_NOWAIT, viewer_path, *args )
			else:
				print 'Unrecognized OS: No msview available'

		return None

msview = __msview_class( )
