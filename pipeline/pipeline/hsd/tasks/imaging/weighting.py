from __future__ import absolute_import

import os
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from .. import common

LOG = infrastructure.get_logger(__name__)

class WeightMSInputs(common.SingleDishInputs):
    """
    Inputs for exporting data to MS 
    """
    def __init__(self, context, infile, outfile, antenna, 
                 spwid, spwtype, onsourceid):
        self._init_properties(vars())
        
        
class WeightMSResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(WeightMSResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(WeightMSResults, self).merge_with_context(context)

    def _outcome_name(self):
        # return [image.imagename for image in self.outcome]
        return self.outcome


class WeightMS(common.SingleDishTaskTemplate):
    Inputs = WeightMSInputs
 
    Rule = {'WeightDistance': 'Gauss',
            'Clipping': 'MinMaxReject',
            'WeightRMS': True,
            'WeightTsysExpTime': False}
   
    def prepare(self):
        # for each data
        outfile = self.inputs.outfile
        is_full_resolution = (self.inputs.spwtype.upper() == 'SP')

        # make row mapping table between scantable and ms
        row_map = self._make_row_map()
        
        # set weight
        if is_full_resolution:
            minmaxclip = (WeightMS.Rule['Clipping'].upper() == 'MINMAXREJECT')
            weight_rms = WeightMS.Rule['WeightRMS']
            weight_tintsys = WeightMS.Rule['WeightTsysExpTime']
        else:
            minmaxclip = False
            weight_rms = False
            weight_tintsys = True
        self._set_weight(row_map, minmaxclip=minmaxclip, weight_rms=weight_rms,
                   weight_tintsys=weight_tintsys)


        result = WeightMSResults(task=self.__class__,
                                 success=True,
                                 outcome=outfile)
        result.task = self.__class__

        if self.inputs.context.subtask_counter is 0: 
            result.stage_number = self.inputs.context.task_counter - 1
        else:
            result.stage_number = self.inputs.context.task_counter 

        return result
    
    def analyse(self, result):
        return result

    def _make_row_map(self):
        infile = self.inputs.infile
        outfile = self.inputs.outfile
        spwid = self.inputs.spwid
        srctype = self.inputs.onsourceid
        rows_list = []
        srctypes_list = []
        with casatools.TableReader(infile) as tb:
            for polno in xrange(4):
                tsel = tb.query('IFNO==%s && POLNO==%s' % (spwid, polno),
                                sortlist='TIME')
                if tsel.nrows() > 0:
                    rows = tsel.rownumbers() 
                    srctypes = tsel.getcol('SRCTYPE')
                    rows_list.append(rows)
                    srctypes_list.append(srctypes)
                tsel.close()
    
        with casatools.TableReader(os.path.join(outfile, 'DATA_DESCRIPTION')) as tb:
            spwids = tb.getcol('SPECTRAL_WINDOW_ID')
            data_desc_id = numpy.where(spwids == spwid)[0][0]
    
        with casatools.TableReader(outfile) as tb:
            tsel = tb.query('DATA_DESC_ID==%s' % (data_desc_id),
                            sortlist='TIME')
            ms_rows = tsel.rownumbers() 
    
        row_map = {}
        # ms_row: (st_row_pol0, st_row_pol1)
        for index in xrange(len(rows)):
            if srctypes[index] == srctype:
                row_map[ms_rows[index]] = [rows[index] for rows in rows_list]
    
        return row_map
         
    def _set_weight(self, row_map, minmaxclip, weight_rms, weight_tintsys):
        outfile = self.inputs.outfile
        antenna = self.inputs.antenna
        spwid = self.inputs.spwid
        srctype = self.inputs.onsourceid

        datatable = self.datatable
        
        # get corresponding datatable rows
        datatable_name = datatable.plaintable
        taqlstring = 'USING STYLE PYTHON SELECT ROWNUMBER() AS ID FROM "%s" WHERE IF==%s && SRCTYPE == %s && ANTENNA == %s' % (os.path.join(datatable_name, 'RO'), spwid, srctype, antenna)
        table = datatable.tb1
        LOG.debug('taqlstring=\'%s\'' % (taqlstring))
        tx = table.taql(taqlstring)
        index_list = tx.getcol('ID')
        tx.close()
        del tx
    
        rows = datatable.tb1.getcol('ROW').take(index_list)
        weight = {}
        for row in rows:
            weight[row] = 1.0
    
        # set weight 
        if weight_rms:
            stats = datatable.tb2.getcol('STATISTICS').take(index_list)
            for index in xrange(len(rows)):
                row = rows[index]
                stat = stats[index]
                if stat != 0.0:
                    weight[row] /= (stat * stat)
                else:
                    weight[row] = 0.0
    
        if weight_tintsys:
            exposures = datatable.tb1.getcol('EXPOSURE').take(index_list)
            tsyss = datatable.tb1.getcol('TSYS').take(index_list)
            for index in xrange(len(rows)):
                row = rows[index]
                exposure = exposures[index]
                tsys = tsyss[index]
                if tsys > 0.5:
                    weight[row] *= (exposure / (tsys * tsys))
                else:
                    weight[row] = 0.0
    
        # put weight
        with casatools.TableReader(outfile, nomodify=False) as tb:
            tsel = tb.query('ROWNUMBER() IN %s' % (row_map.keys()), style='python')
            ms_weights = tsel.getcol('WEIGHT')
            rownumbers = tsel.rownumbers().tolist()
            for (k, v) in row_map.items():
                ms_row = k
                ms_weight = list((weight[row] for row in v))
                ms_index = rownumbers.index(ms_row)
                ms_weights[:, ms_index] = ms_weight
            tsel.putcol('WEIGHT', ms_weights)
            tsel.close()
    
        # set channel flag for min/max in each channel
        minmaxclip = False
        if minmaxclip:
            with casatools.TableReader(outfile, nomodify=False) as tb:
                tsel = tb.query('ROWNUMBER() IN %s' % (row_map.keys()),
                                style='python')
                if 'FLOAT_DATA' in tsel.colnames():
                    data_column = 'FLOAT_DATA'
                else:
                    data_column = 'DATA'
                data = tsel.getcol(data_column)  # (npol, nchan, nrow)
    
                (npol, nchan, nrow) = data.shape
                if nrow > 2:
                    argmax = data.argmax(axis=2)
                    argmin = data.argmin(axis=2)
                    print argmax.tolist()
                    print argmin.tolist()
                    flag = tsel.getcol('FLAG')
                    for ipol in xrange(npol):
                        maxrow = argmax[ipol]
                        minrow = argmin[ipol]
                        for ichan in xrange(nchan):
                            flag[ipol, ichan, maxrow[ichan]] = True
                            flag[ipol, ichan, minrow[ichan]] = True
                    tsel.putcol('FLAG', flag)
                    return (argmin, argmax)

    
