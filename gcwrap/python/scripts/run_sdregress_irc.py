#####################################
#
# IRC10216 SDtasks Regression Script
# Nod data
# Version STM 2007-03-02
#
#####################################
import time
import os

casapath=os.environ['CASAPATH']

os.system('rm -rf IRC+10216_rawACSmod sdregress_irc_* ')
datapath=casapath.split()[0]+'/data/regression/ATST5/IRC+10216/IRC+10216_rawACSmod'
#datapath='/home/rohir3/jmcmulli/SD/IRC+10216_rawACSmod'
copystring='cp -r '+datapath+' .'
os.system(copystring)

#restore()

##########################
#
# IRC+10216 SiO 
# Nod data
#
##########################
startTime=time.time()
startProc=time.clock()

# Set parameters
default('sdreduce')
infile = 'IRC+10216_rawACSmod'
telescope = 'FIX'
fluxunit = 'K'
specunit = 'channel'
calmode = 'nod'
average=True
scanaverage = False
timeaverage = True
tweight='tintsys' 
polaverage = True
pweight='tsys'
tau = 0.09
scanlist = [240,241,242,243,244,245,246,247]
iflist = [30]
kernel = 'boxcar'
kwidth = 5
maskmode='auto'
thresh=5
blfunc='poly'
order=1
avg_limit=4
masklist = []
#edge = [500]
outfile = 'sdregress_irc_sio.asap'
outform = 'asap'
plotlevel = 0

sdreduce()

# No line fitting, the lines are
# broad double-peaked

# Now do some region statistics
# First the line-free region
# Set parameters
default('sdstat')
infile = 'sdregress_irc_sio.asap'
fluxunit = 'K'
specunit = 'channel'
masklist = [[800,1500]]
invertmask = False
xstat = {}

xstat=sdstat()

off_stat = xstat

# Then the line region
# Set parameters
infile = 'sdregress_irc_sio.asap'
fluxunit = 'K'
specunit = 'channel'
masklist = [[1850,2300]]
invertmask = False
xstat = {}

xstat=sdstat()

line_stat = xstat

endProc = time.clock()
endTime = time.time()

project = 'IRC10216-SiO'
prolog  = 'sdregress.irc.sio.'
# Compare to Joe's regression values
# irc_sio_max=0.611
# irc_sio_rms=0.018
# irc_sio_sum=157.074
# Regression values of CASA 2.3(#6654)+ASAP 2.2.0(#1448)
# on 64bit REL5.2 (2008/12/01)
prev_max=0.6180
prev_rms=0.01853
prev_sum=159.8

new_rms = off_stat['rms']
new_max = line_stat['max']
new_sum = line_stat['sum']
diff_max = abs((prev_max-new_max)/prev_max)
diff_rms = abs((prev_rms-new_rms)/prev_rms)
diff_sum = abs((prev_sum-new_sum)/prev_sum)

difpass_max = (diff_max < 0.05)
difpass_rms = (diff_rms < 0.05)
difpass_sum = (diff_sum < 0.05)
difpass = difpass_max & difpass_rms & difpass_sum

pass_regress = difpass

if (difpass_max): print '* Passed spectrum max test '
if (difpass_rms): print '* Passed spectrum rms test '
if (difpass_sum): print '* Passed spectrum sum test'
if (difpass):
        print '---Passed Stat test for '+project
else:
        print '---FAILED Stat test for '+project
print ' '
print 'Total wall clock time was: '+str(endTime - startTime)
print 'Total CPU        time was: '+str(endProc - startProc)
#print 'Processing rate MB/s  was: ', 35.1/(endTime - startTime)

#
# NOW TO REGRESSION LOGFILE 
#
import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile=prolog+datestring+'.log'
logfile=open(outfile,'w')

print >>logfile,''
print >>logfile,'************ Regression ****************'
print >>logfile,'*                                      *'
if (difpass_max): print >>logfile,'* Passed spectrum max test '
print >>logfile,'*  Spectrum max '+str(new_max)
if (difpass_rms): print >>logfile,'* Passed spectrum rms test '
print >>logfile,'*  Spectrum rms '+str(new_rms)
if (difpass_sum): print >>logfile,'* Passed spectrum sum test'
print >>logfile,'*  Spectrum sum '+str(new_sum)
if (difpass):
        print >>logfile,'---'
        print >>logfile,'Passed Stat test for '+project
