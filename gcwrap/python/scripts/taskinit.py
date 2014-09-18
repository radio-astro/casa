import pCASA
from casac import *
import viewertool
import inspect
import string
import sys
import os


def __taskinit_setlogfile( logger ) :
	####
	#### needed to allow pushing of the global 'casa' state dictionary
	####
	a=inspect.stack()
	stacklevel=0
	for k in range(len(a)):
		if a[k][1] == "<string>":
			stacklevel=k

	myf=sys._getframe(stacklevel).f_globals

	if myf.has_key('casa') and myf['casa'].has_key('files') and myf['casa']['files'].has_key('logfile') :
		logger.setlogfile(myf['casa']['files']['logfile'])


def __taskinit_casa( ) :
	a=inspect.stack()
	stacklevel=0
	for k in range(len(a)):
		if a[k][1] == "<string>" or string.find(a[k][1], 'ipython console') > 0 or string.find(a[k][1],"casapy.py") > 0:
			stacklevel=k
			# jagonzal: Take the first level that matches the requirement
			break

	myf=sys._getframe(stacklevel).f_globals

	if myf.has_key('casa') :
		return myf['casa']
	else:
		return { }

#
##casa state...
casa = __taskinit_casa( )

#
##allow globals for taskby default
casaglobals=True
# setup available tools
imager = casac.imager
imtool=imager
calibrater = casac.calibrater
cbtool=calibrater
mstool = casac.ms
tptool = casac.tableplot
tp = tptool()
mptool = casac.msplot
mp = mptool()
pmtool = casac.plotms
pm = pmtool()
cptool = casac.calplot
cp = cptool()
qatool = casac.quanta
qa = casac.qa =  qatool()
tbtool = casac.table
#fgtool = casac.flagger
aftool = casac.agentflagger
af = aftool()
metool = casac.measures
iatool = casac.image
potool = casac.imagepol
smtool = casac.simulator
cltool = casac.componentlist
coordsystool = casac.coordsys
cstool = casac.coordsys
rgtool = casac.regionmanager
sltool = casac.spectralline
dctool = casac.deconvolver
vptool = casac.vpmanager
msmdtool = casac.msmetadata
fitool = casac.fitter
fntool = casac.functional
imdtool = casac.imagemetadata

utilstool = casac.utils
cu = casac.cu = utilstool()
vftask = casac.vlafillertask()
vlafiller=vftask.fill
at = casac.atmosphere()
ca = casac.calanalysis()
mttool = casac.mstransformer
mt = mttool()

from simple_cluster import simple_cluster
clustermanager = simple_cluster()

# Log initialization ###################################################################################################

# IMPORTANT: The following steps must be follow the described order, 
#            otherwise a seg fault occurs when setting the log file.
# 1st Create casalog object, it will be used by tasks when importing taskinit
casalog = casac.logsink()
# 2nd Set log file accessing CASA dictionary of calling context via stack inspection
__taskinit_setlogfile(casalog)
# 3rd Set logger as global
casalog.setglobal(True)

# Set processor origin (normally "casa" but in the MPI case we use the hostname and rank involved)
from mpi4casa.MPIEnvironment import MPIEnvironment
processor_origin = MPIEnvironment.processor_origin
casalog.processor_origin(processor_origin)

# Set showconsole to false for MPIServers
casalog.showconsole(MPIEnvironment.log_to_console)
 

# Log initialization ###################################################################################################

def gentools(tools=None):
	"""
	Generate a fresh set of tools the ones who's
	state can be funny
	im,cb,ms,tb,me,ia,po,sm,cl,cs,rg,sl,dc,vp,msmd,fi=gentools() 
	or if you want specific set of tools
	im, ia, cb=gentools(['im', 'ia', 'cb'])

	"""
	tooldic={'im':'imager()', 'cb' :'calibrater()', 'ms':'mstool()',
		 'tb':'tbtool()',  'me' :'metool()', 
		 'ia': 'iatool()', 'po':'potool()', 'sm' :'smtool()', 
		 'cl': 'cltool()', 'cs' :'cstool()', 'rg':'rgtool()',
		 'sl':'sltool()', 'dc':'dctool()', 'vp':'vptool()',
         'msmd':'msmdtool()','fi':'fitool()','fn':'fntool()', 'imd': 'imdtool()'}
	reqtools=[]
        if (not tools) or not hasattr(tools, '__iter__'):
		reqtools=['im', 'cb', 'ms','tb', 'me', 'ia', 'po',
                          'sm', 'cl', 'cs', 'rg','sl', 'dc', 'vp', 'msmd', 'fi', 'fn', 'imd']
	else:
		reqtools=tools
	return tuple([eval(tooldic[reqtool]) for reqtool in reqtools])

im,cb,ms,tb,me,ia,po,sm,cl,cs,rg,sl,dc,vp,msmd,fi,fn,imd=gentools()

