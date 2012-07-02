# Test script for tableplot
#
# tpdemo1 : multiple panels, overplot, clearplot, markregions, flagdata,
# tpdemo2 : timeplot, crossplot, locatedata. With multicolour
# tpdemo3 : with pointlabels
# tpdemo4 : iteration plots
# tpdemo5 : averagenpoints, connect
# tpdemo6 : iterplot with overplot
# tpdemo7 : flag-versions
#

DATApath = "~/DATA/ngc5921.ms";
DATApathANT = "~/DATA/ngc5921.ms/ANTENNA";

def tpdemo1(tp):
	tp.open(tabnames=[DATApath]);
	
	# clear the plot and underlying data structures
	tp.clearplot();

	pop = { 'nrows':2, 'ncols':1,'panel':1, 'windowsize':8.0, 'showflags':False ,'plotsymbol':'g,','timeplot':'o', 'overplot':False}
	
	# uvdist for pol 1 and chan 1,2 on panel 211
	pop.update({'panel':1,'plotsymbol':'r,'});
	labels = ['Amplitude vs UVdist','uvdist','amplitude'];
	xystr = ['SQRT(SUMSQUARE(UVW[1:2]))','AMPLITUDE(DATA[1,1:2])'];
	tp.plotdata(poption=pop,labels=labels,datastr=xystr);
	
	# uvdist for pol 2 and chan 1,2 (overplot=1) on panel 211
	pop.update({'panel':1,'plotsymbol':'g,', 'overplot':True});
	labels = ['Amplitude vs UVdist','uvdist','amplitude'];
	xystr = ['SQRT(SUMSQUARE(UVW[1:2]))','AMPLITUDE(DATA[2,1:2])'];
	tp.plotdata(poption=pop,labels=labels,datastr=xystr);
	
	# uv coverage on panel 223
	pop = {'nrows':2, 'ncols':2,'panel':3, 'overplot':False};
	labels = ['UV Coverage','u','v'];
	#xystr = ['UVW[1]','UVW[2]','-UVW[1]','-UVW[2]'];
	#tp.plotdata(poption=pop,labels=labels,datastr=xystr);
	xystr = ['UVW[1]','UVW[2]'];
	tp.plotdata(poption=pop,labels=labels,datastr=xystr);
	pop.update({'overplot':True});
	xystr = ['-UVW[1]','-UVW[2]'];
	tp.plotdata(poption=pop,labels=labels,datastr=xystr);
	
	# amp(data[1:2,1:10]) vs channel number on panel 224
	pop = { 'nrows':2, 'ncols':2,'panel':4, 'windowsize':8.0, 'showflags':False ,'plotsymbol':',','timeplot':'o', 'overplot':False}
	labels = ['Amplitude vs Baseline number','baseline number','amplitude'];
	xystr = ['28*ANTENNA1+ANTENNA2-(ANTENNA1-1)*(ANTENNA1+2)/2','AMPLITUDE(DATA[1:2,1:10])'];
	tp.plotdata(poption=pop,labels=labels,datastr=xystr);
	
	
	tp.markregions(nrows=2,ncols=1,panel=1,region=[200,400,0.05,0.3]);
	tp.flagdata();
	tp.markregions(nrows=2,ncols=1,panel=1,region=[300,350,0.07,0.25])
	tp.unflagdata();
	
        #pl.savefig('demo1.eps');


def tpdemo2(tp):
	
	tp.open(tabnames=[DATApath]);
	
	# clear the plot and underlying data structures
	tp.clearplot();
	
	# vistime for 10 chans (timeplot=1)
	pop = { 'nrows':2, 'ncols':1,'panel':1, 'windowsize':8.0, 'showflags':False ,'plotsymbol':',','timeplot':'x', 'overplot':False}
	labels = ['Timeplot','time','amplitude'];
	xystr = ['TIME/86400.0 + 678576.0','AMPLITUDE(DATA[1:2,1:10])'];
	tp.plotdata(poption=pop,labels=labels,datastr=xystr);
	
	# vischannel
	pop = { 'nrows':2, 'ncols':1,'panel':2, 'windowsize':8.0, 'showflags':False ,'plotsymbol':',','timeplot':'o', 'overplot':False}
	labels = ['Amplitude vs Channel number','chan','amplitude'];
	xystr = ['CROSS','AMPLITUDE(DATA[1:2,1:10])'];
	tp.plotdata(poption=pop,labels=labels,datastr=xystr);
	
def tpdemo3(tp):
	# clear the plot and underlying data structures
	tp.clearplot();
	
	# plot antenna locations (pointlabels is specified)
	tp.open(tabnames=[DATApathANT]);
	pop = {'nrows':1,'ncols':1, 'panel':1,  'windowsize':8.0, 'showflags':False , 'plotsymbol':'co','overplot':False, 'pointlabels':[' A1',' A2',' A3',' A4',' A5',' A6',' A7',' A8',' A9',' B1',' B2',' B3',' B4',' B5',' B6',' B7',' B8',' B9',' C1',' C2',' C3',' C4',' C5',' C6',' C7',' C8',' C9',' D1',' D2',' D3',' D4']}
	xystr = ['POSITION[1]','POSITION[2]'];
	labels = ['Antenna positions','x','y'];
	tp.plotdata(poption=pop,labels=labels,datastr=xystr);
	
