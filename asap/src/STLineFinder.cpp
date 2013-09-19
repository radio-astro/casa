//#---------------------------------------------------------------------------
//# STLineFinder.cc: A class for automated spectral line search
//#--------------------------------------------------------------------------
//# Copyright (C) 2004
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id: STLineFinder.cpp 2774 2013-02-25 06:35:27Z WataruKawasaki $
//#---------------------------------------------------------------------------


// ASAP
#include "STLineFinder.h"
#include "STFitter.h"
#include "IndexedCompare.h"

// STL
#include <functional>
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace asap;
using namespace casa;
using namespace std;

namespace asap {

///////////////////////////////////////////////////////////////////////////////
//
// RunningBox -    a running box calculator. This class implements
//                 iterations over the specified spectrum and calculates
//                 running box filter statistics.
//

class RunningBox {
   // The input data to work with. Use reference symantics to avoid
   // an unnecessary copying
   const casa::Vector<casa::Float>  &spectrum; // a buffer for the spectrum
   const casa::Vector<casa::Bool>   &mask; // associated mask
   const std::pair<int,int>         &edge; // start and stop+1 channels
                                           // to work with

   // statistics for running box filtering
   casa::Float sumf;       // sum of fluxes
   casa::Float sumf2;     // sum of squares of fluxes
   casa::Float sumch;       // sum of channel numbers (for linear fit)
   casa::Float sumch2;     // sum of squares of channel numbers (for linear fit)
   casa::Float sumfch;     // sum of flux*(channel number) (for linear fit)

   int box_chan_cntr;     // actual number of channels in the box
   int max_box_nchan;     // maximum allowed number of channels in the box
                          // (calculated from boxsize and actual spectrum size)
   // cache for derivative statistics
   mutable casa::Bool need2recalculate; // if true, values of the statistics
                                       // below are invalid
   mutable casa::Float linmean;  // a value of the linear fit to the
                                 // points in the running box
   mutable casa::Float linvariance; // the same for variance
   int cur_channel;       // the number of the current channel
   int start_advance;     // number of channel from which the box can
                          // be moved (the middle of the box, if there is no
                          // masking)
public:
   // set up the object with the references to actual data
   // as well as the number of channels in the running box
   RunningBox(const casa::Vector<casa::Float>  &in_spectrum,
                 const casa::Vector<casa::Bool>   &in_mask,
                 const std::pair<int,int>         &in_edge,
                 int in_max_box_nchan) throw(AipsError);

   // access to the statistics
   const casa::Float& getLinMean() const throw(AipsError);
   const casa::Float& getLinVariance() const throw(AipsError);
   casa::Float aboveMean() const throw(AipsError);
   int getChannel() const throw();

   // actual number of channels in the box (max_box_nchan, if no channels
   // are masked)
   int getNumberOfBoxPoints() const throw();

   // next channel
   void next() throw(AipsError);

   // checking whether there are still elements
   casa::Bool haveMore() const throw();

   // go to start
   void rewind() throw(AipsError);

protected:
   // supplementary function to control running mean/median calculations.
   // It adds a specified channel to the running box and
   // removes (ch-maxboxnchan+1)'th channel from there
   // Channels, for which the mask is false or index is beyond the
   // allowed range, are ignored
   void advanceRunningBox(int ch) throw(casa::AipsError);

   // calculate derivative statistics. This function is const, because
   // it updates the cache only
   void updateDerivativeStatistics() const throw(AipsError);
};

//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// LFAboveThreshold   An algorithm for line detection using running box
//                    statistics.  Line is detected if it is above the
//                    specified threshold at the specified number of
//                    consequtive channels. Prefix LF stands for Line Finder
//
class LFAboveThreshold : protected LFLineListOperations {
   // temporary line edge channels and flag, which is True if the line
   // was detected in the previous channels.
   std::pair<int,int> cur_line;
   casa::Bool is_detected_before;
   int  min_nchan;                         // A minimum number of consequtive
                                           // channels, which should satisfy
                                           // the detection criterion, to be
                                           // a detection
   casa::Float threshold;                  // detection threshold - the
                                           // minimal signal to noise ratio
   std::list<pair<int,int> > &lines;       // list where detections are saved
                                           // (pair: start and stop+1 channel)
   RunningBox *running_box;                // running box filter
   casa::Vector<Int> signs;                // An array to store the signs of
                                           // the value - current mean
                                           // (used to search wings)
   casa::Int last_sign;                    // a sign (+1, -1 or 0) of the
                                           // last point of the detected line
                                           //
   bool itsUseMedian;                      // true if median statistics is used
                                           // to determine the noise level, otherwise
                                           // it is the mean of the lowest 80% of deviations
                                           // (default)
   int itsNoiseSampleSize;                 // sample size used to estimate the noise statistics
                                           // Negative value means the whole spectrum is used (default)
public:

   // set up the detection criterion
   LFAboveThreshold(std::list<pair<int,int> > &in_lines,
                    int in_min_nchan = 3,
                    casa::Float in_threshold = 5,
                    bool use_median = false,
                    int noise_sample_size = -1) throw();
   virtual ~LFAboveThreshold() throw();

   // replace the detection criterion
   void setCriterion(int in_min_nchan, casa::Float in_threshold) throw();

   // return the array with signs of the value-current mean
   // An element is +1 if value>mean, -1 if less, 0 if equal.
   // This array is updated each time the findLines method is called and
   // is used to search the line wings
   const casa::Vector<Int>& getSigns() const throw();

   // find spectral lines and add them into list
   // if statholder is not NULL, the accumulate function of it will be
   // called for each channel to save statistics
   //    spectrum, mask and edge - reference to the data
   //    max_box_nchan  - number of channels in the running box
   void findLines(const casa::Vector<casa::Float> &spectrum,
                  const casa::Vector<casa::Bool> &mask,
                  const std::pair<int,int> &edge,
                  int max_box_nchan) throw(casa::AipsError);

protected:

