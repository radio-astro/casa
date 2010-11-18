# wrapper script for FITS import/export regression test
import sys
import os
import string
from locatescript import locatescript
import inspect

# Short description
def description():
    return "Test of the exportfits() and importfits() tasks, compliance with FITS standard"

# Reference data
#  The dictionary of test datasets
#  Each entry is a tuple of filename (w/o extension), expected pixel position of the maximum flux,
#  expected absolute position of the maximum flux (RA, DEC), name.
pass_on = { 'datasets': { 1: ('1904-66_AIR', [109, 167], '19:39:23.885, -63.45.36.905', 'Airy Projection (AIR)'), 
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
            }

a=inspect.stack()
stacklevel=0
for k in range(len(a)):
    if (string.find(a[k][1], 'ipython console') > 0):
        stacklevel=k
        break
gl=sys._getframe(stacklevel).f_globals


def run():
    #####locate the regression script
    try: 
        lepath=locatescript('fits-import-export_regression.py')
        print 'Script used is ',lepath
        execfile(lepath, gl, pass_on)
    except:
        print 'execution failed: ', sys.exc_info()
        raise
###return the images that will be templated and compared in future runs
    return []

def data():
    ### return the data files that are needed by the regression script
    myfiles = []
    for i in (pass_on['datasets']).keys():
        myfiles.append(pass_on['datasets'][i][0] + '.fits')
    myfiles.append('stokeslast-test.image')    
    return myfiles

def doCopy():
    ### return a list of the same length as that returned by data()
    ###   containing 0 if the corresponding file should be just linked
    ###   (i.e. is read-only)
    ###   or 1 if the corresponding file should be really copied to
    ###   the work directory
    cp = []
    for i in (pass_on['datasets']).keys():
        cp.append(0)
    return cp
