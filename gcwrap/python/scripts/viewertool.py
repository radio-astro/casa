import os
import re
import sys
import time
import base64
import string
import inspect
from taskinit import casac       ### needed for regionmanager

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


class viewertool(object):
    "manage task engines"

    __t = string.maketrans("abcdefghijklmnopqrstuvwxyz0123456789/*:%$#@!&()~+,.:;{}[]|\\\"'^","abcdefghijklmnopqrstuvwxyz0123456789__________________________")
    __rgm = casac.regionmanager()

    ###
    ### 'use_existing' defaults to false because:
    ###    o  for linux a new dbus session daemon is started for each casapy
    ###    o  for osx it would result in multiple casapy sessions using the same viewer
    ###    o  for casa.py included into python it makes sense to avoid a new viewer
    ###             appearing (and sticking around) for each include
    ###
    def __init__( self, with_gui=True, pre_launch=False, use_existing=False ):

        if type(with_gui) != bool:
            raise Exception, "the 'with_gui' parameter must be a boolean"

        self.__state = { }
        self.__state['proxy'] = None
        self.__state['gui'] = with_gui
        self.__state['launched'] = False
        self.__state['dbus name'] = None

        if type(with_gui) == bool and with_gui == False:
            basename = "vtoolng"
        else:
            basename = "vtool"

        ## for viewer used from plain python, see if a viewer is already available on dbus first...
        bus = dbus_connection( )
        if bus != None and type(use_existing) == bool and use_existing == True :
            candidates = seqselect(lambda x: x.startswith('edu.nrao.casa.%s_' % (basename)),map(str,bus.list_names( )))
            if len( candidates ) > 0 :
                candidate = candidates[0]
                p = re.compile('[^\.]+')
                segments = p.findall(candidate)
                if len(segments) == 4 :
                    self.__state['dbus name'] = segments[3]
                    self.__state['launched'] = True

        if self.__state['dbus name'] == None:
            self.__state['dbus name'] = "%s_%s" % (basename, (base64.b64encode(os.urandom(16))).translate(self.__t,'='))

        if pre_launch:
            self.__launch( )


    def __launch( self ):

        ## if we've already launched the viewer
        if type(self.__state['launched']) == bool and self.__state['launched'] == True:
            return

        if dbus_connection( ) == None:
            raise Exception, "dbus is not available; cannot script the viewer"

        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
            if a[k][1] == "<string>" or (string.find(a[k][1], 'ipython console') > 0 or string.find(a[k][1],"casapy.py") > 0):
                      stacklevel=k

        myf=sys._getframe(stacklevel).f_globals

        viewer_path = None
        if type(myf) == dict and myf.has_key('casa') and type(myf['casa']) == dict and myf['casa'].has_key('helpers') \
                and type(myf['casa']['helpers']) == dict and myf['casa']['helpers'].has_key('viewer'):
            viewer_path = myf['casa']['helpers']['viewer']   #### set in casapy.py
            if len(os.path.dirname(viewer_path)) == 0:
                for dir in os.getenv('PATH').split(':') :
                    dd = dir + os.sep + viewer_path
                    if os.path.exists(dd) and os.access(dd,os.X_OK) :
                        viewer_path = dd
                        break
            args = [ viewer_path, "--casapy" ]
        else:
            for exe in ['casaviewer']:
                for dir in os.getenv('PATH').split(':') :
                    dd = dir + os.sep + exe
                    if os.path.exists(dd) and os.access(dd,os.X_OK) :
                        viewer_path = dd
                        break
                if viewer_path is not None:
                    break
            args = [ viewer_path ]

        if viewer_path == None or not os.access(viewer_path,os.X_OK):
            raise RuntimeError("cannot find casa viewer executable")

        if self.__state['gui']:
            args += [ '--server=' + self.__state['dbus name'] ]
        else:
            args += [ '--nogui=' + self.__state['dbus name'] ]

        if type(myf) == dict and myf.has_key('casa') and type(myf['casa']) == dict and myf['casa'].has_key('files') \
                and type(myf['casa']['files']) == dict and myf['casa']['files'].has_key('logfile'):
            args += [ '--casalogfile=' + myf['casa']['files']['logfile'] ]

        if  type(myf) == dict and myf.has_key('casa') and type(myf['casa']) == dict and myf['casa'].has_key('flags') \
                 and type(myf['casa']['flags']) == dict and myf['casa']['flags'].has_key('--rcdir'):
            args += [ "--rcdir=" + myf['casa']['flags']['--rcdir'] ]

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

        error = None
        for i in range(1,500):
            time.sleep(0.1)
            try:
                self.__state['proxy'] = bus.get_object( "edu.nrao.casa." + self.__state['dbus name'], "/casa/" + self.__state['dbus name'] )
                if self.__state['proxy'] == None:
                    time.sleep(0.25)
                    continue
                error = None
                break
            except dbus.DBusException, e:
                if e.get_dbus_name() == 'org.freedesktop.DBus.Error.Disconnected' :
                    raise RuntimeError('DBus daemon has died...')
                elif e.get_dbus_name() == 'org.freedesktop.DBus.Error.ServiceUnknown' :
                    error = RuntimeError('DBus Viewer service failed to start...')
                    continue
                else:
                    raise RuntimeError('Unexpected DBus problem: ' + e.get_dbus_name( ) + "(" + e.args[0] + ")")
            except Exception, e:
                error = e
                continue

        if error is not None :
            raise error

    def __invoke( self, dt, t, func, *args, **kwargs ):
        ## set maximum dbus timeout...
        kwargs['timeout'] = 0x7fffffff / 1000.0
        try:
            result = func(*args,**kwargs)
        except dbus.DBusException, e:
            if e.get_dbus_name() == 'org.freedesktop.DBus.Error.Disconnected' :
                raise RuntimeError('DBus daemon has died....')
            elif e.get_dbus_name() == 'org.freedesktop.DBus.Error.ServiceUnknown' :
                raise RuntimeError('DBus Viewer service has exited....')
            else:
                raise RuntimeError('Unexpected DBus problem: ' + e.get_dbus_name( ) + "(" + e.args[0] + ")")

        if type(result) == dbus.Dictionary and result.has_key('*error*') :
            raise RuntimeError(str(result['*error*']))
        elif type(result) != dt :
            raise RuntimeError(str(result))

        return t(result)

    def panel( self, paneltype="viewer" ) :
        if type(paneltype) != str or (paneltype != "viewer" and paneltype != "clean"):
            raise Exception, "the only valid panel types are 'viewer' and 'clean'"
        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Int32, int, self.__state['proxy'].panel, paneltype )

    def load( self, path, displaytype="raster", panel=0, scaling=0 ):
        if type(path) != str or type(displaytype) != str or \
               (type(scaling) != float and type(scaling) != int) :
            raise Exception, "load() takes two strings; only the first arg is required..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Int32, int, self.__state['proxy'].load, path, displaytype, panel, float(scaling) )

    def close( self, panel=0 ):
        if type(panel) != int :
            raise Exception, "close() takes one optional integer..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Boolean, bool, self.__state['proxy'].close, panel )

    def popup( self, what, panel=0 ):
        if type(what) != str or type(panel) != int :
            raise Exception, "popup() takes a string followed by one optional integer..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Boolean, bool, self.__state['proxy'].popup, what, panel )

    def freeze( self, panel=0 ):
        if type(panel) != int :
            raise Exception, "freeze() takes only a panel id..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Boolean, bool, self.__state['proxy'].freeze, panel )

    def unfreeze( self, panel=0 ):
        if type(panel) != int :
            raise Exception, "unfreeze() takes only a panel id..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Boolean, bool, self.__state['proxy'].unfreeze, panel )


    def restore( self, path, panel=0 ):
        if type(path) != str or type(panel) != int:
            raise Exception, "restore() takes a string and an integer; only the first arg is required..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Int32, int, self.__state['proxy'].restore, path, panel )

    def cwd( self, new_path='' ):
        if type(new_path) != str:
            raise Exception, "cwd() takes a single (optional) string..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.String, str, self.__state['proxy'].cwd, new_path )

    def output( self, device, devicetype='file', panel=0, scale=1.0, dpi=300, format="jpg", \
                    orientation="portrait", media="letter" ):
        if type(device) != str or type(panel) != int or type(scale) != float or \
                type(dpi) != int or type(format) != str or type(orientation) != str or \
                type( media ) != str:
            raise Exception, "output() takes (str,int,float,int,str,str,str); only the first is required..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Boolean, bool, self.__state['proxy'].output, device, devicetype, panel, scale, dpi, format, orientation, media )

    def axes( self, x='', y='', z='', panel=0 ):
        if type(x) != str or type(y) != str or type(z) != str or type(panel) != int :
            raise Exception, "axes() takes one to three strings and an optional panel id..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Boolean, bool, self.__state['proxy'].axes, x, y, z, panel )

    def datarange( self, range, data=0 ):
        if type(range) != list or type(data) != int or \
           all( map( lambda x: type(x) == int or type(x) == float, range ) ) == False:
            raise Exception, "datarange() takes (numeric list,int)..."
        if len(range) != 2 or range[0] > range[1] :
            raise Exception, "range should be [ min, max ]..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Boolean, bool, self.__state['proxy'].datarange, map( lambda(x): float(x), range ), data )
    
    def contourlevels( self, levels=[], baselevel=2147483648.0, unitlevel=2147483648.0, data=0 ):
        if type(levels) != list or type(data) != int or \
           all( map( lambda x: type(x) == int or type(x) == float, levels ) ) == False:
            raise Exception, "contorlevels() takes (numeric list,int)..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Boolean, bool, self.__state['proxy'].contourlevels, dbus.Array(map( lambda(x): float(x), levels),signature="d"), baselevel, unitlevel, data )

    def colormap( self, map, data_or_panel=0 ):
        if type(map) != str or type(data_or_panel) != int :
            raise Exception, "colormap() takes a colormap name and an optional panel or data id..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Boolean, bool, self.__state['proxy'].colormap, map, data_or_panel )
    

    def colorwedge( self, show, data_or_panel=0 ):
        if type(show) != bool or type(data_or_panel) != int :
            raise Exception, "colorwedge() takes a boolean and an optional panel or data id..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Boolean, bool, self.__state['proxy'].colorwedge, show, data_or_panel )
    

    def channel( self, num=-1, panel=0 ):
        if type(num) != int or type(panel) != int:
            raise Exception, "frame() takes (int,int); each argument is optional..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Int32, int, self.__state['proxy'].channel, num, panel )

    def zoom( self, level=None, blc=[], trc=[], coordinates="pixel", region="", panel=0 ):
        if (type(level) != int and level != None) or \
               type(blc) != list or type(trc) != list or type(panel) != int or \
               type(coordinates) != str or (type(region) != str and type(region) != dict) :
            raise Exception, "zoom() takes (int|None,list,list,str,int); each argument is optional..."

        if self.__state['proxy'] == None:
            self.__connect( )

        if (type(region) == str and os.path.isfile( region )) or \
               type(region) == dict :
            reg = region
            if type(region) == str :
                try:
                    reg = self.__rgm.fromfiletorecord( region )
                except:
                    raise Exception, "region file (" + str(region) + ") exists but is not a valid region file"

            if type(reg) != dict :
                raise Exception, "invalid regions or failed to load region"

            ( _blc, _trc, _coord ) = self.__extract_region_box( reg )

            return self.__invoke( dbus.Boolean, bool, self.__state['proxy'].zoom, _blc, _trc, _coord, panel )
            
        elif len(blc) == 2 and len(trc) == 2 and \
               all( map( lambda x,y: (type(x) == int or type(x) == float) and (type(y) == int or type(y) == float), blc, trc ) ) == True:
            if coordinates != "pixel" and coordinates != "world":
                raise Exception, "zoom() coordinates must be either world or pixel"
            return self.__invoke( dbus.Boolean, bool, self.__state['proxy'].zoom, map( lambda(x): float(x), blc ), map( lambda(x): float(x), trc ), coordinates, panel )
        elif level != None:
            return self.__invoke( dbus.Boolean, bool, self.__state['proxy'].zoom, level, panel )
        else:
            raise Exception, "must supply either blc and trc or a level for zoom"



    def hide( self, panel=0 ):
        if type(panel) != int:
            raise Exception, "hide() takes a single (int) panel identifier ..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Boolean, bool, self.__state['proxy'].hide, panel )

    def show( self, panel=0 ):
        if type(panel) != int:
            raise Exception, "show() takes a single (int) panel identifier ..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Boolean, bool, self.__state['proxy'].show, panel )

    def fileinfo( self, path ):
        if type(path) != str:
            raise Exception, "fileinfo() takes a single path..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Dictionary, dict, self.__state['proxy'].fileinfo, path )

    def keyinfo( self, key ):
        if type(key) != int:
            raise Exception, "keyinfo() takes a single int..."

        if self.__state['proxy'] == None:
            self.__connect( )

        return self.__invoke( dbus.Array, lambda x: map(str,x), self.__state['proxy'].keyinfo, key )

    def done( self ):

        if self.__state['proxy'] == None:
            self.__connect( )

        result = self.__invoke( dbus.Boolean, bool, self.__state['proxy'].done )
        self.__state['proxy'] = None
        self.__state['launched'] = False
        return result

    def __extract_region_box( self, reg ):

        if reg.has_key( 'regions' ) :
            if type(reg['regions']) != dict or not reg['regions'].has_key( '*1' ) :
                raise Exception, "invalid region, has 'regions' field but wrong format"
            reg=reg['regions']['*1']

        if not reg.has_key('trc') or not reg.has_key('blc'):
            raise Exception, "region must have a 'blc' and 'trc' field"

        blc_r = reg['blc']
        trc_r = reg['trc']

        if type(blc_r) != dict or type(trc_r) != dict :
            raise Exception, "region blc/trc of wrong type"

        blc_k = blc_r.keys( )
        trc_k = trc_r.keys( )

        if len(blc_k) < 2 or len(trc_k) < 2:
            raise Exception, "degenerate region"

        blc_k.sort( )
        trc_k.sort( )

        if type(blc_r[blc_k[0]]) != dict or type(blc_r[blc_k[1]]) != dict or \
               type(trc_r[trc_k[0]]) != dict or type(trc_r[trc_k[1]]) != dict :
            raise Exception, "invalid blc/trc in region"

        if not blc_r[blc_k[0]].has_key('value') or not blc_r[blc_k[1]].has_key('value') or \
               not trc_r[trc_k[0]].has_key('value') or not trc_r[trc_k[1]].has_key('value'):
            raise Exception, "invalid shape for blc/trc in region"

        if (type(blc_r[blc_k[0]]['value']) != float and type(blc_r[blc_k[0]]['value']) != int) or \
               (type(blc_r[blc_k[1]]['value']) != float and type(blc_r[blc_k[1]]['value']) != int) or \
               (type(trc_r[trc_k[0]]['value']) != float and type(trc_r[trc_k[0]]['value']) != int) or \
               (type(trc_r[trc_k[0]]['value']) != float and type(trc_r[trc_k[0]]['value']) != int) :
            raise Exception, "invalid type for blc/trc value in region"

        blc = [ float(blc_r[blc_k[0]]['value']), float(blc_r[blc_k[1]]['value']) ]
        trc = [ float(trc_r[trc_k[0]]['value']), float(trc_r[trc_k[1]]['value']) ]

        coord = "pixel"
        if reg.has_key('name') and reg['name'] == "WCBox":
            coord = "world"

        return ( blc, trc, coord )