   // process a channel: update curline and is_detected before and
   // add a new line to the list, if necessary using processCurLine()
   // detect=true indicates that the current channel satisfies the criterion
   void processChannel(Bool detect, const casa::Vector<casa::Bool> &mask)
                                                  throw(casa::AipsError);

   // process the interval of channels stored in curline
   // if it satisfies the criterion, add this interval as a new line
   void processCurLine(const casa::Vector<casa::Bool> &mask)
                                                 throw(casa::AipsError);

   // get the sign of runningBox->aboveMean(). The RunningBox pointer
   // should be defined
   casa::Int getAboveMeanSign() const throw();
};

//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// LFNoiseEstimator  a helper class designed to estimate off-line variance
//                   using statistics depending on the distribution of 
//                   values (e.g. like a median)
//
//                   Two statistics are supported: median and an average of
//                   80% of smallest values. 
//

struct LFNoiseEstimator {
   // construct an object
   // size - maximum sample size. After a size number of elements is processed
   // any new samples would cause the algorithm to drop the oldest samples in the
   // buffer.
   explicit LFNoiseEstimator(size_t size);

   // add a new sample 
   // in - the new value
   void add(float in);

   // median of the distribution
   float median() const;

   // mean of lowest 80% of the samples
   float meanLowest80Percent() const;

   // return true if the buffer is full (i.e. statistics are representative)
   inline bool filledToCapacity() const { return itsBufferFull;}

protected:
   // update cache of sorted indices
   // (it is assumed that itsSampleNumber points to the newly
   // replaced element)
   void updateSortedCache() const;

   // build sorted cache from the scratch
   void buildSortedCache() const;

   // number of samples accumulated so far
   // (can be less than the buffer size)
   size_t numberOfSamples() const;

   // this helper method builds the cache if
   // necessary using one of the methods
   void fillCacheIfNecessary() const;

private:
   // buffer with samples (unsorted)
   std::vector<float> itsVariances;
   // current sample number (<=itsVariances.size())
   size_t itsSampleNumber;
   // true, if the buffer all values in the sample buffer are used
   bool itsBufferFull;
   // cached indices into vector of samples
   mutable std::vector<size_t> itsSortedIndices;
   // true if any of the statistics have been obtained at least
   // once. This flag allows to implement a more efficient way of
   // calculating statistics, if they are needed at once and not 
   // after each addition of a new element
   mutable bool itsStatisticsAccessed;
};

//
///////////////////////////////////////////////////////////////////////////////


} // namespace asap

///////////////////////////////////////////////////////////////////////////////
//
// LFNoiseEstimator  a helper class designed to estimate off-line variance
//                   using statistics depending on the distribution of 
//                   values (e.g. like a median)
//
//                   Two statistics are supported: median and an average of
//                   80% of smallest values. 
//

// construct an object
// size - maximum sample size. After a size number of elements is processed
// any new samples would cause the algorithm to drop the oldest samples in the
// buffer.
LFNoiseEstimator::LFNoiseEstimator(size_t size) : itsVariances(size),
     itsSampleNumber(0), itsBufferFull(false), itsSortedIndices(size),
     itsStatisticsAccessed(false)
{
   AlwaysAssert(size>0,AipsError);
} 


// add a new sample 
// in - the new value
void LFNoiseEstimator::add(float in)
{
   if (isnan(in)) {
       // normally it shouldn't happen
       return;
   }
   itsVariances[itsSampleNumber] = in;

   if (itsStatisticsAccessed) {
       // only do element by element addition if on-the-fly 
       // statistics are needed
       updateSortedCache();
   }

   // advance itsSampleNumber now
   ++itsSampleNumber;
   if (itsSampleNumber == itsVariances.size()) {
       itsSampleNumber = 0;
       itsBufferFull = true;
   } 
   AlwaysAssert(itsSampleNumber<itsVariances.size(),AipsError);
}

// number of samples accumulated so far
// (can be less than the buffer size)
size_t LFNoiseEstimator::numberOfSamples() const
{
  // the number of samples accumulated so far may be less than the
  // buffer size
  const size_t nSamples = itsBufferFull ? itsVariances.size(): itsSampleNumber;
  AlwaysAssert( (nSamples > 0) && (nSamples <= itsVariances.size()), AipsError);
  return nSamples;
}

// this helper method builds the cache if
// necessary using one of the methods
void LFNoiseEstimator::fillCacheIfNecessary() const
{
  if (!itsStatisticsAccessed) {
      if ((itsSampleNumber!=0) || itsBufferFull) {
          // build the whole cache efficiently 
          buildSortedCache();
      } else {
          updateSortedCache();
      }
      itsStatisticsAccessed = true;
  } // otherwise, it is updated in 'add' using on-the-fly method
}

// median of the distribution
float LFNoiseEstimator::median() const
{
  fillCacheIfNecessary();
  // the number of samples accumulated so far may be less than the
  // buffer size
  const size_t nSamples = numberOfSamples();
  const size_t medSample = nSamples / 2;
  AlwaysAssert(medSample < itsSortedIndices.size(), AipsError);
  return itsVariances[itsSortedIndices[medSample]];
}

// mean of lowest 80% of the samples
float LFNoiseEstimator::meanLowest80Percent() const
{
  fillCacheIfNecessary();
  // the number of samples accumulated so far may be less than the
  // buffer size
  const size_t nSamples = numberOfSamples();
  float result = 0;
  size_t numpt=size_t(0.8*nSamples);
  if (!numpt) {
      numpt=nSamples; // no much else left,
                     // although it is very inaccurate
  }
  AlwaysAssert( (numpt > 0) && (numpt<itsSortedIndices.size()), AipsError);
  for (size_t ch=0; ch<numpt; ++ch) {
       result += itsVariances[itsSortedIndices[ch]];
  }
  result /= float(numpt);
  return result;
}

