//#---------------------------------------------------------------------------
//# STLineFinder.h: A class for automated spectral line search
//#---------------------------------------------------------------------------
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
//# $Id: STLineFinder.h 2666 2012-10-15 04:52:38Z MalteMarquarding $
//#---------------------------------------------------------------------------
#ifndef STLINEFINDER_H
#define STLINEFINDER_H

// STL
#include <vector>
#include <list>
#include <utility>
#include <exception>

// AIPS++
#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/Arrays/Vector.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/CountedPtr.h>

// ASAP
#include "ScantableWrapper.h"
#include "Scantable.h"

namespace asap {

///////////////////////////////////////////////////////////////////////////////
//
// LFLineListOperations - a class incapsulating  operations with line lists
//                        The LF prefix stands for Line Finder
//

struct LFLineListOperations {
   // concatenate two lists preserving the order. If two lines appear to
   // be adjacent or have a non-void intersection, they are joined into
   // the new line
   static void addNewSearchResult(const std::list<std::pair<int, int> >
                  &newlines, std::list<std::pair<int, int> > &lines_list)
                           throw(casa::AipsError);

   // extend all line ranges to the point where a value stored in the
   // specified vector changes (e.g. value-mean change its sign)
   // This operation is necessary to include line wings, which are below
   // the detection threshold. If lines becomes adjacent, they are
   // merged together. Any masked channel stops the extension
   static void searchForWings(std::list<std::pair<int, int> > &newlines,
                       const casa::Vector<casa::Int> &signs,
		       const casa::Vector<casa::Bool> &mask,
		       const std::pair<int,int> &edge)
			   throw(casa::AipsError);
protected:

   // An auxiliary object function to test whether two lines have a non-void
   // intersection
   class IntersectsWith : public std::unary_function<pair<int,int>, bool> {
       std::pair<int,int> line1;           // range of the first line
                                           // start channel and stop+1
   public:
       explicit IntersectsWith(const std::pair<int,int> &in_line1);
	// return true if line2 intersects with line1 with at least one
	// common channel, and false otherwise
	bool operator()(const std::pair<int,int> &line2) const throw();
   };

   // An auxiliary object function to build a union of several lines
   // to account for a possibility of merging the nearby lines
   class BuildUnion {
       std::pair<int,int> temp_line;       // range of the first line
                                           // start channel and stop+1
   public:
        explicit BuildUnion(const std::pair<int,int> &line1);
        // update temp_line with a union of temp_line and new_line
	// provided there is no gap between the lines
	void operator()(const std::pair<int,int> &new_line) throw();
	// return the result (temp_line)
	const std::pair<int,int>& result() const throw();
   };

   // An auxiliary object function to test whether a specified line
   // is at lower spectral channels (to preserve the order in the line list)
   class LaterThan : public std::unary_function<pair<int,int>, bool> {
       std::pair<int,int> line1;           // range of the first line
                                           // start channel and stop+1
   public:
        explicit LaterThan(const std::pair<int,int> &in_line1);

	// return true if line2 should be placed later than line1
	// in the ordered list (so, it is at greater channel numbers)
	bool operator()(const std::pair<int,int> &line2) const throw();
   };


};

//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// STLineFinder  -  a class for automated spectral line search
//
//

struct STLineFinder : protected LFLineListOperations {
   STLineFinder() throw();
   virtual ~STLineFinder() throw(casa::AipsError);

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
   // in_box_size  the box size for running mean calculation. Default is
   //              1./5. of the whole spectrum size
   // in_noise_box the box size for off-line noise estimation (if working with
   //              local noise. Negative value means use global noise estimate
   //              Default is -1 (i.e. estimate using the whole spectrum)
   // in_median    true if median statistics is used as opposed to average of
   //              the lowest 80% of deviations (default)
   void setOptions(const casa::Float &in_threshold=sqrt(3.),
                   const casa::Int &in_min_nchan=3,
		   const casa::Int &in_avg_limit=8,
                   const casa::Float &in_box_size=0.2,
                   const casa::Float &in_noise_box=-1.,
                   const casa::Bool &in_median = casa::False) throw();

   void setDetailedOptions( const casa::Int &order=9 ) ;

   // set the scan to work with (in_scan parameter)
   void setScan(const ScantableWrapper &in_scan) throw(casa::AipsError);

   // set spectrum data to work with. this is a method to allow linefinder work 
   // without setting scantable for the purpose of using linefinder inside some 
   // method in scantable class. (Dec. 22, 2010 by W.Kawasaki)
   void setData(const std::vector<float> &in_spectrum);

