# this script tests the table persistence error and multiple applications of noise CAS-2258

name="tmp"

oldnoise=False

sm.open(name+".ms")

from simutil import simutil
u=simutil()
x,y,z,d,padnames,nant,telescope = u.readantenna("ES.250m.20100902.cfg")
sm.setconfig(telescopename=telescope,x=x,y=y,z=z, 
             dishdiameter=d.tolist(), 
             mount=['alt-az'], antname=padnames, padname=padnames, 
             coordsystem='global', referencelocation=me.observatory(telescope))
sm.setspwindow(spwname="band1", freq="230GHz",
               deltafreq="1GHz",freqresolution="1GHz",nchannels=1,stokes='XX YY')
sm.setfeed(mode='perfect X Y',pol=[''])
sm.setlimits(shadowlimit=0.01, elevationlimit='10deg')
sm.setauto(0.0)
sm.setfield(sourcename="src1", 
            sourcedirection="J2000 00:00:00.00 00.00.00",
            calcode="OBJ", distance='0m')
sm.settimes(integrationtime="10s", usehourangle=True, 
            referencetime=me.epoch('TAI', "2012/01/01/00:00:00"))
sm.observe(sourcename="src1", spwname="band1",
           starttime=qa.quantity("0s"),
           stoptime=qa.quantity("30s"));
sm.done()


if os.path.exists(name+".noisy.ms"):
    shutil.rmtree(name+".noisy.ms")
shutil.copytree(name+".ms",name+".noisy.ms")
# check for reopen bug
sm.openfromms(name+".noisy.ms")
sm.setdata()
if oldnoise:    
    sm.oldsetnoise(mode="simplenoise",simplenoise="1Jy")
else:
    sm.setnoise(simplenoise="1Jy")
sm.corrupt()
sm.done()

tb.open(name+".noisy.ms")
v=tb.getcol("DATA")
print "rms= (",pl.rms_flat(v.real),"+",pl.rms_flat(v.imag),"j) -> ",pl.rms_flat(v)
rms1=pl.rms_flat(v)
tb.done()



# check for renoise bug
shutil.rmtree(name+".noisy.ms")
shutil.copytree(name+".ms",name+".noisy.ms")

sm.openfromms(name+".noisy.ms")
sm.setdata()
if oldnoise:
    sm.oldsetnoise(mode="simplenoise",simplenoise="1Jy")
else:
    sm.setnoise(simplenoise="1Jy")
sm.corrupt()
sm.done()


tb.open(name+".noisy.ms")
v=tb.getcol("DATA")
print "rms= (",pl.rms_flat(v.real),"+",pl.rms_flat(v.imag),"j) -> ",pl.rms_flat(v)
rms2=pl.rms_flat(v)
tb.done()

if rms1 != rms2:
    print "ERROR! NOISE is increasing without cause!!"
else:
    print "PASSED"