// update cache of sorted indices
// (it is assumed that itsSampleNumber points to the newly
// replaced element)
void LFNoiseEstimator::updateSortedCache() const
{
  // the number of samples accumulated so far may be less than the
  // buffer size
  const size_t nSamples = numberOfSamples();

  if (itsBufferFull) {
      // first find the index of the element which is being replaced
      size_t index = nSamples;
      for (size_t i=0; i<nSamples; ++i) {
           AlwaysAssert(i < itsSortedIndices.size(), AipsError);
           if (itsSortedIndices[i] == itsSampleNumber) {
               index = i;
               break;
           }
      }
      AlwaysAssert( index < nSamples, AipsError);

      const vector<size_t>::iterator indStart = itsSortedIndices.begin();
      // merge this element with preceeding block first
      if (index != 0) {
          // merge indices on the basis of variances
          inplace_merge(indStart,indStart+index,indStart+index+1, 
                        indexedCompare<size_t>(itsVariances.begin()));
      }
      // merge with the following block
      if (index + 1 != nSamples) {
          // merge indices on the basis of variances
          inplace_merge(indStart,indStart+index+1,indStart+nSamples, 
                        indexedCompare<size_t>(itsVariances.begin()));
      }
  } else {
      // itsSampleNumber is the index of the new element
      AlwaysAssert(itsSampleNumber < itsSortedIndices.size(), AipsError);
      itsSortedIndices[itsSampleNumber] = itsSampleNumber;
      if (itsSampleNumber >= 1) {
          // we have to place this new sample in
          const vector<size_t>::iterator indStart = itsSortedIndices.begin();
          // merge indices on the basis of variances
          inplace_merge(indStart,indStart+itsSampleNumber,indStart+itsSampleNumber+1, 
                        indexedCompare<size_t>(itsVariances.begin()));
      }
  }
}

// build sorted cache from the scratch
void LFNoiseEstimator::buildSortedCache() const
{
  // the number of samples accumulated so far may be less than the
  // buffer size
  const size_t nSamples = numberOfSamples();
  AlwaysAssert(nSamples <= itsSortedIndices.size(), AipsError);
  for (size_t i=0; i<nSamples; ++i) {
       itsSortedIndices[i]=i;
  }

  // sort indices, but check the array of variances
  const vector<size_t>::iterator indStart = itsSortedIndices.begin();
  stable_sort(indStart,indStart+nSamples, indexedCompare<size_t>(itsVariances.begin()));
}

//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// RunningBox -    a running box calculator. This class implements
//                 interations over the specified spectrum and calculates
//                 running box filter statistics.
//

// set up the object with the references to actual data
// and the number of channels in the running box
RunningBox::RunningBox(const casa::Vector<casa::Float>  &in_spectrum,
                       const casa::Vector<casa::Bool>   &in_mask,
                       const std::pair<int,int>         &in_edge,
                       int in_max_box_nchan) throw(AipsError) :
        spectrum(in_spectrum), mask(in_mask), edge(in_edge),
        max_box_nchan(in_max_box_nchan)
{
  rewind();
}

void RunningBox::rewind() throw(AipsError) {
  // fill statistics for initial box
  box_chan_cntr=0; // no channels are currently in the box
  sumf=0.;           // initialize statistics
  sumf2=0.;
  sumch=0.;
  sumch2=0.;
  sumfch=0.;
  int initial_box_ch=edge.first;
  for (;initial_box_ch<edge.second && box_chan_cntr<max_box_nchan;
        ++initial_box_ch)
       advanceRunningBox(initial_box_ch);

  if (initial_box_ch==edge.second)
      throw AipsError("RunningBox::rewind - too much channels are masked");

  cur_channel=edge.first;
  start_advance=initial_box_ch-max_box_nchan/2;
}

// access to the statistics
const casa::Float& RunningBox::getLinMean() const throw(AipsError)
{
  DebugAssert(cur_channel<edge.second, AipsError);
  if (need2recalculate) updateDerivativeStatistics();
  return linmean;
}

const casa::Float& RunningBox::getLinVariance() const throw(AipsError)
{
  DebugAssert(cur_channel<edge.second, AipsError);
  if (need2recalculate) updateDerivativeStatistics();
  return linvariance;
}

casa::Float RunningBox::aboveMean() const throw(AipsError)
{
  DebugAssert(cur_channel<edge.second, AipsError);
  if (need2recalculate) updateDerivativeStatistics();
  return spectrum[cur_channel]-linmean;
}

int RunningBox::getChannel() const throw()
{
  return cur_channel;
}

// actual number of channels in the box (max_box_nchan, if no channels
// are masked)
int RunningBox::getNumberOfBoxPoints() const throw()
{
  return box_chan_cntr;
}

// supplementary function to control running mean/median calculations.
// It adds a specified channel to the running box and
// removes (ch-max_box_nchan+1)'th channel from there
// Channels, for which the mask is false or index is beyond the
// allowed range, are ignored
void RunningBox::advanceRunningBox(int ch) throw(AipsError)
{
  if (ch>=edge.first && ch<edge.second)
      if (mask[ch]) { // ch is a valid channel
          ++box_chan_cntr;
          sumf+=spectrum[ch];
          sumf2+=square(spectrum[ch]);
          sumch+=Float(ch);
          sumch2+=square(Float(ch));
          sumfch+=spectrum[ch]*Float(ch);
          need2recalculate=True;
      }
  int ch2remove=ch-max_box_nchan;
  if (ch2remove>=edge.first && ch2remove<edge.second)
      if (mask[ch2remove]) { // ch2remove is a valid channel
          --box_chan_cntr;
          sumf-=spectrum[ch2remove];
          sumf2-=square(spectrum[ch2remove]);
          sumch-=Float(ch2remove);
          sumch2-=square(Float(ch2remove));
          sumfch-=spectrum[ch2remove]*Float(ch2remove);
          need2recalculate=True;
      }
}

// next channel
void RunningBox::next() throw(AipsError)
{
   AlwaysAssert(cur_channel<edge.second,AipsError);
   ++cur_channel;
   if (cur_channel+max_box_nchan/2<edge.second && cur_channel>=start_advance)
       advanceRunningBox(cur_channel+max_box_nchan/2); // update statistics
}

