#attool = casac.homefinder.find_home_by_name('atmosphereHome')
#at = attool.create()
at = casac.atmosphere()
#
at.initAtmProfile()
#
numchan=64
chansep=0.01
nb = 1
fC = qa.quantity(90.0,'GHz')
fR = qa.quantity(chansep,'GHz')
fW = qa.quantity(numchan*chansep,'GHz')

print "Test 1:"
at.initSpectralWindow(nb,fC,fW,fR)
print "Number of channels retrieved: ", at.getNumChan(), " Expected: ", numchan
rf = at.getRefFreq()
print "Reference frequency retrieved: ", rf['value'][0], rf['unit'], " Expected: ", fC['value'], fC['unit']
rc = at.getRefChan()
print "Reference channel retrieved: ", rc, " Expected: ", numchan/2
cs = at.getChanSep()
print "Channel separation retrieved: ", cs['value'][0], cs['unit'], " Expected: ", fR['value'], fR['unit']
print "Number of spectral windows: ", at.getNumSpectralWindows(), " Expected: ", nb
print "Total bandwidth retrieved: ", at.getBandwidth()['value'][0], at.getBandwidth()['unit'], " Expected: ", chansep*(numchan-1)
print "Frequency range: from ", at.getMinFreq()['value'][0], " to ", at.getMaxFreq()['value'][0], at.getMinFreq()['unit']
print "Channel frequency and number for the first spectral window:"
n = at.getNumChan()
for i in range(n):
	cf = at.getChanFreq(i)
	#print i, " channel: ", i-rc+1, " freq: ", cf['value'][0], cf['unit']
print "Expect an error here"
at.getNumChan(1)  # spwId is 0-based.

#
print
print "Test 2"
print "Adding new spectral window"
numchan1 = 128
chansep1 = 0.02  #GHz
#
trc1 = 32 # reference channel
trf1 = 215.0 #GHz frequency of trc1 channel.
tfc1 = trf1 + (numchan1/2 - trc1)*chansep1
#
fC1 = qa.quantity(tfc1, 'GHz')
fR1 = qa.quantity(chansep1, 'GHz')
fW1 = qa.quantity(numchan1*chansep1, 'GHz')
nc = at.addSpectralWindow(fC1, fW1, fR1)
print "New spectral window has ", nc, " channels. Expected ", numchan1
print "Number of spectral windows: ", at.getNumSpectralWindows(), " Expected: ", nb+1

spwId=0
print "Number of channels retrieved for spwID ", spwId, ": ", at.getNumChan(spwId), " Input:", numchan
print "Reference frequency retrieved: ", (at.getRefFreq(spwId))['value'][0], (at.getRefFreq(spwId))['unit'], " Expected: ", fC['value'], fC['unit']
print "Reference channel retrieved: ", at.getRefChan(spwId), " Expected: ", numchan/2
print "Channel separation retrieved: ", (at.getChanSep(spwId))['value'][0], (at.getChanSep(spwId))['unit'], " Expected: ", fR['value'], fR['unit']
spwId=1
print "Number of channels retrieved for spwID ", spwId, ": ", at.getNumChan(spwId), " Input:", numchan1
print "Reference frequency retrieved: ", (at.getRefFreq(spwId))['value'][0], (at.getRefFreq(spwId))['unit'], " Expected: ", fC1['value'], fC1['unit']
print "Reference channel retrieved: ", at.getRefChan(spwId), " Expected: ", numchan1/2
print "Channel separation retrieved: ", (at.getChanSep(spwId))['value'][0], (at.getChanSep(spwId))['unit'], " Expected: ", fR1['value'], fR1['unit']
print
print "Number of spectral windows: ", at.getNumSpectralWindows(), " Expected: 2"
print "Channel frequency and number for the first spectral window:"
n = at.getNumChan()
rc = at.getRefChan()
for i in range(n):
	cf = at.getChanFreq(i)
	print i, " channel: ", i-rc+1, " freq: ", cf['value'][0], cf['unit']

print
print "Test 2:"
spwId=0
at.initSpectralWindow(nb,fC,fW,fR)
print "Number of channels retrieved: ", at.getNumChan(spwId), " Expected: ", numchan
rf = at.getRefFreq(spwId)
print "Reference frequency retrieved: ", rf['value'][0], rf['unit'], " Expected: ", fC['value'], fC['unit']
rc = at.getRefChan(spwId)
print "Reference channel retrieved: ", rc, " Expected: ", numchan/2
cs = at.getChanSep(spwId)
print "Channel separation retrieved: ", cs['value'][0], cs['unit'], " Expected: ", fR['value'], fR['unit']
print "Number of spectral windows: ", at.getNumSpectralWindows(), " Expected: ", nb


print
print "Test 3:"
numchan = 64
bandwidth=0.62 #GHz
chansep = bandwidth/numchan  #GHz
fC = qa.quantity(90.0, 'GHz')
fR = qa.quantity(chansep, 'GHz')
fW = qa.quantity(bandwidth, 'GHz')

at.initSpectralWindow(nb,fC,fW,fR)
print "Number of channels retrieved: ", at.getNumChan(spwId), " Expected: ", numchan
rf = at.getRefFreq(spwId)
print "Reference frequency retrieved: ", rf['value'][0], rf['unit'], " Expected: ", fC['value'], fC['unit']
rc = at.getRefChan(spwId)
print "Reference channel retrieved: ", rc, " Expected: ", numchan/2
cs = at.getChanSep(spwId)
print "Channel separation retrieved: ", cs['value'][0], cs['unit'], " Expected: ", fR['value'], fR['unit']
print "Number of spectral windows: ", at.getNumSpectralWindows(), " Expected: ", nb
print "Total bandwidth retrieved: ", at.getBandwidth(spwId)['value'][0], at.getBandwidth(spwId)['unit'], " Expected: ", chansep*(numchan-1)

chan=16.123456
chanfreq=qa.add(qa.quantity(rf['value'][0],rf['unit']),qa.mul(qa.quantity(cs['value'][0],cs['unit']),chan))
print "Position (GU) retrieved: ",at.getChanNum(qa.quantity(chanfreq['value'],chanfreq['unit']),spwId), " Exact:   ", chan
print "minFreq: ", (at.getMinFreq(spwId))['value'][0], (at.getMinFreq(spwId))['unit']
print "maxFreq: ", (at.getMaxFreq(spwId))['value'][0], (at.getMaxFreq(spwId))['unit']
print "Channel for the min: ", at.getChanNum(at.getMinFreq(spwId), spwId)
print "Channel for the max: ", at.getChanNum(at.getMaxFreq(spwId), spwId)
print "Frequency range: from ", (at.getMinFreq(spwId))['value'][0], " to ", (at.getMaxFreq(spwId))['value'][0], at.getMinFreq(spwId)['unit']

print "Skipping double sideband spectral grid test..."
print "CASA atmosphere tool currently does not have ability to handle double sideband spectra"

exit()
