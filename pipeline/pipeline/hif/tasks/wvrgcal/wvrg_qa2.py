from __future__ import absolute_import
import math
import numpy as np

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

from pipeline.hif.tasks.common import commonresultobjects
from . import resultobjects


LOG = infrastructure.get_logger(__name__)


def calculate_qa2_numbers(result):
    """Calculate a single number from the qa2 views stored in result.
    
    result -- The Qa2Result object containing the Qa2 views.
    """
    # configure numpy to ignore floating point errors in the divide that
    # follows.
    old_settings = np.seterr(divide='ignore', invalid='ignore')

    qa2_per_view = {}

    for description in result.descriptions():
        before = result.first(description)
        after = result.last(description)

        qa2_data = before.data / after.data 
        qa2_flag = np.logical_or(before.flag, after.flag)
        qa2_flag[np.isinf(qa2_data)] = True
        qa2_flag[np.isnan(qa2_data)] = True

        qa2_per_view[description] = np.median(qa2_data[qa2_flag==False])
    
    # restore normal numpy handling of division errors 
    ignore = np.seterr(**old_settings)

    result.view_score = qa2_per_view
    result.overall_score = np.median(qa2_per_view.values())

def calculate_view(context, gaintable, result):
    """
    tsystable -- CalibrationTableData object giving access to the wvrg
                 caltable.
    """
    # raise an exception when np encounters any error
    np.seterr(all='raise')

    with casatools.TableReader(gaintable) as table:
        result.vis = table.getkeyword('MSName')

        # access field names via domain object
        ms = context.observing_run.get_ms(name=result.vis)
        fields = ms.fields

        fieldidcol = table.getcol('FIELD_ID')
        fieldids = list(set(fieldidcol))
        fieldids.sort()

        for fieldid in fieldids:
            fieldname = [fld.name for fld in fields if fld.id==fieldid][0]

            sb = table.query('FIELD_ID==%s' % fieldid)

            spectral_window_id = sb.getcol('SPECTRAL_WINDOW_ID')
            spwids = list(set(spectral_window_id))
            spwids.sort()
            antenna1 = sb.getcol('ANTENNA1')
            antennas = set(antenna1)
            timecol = sb.getcol('TIME')
            times = list(set(timecol))
            times.sort()
            times = np.array(times)
            # look for continuously sampled chunks of data. Sometimes
            # there are small gaps of 10-15 sec within lengths of data
            # that we want to regard as continuous, so make the minimum
            # 'gap' larger than this.
            time_chunks = findchunks(times, gap_time=30.0)

            cparam = sb.getcol('CPARAM')
            flag = sb.getcol('FLAG')
            rows = np.arange(sb.nrows())

            # to eliminate the effect of the refant on the results 
            # (i.e. phase rms of refant is always 0) we first
            # construct 8 versions of the gain table with the entries
            # corresponding to 8 different values for the refant.
            cparam_refant = {}
            flag_refant = {}
            refants = list(antennas)[:8]
            for refant in refants:
                cparam_refant[refant] = np.array(cparam)
                flag_refant[refant] = np.array(flag)
 
                for spwid in spwids:
                    for t in times:
                        # gains for all antennas at time t
                        selected_rows = rows[(spectral_window_id==spwid) & 
                          (timecol==t)]
                        gain = cparam_refant[refant][0,0,selected_rows]
                        gain_flag = flag_refant[refant][0,0,selected_rows]

                        # gain for refant at time t
                        refant_row = rows[(spectral_window_id==spwid) & 
                          (timecol==t) & (antenna1==refant)]
                        if refant_row:
                            refant_gain = \
                              cparam_refant[refant][0,0,refant_row][0]
                            refant_gain_flag = \
                              flag_refant[refant][0,0,refant_row][0]
                        else:
                            refant_gain_flag = True

                        # now modify entries to make refant the reference
                        if not refant_gain_flag:
                            # use a.b = ab cos(theta) axb = ab sin(theta) to 
                            # calculate theta
                            dot_product = gain.real * refant_gain.real + \
                              gain.imag * refant_gain.imag
                            cross_product = gain.real * refant_gain.imag - \
                              gain.imag * refant_gain.real

                            complex_rel_phase = np.zeros([len(dot_product)],
                              np.complex)
                            complex_rel_phase.imag = np.arctan2(cross_product,
                              dot_product)

                            cparam_refant[refant][0,0,selected_rows] = \
                              np.abs(gain) * np.exp(-complex_rel_phase)
                        else:
                            flag_refant[refant][0,0,selected_rows] = True

            # now calculate the phase rms views
            for spwid in spwids:
                data = np.zeros([max(antennas)+1, len(time_chunks)])
                data_flag = np.ones([max(antennas)+1, len(time_chunks)],
                  np.bool)
                chunk_base_times = np.zeros([len(time_chunks)])

                for antenna in antennas:
                    selected_rows = rows[(spectral_window_id==spwid) & 
                      (antenna1==antenna)]

                    gain_times = timecol[selected_rows]

                    for i,chunk in enumerate(time_chunks):
                        chunk_start = times[chunk[0]] - 0.1
                        chunk_end = times[chunk[-1]] + 0.1
                        chunk_select = (gain_times >= chunk_start) & \
                          (gain_times <= chunk_end)
                        if not chunk_select.any():
                            continue

                        gain_times_chunk = gain_times[chunk_select]
                        chunk_base_times[i] = gain_times_chunk[0]

                        rms_refant = np.zeros([8])
                        rms_refant_flag = np.ones([8], np.bool)

                        for refant in refants:
                            gain = cparam_refant[refant][0,0,selected_rows]
                            gain_chunk = gain[chunk_select]
                            gain_flag = flag_refant[refant][0,0,selected_rows]
                            gain_flag_chunk = gain_flag[chunk_select]
                                 
                            try:
                                valid = np.logical_not(gain_flag_chunk)
                                if not np.any(valid):
                                    continue

                                # complex median: median(reals) + 
                                # i*median(imags)
                                cmedian = complex(
                                  np.median(gain_chunk.real[valid]),
                                  np.median(gain_chunk.imag[valid]))

                                # use a.b = ab cos(theta) and 
                                # axb = ab sin(theta) to calculate theta
                                dot_product = \
                                  gain_chunk.real[valid] * cmedian.real + \
                                  gain_chunk.imag[valid] * cmedian.imag
                                cross_product = \
                                  gain_chunk.real[valid] * cmedian.imag - \
                                  gain_chunk.imag[valid] * cmedian.real

                                phases = \
                                  np.arctan2(cross_product, dot_product) * \
                                  180.0 / math.pi

                                # calculate phase rms
                                phases *= phases
                                phase_rms = np.sum(phases) / float(len(phases))
                                phase_rms = math.sqrt(phase_rms)
                                rms_refant[refant] = phase_rms
                                rms_refant_flag[refant] = False
                            except:
                                pass

                        # set view
                        valid_data = rms_refant[
                          np.logical_not(rms_refant_flag)]
                        if len(valid_data) > 0:
                            data[antenna,i] = np.median(valid_data)
                            data_flag[antenna,i] = False

                axes = [commonresultobjects.ResultAxis(name='Antenna',
                  units='id', data=np.arange(max(antennas)+1)),
                  commonresultobjects.ResultAxis(name='Time', units='',
                  data=chunk_base_times)]

                phase_rms_result = commonresultobjects.ImageResult(
                  filename=result.vis, data=data, datatype='r.m.s. phase',
                  axes=axes, flag=data_flag, field_id=fieldid, 
                  field_name = fieldname, spw=spwid, units='degrees')

                result.addview(phase_rms_result.description, phase_rms_result)

            # free sub table and its resources
            sb.done()

def findchunks(times, gap_time):
    """Return a list of arrays, each containing the indices of a chunk
    of data i.e. a sequence of equally spaced measurements separated
    from other chunks by larger time gaps.

    Keyword arguments:
    times    -- Numeric array of times at which the measurements
                were taken.
    gap_time -- Minimum gap that signifies a 'gap'.
    """
    difference = times[1:] - times[:-1]
    median_diff = np.median(difference)
    gap_diff = max(1.5 * median_diff, gap_time)

    chunks = []
    chunk = [0]
    for i in np.arange(len(difference)):
        if difference[i] < gap_diff:
            chunk.append(i+1)
        else:
            chunks.append(np.array(chunk))
            chunk = [i+1]
    chunks.append(np.array(chunk))
    return chunks


