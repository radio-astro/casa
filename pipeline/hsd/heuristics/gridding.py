import numpy
import math

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.api as api
import pipeline.infrastructure as infrastructure
LOG = infrastructure.get_logger(__name__)


class GenerateGrid(api.Heuristic):
    """
    Generate spatial grid depending on pattern.
    """
    def calculate(self, pattern, ref_dir, beam_radius, allowance, ra, dec):
        """
        Generate spatial grid depending on patten.

        pattern: 'RASTER', 'SINGLE-POINT', or 'MULTI-POINT'
        ref_dir: reference direction (can be direction measure,
                 [ra,dec] in quantity or numeric in degree)
        beam_radius: beam radius as quantity or numeric in degree
        allowance: allowance radius that are identical position
                   (can be quantity or numeric in degree)
        ra: list of R.A. in degree
        dec: list of DEC. in degree
        """
        if pattern.upper() == 'RASTER':
            heuristics = RasterGrid()
        elif pattern.upper() == 'SINGLE-POINT':
            heuristics = SinglePointGrid()
        elif pattern.upper() == 'MULTI-POINT':
            heuristics = MultiPointGrid()
        else:
            raise NameError('invalid pattern: %s'%(pattern))
        return heuristics(ref_dir, beam_radius, allowance, ra, dec)

    def _as_direction_measure(self, dir):
        """
        return direction measure for input dir.
        J2000 frame is assumed.

        dir: direction measure or [ra,dec] in quantity or
             numerical value in degree.
        """
        me = casatools.measures
        qa = casatools.quanta
        if dir is None:
            return None
        elif isinstance(dir,dict):
            # it should be direction measure
            return dir
        elif isinstance(dir[0],dict):
            # ra and dec as quantity
            return me.direction(rf='J2000',
                                v0=dir[0],
                                v1=dir[1])
        else:
            # ra and dec as numeric in deg
            return me.direction(rf='J2000',
                                v0=qa.quantity(dir[0],'deg'),
                                v1=qa.quantity(dir[1],'deg'))
        

class SinglePointGrid(GenerateGrid):
    """
    Generate grid for single-point pattern.
    """
    def calculate(self, ref_dir, beam_radius, allowance, ra, dec):
        me = casatools.measures
        qa = casatools.quanta
        
        nra = 1
        ndec = 1

        if isinstance(allowance, dict):
            q_allowance = allowance
        else:
            # value in deg
            q_allowance = qa.quantity(allowance, 'deg')

        center_ra = numpy.array(ra).mean()   # in deg
        center_dec = numpy.array(dec).mean() # in deg
        m_center = self._as_direction_measure([center_ra,center_dec])

        m_ref = self._as_direction_measure(ref_dir)

        if m_ref is not None:
            sep = me.separation(m_center, m_ref)
            m_grid = m_ref if qa.le(sep, q_allowance) else m_center
        else:
            m_grid = m_center

##         # return only direction list with length one
##         grid_list = [[qa.convert(m_grid['m0'], 'deg')['value'],
##                       qa.convert(m_grid['m1'], 'deg')['value']]]
##         return (nra,ndec,grid_list)
        grid_list = [[qa.convert(m_grid['m0'], 'deg')['value']],
                     [qa.convert(m_grid['m1'], 'deg')['value']]]
        return grid_list

class MultiPointGrid(GenerateGrid):
    """
    Generate grid for multi-point pattern.
    """
    def calculate(self, ref_dir, beam_radius, allowance, ra, dec):
        me = casatools.measures
        qa = casatools.quanta

        nra = 0
        ndec = 1
        
        if isinstance(allowance, dict):
            q_allowance = allowance
        else:
            # value in deg
            q_allowance = qa.quantity(allowance, 'deg')

        ndata = len(ra)
        pointing_list = []
        for i in xrange(ndata):
            m = self._as_direction_measure([ra[i],dec[i]])
            found = False
            for j in xrange(len(pointing_list)):
                ref = pointing_list[j][0]
                sep = me.separation(m, ref)
                if qa.le(sep, q_allowance):
                    found = j
                    break
            if found is False:
                nra += 1
                pointing_list.append([m,[i]])
            else:
                pointing_list[found][1].append(i)
            print 'finished %s len(pointing_list) = %s'%(i,len(pointing_list))
        
##         grid_list = []
##         for p in pointing_list:
##             ra_list = numpy.take(ra, p[1])
##             dec_list = numpy.take(dec, p[1])
##             grid_list.append([ra_list.mean(), dec_list.mean()])
##         return (nra,ndec,grid_list)
        grid_list = [[],[]]
        for p in pointing_list:
            ra_list = numpy.take(ra, p[1])
            dec_list = numpy.take(dec, p[1])
            grid_list[0].append(ra_list.mean())
            grid_list[1].append(dec_list.mean())
        return grid_list
        

