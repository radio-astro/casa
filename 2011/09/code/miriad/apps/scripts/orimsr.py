#####
#
# Fill, Calibrate and Test Map a standard short CARMA fringetest
# 
# you exectute this script from within casapy using
#    execfile('fringecal.py')
#
#####

from glob import glob as filesearch
import os


#  default for fringe test

#vis    = 'orimsr123b.ms'
vis    = 'orimsr4b.ms'
source = '3C273'
ms     = 'test1.ms'


def mysplit(ms):
    ms2=ms+'3'
    os.system('rm -rf %s' % ms2)
    print "split(%s,%s,%s)" % (ms,ms2,'3')
    split(vis=ms, outputvis=ms2, spw='3', datacolumn='data')

def simpletimeclean(vis,times):
    """select a timerange, and clean that map to get the doppler
    need to run fillter via script to control it's using my version
    and not something else
    """
    cmd='rm -rf junk.*; uvcat vis=%s out=junk.mir select="time(%s)"' % (vis,times)
    print cmd
    os.system(cmd)
    cmd='carmafiller.csh junk.mir junk.ms'
    os.system(cmd)
    simple_quick('junk.ms')

def simple_quick(ms=None):
    if ms==None:
        return
    print '--simple listobs --',ms

    # make a listing
    listobs(vis=ms)


def simple(ms=None):
    if ms==None:
        return
    print '--simple clean --',ms

    # make a listing
    listobs(vis=ms)

    # delete old stuff
    cmd  = 'rm -rf %s' % ms+'.fill.clean.*'
    print 'CMD: ',cmd
    os.system(cmd)

    print '--simple clean --'
    clean(vis=ms,imagename=ms+'.fill.clean',
          cell=[1.,1.],
          imsize=[128,128],
          field='0',
          threshold=10., 
          psfmode='hogbom',
#          psfmode='clark',
#          spw='4',
#          mode='channel', nchan=-1, 
#          mode='channel', nchan=-1, start=0, width=1,
#
#          spw='4~5',
#          mode='channel', start=405-3*63, nchan=15, width=1,
          spw='0',
          mode='channel',  nchan=2,
#          mode='velocity', nchan=6, start='-300km/s', width='100km/s',
#          mode='mfs',
          weighting='natural',
#          weighting='uniform',
#          weighting='briggs', robust=0,
          niter=1,
          stokes='I',
          )
    print '--simple done --'
    cmd = 'du -s %s*' % ms
    os.system(cmd)
    imhead(ms+'.fill.clean.image')

def test(vis):
    # first remove all things created in this routine: the .ms, the split, and cleaned
    cmd = 'rm -rf %s.*' % vis
    os.system(cmd)
    #
    ms=vis+".ms"
    cmd = "carmafiller vis=%s ms=%s" % (vis,ms)
    os.system(cmd)
    #
    mysplit(ms)
    ms45=ms+"45"
    simple(ms45)

def bigtest():
    """ loop over listobs sections of ORIMSR, run clean
    to get the Vlsrk of first channel
    """
    f = open('orimsr.hhmmss','r')
    lines = f.readlines()
    f.close()
    n = 0
    for line in lines:
        if line[0] == '#': continue
        n = n + 1
        hms = line.split()[1]
        hms_1 = hms[0:2] + ":" + hms[2:4] + ':' + hms[4:]
        if n == 1:
            hms_0 = hms_1
            continue
        print hms_0,hms_1
        simpletimeclean('orimsr_ww',hms_0+","+hms_1)
        hms_0 = hms_1
        

if __name__ == '__main__':
    #mysplit('vis3.ms')
    #mysplit('n6503_t2.ms')
    #simple('n6503_t2.ms)
    #simple('n6503_t2.ms45')
    #mysplit('n6503c_t2.ms')
    #mysplit('n6503c_t5.ms')
    #simple('n6503c_t2.ms45')
    #simple('n6503c_t5.ms45')
    #
    #mysplit(vis)
    #simple(vis+'3')
    #simple(vis)
    # 
    #simple('orimsr_c1w4.ms')
    #simple('orimsr_c1.ms')
    #simple('orimsr_11.ms')
    #simpletimeclean('orimsr_ww','03:48:18,'04:15:24')
    bigtest()

