import casac
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
		if a[k][1] == "<string>" or (string.find(a[k][1], 'ipython console') > 0 or string.find(a[k][1],"casapy.py") > 0):
			stacklevel=k

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
imager = casac.homefinder.find_home_by_name('imagerHome')
imtool=imager
im = imager.create()
calibrater = casac.homefinder.find_home_by_name('calibraterHome')
cbtool=calibrater
cb = calibrater.create()
mstool = casac.homefinder.find_home_by_name('msHome')
ms = mstool.create()
tptool = casac.homefinder.find_home_by_name('tableplotHome')
tp = tptool.create()
mptool = casac.homefinder.find_home_by_name('msplotHome')
mp = mptool.create()
pmtool = casac.homefinder.find_home_by_name('plotmsHome')
pm = pmtool.create()
cptool = casac.homefinder.find_home_by_name('calplotHome')
cp = cptool.create()
tbtool = casac.homefinder.find_home_by_name('tableHome')
tb = tbtool.create()
fgtool = casac.homefinder.find_home_by_name('flaggerHome')
fg = fgtool.create()
aftool = casac.homefinder.find_home_by_name('autoflagHome')
af = aftool.create()
metool = casac.homefinder.find_home_by_name('measuresHome')
me = metool.create()
qatool = casac.homefinder.find_home_by_name('quantaHome')
qa = casac.qa = qatool.create()
iatool = casac.homefinder.find_home_by_name('imageHome')
ia = iatool.create()
potool = casac.homefinder.find_home_by_name('imagepolHome')
po = potool.create()
smtool = casac.homefinder.find_home_by_name('simulatorHome')
sm = smtool.create()
cltool = casac.homefinder.find_home_by_name('componentlistHome')
cl = cltool.create()
coordsystool = casac.homefinder.find_home_by_name('coordsysHome')
cs = coordsystool.create()
rgtool = casac.homefinder.find_home_by_name('regionmanagerHome')
rg=rgtool.create()
utilstool = casac.homefinder.find_home_by_name('utilsHome')
cu = casac.cu = utilstool.create()
dctool = casac.homefinder.find_home_by_name('deconvolverHome')
dc=dctool.create()
vptool = casac.homefinder.find_home_by_name('vpmanagerHome')
vp=vptool.create()
vftaskhome = casac.homefinder.find_home_by_name('vlafillertaskHome')
vftask = vftaskhome.create()
vlafiller=vftask.fill
loghome =  casac.homefinder.find_home_by_name('logsinkHome')
casalog = loghome.create()
__taskinit_setlogfile(casalog)
casalog.setglobal(True)
attool = casac.homefinder.find_home_by_name('atmosphereHome')
at = attool.create()

# setup viewer tool
if casa.has_key('state') and casa['state'].has_key('startup') :
	ving = viewertool.viewertool( False, pre_launch=casa['state']['startup'] )
	if casa['flags'].has_key('--nogui') :
		vi = ving
	else:
		vi = viewertool.viewertool( True, pre_launch=casa['state']['startup'] )

defaultsdir = {}
defaultsdir['alma'] = 'file:///'+os.environ.get('CASAPATH').split()[0]+'/share/xml/almadefaults.xml'
defaultsdir['evla'] = 'file:///'+os.environ.get('CASAPATH').split()[0]+'/share/xml/evladefaults.xml'


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
