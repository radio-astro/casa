//# ImageMetaData.cc: Meta information for Images
//# Copyright (C) 2009
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: ImageMetaData.cc 20356 2008-06-23 11:37:34Z gervandiepen $


#include <images/Images/ImageInputParameters.h>

namespace casa {

	ImageInputParameters::ImageInputParameters(const ImageInterface<Float>& image) :
		_metaData(ImageMetaData(image)), _spectralRanges(Vector<uInt>(0)) {}


    Bool ImageInputParameters::setSpectralRanges(String& errmsg, const String& specification) {
    	errmsg = "";
        if (! _metaData.hasSpectralAxis()) {
        	errmsg = "No spectral axis exists";
            return False;
        }
        // First split on commas
        Vector<String> parts = stringToVector(specification, ',');
        Vector<uInt> ranges(2*parts.size());
    	Regex regexuInt("^[0-9]+$");
    	Regex regexRange("^[0-9]+[ \n\t\r\v\f]*-[ \n\t\r\v\f]*[0-9]+$");
    	Regex regexLT("^<.*$");
    	Regex regexLTEq("^<=.*$");
    	Regex regexGT("^>.*$");
    	Regex regexGTEq("^>=.*$");
    	uInt nchan = _metaData.nChannels();
		ostringstream os;
        for (uInt i=0; i<parts.size(); i++) {
        	parts[i].trim();
        	uInt min, max;
        	min = max = 0;
        	if (parts[i].matches(regexuInt)) {
        		// just one channel
        		min = String::toInt(parts[i]);
				max = min;
        	}
        	else if(parts[i].matches(regexRange)) {
        		// a range of channels
        		Vector<String> values = stringToVector(parts[i], '-');
        		if (values.size() != 2) {
        			errmsg = "Incorrect specification for range " + parts[i];
        		}
        		values[0].trim();
        		values[1].trim();
        		for(uInt j=0; j < 2; j++) {
        			if (! values[j].matches(regexuInt)) {
        				errmsg = values[j] + " is not a non-negative integer in " + parts[i];
        			}
        		}
        		if (errmsg.empty()) {
        			min = String::toInt(values[0]);
        			max = String::toInt(values[1]);
        		}
        	}
        	else if (parts[i].matches(regexLT)) {
        		String maxs = parts[i].matches(regexLTEq) ? parts[i].substr(2) : parts[i].substr(1);
        		maxs.trim();
        		if (! maxs.matches(regexuInt)) {
        			errmsg = maxs + " is not a non-negative integer in " + parts[i];
        		}
        		min = 0;
        		max = String::toInt(maxs);
        		if (! parts[i].matches(regexLTEq)) {
        			if (max == 0) {
        				errmsg = "Max channel cannot be less than zero in " + parts[i];
        			}
        			else {
        				max--;
        			}
        		}
        	}
        	else if (parts[i].matches(regexGT)) {
           		String mins = parts[i].matches(regexGTEq) ? parts[i].substr(2) : parts[i].substr(1);
           		mins.trim();
            	if (! mins.matches(regexuInt)) {
            		errmsg = mins + " is not an integer in " + parts[i];
            	}
            	max = nchan - 1;
            	min = String::toInt(mins);
            	if(! parts[i].matches(regexGTEq)) {
            		min++;
            	}
            	if (min > nchan - 1) {
            		os << "Min channel cannot be greater than the (zero-based) number of channels ("
            			<< nchan - 1 << ") in the image";
            		errmsg = os.str();

            	}
        	}
        	if (! errmsg.empty()) {
        		return False;
        	}
        	if (min > max) {
        		os << "Min channel " << min << " cannot be greater than max channel "
        			<< max << " in " << parts[i];
        		errmsg = os.str();
        	}
        	else if (max > nchan) {
        		os << "Max channel " << max << " cannot be greater than the total number of channels ("
        			<< nchan << ") in " << parts[i];
        		errmsg = os.str();
        	}
        	if (! errmsg.empty()) {
        		return False;
        	}
        	ranges[2*i] = min;
        	ranges[2*i + 1] = max;
        }
        _spectralRanges = _consolidateAndOrderRanges(ranges);
		return True;
    }

    Vector<uInt> ImageInputParameters::getSpectralRanges() const {
    	return _spectralRanges;
    }

    Vector<uInt> ImageInputParameters::_consolidateAndOrderRanges(const Vector<uInt>& ranges) const {
    	uInt arrSize = ranges.size()/2;
    	uInt arrMin[arrSize];
    	uInt arrMax[arrSize];
    	for (uInt i=0; i<arrSize; i++) {
    		arrMin[i] = ranges[2*i];
    		arrMax[i] = ranges[2*i + 1];
    	}
    	Sort sort;
    	sort.sortKey (arrMin, TpUInt);
    	sort.sortKey (arrMax, TpUInt, 0, Sort::Descending);
    	Vector<uInt> inxvec;
    	Vector<uInt> consol(0);
    	sort.sort(inxvec, arrSize);
    	for (uInt i=0; i<arrSize; i++) {
    		uInt idx = inxvec(i);
    		if (i==0) {
    			consol.resize(2, True);
    			consol[0] = arrMin[idx];
    			consol[1] = arrMax[idx];
    		}
    		else if (arrMin[idx] > consol[consol.size()-1]) {
    			// non overlap of previous range
    			consol.resize(consol.size()+2, True);
    			consol[consol.size()-2] = arrMin[idx];
    			consol[consol.size()-1] = arrMax[idx];
    		}
    		else if (arrMin[idx] < consol[consol.size()-1] && arrMax[idx] > consol[consol.size()-1] ) {
    			// overlaps previous range, so extend
    			consol[consol.size()-1] = arrMax[idx];
    		}
    	}
    	return consol;
    }


} //# NAMESPACE CASA - END

