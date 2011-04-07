/*
 * VisibilityProcessing.h
 *
 *  Created on: Feb 8, 2011
 *      Author: jjacobs
 */

#ifndef VISIBILITYPROCESSING_H_
#define VISIBILITYPROCESSING_H_

namespace casa {

namespace vpu {

class VisibilityProcessingUnit {

    friend class VisibilityAverager;

public:

    typedef enum {
        NoOutput,
        Modified,
        Unmodified
    } Result;

    typedef enum {
        Normal,
        Averager,
        Outputter,
        MultiOut
    } Type;

    VisibilityProcessor () : VisibilityProcessor (Normal) {}
    virtual ~VisibilityProcessor ();

    Result doProcessing (VisibilityBuffer & vb,
                         Bool isNewChunk,
                         Bool isNewSubChunk) = 0;
        // Return true if output produced

    virtual Bool chunkComplete () { return False;}
    virtual void chunkStart ();
    virtual void processingComplete ();
    virtual void processingStart ();

private:

    VisibilityProcessorUnit (Type t);

    // Prevent copying of existing objects

    VisibilityProcessor (const VisibilityProcessor & other); // do not define
    VisibilityProcessor & (const VisibilityProcessor & other); // do not define

    Type type_p;
};

class VisibilityAverager : public VisibilityProcessingUnit {
public:

    VisibilityAverager () : VisibilityProcessor (Averager) {}
    virtual ~VisibilityAverager ();

    void doProcessing (VisibilityBuffer & vb, Bool isNewChunk, Bool isNewSubChunk) = 0;

};

class VpuContainer : public VisibilityProcessingUnit {

public:

    VpuContainer ();
    ~VpuContainer ();

    void add (VisibilityProcessingUnit * processor, bool doNotDestroy = False);
    void connect (VisibilityProcessingUnit * sourceProcessor,
                  VisibilityProcessingUnit * sinkProcessor);
    void connect (VisibilityProcessingUnit * sourceProcessor, Int sourceId,
                  VisibilityProcessingUnit * sinkProcessor);
    void connect (VisibilityProcessingUnit * sourceProcessor,
                  VisibilityProcessingUnit * sinkProcessor, Int sinkId);
    void connect (VisibilityProcessingUnit * sourceProcessor, Int sourceId,
                 VisibilityProcessingUnit * sinkProcessor, Int sinkId);

protected:

    typedef vector<VisibilityProcessingUnit *> Contents;
    typedef Contents::const_iterator const_iterator;
    typedef Contents::iterator iterator;

    iterator begin();
    const_iterator begin() const;
    Bool empty () const;
    iterator end();
    const_iterator end() const;
    size_t size() const;

private:

    Contents contents_p;
    Contents destroyableContents_p; // subset of contents_p

};

class VisibilityProcessing {

public:

    void doProcessing (VpuContainer * processors, ROVisibilityIterator * vi);
};

} // end namespace vpu

} // end namespace casa

#endif /* VISIBILITYPROCESSING_H_ */