// checking whether there are still elements
casa::Bool RunningBox::haveMore() const throw()
{
   return cur_channel<edge.second;
}

// calculate derivative statistics. This function is const, because
// it updates the cache only
void RunningBox::updateDerivativeStatistics() const throw(AipsError)
{
  AlwaysAssert(box_chan_cntr, AipsError);

  Float mean=sumf/Float(box_chan_cntr);

  // linear LSF formulae
  Float meanch=sumch/Float(box_chan_cntr);
  Float meanch2=sumch2/Float(box_chan_cntr);
  if (meanch==meanch2 || box_chan_cntr<3) {
      // vertical line in the spectrum, can't calculate linmean and linvariance
      linmean=0.;
      linvariance=0.;
  } else {
      Float coeff=(sumfch/Float(box_chan_cntr)-meanch*mean)/
                (meanch2-square(meanch));
      linmean=coeff*(Float(cur_channel)-meanch)+mean;
      linvariance=sumf2/Float(box_chan_cntr)-square(mean)-
                    square(coeff)*(meanch2-square(meanch));
      if (linvariance<0.) {
          // this shouldn't happen normally, but could be due to round-off error
          linvariance = 0; 
      } else {
          linvariance = sqrt(linvariance);
      }
  }
  need2recalculate=False;
}


//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// LFAboveThreshold - a running mean/median algorithm for line detection
//
//


// set up the detection criterion
LFAboveThreshold::LFAboveThreshold(std::list<pair<int,int> > &in_lines,
                                   int in_min_nchan,
                                   casa::Float in_threshold,
                                   bool use_median,
                                   int noise_sample_size) throw() :
             min_nchan(in_min_nchan), threshold(in_threshold),
             lines(in_lines), running_box(NULL), itsUseMedian(use_median),
             itsNoiseSampleSize(noise_sample_size) {}

LFAboveThreshold::~LFAboveThreshold() throw()
{
  if (running_box!=NULL) delete running_box;
}

// replace the detection criterion
void LFAboveThreshold::setCriterion(int in_min_nchan, casa::Float in_threshold)
                                 throw()
{
  min_nchan=in_min_nchan;
  threshold=in_threshold;
}

// get the sign of runningBox->aboveMean(). The RunningBox pointer
// should be defined
casa::Int LFAboveThreshold::getAboveMeanSign() const throw()
{
  const Float buf=running_box->aboveMean();
  if (buf>0) return 1;
  if (buf<0) return -1;
  return 0;
}


// process a channel: update cur_line and is_detected before and
// add a new line to the list, if necessary
void LFAboveThreshold::processChannel(Bool detect,
                 const casa::Vector<casa::Bool> &mask) throw(casa::AipsError)
{
  try {
       if (is_detected_before) {
          // we have to check that the current detection has the
          // same sign of running_box->aboveMean
          // otherwise it could be a spurious detection
          if (last_sign && last_sign!=getAboveMeanSign())
              detect=False;
       }
       if (detect) {
           last_sign=getAboveMeanSign();
           if (is_detected_before)
               cur_line.second=running_box->getChannel()+1;
           else {
               is_detected_before=True;
               cur_line.first=running_box->getChannel();
               cur_line.second=running_box->getChannel()+1;
           }
       } else processCurLine(mask);
  }
  catch (const AipsError &ae) {
      throw;
  }
  catch (const exception &ex) {
      throw AipsError(String("LFAboveThreshold::processChannel - STL error: ")+ex.what());
  }
}

// process the interval of channels stored in cur_line
// if it satisfies the criterion, add this interval as a new line
void LFAboveThreshold::processCurLine(const casa::Vector<casa::Bool> &mask)
                                   throw(casa::AipsError)
{
  try {
       if (is_detected_before) {
           if (cur_line.second-cur_line.first>=min_nchan) {
               // it was a detection. We need to change the list
               Bool add_new_line=False;
               if (lines.size()) {
                   for (int i=lines.back().second;i<cur_line.first;++i)
                        if (mask[i]) { // one valid channel in between
                                //  means that we deal with a separate line
                            add_new_line=True;
                            break;
                        }
               } else add_new_line=True;
               if (add_new_line)
                   lines.push_back(cur_line);
               else lines.back().second=cur_line.second;
           }
           is_detected_before=False;
       }
  }
  catch (const AipsError &ae) {
      throw;
  }
  catch (const exception &ex) {
      throw AipsError(String("LFAboveThreshold::processCurLine - STL error: ")+ex.what());
  }
}

// return the array with signs of the value-current mean
// An element is +1 if value>mean, -1 if less, 0 if equal.
// This array is updated each time the findLines method is called and
// is used to search the line wings
const casa::Vector<Int>& LFAboveThreshold::getSigns() const throw()
{
  return signs;
}

