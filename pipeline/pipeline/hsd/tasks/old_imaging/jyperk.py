from __future__ import absolute_import

import os
import numpy
import datetime

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.basetask as basetask
from .. import common

LOG = infrastructure.get_logger(__name__)

class SDJyPerKInputs(common.SingleDishInputs):
    """
    Inputs for imaging
    """
    def __init__(self, context, jyperk_item_list):
        self._init_properties(vars())

    @property
    def outfile(self):
        return 'jyperk.csv'

class SDJyPerKResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDJyPerKResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(SDJyPerKResults, self).merge_with_context(context)
        LOG.todo('need to decide what is done in SDJyPerKResults.merge_with_context')

    def _outcome_name(self):
        # return [image.imagename for image in self.outcome]
        return self.outcome

class SDJyPerK(common.SingleDishTaskTemplate):
    Inputs = SDJyPerKInputs

    @common.datatable_setter
    def prepare(self):
        context = self.inputs.context
        jyperk_items = self.inputs.jyperk_item_list
        args = self.inputs.to_casa_args()
        outfile = self.inputs.outfile

        if len(jyperk_items) == 0: raise Exception, "No image to calculate Jy/K factor"
        
        ous_pid = context.project_structure.ousstatus_entity_id
        sourcename = jyperk_items[0].sourcename
        fluxes={}
        body_text = []
        for this_item in jyperk_items:
            # derive Jy/K factor for each image
            objname = this_item.sourcename
            imagename = this_item.imagename
            antenna = this_item.antenna
            file_index = this_item.file_index
            
            LOG.info("Deriving Jy/K Factor from %s" % imagename)
            (center_mhz, width_mhz, pol) = self.get_image_axis(imagename)
            (time_start, time_end) = self.get_session_time(context, file_index)
            with casatools.ImageReader(imagename) as ia:
                peak_k = ia.statistics()['max'][0]
            srcflux = self.get_flux_of_source(1.5)
            factor = srcflux/peak_k
            LOG.info("- image peak = %f K" % (peak_k))
            LOG.info("- source flux (%s) = %f Jy" % (objname, srcflux))
            LOG.info("- Jy/K factor = %f" % (factor))
            # check for source name
            if objname != sourcename:
                LOG.warn('More than one source name found for amplitude calibration. Only the first source will be listed in header')
            
            session = context.observing_run[file_index[0]].ms.session
            body_text.append(self.format_body(session, time_start, time_end, antenna, center_mhz, width_mhz, pol, factor))

        # generate csv file
        fileobj = open(outfile, 'w')
        try:
            self.write_header(fileobj,ous_pid, sourcename, fluxes)
            for data in body_text:
                fileobj.write(data)
        finally:
            fileobj.close()

        result = SDJyPerKResults(task=self.__class__,
                                 success=True,
                                 outcome=outfile)

        if self.inputs.context.subtask_counter is 0: 
            result.stage_number = self.inputs.context.task_counter - 1
        else:
            result.stage_number = self.inputs.context.task_counter 

        return result

    def analyse(self, result):
        return result
    
    def get_image_axis(self, imagename):
        """Check image axis and return the band center frequency [MHz], band width [MHz], and polarization of image"""
        with casatools.ImageReader(imagename) as ia:
            cs = ia.coordsys()
            imshape = ia.shape()
            freq_ax = cs.findcoordinate("spectral")['world'][0]
            pol_ax = cs.findcoordinate("stokes")['world'][0]
            pol = cs.referencevalue(type='stokes', format='s')['string'][0]
            if imshape[freq_ax] != 1: LOG.error("Integrated intensity image (1 channel) is reuired for Jy/K conversion.")
            if imshape[pol_ax] != 1 or pol != 'I':
                LOG.error("Stokes I image is required for Jy/K conversion")
            qcenter = cs.referencevalue(type='spectral',format='q')['quantity']['*1']
            qincr = cs.increment(type='spectral',format='q')['quantity']['*1']
            cs.done()
        
        qa = casatools.quanta
        center_mhz = qa.getvalue(qa.quantity(qcenter, 'MHz'))
        width_mhz = qa.getvalue(qa.quantity(qincr, 'MHz'))
        return (center_mhz, width_mhz, pol)

    def get_session_time(self, context, file_index):
        """Returns the start and end time of observation of the session"""
        if len(file_index)==0:
            raise Exception, "empty file index."
        ms_list = [context.observing_run[idx].ms for idx in file_index]
        # sort by start time
        sms = self._sorted_ms(ms_list, 'start_time')
        session_start = infrastructure.utils.get_epoch_as_datetime(sms[0].start_time)
        # sort by end time
        sms = self._sorted_ms(ms_list, 'end_time')
        session_end = infrastructure.utils.get_epoch_as_datetime(sms[-1].end_time)
        return (session_start, session_end)

    def _sorted_ms(self, ms_list, time):
        # picked from domain.observingrun.start_time
        qa = casatools.quanta
        return sorted(ms_list, key=lambda ms: getattr(ms, time)['m0'],
                      cmp=lambda x,y: 1 if qa.gt(x,y) else 0 if qa.eq(x,y) else -1)

    def get_flux_of_source(self, val):
        """Returns flux of a source at a give observation day"""
        return val

    def get_flux_of_solar_object(self):
        """Returns flux of a solar system object"""

    def get_flux_of_qso(self):
        """Returns flux of a QSOs"""
        server = 'http://asa.alma.cl/sourcecat/xmlrpc'