def write_history(myms, vis, tname, param_names, param_vals, myclog=None, debug=False):
        """
        Update vis with the parameters that task tname was called with.

        myms - an ms tool instance
        vis  - the MS to write to.
        tname - name of the calling task.
        param_names - list of parameter names.
        param_vals - list of parameter values (in the same order as param_names).
        myclog - a casalog instance (optional)
        debug - Turns on debugging print statements on errors if True.

        Example:
        The end of split does
        param_names = split.func_code.co_varnames[:split.func_code.co_argcount]
        param_vals = [eval(p) for p in param_names]  # Must be done in the task.
        write_history(myms, outputvis, 'split', param_names, param_vals,
                      casalog),
        which appends, e.g.,
        
        vis = 'TWHydra_CO3_2.ms'
        outputvis   = 'scan9.ms'
        datacolumn  = 'data'
        field       = ''
        spw         = ''
        width       = 1
        antenna     = ''
        timebin     = '0s'
        timerange   = ''
        scan        = '9'
        intent      = ''
        array       = ''
        uvrange     = ''
        correlation = ''
        keepflags   = True
        async       = False

        to the HISTORY of outputvis.
        """
        if not hasattr(myms, 'writehistory'):
                if debug:
                        print "write_history(myms, %s, %s): myms is not an ms tool" % (vis, tname)
                return False
        retval = True
        isopen = False
        try:
                if not myclog and hasattr(casalog, 'post'):
                        myclog = casalog
        except Exception, instance:
                # There's no logger to complain to, and I don't want to exit
                # just because of that.
                pass
        try:
                myms.open(vis, nomodify=False)
                isopen = True
                myms.writehistory(message='taskname=%s' % tname, origin=tname)
                vestr = 'version: '
                try:
                        # Don't use myclog.version(); it also prints to the
                        # logger, which is confusing.
                        vestr += casa['build']['version'] + ' '
                        vestr += casa['source']['url'].split('/')[-2]
                        vestr += ' rev. ' + casa['source']['revision']
                        vestr += ' ' + casa['build']['time']
                except Exception, instance:
                        if hasattr(myclog, 'version'):
                                # Now give it a try.
                                vestr += myclog.version()
                        else:
                                vestr += ' could not be determined' # We tried.
                myms.writehistory(message=vestr, origin=tname)

                # Write the arguments.
                for argnum in xrange(len(param_names)):
                        msg = "%-11s = " % param_names[argnum]
                        val = param_vals[argnum]
                        if type(val) == str:
                                msg += '"'
                        msg += str(val)
                        if type(val) == str:
                                msg += '"'
                        myms.writehistory(message=msg, origin=tname)
        except Exception, instance:
                if hasattr(myclog, 'post'):
                        myclog.post("*** Error \"%s\" updating HISTORY of %s" % (instance, vis),
                                    'SEVERE')
                retval = False
        finally:
                if isopen:
                        myms.close()
        return retval        

###done with common tools

# setup viewer tool
# jagonzal (CAS-4322): Don't load viewer at the engine level
if not os.environ.has_key('CASA_ENGINE'):
	try : 
		if casa.has_key('state') and casa['state'].has_key('startup') :
			ving = viewertool.viewertool( False, pre_launch=casa['state']['startup'] )
			if casa['flags'].has_key('--nogui') :
				vi = ving
			else:
				vi = viewertool.viewertool( True, pre_launch=casa['state']['startup'] )
	except :
		print "Unable to start viewer, maybe no dbus available?"

defaultsdir = {}
defaultsdir['alma'] = 'file:///'+os.environ.get('CASAPATH').split()[0]+'/'+os.environ.get('CASAPATH').split()[1]+'/xml/almadefaults.xml'
defaultsdir['evla'] = 'file:///'+os.environ.get('CASAPATH').split()[0]+'/'+os.environ.get('CASAPATH').split()[1]+'/xml/evladefaults.xml'


def selectfield(vis,minstring):
        """Derive the fieldid from  minimum matched string(s): """

        tb.open(vis+'/FIELD')
        fields=list(tb.getcol('NAME'))#get fieldname list
        tb.close()              #close table
        indexlist=list()        #initialize list
	stringlist=list()

        fldlist=minstring.split()#split string into elements
	print 'fldlist is ',fldlist
        for fld in fldlist:     #loop over fields
                _iter=fields.__iter__() #create iterator for fieldnames
                while 1:
                        try:
                                x=_iter.next() # has first value of field name
                        except StopIteration:
                                break
                        #
                        if (x.find(fld)!=-1): 
				indexlist.append(fields.index(x))
				stringlist.append(x)

	print 'Selected fields are: ',stringlist
        return indexlist

def selectantenna(vis,minstring):
        """Derive the antennaid from matched string(s): """

        tb.open(vis+'/ANTENNA')
        ants=list(tb.getcol('NAME'))#get fieldname list
        tb.close()              #close table
        indexlist=list()        #initialize list
	stringlist=list()

        antlist=minstring.split()#split string into elements
        for ant in antlist:     #loop over fields
        	try:
			ind=ants.index(ant)
			indexlist.append(ind)
			stringlist.append(ant)
                except ValueError:
                        pass

	print 'Selected reference antenna: ',stringlist
	print 'indexlist: ',indexlist
        return indexlist[0]