// find spectral lines and add them into list
void LFAboveThreshold::findLines(const casa::Vector<casa::Float> &spectrum,
                              const casa::Vector<casa::Bool> &mask,
                              const std::pair<int,int> &edge,
                              int max_box_nchan)
                        throw(casa::AipsError)
{
  const int minboxnchan=4;
  try {

      if (running_box!=NULL) delete running_box;
      running_box=new RunningBox(spectrum,mask,edge,max_box_nchan);

      // determine the off-line variance first
      // an assumption made: lines occupy a small part of the spectrum

      const size_t noiseSampleSize = itsNoiseSampleSize<0 ? size_t(edge.second-edge.first) : 
                      std::min(size_t(itsNoiseSampleSize), size_t(edge.second-edge.first));
      DebugAssert(noiseSampleSize,AipsError);
      const bool globalNoise = (size_t(edge.second - edge.first) == noiseSampleSize);
      LFNoiseEstimator ne(noiseSampleSize);

      for (;running_box->haveMore();running_box->next()) {
	   ne.add(running_box->getLinVariance());
           if (ne.filledToCapacity()) {
               break;
           }
      }

      Float offline_variance = -1; // just a flag that it is unset
            
      if (globalNoise) {
	  offline_variance = itsUseMedian ? ne.median() : ne.meanLowest80Percent();
      }

      // actual search algorithm
      is_detected_before=False;

      // initiate the signs array
      signs.resize(spectrum.nelements());
      signs=Vector<Int>(spectrum.nelements(),0);

      //ofstream os("dbg.dat");
      for (running_box->rewind();running_box->haveMore();
                                 running_box->next()) {
           const int ch=running_box->getChannel();
           if (!globalNoise) {
               // add a next point for a local noise estimate
	       ne.add(running_box->getLinVariance());
           }    
           if (running_box->getNumberOfBoxPoints()>=minboxnchan) {
               if (!globalNoise) {
	           offline_variance = itsUseMedian ? ne.median() : ne.meanLowest80Percent();
               }
               AlwaysAssert(offline_variance>0.,AipsError);
               processChannel(mask[ch] && (fabs(running_box->aboveMean()) >=
                  threshold*offline_variance), mask);
           } else processCurLine(mask); // just finish what was accumulated before

           signs[ch]=getAboveMeanSign();
            //os<<ch<<" "<<spectrum[ch]<<" "<<fabs(running_box->aboveMean())<<" "<<
            //threshold*offline_variance<<endl;
      }
      if (lines.size())
          searchForWings(lines,signs,mask,edge);
  }
  catch (const AipsError &ae) {
      throw;
  }
  catch (const exception &ex) {
      throw AipsError(String("LFAboveThreshold::findLines - STL error: ")+ex.what());
  }
}

//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// LFLineListOperations::IntersectsWith  -  An auxiliary object function
//                to test whether two lines have a non-void intersection
//


// line1 - range of the first line: start channel and stop+1
LFLineListOperations::IntersectsWith::IntersectsWith(const std::pair<int,int> &in_line1) :
                          line1(in_line1) {}


// return true if line2 intersects with line1 with at least one
// common channel, and false otherwise
// line2 - range of the second line: start channel and stop+1
bool LFLineListOperations::IntersectsWith::operator()(const std::pair<int,int> &line2)
                          const throw()
{
  if (line2.second<line1.first) return false; // line2 is at lower channels
  if (line2.first>line1.second) return false; // line2 is at upper channels
  return true; // line2 has an intersection or is adjacent to line1
}

//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// LFLineListOperations::BuildUnion - An auxiliary object function to build a union
// of several lines to account for a possibility of merging the nearby lines
//

// set an initial line (can be a first line in the sequence)
LFLineListOperations::BuildUnion::BuildUnion(const std::pair<int,int> &line1) :
                             temp_line(line1) {}

// update temp_line with a union of temp_line and new_line
// provided there is no gap between the lines
void LFLineListOperations::BuildUnion::operator()(const std::pair<int,int> &new_line)
                                   throw()
{
  if (new_line.first<temp_line.first) temp_line.first=new_line.first;
  if (new_line.second>temp_line.second) temp_line.second=new_line.second;
}

// return the result (temp_line)
const std::pair<int,int>& LFLineListOperations::BuildUnion::result() const throw()
{
  return temp_line;
}

//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// LFLineListOperations::LaterThan - An auxiliary object function to test whether a
// specified line is at lower spectral channels (to preserve the order in
// the line list)
//

// setup the line to compare with
LFLineListOperations::LaterThan::LaterThan(const std::pair<int,int> &in_line1) :
                         line1(in_line1) {}

// return true if line2 should be placed later than line1
// in the ordered list (so, it is at greater channel numbers)
bool LFLineListOperations::LaterThan::operator()(const std::pair<int,int> &line2)
                          const throw()
{
  if (line2.second<line1.first) return false; // line2 is at lower channels
  if (line2.first>line1.second) return true; // line2 is at upper channels

  // line2 intersects with line1. We should have no such situation in
  // practice
  return line2.first>line1.first;
}

//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// STLineFinder  -  a class for automated spectral line search
//
//

STLineFinder::STLineFinder() throw() : edge(0,0), err("spurious")
{
  useScantable = true;
  setOptions();
}

// set the parameters controlling algorithm
// in_threshold a single channel threshold default is sqrt(3), which
//              means together with 3 minimum channels at least 3 sigma
//              detection criterion
//              For bad baseline shape, in_threshold may need to be
//              increased
// in_min_nchan minimum number of channels above the threshold to report
//              a detection, default is 3
// in_avg_limit perform the averaging of no more than in_avg_limit
//              adjacent channels to search for broad lines
//              Default is 8, but for a bad baseline shape this
//              parameter should be decreased (may be even down to a
//              minimum of 1 to disable this option) to avoid
//              confusing of baseline undulations with a real line.
//              Setting a very large value doesn't usually provide
//              valid detections.
// in_box_size  the box size for running mean/median calculation. Default is
//              1./5. of the whole spectrum size
// in_noise_box the box size for off-line noise estimation (if working with
//              local noise. Negative value means use global noise estimate
//              Default is -1 (i.e. estimate using the whole spectrum)
// in_median    true if median statistics is used as opposed to average of
//              the lowest 80% of deviations (default)
void STLineFinder::setOptions(const casa::Float &in_threshold,
                              const casa::Int &in_min_nchan,
                              const casa::Int &in_avg_limit,
                              const casa::Float &in_box_size,
                              const casa::Float &in_noise_box,
                              const casa::Bool &in_median) throw()
{
  threshold=in_threshold;
  min_nchan=in_min_nchan;
  avg_limit=in_avg_limit;
  box_size=in_box_size;
  itsNoiseBox = in_noise_box;
  itsUseMedian = in_median;
}

STLineFinder::~STLineFinder() throw(AipsError) {}

// set scan to work with (in_scan parameter)
void STLineFinder::setScan(const ScantableWrapper &in_scan) throw(AipsError)
{
  scan=in_scan.getCP();
  AlwaysAssert(!scan.null(),AipsError);
  useScantable = true;
}

