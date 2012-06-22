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
  Atoms/Servo_clnt.cc
  Atoms/Servo_xdr.cc
  PKSIO/ASTEDataset.cc
  PKSIO/ASTEFXDataset.cc
  PKSIO/ASTEFXReader.cc
  PKSIO/ASTEReader.cc
  PKSIO/FITSreader.cc
  PKSIO/MBFITSreader.cc
  PKSIO/MBrecord.cc
  PKSIO/NRO45FITSReader.cc
  PKSIO/NRO45Reader.cc
  PKSIO/NRODataset.cc
  PKSIO/NROFITSDataset.cc
  PKSIO/NROOTFDataset.cc
  PKSIO/NROReader.cc
  PKSIO/PKSFITSreader.cc
  PKSIO/PKSMS2reader.cc
  PKSIO/PKSMS2writer.cc
  PKSIO/PKSreader.cc
  PKSIO/PKSSDwriter.cc
  PKSIO/SDFITSreader.cc
  PKSIO/SDFITSwriter.cc
  PKSIO/GBTFITSreader.cc
  pks/pks_maths.cc
   )

install(FILES
	PKSIO/ASTEDataset.h
	PKSIO/ASTEFXDataset.h
	PKSIO/ASTEFXReader.h
	PKSIO/ASTEReader.h
	PKSIO/FITSreader.h
	PKSIO/GBTFITSreader.h
	PKSIO/MBFITSreader.h
	PKSIO/MBrecord.h
	PKSIO/NRO45FITSReader.h
	PKSIO/NRO45Reader.h
	PKSIO/NRODataRecord.h
	PKSIO/NRODataset.h
	PKSIO/NROFITSDataset.h
	PKSIO/NROOTFDataset.h
	PKSIO/NROReader.h
	PKSIO/PKSFITSreader.h
	PKSIO/PKSMS2reader.h
	PKSIO/PKSMS2writer.h
	PKSIO/PKSSDwriter.h
	PKSIO/PKSreader.h
	PKSIO/PKSrecord.h
	PKSIO/PKSwriter.h
	PKSIO/SDFITSreader.h
	PKSIO/SDFITSwriter.h
	PKSIO/SrcType.h
	DESTINATION include/casacode/atnf/PKSIO
	}
install(FILES
	pks/pks_maths.h
	DESTINATION include/casacode/atnf/pks
	}
