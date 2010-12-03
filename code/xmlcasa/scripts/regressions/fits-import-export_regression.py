#############################################################################
# $Id:$
# Test Name:                                                                #
#    Regression Test Script for importfits/exportfits                       #
#                                                                           #
# Rationale for Inclusion:                                                  #
#    The proper handling of the FITS standard needs to be verified          #
#    using standard input files constructed based on the FITS               #
#    standard version 3.0 by Mark Calabretta                                #
#                                                                           # 
# Features tested:                                                          #
#    1) Is the import performed without raising exceptions?                 #
#    2) Is the imported image properly handling the WCS?                    #
#       (i.e. is the RA and DEC of the max. flux properly determined?)      #
#    3) Is the export performed without raising exceptions?                 #
#    4) Is the exported file compatible with the original?                  #
#    5) Does the bitpix=16 feature work                                     #
#                                                                           #
# Input data:                                                               #
#    28 sample fits files constructed by Mark Calabretta                    #
#    for all standard projections                                           #
#                                                                           #
#############################################################################



# get the dataset name from the wrapper if possible

if not (locals()).has_key('datasets'):
    myname = 'fits-import-export_regression :'
    # The default dictionary of test datasets
    #  Each entry is a tuple of filename (w/o extension), expected pixel position of the maximum flux,
    #  expected absolute position of the maximum flux (RA, DEC), name.
    mydict = { 1: ('1904-66_AIR', [109, 167], '19:39:23.885, -63.45.36.905', 'Airy Projection (AIR)'),
               2: ('1904-66_AIT', [109, 168], '19:39:41.653, -63.43.54.147', 'Hammer-Aitoff Projection (AIT)'),  
               3: ('1904-66_ARC', [110, 170], '19:39:28.622, -63.41.53.659', 'Zenithal Equidistant Projection (ARC)'), 
               4: ('1904-66_AZP', [116, 186], '19:39:21.120, -63.44.26.642', 'Zenithal Perspective Projection (AZP)'), 
               5: ('1904-66_BON', [108, 173], '19:39:28.718, -63.41.12.383', 'Bonne\'s Equal Area Projection (BON)'), 
               6: ('1904-66_CAR', [113, 168], '19:39:42.371, -63.41.36.035', 'Plate Caree Projection (CAR)'), 
               7: ('1904-66_CEA', [113, 167], '19:39:35.136, -63.41.56.055', 'Cylindrical Equal Area Projection (CEA)'),
               8: ('1904-66_COD', [109, 166], '19:39:39.760, -63.42.02.640', 'Conic Equidistant Projection (COD)'), 
               9: ('1904-66_COE', [112, 172], '19:39:34.041, -63.44.23.296', 'Conic Equal-Area Projection (COE)'),
               10: ('1904-66_COO', [109, 161], '19:39:31.237, -63.44.09.556', 'Conic Orthomorphic Projection (COO)'), 
               11: ('1904-66_COP', [110, 161], '19:39:28.345, -63.44.40.626', 'Conic Perspective Projection (COP)'), 
               12: ('1904-66_CSC', [113, 180], '19:39:41.073, -63.43.25.624', 'COBE Quadrilateralized Spherical Cube Projection (CSC)'), 
               13: ('1904-66_CYP', [108, 157], '19:39:12.028, -63.43.07.315', 'Cylindrical Perspective Projection (CYP)'),  
               14: ('1904-66_HPX', [113, 179], '19:39:16.552, -63.42.47.347', 'HEALPix Grid Projection (HPX)'), 
               15: ('1904-66_MER', [113, 168], '19:39:16.276, -63.42.48.107', 'Mercator Projection (MER)'), 
               16: ('1904-66_MOL', [109, 175], '19:39:20.341, -63.41.44.201', 'Mollweide Projection (MOL)'), 
               17: ('1904-66_NCP', [107, 167], '19:39:38.614, -63.42.51.577', 'North Celetial Pole (SIN spcial case) Projection (NCP)'), 
               18: ('1904-66_PAR', [109, 171], '19:39:32.698, -63.42.04.737', 'Parabolic Projection (PAR)'), 
               19: ('1904-66_PCO', [108, 174], '19:39:21.403, -63.43.49.358', 'Polyconic Projection (PCO)'), 
               20: ('1904-66_QSC', [120, 182], '19:39:23.808, -63.41.22.666', 'Quadrilateralized Spherical Cube Projection (QSC)'), 
               21: ('1904-66_SFL', [108, 167], '19:39:16.950, -63.45.15.188', 'Samson-Flamsteed Projection (SFL)'), 
               22: ('1904-66_SIN', [107, 167], '19:39:38.614, -63.42.51.577', 'Slant Orthographic Projection (SIN)'), 
               23: ('1904-66_STG', [111, 171], '19:39:14.752, -63.44.20.882', 'Stereographic Projection (STG)'), 
               24: ('1904-66_SZP', [110, 177], '19:39:42.475, -63.42.13.751', 'Slant Zenithal Perspective Projection (SZP)'),
               25: ('1904-66_TAN', [116, 177], '19:39:30.753, -63.42.59.218', 'Gnomonic Projection (TAN)'), 
               26: ('1904-66_TSC', [112, 160], '19:39:39.997, -63.41.14.586', 'Tangential Spherical Cube Projection (TSC)'), 
               27: ('1904-66_ZEA', [109, 169], '19:39:26.872, -63.43.26.060', 'Zenithal Equal Area Projection (ZEA)'), 
               28: ('1904-66_ZPN', [94, 150], '19:39:24.948, -63.46.43.636', 'Zenithal Polynomial Projection (ZPN)'), 
               29: ('1904-66_AIT-obsgeo', [109, 168], '19:39:41.653, -63.43.54.147', 'Hammer-Aitoff Projection (AIT)')
           }
