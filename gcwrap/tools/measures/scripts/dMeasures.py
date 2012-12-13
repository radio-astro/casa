print "\n____Testing me.type()____"
print "me.type()"
print me.type()
print "\n____Testing me.epoch()____"
print "me.epoch('utc','today')"
print me.epoch('utc','today')
print "\n____Testing me.direction()____"
print "me.direction('j2000','5h20m','-30.2deg')"
print me.direction('j2000','5h20m','-30.2deg') 
print "\n____Testing me.gettype()____"
print "a = me.direction('j2000','5h20m','-30.2deg')"
a = me.direction('j2000','5h20m','-30.2deg')
print "me.gettype(a)"
print me.gettype(a)
print "\n____Testing me.getoffset()____"
print "me.getoffset(a)"
print me.getoffset(a)
print "\n____Testing me.getref()____"
print "me.getref(a)"
print me.getref(a)
print "\n____Testing me.getvalue()____"
print "me.getvalue(a)"
print me.getvalue(a)
print "me.getvalue(a)['m0']"
print me.getvalue(a)['m0']
print "me.getvalue(a)['m1']"
print me.getvalue(a)['m1']
#print "___try as a scalar quantity with multiple values"
#print "a = me.direction('j2000', qa.quantity([10,20],'deg'), qa.quantity([30,40], 'deg'))"
#a = me.direction('j2000', qa.quantity([10,20],'deg'), qa.quantity([30,40], 'deg'))
#print me.getvalue(a)['value'][0]
#print "Expect [unit=rad, value=[0.174532925 0.34906585] ]"
#print me.getvalue(me.getvalue(a)['level'][0])['level'][1]
#print "Expect 0.34906585"
#print a
#print "Expect
#print " [type=direction, refer=J2000, m1=[value=[0.523598776 0.698131701] , unit=rad],"
#print "	 m0=[unit=rad, value=[0.174532925 0.34906585] ]]"
#print "___try as an r_array"
