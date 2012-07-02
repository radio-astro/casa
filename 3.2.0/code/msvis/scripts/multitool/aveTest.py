# Sample script that does averaging via the tp and mp tools.  This
# is a good script to verify the correctness of averaging.


########## these two need to be the same.
#####  Please manually flag a few "red" points ( the top 5 ) 
#####  for a couple of timestamps, and see how the averages move.

#################  Channel with 1 correlation scalar ############
######## tableplot

tp.open('/u/jaeger/casa/testdata/MSs/ngc5921.ms')
tp.selectdata('FIELD_ID==0 && ANTENNA1==1 && ANTENNA2==26')
labels = ['Amplitude vs Time','time','amplitude'];
pop = { 'nrows':2, 'ncols':1,'panel':1, 'timeplot':'x', 'plotsymbol':'r.','doscalingcorrection':False,'fontsize':8.0}
xystr = ['TIME/86400.0 + 678576','AMPLITUDE(DATA[1,1:10])'];
tp.plotdata(poption=pop,labels=labels,datastr=xystr);
xystr = ['TIME/86400.0 + 678576','SUM(AMPLITUDE(IIF(FLAG[1,1:10],0.0,DATA[1,1:10])))/SUM(IIF(FLAG[1,1:10],0.0,1.0))'];
pop.update( {'plotsymbol':'go', 'overplot':True } );
tp.plotdata(poption=pop,labels=labels,datastr=xystr);

######### msplot

mp.open('/u/jaeger/casa/testdata/MSs/ngc5921.ms')
mp.setdata(field='0',baseline='0&25',spw='0:0~9',correlation='RR')
mp.plotoptions(overplot=False,plotsymbol='.',subplot=212, fontsize=8.0);
mp.plot('vistime');
mp.setdata(field='0',baseline='0&25',spw='0:0~9^1',correlation='RR',corravemode='scalarstep',chanavemode="scalarstep");
mp.plotoptions(overplot=True,plotcolor='green',plotsymbol='o',subplot=212, fontsize=8.0);
mp.plot('vistime');


###############   Channels and 2 correlations vector average ############
######## tableplot

tp.open('/u/jaeger/casa/testdata/MSs/ngc5921.ms')
tp.selectdata('FIELD_ID==0 && ANTENNA1==1 && ANTENNA2==26')
labels = ['Amplitude vs Time','time','amplitude'];
pop = { 'nrows':2, 'ncols':1,'panel':1, 'timeplot':'x', 'plotsymbol':'r.','doscalingcorrection':False,'fontsize':8.0}
xystr = ['TIME/86400.0 + 678576','AMPLITUDE(DATA[1:2:1,10:29:5])'];
tp.plotdata(poption=pop,labels=labels,datastr=xystr);
xystr = ['TIME/86400.0 + 678576','AMPLITUDE(SUM(IIF(FLAG[1:2:1,10:14:1],0.0,DATA[1:2:1,10:14:1])))/SUM(IIF(FLAG[1:2:1,10:14:1],0.0,1.0))',
    'TIME/86400.0 + 678576','AMPLITUDE(SUM(IIF(FLAG[1:2:1,15:19:1],0.0,DATA[1:2:1,15:19:1])))/SUM(IIF(FLAG[1:2:1,15:19:1],0.0,1.0))',
    'TIME/86400.0 + 678576','AMPLITUDE(SUM(IIF(FLAG[1:2:1,20:24:1],0.0,DATA[1:2:1,20:24:1])))/SUM(IIF(FLAG[1:2:1,20:24:1],0.0,1.0))',
    'TIME/86400.0 + 678576','AMPLITUDE(SUM(IIF(FLAG[1:2:1,25:29:1],0.0,DATA[1:2:1,25:29:1])))/SUM(IIF(FLAG[1:2:1,25:29:1],0.0,1.0))'];
pop.update( {'plotsymbol':'go', 'overplot':True } );
tp.plotdata(poption=pop,labels=labels,datastr=xystr);

######### msplot

mp.open('/u/jaeger/casa/testdata/MSs/ngc5921.ms')
mp.setdata(field='0',baseline='0&25',spw='0:9~28^5',correlation='RR,LL' )
mp.plotoptions(overplot=False,plotsymbol='.',subplot=212,fontsize=8.0);
mp.plot('vistime');
mp.setdata(field='0',baseline='0&25',spw='0:9~28^5',correlation='RR,LL',corravemode='vectorchunk',chanavemode="vectorchunk");
mp.plotoptions(overplot=True,plotcolor='green',plotsymbol='o',subplot=212,fontsize=8.0);
mp.plot('vistime');

