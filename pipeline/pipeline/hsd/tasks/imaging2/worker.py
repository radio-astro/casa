from __future__ import absolute_import

import os
import numpy
import math

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from pipeline.infrastructure import casa_tasks
from .. import common
from ..common import temporary_filename

LOG = infrastructure.get_logger(__name__)

class SDImaging2WorkerInputs(common.SingleDishInputs):
    """
    Inputs for imaging worker
    """
    def __init__(self, context, infiles, outfile, spwid, onsourceid, 
                 edge=None, vislist=None):
        self._init_properties(vars())

class SDImaging2WorkerResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDImaging2WorkerResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDImaging2WorkerResults, self).merge_with_context(context)

    def _outcome_name(self):
        # return [image.imagename for image in self.outcome]
        return self.outcome


class SDImaging2Worker(common.SingleDishTaskTemplate):
    Inputs = SDImaging2WorkerInputs
    
    def prepare(self):
        infiles = self.inputs.infiles
        outfile = self.inputs.outfile
        edge = self.inputs.edge
        spwlist = [self.inputs.spwid for i in xrange(len(infiles))]
 
        self._do_imaging(infiles, spwlist, outfile, edge)
 
        result = SDImaging2WorkerResults(task=self.__class__,
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

    def _do_imaging(self, infiles, spwlist, imagename, edge):
        context = self.context
        datatable = self.datatable
        antenna_list = [context.observing_run.st_names.index(f) 
                        for f in infiles]
        vislist = [self.inputs.vislist[i] for i in antenna_list]
        srctype = self.inputs.onsourceid
        reference_data = context.observing_run[antenna_list[0]]
        spwid = spwlist[0]
        
        # imager tool
        im = casatools.imager
    
        # qa tool
        qa = casatools.quanta
    
        # field
        target_sources = [v for v in reference_data.source.values() 
                          if 'TARGET' in v.intents]
        source_name = target_sources[0].name
        field = '%s*' % (source_name)
    
        # baseline
        #baseline = '0&&&'
    
        # cellx and celly
        grid_size = reference_data.beam_size[spwid]
        cellx = qa.div(grid_size, 3.0)
        celly = cellx
        cell_in_deg = qa.convert(cellx, 'deg')['value']
        print 'cell=%s' % (cellx)
    
        # nx and ny
        tb = datatable.tb1
        datatable_name = datatable.plaintable
        index_list = []
        for (_spw, _antenna) in zip(spwlist, antenna_list):
            taqlstring = 'USING STYLE PYTHON SELECT ROWNUMBER() AS ID FROM "%s" WHERE IF==%s && SRCTYPE == %s && ANTENNA == %s' % (os.path.join(datatable_name, 'RO'), _spw, srctype, _antenna)
            LOG.debug('taqlstring=\'%s\'' % (taqlstring))
            tx = tb.taql(taqlstring)
            index_list.append(tx.getcol('ID'))
            tx.close()
            del tx
        index_list = numpy.sort(numpy.concatenate(index_list))
    
        ra = datatable.tb1.getcol('RA').take(index_list)
        dec = datatable.tb1.getcol('DEC').take(index_list)
    
        ra_min = min(ra)
        ra_max = max(ra)
        dec_min = min(dec)
        dec_max = max(dec)
        dec_correction = 1.0 / math.cos(0.5 * (dec_min + dec_max) / 180.0 * 3.1415926535897931)
    
        nx = int((ra_max - ra_min) / (cell_in_deg * dec_correction)) + 1
        ny = int((dec_max - dec_min) / cell_in_deg) + 1
        
        # Adjust nx and ny to be even number for performance (which is 
        # recommended by imager). 
        # Also increase nx and ny  by 2 if they are even number. 
        # This is due to a behavior of the imager. The imager configures 
        # direction axis as follows:
        #     reference value: phasecenter
        #           increment: cellx, celly
        #     reference pixel: ceil((nx-1)/2), ceil((ny-1)/2)
        # It means that reference pixel will not be map center if nx/ny 
        # is even number. It results in asymmetric area coverage on both 
        # sides of the reference pixel, which may miss certain range of 
        # (order of 1 pixel) observed area near the edge. 
        if nx % 2 == 0:
            nx += 2
        else:
            nx += 1
        if ny % 2 == 0:
            ny += 2
        else:
            ny += 1
    
        # increase nx and ny to reduce the effect of edges
        margin = 3
        nx += 2 * margin
        ny += 2 * margin
        LOG.info('nx,ny=%s,%s' % (nx, ny))
    
        # phasecenter
        ra_center_in_deg = (ra_min + ra_max) * 0.5
        dec_center_in_deg = (dec_min + dec_max) * 0.5
        ra_center = qa.quantity(ra_center_in_deg, 'deg')
        dec_center = qa.quantity(dec_center_in_deg, 'deg')
        phasecenter = 'J2000 %s %s' % (qa.formxxx(ra_center, 'hms'),
                                     qa.formxxx(dec_center, 'dms'))
        LOG.info('phasecenter=\'%s\'' % (phasecenter))
    
        # mode
        mode = 'channel'
    
        # stokes
        # stokes = 'I'
        stokes = 'XXYY'
    
        # start, nchan, step
        total_nchan = reference_data.spectral_window[spwid].nchan
        if total_nchan == 1:
            start = 0
            step = 1
            nchan = 1
        else:
            start = edge[0]
            step = 1
            nchan = total_nchan - sum(edge)
    
        # restfreq
        spw = reference_data.spectral_window[spwid]
        rest_freqs = spw.rest_frequencies
        if len(rest_freqs) > 0:
            restfreq = rest_freqs[0]
        else:
            restfreq = spw.refval
    
        # outframe
        outframe = 'LSRK'
    
        # gridfunction
        gridfunction = 'GAUSS'
    
        # truncate, gwidth, jwidth, and convsupport
        truncate = '3pixel'
        gwidth = '1.5pixel'
        jwidth = '-1pixel'  # default (not used)
        convsupport = 3
    
        temporary_name = imagename.rstrip('/')+'.tmp'
        with temporary_filename(temporary_name) as name:
            # imaging
            for (vis, spw) in zip(vislist, spwlist):
                im.selectvis(vis=vis, spw=spw, field=field)
            im.defineimage(nx=nx, ny=ny, cellx=cellx, celly=celly, stokes=stokes,
                           phasecenter=phasecenter, mode=mode, start=start,
                           nchan=nchan, step=step, restfreq=restfreq,
                           outframe=outframe, spw=spwlist[0])
            im.setoptions(ftmachine='sd', gridfunction=gridfunction)
            im.setsdoptions(convsupport=convsupport, truncate=truncate,
                            gwidth=gwidth, jwidth=jwidth)
            im.makeimage(image=name)
            im.done()
            
            # post imaging process
            # cut margin area
            if os.path.exists(imagename):
                os.system('rm -rf %s'%(imagename))
            box = '%s,%s,%s,%s'%(margin, margin, nx - margin - 1, ny - margin - 1)
            args = {'imagename': name,
                      'mode': 'evalexpr',
                      'expr': 'IM0',
                      'outfile': imagename,
                      'box': box}
            LOG.debug('Executing immath task: args=%s'%(args))
            job = casa_tasks.immath(**args)
            self._executor.execute(job, merge=False)
            
        # post imaging process
        # set brightness unit to K
        #with casatools.ImageReader(imagename) as ia:
            #ia.setbrightnessunit('K')
            
                
