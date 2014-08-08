# test occurence of segfault in fits file handling when async==False
os.system('rm -rf test.uvfits test-reimport.ms')
os.system('rm -rf test.fits')
exportuvfits(vis='test.ms', fitsfile='test.uvfits', datacolumn = 'data')
exportfits(fitsimage='test.fits', imagename='test.clean.image')
importuvfits(vis='test-reimport.ms', fitsfile='test.uvfits')
print ''
print 'Regression PASSED'
print ''

