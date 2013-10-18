from os.path import expanduser, expandvars
from asap._asap import Plotter2

class plotter2:
    def __init__(self):
        self._plotter = Plotter2()

    def set_output(self, filename=None, dev=None):
        """\
        set output filename and/or output device.
        when only filename given, filename is splitted with '.' then
        the last part (filename extension) will be used as output device.
        Parameters:
            filename: output file name (default is '')
            device:   output device. default is 'xwindow'. 'png', 'ps'
                      and 'vps'(PostScript in portrait shape) are
                      available also.

        Example:
            set_output()                -- display in X Window (default)
            set_output('foo.png')       -- generate foo.png in PNG format.
            set_output('bar.ps', 'vps') -- generate bar.ps in portrait shape.
        """
        if filename is None:
            filename = ''
            dev = 'xwindow'
        else:
            filename = filename.strip()
            if filename == '':
                dev = 'xwindow'
            else:
                if (dev is None) or (dev.strip() == ''):
                    fnamelem = filename.lower().split('.')
                    dev = fnamelem[len(fnamelem)-1].strip()
                    if dev == 'ps': dev = 'cps'

        self._plotter.set_filename(expanduser(expandvars(filename)))
        self._plotter.set_device(dev.strip())

    def set_vp(self, xmin, xmax, ymin, ymax, id=None):
        """\
        change the position/shape of viewport in which data are to be plotted.
        positions should be given in window coordinate (0<={x,y}<=1), but,
        unlike the usual definition of window coordinates, with the origin at
        the bottom left corner.
        when you create plotter2 instance, it already has a single viewport
        with xmin=ymin=0.1 and xmax=ymax=0.9 (id=0). in case you don't want to 
        change the size/position of the viewport, you never need to run this
        function. 
        if you want to have two or more viewports, run set_vp() first to
        move/resize the first viewport, then you will be able to add a new
        viewport by executing set_vp without id (the id of the new viewport
        is returned in this case). the location and shape of any viewport
        can be modified later by specifying its viewport id. 

        Parameters:
            xmin: the x position of the left side.
            xmax: the x position of the rightt side.
            ymin: the y position of the top of viewport.
            ymax: the y position of the bottom of viewport.
            id:   viewport id
        """
        if id is None:
            if self._plotter.get_hasdefaultvp():
                self._plotter.set_viewport(xmin, xmax, ymin, ymax, 0)
            else:
                return self._plotter.add_viewport(xmin, xmax, ymin, ymax)
        else:
            self._plotter.set_viewport(xmin, xmax, ymin, ymax, id)

    def show_vp(self, vpid=None):
        """\
        show viewport specified with its id.

        Parameter:
            vpid: viewport id. when unset, operation is applied to 
                  the viewport with the largest id. 
        """
        if vpid is None: vpid = -1
        self._plotter.show_viewport(vpid)
    
    def hide_vp(self, vpid=None):
        """\
        hide viewport specified with its id.

        Parameter:
            vpid: viewport id. when unset, operation is applied to 
                  the viewport with the largest id.
        """
        if vpid is None: vpid = -1
        self._plotter.hide_viewport(vpid)
    
    def set_range(self, xmin, xmax, ymin, ymax, vpid=None):
        """\
        set 2D range to be displayed in the specified viewport.
        by default, the display range is set automatically. 

        Parameter:
            xmin: the minimum of x range.
            xmax: the maximum of x range.
            ymin: the minimum of y range.
            ymax: the maximum of y range.
            vpid: viewport id. when unset, operation is applied to 
                  the viewport with the largest id.
        """
        if vpid is None: vpid = -1
        self._plotter.set_range(xmin, xmax, ymin, ymax, vpid)
        
    def set_xrange(self, xmin, xmax, vpid=None):
        """\
        set x range to be displayed in the specified viewport.
        by default, the display range is set automatically. 

        Parameter:
            xmin: the minimum of x range.
            xmax: the maximum of x range.
            vpid: viewport id. when unset, operation is applied to 
                  the viewport with the largest id.
        """
        if vpid is None: vpid = -1
        self._plotter.set_range_x(xmin, xmax, vpid)
    
    def set_yrange(self, ymin, ymax, vpid=None):
        """\
        set y range to be displayed in the specified viewport.
        by default, the display range is set automatically. 

        Parameter:
            ymin: the minimum of y range.
            ymax: the maximum of y range.
            vpid: viewport id. when unset, operation is applied to 
                  the viewport with the largest id.
        """
        if vpid is None: vpid = -1
        self._plotter.set_range_y(ymin, ymax, vpid)

    def get_xrange(self, vpid=None):
        """\
        returns x range of displayed region of the specified viewport 
        as a list of [xmin, xmax]. 

        Parameter:
            vpid: viewport id. when unset, operation is applied to 
                  the viewport with the largest id.
        """
        if vpid is None: vpid = -1
        return self._plotter.get_range_x(vpid)
    
    def get_yrange(self, vpid=None):
        """\
        returns y range of displayed region in the specified viewport
        as a list of [ymin, ymax]. 

        Parameter:
            vpid: viewport id. when unset, operation is applied to 
                  the viewport with the largest id.
        """
        if vpid is None: vpid = -1
        return self._plotter.get_range_y(vpid)
    
    def set_autorange(self, vpid=None):
        """\
        set the 2-Dimensional range of displayed area of the specified
        viewport to automatically enclose the given data.
        the x range will be from the minimum up to the maximum value
        of the given x values, whereas y range has margins of 10% of
        the y value range both at the top and the bottom sides. 

        Parameter:
            vpid: viewport id. when unset, operation is applied to 
                  the viewport with the largest id.
        """
        if vpid is None: vpid = -1
        self._plotter.set_autorange(vpid)

    def set_xautorange(self, vpid=None):
        """\
        set x range of displayed area of the specified viewport to 
        automatically enclose the given data with no margins. 

        Parameter:
            vpid: viewport id. when unset, operation is applied to 
                  the viewport with the largest id.
        """
        if vpid is None: vpid = -1
        self._plotter.set_autorange_x(vpid)
    
    def set_yautorange(self, vpid=None):
        """\
        set y range of displayed area of the specified viewport to 
        automatically enclose the given data with margins of 10% of
        the y value range both at the top and the bottom sides. 

        Parameter:
            vpid: viewport id. when unset, operation is applied to 
                  the viewport with the largest id.
        """
        if vpid is None: vpid = -1
        self._plotter.set_autorange_y(vpid)

    def set_fontsize(self, size, vpid=None):
        """\
        set the font size that is used for number labels etc.

        Parameter:
            size: font size (default is 1)
            vpid: viewport id. when unset, operation is applied to 
                  the viewport with the largest id.
        """
        if vpid is None: vpid = -1
        self._plotter.set_fontsize(size, vpid)
    
    def set_xtics(self, interval_major, num_minor=None, vpid=None):
        """\
        set the interval of ticks along x axis. 

        Parameter:
            interval_major: interval of major ticks
            num_minor:      major tick interval / minor tick interval.
                            default is 5.
            vpid:           viewport id. when unset, operation is
                            applied to the viewport with the largest id.
        """
        if vpid is None: vpid = -1 
        if num_minor is None: num_minor = 5
        self._plotter.set_tics_x(interval_major, num_minor, vpid)
    
    def set_ytics(self, interval_major, num_minor=None, vpid=None):
        """\
        set the interval of ticks along y axis. 

        Parameter:
            interval_major: interval of major ticks
            num_minor:      major tick interval / minor tick interval.
                            default is 5.
            vpid:           viewport id. when unset, operation is
                            applied to the viewport with the largest id.
        """
        if vpid is None: vpid = -1
        if num_minor is None: num_minor = 5
        self._plotter.set_tics_y(interval_major, num_minor, vpid)
    
    def set_xautotics(self, vpid=None):
        """\
        set the interval of ticks and number labels along x axis
        given automatically.
        
        Parameter:
            vpid: viewport id. when unset, operation is applied to 
                  the viewport with the largest id.
        """
        if vpid is None: vpid = -1
        self._plotter.set_autotics_x(vpid)
    
    def set_yautotics(self, vpid=None):
        """\
        set the interval of ticks and number labels along y axis
        given automatically.
        
        Parameter:
            vpid: viewport id. when unset, operation is applied to 
                  the viewport with the largest id.
        """
        if vpid is None: vpid = -1
        self._plotter.set_autotics_y(vpid)
    
    def set_xnuminterval(self, interval, vpid=None):
        """\
        set the interval of number labels along x axis. 

        Parameter:
            interval: interval of number labels
            vpid:     viewport id. when unset, operation is
                      applied to the viewport with the largest id.
        """
        if vpid is None: vpid = -1 
        self._plotter.set_ninterval_x(interval, vpid)
    
    def set_ynuminterval(self, interval, vpid=None):
        """\
        set the interval of number labels along y axis. 

        Parameter:
            interval: interval of number labels
            vpid:     viewport id. when unset, operation is
                      applied to the viewport with the largest id.
        """
        if vpid is None: vpid = -1
        self._plotter.set_ninterval_y(interval, vpid)

    def set_xnumlocation(self, location=None, vpid=None):
        """\
        set the location of number labels along x axis.

        Parameters:
            location: 'l' (left side) or 'r' (right side). default is 'l'.
            vpid:     viewport id. when unset, operation is
                      applied to the viewport with the largest id.
        """
        if vpid     is None: vpid     = -1
        if location is None: location = "l"
        self._plotter.set_nlocation_x(location.lower(), vpid)
    
    def set_ynumlocation(self, location=None, vpid=None):
        """\
        set the location of number labels along y axis.

        Parameters:
            location: 'b' (bottom) or 't' (top). default is 'b'.
            vpid:     viewport id. when unset, operation is
                      applied to the viewport with the largest id.
        """
        if vpid     is None: vpid     = -1
        if location is None: location = "b"
        self._plotter.set_nlocation_y(location.lower(), vpid)
    
    def set_data(self, xdata, ydata, vpid=None, dataid=None):
        """\
        append or change dataset to be plotted.

        Parameters:
            xdata:  a list of x positions of the input data.
            ydata:  a list of y positions of the input data.
            vpid:   viewport id. when not given, the last viewport
                    will be the target. 
            dataid: dataset id. a integer starting from 0. when dataid
                    is given, the corresponding dataset of the specified
                    viewport is replaced by the given xdata and ydata.
                    when not given, a new dataset of xdata and ydata is
                    appended. 
        """
        if dataid is None:
            if vpid is None: vpid = -1
            dataid = -1
        
        self._plotter.set_data(xdata, ydata, vpid, dataid)

    def set_line(self, color, width=None, style=None, vpid=None, dataid=None):
        """\
        change line attributes.

        Parameters:
            color:  line color specified by color name. available colors 
                    can be listed via list_colornames().
            width:  line width. default is 1.
            style:  line style. available styles can be listed via
                    list_linestyles().
            vpid:   viewport id. when not given, the last viewport
                    will be the target of operation. 
            dataid: dataset id. when not given, the last dataset for the
                    specified viewport is the target.
        """
        if width  is None: width  = 1
        if style  is None: style  = "solid"
        if vpid   is None: vpid   = -1
        if dataid is None: dataid = -1
        
        coloridx = self.get_colorindex(color)
        styleidx = self.get_linestyleindex(style)
        self._plotter.set_line(coloridx, width, styleidx, vpid, dataid)

    def show_line(self, vpid=None, dataid=None):
        """\
        show line connecting the specified dataset.

        Parameters:
            vpid:   viewport id. when not given, the last viewport
                    will be the target. 
            dataid: dataset id. when not given, the last dataset used.
        """
        if dataid is None:
            if vpid is None: vpid = -1
            dataid = -1

        self._plotter.show_line(vpid, dataid)
    
    def hide_line(self, vpid=None, dataid=None):
        """\
        hide line connecting the specified dataset.

        Parameters:
            vpid:   viewport id. when not given, the last viewport
                    will be the target. 
            dataid: dataset id. when not given, the last dataset used.
        """
        if dataid is None:
            if vpid is None: vpid = -1
            dataid = -1

        self._plotter.hide_line(vpid, dataid)
    
    def set_point(self, type, size, color, vpid=None, dataid=None):
        """\
        change marker attributes for the specified dataset.

        Parameters:
            type:   type of marker symbol. see PGPLOT manual for detail.
            size:   marker size. 
            color:  color of marker. see output of list_colornames().
            vpid:   viewport id. when not given, the last viewport
                    will be the target. 
            dataid: dataset id. when not given, the last dataset used.
        """
        if dataid is None:
            if vpid is None: vpid = -1
            dataid = -1
        coloridx = self.get_colorindex(color)
        self._plotter.set_point(type, size, coloridx, vpid, dataid)

    def show_point(self, vpid=None, dataid=None):
        """\
        show markers for the specified dataset.

        Parameters:
            vpid:   viewport id. when not given, the last viewport
                    will be the target. 
            dataid: dataset id. when not given, the last dataset used.
        """
        if dataid is None:
            if vpid is None: vpid = -1
            dataid = -1
        
        self._plotter.show_point(vpid, dataid)

    def hide_point(self, vpid=None, dataid=None):
        """\
        hide markers for the specified dataset.

        Parameters:
            vpid:   viewport id. when not given, the last viewport
                    will be the target. 
            dataid: dataset id. when not given, the last dataset used.
        """
        if dataid is None:
            if vpid is None: vpid = -1
            dataid = -1
        
        self._plotter.hide_point(vpid, dataid)

    def set_xmask(self, xmin, xmax, color=None, fstyle=None, width=None, hsep=None, vpid=None):
        """\
        add a rectangle which spans full y range.

        Parameters:
            xmin:   the smaller end of mask region
            xmax:   the larger end of mask region
            color:  color of mask region. see output of list_colornames().
                    default is "lightgray".
            fstyle: fill style. see output of list_fillstyles().
                    default is "solid".
            width:  width of outline of mask region. default is 1.
            hsep:   spacing of hatched lines. default is 1.0
            vpid:   viewport id. when not given, the last viewport
                    will be the target. 
        """
        if color  is None: color  = "lightgray"
        if fstyle is None: fstyle = "solid"
        if width  is None: width  = 1
        if hsep   is None: hsep   = 1.0
        if vpid   is None: vpid   = -1
        coloridx = self.get_colorindex(color)
        fstyleidx = self.get_fillstyleindex(fstyle)
        self._plotter.set_mask_x(xmin, xmax, coloridx, fstyleidx, width, hsep, vpid)

    def set_xlabel(self, label, style=None, size=None, posx=None, posy=None, vpid=None):
        """\
        set label string along x axis. when the position of label is
        specified explicitly by posx and posy, the label appears so 
        that its center placed on the specified position. 

        Parameters:
            label: label string.
            style: font style. "normal", "roman", "italic" and "script"
                   are available. default is "normal".
            size:  font size. default is 1.1 (10% larger than that
                   of number labels)
            posx:  x position of label string in window coordinate.
                   default is the center of x axis.
            posy:  y position of label string.
            vpid:  viewport id. when not given, the last viewport
                   will be the target. 
        """
        if style is None: style = ""
        if size  is None: size  = 1.1
        if posx  is None: posx  = -1.0
        if posy  is None: posy  = -1.0
        if vpid  is None: vpid  = -1
        
        self._plotter.set_label_x(label, posx, posy, size, style, 1, 0, vpid)
    
    def set_ylabel(self, label, style=None, size=None, posx=None, posy=None, vpid=None):
        """\
        set label string along y axis. when the position of label is
        specified explicitly by posx and posy, the label appears so 
        that its center placed on the specified position. 

        Parameters:
            label: label string.
            style: font style. "normal", "roman", "italic" and "script"
                   are available. default is "normal".
            size:  font size. default is 1.1 (10% larger than that
                   of number labels)
            posx:  x position of label string in window coordinate. 
            posy:  y position of label string.
                   default is the center of y axis.
            vpid:   viewport id. when not given, the last viewport
                    will be the target. 
        """
        if style is None: style = ""
        if size  is None: size  = 1.1
        if posx  is None: posx  = -1.0
        if posy  is None: posy  = -1.0
        if vpid  is None: vpid  = -1
        
        self._plotter.set_label_y(label, posx, posy, size, style, 1, 0, vpid)
    
    def set_title(self, title, style=None, size=None, posx=None, posy=None, vpid=None):
        """\
        set title string over the top of the specified viewport.
        when the position of title is specified explicitly by posx
        and posy, the title appears so that its center placed on
        the specified position. 

        Parameters:
            title: title string.
            style: font style. "normal", "roman", "italic" and "script"
                   are available. default is "normal".
            size:  font size. default is 1.5 (50% larger than that
                   of number titles)
            posx:  x position of title string in window coordinate. 
            posy:  y position of title string.
                   default is the center of y axis.
            vpid:   viewport id. when not given, the last viewport
                    will be the target. 
        """
        if style is None: style = ""
        if size  is None: size  = 1.5
        if posx  is None: posx  = -1.0
        if posy  is None: posy  = -1.0
        if vpid  is None: vpid  = -1
        
        self._plotter.set_title(title, posx, posy, size, style, 1, 0, vpid)

    def set_vpbgcolor(self, bgcolor, vpid=None):
        """\
        change the background color of the specified viewport.
        default is transparent.

        Parameters:
            bgcolor: background color. see output of list_colornames().
                     default is "" (transparent).
            vpid:    viewport id. when not given, the last viewport
                     will be the target. 
        """
        if vpid  is None: vpid  = -1

        if bgcolor.strip() == "":
            coloridx = -1
        else:
            coloridx = self.get_colorindex(bgcolor)
        
        self._plotter.set_vpbgcolor(coloridx, vpid)
    
    def plot(self):
        """\
        execute actual plotting.
        """
        self._plotter.plot()

    def save(self, filename):
        """\
        save the figure in a file with specified name.
        the filename and device previously set by set_output() is
        not affected by this command.

        Parameters:
            filename: output file name.
        """
        prev_filename = self._plotter.get_filename()
        prev_dev      = self._plotter.get_device()

        self.set_output(filename)
        self.plot()
        
        self.set_output(prev_filename, prev_dev)
    
    def get_vinfo(self):
        self._plotter.get_vinfo()

    @classmethod
    def get_colorindex(cls, colorname):
        """\
        convert the given color name into color index used in PGPLOT.
        """
        name = colorname.strip().lower()
        available_color = True

        if   name == "white":       idx =  0  # our definition of bgcolor
        elif name == "black":       idx =  1  # our definition of fgcolor
        elif name == "red":         idx =  2
        elif name == "green":       idx =  3
        elif name == "blue":        idx =  4
        elif name == "cyan":        idx =  5
        elif name == "magenta":     idx =  6
        elif name == "yellow":      idx =  7
        elif name == "orange":      idx =  8
        elif name == "yellowgreen": idx =  9
        elif name == "emerald":     idx = 10
        elif name == "skyblue":     idx = 11
        elif name == "purple":      idx = 12
        elif name == "pink":        idx = 13
        elif name == "gray":        idx = 14
        elif name == "lightgray":   idx = 15
        else: available_color = False

        if (available_color):
            return idx
        else:
            raise ValueError("Unavailable colour name.")

    @classmethod
    def list_colornames(cls):
        """\
        list the available color names.
        """
        print "plotter2: default color list ----"
        print "  (0) white (background)"
        print "  (1) black (foreground)"
        print "  (2) red"
        print "  (3) green"
        print "  (4) blue"
        print "  (5) cyan"
        print "  (6) magenta"
        print "  (7) yellow"
        print "  (8) orange"
        print "  (9) yellowgreen"
        print " (10) emerald"
        print " (11) skyblue"
        print " (12) purple"
        print " (13) pink"
        print " (14) gray"
        print " (15) lightgray"
        print "---------------------------------"

    @classmethod
    def get_linestyleindex(cls, fstyle):
        """\
        convert the given line style into style index used in PGPLOT.
        """
        style = fstyle.strip().lower()
        available_style = True
        
        if   style == "solid":               idx = 1
        elif style == "dashed":              idx = 2
        elif style == "dash-dotted":         idx = 3
        elif style == "dotted":              idx = 4
        elif style == "dash-dot-dot-dotted": idx = 5
        else: available_style = False

        if (available_style):
            return idx
        else:
            raise ValueError("Unavailable line style.")
    
    @classmethod
    def list_linestyles(cls):
        """\
        list the available line styles.
        """
        print "plotter2: fill style list ----"
        print "  (1) solid"
        print "  (2) dashed"
        print "  (3) dash-dotted"
        print "  (4) dotted"
        print "  (5) dash-dot-dot-dotted"
        print "------------------------------"

    @classmethod
    def get_fillstyleindex(cls, fstyle):
        """\
        convert the given fill style into style index used in PGPLOT.
        """
        style = fstyle.strip().lower()
        available_style = True
        
        if   style == "solid":        idx = 1
        elif style == "outline":      idx = 2
        elif style == "hatched":      idx = 3
        elif style == "crosshatched": idx = 4
        else: available_style = False

        if (available_style):
            return idx
        else:
            raise ValueError("Unavailable fill style.")
    
    @classmethod
    def list_fillstyles(cls):
        """\
        list the available fill styles.
        """
        print "plotter2: fill style list ----"
        print "  (1) solid"
        print "  (2) outline"
        print "  (3) hatched"
        print "  (4) crosshatched"
        print "------------------------------"

"""
    def set_annotation(self, label, posx=None, posy=None, angle=None, fjust=None, size=None, style=None, color=None, bgcolor=None, vpid=None):
        if posx    is None: posx    = -1.0
        if posy    is None: posy    = -1.0
        if angle   is None: angle   = 0.0
        if fjust   is None: fjust   = 0.5
        if size    is None: size    = 2.0
        if style   is None: style   = ""
        if color   is None: color   = 1 #default foreground colour (b)
        if bgcolor is None: bgcolor = 0 #default backgound colour (w)
        if vpid    is None: vpid    = -1
        
        coloridx = self.get_colorindex(color)
        bgcoloridx = self.get_colorindex(bgcolor)
        self._plotter.set_annotation(label, posx, posy, angle, fjust, size, style, coloridx, bgcoloridx, vpid)
"""
