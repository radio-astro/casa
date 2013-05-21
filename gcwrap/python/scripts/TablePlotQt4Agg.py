import sys
import pylab as pl
#from Tkinter import *

#defaults
alg='hogbom'
antennaid=-1
archivefiles=False
bandname=False
blbased=False
bptable=''
caltable=False
cell=[1,1]
channels=[]
complist=''
comptype='P'
constpb=0.4
correlations=''
datacolumn='CORRECTED_DATA'
diskwrite=False
fieldid=-1
file=''
fitmode='subtract'
fitorder=0
fitsfile=False
fixpar=[]
fluxdensity=-1
fluxtable=False
fontsize=15.
freqdep=False
freqtol=150000.
gain=0.1
gainselect=''
gaintable=''
highres=False
imagename=False
imsize=[256,256]
incremental=False
iteration=''
lowres=False
mask=['']
mfalg='mfclark'
minpb=0.1
mode='none'
model=''
msselect=''
nchan=-1
niter=0
outputvis=False
overplot=False
plotsymbol=','
refant=-1
reference=[]
region=[0.0]
rmode='none'
robust=0.0
scaletype='PBCOR'
sourcepar=[1,0,0]
subplot=111
solint=0.0
spwid=-1
standard='Perley-Taylor 99'
start=0
step=1
stokes='I'
threshold=0.0
timebin='-1s'
timerange=''
title=''
transfer=[]
verbose = False
vis = False
weighting='natural'
width=1
windowsize=1.0
xaxis=False
xlabels=''
yaxis='amp'
ylabels=''

class Cursors:  #namespace
    HAND, POINTER, SELECT_REGION, MOVE = range(4)
cursors = Cursors()


class PlotFlag:   

    def __init__(self,mm,TPLP):
	self.tb = mm.toolbar;
	self.TPLP = TPLP;
        self.canvas = self.tb.canvas;

	self.lastrect = None;
	self.flaglist = [];
	self.flagregionlist = [];
	self.axeslist = [];

	self.region = None;
	self.kpress = None;
	self.panel = 0;
	self.rows = 0;
	self.cols = 0;
	
        # a dict from axes index to a list of view limits
