#
# CASA - Common Astronomy Software Applications
# Copyright (C) 2015
# Associated Universities, Inc. Washington DC, USA.
# Copyright by ESO (in the framework of the ALMA collaboration).
#
# This file is part of CASA.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

casa_add_library ( bnmin1
    src/bnmin_main.cxx
    src/gradientminim.cxx
    src/gradientmodel.cxx
    src/linemodel.cxx
    src/lmmin.cxx
    src/lmminutils.cxx
    src/markovchain.cxx
    src/mcmonitor.cxx
    src/mcpoint.cxx
    src/metropolis.cxx
    src/metro_propose.cxx
    src/minim.cxx
    src/minimio.cxx
    src/minimmodel.cxx
    src/monitor.cxx
    src/nestederr.cxx
    src/nestedinitial.cxx
    src/nestedsampler.cxx
    src/paramalgo.cxx
    src/prior_sampler.cxx
    src/priors.cxx
    src/robustline.cxx
    src/twoerrline.cxx
    src/twoerrline_ml.cxx
)


