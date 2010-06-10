#!/usr/bin/env python 
# run this script with a command line parameter "wait" in order it to wait
# pressing enter instead of a time delay

from asap import *
import time,sys

wait_str = ""
if len(sys.argv)>=2:
   if sys.argv[1].lower() == "wait":
      wait_str = " Press Enter."

# read the spectrum from ascii file
f = open('data/atca_spectrum.dat')
spc=[]
try:
   for line in f:
      parts=line.split()
      if len(parts)!=3:
         raise RuntimeError, "Expect 3 parts, you have %s"% (parts,)
      spc.append(float(parts[2]))
finally:
   f.close()   


#run the line finder

lf = simplelinefinder()
rng=lf.find_lines(spc,threshold=3,splitFeatures=False)

print "Found %i lines.%s" % (len(rng),wait_str)
b=len(spc)/4
e=len(spc)*3/4
plotter._plotter.plot(range(b,e),spc[b:e])
plotter._plotter.set_line(number=0,label='Input spectrum')
plotter._plotter.show()
if wait_str == "":
   time.sleep(1.5)
else:
   raw_input()
cnt = 1
for line in rng:
   if len(line)!=2:
      raise RuntimeError, "Line is supposed to have two parameters"
   b = line[0]-1
   if b<0:
      b=0
   e = line[1]+1
   if e>len(spc):
      e=len(spc)
   print "Showing line %i located at %s.%s" % (cnt,line,wait_str)
   plotter._plotter.set_line(label='Line %i' % cnt)
   cnt = cnt + 1
   plotter._plotter.plot(range(b,e),spc[b:e])
   plotter._plotter.show()
   if wait_str == "":
      time.sleep(1.5)
   else:
      raw_input()
      
plotter.save('output/simple_linefinder.png', dpi=80)
print "Test of simple line finder is successful"
