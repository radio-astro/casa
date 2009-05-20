taskname = 'plotxy'
default(taskname)
vis = 'testPhase.sdm.ms'
field = ''
selectdata = true
correlation = ''
antenna = ''
timerange = ''
yaxis = 'amp'
averagemode = 'vector'
width = '2'
timebin = '60'
overplot = false
plotsymbol = '.'
plotcolor = 'green'

subplot=231
spw = '0,1,2,3'
xaxis='channel'
plotxy()

subplot=232
spw = '4,5'
xaxis='channel'
plotxy()

subplot=233
spw = '6,7'
xaxis='channel'
plotxy()

subplot=234
spw = '0,1,2,3'
xaxis='frequency'
plotxy()

subplot=235
spw = '4,5'
xaxis='frequency'
plotxy()

subplot=236
spw = '6,7'
xaxis='frequency'
plotxy()
