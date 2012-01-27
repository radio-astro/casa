# Sample script that use all of the plotting tools on the same
# plotting window!
#
# All of the plots are, relatively simple plots (no averaging)
#
# TablePlot (tp tool): Plots uvdist vs. amplitude
# MsPlot (mp tool):    Plots the same thing as the tp tool
# CalPlot( cp tool):   Plots the amplitude.
#

# Table Plot
tp.open(['~/DATA/ngc5921.ms']);
tp.selectdata('FIELD_ID==1')
pop = { 'nrows':1, 'ncols':1,'panel':1, 'multicolour':'none','overplot':True };
labels = ['Amplitude vs UVdist','uvdist','amplitude'];
xystr = ['SQRT(SUMSQUARE(UVW[1:2]))','AMPLITUDE(DATA[1:2,1:5])'];
tp.plotdata(poption=pop,labels=labels,datastr=xystr);

# MS Plot
mp.open('~/DATA/ngc5921.ms')
mp.setdata(field='0', spw='>0:0~4');
mp.plot( 'uvdist' );

# Cal Plot
cp.open('~/DATA/0814+459.bcal');
cp.plot(plottype=AMP);

