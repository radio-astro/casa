#!/usr/bin/env python
# run this script with a command line parameter "wait" in order it to wait
# pressing enter instead of a time delay

from asap import *
import time,sys

input_dataset = "data/mopra_g327.3-0.6.sdfits"

# read command line parameter
wait_str = ""
if len(sys.argv)>=2:
   if sys.argv[1].lower() == "wait":
      wait_str = " Press Enter."

# read scantable
sc=scantable(input_dataset)
# this file is already reduced, so we can just plot it

plotter.plot(sc)
plotter.set_legend('Input spectrum')
#plotter._plotter.set_line(number=0,label='Input spectrum')

# now set up and run line finder
fl=linefinder()
fl.set_scan(sc)
fl.set_options(avg_limit=1,threshold=4,box_size=0.1,noise_box=0.1,noise_stat='median')
nlines=fl.find_lines()
print 'Found %i lines.' % nlines

if nlines!=0:
   r=fl.get_ranges()
   print "Channel ranges: %s" % (r,)
   print wait_str
   if wait_str == "":
      time.sleep(1)
   else:
      raw_input()
   for l in range(1,len(r),2):
        print "Line %i from channel %i to %i.%s" % (l/2+1,r[l-1],r[l],wait_str)
        #plotter.set_range(r[l-1]-3,r[l]+3)
        b=int(r[l-1])-3
        e=int(r[l])+3
        if b<0:
           b=0
        if e>=sc.nchan():
           e=sc.nchan()
        add_colours = ['red','blue','pink','brown']
        if l/2 >= 9:
           plotter._plotter.set_line(colour=add_colours[(l/2-9)%len(add_colours)])
        plotter._plotter.set_line(label='Line %i'% (l/2+1))
        plotter._plotter.plot(range(b,e),sc.get_spectrum(0)[b:e])
        plotter._plotter.show()
        if wait_str == "":
           time.sleep(1)
        else:
           raw_input()
          

plotter.save('output/linefinder.png', dpi=80)
print "Test of line finder is successfull"
