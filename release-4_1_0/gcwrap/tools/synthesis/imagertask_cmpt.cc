
/***
 * Framework independent implementation file for imagertask...
 *
 * Implement the imagertask component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <imagertask_cmpt.h>
#include <imager_cmpt.h>

#include <casa/Logging/LogIO.h>
#include <casa/Exceptions/Error.h>

using namespace std;
using namespace casa;

namespace casac {

imagertask::imagertask()
{

}

imagertask::~imagertask()
{

}

void
imagertask::clean(const std::string& vis, const std::string& alg, const int niter, const double gain, const double threshold, const std::vector<std::string>& residual, const std::vector<std::string>& image, const std::vector<std::string>& model, const std::vector<std::string>& mask, const std::string& mode, const std::vector<int>& nchan, const std::vector<int>& start, const std::vector<int>& width, const std::vector<int>& step, const std::vector<int>& imsize, const std::vector<int>& cell, const std::string& stokes, const std::vector<int>& fieldid, const std::vector<int>& spwid, const std::string& weighting, const std::string& rmode, const double robust)
{
    try {
        // create imager
        imager imgr;
	/*
	imgr.setdata(mode, nchan, start, step, Quantity(std::vector<double> (1,0), "km/s"),
		     Quantity(std::vector<double> (1,0), "km/s"), spwid, fieldid, "");
        imgr.setimage(imsize[0], imsize[1], Quantity(std::vector<double> (1,cell[0]), "arcsec"),
                      Quantity(std::vector<double> (1,cell[1]), "arcsec"), stokes, false, "",
                      Quantity(std::vector<double> (1,0), "arcsec"),
                      Quantity(std::vector<double> (1,0), "arcsec"), mode, nchan[0],
                      start[0], step[0], "1 km/s", "1 km/s", spwid, fieldid[0], 1,
                      Quantity(std::vector<double> (1,0),"m"));
	imgr.weight("uniform", rmode, Quantity(std::vector<double> (1,0.0), "Jy"), robust,
                    Quantity(std::vector<double> (1,0),"rad"), 0);
        imgr.clean(alg, niter, gain, Quantity(std::vector<double> (1,threshold), "Jy"), false,
                   model, std::vector<bool>(), "", mask, image, residual, false, 1, "", false);
	*/
    } catch (AipsError x) {
        LogIO log;
        log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
}

void
imagertask::feather(const std::string& vis, const std::string& featherimage, const std::string& highres, const std::string& lowres, const std::string& lowpsf)
{
    try {
        // create imager
        imager imgr;
	/*
        imgr.setvp(true, true, "", false, Quantity(std::vector<double> (1,360), "deg"), Quantity(std::vector<double>(1,180), "deg"), "");
        imgr.feather(featherimage, highres, lowres, lowpsf);
	*/
    } catch (AipsError x) {
        LogIO log;
        log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
}

void
imagertask::invert(const std::string& vis, const std::string& map, const std::string& beam, const std::string& mode, const std::vector<int>& nchan, const std::vector<int>& start, const std::vector<int>& width, const std::vector<int>& step, const std::vector<int>& imsize, const std::vector<int>& cell, const std::string& stokes, const std::vector<int>& fieldid, const std::vector<int>& spwid, const std::string& weighting, const std::string& rmode, const double robust)
{
    try {
        // create imager
        imager imgr;
	/*
        imgr.setdata(mode, nchan, start, step, Quantity(std::vector<double> (1,0), "km/s"),
                     Quantity(std::vector<double> (1,0), "km/s"), spwid, fieldid, "");
        imgr.setimage(imsize[0], imsize[1], Quantity(std::vector<double> (1,cell[0]), "arcsec"),
                      Quantity(std::vector<double> (1,cell[1]), "arcsec"), stokes, false, "",
                      Quantity(std::vector<double> (1,0), "arcsec"),
                      Quantity(std::vector<double> (1,0), "arcsec"), mode, nchan[0],
                      start[0], step[0], "1 km/s", "1 km/s", spwid, fieldid[0], 1,
                      Quantity(std::vector<double> (1,0),"m"));
        imgr.weight("uniform", rmode, Quantity(std::vector<double> (1,0.0), "Jy"), robust,
                    Quantity(std::vector<double> (1,0),"rad"), 0);
        imgr.makeimage("corrected", map, "", false);
        imgr.makeimage("psf", beam, "", false);
	*/
    } catch (AipsError x) {
        LogIO log;
        log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
}

void
imagertask::mosaic(const std::string& vis, const std::string& alg, const int niter, const double gain, const double threshold, const std::vector<std::string>& residual, const std::vector<std::string>& image, const std::vector<std::string>& model, const std::vector<std::string>& mask, const std::string& mode, const std::string& gridfn, const std::string& grid, const std::vector<int>& nchan, const std::vector<int>& start, const std::vector<int>& width, const std::vector<int>& step, const std::vector<int>& imsize, const std::vector<int>& cell, const std::string& stokes, const std::vector<int>& fieldid, const int reffieldid, const std::vector<int>& spwid, const std::string& weighting, const std::string& mosweight, const std::string& rmode, const double robust, const double minpb, const std::string& scaletype, const int stoplargenegatives, const double cyclefactor, const double cyclespeedup)
{
    try {
        // create imager
        imager imgr;
	/*
        imgr.setvp(true, true, "", false, Quantity(std::vector<double> (1,360), "deg"), Quantity(std::vector<double> (1,180), "deg"), "");
        imgr.setdata(mode, nchan, start, step, Quantity(std::vector<double> (1,0), "km/s"),
                     Quantity(std::vector<double> (1,0), "km/s"), spwid, fieldid, "");
        imgr.setimage(imsize[0], imsize[1], Quantity(std::vector<double> (1,cell[0]), "arcsec"),
                      Quantity(std::vector<double> (1,cell[1]), "arcsec"), stokes, false, "",
                      Quantity(std::vector<double> (1,0), "arcsec"),
                      Quantity(std::vector<double> (1,0), "arcsec"), mode, nchan[0],
                      start[0], step[0], "1 km/s", "1 km/s", spwid, reffieldid, 1,
                      Quantity(std::vector<double> (1,0),"m"));
        imgr.weight(weighting, rmode, Quantity(std::vector<double> (1,0.0), "Jy"), robust,
                    Quantity(std::vector<double> (1,0),"rad"), 0);
        imgr.setoptions(grid, 0, 16, gridfn, "", 1.0, true, 1);
        imgr.setmfcontrol(cyclefactor, cyclespeedup, stoplargenegatives,
                          0, minpb, scaletype, 0.0, std::vector<std::string>());
        imgr.clean(alg, niter, gain, Quantity(std::vector<double> (1,threshold), "Jy"), false, model,
                   std::vector<bool>(), "", mask, image, residual, false, 1, "", false);
	*/
    } catch (AipsError x) {
        LogIO log;
        log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
}

} // casac namespace