// set spectrum data to work with. this is a method to allow linefinder work 
// without setting scantable for the purpose of using linefinder inside some 
// method in scantable class. (Dec 22, 2010 by W.Kawasaki)
void STLineFinder::setData(const std::vector<float> &in_spectrum)
{
  //spectrum = Vector<Float>(in_spectrum);
  spectrum.assign( Vector<Float>(in_spectrum) );
  useScantable = false;
}

// search for spectral lines. Number of lines found is returned
// in_edge and in_mask control channel rejection for a given row
//   if in_edge has zero length, all channels chosen by mask will be used
//   if in_edge has one element only, it represents the number of
//      channels to drop from both sides of the spectrum
//   in_edge is introduced for convinience, although all functionality
//   can be achieved using a spectrum mask only
int STLineFinder::findLines(const std::vector<bool> &in_mask,
			    const std::vector<int> &in_edge,
			    const casa::uInt &whichRow) throw(casa::AipsError)
{
  if (useScantable && scan.null())
      throw AipsError("STLineFinder::findLines - a scan should be set first,"
                      " use set_scan");

  uInt nchan = useScantable ? scan->nchan(scan->getIF(whichRow)) : spectrum.nelements();
  // set up mask and edge rejection
  // no mask given...
  if (in_mask.size() == 0) {
    //mask = Vector<Bool>(nchan,True);
    mask.assign( Vector<Bool>(nchan,True) );
  } else {
    // use provided mask
    //mask=Vector<Bool>(in_mask);
    mask.assign( Vector<Bool>(in_mask) );
  }
  if (mask.nelements()!=nchan)
      throw AipsError("STLineFinder::findLines - in_scan and in_mask, or in_spectrum "
		      "and in_mask have different number of spectral channels.");

  // taking flagged channels into account
  if (useScantable) {
    vector<bool> flaggedChannels = scan->getMask(whichRow);
    if (flaggedChannels.size()) {
      // there is a mask set for this row
      if (flaggedChannels.size() != mask.nelements()) {
          throw AipsError("STLineFinder::findLines - internal inconsistency: number of "
			  "mask elements do not match the number of channels");
      }
      for (size_t ch = 0; ch<mask.nelements(); ++ch) {
           mask[ch] &= flaggedChannels[ch];
      }
    }
  }

  // number of elements in in_edge
  if (in_edge.size()>2)
      throw AipsError("STLineFinder::findLines - the length of the in_edge parameter"
                      "should not exceed 2");
      if (!in_edge.size()) {
           // all spectra, no rejection
           edge.first=0;
           edge.second=nchan;
      } else {
           edge.first=in_edge[0];
           if (edge.first<0)
               throw AipsError("STLineFinder::findLines - the in_edge parameter has a negative"
                                "number of channels to drop");
           if (edge.first>=int(nchan)) {
               throw AipsError("STLineFinder::findLines - all channels are rejected by the in_edge parameter");
	   }
           if (in_edge.size()==2) {
               edge.second=in_edge[1];
               if (edge.second<0)
                   throw AipsError("STLineFinder::findLines - the in_edge parameter has a negative"
                                   "number of channels to drop");
               edge.second=nchan-edge.second;
           } else edge.second=nchan-edge.first;
           if (edge.second<0 || (edge.first>=edge.second)) {
               throw AipsError("STLineFinder::findLines - all channels are rejected by the in_edge parameter");
	   }
       }

  //
  int max_box_nchan=int(nchan*box_size); // number of channels in running
                                                 // box
  if (max_box_nchan<2)
      throw AipsError("STLineFinder::findLines - box_size is too small");

  // number of elements in the sample for noise estimate
  const int noise_box = itsNoiseBox<0 ? -1 : int(nchan * itsNoiseBox); 

  if ((noise_box!= -1) and (noise_box<2))
      throw AipsError("STLineFinder::findLines - noise_box is supposed to be at least 2 elements");

  if (useScantable) {
    spectrum.resize();
    spectrum = Vector<Float>(scan->getSpectrum(whichRow));
  }

  lines.resize(0); // search from the scratch
  last_row_used=whichRow;
  Vector<Bool> temp_mask(mask);

  Bool first_pass=True;
  Int avg_factor=1; // this number of adjacent channels is averaged together
                    // the total number of the channels is not altered
                    // instead, min_nchan is also scaled
                    // it helps to search for broad lines
  Vector<Int> signs; // a buffer for signs of the value - mean quantity
                     // see LFAboveThreshold for details
                     // We need only signs resulted from last iteration
                     // because all previous values may be corrupted by the
                     // presence of spectral lines

  while (true) {
     // a buffer for new lines found at this iteration
     std::list<pair<int,int> > new_lines;

     try {
         // line find algorithm
         LFAboveThreshold lfalg(new_lines,avg_factor*min_nchan, threshold, itsUseMedian,noise_box);
         lfalg.findLines(spectrum,temp_mask,edge,max_box_nchan);
         signs.resize(lfalg.getSigns().nelements());
         signs=lfalg.getSigns();
         first_pass=False;
         if (!new_lines.size())
//               throw AipsError("spurious"); // nothing new - use the same
//                                            // code as for a real exception
              throw err; // nothing new - use the same
                         // code as for a real exception
     }
     catch(const AipsError &ae) {
         if (first_pass) throw;
         // nothing new - proceed to the next step of averaging, if any
         // (to search for broad lines)
         if (avg_factor>=avg_limit) break; // averaging up to avg_limit
                                          // adjacent channels,
                                          // stop after that
         avg_factor*=2; // twice as more averaging
         subtractBaseline(temp_mask,9);
         averageAdjacentChannels(temp_mask,avg_factor);
         continue;
     } 
     keepStrongestOnly(temp_mask,new_lines,max_box_nchan);
     // update the list (lines) merging intervals, if necessary
     addNewSearchResult(new_lines,lines);
     // get a new mask
     temp_mask=getMask();
  }

  // an additional search for wings because in the presence of very strong
  // lines temporary mean used at each iteration will be higher than
  // the true mean

  if (lines.size())
      LFLineListOperations::searchForWings(lines,signs,mask,edge);

  return int(lines.size());
}

