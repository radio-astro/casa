print qa.type()             # Default tool created for you
#
print qa.quantity(5.4, 'km/s')
q1 = qa.quantity([8.57132661e+09, 1.71426532e+10], 'km/s')
q1
print qa.convert(q1, 'pc/h');
#
print qa.quantity('5.4km/s')          
print qa.quantity(qa.unit('5.4km/s')) 
#
#q1 = qa.unit("5s 5.4km/s") #can't handle string vector using qh.fromString
#q1
#2
#: q1[1]
#[value=5, unit=s]
#: q1[2]
#[value=5.4, unit=km/s]
#
q1 = qa.unit('5km');
q2 = qa.unit('200m');
qa.canonical(qa.add(q1,q2))
#
q1 = qa.quantity('6rad');
q2 = qa.quantity('3deg');
qa.compare(q1,q2)
qa.compare(q1,qa.unit('3km'))
#
#: q1 = qa.unit(array("5s 5.4km/s", 3, 2))
#: length(q1)
#6 
#: q1[1]
#[value=5, unit=s] 
#: q1[2]
#[value=5.4, unit=km/s] 
#: q1[6]
#[value=5.4, unit=km/s] 
#: q1::
#[id=quant, shape=[3 2] ]
#
print qa.map('const')
boltzmann = qa.constants('k')
print 'Boltzmann constant is ', boltzmann
#
qa.quantity('today')
qa.quantity('5jul1998')
qa.quantity('5jul1998/12:')
qa.quantity('-30.12.2')
qa.quantity('2:2:10')
qa.unit('23h3m2.2s')  
#
a = qa.quantity('today');        # 1 Get the time now
print a
b = qa.toangle(a);               # 2 Get the time as an angle
print b
qa.angle(qa.norm(qa.toangle(a)));       # 3 Get the time as a normalized angle (-pi to +pi) and show as dms
qa.angle(qa.norm(qa.toangle(a), 0));    # 4 Get the time as a normalised angle (0 to 2pi) and show as dms
qa.sub('today',a);                      # 5 Get time since creation of a
#
#: q1 = qa.unit(array("5s 5.4km/s", 3, 2))
#: length(q1)
#6
#: q1::
#[id=quant, shape=[3 2] ]
