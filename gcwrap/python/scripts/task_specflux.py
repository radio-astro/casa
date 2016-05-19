
##########################################################################
# task_specflux.py
#
# Copyright (C) 2008, 2009, 2010
# Associated Universities, Inc. Washington DC, USA.
#
# This script is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation,
# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning AIPS++ should be adressed as follows:
#        Internet email: aips2-request@nrao.edu.
#        Postal address: AIPS++ Project Office
#                        National Radio Astronomy Observatory
#                        520 Edgemont Road
#                        Charlottesville, VA 22903-2475 USA
#
# <author>
# Dave Mehringer
# </author>
#

###########################################################################
from taskinit import *
import os.path
import numpy

def specflux(
    imagename, box, region, chans, stokes, mask,
    stretch, unit, major, minor, logfile, overwrite
):
    casalog.origin('specflux')
    myia = iatool()
    myrg = rgtool()
    try:
        if logfile and not overwrite and os.path.exists(logfile):
            raise Exception(logfile + " exists and overwrite is False")
        myia.open(imagename)
        try:
            axis = myia.coordsys().axiscoordinatetypes().index("Spectral")
        except Exception, instance:
            raise Exception("Image does not have a spectral coordinate, cannot proceed")
        csys=myia.coordsys()
        reg = myrg.frombcs(
            csys=csys.torecord(), shape=myia.shape(), box=box,
            chans=chans, stokes=stokes, stokescontrol="a", region=region
        )
        if bool(major) != bool(minor):
            raise Exception("You must specify both of major and minor, or neither of them")
        if bool(major):
            bunit = myia.brightnessunit()
            if (bunit.find("/beam") >= 0):
                myia = myia.subimage()
                myia.setrestoringbeam(major=major, minor=minor, pa="0deg")
            else:
                casalog.post(
                    "Image brightness unit is " + bunit
                        + ". Ignorming major and minor specificaitons.",
                    "WARN"
                )
        rec = myia.getprofile(
            axis=axis, function="flux", region=reg, 
            mask=mask, unit=unit, stretch=stretch
        )
        xunit = rec['xUnit']
        wreg = "region=" + str(region)
        if box or chans or stokes:
            wreg = "box=" + box + ", chans=" + chans + ", stokes=" + stokes
        header = "# " + imagename + ", " + wreg + "\n"
        if myia.restoringbeam():
            beamsize = myia.beamarea()
            header += "# beam size: " + str(beamsize['arcsec2'])
            header += " arcsec2, " + str(beamsize["pixels"]) + " pixels\n"
        else:
            header += "# no beam\n"
        coords = rec['coords']
        shifted = list(coords)
        shifted.pop(0)
        shifted = numpy.array(shifted)
        increments = shifted - coords[:-1]
        increments = numpy.append(increments, increments[-1])
        increments = numpy.abs(increments)
        fd = rec['values']
        flux = numpy.sum(fd*increments)
        header += "# Total flux: " + str(flux) + " " + rec['yUnit'] + "." + xunit + "\n"
        need_freq = True
        need_vel = True
        myq = qa.quantity("1" + xunit)
        if qa.convert(qa, "km/s")['unit'] == "km/s":
            need_vel = False
            vels = rec['coords']
            vel_unit = xunit
        elif qa.convert(qa, "MHz")['unit'] == "MHz":
            need_freq = False
            freqs = rec['coords']
            freq_unit = xunit
        if need_vel:
            vels = myia.getprofile(
                axis=axis, function="flux", region=reg, 
                mask=mask, unit="km/s", stretch=stretch
            )['coords']
            vel_unit = "km/s"
        if need_freq:
            freqs = myia.getprofile(
                axis=axis, function="flux", region=reg, 
                mask=mask, unit="MHz", stretch=stretch
            )['coords']
            freq_unit = "MHz"
        freq_col = "frequency_(" + freq_unit + ")"
        freq_width = len(freq_col)
        freq_spec = "%" + str(freq_width) + ".6f"
        vel_col = "Velocity_(" + vel_unit + ")"
        vel_width = len(vel_col)
        vel_spec = "%" + str(vel_width) + ".6f"
        flux_col = "Flux_density_(" + rec['yUnit'] + ")"
        flux_width = len(flux_col)
        flux_spec = "%" + str(flux_width) + ".6e"
        header += "# Channel number_of_unmasked_pixels " + freq_col
        header += " " + vel_col
        header += " " + flux_col + "\n"
        planes = rec['planes']
        npix = rec['npix']
        for i in range(len(rec['values'])):
            header += "%9d %25d " % (planes[i], npix[i])
            header += freq_spec % (freqs[i]) + " "
            header += vel_spec % (vels[i]) + " "
            header += flux_spec % (fd[i])
            header += "\n"
        casalog.post(header, "NORMAL")
        if (logfile):
            with open(logfile, "w") as myfile:
                myfile.write(header)
        return True
    except Exception, instance:
        casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
        raise
    finally:
        if (myia):
            myia.done()
            myrg.done()