else: # the script has been called from the wrapper, don't need to output name
    myname = ' '
    mydict = (locals())['datasets']


def checkimage(myfitsimage_name, maxpos_expect, maxposf_expect):
    global myname
    subtest_passed = True

    # import the image
    default('importfits')
    try:
        print myname, ' Importing ', myfitsimage_name+'.fits', ' ...'
        importfits(fitsimage = myfitsimage_name+'.fits',
                   imagename = myfitsimage_name,
                   overwrite = True)
    except:
        print myname, ' Error ', sys.exc_info()[0]
        raise    
    else:
        print myname, ' No exceptions raised! Now checking image ...'
        # perform a basic check of the coordinate system
        ia.open(myfitsimage_name)
        mystat = imstat(imagename = myfitsimage_name)
        if not (myname == ' '):
            print mystat
        if not ((mystat['maxpos']) == maxpos_expect).all():
            print myname, ' Error in imported image ', myfitsimage_name, ':'
            print myname, '    expected pixel position of maximum is ', maxpos_expect
            print myname, '                               but found ', mystat['maxpos'] 
            subtest_passed = False    
        if not (mystat['maxposf'] == maxposf_expect):
            print myname, ' Error in imported image ', myfitsimage_name, ':'
            print myname, '   expected absolute position of maximum is ', maxposf_expect
            print myname, '                                  but found ', mystat['maxposf'] 
            subtest_passed = False 
        if subtest_passed:
            print myname, ' imported image ', myfitsimage_name, ' as expected.'

            # export the image
            default('exportfits')
            try:
                print 'Exporting ', myfitsimage_name, '...'
                exportfits(fitsimage = myfitsimage_name + 'exp.fits',
                           imagename = myfitsimage_name,
                           overwrite = True)
            except:
                print myname, ' Error ', sys.exc_info()[0]
                raise    
            else:
                print myname, ' No exceptions raised! Now comparing exported image with original by re-importing it ...'
                # re-import the image    
                default('importfits')
                try:
                    print myname, ' Re-importing ', myfitsimage_name+'exp.fits', ' ...'
                    importfits(fitsimage = myfitsimage_name+'exp.fits',
                               imagename = myfitsimage_name+'exp',
                               overwrite = True)
                except:
                    print myname, ' Error ', sys.exc_info()[0]
                    raise    
                else:
                    print myname, ' No exceptions raised! Now checking image ...'
                    ia.open(myfitsimage_name+'exp')
                    csm = ia.coordsys()
                    csm.summary() 
                    mystat = imstat(imagename = myfitsimage_name+'exp')
                    if not ((mystat['maxpos']) == maxpos_expect).all():
                        print myname, ' Error in re-imported image ', myfitsimage_name+'exp', ':'
                        print myname, '   expected pixel position of maximum is ', maxpos_expect
                        print myname, '                               but found ', mystat['maxpos'] 
                        subtest_passed = False    
                    if not (mystat['maxposf'] == maxposf_expect):
                        print myname, ' Error in re-imported image ', myfitsimage_name+'exp', ':'
                        print myname, '   expected absolute position of maximum is ', maxposf_expect
                        print myname, '                                  but found ', mystat['maxposf'] 
                        subtest_passed = False 
                    if subtest_passed:
                        print myname, ' re-imported image ', myfitsimage_name+'exp', ' as expected.'
    return subtest_passed
# end def checkimage()

