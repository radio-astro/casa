from taskinit import *
try:
    import dbus
    try:
        bus = dbus.SessionBus( )
        have_dbus_module = True
    except:
        print "warning: dbus is not properly configured, viewer scripting will not be available"
        have_dbus_module = False
        bus = None
except:
    print "warning: dbus is not available, viewer scripting will not be available"
    have_dbus_module = False
    bus = None


def seqselect(test, list):  
    """ 
    Select the elements from a sequence that 
    satisfy the given test function 
    - compare The test function should have following 
    signature def test(item): and must return a boolean 
    - list The List from which element need to be selected 
    """  
    selected = [ ]
    for item in list:  
        if test(item) == True:  
            selected.append(item)
    return selected  

def dbus_connection( ):
    return bus

def viewerinstances( ):
    if not have_dbus_module:
        return None
    return seqselect(lambda x: x.startswith('edu.nrao.casa.viewer_'),map(str,bus.list_names( )))

def viewerconnection( name='', warn=True ):

        """Scripting connection to the CASA Viewer

	This provides a connection interface to the CASA Viewer. The
	only parameter is the name of the viewer to which we wish to
	connect.

	The names of the available viewers can be found with
	viewerinstances(). It is also possible to have viewer( ) return
	a viewerconnection.


  
        """

        if not have_dbus_module:
            return None

        viewers = viewerinstances( )
        if len(name) == 0:
            if len(viewers) == 0:
                if warn:
                    print "no viewers available, please start one with viewer()..."
                return None
            return viewer_connection(viewers[0])

        matches = seqselect(lambda x: x == name, viewers)
        if len(matches) > 0:
            return viewer_connection(matches[0])

        matches = seqselect(lambda x: x.endswith("." + name), viewers)
        if len(matches) > 0:
            return viewer_connection(matches[0])

        if warn:
            print "could not find a viewer matching: " + name

        return None

class viewer_connection:
    "manage task engines"

    PROXY = None

    def __init__( self, name ):
        cname = name.split('.')
        if len(cname) <= 1:
            raise Exception, "invalid service name: " + name

        self.PROXY = bus.get_object( name, "/" + cname[len(cname)-2] + "/" + cname[len(cname)-1] )

    def load( self, path, displaytype = "raster" ):
        if self.PROXY == None:
            raise Exception, "initialization failed (or viewer exited)..."
        if type(path) != str or type(displaytype) != str:
            raise Exception, "load() takes two strings; only the first arg is required..."

        return int(self.PROXY.load( path, displaytype ))

    def restore( self, path, new_window=True ):
        if self.PROXY == None:
            raise Exception, "initialization failed (or viewer exited)..."
        if type(path) != str or type(new_window) != bool:
            raise Exception, "restore() takes a string and a boolean; only the first arg is required..."

        return int(self.PROXY.restore( path, new_window ))

    def cwd( self, new_path='' ):
        if self.PROXY == None:
            raise Exception, "initialization failed (or viewer exited)..."
        if type(new_path) != str:
            raise Exception, "cwd() takes a single (optional) string..."

        return str(self.PROXY.cwd(new_path))

    def output( self, device, devicetype='file', panel=0, scale=1.0, dpi=300, format="jpg", \
                    orientation="portrait", media="letter" ):
        if self.PROXY == None:
            raise Exception, "initialization failed (or viewer exited)..."
        if type(device) != str or type(panel) != int or type(scale) != float or \
                type(dpi) != int or type(format) != str or type(orientation) != str or \
                type( media ) != str:
            raise Exception, "output() takes (str,int,float,int,str,str,str); only the first is required..."

        return bool(self.PROXY.output( device, devicetype, panel, scale, dpi, format, orientation, media ))

    def keyinfo( self, key ):
        if self.PROXY == None:
            raise Exception, "initialization failed (or viewer exited)..."
        if type(key) != int:
            raise Exception, "keyinfo() takes a single int..."

        return map(str,self.PROXY.keyinfo(key))

    def done( self ):
        if self.PROXY == None:
            raise Exception, "initialization failed (or viewer exited)..."

        result = self.PROXY.done( )
        self.PROXY = None
        return bool(result)
