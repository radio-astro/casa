taskname = 'plotxy'
default(taskname)
vis = '3C84.ms'
field = '0'
selectdata = true
correlation = 'RR'
spw = '0:5~6'
antenna = 'VA04&VA08'
timerange = '10:39:00~10:40:00'
yaxis = 'phase'
xaxis='time'
averagemode = 'vector'
width = '1'
timebin = '0'
overplot = false
plotsymbol = '.'
plotcolor = 'red'
plotxy()


width ='2'
timebin = '0'
overplot = true
plotsymbol = 'o'
plotcolor = 'green'
plotxy()


width ='2'
timebin = '20'
overplot=true
plotsymbol = 'o'
plotcolor = 'blue'
plotxy()

width ='2'
timebin = '30'
overplot=true
plotsymbol = 'o'
plotcolor = 'yellow'
plotxy()

width ='2'
timebin = '40'
overplot=true
plotsymbol = 'o'
plotcolor = 'pink'
plotxy()

width ='2'
timebin = '50'
overplot=true
plotsymbol = 'o'
plotcolor = 'black'
plotxy()

width ='2'
timebin = '60'
overplot=true
plotsymbol = 'x'
plotcolor = 'black'
plotxy()

width ='2'
timebin = '70'
overplot=true
plotsymbol = 'x'
plotcolor = 'black'
plotxy()