// auxiliary function to fit and subtract a polynomial from the current
// spectrum. It uses the Fitter class. This action is required before
// reducing the spectral resolution if the baseline shape is bad
void STLineFinder::subtractBaseline(const casa::Vector<casa::Bool> &temp_mask,
                      const casa::Int &order) throw(casa::AipsError)
{
  AlwaysAssert(spectrum.nelements(),AipsError);
  // use the fact that temp_mask excludes channels rejected at the edge
  Fitter sdf;
  std::vector<float> absc(spectrum.nelements());
  for (unsigned int i=0;i<absc.size();++i)
       absc[i]=float(i)/float(spectrum.nelements());
  std::vector<float> spec;
  spectrum.tovector(spec);
  std::vector<bool> std_mask;
  temp_mask.tovector(std_mask);
  sdf.setData(absc,spec,std_mask);
  sdf.setExpression("poly",order);
  if (!sdf.lfit()) return; // fit failed, use old spectrum
  spectrum=casa::Vector<casa::Float>(sdf.getResidual());
}

// auxiliary function to average adjacent channels and update the mask
// if at least one channel involved in summation is masked, all
// output channels will be masked. This function works with the
// spectrum and edge fields of this class, but updates the mask
// array specified, rather than the field of this class
// boxsize - a number of adjacent channels to average
void STLineFinder::averageAdjacentChannels(casa::Vector<casa::Bool> &mask2update,
                                   const casa::Int &boxsize)
                            throw(casa::AipsError)
{
  DebugAssert(mask2update.nelements()==spectrum.nelements(), AipsError);
  DebugAssert(boxsize!=0,AipsError);

  for (int n=edge.first;n<edge.second;n+=boxsize) {
       DebugAssert(n<spectrum.nelements(),AipsError);
       int nboxch=0; // number of channels currently in the box
       Float mean=0; // buffer for mean calculations
       for (int k=n;k<n+boxsize && k<edge.second;++k)
            if (mask2update[k]) {  // k is a valid channel
                mean+=spectrum[k];
                ++nboxch;
            }
       if (nboxch<boxsize) // mask these channels
           for (int k=n;k<n+boxsize && k<edge.second;++k)
                mask2update[k]=False;
       else {
          mean/=Float(boxsize);
           for (int k=n;k<n+boxsize && k<edge.second;++k)
                spectrum[k]=mean;
       }
  }
}


// get the mask to mask out all lines that have been found (default)
// if invert=true, only channels belong to lines will be unmasked
// Note: all channels originally masked by the input mask (in_mask
//       in setScan) or dropped out by the edge parameter (in_edge
//       in setScan) are still excluded regardless on the invert option
std::vector<bool> STLineFinder::getMask(bool invert)
                                        const throw(casa::AipsError)
{
  try {
    if (useScantable) {
      if (scan.null())
	throw AipsError("STLineFinder::getMask - a scan should be set first,"
			" use set_scan followed by find_lines");
      DebugAssert(mask.nelements()==scan->getChannels(last_row_used), AipsError);
    }
    /*
    if (!lines.size())
       throw AipsError("STLineFinder::getMask - one have to search for "
                           "lines first, use find_lines");
    */
    std::vector<bool> res_mask(mask.nelements());
    // iterator through lines
    std::list<std::pair<int,int> >::const_iterator cli=lines.begin();
    for (int ch=0;ch<int(res_mask.size());++ch) {
      if (ch<edge.first || ch>=edge.second) res_mask[ch]=false;
      else if (!mask[ch]) res_mask[ch]=false;
      else {
	res_mask[ch]=!invert; // no line by default
        if (cli!=lines.end())
	  if (ch>=cli->first && ch<cli->second)
	    res_mask[ch]=invert; // this is a line
      }
      if (cli!=lines.end())
	if (ch>=cli->second)
	  ++cli; // next line in the list
    }
    return res_mask;
  }
  catch (const AipsError &ae) {
    throw;
  }
  catch (const exception &ex) {
    throw AipsError(String("STLineFinder::getMask - STL error: ")+ex.what());
  }
}

// get range for all lines found. The same units as used in the scan
// will be returned (e.g. velocity instead of channels).
std::vector<double> STLineFinder::getLineRanges()
                             const throw(casa::AipsError)
{
  std::vector<double> vel;
  if (useScantable) {
    // convert to required abscissa units
    vel = scan->getAbcissa(last_row_used);
  } else {
    for (uInt i = 0; i < spectrum.nelements(); ++i)
      vel.push_back((double)i);
  }
  std::vector<int> ranges=getLineRangesInChannels();
  std::vector<double> res(ranges.size());

  std::vector<int>::const_iterator cri=ranges.begin();
  std::vector<double>::iterator outi=res.begin();
  for (;cri!=ranges.end() && outi!=res.end();++cri,++outi)
       if (uInt(*cri)>=vel.size())
          throw AipsError("STLineFinder::getLineRanges - getAbcissa provided less channels than reqired");
       else *outi=vel[*cri];
  return res;
}

// The same as getLineRanges, but channels are always used to specify
// the range
std::vector<int> STLineFinder::getLineRangesInChannels()
                                   const throw(casa::AipsError)
{
  try {
    if (useScantable) {
      if (scan.null())
	throw AipsError("STLineFinder::getLineRangesInChannels - a scan should be set first,"
			" use set_scan followed by find_lines");
      DebugAssert(mask.nelements()==scan->getChannels(last_row_used), AipsError);
    }

    if (!lines.size())
      throw AipsError("STLineFinder::getLineRangesInChannels - one have to search for "
		      "lines first, use find_lines");

    std::vector<int> res(2*lines.size());
    // iterator through lines & result
    std::list<std::pair<int,int> >::const_iterator cli = lines.begin();
    std::vector<int>::iterator ri = res.begin();
    for (; cli != lines.end() && ri != res.end(); ++cli,++ri) {
      *ri = cli->first;
      if (++ri != res.end())
	*ri = cli->second - 1;
    }
    return res;
  } catch (const AipsError &ae) {
    throw;
  } catch (const exception &ex) {
    throw AipsError(String("STLineFinder::getLineRanges - STL error: ") + ex.what());
  }
}



