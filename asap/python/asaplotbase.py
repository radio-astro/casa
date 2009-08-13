"""
ASAP plotting class based on matplotlib.
"""

import sys
from re import match

import matplotlib

from matplotlib.figure import Figure, Text
from matplotlib.font_manager import FontProperties as FP
from matplotlib.numerix import sqrt
from matplotlib import rc, rcParams
from asap import rcParams as asaprcParams
from matplotlib.ticker import OldScalarFormatter
from matplotlib.ticker import NullLocator

# API change in mpl >= 0.98
try:
    from matplotlib.transforms import blended_transform_factory
except ImportError:
    from matplotlib.transforms import blend_xy_sep_transform  as blended_transform_factory

from asap import asaplog

if int(matplotlib.__version__.split(".")[1]) < 87:
    #print "Warning: matplotlib version < 0.87. This might cause errors. Please upgrade."
    asaplog.push( "matplotlib version < 0.87. This might cause errors. Please upgrade." )
    print_log( 'WARN' )

#class MyFormatter(OldScalarFormatter):
#    def __call__(self, x, pos=None):
#        last = len(self.locs)-2
#        if pos==0:
#            return ''
#        else: return OldScalarFormatter.__call__(self, x, pos)

class asaplotbase:
    """
    ASAP plotting base class based on matplotlib.
    """

    def __init__(self, rows=1, cols=0, title='', size=(8,6), buffering=False):
        """
        Create a new instance of the ASAPlot plotting class.

        If rows < 1 then a separate call to set_panels() is required to define
        the panel layout; refer to the doctext for set_panels().
        """
        self.is_dead = False
        self.figure = Figure(figsize=size, facecolor='#ddddee')
        self.canvas = None

        self.set_title(title)
        self.subplots = []
        if rows > 0:
            self.set_panels(rows, cols)

        # Set matplotlib default colour sequence.
        self.colormap = "green red black cyan magenta orange blue purple yellow pink".split()

        c = asaprcParams['plotter.colours']
        if isinstance(c,str) and len(c) > 0:
            self.colormap = c.split()

        self.lsalias = {"line":  [1,0],
                        "dashdot": [4,2,1,2],
                        "dashed" : [4,2,4,2],
                        "dotted" : [1,2],
                        "dashdotdot": [4,2,1,2,1,2],
                        "dashdashdot": [4,2,4,2,1,2]
                        }

        styles = "line dashed dotted dashdot".split()
        c = asaprcParams['plotter.linestyles']
        if isinstance(c,str) and len(c) > 0:
            styles = c.split()
        s = []
        for ls in styles:
            if self.lsalias.has_key(ls):
                s.append(self.lsalias.get(ls))
            else:
                s.append('-')
        self.linestyles = s

        self.color = 0;
        self.linestyle = 0;
        self.attributes = {}
        self.loc = 0

        self.buffering = buffering

    def clear(self):
        """
        Delete all lines from the plot.  Line numbering will restart from 0.
        """

        for i in range(len(self.lines)):
           self.delete(i)
        self.axes.clear()
        self.color = 0
        self.lines = []

    def palette(self, color, colormap=None, linestyle=0, linestyles=None):
        if colormap:
            if isinstance(colormap,list):
                self.colormap = colormap
            elif isinstance(colormap,str):
                self.colormap = colormap.split()
        if 0 <= color < len(self.colormap):
            self.color = color
        if linestyles:
            self.linestyles = []
            if isinstance(linestyles,list):
                styles = linestyles
            elif isinstance(linestyles,str):
                styles = linestyles.split()
            for ls in styles:
                if self.lsalias.has_key(ls):
                    self.linestyles.append(self.lsalias.get(ls))
                else:
                    self.linestyles.append(self.lsalias.get('line'))
        if 0 <= linestyle < len(self.linestyles):
            self.linestyle = linestyle

    def delete(self, numbers=None):
        """
        Delete the 0-relative line number, default is to delete the last.
        The remaining lines are NOT renumbered.
        """

        if numbers is None: numbers = [len(self.lines)-1]

        if not hasattr(numbers, '__iter__'):
            numbers = [numbers]

        for number in numbers:
            if 0 <= number < len(self.lines):
                if self.lines[number] is not None:
                    for line in self.lines[number]:
                        line.set_linestyle('None')
                        self.lines[number] = None
        self.show()

    def get_line(self):
        """
        Get the current default line attributes.
        """
        return self.attributes


    def hist(self, x=None, y=None, fmt=None, add=None):
        """
        Plot a histogram.  N.B. the x values refer to the start of the
        histogram bin.

        fmt is the line style as in plot().
        """
        from matplotlib.numerix import array
        from matplotlib.numerix.ma import MaskedArray
        if x is None:
            if y is None: return
            x = range(len(y))

        if len(x) != len(y):
            return
        l2 = 2*len(x)
        x2 = range(l2)
        y2 = range(12)
        y2 = range(l2)
        m2 = range(l2)
        ymsk = None
        ydat = None
        if hasattr(y, "raw_mask"):
            # numpy < 1.1
            ymsk = y.raw_mask()
            ydat = y.raw_data()
        else:
            ymsk = y.mask
            ydat = y.data
        for i in range(l2):
            x2[i] = x[i/2]
            m2[i] = ymsk[i/2]

        y2[0] = 0.0
        for i in range(1,l2):
            y2[i] = ydat[(i-1)/2]

        self.plot(x2, MaskedArray(y2,mask=m2,copy=0), fmt, add)


    def hold(self, hold=True):
        """
        Buffer graphics until subsequently released.
        """
        self.buffering = hold


    def legend(self, loc=None):
        """
        Add a legend to the plot.

        Any other value for loc else disables the legend:
             1: upper right
             2: upper left
             3: lower left
             4: lower right
             5: right
             6: center left
             7: center right
             8: lower center
             9: upper center
            10: center

        """
        if isinstance(loc, int):
            self.loc = None
            if 0 <= loc <= 10: self.loc = loc
        else:
            self.loc = None
        #self.show()


    def plot(self, x=None, y=None, fmt=None, add=None):
        """
        Plot the next line in the current frame using the current line
        attributes.  The ASAPlot graphics window will be mapped and raised.

        The argument list works a bit like the matlab plot() function.
        """
        if x is None:
            if y is None: return
            x = range(len(y))

        elif y is None:
            y = x
            x = range(len(y))
        if fmt is None:
            line = self.axes.plot(x, y)
        else:
            line = self.axes.plot(x, y, fmt)

        # Add to an existing line?
        i = None
        if add is None or len(self.lines) < add < 0:
            # Don't add.
            self.lines.append(line)
            i = len(self.lines) - 1
        else:
            if add == 0: add = len(self.lines)
            i = add - 1
            self.lines[i].extend(line)

        # Set/reset attributes for the line.
        gotcolour = False
        for k, v in self.attributes.iteritems():
            if k == 'color': gotcolour = True
            for segment in self.lines[i]:
                getattr(segment, "set_%s"%k)(v)

        if not gotcolour and len(self.colormap):
            for segment in self.lines[i]:
                getattr(segment, "set_color")(self.colormap[self.color])
                if len(self.colormap)  == 1:
                    getattr(segment, "set_dashes")(self.linestyles[self.linestyle])

            self.color += 1
            if self.color >= len(self.colormap):
                self.color = 0

            if len(self.colormap) == 1:
                self.linestyle += 1
            if self.linestyle >= len(self.linestyles):
                self.linestyle = 0

        self.show()


    def position(self):
        """
        Use the mouse to get a position from a graph.
        """

        def position_disable(event):
            self.register('button_press', None)
            print '%.4f, %.4f' % (event.xdata, event.ydata)

        print 'Press any mouse button...'
        self.register('button_press', position_disable)


    def region(self):
        """
        Use the mouse to get a rectangular region from a plot.

        The return value is [x0, y0, x1, y1] in world coordinates.
        """

        def region_start(event):
            height = self.canvas.figure.bbox.height()
            self.rect = {'fig': None, 'height': height,
                         'x': event.x, 'y': height - event.y,
                         'world': [event.xdata, event.ydata,
                                   event.xdata, event.ydata]}
            self.register('button_press', None)
            self.register('motion_notify', region_draw)
            self.register('button_release', region_disable)

        def region_draw(event):
            self.canvas._tkcanvas.delete(self.rect['fig'])
            self.rect['fig'] = self.canvas._tkcanvas.create_rectangle(
                                self.rect['x'], self.rect['y'],
                                event.x, self.rect['height'] - event.y)

        def region_disable(event):
            self.register('motion_notify', None)
            self.register('button_release', None)

            self.canvas._tkcanvas.delete(self.rect['fig'])

            self.rect['world'][2:4] = [event.xdata, event.ydata]
            print '(%.2f, %.2f)  (%.2f, %.2f)' % (self.rect['world'][0],
                self.rect['world'][1], self.rect['world'][2],
                self.rect['world'][3])

        self.register('button_press', region_start)

        # This has to be modified to block and return the result (currently
        # printed by region_disable) when that becomes possible in matplotlib.

        return [0.0, 0.0, 0.0, 0.0]


    def register(self, type=None, func=None):
        """
        Register, reregister, or deregister events of type 'button_press',
        'button_release', or 'motion_notify'.

        The specified callback function should have the following signature:

            def func(event)

        where event is an MplEvent instance containing the following data:

            name                # Event name.
            canvas              # FigureCanvas instance generating the event.
            x      = None       # x position - pixels from left of canvas.
            y      = None       # y position - pixels from bottom of canvas.
            button = None       # Button pressed: None, 1, 2, 3.
            key    = None       # Key pressed: None, chr(range(255)), shift,
                                  win, or control
            inaxes = None       # Axes instance if cursor within axes.
            xdata  = None       # x world coordinate.
            ydata  = None       # y world coordinate.

        For example:

            def mouse_move(event):
                print event.xdata, event.ydata

            a = asaplot()
            a.register('motion_notify', mouse_move)

        If func is None, the event is deregistered.

        Note that in TkAgg keyboard button presses don't generate an event.
        """

        if not self.events.has_key(type): return

        if func is None:
            if self.events[type] is not None:
                # It's not clear that this does anything.
                self.canvas.mpl_disconnect(self.events[type])
                self.events[type] = None

                # It seems to be necessary to return events to the toolbar.
                if type == 'motion_notify':
                    self.canvas.mpl_connect(type + '_event',
                        self.figmgr.toolbar.mouse_move)
                elif type == 'button_press':
                    self.canvas.mpl_connect(type + '_event',
                        self.figmgr.toolbar.press)
                elif type == 'button_release':
                    self.canvas.mpl_connect(type + '_event',
                        self.figmgr.toolbar.release)

        else:
            self.events[type] = self.canvas.mpl_connect(type + '_event', func)


    def release(self):
        """
        Release buffered graphics.
        """
        self.buffering = False
        self.show()


    def save(self, fname=None, orientation=None, dpi=None, papertype=None):
        """
        Save the plot to a file.

        fname is the name of the output file.  The image format is determined
        from the file suffix; 'png', 'ps', and 'eps' are recognized.  If no
        file name is specified 'yyyymmdd_hhmmss.png' is created in the current
        directory.
        """
        from asap import rcParams
        if papertype is None:
            papertype = rcParams['plotter.papertype']
        if fname is None:
            from datetime import datetime
            dstr = datetime.now().strftime('%Y%m%d_%H%M%S')
            fname = 'asap'+dstr+'.png'

        d = ['png','.ps','eps']

        from os.path import expandvars
        fname = expandvars(fname)

        if fname[-3:].lower() in d:
            try:
                if fname[-3:].lower() == ".ps":
                    from matplotlib import __version__ as mv
                    w = self.figure.get_figwidth()
                    h = self.figure.get_figheight()

                    if orientation is None:
                        # oriented
                        if w > h:
                            orientation = 'landscape'
                        else:
                            orientation = 'portrait'
                    from matplotlib.backends.backend_ps import papersize
                    pw,ph = papersize[papertype.lower()]
                    ds = None
                    if orientation == 'landscape':
                        ds = min(ph/w, pw/h)
                    else:
                        ds = min(pw/w, ph/h)
                    ow = ds * w
                    oh = ds * h
                    self.figure.set_size_inches((ow, oh))
                    self.figure.savefig(fname, orientation=orientation,
                                        papertype=papertype.lower())
                    self.figure.set_size_inches((w, h))
                    print 'Written file %s' % (fname)
                else:
                    if dpi is None:
                        dpi =150
                    self.figure.savefig(fname,dpi=dpi)
                    print 'Written file %s' % (fname)
            except IOError, msg:
                #print 'Failed to save %s: Error msg was\n\n%s' % (fname, err)
                print_log()
                asaplog.push('Failed to save %s: Error msg was\n\n%s' % (fname, msg))
                print_log( 'ERROR' )
                return
        else:
            #print "Invalid image type. Valid types are:"
            #print "'ps', 'eps', 'png'"
            asaplog.push( "Invalid image type. Valid types are:" )
            asaplog.push( "'ps', 'eps', 'png'" )
            print_log('WARN')


    def set_axes(self, what=None, *args, **kwargs):
        """
        Set attributes for the axes by calling the relevant Axes.set_*()
        method.  Colour translation is done as described in the doctext
        for palette().
        """

        if what is None: return
        if what[-6:] == 'colour': what = what[:-6] + 'color'

        key = "colour"
        if kwargs.has_key(key):
            val = kwargs.pop(key)
            kwargs["color"] = val

        getattr(self.axes, "set_%s"%what)(*args, **kwargs)

        self.show(hardrefresh=False)


    def set_figure(self, what=None, *args, **kwargs):
        """
        Set attributes for the figure by calling the relevant Figure.set_*()
        method.  Colour translation is done as described in the doctext
        for palette().
        """

        if what is None: return
        if what[-6:] == 'colour': what = what[:-6] + 'color'
        #if what[-5:] == 'color' and len(args):
        #    args = (get_colour(args[0]),)

        newargs = {}
        for k, v in kwargs.iteritems():
            k = k.lower()
            if k == 'colour': k = 'color'
            newargs[k] = v

        getattr(self.figure, "set_%s"%what)(*args, **newargs)
        self.show(hardrefresh=False)


    def set_limits(self, xlim=None, ylim=None):
        """
        Set x-, and y-limits for each subplot.

        xlim = [xmin, xmax] as in axes.set_xlim().
        ylim = [ymin, ymax] as in axes.set_ylim().
        """
        for s in self.subplots:
            self.axes  = s['axes']
            self.lines = s['lines']
            oldxlim =  list(self.axes.get_xlim())
            oldylim =  list(self.axes.get_ylim())
            if xlim is not None:
                for i in range(len(xlim)):
                    if xlim[i] is not None:
                        oldxlim[i] = xlim[i]
            if ylim is not None:
                for i in range(len(ylim)):
                    if ylim[i] is not None:
                        oldylim[i] = ylim[i]
            self.axes.set_xlim(oldxlim)
            self.axes.set_ylim(oldylim)
        return


    def set_line(self, number=None, **kwargs):
        """
        Set attributes for the specified line, or else the next line(s)
        to be plotted.

        number is the 0-relative number of a line that has already been
        plotted.  If no such line exists, attributes are recorded and used
        for the next line(s) to be plotted.

        Keyword arguments specify Line2D attributes, e.g. color='r'.  Do

            import matplotlib
            help(matplotlib.lines)

        The set_* methods of class Line2D define the attribute names and
        values.  For non-US usage, "colour" is recognized as synonymous with
        "color".

        Set the value to None to delete an attribute.

        Colour translation is done as described in the doctext for palette().
        """

        redraw = False
        for k, v in kwargs.iteritems():
            k = k.lower()
            if k == 'colour': k = 'color'

            if 0 <= number < len(self.lines):
                if self.lines[number] is not None:
                    for line in self.lines[number]:
                        getattr(line, "set_%s"%k)(v)
                    redraw = True
            else:
                if v is None:
                    del self.attributes[k]
                else:
                    self.attributes[k] = v

        if redraw: self.show(hardrefresh=False)


    def set_panels(self, rows=1, cols=0, n=-1, nplots=-1, ganged=True):
        """
        Set the panel layout.

        rows and cols, if cols != 0, specify the number of rows and columns in
        a regular layout.   (Indexing of these panels in matplotlib is row-
        major, i.e. column varies fastest.)

        cols == 0 is interpreted as a retangular layout that accomodates
        'rows' panels, e.g. rows == 6, cols == 0 is equivalent to
        rows == 2, cols == 3.

        0 <= n < rows*cols is interpreted as the 0-relative panel number in
        the configuration specified by rows and cols to be added to the
        current figure as its next 0-relative panel number (i).  This allows
        non-regular panel layouts to be constructed via multiple calls.  Any
        other value of n clears the plot and produces a rectangular array of
        empty panels.  The number of these may be limited by nplots.
        """
        if n < 0 and len(self.subplots):
            self.figure.clear()
            self.set_title()

        if rows < 1: rows = 1

        if cols <= 0:
            i = int(sqrt(rows))
            if i*i < rows: i += 1
            cols = i

            if i*(i-1) >= rows: i -= 1
            rows = i

        if 0 <= n < rows*cols:
            i = len(self.subplots)
            self.subplots.append({})

            self.subplots[i]['axes']  = self.figure.add_subplot(rows,
                                            cols, n+1)
            self.subplots[i]['lines'] = []

            if i == 0: self.subplot(0)

            self.rows = 0
            self.cols = 0

        else:
            self.subplots = []

            if nplots < 1 or rows*cols < nplots:
                nplots = rows*cols
            if ganged:
                hsp,wsp = None,None
                if rows > 1: hsp = 0.0001
                if cols > 1: wsp = 0.0001
                self.figure.subplots_adjust(wspace=wsp,hspace=hsp)
            for i in range(nplots):
                self.subplots.append({})
                self.subplots[i]['lines'] = []
                if not ganged:
                    self.subplots[i]['axes'] = self.figure.add_subplot(rows,
                                                cols, i+1)
                    if asaprcParams['plotter.xaxisformatting'] == 'mpl':
                        self.subplots[i]['axes'].xaxis.set_major_formatter(OldScalarFormatter())
                else:
                    if i == 0:
                        self.subplots[i]['axes'] = self.figure.add_subplot(rows,
                                                cols, i+1)
                        if asaprcParams['plotter.xaxisformatting'] != 'mpl':
                            
                            self.subplots[i]['axes'].xaxis.set_major_formatter(OldScalarFormatter())
                    else:
                        self.subplots[i]['axes'] = self.figure.add_subplot(rows,
                                                cols, i+1,
                                                sharex=self.subplots[0]['axes'],
                                                sharey=self.subplots[0]['axes'])

                    # Suppress tick labelling for interior subplots.
                    if i <= (rows-1)*cols - 1:
                        if i+cols < nplots:
                            # Suppress x-labels for frames width
                            # adjacent frames
                            for tick in self.subplots[i]['axes'].xaxis.majorTicks:
                                tick.label1On = False
                            self.subplots[i]['axes'].xaxis.label.set_visible(False)
                    if i%cols:
                        # Suppress y-labels for frames not in the left column.
                        for tick in self.subplots[i]['axes'].yaxis.majorTicks:
                            tick.label1On = False
                        self.subplots[i]['axes'].yaxis.label.set_visible(False)
                    # disable the first tick of [1:ncol-1] of the last row
                    #if i+1 < nplots:
                    #    self.subplots[i]['axes'].xaxis.majorTicks[0].label1On = False
                self.rows = rows
                self.cols = cols
            self.subplot(0)

    def tidy(self):
        # this needs to be exceuted after the first "refresh"
        nplots = len(self.subplots)
        if nplots == 1: return
        for i in xrange(nplots):
            ax = self.subplots[i]['axes']
            if i%self.cols:
                ax.xaxis.majorTicks[0].label1On = False
            else:
                if i != 0:
                    ax.yaxis.majorTicks[-1].label1On = False


    def set_title(self, title=None):
        """
        Set the title of the plot window.  Use the previous title if title is
        omitted.
        """
        if title is not None:
            self.title = title

        self.figure.text(0.5, 0.95, self.title, horizontalalignment='center')


    def show(self, hardrefresh=True):
        """
        Show graphics dependent on the current buffering state.
        """
        if not hardrefresh: return
        if not self.buffering:
            if self.loc is not None:
                for sp in self.subplots:
                    lines  = []
                    labels = []
                    i = 0
                    for line in sp['lines']:
                        i += 1
                        if line is not None:
                            lines.append(line[0])
                            lbl = line[0].get_label()
                            if lbl == '':
                                lbl = str(i)
                            labels.append(lbl)

                    if len(lines):
                        fp = FP(size=rcParams['legend.fontsize'])
                        fsz = fp.get_size_in_points() - len(lines)
                        fp.set_size(max(fsz,6))
                        sp['axes'].legend(tuple(lines), tuple(labels),
                                          self.loc, prop=fp)
                    else:
                        sp['axes'].legend((' '))

            from matplotlib.artist import setp
            fp = FP(size=rcParams['xtick.labelsize'])
            xts = fp.get_size_in_points()- (self.cols)/2
            fp = FP(size=rcParams['ytick.labelsize'])
            yts = fp.get_size_in_points() - (self.rows)/2
            for sp in self.subplots:
                ax = sp['axes']
                s = ax.title.get_size()
                tsize = s-(self.cols+self.rows)
                ax.title.set_size(tsize)
                fp = FP(size=rcParams['axes.labelsize'])
                setp(ax.get_xticklabels(), fontsize=xts)
                setp(ax.get_yticklabels(), fontsize=yts)
                origx =  fp.get_size_in_points()
                origy = origx
                off = 0
                if self.cols > 1: off = self.cols
                xfsize = origx-off
                ax.xaxis.label.set_size(xfsize)
                off = 0
                if self.rows > 1: off = self.rows
                yfsize = origy-off
                ax.yaxis.label.set_size(yfsize)

    def subplot(self, i=None, inc=None):
        """
        Set the subplot to the 0-relative panel number as defined by one or
        more invokations of set_panels().
        """
        l = len(self.subplots)
        if l:
            if i is not None:
                self.i = i

            if inc is not None:
                self.i += inc

            self.i %= l
            self.axes  = self.subplots[self.i]['axes']
            self.lines = self.subplots[self.i]['lines']

    def text(self, *args, **kwargs):
        """
        Add text to the figure.
        """
        self.figure.text(*args, **kwargs)
        self.show()

    def vline_with_label(self, x, y, label,
                         location='bottom', rotate=0.0, **kwargs):
        """
        Plot a vertical line with label.
        It takes "world" values fo x and y.
        """
        ax = self.axes
        # need this to suppress autoscaling during this function
        self.axes.set_autoscale_on(False)
        ymin = 0.0
        ymax = 1.0
        valign = 'center'
        if location.lower() == 'top':
            y = max(0.0, y)
        elif location.lower() == 'bottom':
            y = min(0.0, y)
        lbloffset = 0.06
        # a rough estimate for the bb of the text
        if rotate > 0.0: lbloffset = 0.03*len(label)
        peakoffset = 0.01
        xy = None
        xy0 = None
        # matplotlib api change 0.98 is using transform now
        if hasattr(ax.transData, "inverse_xy_tup"):
            # get relative coords
            xy0 = ax.transData.xy_tup((x,y))
            xy = ax.transAxes.inverse_xy_tup(xy0)
        else:
            xy0 = ax.transData.transform((x,y))
            # get relative coords
            xy = ax.transAxes.inverted().transform(xy0)
        if location.lower() == 'top':
            ymax = 1.0-lbloffset
            ymin = xy[1]+peakoffset
            valign = 'bottom'
            ylbl = ymax+0.01
        elif location.lower() == 'bottom':
            ymin = lbloffset
            ymax = xy[1]-peakoffset
            valign = 'top'
            ylbl = ymin-0.01
        trans = blended_transform_factory(ax.transData, ax.transAxes)
        l = ax.axvline(x, ymin, ymax, color='black', **kwargs)
        t = ax.text(x, ylbl ,label, verticalalignment=valign,
                                    horizontalalignment='center',
                    rotation=rotate,transform = trans)
        self.axes.set_autoscale_on(True)
