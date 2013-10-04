import sys
import os
import inspect
import string
import time
from taskinit import *
import viewertool


class __imview_class(object):
    "imview() task with local state for created viewer tool"

    def __init__( self ):
        self.local_vi = None
        self.local_ving = None
        self.__dirstack = [ ]
        self.__colorwedge_queue = [ ]

    def __call__( self, raster=None, contour=None, zoom=None, axes=None, out=None ):
        """ Old parameters:
               infile=None,displaytype=None,channel=None,zoom=None,outfile=None,
               outscale=None,outdpi=None,outformat=None,outlandscape=None,gui=None
        The imview task will display images in raster, contour, vector or
        marker form.  Images can be blinked, and movies are available
        for spectral-line image cubes.  For measurement sets, many
        display and editing options are available.

        examples of usage:

        imview
        imview "myimage.im"
        imview "myrestorefile.rstr"
        
        imview "myimage.im", "contour"

        imview "'myimage1.im' - 2 * 'myimage2.im'", "lel"
    
        Executing imview( ) will bring up a display panel
        window, which can be resized.  If no data file was specified,
        a Load Data window will also appear.  Click on the desired data
        file and choose the display type; the rendered data should appear
        on the display panel.

        A Data Display Options window will also appear.  It has drop-down
        subsections for related options, most of which are self-explanatory.
      
        The state of the imview task -- loaded data and related display
        options -- can be saved in a 'restore' file for later use.
        You can provide the restore filename on the command line or
        select it from the Load Data window.

        See the cookbook for more details on using the imview task.
    
        Keyword arguments:
        infile -- Name of file to visualize
            default: ''
            example: infile='ngc5921.image'
            If no infile is specified the Load Data window
            will appear for selecting data.
        displaytype -- (optional): method of rendering data
            visually (raster, contour, vector or marker).  
            You can also set this parameter to 'lel' and
            provide an lel expression for infile (advanced).
            default: 'raster'
            example: displaytype='contour'

        Note: the filetype parameter is optional; typing of
                data files is now inferred.
                example:  imview infile='my.im'
            implies:  imview infile='my.im', filetype='raster'
        the filetype is still used to load contours, etc.


        """
        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
            if a[k][1] == "<string>" or (string.find(a[k][1], 'ipython console') > 0 or string.find(a[k][1],"casapy.py") > 0):
                stacklevel=k

        myf=sys._getframe(stacklevel).f_globals

        casalog.origin('imview')
        
        if (type(out) == str and len(out) != 0) or \
               (type(out) == dict and len(out) != 0) :
            gui = False
            (out_file, out_format, out_scale, out_dpi, out_orientation) = self.__extract_outputinfo( out )
        else:
            gui = True

        if gui and self.local_vi is None or \
           not gui and self.local_ving is None:
            try:
                ## vi/ving might not be defined in taskinit if
                ## loading directly from python via casa.py...
                vwr = vi if gui else ving

                if type(vwr) == type(None) or type(vwr.cwd( )) != str:
                    vwr = viewertool.viewertool( gui, True, (type(myf) == dict and myf.has_key('casa') and type(myf['casa']) == type(os)) )
            except:
                vwr = None

            if gui:
                self.local_vi = vwr
            else:
                self.local_ving = vwr
        else:
            vwr = self.local_vi if gui else self.local_ving

        if type(vwr) == type(None):
            casalog.post( "failed to find a viewertool...", 'SEVERE')
            raise Exception, "failed to find a viewertool..."

        self.__pushd( vwr, os.path.abspath(os.curdir) )

        if (raster is None or len(raster) == 0) and \
           (contour is None or len(contour) == 0) :
            panel = self.__panel(vwr)
        else:
            panel = self.__load_files( "raster", vwr, None, raster )
            panel = self.__load_files( "contour", vwr, panel, contour )
            
        self.__set_axes( vwr, panel, axes )
        self.__zoom( vwr, panel, zoom )
        self.__process_colorwedges( vwr, panel )
        #vwr.unfreeze( panel )

        if not gui:
            vwr.output(out,scale=out_scale,dpi=out_dpi,format=out_format,orientation=out_orientation,panel=panel)
            vwr.close(panel)

        self.__popd( vwr )

        return None

    def __panel( self, vwr ):
        panel = vwr.panel("viewer")
        #vwr.freeze( panel )
        return panel

    def __load_raster( self, vwr, panel, raster ):
        ## here we can assume we have a dictionary
        ## that specifies what needs to be done...
        data = None
        if not raster.has_key('file'):
            return panel

        if  type(raster['file']) != str or not os.path.exists(raster['file']) or \
               vwr.fileinfo(raster['file'])['type'] != 'image':
            casalog.post( str(raster['file']) + " does not exist or is not an image", 'SEVERE')
            raise Exception, raster['file'] + " does not exist or is not an image"

        if panel is None:
            panel = self.__panel(vwr)
        
        scaling = 0.0
        if raster.has_key('scaling'):
            scaling = self.__checknumeric(raster['scaling'], float, "raster scaling")

            
        data = vwr.load( raster['file'], 'raster', panel=panel, scaling=scaling )
        
        if raster.has_key('range'):
            vwr.datarange( self.__checknumeric(raster['range'], float, "data range", array_size=2), data=data )

        if raster.has_key('colormap'):
            if type(raster['colormap']) == str:
                vwr.colormap( raster['colormap'], data )
            else:
                casalog.post( "raster colormap must be a string", 'SEVERE')
                raise Exception, "raster colormap must be a string"

        if raster.has_key('colorwedge'):
            if type(raster['colorwedge']) == bool:
                self.__colorwedge_queue.append( (data,raster['colorwedge']) )
            else:
                casalog.post( "colorwedge must be a boolean", 'SEVERE')
                raise Exception, "colorwedge must be a boolean"

        return panel

    def __process_colorwedges( self, vwr, panel ):
        self.__colorwedge_queue.reverse( )
        while len(self.__colorwedge_queue) > 0:
            element = self.__colorwedge_queue.pop( )
            vwr.colorwedge( element[1], element[0] )

    def __load_contour( self, vwr, panel, contour ):
        ## here we can assume we have a dictionary
        ## that specifies what needs to be done...
        data = None
        if not contour.has_key('file'):
            return panel

        if type(contour['file']) != str or not os.path.exists(contour['file']) or \
               vwr.fileinfo(contour['file'])['type'] != 'image':
            casalog.post( str(contour['file']) + " does not exist or is not an image", 'SEVERE')
            raise Exception, contour['file'] + " does not exist or is not an image"

        if panel is None:
            panel = self.__panel(vwr)

        data = vwr.load( contour['file'], 'contour', panel=panel )

        if contour.has_key('levels'):
            vwr.contourlevels( self.__checknumeric(contour['levels'], float, "contour levels", array_size=0), data=data )
        if contour.has_key('unit'):
            vwr.contourlevels( unitlevel=self.__checknumeric(contour['unit'], float, "contour unitlevel"), data=data )
        if contour.has_key('base'):
            vwr.contourlevels( baselevel=self.__checknumeric(contour['base'], float, "contour baselevel"), data=data )

        return panel

    def __set_axes( self, vwr, panel, axes ):
        x=''
        y=''
        z=''
        invoke = False
        if type(axes) == list and len(axes) == 3 and \
           all( map( lambda x: type(x) == str, axes ) ) :
            x = axes[0]
            y = axes[1]
            z = axes[2]
            invoke = True
        elif type(axes) == dict :
            if axes.has_key('x'):
                if type(axes['x']) != str:
                    casalog.post( "dimensions of axes must be strings (x is not)", 'SEVERE')
                    raise Exception, "dimensions of axes must be strings (x is not)"
                x = axes['x']
                invoke = True
            if axes.has_key('y'):
                if type(axes['y']) != str:
                    casalog.post( "dimensions of axes must be strings (y is not)", 'SEVERE')
                    raise Exception, "dimensions of axes must be strings (y is not)"
                y = axes['y']
                invoke = True
            if axes.has_key('z'):
                if type(axes['z']) != str:
                    casalog.post( "dimensions of axes must be strings (z is not)", 'SEVERE')
                    raise Exception, "dimensions of axes must be strings (z is not)"
                z = axes['z']
                invoke = True
        else :
            casalog.post( "'axes' must either be a string list of 3 dimensions or a dictionary", 'SEVERE')
            raise Exception, "'axes' must either be a string list of 3 dimensions or a dictionary"

        result = False
        if invoke:
            vwr.axes( x, y, z, panel=panel )
            result = True

        return result


    def __zoom( self, vwr, panel, zoom ) :      

        channel = -1
        if type(zoom) == dict and zoom.has_key('channel') :
            channel = self.__checknumeric(zoom['channel'], int, "channel")

        if type(zoom) == int :
            vwr.zoom(level=zoom,panel=panel)
        elif type(zoom) == str and os.path.isfile( zoom ):
            vwr.zoom(region=zoom,panel=panel)
        elif type(zoom) == dict and zoom.has_key('blc') and zoom.has_key('trc'):
            blc = zoom['blc']
            trc = zoom['trc']
            if type(blc) == list and type(trc) == list:
                blc = self.__checknumeric( blc, float, "zoom blc", array_size=2 )
                trc = self.__checknumeric( trc, float, "zoom trc", array_size=2 )

                coord = "pixel"
                if zoom.has_key('coordinates'):
                    if zoom.has_key('coord'):
                        casalog.post( "cannot specify both 'coord' and 'coordinates' for zoom", 'SEVERE')
                        raise Exception, "cannot specify both 'coord' and 'coordinates' for zoom"
                    if type(zoom['coordinates']) != str:
                        casalog.post( "zoom coordinates must be a string", 'SEVERE')
                        raise Exception, "zoom coordinates must be a string"
                    coord = zoom['coordinates']
                    if coord != 'world' and coord != 'pixel' :
                        casalog.post( "zoom coordinates must be either 'world' or 'pixel'", 'SEVERE')
                        raise Exception, "zoom coordinates must be either 'world' or 'pixel'"
                elif zoom.has_key('coord'):
                    if type(zoom['coord']) != str:
                        casalog.post( "zoom coord must be a string", 'SEVERE')
                        raise Exception, "zoom coord must be a string"
                    coord = zoom['coord']
                    if coord != 'world' and coord != 'pixel' :
                        casalog.post( "zoom coord must be either 'world' or 'pixel'", 'SEVERE')
                        raise Exception, "zoom coord must be either 'world' or 'pixel'"
                if channel >= 0:
                    vwr.channel( channel, panel=panel )
                vwr.zoom(blc=blc,trc=trc, coordinates=coord, panel=panel)
            elif type(blc) == dict and type(trc) == dict and \
                 blc.has_key( '*1' ) and trc.has_key( '*1' ) :
                if channel >= 0:
                    vwr.channel( channel, panel=panel )
                vwr.zoom(region=zoom,panel=panel)
            else:
                casalog.post( "zoom blc & trc must be either lists or dictionaries", 'SEVERE')
                raise Exception, "zoom blc & trc must be either lists or dictionaries" 

        elif type(zoom) == dict and zoom.has_key('regions'):
            if channel >= 0:
                vwr.channel( channel, panel=panel )
            vwr.zoom(region=zoom,panel=panel)
        elif type(zoom) == dict and zoom.has_key('file') and type(zoom['file']) == str and os.path.isfile( zoom['file'] ):
            if channel >= 0:
                vwr.channel( channel, panel=panel )
            vwr.zoom(region=zoom['file'],panel=panel)
        else:
            if channel < 0:
                casalog.post( "invalid zoom parameters", 'SEVERE')
                raise Exception, "invalid zoom parameters"
            else:
                vwr.channel( channel, panel=panel )

    def __load_files( self, filetype, vwr, panel, files ):

        if filetype != "raster" and filetype != "contour":
            casalog.post( "internal error __load_files( )...", 'SEVERE')
            raise Exception, "internal error __load_files( )..."

        if type(files) == str:
            panel = self.__load_raster( vwr, panel, { 'file': files } ) if filetype == 'raster' else \
                        self.__load_contour( vwr, panel, { 'file': files } )
        elif type(files) == dict:
            panel = self.__load_raster( vwr, panel, files ) if filetype == 'raster' else \
                        self.__load_contour( vwr, panel, files )
        elif type(files) == list:
            if all(map( lambda x: type(x) == dict, files )):
                for f in files:
                    panel = self.__load_raster( vwr, panel, f ) if filetype == 'raster' else \
                                self.__load_contour( vwr, panel, f )
            elif all(map( lambda x: type(x) == str, files )):
                for f in files:
                    panel = self.__load_raster( vwr, panel, { 'file': f } ) if filetype == 'raster' else \
                                self.__load_contour( vwr, panel, { 'file': f } )
            else:
                casalog.post( "multiple " + str(filetype) + " specifications must be either all dictionaries or all strings", 'SEVERE')
                raise Exception, "multiple " + filetype + " specifications must be either all dictionaries or all strings"
        else:
            casalog.post( filetype + "s can be a single file path (string), a single specification (dictionary), or a list containing all strings or all dictionaries", 'SEVERE')
            raise Exception, filetype + "s can be a single file path (string), a single specification (dictionary), or a list containing all strings or all dictionaries"
        return panel


    def __extract_outputinfo( self, out ):
        output_file=None
        output_format=None
        output_scale=1.0
        output_dpi=300
        output_orientation="portrait"
        
        if type(out) == str:
            output_format = self.__check_filename(out)
            output_file = out

        elif type(out) == dict:
            if out.has_key('file'):
                if type(out['file']) != str:
                    casalog.post( "output filename must be a string", 'SEVERE')
                    raise Exception, "output filename must be a string"
                if out.has_key('format'):
                    if type(out['format']) != str:
                        casalog.post( "output format must be a string", 'SEVERE')
                        raise Exception, "output format must be a string"
                    output_format = self.__check_fileformat( out['format'] )
                    self.__check_filename( out['file'], False )
                else:
                    output_format = self.__check_filename( out['file'] )

                output_file = out['file']

            else:
                casalog.post( "an output dictionary must include a 'file' field", 'SEVERE')
                raise Exception, "an output dictionary must include a 'file' field"

            if out.has_key('scale'):
                output_scale = self.__checknumeric(out['scale'], float, "output scale")

            if out.has_key('dpi'):
                output_dpi = self.__checknumeric(out['dpi'], int, "output dpi")
                output_dpi = int(out['dpi'])

            if out.has_key('orientation'):
                if out.has_key('orient'):
                    casalog.post( "output dictionary cannot have both 'orient' and 'orientation' fields", 'SEVERE')
                    raise Exception, "output dictionary cannot have both 'orient' and 'orientation' fields"
                if type(out['orientation']) != str:
                    casalog.post( "output orientation must be a string", 'SEVERE')
                    raise Exception, "output orientation must be a string"
                if out['orientation'] != 'portrait' and out['orientation'] != 'landscape':
                    casalog.post( "output orientation must be either 'portrait' or 'landscape'", 'SEVERE')
                    raise Exception, "output orientation must be either 'portrait' or 'landscape'"
                output_orientation = out['orientation']

            if out.has_key('orient'):
                if type(out['orient']) != str:
                    casalog.post( "output orient field must be a string", 'SEVERE')
                    raise Exception, "output orient field must be a string"
                if out['orient'] != 'portrait' and out['orient'] != 'landscape':
                    casalog.post( "output orient field must be either 'portrait' or 'landscape'", 'SEVERE')
                    raise Exception, "output orient field must be either 'portrait' or 'landscape'"
                output_orientation = out['orient']

        return (output_file, output_format, output_scale, output_dpi, output_orientation)

    def __checknumeric( self, value, otype, error_string, array_size=None ):
        if array_size is not None:
            if type(array_size) != int:
                casalog.post( "internal error: array_size is expected to be of type int", 'SEVERE')
                raise Exception, "internal error: array_size is expected to be of type int"
            if type(value) != list:
                casalog.post( error_string + " must be a list", 'SEVERE')
                raise Exception, error_string + " must be a list"
            if array_size > 0 and len(value) != array_size:
                numbers = { '1': 'one', '2': 'two', '3': 'three' }
                casalog.post( error_string + " can only be a " + numbers[str(array_size)] + " element numeric list", 'SEVERE')
                raise Exception, error_string + " can only be a " + numbers[str(array_size)] + " element numeric list"
            if not all(map( lambda x: type(x) == int or type(x) == float, value )):
                casalog.post( error_string + " must be a numeric list", 'SEVERE')
                raise Exception, error_string + " must be a numeric list"
            return map( lambda x: otype(x), value )
                    
        if type(value) != int and type(value) != float:
            casalog.post( error_string + " must be numeric", 'SEVERE')
            raise Exception, error_string + " must be numeric"

        return otype(value)

    def __check_fileformat( self, ext ):
        supported_files = [ 'jpg', 'pdf', 'eps', 'ps', 'png', 'xbm', 'xpm', 'ppm' ]
        if supported_files.count(ext.lower( )) == 0:
            casalog.post( "output format '" + str(ext) + "' not supported; supported types are: " + str(supported_files), 'SEVERE')
            raise Exception, "output format '" + str(ext) + "' not supported; supported types are: " + str(supported_files)
        return ext.lower( )


    def __check_filename( self, out, check_extension = True ):
        dir = os.path.dirname(out)
        if len(dir) > 0 and not os.path.isdir(dir):
            casalog.post( "output directory (" + str(dir) + ") does not exist", 'SEVERE')
            raise Exception, "output directory (" + str(dir) + ") does not exist"
        file = os.path.basename(out)
        if len(file) == 0:
            casalog.post( "could not find a valid file name in '" + str(out) + "'", 'SEVERE')
            raise Exception, "could not find a valid file name in '" + str(out) + "'"
        (base,ext) = os.path.splitext(file)
        if len(ext) == 0:
            casalog.post( "could not infer the ouput type from file name '" + str(file) + "'", 'SEVERE')
            raise Exception, "could not infer the ouput type from file name '" + str(file) + "'"
        return self.__check_fileformat(ext[1:]) if check_extension else ''

    def __pushd( self, vwr, newdir ):
        try:
            old_path = vwr.cwd( )
        except:
            casalog.post( "imview() failed to get the current working directory [" + str(sys.exc_info()[0]) + ": " + str(sys.exc_info()[1]) + "]", 'SEVERE')
            raise Exception, "imview() failed to get the current working directory [" + str(sys.exc_info()[0]) + ": " + str(sys.exc_info()[1]) + "]"

        self.__dirstack.append(old_path)
        try:
            vwr.cwd(newdir)
        except:
            casalog.post( "imview() failed to change to the new working directory (" + os.path.abspath(os.curdir) + ") [" + str(sys.exc_info()[0]) + ": " + str(sys.exc_info()[1]) + "]", 'SEVERE')
            raise Exception, "imview() failed to change to the new working directory (" + os.path.abspath(os.curdir) + ") [" + str(sys.exc_info()[0]) + ": " + str(sys.exc_info()[1]) + "]"


    def __popd( self, vwr ):
        try:
            vwr.cwd(self.__dirstack.pop( ))
        except:
            casalog.post( "imview() failed to restore the old working directory (" + old_path + ") [" + str(sys.exc_info()[0]) + ": " + str(sys.exc_info()[1]) + "]", 'SEVERE')
            raise Exception, "imview() failed to restore the old working directory (" + old_path + ") [" + str(sys.exc_info()[0]) + ": " + str(sys.exc_info()[1]) + "]"


imview = __imview_class( )
