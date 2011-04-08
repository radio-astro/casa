
/***
 * Framework independent implementation file for imagepol...
 *
 * Implement the imagepol component here.
 * 
 * // imagepol_cmpt.cc: defines imagepol class which implements functionity
 * // for image polarimetry.
 * @author
 * @version 
 ***/

#include <iostream>
#include <imagepol_cmpt.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageAnalysis.h>
#include <images/Images/ImageExpr.h>
#include <images/Images/ImagePolProxy.h>
#include <images/Images/TempImage.h>
#include <images/Images/PagedImage.h>
#include <casa/namespace.h>
using namespace std;

namespace casac {

imagepol::imagepol()
{
  itsImPol=0;
  itsLog=0;

  itsLog= new LogIO();
}

imagepol::~imagepol()
{

}

bool 
imagepol::open(const variant& image){
  bool rstat(false);
  try {
    *itsLog << LogOrigin("imagepol", "open");
    if(itsLog==0) {
      itsLog=new LogIO();
    }
    if(itsImPol) delete itsImPol;
    if(image.type()==variant::RECORD){
      variant localvar(image);
      Record *tmp = toRecord(localvar.asRecord());
      TempImage<Float> tmpim;
      String err;
      rstat=tmpim.fromRecord(err, *tmp);
      delete tmp;
      if(!rstat){
	*itsLog << LogIO::SEVERE << "Could not convert image record"
		<< LogIO::EXCEPTION;
	return rstat;
      }
      itsImPol= new ImagePol(tmpim);
      rstat = true;
    } else if(image.type()== variant::STRING){
      if(casa::Table::isReadable(toCasaString(image))){
	PagedImage<Float> tmpim(toCasaString(image));
	itsImPol= new ImagePol(tmpim);
	rstat=True;
      }
      else{
	*itsLog << LogIO::SEVERE << "image not found on disk"
		<< LogIO::EXCEPTION;
      }
    }

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
imagepol::imagepoltestimage(const std::string& outfile,
			    const std::vector<double>& rm,
			    const double pa0, const double sigma,
			    const int nx, const int ny, const int nf,
			    const double f0, const double bw) {

  bool rstat(false);
  bool rmdefault(false);

  try {
    *itsLog << LogOrigin("imagepol", "imagepoltestimage");
    if(itsImPol) delete itsImPol;
    itsImPol = new ImagePol();
    if(itsLog==0) itsLog=new LogIO();

    if (rm.size() == 1 and rm[0]==0.0) {
      rmdefault = true;
    }
    rstat = itsImPol->imagepoltestimage(outfile, rm, rmdefault, pa0, sigma,
    					nx, ny, nf, f0, bw);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "
	    << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool 
imagepol::close() {
  bool rstat(false);
  try {
    *itsLog << LogOrigin("imagepol", "close");

    if (itsImPol != 0) {
      delete itsImPol;
      itsImPol = 0;
    }
    rstat = true;

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
imagepol::complexlinpol(const std::string& outfile)
{
  bool rstat(false);
  try{
    *itsLog << LogOrigin("imagepol", "complexlinpol");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return rstat;
    }
    rstat = itsImPol->complexlinpol(String(outfile));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
imagepol::complexfraclinpol(const std::string& outfile)
{

  bool rstat(false);
  try{
    *itsLog << LogOrigin("imagepol", "complexfraclinpol");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return rstat;
    }
    itsImPol->complexFractionalLinearPolarization(String(outfile));
    rstat = true;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

image *
imagepol::depolratio(const std::string& infile, const bool debias, const double clip, const double sigma, const std::string& outfile)
{

  image *outim = new image();
  try{
    *itsLog << LogOrigin("imagepol", "depolratio");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return outim;
    }
    ImageInterface<Float> *out;
    Bool rstat(False);
    rstat = itsImPol->depolratio(out, infile, debias, clip, sigma, outfile);
    if(rstat && !outim->open(out))
      throw(AipsError("could not attach depolratio image"));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return outim;
}

bool
imagepol::done()
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("imagepol", "done");

    if (itsImPol != 0) {
      delete itsImPol;
      itsImPol = 0;
    }
    rstat = true;

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
imagepol::fourierrotationmeasure(const std::string& complex, const std::string& amp, const std::string& pa, const std::string& real, const std::string& imag, const bool zerolag0)
{
  bool rstat(false);
  try{
    *itsLog << LogOrigin("imagepol", "fourierrotationmeasure");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return rstat;
    }
    itsImPol->fourierRotationMeasure(String(complex), String(amp),
				     String(pa), String(real),
				     String(imag), zerolag0);
    rstat = true;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

image *
imagepol::fraclinpol(const bool debias, const double clip, const double sigma, const std::string& outfile)
{
  image *outim = new image();
  try{
    *itsLog << LogOrigin("imagepol", "fraclinpol");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return outim;
    }
    ImageInterface<Float> *out;
    Bool rstat(False);
    rstat = itsImPol->fracLinPol(out,debias,Float(clip),
				 Float(sigma),String(outfile));
    if(rstat && !outim->open(out))
      throw(AipsError("could not attach fraclinpol image"));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return outim;
}

image *
imagepol::fractotpol(const bool debias, const double clip, const double sigma, const std::string& outfile)
{
  image *outim = new image();
  try{
    *itsLog << LogOrigin("imagepol", "fractotpol");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return outim;
    }
    ImageInterface<Float> *out;
    Bool rstat(False);
    rstat = itsImPol->fracTotPol(out,debias,Float(clip),
				 Float(sigma),String(outfile));
    if(rstat && !outim->open(out))
      throw(AipsError("could not attach fractotpol image"));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return outim;
}

image *
imagepol::linpolint(const bool debias, const double clip, const double sigma, const std::string& outfile)
{
  image *outim = new image();
  try{
    *itsLog << LogOrigin("imagepol", "linpolint");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return outim;
    }
    ImageInterface<Float> *out;
    Bool rstat(False);
    rstat = itsImPol->linPolInt(out,debias,Float(clip),
				Float(sigma),String(outfile));
    if(rstat && !outim->open(out))
      throw(AipsError("could not attach linpolint image"));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return outim;
}

image *
imagepol::linpolposang(const std::string& outfile)
{
  image *outim = new image();
  try{
    *itsLog << LogOrigin("imagepol", "linpolposang");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return outim;
    }
    ImageInterface<Float> *out;
    Bool rstat(False);
    rstat = itsImPol->linPolPosAng(out,String(outfile));
    if(rstat && !outim->open(out))
      throw(AipsError("could not attach linpolposang image"));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return outim;
}

bool
imagepol::makecomplex(const std::string& complex, const std::string& real, const std::string& imag, const std::string& amp, const std::string& phase)
{
  bool rstat(false);
  try{
    *itsLog << LogOrigin("imagepol", "makecomplex");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return rstat;
    }
    itsImPol->makeComplex(String(complex), String(real),
			  String(imag), String(amp),
			  String(phase));
    rstat = true;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

image *
imagepol::pol(const std::string& which, const bool debias, const double clip, const double sigma, const std::string& outfile)
{
  image *outim = new image();
  try{
    *itsLog << LogOrigin("imagepol", "pol");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return outim;
    }
    ImageInterface<Float> *out;
    String type(which);
    type.upcase();
    Bool rstat(False);
    if (type=="LPI") {
      rstat = itsImPol->linPolInt(out,debias,Float(clip),
				  Float(sigma),String(outfile));
    } else if (type=="TPI") {
      rstat = itsImPol->totPolInt(out,debias,Float(clip),
				  Float(sigma),String(outfile));
    } else if (type=="LPPA") {
      rstat = itsImPol->linPolPosAng(out,String(outfile));
    } else if (type=="FLP") {
      rstat = itsImPol->fracLinPol(out,debias,Float(clip),
				   Float(sigma),String(outfile));
    } else if (type=="FTP") {
      rstat = itsImPol->fracTotPol(out,debias,Float(clip),
				   Float(sigma),String(outfile));
    } else {
      throw(AipsError("Code "+String(which)+" is unrecognized"));
    }
      
    if(rstat && !outim->open(out))
      throw(AipsError("could not attach pol image"));
    
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return outim;
}

bool
imagepol::rotationmeasure(const std::string& rm, const std::string& rmerr, const std::string& pa0, const std::string& pa0err, const std::string& nturns, const std::string& chisq, const double sigma, const double rmfq, const double rmmax, const double maxpaerr, const std::string& plotter, const int nx, const int ny)
{
  bool rstat(false);
  try{
    *itsLog << LogOrigin("imagepol", "rotationmeasure");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return rstat;
    }
    itsImPol->rotationMeasure(String(rm), String(rmerr),
			      String(pa0), String(pa0err),
			      String(nturns), String(chisq),
			      -1, Float(sigma), Float(rmfq),
			      Float(rmmax), Float(maxpaerr),
			      String(plotter), nx, ny);
    rstat = true;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

double
imagepol::sigma(const double clip)
{
  double rvalue(-1.0);
  try{
    *itsLog << LogOrigin("imagepol", "sigma");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return rvalue;
    }
    rvalue = itsImPol->sigma(Float(clip));
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rvalue;
}

image *
imagepol::sigmadepolratio(const std::string& infile, const bool debias, const double clip, const double sigma, const std::string& outfile)
{
  image *outim = new image();
  try{
    *itsLog << LogOrigin("imagepol", "sigmadepolratio");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return outim;
    }
    ImageInterface<Float> *out;
    Bool rstat(False);
    rstat = itsImPol->sigmaDepolarizationRatio(out,String(infile), debias,
					       Float(clip), Float(sigma),
					       String(outfile));
    if(rstat && !outim->open(out))
      throw(AipsError("could not attach sigmadepolratio image"));
    
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

  return outim;
}

image *
imagepol::sigmafraclinpol(const double clip, const double sigma, const std::string& outfile)
{
  image *outim = new image();
  try{
    *itsLog << LogOrigin("imagepol", "sigmafraclinpol");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return outim;
    }
    ImageInterface<Float> *out;
    Bool rstat(False);
    rstat = itsImPol->sigmaFracLinPol(out, Float(clip), Float(sigma),
				      String(outfile));
    if(rstat && !outim->open(out))
      throw(AipsError("could not attach sigmafraclinpol image"));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

  return outim;
}

image *
imagepol::sigmafractotpol(const double clip, const double sigma, const std::string& outfile)
{
  image *outim = new image();
  try{
    *itsLog << LogOrigin("imagepol", "sigmafractotpol");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return outim;
    }
    ImageInterface<Float> *out;
    Bool rstat(False);
    rstat = itsImPol->sigmaFracTotPol(out, Float(clip),
				      Float(sigma), String(outfile));
    if(rstat && !outim->open(out))
      throw(AipsError("could not attach sigmafractotpol image"));
    
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

  return outim;
}

double
imagepol::sigmalinpolint(const double clip, const double sigma, const std::string& outfile)
{
  double rvalue(-1.0);
  try{
    *itsLog << LogOrigin("imagepol", "sigmalinpolint");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return rvalue;
    }
    rvalue = itsImPol->sigma(Float(clip));
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rvalue;
}

image *
imagepol::sigmalinpolposang(const double clip, const double sigma, const std::string& outfile)
{
  image *outim = new image();
  try{
    *itsLog << LogOrigin("imagepol", "sigmalinpolposang");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return outim;
    }
    ImageInterface<Float> *out;
    Bool rstat(False);
    rstat = itsImPol->sigmaLinPolPosAng(out,Float(clip),
					Float(sigma),String(outfile));
    if(rstat && !outim->open(out))
      throw(AipsError("could not attach sigmalinpolposang image"));
    
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

  return outim;
}

double
imagepol::sigmastokes(const std::string& which, const double clip)
{
  double rvalue(-1.0);
  try{
    *itsLog << LogOrigin("imagepol", "sigmastokes");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return rvalue;
    }
    String type(which);
    type.upcase();
    if (type=="I") {
      rvalue = itsImPol->sigmaStokesI(Float(clip));
    } else if (type=="Q") {
      rvalue = itsImPol->sigmaStokesQ(Float(clip));
    } else if (type=="U") {
      rvalue = itsImPol->sigmaStokesU(Float(clip));
    } else if (type=="V") {
      rvalue = itsImPol->sigmaStokesV(Float(clip));
    } else {
      throw(AipsError("Stokes "+String(which)+" is unrecognized"));
    }
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rvalue;
}

double
imagepol::sigmastokesi(const double clip)
{
  double rvalue(-1.0);
  try{
    *itsLog << LogOrigin("imagepol", "sigmastokesi");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return rvalue;
    }
    rvalue = itsImPol->sigmaStokesI(Float(clip));
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rvalue;
}

double
imagepol::sigmastokesq(const double clip)
{
  double rvalue(-1.0);
  try{
    *itsLog << LogOrigin("imagepol", "sigmastokesq");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return rvalue;
    }
    rvalue = itsImPol->sigmaStokesQ(Float(clip));
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rvalue;
}

double
imagepol::sigmastokesu(const double clip)
{
  double rvalue(-1.0);
  try{
    *itsLog << LogOrigin("imagepol", "sigmastokesu");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return rvalue;
    }
    rvalue = itsImPol->sigmaStokesU(Float(clip));
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rvalue;
}

double
imagepol::sigmastokesv(const double clip)
{
  double rvalue(-1.0);
  try{
    *itsLog << LogOrigin("imagepol", "sigmastokesv");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return rvalue;
    }
    rvalue = itsImPol->sigmaStokesV(Float(clip));
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rvalue;
}

double
imagepol::sigmatotpolint(const double clip, const double sigma)
{
  double rvalue(-1.0);
  try{
    *itsLog << LogOrigin("imagepol", "sigmatotpolint");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return rvalue;
    }
    rvalue = itsImPol->sigmaTotPolInt(Float(clip),Float(sigma));
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rvalue;
}

image *
imagepol::stokes(const std::string& which, const std::string& outfile)
{
  image *outim = new image();
  try{
    *itsLog << LogOrigin("imagepol", "stokes");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return outim;
    }
    ImageInterface<Float> *out;
    Bool rstat(false);
    String type(which);
    type.upcase();
    if (type=="I") {
      rstat = itsImPol->stokesI(out,String(outfile));
    } else if (type=="Q") {
      rstat = itsImPol->stokesQ(out,String(outfile));
    } else if (type=="U") {
      rstat = itsImPol->stokesU(out,String(outfile));
    } else if (type=="V") {
      rstat = itsImPol->stokesV(out,String(outfile));
    } else {
      throw(AipsError("Stokes "+String(which)+" is unrecognized"));
    }
    if(rstat && !outim->open(out))
      throw(AipsError("could not attach sigmadepolratio image"));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return outim;
}

image *
imagepol::stokesi(const std::string& outfile)
{
  image *outim = new image();
  try{
    *itsLog << LogOrigin("imagepol", "stokesi");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return outim;
    }
    ImageInterface<Float> *out;
    Bool rstat(False);
    rstat = itsImPol->stokesI(out, String(outfile));
    if(rstat && !outim->open(out))
      throw(AipsError("could not attach stokesi image"));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

  return outim;
}

image *
imagepol::stokesq(const std::string& outfile)
{
  image *outim = new image();
  try{
    *itsLog << LogOrigin("imagepol", "stokesq");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return outim;
    }
    ImageInterface<Float> *out;
    Bool rstat(False);
    rstat = itsImPol->stokesQ(out, String(outfile));
    if(rstat && !outim->open(out))
      throw(AipsError("could not attach stokesq image"));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

  return outim;
}

image *
imagepol::stokesu(const std::string& outfile)
{
  image *outim = new image();
  try{
    *itsLog << LogOrigin("imagepol", "stokesu");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return outim;
    }
    ImageInterface<Float> *out;
    Bool rstat(False);
    rstat = itsImPol->stokesU(out, String(outfile));
    if(rstat && !outim->open(out))
      throw(AipsError("could not attach stokesu image"));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

  return outim;
}

image *
imagepol::stokesv(const std::string& outfile)
{
  image *outim = new image();
  try{
    *itsLog << LogOrigin("imagepol", "stokesv");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return outim;
    }
    ImageInterface<Float> *out;
    Bool rstat(False);
    rstat = itsImPol->stokesV(out, String(outfile));
    if(rstat && !outim->open(out))
      throw(AipsError("could not attach stokesv image"));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

  return outim;
}

bool
imagepol::summary()
{
  bool rstat(false);
  try{
    *itsLog << LogOrigin("imagepol", "summary");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return rstat;
    }
    itsImPol->summary();
    rstat = true;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

image *
imagepol::totpolint(const bool debias, const double clip, const double sigma, const std::string& outfile)
{
  image *outim = new image();
  try{
    *itsLog << LogOrigin("imagepol", "totpolint");
    if(itsImPol==0){
      *itsLog << LogIO::SEVERE <<"No attached image, please use open " 
	      << LogIO::POST;
      return outim;
    }
    ImageInterface<Float> *out;
    Bool rstat(False);
    rstat = itsImPol->totPolInt(out,debias,Float(clip),
				Float(sigma),String(outfile));
    if(rstat && !outim->open(out))
      throw(AipsError("could not attach totpolint image"));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

  return outim;
}

} // casac namespace
