import os
import shutil
import numpy
from __main__ import default
from tasks import *
from taskinit import *
import unittest

IMAGE = 'image.im'

total = 0
fail  = 0
current_test =""
stars = "*************"

def test_start(msg):
    global total, current_test
    total += 1
    print
    print stars + " Test " + msg + " start " + stars
    current_test = msg
    
def test_end(condition, error_msg):
    global total, fail
    status = "OK"
    if not condition:
        print >> sys.stderr, error_msg
        fail += 1
        status = "FAIL"
    print stars + " Test " + current_test + " " + status + " " + stars
        
out1 = 'regridded'
out2 = 'bigger_image'
out3 = 'shifted_image'
out4 = 'back_to_image'
out5 = 'template'

class imregrid_test(unittest.TestCase):

    def setUp(self):
        pass
    
    def tearDown(self):
        os.system('rm -rf ' +IMAGE)
        os.system('rm -rf ' +out1)
        os.system('rm -rf ' +out2)
        os.system('rm -rf ' +out3)
        os.system('rm -rf ' +out4)
        os.system('rm -rf ' +out5)

        
    def test1(self):    
        myia = iatool.create()  
        myia.maketestimage(outfile = IMAGE)
        default('imregrid')
        
        # identity regrid
        imregrid(imagename = IMAGE,
                 template = IMAGE,
                 output = out1)
        
        im1 = myia.newimage(IMAGE)
        im2 = myia.newimage(out1)
        
        im1.statistics()
        im2.statistics()
        
        rec1 = im1.torecord()
        print '*************'
        print rec1['shape']
        print '*************'
        shape = im1.shape()
        print shape
        checked = 0
        for x in range(shape[0]):
            for y in range(shape[1]):
                p1 = im1.pixelvalue([x, y])
                p2 = im2.pixelvalue([x, y])
                if p1['mask'] != p2['mask']: raise Exception, p1['mask'] + ' != ' + p2['mask']
                if p1['value']['value'] != p2['value']['value']: raise Exception, p1['value']['value'] + ' != ' + p2['value']['value']
                if p1['value']['unit'] != p2['value']['unit']: raise Exception, p1['value']['unit'] + ' != ' + p2['value']['unit']
                checked += 3
        
        im2.done()
        
        print str(checked) + ' values checked'
        
        # rescale by factors 3 x 2
        rec1 = im1.torecord()
        print '*************'
        print rec1['shape']
        print '*************'
        rec1['shape'] = numpy.array([3*rec1['shape'][0], 2*rec1['shape'][1]], numpy.int32)
        rec1['coordsys']['coordsys']['direction0']['cdelt'] = [
            rec1['coordsys']['coordsys']['direction0']['cdelt'][0]/3.0,
            rec1['coordsys']['coordsys']['direction0']['cdelt'][1]/2.0]
        rec1['coordsys']['coordsys']['direction0']['crpix'] = [
            rec1['coordsys']['coordsys']['direction0']['crpix'][0]*3.0,
            rec1['coordsys']['coordsys']['direction0']['crpix'][1]*2.0]
        print rec1
        
        myia.fromrecord(rec1, out2)
        
        # First we need to remove the output file.
        if (  os.path.exists(out1) ):
              shutil.rmtree( out1 )
        imregrid(imagename = IMAGE,
                 template = out2,
                 output = out1)
        
        s1 = imstat(IMAGE)
        s2 = imstat(out1)
        
        print "S1: ", s1
        print " "
        print " "
        print "S2: ", s2
        
        if s1['maxpos'][0]*3 != s2['maxpos'][0]:
            raise Exception, str(s1['maxpos'][0]*3) + ' != ' + str(s2['maxpos'][0])
        if s1['maxpos'][1]*2 != s2['maxpos'][1]:
            raise Exception, str(s1['maxpos'][1]*2) + ' != ' + str(s2['maxpos'][1])
        
        
        
        # shift by -13, 1 pixels

        rec1 = im1.torecord()
        rec1['coordsys']['coordsys']['direction0']['crpix'] = [
            rec1['coordsys']['coordsys']['direction0']['crpix'][0]-13,
            rec1['coordsys']['coordsys']['direction0']['crpix'][1]+1]
        
        myia.fromrecord(rec1, out3)
        myia.close()
        # First we need to remove the output file.
        if (  os.path.exists(out1 ) ):
              shutil.rmtree( out1)
        imregrid(imagename = IMAGE,
                 template = out3,
                 output = out1)
        
        s1 = imstat(IMAGE)
        s2 = imstat(out1)
        if s1['maxpos'][0]-13 != s2['maxpos'][0]:
            raise Exception, str(s1['maxpos'][0]-13) + ' != ' + str(s2['maxpos'][0])
        if s1['maxpos'][1]+1 != s2['maxpos'][1]:
            raise Exception, str(s1['maxpos'][1]+1) + ' != ' + str(s2['maxpos'][1])
        
        
        # Shift back to original
        rec1['coordsys']['coordsys']['direction0']['crpix'] = [
            rec1['coordsys']['coordsys']['direction0']['crpix'][0]+13,
            rec1['coordsys']['coordsys']['direction0']['crpix'][1]-1]
        if (  os.path.exists(out3 ) ):
            shutil.rmtree( out3)
        myia.fromrecord(rec1, out3)
        myia.close()
        imregrid(imagename = IMAGE,
                 template = out3,
                 output = out4)
        
        s1 = imstat(IMAGE)
        s2 = imstat(out4)
        print s1
        print s2
        for stat in ['rms', 'medabsdevmed', 'minpos',
                     'min', 'max', 'sum', 'minposf',
                     'median', 'flux', 'sumsq', 'maxposf',
                     'trcf', 'quartile', 'npts', 'maxpos',
                     'mean', 'sigma', 'trc', 'blc', 'blcf']:
            if type(s1[stat]) == type('a string'):
                print "Checking string", stat, s1[stat]
                if s1[stat] != s2[stat]:
                    raise Exception
            else:
                for i in range(len(s1[stat])):
                    print "Checking", stat, "[", i, "]", s1[stat][i]
                    if s1[stat][i] != s2[stat][i]:
                        # Note:  == comparison of floating point values,
                        # it works right now on this computer but might need to get fixed...
                        raise Exception
        
        
        # Exercise various reference codes (no check on output)
        codes = cs.newcoordsys(direction=True).referencecode('dir', True)
        rec1 = im1.torecord()
        im1.done()
        for ref in codes:
            print "Regrid to", ref
            if ref not in ['JMEAN', 'JTRUE', 'APP',
                           'BMEAN', 'BTRUE', 'HADEC',
                           'AZEL', 'AZELSW', 'AZELNE',
                           'AZELGEO',
                           'AZELSWGEO', 'AZELNEGEO',
                           'JNAT',
                           'MECLIPTIC', 'TECLIPTIC',
                           'ITRF', 'TOPO']:
                rec1['coordsys']['coordsys']['direction0']['conversionSystem'] = ref
                
                myia.fromrecord(rec1, out5)
                myia.close()
                if (  os.path.exists(out1 ) ):
                    shutil.rmtree( out1 )
                imregrid(imagename = IMAGE,
                         template = out5,
                         output = out1)
            
def suite():
    return [imregrid_test]
    
    
