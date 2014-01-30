/*
 * AveragingViFactory.h
 *
 *  Created on: Feb 25, 2013
 *      Author: jjacobs
 */

#ifndef MSVIS_AVERAGINGVIFACTORY_H_
#define MSVIS_AVERAGINGVIFACTORY_H_

#include <msvis/MSVis/VisibilityIterator2.h>

namespace casa {

class MeasurementSet2;

namespace vi {

// AveragingOptions
//
// A container for the options that can be specified when creating
// an AveragingTvi2 based VI.  The user can specify which of the
// data cubes (observed, model and corrected) are to be averaged.
// The source of the weights applied to each cube can also be specified.
// See enum Options below for the symbols to use; the symbols are usually
// combined using bitwise-and ("|") in the constructor.

class AveragingOptions {

public:

    enum Options {Nothing = 0,
                  AverageObserved = 1 << 0,  // Average the observed data
                  AverageModel = 1 << 1,     // Average the model data
                  AverageCorrected = 1 << 2, // Average the corrected data
                  ObservedUseNoWeights = 1 << 3, // Use no weights when averaging observed data
                  ModelUseWeights = 1 << 4, // Use normal weights when averaging model data
                  ModelUseCorrectedWeights = 1 << 5, // Use corrected weights when averaging model data
                  ModelUseNoWeights = 1 << 6, // Use no weights when averaging model data
                  CorrectedUseNoWeights = 1 << 7, // Use no weights when averaging corrected data
                  CorrectedUseWeights = 1 << 8, // Use normal weights when averaging corrected data
                  CorrectedUseCorrectedWeights = 1 << 9, // Use corrected weights when averaging corrected data
                  BaselineDependentAveraging = 1 << 10, // Do averaging with lengths dependent on baselines
                                                        // Requires specifying a max uvw distance parameter
                  MarksLast
    };

    AveragingOptions () : options_p (AverageObserved) {}
    AveragingOptions (Int options) : options_p ((Options) options) {}
    explicit AveragingOptions (Options o) : options_p (o) {}

    AveragingOptions operator& (const AveragingOptions & other) const
    {
        return AveragingOptions (other.options_p & options_p);
    }

    AveragingOptions operator| (const AveragingOptions & other) const
    {
        return AveragingOptions (other.options_p | options_p);
    }

    AveragingOptions & operator|= (const AveragingOptions & other)
    {
        * this = AveragingOptions (options_p | other.options_p);

        return * this;
    }

    AveragingOptions & operator|= (Options options)
    {
        * this = AveragingOptions (options_p | options);

        return * this;
    }

    AveragingOptions operator~ () const
    {
        return AveragingOptions (~ options_p);
    }

    Bool contains (Options o) const { return (o & options_p) != 0; }

    Int
    nSet (Int o) const
    {
        Int result = o & options_p;
        Int nSet = 0;

        for (Int mask = 1; mask < MarksLast; mask = mask << 1){

            nSet += (result & mask) != 0 ? 1 : 0;

        }

        return nSet;
    }

private:


    Options options_p;
};

class AveragingParameters {

public:

    AveragingParameters ();

    AveragingParameters (Double averagingInterval,
                         Double chunkInterval,
                         const SortColumns & sortColumns = SortColumns (),
                         const AveragingOptions & options = AveragingOptions (),
                         Double maxUvwDistance = 0,
                         WeightScaling * weightScalingForAveraging = 0);

    AveragingParameters (const AveragingParameters & other);

    AveragingParameters & operator= (const AveragingParameters & other);

    Double getAveragingInterval () const;
    Double getChunkInterval () const;
    Double getMaxUvwDistance () const;
    const AveragingOptions & getOptions() const;
    const SortColumns & getSortColumns () const;
    WeightScaling * getWeightScaling () const;

    void setAveragingInterval (Double);
    void setChunkInterval (Double);
    void setSortColumns (const SortColumns &);
    void setOptions (const AveragingOptions &);
    void setMaxUvwDistance (Double);
    void setWeightScaling (WeightScaling *);

private:

    void validate ();
    void validateOptions ();

    Double averagingInterval_p;
    AveragingOptions averagingOptions_p;
    Double chunkInterval_p;
    Double maxUvwDistance_p;
    SortColumns sortColumns_p;
    WeightScaling * weightScaling_p;

    VisBufferComponents2 allDataColumns () const;

};

// The AveragingVi2Factory is used to initialize a VisibilityIterator2 so that
// it performs time averaging.  The parameters are fairly straightforward:
//
// averagingInterval - the time period to average over
// chunkInterval - the time interval of an averaged chunk
// sortColumns - the usual set of sort columns.  The sort columns also determine
//               the boundaries of a chunk.  If a column is present in the sort
//               columns, then there will be no averaging across values with different
//               values of that column (e.g., scan_number, array_id, etc.).  This is
//               because a chunk is defined to be the set of records having the same
//               values for all of the sort columns except for time.  The time column
//               is used to select rows having the same sort-column values but within
//               a specified interval of time.

class AveragingVi2Factory : public ViFactory {

public:

    AveragingVi2Factory (const AveragingParameters & parameters,
                         MeasurementSet * ms);

    AveragingVi2Factory (const AveragingParameters & parameters,
                         MeasurementSet * ms1,
                         MeasurementSet * ms2,
                         ...);
    // use 0 to mark end of MSs

    AveragingVi2Factory (const AveragingParameters & parameters,
                         const Block<const MeasurementSet *> & mss);

    ~AveragingVi2Factory ();

protected:

    virtual ViImplementation2 * createVi (VisibilityIterator2 *) const;

private:

    void initialize (const AveragingParameters & parameters,
                     const Block<const MeasurementSet *> & mss);

    Block<const MeasurementSet *> mss_p;
    AveragingParameters parameters_p;
};

} // end namesapce vi
} // end namespace casa


#endif /* MSVIS_AVERAGINGVIFACTORY_H_ */
