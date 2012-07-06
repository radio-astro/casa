#!/usr/bin/env python

from asap import *
from numpy import array

# don't print any messages
rcParams['verbose'] = 0
# load data unaveraged

scan = scantable("../test/data/tid-t002.rpf", average=False)
print scan

# select only one IF/Pol/Beam
scan.set_selection(ifs=0, beams=0, pols=0)

# plot in GHz
scan.set_unit("GHz")

# get all spectra into a matrix [ ncycle x nchan ]
mat = array([ spectrum for spectrum in scan ])

# find the min/max values
xax = scan.get_abcissa()
xmin = min(xax[0])
xmax = max(xax[0])

# a bit more complicate because we want to plot nice time axis labels
yax = scan.get_time(asdatetime=True)
ymin = min(xyplotter.date2num(yax))
ymax = max(xyplotter.date2num(yax))


# the date labels are a bit wide, so set the viewport
xyplotter.axes([0.2,0.1,0.7,0.8])

# plot the waterfall
xyplotter.imshow(mat, 
                 aspect='auto', # scale to fill the plot
                 origin='lower', # start at the bottom-left
                 extent=[xmin,xmax,ymin,ymax], # give the axes coordinates
                 interpolation='nearest'
                 )
xyplotter.gca().yaxis_date() # tell the plotter that the yaxis is using dates

# display axis labels 
xyplotter.xlabel(xax[1])
xyplotter.ylabel("Obs. Time")

xyplotter.savefig("test.png",orientation="landscape")