#         ccu = calDatabaseQuery.CalibratorCatalogUpdate(tunnel=tunnel,server=server)
#         if (ccu.connectionFailed):
#             LOG.warn("Failed to connect to ")
#             return(None)
#         status = ccu.searchFlux(sourcename,date,band,fLower,fUpper,tunnel,maxrows,
#                                 limit,debug,server,dateCriteria,verbose,measurements,
#                                 returnMostRecent,showDateReduced, sourceBandLimit=limit,
#                                 dayWindow=dayWindow)
#         if (status == -1):
#             return (None)
#         return(status)

    def write_header(self, fileobj, ous_pid, object_name, fluxes):
        """Fills header of Jy/K file"""
        import pipeline
        calc_date = infrastructure.utils.format_datetime(datetime.datetime.utcnow())
        fileobj.write("##Jy/K factor generated by pipeline %s\n" % pipeline.revision)
        fileobj.write("##Calculated on %s\n" % calc_date)
        fileobj.write("##\n")
        fileobj.write("#OUSPID=%s\n" % ous_pid)
        fileobj.write("#OBJECT=%s\n" % object_name)
        fileobj.write("#FLUXJY=%s\n" % str(",").join(fluxes.values()))
        fileobj.write("#FLUXFREQ=%s\n" % str(",").join(fluxes.keys()))
        fileobj.write("##\n")
        fileobj.write("##SessionID,ObservationStartDate(UTC),ObservationEndDate(UTC),Antenna,BandCenter(MHz),BandWidth(MHz),Pol,Factor\n")

    def format_body(self, session, time_start, time_end, antenna, center_mhz, width_mhz, pol, factor):
        """Formats values to a comma separated strings for the body of Jy/K file"""
        session_id = session.split('_')[1]
        obs_start = infrastructure.utils.format_datetime(time_start)
        obs_end = infrastructure.utils.format_datetime(time_end)
        return ("%s,%s,%s,%s,%f,%f,%s,%f\n" % (session_id, obs_start, obs_end, antenna, center_mhz, width_mhz, pol, factor))
   

class JyPerKItems:
    def __init__(self, imagename, sourcename, antenna, file_index):
        self.imagename = imagename
        self.sourcename = sourcename
        self.antenna = antenna
        self.file_index = file_index