// an auxiliary function to remove all lines from the list, except the
// strongest one (by absolute value). If the lines removed are real,
// they will be find again at the next iteration. This approach
// increases the number of iterations required, but is able to remove
// spurious detections likely to occur near strong lines.
// Later a better criterion may be implemented, e.g.
// taking into consideration the brightness of different lines. Now
// use the simplest solution
// temp_mask - mask to work with (may be different from original mask as
// the lines previously found may be masked)
// lines2update - a list of lines to work with
//                 nothing will be done if it is empty
// max_box_nchan - channels in the running box for baseline filtering
void STLineFinder::keepStrongestOnly(const casa::Vector<casa::Bool> &temp_mask,
                  std::list<std::pair<int, int> > &lines2update,
                  int max_box_nchan)
                                   throw (casa::AipsError)
{
  try {
      if (!lines2update.size()) return; // ignore an empty list

      // current line
      std::list<std::pair<int,int> >::iterator li=lines2update.begin();
      // strongest line
      std::list<std::pair<int,int> >::iterator strongli=lines2update.begin();
      // the flux (absolute value) of the strongest line
      Float peak_flux=-1; // negative value - a flag showing uninitialized
                          // value
      // the algorithm below relies on the list being ordered
      Float tmp_flux=-1; // a temporary peak
      for (RunningBox running_box(spectrum,temp_mask,edge,max_box_nchan);
           running_box.haveMore(); running_box.next()) {

           if (li==lines2update.end()) break; // no more lines
           const int ch=running_box.getChannel();
           if (ch>=li->first && ch<li->second)
               if (temp_mask[ch] && tmp_flux<fabs(running_box.aboveMean()))
                   tmp_flux=fabs(running_box.aboveMean());
           if (ch==li->second-1) {
               if (peak_flux<tmp_flux) { // if peak_flux=-1, this condition
                   peak_flux=tmp_flux;   // will be satisfied
                   strongli=li;
               }
               ++li;
               tmp_flux=-1;
           }
      }
      std::list<std::pair<int,int> > res;
      res.splice(res.end(),lines2update,strongli);
      lines2update.clear();
      lines2update.splice(lines2update.end(),res);
  }
  catch (const AipsError &ae) {
      throw;
  }
  catch (const exception &ex) {
      throw AipsError(String("STLineFinder::keepStrongestOnly - STL error: ")+ex.what());
  }

}

//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// LFLineListOperations - a class incapsulating  operations with line lists
//                        The LF prefix stands for Line Finder
//

// concatenate two lists preserving the order. If two lines appear to
// be adjacent, they are joined into the new one
void LFLineListOperations::addNewSearchResult(const std::list<pair<int, int> > &newlines,
                         std::list<std::pair<int, int> > &lines_list)
                        throw(AipsError)
{
  try {
      for (std::list<pair<int,int> >::const_iterator cli=newlines.begin();
           cli!=newlines.end();++cli) {

           // the first item, which has a non-void intersection or touches
           // the new line
           std::list<pair<int,int> >::iterator pos_beg=find_if(lines_list.begin(),
                          lines_list.end(), IntersectsWith(*cli));
           // the last such item
           std::list<pair<int,int> >::iterator pos_end=find_if(pos_beg,
                          lines_list.end(), not1(IntersectsWith(*cli)));

           // extract all lines which intersect or touch a new one into
           // a temporary buffer. This may invalidate the iterators
           // line_buffer may be empty, if no lines intersects with a new
           // one.
           std::list<pair<int,int> > lines_buffer;
           lines_buffer.splice(lines_buffer.end(),lines_list, pos_beg, pos_end);

           // build a union of all intersecting lines
           pair<int,int> union_line=for_each(lines_buffer.begin(),
                   lines_buffer.end(),BuildUnion(*cli)).result();

           // search for a right place for the new line (union_line) and add
           std::list<pair<int,int> >::iterator pos2insert=find_if(lines_list.begin(),
                          lines_list.end(), LaterThan(union_line));
           lines_list.insert(pos2insert,union_line);
      }
  }
  catch (const AipsError &ae) {
      throw;
  }
  catch (const exception &ex) {
      throw AipsError(String("LFLineListOperations::addNewSearchResult - STL error: ")+ex.what());
  }
}

// extend all line ranges to the point where a value stored in the
// specified vector changes (e.g. value-mean change its sign)
// This operation is necessary to include line wings, which are below
// the detection threshold. If lines becomes adjacent, they are
// merged together. Any masked channel stops the extension
void LFLineListOperations::searchForWings(std::list<std::pair<int, int> > &newlines,
           const casa::Vector<casa::Int> &signs,
           const casa::Vector<casa::Bool> &mask,
           const std::pair<int,int> &edge) throw(casa::AipsError)
{
  try {
      for (std::list<pair<int,int> >::iterator li=newlines.begin();
           li!=newlines.end();++li) {
           // update the left hand side
           for (int n=li->first-1;n>=edge.first;--n) {
                if (!mask[n]) break;
                if (signs[n]==signs[li->first] && signs[li->first])
                    li->first=n;
                else break;
           }
           // update the right hand side
           for (int n=li->second;n<edge.second;++n) {
                if (!mask[n]) break;
                if (signs[n]==signs[li->second-1] && signs[li->second-1])
                    li->second=n;
                else break;
           }
      }
      // need to search for possible mergers.
      std::list<std::pair<int, int> >  result_buffer;
      addNewSearchResult(newlines,result_buffer);
      newlines.clear();
      newlines.splice(newlines.end(),result_buffer);
  }
  catch (const AipsError &ae) {
      throw;
  }
  catch (const exception &ex) {
      throw AipsError(String("LFLineListOperations::extendLines - STL error: ")+ex.what());
  }
}

//
///////////////////////////////////////////////////////////////////////////////
