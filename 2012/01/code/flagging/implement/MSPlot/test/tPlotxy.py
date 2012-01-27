casalog.filter('DEBUG2') 

default('plotxy')
vis='ngc5921.ms'
inp('plotxy')

average='both'
averagetime='90'
averagechan='4'

iterList = ['field', 'antenna', 'baseline', 'scan', 'feed']#, 'corr']

axisList = ['time', 'channel', 'uvdist', 'azimuth', 'elevation',
            'baseline', 'hourangle', 'parallacticangle', 'u', 'v', 'w', 'x',
            'frequency', 'correlation', 'real','imag','amp','phase','weight']

for k in range(len(axisList)):
   xaxis=axisList[k]
   print '\n---------', yaxis, 'vs', xaxis , ' 4 chan 90 sec'
   plotxy()


averagemode='scalar'
subplot=121

for k in range(len(iterList)):
   iteration=iterList[k]
   print '\n---------- iteration: ', iteration , ' 4 chan 90 sec'
   plotxy()

iteration=''
subplot=111

xaxis='real'
yaxis='imag'
averagemode='vector'
print '\n--------- ngc5921 imag vs real 4 chan 90 sec'
plotxy()

vis='../../data/3c273.ms'
print '\n--------- 3c273 imag vs real 4 chan 90 sec'
plotxy()

vis='ngc5921.ms'
print '\n--------- ngc5921 imag vs real 4 chan 90 sec'
plotxy()

mp.open('ngc5921.ms')

mp.setdata(averagemode='vector', averagetime='90', averagechan='4')
print '\n--------- mp vischannel 4 chan 90 sec'
mp.plot('vischannel')

mp.setdata(averagemode='vector', averagetime='90', averagechan='4')
print '\n--------- mp visfreq 4 chan 90 sec'
mp.plot('visfreq')

mp.setdata(averagemode='vector', averagetime='90', averagechan='4')
print '\n--------- mp vistime 4 chan 90 sec'
mp.plot('vistime')

mp.setdata(averagemode='vector', averagetime='90', averagechan='4')
print '\n--------- mp uvdist 4 chan 90 sec'
mp.plot('uvdist')

mp.setdata(averagemode='vector', averagetime='90', averagechan='4')
print '\n--------- mp uvcoverage 4 chan 90 sec'
mp.plot('uvcoverage')

mp.setdata(averagemode='vector', averagetime='90', averagechan='4')
print '\n--------- mp hourangle 4 cah 90 sec'
mp.plot('hourangle')

mp.setdata(averagemode='vector', averagetime='90', averagechan='4')
print '\n--------- mp viscorr 4 chan 90 sec'
mp.plot('viscorr')

mp.setdata(averagemode='vector', averagetime='90', averagechan='4')
print '\n--------- mp azimuth 4 chan 90 sec'
mp.plot('azimuth')

mp.setdata(averagemode='vector', averagetime='90', averagechan='4')
print '\n--------- mp elevation 4 chan 90 sec'
mp.plot('elevation')

mp.setdata(averagemode='vector', averagetime='90', averagechan='4')
print '\n--------- mp parallacticangle 4 chan 90 sec'
mp.plot('parallacticangle')

mp.setdata(averagemode='vector', averagetime='90', averagechan='4')
print '\n--------- mp weight 4 chan 90 sec'
mp.plot('weight')


xaxis='channel'
yaxis='amp'
vis='whysong.ms'
iteration='scan'
subplot=121
averagemode='vector'
print '\n--------- whysong.ms 4 chan 90 sec'
plotxy()

xaxis='time'
yaxis='amp'
vis='whysong.ms'
iteration='field'
subplot=121
averagemode='scalar'
averagetime='300'
averagechan='2'
print '\n--------- whysong.ms 2 chan 300 sec'
plotxy()


iteration=''
vis='ngc5921.ms'
subplot=121
spw=''
average='both'
averagetime='10'
averagechan='4'
print '\n--------- ngc5921.ms, 4 chan-only, new '
plotxy()
average='chan'
spw='*:0~89^4'
subplot=122
print '\n--------- ngc5921.ms, no-average'
plotxy()

vis='coma.ms'
subplot=121
spw=''
average='both'
averagetime='3'
averagechan='4'
print '\n--------- coma.ms, 4 chan-only, new '
plotxy()
average='chan'
spw='*:0~89^4'
subplot=122
print '\n--------- coma.ms, no-average'
plotxy()


vis='ngc5921.ms'
spw=''
average='both'
averagetime='90'
averagechan='4'
xaxis='channel'
plotxy

averagemode='vector'
subplot=121
print '\n--------- ngc5921.ms, vector average 2 chan 90 sec'
plotxy()
averagemode='scalar'
subplot=122
print '\n--------- ngc5921.ms, scalar average 2 chan 90 sec'
plotxy()

averagemode='vector'
subplot=121
yaxis='real'
print '\n--------- ngc5921.ms, real part average 2 chan 90 sec'
plotxy()
averagemode='vector'
subplot=122
yaxis='imag'
print '\n--------- ngc5921.ms, image part average 2 chan 90 sec'
plotxy()

averagemode='vector'
subplot=121
yaxis='phase'
xaxis='time'
print '\n--------- ngc5921.ms, phase average 2 chan 90 sec'
plotxy()
averagemode='vector'
subplot=122
yaxis='amp'
xaxis='channel'
print '\n--------- ngc5921.ms, amplitude average 2 chan 90 sec'
plotxy()

vis='ngc5921.ms'
subplot=121
spw=''
average='both'
averagetime='10'
averagechan='1'
print '\n--------- ngc5921.ms, no average with averager'
plotxy()
average=''
spw=''
subplot=122
print '\n--------- ngc5921.ms, no average '
plotxy()


print "\ndone"
