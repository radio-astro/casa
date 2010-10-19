# Test script for tableplot

class XYPlot:

   # Initialization
   def __init__(self,tp,gui=True):
        self.tool = tp;
	self.tool.setgui(gui);
	self.__nsecperyear__ = '(86400.0)';
	self.__mjdzeroforpylab__ = '(678575.0+1.0)';

   # Open a table.
   def open(self,table=''):
	"""
	Input table name here.
	"""
	if(len(table)==0):return False;
	self.tool.open(tabnames=[table]);
	self.tabname = table;
	self.__sel__ = '';
        self.__plop__ = {};
	self.setdata();
	self.setpanel(1,1,1);
        self.setplotoptions();

   def done(self):
	self.tool.done();

   # Clear all flags from the table
   def clearallflags(self):
	self.tool.clearflags();

   # Clear Plots
   def clearplot(self,nrows=0,ncols=0,panel=0):
	"""
	Clear a specified panel. Default is all panels.
	"""
	self.tool.clearplot(nrows,ncols,panel);

   def selectdata(self,selstr=''):
	"""
	Make a sub-selection on the MS, using TaQL selection.
	example : 'ANTENNA1==3 && ANTENNA2==6' selects baseline 3-6
	Default is no sub-selection.
	"""
	if(len(selstr)>0):self.tool.selectdata(taqlstring=selstr);
	self.__sel__ = selstr;
	if(len(self.__sel__)>0):print "New sub selection : " , self.__sel__ ;

   # Select data using a TaQL string
   def setdata(self,startchan=0,endchan=0,stepchan=0,startcorr=0,endcorr=0,stepcorr=0,col='data',quant='amp'):
	"""
	Set the channel ranges and correlations to be plotted.
	Set the data column to be plotted 
	  col = data,corrected,model,residual,weighteddata,
	        weightedcorrected, weightedmodel, weightedresidual
          quant = amp, pha, re, im
          The following 6 params are ArrayColumn cell indices.
	  startchan : first channel ( start cell col )
	  endchan : last channel ( end cell col )
	  stepchan : channel increment ( col step )
	  startcorr : first correlation ( start cell row )
	  endcorr : last correlation ( end cell row )
          stepcorr : correlation increment. ( row step )
          
	"""
	self.__endchan__ = endchan;
	self.__startchan__ = startchan;
	self.__stepchan__ = stepchan;
	self.__startcorr__ = startcorr;
	self.__endcorr__ = endcorr;
        self.__stepcorr__ = stepcorr;
	self.__column__ = col;
	self.__quant__ = quant;
	print "Selected data : ", self.tabname;
	if(len(self.__sel__)>0):print "Current sub selection : " , self.__sel__ ;
	print "Channels : " , self.__startchan__ , " to " , self.__endchan__ , " with stepchan ", self.__stepchan__;
	print "Correlations : " , self.__startcorr__ , " to " , self.__endcorr__;
	print "(Zero indicates full range)";
	print "Data column to plot : ", self.__column__;
	print "Quantity to plot : ", self.__quant__;

   # Set panel info - all plots will use this.
   def setpanel(self,nrows=1,ncols=1,panel=1):
	"""
	Choose a panel for the next plot to go onto.
	"""
   	self.__plop__.update({'nrows':nrows,'ncols':ncols,'panel':panel});


   # Set plot options for the next plot.
   def setplotoptions(self,plotrange=[0.0,0.0,0.0,0.0],timeplot='o',columnsxaxis=True,overplot=False,replacetopplot=False,removeoldpanels=True,fontsize=12,windowsize=8.0,aspectratio=1.0,plotsymbol="",plotcolour="default",pointlabels=[],markersize=10.0,linewidth=2.0,multicolour=False,showflags=False,useflagversion="main",skipnpoints=1,averagenpoints=1,connect="none",mean=0,aswidth=False):
	"""
	Set plot options for the next plot.
        Defaults :
        plotsymbol = "" [ 'r,' 'go'... {r,g,b,c,y,k},{-.:.,o^v<>s+xDd1234hHP|_} ] 
        plotcolour = "default" [ '(r,g,b)' or '#eeefff' or 'skyblue','yellowgreen',... ]
        multicolour = False   [ True : multiple ArrrayCol rows and cols are in different colours ]
        overplot = False [ True : overlay plot , False : clear all lower layers first ]
        replacetopplot = False [ True : when overplot=True, clear and replace only top layer ]
        removeoldpanels = True [ True : automatically detect overlapping panels and clear
                                        any partially or completely covered panels.
        plotrange = [0.0,0.0,0.0,0.0] [[xmin,xmax,ymin,ymax]. min=max : use data range ]
        timeplot = 'o' [ time formatting off ('o'), on xaxis ('x'), yaxis ('y'), both ('b') ]
        columnsxaxis = True [ True : columns of ArrayColumn cell on x axis. 
                              False : rows of ArrayColumn cell on x axis. 
                              To be used along with xtaql = 'CROSS' ]
        skipnpoints = 1     [ Plot if pointnumber % skipnpoints == 0 ]
        averagenpoints = 1  [ Average across table rows. For time-averaging, make sure that
                              a sub-selection is performed so that consecutive rows are
                              multiple timestamps for a single baseline... ]
        connect = "none"  [ 'none' : no points are connected by a line
                            'tablerow' : points are connected across table rows.
                                         Separate lines for ArrayColumn rows and cols.
                            'cellcol','cellrow' : points are connected across 
                                                  ArrayColumn cell elements. Separate lines
                                                  for table rows. ]
        mean = 0 [ 0 : no averaging across ArrayColumn cell elements
                     1 : average over both cell rows and cell cols
	             2 : average over cell cols ( chans )
                     3 : average over cell rows ( corrs ) ]
        aswidth = False [ True : treat "stepchan" as a "width" to average on. ]
        showflags = False [ True : plot only flagged data in purple. ]
        useflagversion = "main"  [ name of existing flag version. "main"is just the main table. ]
        pointlabels = []   [ list of strings that will be applied to the first N data points ]
        fontsize = 12  
        windowsize = 8.0
        aspectratio = 1.0
        markersize = 10.0
        linewidth = 2.0
	"""
        self.__plop__.update( {'plotsymbol':plotsymbol,'color':plotcolour } );
        self.__plop__.update( {'overplot':overplot, 'replacetopplot':replacetopplot } );
        self.__plop__.update( {'removeoldpanels':removeoldpanels,'plotrange':plotrange } );
        self.__plop__.update( {'timeplot':timeplot, 'columnsxaxis':columnsxaxis } );
        self.__plop__.update( {'skipnpoints':skipnpoints, 'averagenpoints':averagenpoints } );
        self.__plop__.update( {'connect':connect, 'multicolour':multicolour } );
        self.__plop__.update( {'showflags':showflags, 'useflagversion':useflagversion } );
        if(len(pointlabels)):self.__plop__.update( {'pointlabels':pointlabels} );
        self.__plop__.update( {'windowsize':windowsize, 'aspectratio':aspectratio } );
        self.__plop__.update( {'markersize':markersize, 'linewidth':linewidth,'fontsize':fontsize  } );
        self.__mean__ = mean;
        self.__aswidth__ = aswidth;


   # Make regular plots with DATA (type) columns, and
   def plot(self,type='uvdist',iteraxes=[]):
	"""
	Make a plot of the specified type.
	Currently supported are 'uvdist,vistime,vischannel,viscorrelation,baseline'
	"""
        xystr=[];
        labels=[];
        ok=False;
        if(self.__aswidth__==False):
                ok,xystr,labels = self.__maketaql__(type,self.__startcorr__,self.__endcorr__,self.__stepcorr__,self.__startchan__,self.__endchan__,self.__stepchan__);
        else:
	        if(self.__endchan__==0 or self.__stepchan__==0):
		        print 'Please setdata with startchan,endchan,stepchan';
		        return False;
	        if(self.__startchan__==0):self.__startchan__=1;
                xystr = [];
	        for ch in range(self.__startchan__,self.__endchan__+1,self.__stepchan__):
	        	endch=ch+self.__stepchan__-1;
	        	if(endch>self.__endchan__):endch=self.__endchan__;
	        	ok,tempxystr,labels = self.__maketaql__(type,self.__startcorr__,self.__endcorr__,self.__stepcorr__,ch,endch,1);
                        xystr = xystr + tempxystr;

	if(self.__plop__['useflagversion'] is not 'main'):
	   labels[0] = labels[0] + ' : FlagVer : ' + self.__plop__['useflagversion'];
	#print pop;
	if(ok):
                if(len(iteraxes)==0):
                        self.tool.plotdata(poption=self.__plop__,labels=labels,datastr=xystr);
                else:
	                self.tool.iterplotstart(poption=self.__plop__,labels=labels,datastr=xystr, iteraxes=iteraxes);
        
        return ok;

   def iterplotnext(self):
	"""
	Plot the next set of iteration plots.
	"""
	self.tool.iterplotnext();

   def iterplotstop(self):
	"""
	Stop iteration plots.
	"""
	self.tool.iterplotstop();
	self.tool.open([self.tabname]);
	self.tool.clearplot();

   # Other useful plots
   # UV coverage for the selected data
   def uvcoverage(self,pop={}):
	"""
	UV Coverage
	"""
	pop = pop;
	pop.update(self.__plop__);
	pop.update({'plotsymbol':'r,'});
	labels = ['UV Coverage','u','v'];
	xystr = ['UVW[1]','UVW[2]'];
	self.tool.plotdata(poption=pop,labels=labels,datastr=xystr);
	pop.update({'overplot':True,'plotsymbol':'g,'});
	xystr = ['-UVW[1]','-UVW[2]'];
	print pop;
	self.tool.plotdata(poption=pop,labels=labels,datastr=xystr);

   # Antenna locations from the ANTENNA subtable.
   def array(self,pop={}):
	"""
	Antenna locations
	"""
	pop = pop;
	pop.update(self.__plop__);
	## This uses spurious antenna names...
	self.tool.open([self.tabname+'/ANTENNA']);
	pop.update({'plotsymbol':'co', 'pointlabels':[' A1',' A2',' A3',' A4',' A5',' A6',' A7',' A8',' A9',' B1',' B2',' B3',' B4',' B5',' B6',' B7',' B8',' B9',' C1',' C2',' C3',' C4',' C5',' C6',' C7',' C8',' C9',' D1',' D2',' D3',' D4']})
	xystr = ['POSITION[1]','POSITION[2]'];
	labels = ['Antenna positions','x(m)','y(m)'];
	self.tool.plotdata(poption=pop,labels=labels,datastr=xystr);
	#self.tool.open([self.tabname]);


   # Mark a region on the plot
   def markregion(self):
	"""
	Mark regions on the plot.
	"""
    	self.tool.markregions();

   # Flag a region on the plot
   def flag(self,diskwrite=1,rowflag=0):
	"""
	Flag selected regions
	Need to call "markregion" before this.
	Default : flags are written to disk.
	"""
    	self.tool.flagdata(diskwrite,rowflag);

   # UnFlag a region on the plot
   def unflag(self,diskwrite=1,rowflag=0):
	"""
	UnFlag selected regions
	Need to call "markregion" before this.
	Default : flags are written to disk.
	"""
    	self.tool.unflagdata(diskwrite,rowflag);

   # Print locate info for selected region 
   def locate(self):
	"""
	Print out some useful info about the selected region.
	Need to call "markregion" before this.
	"""
    	self.tool.locatedata(['FIELD_ID','ANTENNA1','ANTENNA2','TIME'])

   # Save flag version  - append the date, time as default comment
   def saveflagversion(self, version, comment='',merge='replace'):
	"""
	Save the current main table flags, into a new/different flag version.
	Use "getflagversionlist" to see a list of flag tables for this MS.
	"""
    	self.tool.saveflagversion(version,comment,merge);

   # Restore flag version 
   def restoreflagversion(self, version, merge='replace'):
	"""
	Copy flags from the specified version, to the current main table.
	merge = 'replace' : copies flags into the main table.
	merge = 'and'     : logical AND of version flags with main table flags
	merge = 'or'     : logical OR of version flags with main table flags
	Use "getflagversionlist" to see a list of flag tables for this MS.
	"""
    	self.tool.restoreflagversion(version,merge);

   # Get a list of flag versions 
   def getflagversionlist(self):
	"""
	To see a list of flag tables for this MS.
	"""
    	self.tool.getflagversionlist()

   # Delete flag version 
   def deleteflagversion(self, version):
	"""
	Delete the specified flag version.
	Use "getflagversionlist" to see a list of flag tables for this MS.
	"""
    	self.tool.deleteflagversion(version)

   # Setup of labels and xtaql for pre-defined plot types
   def __getxtaql__(self,type='uvdist'):
	"""
	Function to generate labels and x-taql strings for various plot-types.
	"""
	tpop = {'timeplot':'o'};
   	
	if(type == 'uvdist'):
	  labels = ['Amplitude vs uvdist','uvdist (kilolambda for lambda=21cm)','amplitude'];
	  xtaql = 'SQRT(SUMSQUARE(UVW[1:2]))/(0.21*1000)'
	if(type == 'vistime'):
	  labels = ['Amplitude vs Time','Time','amplitude'];
	  xtaql = '(TIME/' + self.__nsecperyear__ + ')+' + self.__mjdzeroforpylab__;
	  tpop.update({'timeplot':'x'});
	if(type == 'vischannel'):
	  labels = ['Amplitude vs channel','chan','amplitude'];
	  xtaql = 'CROSS';
	  tpop.update({'columnsxaxis':True});
	if(type == 'viscorrelation'):
	  labels = ['Amplitude vs Correlation','correlation index','amplitude'];
	  xtaql = 'CROSS';
	  tpop.update({'columnsxaxis':False});
	if(type == 'baseline'):
	  labels = ['Amplitude vs baseline number)','baseline','amplitude'];
	  xtaql = '28*ANTENNA1+ANTENNA2-(ANTENNA1-1)*(ANTENNA1+2)/2';
	return xtaql,labels,tpop;

   def __maketaql__(self,type,startcorr,endcorr,stepcorr,startchan,endchan,stepchan):
	"""
	Takes in plot type, and indices and makes the taql
	"""
        xtaql,labels,tpop = self.__getxtaql__(type);
	self.__plop__.update(tpop);
        
	parttaql = '';
	expr = '';
	index = '';
	indexcorr = '';
	indexchan = '';
        col = self.__column__;
        quant = self.__quant__;
	
	if(startcorr==0 and endcorr==0 and stepcorr==0 and startchan==0 and endchan==0 and stepchan==0):
	  index = '';
	else:
	  index = '[';
	  if(startcorr>0): indexcorr = str(startcorr);
	  if(endcorr>0): indexcorr = indexcorr + ':' + str(endcorr);
	  if(stepcorr>0 and endcorr==0): indexcorr = indexcorr + ':';
	  if(stepcorr>0): indexcorr = indexcorr +':'+ str(stepcorr);
	  index = index + indexcorr + ',';
	  if(startchan>0): indexchan = str(startchan);
	  if(endchan>0): indexchan = indexchan + ':' + str(endchan);
	  if(stepchan>0 and endchan==0): indexchan = indexchan + ':';
	  if(stepchan>0): indexchan = indexchan +':'+ str(stepchan);
	  index = index + indexchan + ']';
	
	if(col == 'data'):
	  parttaql = 'DATA' + index ;
	if(col == 'corrected'):
	  parttaql = 'CORRECTED_DATA' + index ;
	if(col == 'model'):
	  parttaql = 'MODEL_DATA' + index ;
	if(col == 'residual'):
	  parttaql = 'DATA' + index + '-MODEL_DATA' + index ;

	if(quant == 'amp' or quant == 'amplitude'):
	  expr = 'AMPLITUDE';
	if(quant == 'pha' or quant == 'phase'):
	  expr = 'PHASE';
	if(quant == 're' or quant == 'real'):
	  expr = 'REAL';
	if(quant == 'im' or quant == 'imag'):
	  expr = 'IMAG';
	
	if(len(parttaql)==0 or len(expr)==0):
	  print 'Invalid col or quant !';
	  print 'col = data,corrected,model,residual';
	  print 'quant = amp,phase,real,imag';
	  return False,[],[];
        
	ytaql = expr+'('+parttaql+')';
        showflags = self.__plop__['showflags'];

	if(self.__mean__==1):
	  if(showflags==False):
                  ytaql = 'MEAN('+expr+'(IIF(FLAG'+index+',0.0,'+parttaql+')))';
          else:
                  ytaql = 'MEAN('+expr+'(IIF(FLAG'+index+','+parttaql+',0.0'+')))';
	  labels[0] = labels[0] + ' : mean over chans and pols';
	if(self.__mean__==2):
	  if(showflags==False):
                  ytaql = 'MEANS('+expr+'(IIF(FLAG'+index+',0.0,'+parttaql+')),2)';
          else:
                  ytaql = 'MEANS('+expr+'(IIF(FLAG'+index+','+parttaql+',0.0'+')),2)';
	  labels[0] = labels[0] + ' : mean over chans';
	if(self.__mean__==3):
	  if(showflags==False):
                  ytaql = 'MEANS('+expr+'(IIF(FLAG'+index+',0.0,'+parttaql+')),1)';
          else:
                  ytaql = 'MEANS('+expr+'(IIF(FLAG'+index+','+parttaql+',0.0'+')),1)';
	  labels[0] = labels[0] + ' : mean over pols';

	xystr = [xtaql,ytaql];
	print xystr;
        return True,xystr,labels;

###############################################################################

