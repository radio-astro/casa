from xyplot import *;
xy = XYPlot(tp,gui=True);

DATA = '~/DATA/ngc5921.ms';

# open the MS
xy.open(DATA);

# UVdist plot all data points, overlaid with channel average.
xy.selectdata('FIELD_ID==1');
xy.setdata(startcorr=1,endcorr=2,startchan=5,endchan=50,col='data',quant='amp');
xy.setpanel(1,1,1);
xy.setplotoptions();
xy.plot(type='uvdist');
xy.setplotoptions(mean=2,plotcolour='green',overplot=True,multicolour=True);
xy.plot(type='uvdist');
xy.setplotoptions(overplot=True,showflags=True);
xy.plot(type='uvdist');


# Same as above, but with separate panels
xy.setpanel(1,2,1);
xy.setplotoptions(plotrange=[0.0,5.0,0.06,0.46]);
xy.plot(type='uvdist');
xy.setpanel(1,2,2);
xy.setplotoptions(mean=2,multicolour=True,plotrange=[0.0,5.0,0.06,0.46]);
xy.plot(type='uvdist');

# Vis-Channel plot with channel averaging...
xy.setpanel(2,1,1);
xy.setdata(startcorr=1,endcorr=2,startchan=5,endchan=50,stepchan=1);
xy.setplotoptions(plotrange=[4.0,51.0,0.0,0.0]);
xy.plot(type='vischannel');
xy.setpanel(2,1,2);
xy.setdata(startcorr=1,endcorr=2,startchan=5,endchan=50,stepchan=5);
xy.setplotoptions(mean=2,aswidth=True,plotrange=[4.0,51.0,0.0,0.0]);
xy.plot(type='vischannel');

# Other simple demo plots...
xy.setdata(startcorr=1,endcorr=2,startchan=5,endchan=50,stepchan=1,col='data',quant='amp');
xy.setpanel(2,2,1);
xy.setplotoptions();
xy.plot(type='baseline');
xy.setpanel(2,2,3);
xy.setplotoptions(multicolour=True);
xy.plot(type='vischannel');
xy.setpanel(2,2,4);
xy.setplotoptions(multicolour=True);
xy.plot(type='viscorrelation');

# Visibilities vs time
xy.setpanel(1,1,1);
xy.setplotoptions();
xy.plot(type='vistime');
xy.setplotoptions(mean=2,plotsymbol='.',overplot=True,multicolour=True);
xy.plot(type='vistime');

xy.setpanel(1,1,1);
xy.setdata(startchan=5,endchan=40,stepchan=10,startcorr=1,endcorr=2)
xy.setplotoptions(aswidth=True);
xy.plot(type='uvdist');
xy.plot(type='baseline');
xy.plot(type='vistime');
xy.plot(type='vischannel');

# Iteration plots with overplot
xy.clearplot();

xy.open(DATA);
xy.setpanel(3,1,1);
xy.setplotoptions();
xy.setdata(startchan=1,endchan=10,stepchan=2);
xy.plot(type='uvdist',iteraxes='ANTENNA1');
xy.setplotoptions(overplot=True);
xy.setdata(startchan=11,endchan=20,stepchan=2);
xy.plot(type='uvdist',iteraxes='ANTENNA1');

# can be done from the gui.
# need not call iterplotstop explicitly.
xy.iterplotnext();
xy.iterplotnext();
xy.iterplotstop();

# Other plots...
xy.clearplot();
xy.open(DATA);
xy.setpanel(1,1,1);
xy.uvcoverage();
xy.array();

# Time averaging... and lines.
xy.clearplot();
xy.open(DATA);
xy.selectdata('FIELD_ID==0 && ANTENNA1==1 && ANTENNA2==26');
xy.setpanel(2,1,1);
xy.setdata(startchan=1,endchan=10,startcorr=1,endcorr=2);
# all data points.
xy.setplotoptions(averagenpoints=1,timeplot='x',plotsymbol='r.');
xy.plot(type='vistime');
# average every 5 timestamps, and also average all channels. Join by lines
xy.setplotoptions(mean=2,averagenpoints=5,timeplot='x',plotsymbol='go',overplot=True,connect='tablerow');
xy.plot(type='vistime');
# average all channels only, and join by lines.
xy.setplotoptions(mean=2,averagenpoints=1,timeplot='x',plotsymbol='b.',overplot=True);
xy.plot(type='vistime');

xy.setpanel(2,1,2);
xy.setdata(startchan=1,endchan=10,startcorr=1,endcorr=2);
# all data points.
xy.setplotoptions(plotsymbol='r.');
xy.plot(type='vischannel');
xy.setdata(startchan=1,endchan=10,startcorr=1,endcorr=1);
# Average all timestamps, and join channels by lines. ( RR )
xy.setplotoptions(overplot=True, connect='cellcol', plotsymbol='g+',averagenpoints=100);
xy.plot(type='vischannel');
xy.setdata(startchan=1,endchan=10,startcorr=2,endcorr=2);
# Average all timestamps, and join channels by lines. ( LL )
xy.setplotoptions(overplot=True, connect='cellcol', plotsymbol='b+',averagenpoints=100);
xy.plot(type='vischannel');


# UVdist plot with iteration over the first 10 antennas.
# background is with all baselines.
# foreground is one antenna at a time, with only the top layer being replaced
# between iterations.
xy.clearplot();
xy.setpanel(1,1,1);
xy.selectdata('FIELD_ID==0');
xy.setdata(startchan=4,endchan=50,startcorr=1,endcorr=2);
xy.setplotoptions(plotsymbol='r,');
xy.plot(type='uvdist');
xy.setplotoptions(overplot=True, plotsymbol='b,');
for ant in range(1,10):
	xy.selectdata('FIELD_ID==0 && (ANTENNA1=='+str(ant)+' || ANTENNA2=='+str(ant)+')');
	xy.plot('uvdist');
	xy.setplotoptions(overplot=True,replacetopplot=True, plotsymbol='b,');
        xy.plot(type='uvdist');

###########