def checkimageb(myfitsimage_name):
    global myname
    subtest_passed = True

    # import the image
    default('importfits')
    try:
        print myname, ' Importing ', myfitsimage_name+'.fits', ' ...'
        importfits(fitsimage = myfitsimage_name+'.fits',
                   imagename = myfitsimage_name,
                   overwrite = True)
    except:
        print myname, ' Error ', sys.exc_info()[0]
        raise    
    else:
        print myname, ' No exceptions raised!'
        mystat = imstat(imagename = myfitsimage_name)
        # export the image
        default('exportfits')
        try:
            print 'Exporting ', myfitsimage_name, ' with bitpix=16 ...'
            exportfits(fitsimage = myfitsimage_name + 'exp.fits',
                       imagename = myfitsimage_name,
                       overwrite = True, bitpix=16) # with BITPIX=16 !
        except:
            print myname, ' Error ', sys.exc_info()[0]
            raise    
        else:
            print myname, ' No exceptions raised! Now testing minimum and maximum values ...'
            # re-import the image    
            default('importfits')
            try:
                print myname, ' Re-importing ', myfitsimage_name+'exp.fits', ' ...'
                importfits(fitsimage = myfitsimage_name+'exp.fits',
                           imagename = myfitsimage_name+'exp',
                           overwrite = True)
            except:
                print myname, ' Error ', sys.exc_info()[0]
                raise    
            else:
                print myname, ' No exceptions raised! Now checking image ...'
                myotherstat = imstat(imagename = myfitsimage_name+'exp')
                if not (abs((mystat['min'] - myotherstat['min'])/mystat['min']) < 1E-6):
                    print myname, ' Error in re-imported image ', myfitsimage_name+'exp', ':'
                    print myname, '   expected  minimum is ', mystat['min']
                    print myname, '                               but found ', myotherstat['min'] 
                    subtest_passed = False    
                if not (abs((mystat['max'] - myotherstat['max'])/mystat['max']) < 1E-6):
                    print myname, ' Error in re-imported image ', myfitsimage_name+'exp', ':'
                    print myname, '   expected  maximum is ', mystat['max']
                    print myname, '                               but found ', myotherstat['max'] 
                    subtest_passed = False    
                if subtest_passed:
                    print myname, ' re-imported image ', myfitsimage_name+'exp', ' as expected.'
    return subtest_passed
# end def checkimage()

failed_tests = []
passed_tests = []

for i in mydict.keys():

    thefitsimage_name = mydict[i][0]
    themaxpos_expect = mydict[i][1]
    themaxposf_expect = mydict[i][2]
    print myname, ' ***********************************************************'
    print myname, ' Subtest ', i, ': ', mydict[i][3]
    passed = checkimage(thefitsimage_name, themaxpos_expect, themaxposf_expect)
    if passed:
        print myname, ' Subtest ', i, ' passed.'
        passed_tests.append(mydict[i][3])
    else:
        print myname, ' Subtest ', i, ' failed.'
        failed_tests.append(mydict[i][3])
    print myname, ' ***********************************************************'


thefitsimage_name = mydict[1][0]
print myname, ' ***********************************************************'
print myname, ' Test of the BITPIX parameter:'
passed = checkimageb(thefitsimage_name)
if passed:
    print myname, ' bitpix test passed.'
    passed_tests.append('bitpix')
else:
    print myname, ' bitpix test failed.'
    failed_tests.append('bitpix')
print myname, ' ***********************************************************'

print myname, ' ***********************************************************'
print myname, ' Test of the stokeslast parameter:'
exportfits(imagename='stokeslast-test.image', fitsimage='stokeslast-test.fits', stokeslast=True)
importfits(imagename='stokeslast-test2.image', fitsimage='stokeslast-test.fits')
myrgn1 = rg.box([0,0,1,0],[64,64,1,0])
myrgn2 = rg.box([0,0,0,1],[64,64,0,1])
ia.open('stokeslast-test.image')
ia.subimage(outfile='sub1.im', region = myrgn1)
ia.close()
ia.open('stokeslast-test2.image')
ia.subimage(outfile='sub2.im', region = myrgn2)
ia.close()
myresult1 = imstat('sub1.im')
myresult2 = imstat('sub2.im')
# imagecalc is on strike here because the formal coordinates of the slices disagree because the order is different
# so use min, max, and sum
passed = (myresult1['min']==myresult2['min']) and (myresult1['max']==myresult2['max']) and (myresult1['sum']==myresult2['sum'])
if passed:
    print myname, ' stokeslast test passed.'
    passed_tests.append('stokeslast')
else:
    print myname, ' stokeslast test failed.'
    failed_tests.append('stokeslast')
print myname, ' ***********************************************************'



if len(failed_tests)>0:
    if len(passed_tests)>0:
        print myname, ' Summary of passed subtests:'
        for i in range(0, len(passed_tests)):
            print myname, '          ', passed_tests[i]            
    print myname, ' Summary of failed subtests:'
    for i in range(0, len(failed_tests)):
        print myname, '          ', failed_tests[i]
    raise Exception, repr(len(failed_tests))+' out of '+repr(len(mydict.keys()))+' subtests failed.'
else:
    print 'All subtests passed.'
print 'Done.'


#End of Script


        
        
    