   // search for spectral lines in a row specified by whichRow 
   // in_mask and in_edge parameters control channel rejection
   //  if in_edge has zero length, all channels chosen by mask will be used
   //   if in_edge has one element only, it represents the number of
   //      channels to drop from both sides of the spectrum
   //   in_edge is introduced for convinience, although all functionality
   //   can be achieved using a spectrum mask only
   // Number of lines found is returned
   int findLines(const std::vector<bool> &in_mask,
		 const std::vector<int> &in_edge = std::vector<int>(),
		 const casa::uInt &whichRow = 0) throw(casa::AipsError);

   // get the mask to mask out all lines that have been found (default)
   // if invert=true, only channels belong to lines will be unmasked
   // Note: all channels originally masked by the input mask (in_mask
   //       in setScan) or dropped out by the edge parameter (in_edge
   //       in setScan) are still excluded regardless on the invert option
   std::vector<bool> getMask(bool invert=false) const throw(casa::AipsError);

   // get range for all lines found. The same units as used in the scan
   // will be returned (e.g. velocity instead of channels).
   std::vector<double>   getLineRanges() const throw(casa::AipsError);
   // The same as getLineRanges, but channels are always used to specify
   // the range
   std::vector<int> getLineRangesInChannels() const throw(casa::AipsError);
protected:
   // auxiliary function to average adjacent channels and update the mask
   // if at least one channel involved in summation is masked, all
   // output channels will be masked. This function works with the
   // spectrum and edge fields of this class, but updates the mask
   // array specified, rather than the field of this class
   // boxsize - a number of adjacent channels to average
   void averageAdjacentChannels(casa::Vector<casa::Bool> &mask2update,
                               const casa::Int &boxsize)
                               throw(casa::AipsError);

   // auxiliary function to fit and subtract a polynomial from the current
   // spectrum. It uses the Fitter class. This action is required before
   // reducing the spectral resolution if the baseline shape is bad
   void subtractBaseline(const casa::Vector<casa::Bool> &temp_mask,
                         const casa::Int &order) throw(casa::AipsError);

   // an auxiliary function to remove all lines from the list, except the
   // strongest one (by absolute value). If the lines removed are real,
   // they will be find again at the next iteration. This approach
   // increases the number of iterations required, but is able to remove
   // the sidelobes likely to occur near strong lines.
   // Later a better criterion may be implemented, e.g.
   // taking into consideration the brightness of different lines. Now
   // use the simplest solution
   // temp_mask - mask to work with (may be different from original mask as
   // the lines previously found may be masked)
   // lines2update - a list of lines to work with
   //                 nothing will be done if it is empty
   // max_box_nchan - channels in the running box for baseline filtering
   void keepStrongestOnly(const casa::Vector<casa::Bool> &temp_mask,
			  std::list<std::pair<int, int> > &lines2update,
			  int max_box_nchan)
                                      throw (casa::AipsError);
private:
   casa::CountedConstPtr<Scantable> scan; // the scan to work with
   casa::Vector<casa::Bool> mask;          // associated mask
   std::pair<int,int> edge;                // start and stop+1 channels
                                           // to work with
   casa::Float threshold;                  // detection threshold - the
                                           // minimal signal to noise ratio
   casa::Double box_size;	           // size of the box for running
                                           // mean calculations, specified as
					   // a fraction of the whole spectrum
   int  min_nchan;                         // A minimum number of consequtive
                                           // channels, which should satisfy
					   // the detection criterion, to be
					   // a detection
   casa::Int   avg_limit;                  // perform the averaging of no
                                           // more than in_avg_limit
					   // adjacent channels to search
					   // for broad lines. see setOptions
   casa::uInt last_row_used;                // the Row number specified
                                           // during the last findLines call
   std::list<std::pair<int, int> > lines;  // container of start and stop+1
                                           // channels of the spectral lines
   // a buffer for the spectrum
   mutable casa::Vector<casa::Float>  spectrum;

   // the box size for off-line noise estimation (if working with
   // local noise. Negative value means use global noise estimate
   // Default is -1 (i.e. estimate using the whole spectrum)
   casa::Float itsNoiseBox;

   // true if median statistics is used as opposed to average of
   // the lowest 80% of deviations (default)
   casa::Bool itsUseMedian;

   // true if spectra and mask data should be provided from 
   // scantable (default = true)
   bool useScantable;

   // shared object for nominal throw
   casa::AipsError err ;
};

//
///////////////////////////////////////////////////////////////////////////////

} // namespace asap
#endif // #ifndef STLINEFINDER_H
