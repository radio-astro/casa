import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

ephemeris_fields = set(['Venus', 'Mars', 'Jupiter', 'Uranus', 'Neptune',
                        'Pluto', 'Io', 'Europa', 'Ganymede', 'Callisto',
                        'Titan', 'Triton', 'Ceres', 'Pallas', 'Vesta',
                        'Juno', 'Victoria', 'Davida'])

LOG = infrastructure.get_logger(__name__)

def fixcaltable (msname, caltable):

    '''
    This routine if for use with the C3R4 pipeline release and
    should be removed for C3R4
    '''

    # Return if there is no caltable:
    if not os.path.exists(caltable):
        LOG.warn('Caltable %s does not exist' % os.path.basename(caltable))
        return

    # Get a table tool
    table = casatools.table

    # Get field name and ephemeris id columns from the MS
    #    Ephemeris ids are required to tell if the data is Cycle 3
    #    Names might be useful later
    table.open (os.path.join(msname, 'FIELD'))
    msfieldnames = table.getcol('NAME')
    msephemids = table.getcol('EPHEMERIS_ID')
    table.close()

    # Open the MSMD tool
    with casatools.MSMDReader (msname) as msmd:

        # Open field table and get field names
        table.open (os.path.join(caltable, 'FIELD'), nomodify = False)
        fieldnames = table.getcol('NAME')

        # Check that the MS and field tables have the same length
        if len(fieldnames) != len(msfieldnames):
            LOG.warn('Caltable %s and MS %s FIELD tables are not the same size' % (os.path.basename(caltable), os.path.basename(mnsname)))
            table.close
            return

        # Get the directions
        phasedirs = table.getcol('PHASE_DIR')
        delaydirs = table.getcol('DELAY_DIR')
        refdirs = table.getcol('REFERENCE_DIR')

        # Update the directions for ephemeris fields
        nupdates = 0
        for i in range(len(fieldnames)):
            # Object does not have an ephemeris table
            if msephemids[i] < 0:
                continue
            # Object is not a known ephemeris calibrator
            if fieldnames[i] not in ephemeris_fields:
                continue
            LOG.info('Fixing coordinates for field %s in caltable %s' % (fieldnames[i], os.path.basename(caltable)))
            coords = msmd.phasecenter(i)
            m0 = coords['m0']['value']
            m1 = coords['m1']['value']

            phasedirs[0][0][i] = m0
            phasedirs[1][0][i] = m1
            delaydirs[0][0][i] = m0
            delaydirs[1][0][i] = m1
            refdirs[0][0][i] = m0
            refdirs[1][0][i] = m1

            nupdates = nupdates + 1

        # Update the table
        if nupdates > 0:
            table.putcol('PHASE_DIR', phasedirs)
            table.putcol('DELAY_DIR', delaydirs)
            table.putcol('REFERENCE_DIR', refdirs)
            table.flush()

        table.close()