else:
        print >>logfile,'---'
        print >>logfile,'FAILED Stat test for '+project
print >>logfile,'****************************************'
print >>logfile,''
print >>logfile,''
print >>logfile,'************ Benchmarking **************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
#print >>logfile,'Processing rate MB/s  was: ', 35.1/(endTime - startTime)

logfile.close()

##########################
#
# IRC+10216 HC3N
# Nod data
#
##########################
startTime=time.time()
startProc=time.clock()

# Set parameters
default('sdreduce')
infile = 'IRC+10216_rawACSmod'
telescope = 'FIX'
fluxunit = 'K'
specunit = 'channel'
calmode = 'nod'
average=True
scanaverage = False
timeaverage = True
tweight='tintsys' 
polaverage = True
pweight='tsys'
tau = 0.09
scanlist = [236,237,238,239,248,249,250,251]
iflist = [17]
kernel = 'boxcar'
kwidth = 5
maskmode='auto'
thresh=5
blfunc='poly'
order=2
avg_limit=4
#edge = [500]
masklist = []
outfile = 'sdregress_irc_hc3n.asap'
outform = 'asap'
plotlevel = 0

sdreduce()

# No line fitting, the lines are
# broad double-peaked

# Now do some region statistics
# First the line-free region
# Set parameters
default('sdstat')
infile = 'sdregress_irc_hc3n.asap'
fluxunit = 'K'
specunit = 'channel'
masklist = [[200,1500]]
invertmask = False
xstat = {}

xstat=sdstat()

off_stat = xstat

# Then the line region
# Set parameters
infile = 'sdregress_irc_hc3n.asap'
fluxunit = 'K'
specunit = 'channel'
masklist = [[1800,2400]]
invertmask = False
xstat = {}

xstat=sdstat()

line_stat = xstat

endProc = time.clock()
endTime = time.time()

project = 'Irc-HC3N'
prolog  = 'sdregress.irc.hc3n.'
# Compare to Joe's regression values
# irc_hc3n_max=1.827
# irc_hc3n_rms=0.022
# irc_hc3n_sum=474.123
#
# Regression values of CASA 2.3(#6654)+ASAP 2.2.0(#1448)
# on 64bit REL5.2 (2008/12/01)
prev_max=1.827
prev_rms=0.02213
prev_sum=474.1

new_rms = off_stat['rms']
new_max = line_stat['max']
new_sum = line_stat['sum']
diff_max = abs((prev_max-new_max)/prev_max)
diff_rms = abs((prev_rms-new_rms)/prev_rms)
diff_sum = abs((prev_sum-new_sum)/prev_sum)

difpass_max = (diff_max < 0.05)
difpass_rms = (diff_rms < 0.05)
difpass_sum = (diff_sum < 0.05)
difpass = difpass_max & difpass_rms & difpass_sum

pass_regress = pass_regress & difpass

if (difpass_max): print '* Passed spectrum max test '
if (difpass_rms): print '* Passed spectrum rms test '
if (difpass_sum): print '* Passed spectrum sum test'
if (difpass):
        print '---Passed Stat test for '+project
else:
        print '---FAILED Stat test for '+project
print ' '
print 'Total wall clock time was: '+str(endTime - startTime)
print 'Total CPU        time was: '+str(endProc - startProc)
#print 'Processing rate MB/s  was: ', 35.1/(endTime - startTime)

#
# NOW TO REGRESSION LOGFILE 
#
import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile=prolog+datestring+'.log'
logfile=open(outfile,'w')

print >>logfile,''
print >>logfile,'************ Regression ****************'
print >>logfile,'*                                      *'
if (difpass_max < 0.05): print >>logfile,'* Passed spectrum max test '
print >>logfile,'*  Spectrum max '+str(new_max)
if (difpass_rms < 0.05): print >>logfile,'* Passed spectrum rms test '
print >>logfile,'*  Spectrum rms '+str(new_rms)
if (difpass_sum < 0.05): print >>logfile,'* Passed spectrum sum test'
print >>logfile,'*  Spectrum sum '+str(new_sum)
if (difpass):
        print >>logfile,'---'
        print >>logfile,'Passed Stat test for '+project
else:
        print >>logfile,'---'
        print >>logfile,'FAILED Stat test for '+project
print >>logfile,'****************************************'
print >>logfile,''
print >>logfile,''
print >>logfile,'************ Benchmarking **************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
#print >>logfile,'Processing rate MB/s  was: ', 35.1/(endTime - startTime)

