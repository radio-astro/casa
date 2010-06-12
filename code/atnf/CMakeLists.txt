#
# CASA - Common Astronomy Software Applications
# Copyright (C) 2010 by ESO (in the framework of the ALMA collaboration)
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

casa_add_library( atnf
  implement/Atoms/Servo_clnt.cc
  implement/Atoms/Servo_xdr.cc
  implement/PKSIO/ASTEDataset.cc
  implement/PKSIO/ASTEFXDataset.cc
  implement/PKSIO/ASTEFXReader.cc
  implement/PKSIO/ASTEReader.cc
  implement/PKSIO/FITSreader.cc
  implement/PKSIO/MBFITSreader.cc
  implement/PKSIO/MBrecord.cc
  implement/PKSIO/NRO45FITSReader.cc
  implement/PKSIO/NRO45Reader.cc
  implement/PKSIO/NRODataset.cc
  implement/PKSIO/NROFITSDataset.cc
  implement/PKSIO/NROOTFDataset.cc
  implement/PKSIO/NROReader.cc
  implement/PKSIO/PKSFITSreader.cc
  implement/PKSIO/PKSMS2reader.cc
  implement/PKSIO/PKSMS2writer.cc
  implement/PKSIO/PKSreader.cc
  implement/PKSIO/PKSSDwriter.cc
  implement/PKSIO/SDFITSreader.cc
  implement/PKSIO/SDFITSwriter.cc
  implement/pks/pks_maths.cc
   )

