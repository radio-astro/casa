import time;

startTime1=time.time();
plotxy( subplot=211, vis='/Users/shannonjaeger/casa/testdata/ggtau_07feb97.ms', xaxis='uvdist', datacolumn='residual' );
endTime1=time.time();


# In order to do an accurate time test the ms plot tool is reset, to free
# up memory.  However, to verify that these two things plot the same thing
# comment the couple of lines below and let her rip!
#
# Things get slower because TablePlot needs to store more and more tables
# without the reset.
#mp.clearplot();
#mp.reset();

default();
startTime2=time.time();
for i in range(0,23):
	plotxy( subplot=212, vis='/Users/shannonjaeger/casa/testdata/ggtau_07feb97.ms', xaxis='uvdist', datacolumn='residual', overplot=true, spw=str(i) );
endTime2=time.time();


print "Time for auto-msplot:     " + str(endTime1-startTime1);
print "Time for msplot in scrpt: " + str(endTime2-startTime2);

#mp.close();
