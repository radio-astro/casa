//# FlagAgentRFlag.cc: This file contains the implementation of the FlagAgentAntennaIntegrations class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2017, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2017, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#include <flagging/Flagging/FlagAgentAntennaIntegrations.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Utilities/DataType.h>

namespace casa { //# NAMESPACE CASA - BEGIN

/**
 * Will use the per-row iteration approach, with vis-buffer preprocessing.
 *
 * @param dh Data handler, just passed to the base class 
 * @param config Flag agent configuration object, antint specific parameters are processed
 * @param writePrivateFlagCube Write option for the base class
 * @param flag Flag for the base class
 */
FlagAgentAntennaIntegrations::FlagAgentAntennaIntegrations(
                             FlagDataHandler *dh, casacore::Record config, 
			     casacore::Bool writePrivateFlagCube, 
			     casacore::Bool flag):
  FlagAgentBase(dh, config, FlagAgentBase::ROWS_PREPROCESS_BUFFER, writePrivateFlagCube,
		flag)
{
  logger_p->origin(casacore::LogOrigin(agentName_p,__FUNCTION__,WHERE));

  setAgentParameters(config);
}

/**
 * Deal with parameters specific to 'antint'.
 *
 * minchanfrac: fraction of channels with flagged antenna to initiate flagging.
 * verbose: boolean to print the timestamps of the integrations that are flagged
 *
 * @param config A flagdata configuration/parameters object
 */
void FlagAgentAntennaIntegrations::setAgentParameters(casacore::Record config)
{
  const casacore::uInt fields = config.nfields();
  *logger_p << casacore::LogIO::NORMAL << "The configuration received by this agent has " 
	    << fields << " fields with the following values:" << casacore::LogIO::POST;
  ostringstream os;
  config.print(os);
  *logger_p << casacore::LogIO::NORMAL << os.str() << casacore::LogIO::POST;


  const std::string minChanOpt = "minchanfrac";
  int found = config.fieldNumber(minChanOpt);
  if (found >= 0) {
    minChanThreshold_p = config.asDouble(minChanOpt);
  }

  const std::string verboseOpt = "verbose";
  found = config.fieldNumber(verboseOpt);
  if (found >= 0) {
    verbose_p = config.asBool(verboseOpt);
  }
}

/*
 * Processes blocks of rows by every point in time, 
 * Checking all the baselines to the antenna of interest
 *
 * @param visBuffer Buffer to pre-process
 */
void
FlagAgentAntennaIntegrations::preProcessBuffer(const vi::VisBuffer2 &visBuffer)
{
  const casacore::Vector<casacore::Double> time_p = visBuffer.time();
  casacore::uInt rowCnt = time_p.size();
  if (rowCnt <= 0)
    return;

  casacore::uInt antennasCnt = visBuffer.nAntennas();
  casacore::uInt channelsCnt = visBuffer.nChannels();
  *logger_p << casacore::LogIO::DEBUG1 << "Pre-processing visibility buffer, with "
	    << " nRows: " << visBuffer.nRows()
	    << " nCorrelations: " << visBuffer.nCorrelations()
	    << " nChannels: " << channelsCnt << ", nAntennas:: " 
	    << antennasCnt << casacore::LogIO::POST;  

  // TODO: use a casacore::Matrix here?
  TableFlagPerBaselinePerChannel flagPerBaselinePerChannel;
  flagPerBaselinePerChannel.resize(antennasCnt - 1);
  for(auto &row : flagPerBaselinePerChannel) {
    row.assign(channelsCnt, false);
  }

  const casacore::Vector<casacore::Int> ant1 = visBuffer.antenna1();
  const casacore::Vector<casacore::Int> ant2 = visBuffer.antenna2();
  const casacore::Cube<casacore::Bool> polChanRowFlags = visBuffer.flagCube();
  casacore::Double currentTime = time_p[0];
  casacore::uInt baselineIdx = 0;
  for (casacore::uInt rowIdx = 0; rowIdx < rowCnt; ++rowIdx) {
    casacore::Double rowTime = visBuffer.time()[rowIdx];

    *logger_p << casacore::LogIO::DEBUG2 << " iteration, ant1 id:: " << ant1[rowIdx] << ", ant 2 id: " << ant2[rowIdx] << casacore::LogIO::POST;  

    // updateNextRowTime
    rowTime = time_p[rowIdx];
    if (rowTime != currentTime) {
      doPreProcessingTimePoint(doFlagTime_p, currentTime, flagPerBaselinePerChannel);
      currentTime = rowTime;
      baselineIdx = 0;
    }

    // This is selected already in the generic selection of FlagAgentBase
    //if (ant1[rowIdx] != antId_p and ant2[rowIdx] != antId_p)
    //  continue;

    checkAnyPolarizationFlagged(polChanRowFlags, flagPerBaselinePerChannel, rowIdx, baselineIdx++);

  }
  // last time
  doPreProcessingTimePoint(doFlagTime_p, currentTime, flagPerBaselinePerChannel);
}

/**
 * Checks channel by channel. Count if there are any polarizations flagged per channel
 * once all the rows for a time point are scanned, doPreProcessingTimePoint() will
 * decide whether to flag all the integrations of the antenna of interest
 *
 * @param polChanRowFlags flag cube from a row, with [polarizations, channels, rows]
 * @param flagPerBaselinePerChannel
 * @param row row index for 
 * @param baselineIdx index for the table of baseline-channel flags. Counting from 0.
 */
void FlagAgentAntennaIntegrations::checkAnyPolarizationFlagged(const casacore::Cube<casacore::Bool> &polChanRowFlags, 
							       TableFlagPerBaselinePerChannel &flagPerBaselinePerChannel,
							       casacore::uInt row, casacore::uInt baselineIdx) {

  const casacore::uInt channelCnt = polChanRowFlags.ncolumn();
  const casacore::uInt polCnt = polChanRowFlags.nrow();
  for (casacore::uInt chanIdx = 0; chanIdx < channelCnt; ++chanIdx) {
    for (casacore::uInt polIdx = 0; polIdx < polCnt; ++polIdx) {
      // assume that all polarization products must be unflagged for a baseline to be
      // deemed unflagged
      //if (polChanRowFlags.at(polIdx, chanIdx, row)) {
      if (polChanRowFlags(polIdx, chanIdx, row)) {
	  flagPerBaselinePerChannel[baselineIdx][chanIdx] = true;
	  break;
      }
    }
  }
  
  *logger_p << casacore::LogIO::DEBUG1 << "checkAnyPolarizationFlagged: finished with FLAG ON! baselineIdx: "  << baselineIdx << casacore::LogIO::POST;
}

/**
 * Once all the rows for a point in time have been processed, we know
 * what baselines to the antenna of interest are flagged. If all these
 * baselines are flagged, note down that all integrations for this
 * point in time should be flagged.
 *
 * @param flaggedTimes Structure to record what times must be flagged
 * @param rowTime value of the TIME column
 * @param flagPerBaselinePerChannel table with flag values of every baseline for the
 *        different channels
 */
void
FlagAgentAntennaIntegrations::doPreProcessingTimePoint(FlaggedTimesMap &flaggedTimes, 
						       casacore::Double rowTime,
						       TableFlagPerBaselinePerChannel &flagPerBaselinePerChannel)
{

  const casacore::uInt channelCnt = flagPerBaselinePerChannel.front().size();
  if (1 == channelCnt) {
    doPreProcessingTimePointSingleChannel(flaggedTimes, rowTime, flagPerBaselinePerChannel);
  } else {
    doPreProcessingTimePointMultiChannel(flaggedTimes, rowTime, flagPerBaselinePerChannel);
  }

  // clear flag counting table
  for (auto row : flagPerBaselinePerChannel) {
    row.assign(row.size(), false);
  }
}

/**
 * Pre-process with multiple channels (requires calculating the
 * fraction of channels flagged).
 *
 * Same parameters as doPreProcessingTimePoint()
 */
void
FlagAgentAntennaIntegrations::doPreProcessingTimePointMultiChannel(FlaggedTimesMap &flaggedTimes,
								   casacore::Double rowTime,
								   const TableFlagPerBaselinePerChannel &flagPerBaselinePerChannel)
{

  const casacore::uInt baseCnt = flagPerBaselinePerChannel.size();
  const casacore::uInt channelCnt = flagPerBaselinePerChannel.front().size();
  float frac = .0;
  float totalFlaggedCount = 0;
  for (casacore::uInt baseIdx = 0; baseIdx < baseCnt; ++baseIdx) {
    casacore::uInt flaggedChannelsCnt = 0;
    for (casacore::uInt chanIdx = 0; chanIdx < channelCnt; ++chanIdx) {
      if (flagPerBaselinePerChannel[baseIdx][chanIdx]) {
	++flaggedChannelsCnt;
      }
    }
    totalFlaggedCount += flaggedChannelsCnt;

    // A baseline will be considered flagged if a fraction greater than a nominated fraction
    // of channels is flagged.
    frac = static_cast<float>(flaggedChannelsCnt) / channelCnt;
    if (frac <= minChanThreshold_p) {
      // found a baseline that is not to be flagged
      return;
    }
  }

  // all baselines are flagged for a fraction of channels greater than the threshold
  flaggedTimes[rowTime] = true;
  if (verbose_p) {
    casacore::MVTime time(rowTime/casacore::C::day);
    float fracAlreadyFlagged = static_cast<float>(totalFlaggedCount) / (baseCnt * channelCnt);
    *logger_p << casacore::LogIO::NORMAL << "Flagging time:"
	      << time.string(casacore::MVTime::YMD,7) 
	      << " (fraction of flagged channels found: " << fracAlreadyFlagged << ")"
	      << casacore::LogIO::POST;  
  }
}


/**
 * If the selected channel is flagged for all the baselines, mark this time for flagging.
 *
 * This method Is not exactly the same as the MultiChannel version. When minchanfrac=1
 * MultiChannel would never flag, as the comparison requires that the 'fraction of channels
 * flagged' be > minchanfrac.
 *
 * Same parameters as doPreProcessingTimePoint()
 */
void
FlagAgentAntennaIntegrations::doPreProcessingTimePointSingleChannel(FlaggedTimesMap &flaggedTimes, 
								    casacore::Double rowTime,
								    const TableFlagPerBaselinePerChannel &flagPerBaselinePerChannel)
{

  const casacore::uInt baseCnt = flagPerBaselinePerChannel.size();
  for (casacore::uInt baseIdx = 0; baseIdx < baseCnt; ++baseIdx) {
    // A baseline is flagged if the only channel selected is flagged
    // Found a baseline that is not flagged:
    if (!flagPerBaselinePerChannel[baseIdx].front())
      return;
  }

  // all baselines are flagged for a fraction of channels greater than the threshold
  flaggedTimes[rowTime] = true;
  if (verbose_p) {
    casacore::MVTime time(rowTime/casacore::C::day);
    *logger_p << casacore::LogIO::NORMAL << "Flagging time:"
	      << time.string(casacore::MVTime::YMD,7) << casacore::LogIO::POST;
  }
}

bool
FlagAgentAntennaIntegrations::computeRowFlags(const vi::VisBuffer2 &visBuffer, FlagMapper &flags, casacore::uInt row)
{
  bool flag = false;
  // As per preprocessBuffer(), all rows for this point in time have to be flagged
  const casacore::Double rowTime = visBuffer.time()[row];
  if (doFlagTime_p.cend() != doFlagTime_p.find(rowTime)) {
    flag = true;
  }

  return flag;
}


} //# NAMESPACE CASA - END
