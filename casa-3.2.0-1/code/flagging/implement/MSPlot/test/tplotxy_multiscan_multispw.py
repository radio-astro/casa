taskname = 'plotxy'
default(taskname)
vis = 'n4826_16apr98.ms'
selectdata = true
correlation = ''
spw = ''
antenna = ''
timerange = ''
yaxis = 'amp'
averagemode = 'vector'
width = '1'
timebin = '0'
subplot=111
overplot = false
plotsymbol = '.'
plotcolor = 'green'

selectplot=true

crossscans=false
subplot=231
xaxis='time'
title='width=1,timebin=0'
plotxy()

field = '2'
spw='2'

crossscans=false
subplot=232
xaxis='time'
width='2'
timebin='6000'
title='field=2,spw=2,width=2,timebin=6000,crossscans=0'
plotxy()

crossscans=true
subplot=233
xaxis='time'
timebin='6000'
title='field=2,spw=2,width=2,timebin=6000,crossscans=1'
plotxy()

crossscans=false
subplot=234
timebin='60000'
xaxis='time'
title='field=2,spw=2,width=2,timebin=60000,crossscans=0'
plotxy()

crossscans=true
subplot=235
timebin='60000'
title='field=2,spw=2,width=2,timebin=60000,crossscans=1'
xaxis='time'
plotxy()

crossscans=true
subplot=236
timebin='60000'
antenna='2'
width='4'
title='field=2,spw=2,width=16,timebin=60000,crossscans=1,antenna=2'
xaxis='time'
plotxy()


crossscans=true
subplot=231
field='4'
width='16'
antenna='2&3'
xaxis='time'
timebin='6000'
title='field=2,spw=2,width=16,timebin=6000,crossscans=1'
connect='chan'
plotxy()

crossscans=false
subplot=234
timebin='6000'
xaxis='time'
title='field=2,spw=2,width=16,timebin=6000,crossscans=0'
plotxy()

