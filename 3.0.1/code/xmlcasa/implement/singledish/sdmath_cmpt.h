
/***
 * Framework independent header file for sdmath...
 *
 * Implement the sdmath component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _sdmath_cmpt__H__
#define _sdmath_cmpt__H__
#ifndef _sdmath_cpnt__H__
#define _sdmath_cpnt__H__

#include <vector>
//#include <impl/casac/casac.h>
#include <xmlcasa/record.h>
#include <xmlcasa/singledish/sdmath_forward.h>
//#include "impl/sdtable_cmpt.h"

// put includes here

namespace casac {

/**
 * sdmath component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class sdmath
{
  private:

	

  public:

    sdmath();
    virtual ~sdmath();

    ::casac::sdtable * b_operate(const ::casac::record& left, const ::casac::record& right, const std::string& op, const bool doTSys);

    ::casac::sdtable * quotient(const ::casac::record& onscan, const ::casac::record& offscan, const bool preserve);

    ::casac::sdtable * scale(const ::casac::record& intable, const double factor, const bool all);

    void scaleinsitu(const ::casac::record& intable, const double factor, const bool doAll, const bool doTSys);

    ::casac::sdtable * add(const ::casac::record& intable, const double offset, const bool all);

    void addinsitu(const ::casac::record& intable, const double offset, const bool doAll);

    ::casac::sdtable * smooth(const ::casac::record& intable, const std::string& kernel, const double width, const bool doAll);

    void smoothinsitu(const ::casac::record& intable, const std::string& kernel, const double width);

    ::casac::sdtable * convertflux(const ::casac::record& intable, const double dia, const double eta, const double JyPerK, const bool doAll);

    void convertfluxinsitu(const ::casac::record& intable, const double dia, const double eta, const double JyPerK, const bool doAll);

    ::casac::sdtable * gainel(const ::casac::record& intable, const std::vector<double>& coeffs, const std::string& fname, const std::string& method, const bool doAll);

    void gainelinsitu(const ::casac::record& intable, const std::vector<double>& coeffs, const std::string& fname, const std::string& method, const bool doAll);

    ::casac::sdtable * freqalign(const ::casac::record& intable, const std::string& reftime, const std::string& method, const bool perfreqid);

    void freqaligninsitu(const ::casac::record& intable, const std::string& reftime, const std::string& method, const bool perfreqid);

    ::casac::sdtable * opacity(const ::casac::record& intable, const double tau, const bool doAll);

    void opacityinsitu(const ::casac::record& intable, const double tau, const bool doAll);

    ::casac::sdtable * average(const ::casac::record& intable, const std::vector<bool>& mask, const bool scanAv, const std::string& wt);

    ::casac::sdtable * averagepol(const ::casac::record& intable, const std::vector<bool>& mask, const std::string& weightstr);

    void averagepolinsitu(const ::casac::record& intable, const std::vector<bool>& mask, const std::string& weightstr);

    ::casac::sdtable * bin(const ::casac::record& intable, const int width);

    void bininsitu(const ::casac::record& intable, const int width);

    ::casac::sdtable * resample(const ::casac::record& intable, const std::string& method, const double width);

    void resampleinsitu(const ::casac::record& intable, const std::string& method, const double width);

    std::vector<double> stats(const ::casac::record& intable, const std::vector<bool>& mask, const std::string& which, const int row);

    void rotatexyphase(const ::casac::record& intable, const double angle, const bool doAll);

    void rotatelinpolphase(const ::casac::record& intable, const double angle, const bool doAll);

    ::casac::sdtable * freqsw(const ::casac::record& intable);

    void insitufreqsw(const ::casac::record& intable);

    private:

#include <singledish/sdmath_private.h>

};

} // casac namespace
#endif
#endif

