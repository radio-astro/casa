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

####-------- return path to XML files --------
def static_var(varname, value):
	def decorate(func):
		setattr(func, varname, value)
		return func
	return decorate

@static_var("path", None)
def xmlpath( ):
	if xmlpath.path is None:
		__casapath__ = os.environ['CASAPATH'].split(' ')[0]
		__casaarch__ = os.environ['CASAPATH'].split(' ')[1]
		if os.path.exists(__casapath__ + "/" + __casaarch__ + "/xml"):
			xmlpath.path = __casapath__ + "/" + __casaarch__ + "/xml"
		elif os.path.exists(__casapath__ + "/xml"):
			xmlpath.path = __casapath__ + "/xml"
		else:
			raise RuntimeError, "Unable to find the XML constraints directory in your CASAPATH"
		
	return xmlpath.path
