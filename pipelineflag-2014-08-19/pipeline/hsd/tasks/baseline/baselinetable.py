from __future__ import absolute_import

import os
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)

class BaselineTableGenerator(object):
    fitfunc = 'none'

    @property
    def nrow(self):
        if hasattr(self, 'scan'):
            return len(self.scan)
        else:
            return 0
    
    def update_table(self, tablename, datatable, idxlist):
        if len(idxlist) != self.nrow:
            raise RuntimeError('length mismatch between index list and imported csv table: %s, %s'%(len(idxlist), self.nrow))
            
        with casatools.TableReader(tablename, nomodify=False) as tb:
            rows = tb.getcol('Row').tolist()
            for i in xrange(self.nrow):
                idx = idxlist[i]
                
                # do not update table if line window doesn't change
                nochange = datatable.getcell('NOCHANGE', idx)
                if type(nochange) == int:
                    continue

                masklist = datatable.getcell('MASKLIST', idx)
                row = datatable.getcell('ROW', idx)
                try:
                    irow = rows.index(row)
                except:
                    irow = tb.nrows()
                    tb.addrows()
                    tb.putcell('Row', irow, row)
                    rows.append(row)
                tb.putcell('Sections', irow, self.segments[i])
                tb.putcell('SectionCoefficients', irow, self.coefficients[i])
                if len(masklist) > 0:
                    tb.putcell('LineMask', irow, masklist)
                else:
                    tb.putcell('LineMask', irow, [[0,0]])
                tb.putcell('FitFunc', irow, self.fitfunc)
                    
    def import_csv(self, csvfile):
        self.scan = []
        self.beam = []
        self.ifno = []
        self.pol = []
        self.cycle = []
        self.rms = []
        self.segments = []
        self.coefficients = []
        with open(csvfile, 'r') as f:
            # read first line
            line = f.readline()
            while len(line) > 0:
                # format line
                splitted_line = line.split(']]')
                formatted_line_left = splitted_line[0].replace('[','').replace(']','').replace(';',',')
                formatted_line_right = splitted_line[1].lstrip(',').replace('[','').replace(']','').replace(';',',')
                # convert to numpy array
                left_data = numpy.fromstring(formatted_line_left, dtype=float, sep=',')
                right_data = numpy.fromstring(formatted_line_right, dtype=float, sep=',')
                self._import_data(left_data, right_data)
                
                # update line
                line = f.readline()

class SplineBaselineTableGenerator(BaselineTableGenerator):
    fitfunc = 'spline'

    def _import_data(self, left_data, right_data):
        # format of left_data:
        # scan, beam, if, pol, cycle,
        #   ledge_mask0, redge_mask0,
        #   ledge_mask1, redge_mask1,
        #   ...
        #   ledge_maskN, redge_maskN
        #
        # format of right_data:
        #   ledge_seg1, redge_seg1, p0_seg1, p1_seg1, p2_seg1, p3_seg1,
        #   ledge_seg2, redge_seg2, p0_seg2, p1_seg2, p2_seg2, p3_seg2,
        #   ...
        #   ledge_segN, redge_segN, p0_segN, p1_segN, p2_segN, p3_segN,
        #   rms, num_clipped
        self.scan.append(int(left_data[0]))
        self.beam.append(int(left_data[1]))
        self.ifno.append(int(left_data[2]))
        self.pol.append(int(left_data[3]))
        self.cycle.append(int(left_data[4]))
        self.rms.append(right_data[-2])
        _seg = []
        _prm = []
        for iseg in xrange(0, len(right_data)-2, 6):
            _seg.append(numpy.array(right_data[iseg:iseg+2],dtype=numpy.int32))
            _prm.append(right_data[iseg+2:iseg+6])
        self.segments.append(numpy.array(_seg))
        self.coefficients.append(numpy.array(_prm))
        
    
    
class PolynomialBaselineTableGenerator(BaselineTableGenerator):
    fitfunc = 'polynomial'

    def __init__(self, segments):
        self._segments = numpy.array(segments, dtype=numpy.int32)
        self._counter = 0

    @property
    def num_segments(self):
        return len(self._segments)

    @property
    def end_segment(self):
        return (self._counter == self.num_segments - 1)

    @property
    def begin_segment(self):
        return (self._counter == 0)
    
    def _import_data(self, left_data, right_data):
        # format of left_data:
        # scan, beam, if, pol, cycle, ledge, redge
        #
        # format of right_data:
        # coeff0, coeff1, ..., coeffN, rms, num_clipped
        if self.begin_segment:
            self.scan.append(int(left_data[0]))
            self.beam.append(int(left_data[1]))
            self.ifno.append(int(left_data[2]))
            self.pol.append(int(left_data[3]))
            self.cycle.append(int(left_data[4]))
            self.rms.append(right_data[-2])
            self.segments.append(self._segments)
            self._coefficients = []

        if self.end_segment:
            self._coefficients.append(right_data[0:-2])
            self.coefficients.append(self._coefficients)
        else:
            self._coefficients.append(right_data[0:-2])
            
        self.next_segment()

    def next_segment(self):
        self._counter += 1
        if self._counter >= self.num_segments:
            self._counter = 0
            