def readboxfile(boxfile):
	""" Read a file containing clean boxes (compliant with AIPS BOXFILE)

	Format is:
	#FIELDID BLC-X BLC-Y TRC-X TRC-Y
	0       110   110   150   150 
	or
	0       hh:mm:ss.s dd.mm.ss.s hh:mm:ss.s dd.mm.ss.s

	Note all lines beginning with '#' are ignored.

	"""
	union=[]
	f=open(boxfile)
	while 1:
		try: 
			line=f.readline()
			if (line.find('#')!=0): 
				splitline=line.split('\n')
				splitline2=splitline[0].split()
				if (len(splitline2[1])<6): 
					boxlist=[int(splitline2[1]),int(splitline2[2]),
					int(splitline2[3]),int(splitline2[4])]
				else:
					boxlist=[splitline2[1],splitline2[2],splitline2[3],
 					splitline2[4]]
	
				union.append(boxlist)
	
		except:
			break

	f.close()
	return union


# AUTHOR: S. Jaeger
#
# NAME: getimaxes
#
# DESCRIPTION:
# This function uses the coordinate information associated
# with an image to find where the directional (sky) axes are,
# the spectral axes, and the stokes axes.
#
# INPUT:
#    imagename   string   path to a file on disk.
#
# RETURN
#    list of four lists, [list1, list2, list3, list4 ], as follows :
#       list1: ['axis num of 1st sky axis', 'Name of axis' ]
#       list2: ['axis num of 2nd sky axis', 'Name of axis' ]
#       list3: ['axis num of spectral axis', 'Spectral' ]
#       list4: ['axis num of stokes axis', 'Stokes' ]

def getimaxes(imagename):
	"""
	Open an image file, looking at its coordinate system information
	to determine which axes are directional, linear, spectral, and
	the stokes axies.

	The return list or lists contains the axis numbers and names in
	the following order:
	     1. Directional or Linear
	     2. Directional or Linear
	     3. Spectral
	     4. Stokes

	Note that if an axis type is not found an empty list is returned
        for that axis.
	"""

	# Get the images coord. sys.
	csys=None
	ia.open( imagename )
	csys=ia.coordsys()

	# Find where the directional and channel axies are
	# Save the internal placement of the axies in a list
	# (which will be in the following order:
	#    direction1: RA, Longitude, Linear, el, ..
	#    direction2: DEC, Lattitude, Linear, az, ..
	#    spectral:
	#    stokes: I or V
	axisTypes=csys.axiscoordinatetypes()
	axisNames=csys.names()
	
	# Note that we make a potentially dangerous assumption here
	# that the first directional access is always RA, but it
	# may not be.  The names given to the axies are completely
	# arbitrary and can not be used to determine one axis from
	# another.
	# TODO throw exception??? if we find extra axies or
	#      unrecognized axies.
	retValue=[['',''],['',''],['',''],['','']]
	foundFirstDir=False
	for i in range( len( axisTypes ) ):
		if ( axisTypes[i]=='Direction' or axisTypes[i]=='Linear' ):
			if ( not foundFirstDir ) :
				retValue[0]=[i,axisNames[i]]
				foundFirstDir=True
			else:
				retValue[1]=[i,axisNames[i]]
		elif ( axisTypes[i]=='Spectral' ) :
			retValue[2]=[i,axisNames[i]]
		elif ( axisTypes[i]=='Stokes' ) :
			retValue[3]=[i,axisNames[i]]

	if ( csys != None ):
	    del csys
	if ( ia.isopen() ):
	    ia.close()
	return retValue


# from RR
def announce_async_task(taskname):
	"""Use the logger to herald the beginning of an asynchronous task."""
	casalog.origin(taskname)
	casalog.post('')
	casalog.post('###############################################')
	casalog.post('###  Begin Task: ' + taskname + ' ' * (27 - len(taskname)) + '###')
	casalog.post("")
	casalog.post("Use: ")
	casalog.post("      tm.retrieve(return_value) # to retrieve the status")
	casalog.post("")
	
def write_task_obit(taskname):
	"""Eulogize the task in the logger."""
	casalog.post('###  End Task: ' + taskname + ' ' * (29 - len(taskname)) + '###')
	casalog.post('###############################################')
	casalog.post('')


def array2string( array ):
	returnValue=""
	for i in range( len(array) ):
		if ( i > 1 ):
			returnValue+=","
		if ( isinstance( array[i], str ) ):
			returnValue+=array[i]
		else:
			returnValue+=str(array[i])
	return returnValue

def recursivermdir( top='' ):
	# Delete everything from the directory named in 'top',
	# assuming there are no symbolic links.
	for root, dirs, files in os.walk( top, topdown=False ):
		for name in files:
			os.remove( os.path.join( root, name ) )
		for name in dirs:
			os.rmdir( os.path.join( root, name ) )
	os.rmdir(top)