def tpdemo4(tp):
	
	# clear the plot and underlying data structures
	tp.clearplot();
	
	#iteration plotting (changed aspectratio) (remember to change back the table)
	tp.open(tabnames=[DATApath]);
        tp.selectdata('FIELD_ID==0');
        pop = { 'nrows':3, 'ncols':1,'panel':1 };
        labels = ['Amplitude vs UVdist','uvdist','amplitude'];
        xystr = ['SQRT(SUMSQUARE(UVW[1:2]))','AMPLITUDE(DATA[1,1:10])'];
        iteraxes = ['ANTENNA1'];
        tp.iterplotstart(poption=pop,labels=labels,datastr=xystr,iteraxes=iteraxes);
        tp.iterplotnext();
        tp.iterplotnext();
        tp.iterplotnext();
        
        ## interrupt the iterplots with a regular plot.
        ## the iterplotstop gets called automatically.
	tp.open(tabnames=[DATApath]);
        tp.selectdata('FIELD_ID==0')
        pop = { 'nrows':2, 'ncols':1,'panel':1 };
        labels = ['Amplitude vs UVdist','uvdist','amplitude'];
        xystr = ['SQRT(SUMSQUARE(UVW[1:2]))','AMPLITUDE(DATA[1,1:10])'];
        tp.plotdata(poption=pop,labels=labels,datastr=xystr);
        tp.selectdata('FIELD_ID==1')
        pop = { 'nrows':2, 'ncols':1,'panel':2 };
        tp.plotdata(poption=pop,labels=labels,datastr=xystr);

def tpdemo5(tp):
        ''' averagenpoints test '''
        tp.open(tabnames=[DATApath]);
        tp.selectdata('FIELD_ID==0 && ANTENNA1==1 && ANTENNA2==26')
        labels = ['Amplitude vs Time','time','amplitude'];
        pop = { 'nrows':2, 'ncols':1,'panel':1, 'averagenpoints':1, 'timeplot':'x', 'plotsymbol':'r.'}
        xystr = ['TIME/86400.0 + 678576','MEAN(AMPLITUDE(IIF(FLAG[1,1:10],0.0,DATA[1,1:10])))'];
        tp.plotdata(poption=pop,labels=labels,datastr=xystr);
        pop.update( {'plotsymbol':'g+', 'overplot':True, 'averagenpoints':5, 'connect':'tablerow'} );
        tp.plotdata(poption=pop,labels=labels,datastr=xystr);
        xystr = ['TIME/86400.0 + 678576','MEAN(AMPLITUDE(IIF(FLAG[1,1:10],0.0,DATA[1,1:10])))'];
        pop.update( {'plotsymbol':'bx', 'overplot':True, 'averagenpoints':1} );
        tp.plotdata(poption=pop,labels=labels,datastr=xystr);
        
        tp.selectdata('FIELD_ID==0 && ANTENNA1==1 && ANTENNA2==26')
        labels = ['vis channel','chan','amplitude'];
        pop = { 'nrows':2, 'ncols':1,'panel':2, 'averagenpoints':1, 'timeplot':'o'}
        xystr = ['CROSS','AMPLITUDE(DATA[1,4:50])'];
        tp.plotdata(poption=pop,labels=labels,datastr=xystr);
        pop.update( {'overplot':True, 'connect':'cellcol','plotsymbol':'.','averagenpoints':100} ); 
        tp.plotdata(poption=pop,labels=labels,datastr=xystr);
        
        tp.markregions(nrows=2,ncols=1,panel=2,region=[20, 30, 1.3, 1.4]);
        
        tp.locatedata(['ANTENNA1','ANTENNA2']);


def tpdemo6(tp):
        ''' iterplot and overplot '''
        tp.open(tabnames=[DATApath]);
        tp.selectdata('FIELD_ID==0')
        pop = { 'nrows':3, 'ncols':1,'panel':1 };
        labels = ['Amplitude vs UVdist','uvdist','amplitude'];
        xystr = ['SQRT(SUMSQUARE(UVW[1:2]))','AMPLITUDE(DATA[1,1:10])'];
        iteraxes = ['ANTENNA1'];
        tp.iterplotstart(poption=pop,labels=labels,datastr=xystr,iteraxes=iteraxes);
        
        tp.open(tabnames=[DATApath]);
        tp.selectdata('FIELD_ID==1')
        pop = { 'nrows':3, 'ncols':1,'panel':1, 'overplot':True };
        tp.iterplotstart(poption=pop,labels=labels,datastr=xystr,iteraxes=iteraxes);
        
        tp.iterplotnext();

def tpdemo7(tp):
        ''' flag versions '''
        tp.open(tabnames=[DATApath]);
        tp.selectdata('FIELD_ID==0')
        pop = { 'nrows':1, 'ncols':1,'panel':1 };
        labels = ['Amplitude vs UVdist','uvdist','amplitude'];
        xystr = ['SQRT(SUMSQUARE(UVW[1:2]))','AMPLITUDE(DATA[1,1:10])'];
        tp.plotdata(poption=pop,labels=labels,datastr=xystr);
	tp.markregions(nrows=1,ncols=1,panel=1,region=[300,500,0.75,1.25]);
	tp.unflagdata();
        tp.saveflagversion('backup','a backup');
	tp.markregions(nrows=1,ncols=1,panel=1,region=[300,500,0.75,1.25]);
	tp.flagdata();
        tp.saveflagversion('new_1','a new one');
        tp.restoreflagversion('backup');
        pop = { 'nrows':2, 'ncols':1,'panel':1};
        tp.plotdata(poption=pop,labels=labels,datastr=xystr);
        pop = { 'nrows':2, 'ncols':1,'panel':2,'useflagversion':'new_1'};
        tp.plotdata(poption=pop,labels=labels,datastr=xystr);
        tp.getflagversionlist();