logfile.close()

##########################
#
# IRC+10216 13CS
# Nod data
#
##########################
startTime=time.time()
startProc=time.clock()

# Set parameters
default('sdreduce')
infile = 'IRC+10216_rawACSmod'
telescope = 'FIX'
fluxunit = 'K'
specunit = 'channel'
calmode = 'nod'
average=True
scanaverage = False
timeaverage = True
tweight='tintsys' 
polaverage = True
pweight='tsys'
tau = 0.09
scanlist = [229,230]
iflist = [3]
kernel = 'boxcar'
kwidth = 5
maskmode='auto'
thresh=5
blfunc='poly'
order = 1
avg_limit=4
#edge = [500]
masklist = []
outfile = 'sdregress_irc_cs.asap'
outform = 'asap'
plotlevel = 0

sdreduce()

# No line fitting, the lines are
# broad double-peaked

# Now do some region statistics
# First the line-free region
# Set parameters
default('sdstat')
infile = 'sdregress_irc_cs.asap'
fluxunit = 'K'
specunit = 'channel'
masklist = [[800,1500]]
invertmask = False
xstat = {}

xstat=sdstat()

off_stat = xstat

# Then the line region
# Set parameters
infile = 'sdregress_irc_cs.asap'
fluxunit = 'K'
specunit = 'channel'
masklist = [[1850,2300]]
invertmask = False
xstat = {}

xstat=sdstat()

line_stat = xstat

endProc = time.clock()
endTime = time.time()

project = 'IRC10216-13CS'
prolog  = 'sdregress.irc.cs.'
# Compare to Joe's regression values
# irc_cs_max=3.324
# irc_cs_rms=0.1473
# irc_cs_sum=627.587
#
# Regression values of CASA 2.3(#6654)+ASAP 2.2.0(#1448)
# on 64bit REL5.2 (2008/12/01)
prev_max=3.325
prev_rms=0.1473
prev_sum=627.9

new_rms = off_stat['rms']
new_max = line_stat['max']
new_sum = line_stat['sum']
diff_max = abs((prev_max-new_max)/prev_max)
diff_rms = abs((prev_rms-new_rms)/prev_rms)
diff_sum = abs((prev_sum-new_sum)/prev_sum)

difpass_max = (diff_max < 0.05)
difpass_rms = (diff_rms < 0.05)
difpass_sum = (diff_sum < 0.05)
difpass = difpass_max & difpass_rms & difpass_sum

pass_regress = pass_regress & difpass

if (difpass_max): print '* Passed spectrum max test '
if (difpass_rms): print '* Passed spectrum rms test '
if (difpass_sum): print '* Passed spectrum sum test'
if (difpass):
        print '---Passed Stat test for '+project
else:
        print '---FAILED Stat test for '+project
print ' '
print 'Total wall clock time was: '+str(endTime - startTime)
print 'Total CPU        time was: '+str(endProc - startProc)
#print 'Processing rate MB/s  was: ', 35.1/(endTime - startTime)

#
# NOW TO REGRESSION LOGFILE 
#
import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile=prolog+datestring+'.log'
logfile=open(outfile,'w')

print >>logfile,''
print >>logfile,'************ Regression ****************'
print >>logfile,'*                                      *'
if (difpass_max < 0.05): print >>logfile,'* Passed spectrum max test '
print >>logfile,'*  Spectrum max '+str(new_max)
if (difpass_rms < 0.05): print >>logfile,'* Passed spectrum rms test '
print >>logfile,'*  Spectrum rms '+str(new_rms)
if (difpass_sum < 0.05): print >>logfile,'* Passed spectrum sum test'
print >>logfile,'*  Spectrum sum '+str(new_sum)
if (difpass):
        print >>logfile,'---'
        print >>logfile,'Passed Stat test for '+project
else:
        print >>logfile,'---'
        print >>logfile,'FAILED Stat test for '+project
print >>logfile,'****************************************'
print >>logfile,''
print >>logfile,''
print >>logfile,'************ Benchmarking **************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
#print >>logfile,'Processing rate MB/s  was: ', 35.1/(endTime - startTime)

logfile.close()

if ( pass_regress ):
	print "All regressions passed"
else:
	print "WARNING: There were some regressions FAILED"

##########################
#
# End IRC+10216 Regression
#
##########################
