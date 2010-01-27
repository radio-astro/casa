import os
import sys
import time
import base64
import string
import inspect

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


def dbus_connection( ):
    return bus

class viewertool(object):
    "manage task engines"

    __t = string.maketrans("abcdefghijklmnopqrstuvwxyz0123456789/*:%$#@!&()~+,.:;{}[]|\\\"'^","abcdefghijklmnopqrstuvwxyz0123456789__________________________")

    def __init__( self, with_gui=True ):

        if type(with_gui) != bool:
            raise Exception, "the 'with_gui' parameter must be a boolean"

        self.__state = { }
        self.__state['proxy'] = None
        self.__state['gui'] = with_gui
        self.__state['launched'] = False
        self.__state['dbus name'] = "vtool_%s" % (base64.b64encode(os.urandom(16))).translate(self.__t,'=')
        self.__launch( )


    def __launch( self ):

        if dbus_connection( ) == None:
            raise Exception, "dbus is not available; cannot script the viewer"

        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
           if (string.find(a[k][1], 'ipython console') > 0 or string.find(a[k][1],"casapy.py") > 0):
                      stacklevel=k

        myf=sys._getframe(stacklevel).f_globals

	viewer_path = myf['casa']['helpers']['viewer']   #### set in casapy.py

        args = [ viewer_path, "--casapy" ]
        if self.__state['gui']:
            args += [ '--server=' + self.__state['dbus name'] ]
        else:
            args += [ '--nogui=' + self.__state['dbus name'] ]

	if (os.uname()[0]=='Darwin'):
		vwrpid=os.spawnvp( os.P_NOWAIT, viewer_path, args )
	elif (os.uname()[0]=='Linux'):
		vwrpid=os.spawnlp( os.P_NOWAIT, viewer_path, *args )
	else:
        	raise Exception,'unrecognized operating system'

        self.__state['launched'] = True


    def __connect( self ):
        if not self.__state['launched']:
            self.__launch( )

        if not self.__state['launched']:
            raise Exception, 'launch failed'

        for i in range(1,500):
            time.sleep(0.1)
            try:
                self.__state['proxy'] = bus.get_object( "edu.nrao.casa." + self.__state['dbus name'], "/casa/" + self.__state['dbus name'] )
                if self.__state['proxy'] == None:
                    time.sleep(0.25)
                    continue
                break
            except:
                continue

    def panel( self, paneltype="viewer" ):
        if type(paneltype) != str or (paneltype != "viewer" and paneltype != "clean"):
            raise Exception, "the only valid panel types are 'viewer' and 'clean'"
        if self.__state['proxy'] == None:
            self.__connect( )
        return int(self.__state['proxy'].panel( paneltype ))

    def load( self, path, displaytype="raster", panel=0 ):
        if type(path) != str or type(displaytype) != str:
            raise Exception, "load() takes two strings; only the first arg is required..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return int(self.__state['proxy'].load( path, displaytype, panel ))

    def restore( self, path, new_window=True ):
        if type(path) != str or type(new_window) != bool:
            raise Exception, "restore() takes a string and a boolean; only the first arg is required..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return int(self.__state['proxy'].restore( path, new_window ))

    def cwd( self, new_path='' ):
        if type(new_path) != str:
            raise Exception, "cwd() takes a single (optional) string..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return str(self.__state['proxy'].cwd(new_path))

    def output( self, device, devicetype='file', panel=0, scale=1.0, dpi=300, format="jpg", \
                    orientation="portrait", media="letter" ):
        if type(device) != str or type(panel) != int or type(scale) != float or \
                type(dpi) != int or type(format) != str or type(orientation) != str or \
                type( media ) != str:
            raise Exception, "output() takes (str,int,float,int,str,str,str); only the first is required..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return bool(self.__state['proxy'].output( device, devicetype, panel, scale, dpi, format, orientation, media ))

    def keyinfo( self, key ):
        if type(key) != int:
            raise Exception, "keyinfo() takes a single int..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return map(str,self.__state['proxy'].keyinfo(key))

    def done( self ):

        if self.__state['proxy'] == None:
            return

        result = self.__state['proxy'].done( )
        self.__state['proxy'] = None
        return bool(result)
