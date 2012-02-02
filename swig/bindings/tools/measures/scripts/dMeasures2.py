#import sys

class tee_stdout:

    def __init__(self, old_stdout, filename):
        self.old_stdout = old_stdout
        self.new_stdout = open(filename, 'w+')

    def write(self, item):
        self.old_stdout.write(item)
        self.new_stdout.write(item)

#sys.stdout = tee_stdout(sys.stdout, 'measuresExample.out')

print "\n 2. Get the time now as an epoch measure"
tim = me.epoch('utc','today')
print tim
print "\n 3. Get the position of an observatory"
pos = me.observatory('ATCA')
print pos
print "\n 4. Set the time in reference frame. Can be shown with the showframe method"
print me.doframe(tim)
print "\n 5. Set the position in reference frame"
print me.doframe(pos)
print "\n 6. Set the coordinates as a measure"
coord = me.direction('J2000', '5h20m30.2', '-30d15m12.5')
print coord
print "\n 7. Set the cordinates to hour-angle, declination"
hadec = me.measure(coord,'hadec')
print hadec
print "\n 8. Get the sidereal time for now (just for the fun)"
last = me.measure(tim,'last')
print last
print "\n 9. calculate the sines/cosines of the declinations and latitudes"
u=me.getvalue(hadec)
sdec = qa.sin(me.getvalue(hadec)['m1'])
cdec = qa.cos(me.getvalue(hadec)['m1'])
clat = qa.cos(me.getvalue(pos)['m1'])
slat = qa.sin(me.getvalue(pos)['m1'])
print "sdec=", sdec
print "cdec=", cdec
print "clat=", clat
print "slat=", slat
print "\n10. Get the hour-angle for zero elevation"
ha = qa.acos(qa.neg(qa.div(qa.mul(sdec, slat), qa.mul(cdec, clat))))
print ha
print "\n11. Get the sidereal time of setting(rising?), normalized between"
print " 0 and 360 degrees"
print qa.norm(qa.add(ha, me.getvalue(coord)['m0']))
print "\n12. Show it in a time format"
print qa.time(qa.norm(qa.add(ha,me.getvalue(coord)['m0'])))
print "\n13. Save it as a time (remember, we calculated it as an angle)"
rt = qa.totime(qa.norm(qa.add(ha,me.getvalue(coord)['m0'])))
print rt
print "\n14. We would like to convert this sidereal time (rte) back to UTC. To be"
print "able to do that, we most know when (for which date) the sidereal time"
print "is valid. In general we do not know the Sidereal date, else it would"
print "be easy. We can however, specify an offset with a measure. Special"
print "reference codes are available (called 'raze'), which will after"
print "conversion only retain the integral part. This line says that we"
print "specify time as an epoch in UTC, to be razed after conversion. If we"
print "now use this as an offset for a sidereal time, the offset will be"
print "automatically converted to sidereal time (since it has to be added to"
print "a sidereal time), razed, i.e. giving the Greenwich sidereal date, and"
print "hence the complete epoch is known. (Note that the indecision over the"
print "4 minutes per day has to be handled in special cases)."
rtoff = me.epoch('r_utc', me.getvalue(tim)['m0'])
print "rtoff=", rtoff
print "\n15. We can now define our sidereal time rt as a real epoch"
rte = me.epoch('last', rt, off=rtoff)
print "rte=", rte
print "Expect [type=epoch, refer=LAST, m0=[value=58062.5276, unit=d]]"
print "\n16. We now convert the sidereal time rte, to an UTC, and show it"
print " as a time as well."
print me.measure(rte, 'utc')
print "Expect [type=epoch, refer=UTC, m0=[value=51349.3674, unit=d]]"
print qa.time(me.getvalue(me.measure(rte, 'utc'))['m0'], form=["ymd","time"])
print "Expect 1999/06/20/08:48:59.738"
print "\n17. We do not trust this funny razing, and convert the UTC obtained"
print "straight back to a sidereal time. And, indeed it worked (compare line"
print "12)"
print qa.time(me.getvalue(me.measure(me.measure(rte, 'utc'), 'last'))['m0'])
# try it another way
print "\n18. Another time"
print "tim = me.epoch('utc', 'today')", tim
tim = me.epoch('utc', 'today')
print "\n19. Convert it to TAI (just to show how to do it)"
print "tim = me.measure(tim,'tai')", tim
tim = me.measure(tim,'tai')
print "\n20. Frame it"
print me.doframe(tim)	
print "\n21. Show the current frame (the F argument indicates to not"
print "use GUI if one present) (note that the position is set to ATCA,"
print "as defined by the .aipsrc variable). If an error happens, the command"
print "me.doframe(me.observatory('atca')) will solve it."
print "me.showframe()="
print me.showframe()
print "Expect position: 09:58:12.033 -030.08.43.736 6372960.26 m ITRF"
print "Expect epoch: 2002/10/18/00:15:49 UTC"
print "Expect T"
print "\n22. to check if we really got a measure, or an error occurred."
print me.ismeasure(tim)
print "\n23. define the position of the sun (the actual position will be"
print "at the frame time)"
sun=me.direction('sun')
print sun
#print "\n24. get the sidereal time of rise/set of the sun (at the default elevation of 5 deg)"
#me.rise(sun)
#print "[rise=[value=20.4413559, unit=deg], set=[value=158.042775, unit=deg]]"
#print "\n25. try again for an elevation of 10 deg"
#me.rise(sun,'10deg')
#print "[rise=[value=27.3170864, unit=deg], set=[value=151.167045, unit=deg]]"
#print "\n26. display the rise sidereal time as a 'longitude'. A variety of"
#print "dq.form.x routines exist, using a global format setting mechanism"
#print "(in general you have your preferred way to display something)"
#qa.form.long(me.rise(sun).rise)
#print "01:21:45.925"
#print "\n27. change the format, and show again"
#qa.setformat('long','dms')
#print "T"
#qa.form.long(me.rise(sun).rise)
#print "+020.26.28.881"