class RasterGrid(GenerateGrid):
    """
    Generate grid for raster pattern.
    """
    def calculate(self, ref_dir, beam_radius, allowance, ra, dec):
        me = casatools.measures
        qa = casatools.quanta

        if isinstance(beam_radius, dict):
            spacing = qa.convert(beam_radius,'deg')['value'] * 2.0 / 3.0
        else:
            spacing = beam_radius * 2.0 / 3.0
            
        m_ref = self._as_direction_measure(ref_dir)

        np_ra = numpy.array(ra)
        np_dec = numpy.array(dec)
        min_ra = np_ra.min()
        min_dec = np_dec.min()
        max_ra = np_ra.max()
        max_dec = np_dec.max()
        if min_ra < 10.0 and max_ra > 350.0:
            np_ra = np_ra + numpy.less_equal(np_ra, 180.0) * 360.0
            min_ra = np_ra.min()
            max_ra = np_ra.max()

        # DEC correction factor on R.A. gridding
        if m_ref is not None:
            ref_dec = qa.convert(m_ref['m1'], 'rad')['value']
        else:
            ref_dec = qa.quantity(0.5*(max_dec+min_dec), 'deg')
            ref_dec = qa.convert(ref_dec, 'rad')['value']
        dec_corr = 1.0 / math.cos(ref_dec)

        # create grid
        ra_list = []
        dec_list = []
        func_ra = self._ra_without_ref
        func_dec = self._dec_without_ref
        arg_ra = {'min_ra': min_ra, 'max_ra': max_ra,
                  'spacing': spacing*dec_corr}
        arg_dec = {'min_dec': min_dec, 'max_dec': max_dec,
                   'spacing': spacing}
        if m_ref is not None:
            ref_ra = qa.convert(m_ref['m0'], 'deg')['value']
            ref_dec = qa.convert(m_ref['m1'], 'deg')['value']
            if ref_ra <= max_ra and ref_ra >= min_ra and \
                   ref_dec <= max_dec and ref_dec >= min_dec:
                # setup grid such that m_ref is located at the center of
                # any grid
                func_ra = self._ra_with_ref
                func_dec = self._dec_with_ref
                arg_ra['ref_ra'] = ref_ra
                arg_dec['ref_dec'] = ref_dec
        ra_list = func_ra(**arg_ra)
        dec_list = func_dec(**arg_dec)

##         nra = len(ra_list)
##         ndec = len(dec_list)
##         grid_list = []
##         for y in dec_list:
##             for x in ra_list:
##                 grid_list.append([x, y])                 

##         return (nra,ndec,grid_list)
        return [ra_list,dec_list]


    def _ra_with_ref(self, min_ra, max_ra, ref_ra, spacing):
        ra_list = [ref_ra]
        idx = -1
        while True:
            ra_list.insert(0, locate(ref_ra, idx, spacing))
            if ra_list[0] - 0.5 * spacing <= min_ra:
                break
            idx -= 1
        idx = 1
        while True:
            ra_list.append(locate(ref_ra, idx, spacing))
            if ra_list[-1] + 0.5 * spacing >= max_ra:
                break
            idx += 1
        return ra_list

    def _ra_without_ref(self, min_ra, max_ra, spacing):
        nra = int((max_ra - min_ra) / spacing) + 1
        min_ra = 0.5 * ( (min_ra + max_ra) - (nra - 1) * spacing)
        ra_list = [locate(min_ra, x, spacing) for x in xrange(nra)]
        return ra_list
    
    def _dec_with_ref(self, min_dec, max_dec, ref_dec, spacing):
        dec_list = [ref_dec]
        idx = -1
        while True:
            dec_list.insert(0, locate(ref_dec, idx, spacing))
            if dec_list[0] - 0.5 * spacing <= min_dec:
                break
            idx -= 1
        idx = 1
        while True:
            dec_list.append(locate(ref_dec, idx, spacing))
            if dec_list[-1] + 0.5 * spacing >= max_dec:
                break
            idx += 1
        return dec_list

    def _dec_without_ref(self, min_dec, max_dec, spacing):
        ndec = int((max_dec - min_dec) / spacing) + 1
        min_dec = 0.5 * ( (min_dec + max_dec) - (ndec - 1) * spacing)
        dec_list = [locate(min_dec, y, spacing) for y in xrange(ndec)]
        return dec_list

locate = lambda x, y, z : x + y * z
    
