from __future__ import absolute_import

import numpy as np
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)


def printTsysFlags(tsystable, htmlreport):
    """Method that implements a version of printTsysFlags by Todd Hunter.
    """
    with casatools.TableReader(tsystable) as mytb:
        flags = mytb.getcol("FLAG")
        npol = len(flags)
        antIndices=mytb.getcol("ANTENNA1")
        spws=mytb.getcol("SPECTRAL_WINDOW_ID")
        times=mytb.getcol("TIME")
        fields=mytb.getcol("FIELD_ID")
        uniqueTimes = np.unique(times)

    with casatools.TableReader(tsystable+"/ANTENNA") as mytb:
        antNames = mytb.getcol("NAME")

    with open(htmlreport, 'w') as stream:
        stream.write('<html>')

        for iant in range(len(antNames)):
            for spw in np.unique(spws):
                # zsel will be an array of (a few) row numbers that
                # match the ant and spw
                zsel = np.where((antIndices==iant)*(spws==spw))[0]

                for pol in range(npol):
                    for t in range(len(zsel)):
                        zflag=np.where(flags[pol,:,zsel[t]])

                        if (len(zflag[0]) > 0):
                            if (len(zflag[0]) == len(flags[0,:,zsel[t]])):
                                chans = 'all channels'
                            else:
                                chans = '%d channels: ' % (
                                  len(zflag[0])) + str(zflag[0])

                            timeIndex = list(uniqueTimes).index(times[zsel[t]])
                            myline = antNames[iant] + \
                              " (#%02d), field %d, time %d, pol %d, spw %d, "%(
                              iant, fields[zsel[t]], timeIndex, pol, spw) + \
                              chans + "<br>\n"

                            stream.write(myline)

            # format break between antennas
            stream.write('<br>\n')

def renderflagcmds(flagcmdsfile, htmlflagcmds, reason):
    """Method to render a flagdata file into html format. flagcmds 
    that match the given reason are rendered black, others grey.
    """
    if type(flagcmdsfile) is types.StringType:
        # input flagcmds from file named
        with open(flagcmdsfile, 'r') as stream:
            lines = stream.readlines()
    elif type(flagcmdsfile) is types.ListType:
        # flagcmds are listed in flagcmdsfile variable
        lines = flagcmdsfile
    else:
        raise Exception, 'bad type for flagcmdsfile: %s' % type(flagcmdsfile)

    with open(htmlflagcmds, 'w') as stream:
        stream.write('<html>')
        stream.write('<head/>')
        stream.write('<body>')
        stream.write('''This is the list of flagcmds created by this stage.
          <br>''')

        for line in lines:
            if reason != '' and reason in line:
                stream.write('%s<br>' % line)
        stream.write('</body>')
        stream.write('</html>')

