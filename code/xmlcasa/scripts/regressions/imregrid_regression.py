import os
import shutil

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
        
    
ia.maketestimage(outfile = IMAGE)
default('imregrid')

# identity regrid
imregrid(imagename = IMAGE,
         template = IMAGE,
         output = 'regridded')

im1 = ia.newimage(IMAGE)
im2 = ia.newimage('regridded')

im1.statistics()
im2.statistics()

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
print rec1
rec1['shape'] = [3*rec1['shape'][0], 2*rec1['shape'][1]]
rec1['coordsys']['coordsys']['direction0']['cdelt'] = [
    rec1['coordsys']['coordsys']['direction0']['cdelt'][0]/3.0,
    rec1['coordsys']['coordsys']['direction0']['cdelt'][1]/2.0]
rec1['coordsys']['coordsys']['direction0']['crpix'] = [
    rec1['coordsys']['coordsys']['direction0']['crpix'][0]*3.0,
    rec1['coordsys']['coordsys']['direction0']['crpix'][1]*2.0]
print rec1

ia.fromrecord(rec1, "bigger_image")

# First we need to remove the output file.
if (  os.path.exists('regridded' ) ):
      shutil.rmtree( 'regridded' )
imregrid(imagename = IMAGE,
         template = 'bigger_image',
         output = 'regridded')

s1 = imstat(IMAGE)
s2 = imstat('regridded')

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

ia.fromrecord(rec1, 'shifted_image')
# First we need to remove the output file.
if (  os.path.exists('regridded' ) ):
      shutil.rmtree( 'regridded' )
imregrid(imagename = IMAGE,
         template = 'shifted_image',
         output = 'regridded')


s1 = imstat(IMAGE)
s2 = imstat('regridded')
if s1['maxpos'][0]-13 != s2['maxpos'][0]:
    raise Exception, str(s1['maxpos'][0]-13) + ' != ' + str(s2['maxpos'][0])
if s1['maxpos'][1]+1 != s2['maxpos'][1]:
    raise Exception, str(s1['maxpos'][1]+1) + ' != ' + str(s2['maxpos'][1])


# Shift back to original
rec1['coordsys']['coordsys']['direction0']['crpix'] = [
    rec1['coordsys']['coordsys']['direction0']['crpix'][0]+13,
    rec1['coordsys']['coordsys']['direction0']['crpix'][1]-1]

ia.fromrecord(rec1, 'shifted_image')
imregrid(imagename = IMAGE,
         template = 'shifted_image',
         output = 'back_to_image')

s1 = imstat(IMAGE)
s2 = imstat('back_to_image')
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
codes = cs.newcoordsys(direction=T).referencecode('dir', T)
rec1 = im1.torecord()
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
        
        ia.fromrecord(rec1, 'template')

        if (  os.path.exists('regridded' ) ):
            shutil.rmtree( 'regridded' )
        imregrid(imagename = IMAGE,
                 template = 'template',
                 output = 'regridded')
    
im1.done()

