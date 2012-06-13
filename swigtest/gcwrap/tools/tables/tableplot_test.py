# Test script for tableplot

#DATApath = "/home/basho2/urvashi/work/DATA/ngc5921.ms";
#DATApath2 = "/home/basho2/urvashi/work/DATA/ngc5921_copy.ms";
#DATApathANT = "/home/basho2/urvashi/work/DATA/ngc5921.ms/ANTENNA";
DATApath = "~/casa/testdata/ngc5921.ms";
DATApath2 = DATApath;
DATApathANT = "~/casa/testdata/ngc5921.ms/ANTENNA";


tp.open(tabnames=[DATApath]);

# uvdist for pol 1 and chan 1,2 on panel 211
pop = { 'nrows':2, 'ncols':1,'panel':1, 'plotcolour':1,'windowsize':8.0, 'showflags':false,'py_plotsymbol':',','timeplot':False, 'overplot':False, 'multipanel':True}
labels = ['Amplitude vs UVdist','uvdist','amplitude'];
xystr = ['SQRT(SUMSQUARE(UVW[1:2]))','AMPLITUDE(DATA[1,1:2])'];
tp.plotdata(poption=pop,labels=labels,datastr=xystr);

# uvdist for pol 2 and chan 1,2 (overplot=1) on panel 211
pop = { 'nrows':2, 'ncols':1,'panel':1, 'plotcolour':1,'windowsize':8.0, 'showflags':false,'py_plotsymbol':',','timeplot':False, 'overplot':True, 'multipanel':True}
labels = ['Amplitude vs UVdist','uvdist','amplitude'];
xystr = ['SQRT(SUMSQUARE(UVW[1:2]))','AMPLITUDE(DATA[2,1:2])'];
tp.plotdata(poption=pop,labels=labels,datastr=xystr);

# uv coverage on panel 223
pop = { 'nrows':2, 'ncols':2,'panel':3, 'plotcolour':1,'windowsize':8.0, 'showflags':false,'py_plotsymbol':',','timeplot':False, 'overplot':False, 'multipanel':True}
labels = ['UV Coverage','u','v'];
xystr = ['UVW[1]','UVW[2]','-UVW[1]','-UVW[2]'];
tp.plotdata(poption=pop,labels=labels,datastr=xystr);

# amp(data[1:2,1:10]) vs channel number on panel 224
pop = { 'nrows':2, 'ncols':2,'panel':4, 'plotcolour':1,'windowsize':8.0, 'showflags':false,'py_plotsymbol':',','timeplot':False, 'overplot':False, 'multipanel':True}
labels = ['Amplitude vs Baseline number','baseline number','amplitude'];
xystr = ['28*ANTENNA1+ANTENNA2-(ANTENNA1-1)*(ANTENNA1+2)/2','AMPLITUDE(DATA[1:2,1:10])'];
tp.plotdata(poption=pop,labels=labels,datastr=xystr);


tp.markflags(nrows=2,ncols=1,panel=1,region=[200,400,0.05,0.3]);
tp.flagdata();
tp.markflags(nrows=2,ncols=1,panel=1,region=[300,350,0.07,0.25])
tp.unflagdata();

# clear the plot and underlying data structures
tp.clearplot();

# vistime for 10 chans (timeplot=1)
pop = { 'nrows':2, 'ncols':1,'panel':1, 'plotcolour':1,'windowsize':8.0, 'showflags':false,'py_plotsymbol':',','timeplot':True, 'overplot':False, 'multipanel':False}
labels = ['Timeplot','time','amplitude'];
xystr = ['TIME','AMPLITUDE(DATA[1:2,1:10])'];
tp.plotdata(poption=pop,labels=labels,datastr=xystr);

# vischannel
pop = { 'nrows':2, 'ncols':1,'panel':2, 'plotcolour':1,'windowsize':8.0, 'showflags':false,'py_plotsymbol':',','timeplot':False, 'overplot':False, 'multipanel':True}
labels = ['Amplitude vs Channel number','chan','amplitude'];
xystr = ['CROSS','AMPLITUDE(DATA[1:2,1:10])'];
tp.plotdata(poption=pop,labels=labels,datastr=xystr);


# clear the plot and underlying data structures
tp.clearplot();

# plot antenna locations (pointlabels is specified)
tp.open(tabnames=[DATApathANT]);
pop = {'nrows':1,'ncols':1, 'panel':1, 'plotcolour':1, 'windowsize':8.0, 'showflags':false, 'py_plotsymbol':'o','timeplot':False, 'overplot':False, 'pointlabels':[' A1',' A2',' A3',' A4',' A5',' A6',' A7',' A8',' A9',' B1',' B2',' B3',' B4',' B5',' B6',' B7',' B8',' B9',' C1',' C2',' C3',' C4',' C5',' C6',' C7',' C8',' C9',' D1',' D2',' D3',' D4'], 'multipanel':False}
xystr = ['POSITION[1]','POSITION[2]'];
labels = ['Antenna positions','x','y'];
tp.plotdata(poption=pop,labels=labels,datastr=xystr);

# clear the plot and underlying data structures
tp.clearplot();

#iteration plotting (changed aspectratio) (remember to change back the table)
tp.open(tabnames=[DATApath]);
pop = { 'nrows':3, 'ncols':1,'panel':1, 'plotcolour':1,'windowsize':8.0, 'aspectratio':1.6, 'showflags':false,'py_plotsymbol':',','timeplot':False, 'overplot':False}
iteraxes = ['ANTENNA1'];
labels = ['Amplitude vs UVdist','uvdist','amplitude'];
xystr = ['SQRT(SUMSQUARE(UVW[1:2]))','AMPLITUDE(DATA[1,1:2])'];
tp.iterplotstart(poption=pop, labels=labels, datastr=xystr, iteraxes=iteraxes);
ret = tp.iterplotnext();


ret = tp.iterplotnext();
tp.iterplotstop();

# clear the plot and underlying data structures
tp.clearplot();

# uvdist for 2 chans. Each chan/pol : diff colour
# (multicolour=1, plotcolour>5)
pop = { 'nrows':1, 'ncols':1,'panel':1, 'plotcolour':6,'windowsize':8.0, 'showflags':false,'py_plotsymbol':',','timeplot':False, 'overplot':False, 'multicolour':True, 'multipanel':False}
labels = ['Amplitude vs UVdist','uvdist','amplitude'];
xystr = ['SQRT(SUMSQUARE(UVW[1:2]))','AMPLITUDE(DATA[1:2,1:2])'];
tp.plotdata(poption=pop,labels=labels,datastr=xystr);

tp.done()