#self._views = Stack()
#        self._positions = Stack()  # stack of subplot positions
#        self._xypress = None  # the  location and axis info at the time of the press
#        self._idPress = None
#        self._idRelease = None
#        self._active = None
#        self._lastCursor = None
#        self._init_toolbar()
#        self._idDrag=self.canvas.mpl_connect('motion_notify_event', self.mouse_move)
#        self._button_pressed = None # determined by the button pressed at start
#        self.mode = ''  # a mode string for the status bar
#        self.set_history_buttons()
        

    def press_flag(self, event):
        'the press mouse button in flag mode callback'
        if event.button == 1:
            self.tb._button_pressed=1
        elif  event.button == 3:
            self.tb._button_pressed=3
        else:
            self.tb._button_pressed=None
            return

        x, y = event.x, event.y
	self.panel = event.inaxes._num
	self.rows = event.inaxes._rows
	self.cols = event.inaxes._cols

        # push the current view to define home if stack is empty
        if self.tb._views.empty(): self.tb.push_current()

        for i, a in enumerate(self.tb.canvas.figure.get_axes()):
            if event.inaxes==a and event.inaxes.get_navigate():
                xmin, xmax = a.get_xlim()
                ymin, ymax = a.get_ylim()
                lim = xmin, xmax, ymin, ymax
                self.tb._xypress = x, y, a, i, lim, a.transData.deepcopy()
		self.region = event.xdata, event.ydata

                break
        
        self.tb.canvas.mpl_disconnect(self.tb._idDrag)
	self.tb._idDrag=self.canvas.mpl_connect('motion_notify_event', self.flag_mouse_move)
        self.tb.press(event)


    def release_flag(self, event):
        'the release mouse button callback in flag mode'
        if self.tb._xypress is None: return
        x, y = event.x, event.y


        lastx, lasty, a, ind, lim, trans = self.tb._xypress
	lastxdata, lastydata = self.region
	
	# ignore singular clicks - 5 pixels is a threshold
        if abs(x-lastx)<5 or abs(y-lasty)<5:
            self.tb._xypress = None
            self.tb.release(event)
            self.tb.draw()
            return

        # mark rect
	lastx, lasty = a.transData.inverse_xy_tup( (lastx, lasty) )
        x, y = a.transData.inverse_xy_tup( (x, y) )
	Xmin,Xmax=a.get_xlim()
	Ymin,Ymax=a.get_ylim()

	if Xmin < Xmax:
            if x<lastx:  xmin, xmax = x, lastx
            else: xmin, xmax = lastx, x
            if xmin < Xmin: xmin=Xmin
            if xmax > Xmax: xmax=Xmax
        else:
            if x>lastx:  xmin, xmax = x, lastx
            else: xmin, xmax = lastx, x
            if xmin > Xmin: xmin=Xmin
            if xmax < Xmax: xmax=Xmax

        if Ymin < Ymax:
            if y<lasty:  ymin, ymax = y, lasty
            else: ymin, ymax = lasty, y
            if ymin < Ymin: ymin=Ymin
            if ymax > Ymax: ymax=Ymax
        else:
            if y>lasty:  ymin, ymax = y, lasty
            else: ymin, ymax = lasty, y
            if ymin > Ymin: ymin=Ymin
            if ymax < Ymax: ymax=Ymax

	px1,py1 = a.transData.xy_tup( (xmin, ymin) )
	px2,py2 = a.transData.xy_tup( (xmax, ymax) )
	    
	self.draw_rect(px1, py1, px2, py2, xmin, ymin, xmax, ymax, a)
	print "Region to Flag on panel ", self.rows, self.cols, self.panel+1, ": ", xmin, ymin , "->" , xmax, ymax;
		
	#if a.get_aspect() == 'equal': a.set_aspect('equal',True)
	#self.tb.draw()
        self.tb._xypress = None
        self.tb._button_pressed == None

        self.tb.push_current()
        self.tb.canvas.mpl_disconnect(self.tb._idDrag)
	self.tb._idDrag=self.canvas.mpl_connect('motion_notify_event', self.tb.mouse_move)
        self.tb.release(event)
	
	#self.TPLP.clickCoords([xmin,ymin,xmax,ymax])


    def flag_mouse_move(self, event):
        #print 'mouse_move', event.button

        if not event.inaxes or not self.tb._active:
            if self.tb._lastCursor != cursors.POINTER:
                self.tb.set_cursor(cursors.POINTER)
                self.tb._lastCursor = cursors.POINTER
        else:
            if self.tb._active=='ZOOM':
                if self.tb._lastCursor != cursors.SELECT_REGION:
                    self.tb.set_cursor(cursors.SELECT_REGION)
                    self.tb._lastCursor = cursors.SELECT_REGION
                if self.tb._xypress is not None:
                    x, y = event.x, event.y
                    lastx, lasty, a, ind, lim, trans= self.tb._xypress
                    self.tb.draw_rubberband(event, x, y, lastx, lasty)
            elif self.tb._active=='FLAG':
                if self.tb._lastCursor != cursors.SELECT_REGION:
                    self.tb.set_cursor(cursors.SELECT_REGION)
                    self.tb._lastCursor = cursors.SELECT_REGION
                if self.tb._xypress is not None:
                    x, y = event.x, event.y
                    lastx, lasty, a, ind, lim, trans= self.tb._xypress
                    self.tb.draw_rubberband(event, x, y, lastx, lasty)
		    #print lastx,lasty,"->", x,y
            elif (self.tb._active=='PAN' and
                  self.tb._lastCursor != cursors.MOVE):
                self.tb.set_cursor(cursors.MOVE)

                self.tb._lastCursor = cursors.MOVE

        if event.inaxes and event.inaxes.get_navigate():

            try: s = event.inaxes.format_coord(event.xdata, event.ydata)
            except ValueError: pass
            except OverflowError: pass            
            else:
                if len(self.tb.mode):
                    self.tb.set_message('%s : %s' % (self.tb.mode, s))
                else:
                    self.tb.set_message(s)
        else: self.tb.set_message(self.tb.mode)


    def press_key(self,event):
	#print event.key, event.xdata, event.ydata
	if event.key=='alt' and len(self.flaglist)>0:
	   #self.tb.canvas._tkcanvas.delete(self.flaglist[len(self.flaglist)-1]);
	   self.flaglist.pop();
	   self.flagregionlist.pop();
	   self.axeslist.pop();
	   #print self.flagregionlist


    def flag(self, *args):
        'activate flag mode'
        if self.tb._active == 'FLAG':
            self.tb._active = None
        else:
            self.tb._active = 'FLAG'
	    self.erase_rects();

        if self.tb._idPress is not None:
            self.tb._idPress=self.tb.canvas.mpl_disconnect(self.tb._idPress)
            self.tb.mode = ''

        if self.tb._idRelease is not None:
            self.tb._idRelease=self.tb.canvas.mpl_disconnect(self.tb._idRelease)
            self.tb.mode = ''

        if  self.tb._active:
            self.tb._idPress = self.tb.canvas.mpl_connect('button_press_event', self.press_flag)
            self.tb._idRelease = self.tb.canvas.mpl_connect('button_release_event', self.release_flag)
            self.tb.mode = 'Flag mode'

        self.tb.set_message(self.tb.mode)




    def draw_rect(self, x0, y0, x1, y1, x0data, y0data, x1data, y1data,a):
	self.flagregionlist.append([x0data,y0data,x1data,y1data,self.panel+1,self.rows,self.cols]);
	self.axeslist.append(a);
        height = self.tb.canvas.figure.bbox.height()
        y0 =  height-y0
        y1 =  height-y1
	#self.tb.canvas._tkcanvas.delete(self.tb.lastrect)
        #self.lastrect = self.tb.canvas.drawRectangle([x0, y0, x1, y1])
        self.lastrect = pl.plot([x0data, x1data, x1data, x0data, x0data],[y0data, y0data, y1data, y1data, y0data])
	self.flaglist.append(self.lastrect);

    def redraw_rects(self):
	#for q in self.flaglist:
	  #self.tb.canvas._tkcanvas.delete(q);
	self.flaglist = [];
	
	#for q in self.flagregionlist:
	for z in range(0,len(self.flagregionlist)):
	  q = self.flagregionlist[z];
	  a = self.axeslist[z];
          x0=q[0]; y0=q[1]; x1=q[2]; y1=q[3];
          # map to new zoom limits (current fig co-ords)
	  px0,py0 = a.transData.xy_tup( (x0, y0) )
	  px1,py1 = a.transData.xy_tup( (x1, y1) )

	  height = self.tb.canvas.figure.bbox.height()
  	  py0 =  height-py0
  	  py1 =  height-py1
          self.lastrect = pl.plot([x0data, x1data, x1data, x0data, x0data],[y0data, y0data, y1data, y1data, y0data])
	  self.flaglist.append(self.lastrect);

          
    def erase_rects(self):
	#print "erase rects"
	#for q in self.flaglist:
	  #self.tb.canvas._tkcanvas.delete(q);
	self.flaglist = [];
	self.flagregionlist = [];
	self.axeslist = [];

    def close(self):
	#self.bb.forget();
	#if self.tb._active == 'ZOOM':
	#    self.erase_rects();
	#if self.tb._active == 'PAN':
	#    self.erase_rects();
	
	#print "number of flag regions :",len(self.flagregionlist)
	    
        if self.tb._active == 'FLAG':
            self.tb._active = None
	
	self.erase_rects();

        if self.tb._idPress is not None:
            self.tb._idPress=self.tb.canvas.mpl_disconnect(self.tb._idPress)
            self.tb.mode = ''

        if self.tb._idRelease is not None:
            self.tb._idRelease=self.tb.canvas.mpl_disconnect(self.tb._idRelease)
            self.tb.mode = ''

	if self.kpress is not None:
	    self.kpress = self.tb.canvas.mpl_disconnect(self.kpress)

	    
        self.tb.set_message(self.tb.mode)



    def start(self):
	##  Read the current xylims and in plotdata (of a flagdata), force these xylims.
	##  Do similar thing for getting the flag boxes to rescale when zoomed.
	#self.bb = self.tb._Button(text="FlagButton",file='hand.ppm',command=self.flag)
	if self.tb._active == 'ZOOM':
	    self.tb.zoom();
	if self.tb._active == 'PAN':
	    self.tb.pan();
        self.tb._active = 'FLAG'
        self.tb._idPress = self.tb.canvas.mpl_connect('button_press_event', self.press_flag)
        self.tb._idRelease = self.tb.canvas.mpl_connect('button_release_event', self.release_flag)
        self.tb.mode = 'Flag mode'
	self.kpress = self.tb.canvas.mpl_connect('key_press_event', self.press_key)
        self.tb.set_message(self.tb.mode)




#mm = pl.get_current_fig_manager();
#pf = PlotFlag(mm);
#pf.start()


  ##  /opt/local/stow/python-2.4.2/share/matplotlib/

