
/***
 * Framework independent implementation file for sdmath...
 *
 * Implement the sdmath component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <sdmath_cmpt.h>

using namespace std;
using namespace casa;

namespace casac {

sdmath::sdmath()
{

}

sdmath::~sdmath()
{

}

::casac::sdtable *
sdmath::b_operate(const ::casac::record& left, const ::casac::record& right, const std::string& op, const bool doTSys)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::sdtable *
sdmath::quotient(const ::casac::record& onscan, const ::casac::record& offscan, const bool preserve)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::sdtable *
sdmath::scale(const ::casac::record& intable, const double factor, const bool all)
{

    // TODO : IMPLEMENT ME HERE !
}

void
sdmath::scaleinsitu(const ::casac::record& intable, const double factor, const bool doAll, const bool doTSys)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::sdtable *
sdmath::add(const ::casac::record& intable, const double offset, const bool all)
{

    // TODO : IMPLEMENT ME HERE !
}

void
sdmath::addinsitu(const ::casac::record& intable, const double offset, const bool doAll)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::sdtable *
sdmath::smooth(const ::casac::record& intable, const std::string& kernel, const double width, const bool doAll)
{

    // TODO : IMPLEMENT ME HERE !
}

void
sdmath::smoothinsitu(const ::casac::record& intable, const std::string& kernel, const double width)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::sdtable *
sdmath::convertflux(const ::casac::record& intable, const double dia, const double eta, const double JyPerK, const bool doAll)
{

    // TODO : IMPLEMENT ME HERE !
}

void
sdmath::convertfluxinsitu(const ::casac::record& intable, const double dia, const double eta, const double JyPerK, const bool doAll)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::sdtable *
sdmath::gainel(const ::casac::record& intable, const std::vector<double>& coeffs, const std::string& fname, const std::string& method, const bool doAll)
{

    // TODO : IMPLEMENT ME HERE !
}

void
sdmath::gainelinsitu(const ::casac::record& intable, const std::vector<double>& coeffs, const std::string& fname, const std::string& method, const bool doAll)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::sdtable *
sdmath::freqalign(const ::casac::record& intable, const std::string& reftime, const std::string& method, const bool perfreqid)
{

    // TODO : IMPLEMENT ME HERE !
}

void
sdmath::freqaligninsitu(const ::casac::record& intable, const std::string& reftime, const std::string& method, const bool perfreqid)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::sdtable *
sdmath::opacity(const ::casac::record& intable, const double tau, const bool doAll)
{

    // TODO : IMPLEMENT ME HERE !
}

void
sdmath::opacityinsitu(const ::casac::record& intable, const double tau, const bool doAll)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::sdtable *
sdmath::average(const ::casac::record& intable, const std::vector<bool>& mask, const bool scanAv, const std::string& wt)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::sdtable *
sdmath::averagepol(const ::casac::record& intable, const std::vector<bool>& mask, const std::string& weightstr)
{

    // TODO : IMPLEMENT ME HERE !
}

void
sdmath::averagepolinsitu(const ::casac::record& intable, const std::vector<bool>& mask, const std::string& weightstr)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::sdtable *
sdmath::bin(const ::casac::record& intable, const int width)
{

    // TODO : IMPLEMENT ME HERE !
}

void
sdmath::bininsitu(const ::casac::record& intable, const int width)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::sdtable *
sdmath::resample(const ::casac::record& intable, const std::string& method, const double width)
{

    // TODO : IMPLEMENT ME HERE !
}

void
sdmath::resampleinsitu(const ::casac::record& intable, const std::string& method, const double width)
{

    // TODO : IMPLEMENT ME HERE !
}

std::vector<double>
sdmath::stats(const ::casac::record& intable, const std::vector<bool>& mask, const std::string& which, const int row)
{

    // TODO : IMPLEMENT ME HERE !
}

void
sdmath::rotatexyphase(const ::casac::record& intable, const double angle, const bool doAll)
{

    // TODO : IMPLEMENT ME HERE !
}

void
sdmath::rotatelinpolphase(const ::casac::record& intable, const double angle, const bool doAll)
{

    // TODO : IMPLEMENT ME HERE !
}

::casac::sdtable *
sdmath::freqsw(const ::casac::record& intable)
{

    // TODO : IMPLEMENT ME HERE !
}

void
sdmath::insitufreqsw(const ::casac::record& intable)
{

    // TODO : IMPLEMENT ME HERE !
}

} // casac namespace

