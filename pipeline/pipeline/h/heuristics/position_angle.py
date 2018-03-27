import numpy as np

import pipeline.infrastructure.api as api
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)

class PositionAngle(api.Heuristic):
    """Apply the position angle of a calibrator source to model data in MS.

    """

    def __init__(self):
        """Define a RM and parallactic angle lookup table.

            The values in this table can be found in the following
            paper by Perley and Butler.
                https://arxiv.org/abs/1302.6662
        """
        self.rm_lookup = {'3C48':  {'rotation_measure': -68, 'parallactic_angle': 122},
                     '3C138': {'rotation_measure': 0, 'parallactic_angle': -10},
                     '3C147': {'rotation_measure': -1467, 'parallactic_angle': 88},
                     '3C286': {'rotation_measure': 0, 'parallactic_angle': 33}}

    def calculate(self, measurement_set_name, pipeline_context):
        """Apply the position angle of a calibrator source to model data in MS.

        Args:
            measurement_set_name
            pipeline_context

            The MS name and context are used to retrieve the domain object and look
            for an amplitude calibrator.

            The name of the calibrator is then used to get the rotation measure
            and parallactic angle from a lookup table.

            Those values are used to compute and apply the position angle to
            the model data.

        """

        ms_domain_object = pipeline_context.observing_run.get_ms(measurement_set_name)

        field = ms_domain_object.get_fields(intent='AMPLITUDE')[0]
        if field.name not in self.rm_lookup.keys():
            LOG.warn('amplitude calibrator {0} not in rotation measure lookup table'.format(field.name))
            return
        else:
            rotation_measure = self.rm_lookup[field.name]['rotation_measure']
            parallactic_angle = self.rm_lookup[field.name]['parallactic_angle']

        with casatools.TableReader(measurement_set_name + '/SPECTRAL_WINDOW') as tb:
            channel_freq_column = tb.getcol('CHAN_FREQ')

        with casatools.MSReader(measurement_set_name, nomodify=False) as ms:
            for spw in range(channel_freq_column.shape[1]):
                print('spw={}'.format(spw))
                ms.select({'field_id': [field.id], 'data_desc_id': [spw]})
                mdata = ms.getdata('model_data')
                if not mdata:
                    LOG.warn('No model data in spectral window {0}'.format(spw))
                for cc in range(mdata['model_data'].shape[1]):
                    position_angle = parallactic_angle + rotation_measure * (3e8 / channel_freq_column[cc, spw]) ** 2

                    # print str(channel_freq_column[c,s])+' '+str(pa*180.0/pi)
                    qq = np.cos(2 * position_angle)
                    uu = np.sin(2 * position_angle)
                    mdata['model_data'][1, cc, :] = complex(qq, uu)
                    mdata['model_data'][2, cc, :] = complex(qq, -uu)

                ms.putdata(mdata)
                ms.reset()

            ms.close()
