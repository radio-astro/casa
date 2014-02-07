//# QtDisplayData.cc: Qt DisplayData wrapper.
//# Copyright (C) 2005
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
//# $Id$

#include <display/QtViewer/QtViewer.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/DisplayDatas/DisplayData.h>
#include <display/DisplayDatas/MSAsRaster.h>
#include <images/Images/SubImage.h>
#include <images/Images/ImageInterface.h>
#include <display/DisplayDatas/LatticeAsRaster.h>
#include <display/DisplayDatas/LatticeAsContour.h>
#include <display/DisplayDatas/LatticeAsVector.h>
#include <display/DisplayDatas/LatticeAsMarker.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <display/DisplayDatas/WedgeDD.h>
#include <display/DisplayDatas/SkyCatOverlayDD.h>
#include <casa/OS/Path.h>
#include <images/Images/PagedImage.h>
#include <images/Images/FITSImage.h>
#include <images/Images/FITSQualityImage.h>
#include <images/Images/FITSImgParser.h>
#include <images/Images/MIRIADImage.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/ImageInfo.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayEvents/WCMotionEvent.h>
#include <casa/iostream.h>
#include <casa/iomanip.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <images/Images/SubImage.h>
#include <images/Images/ImageStatistics.h>
#include <images/Regions/WCBox.h>
#include <images/Regions/WCPolygon.h>
#include <images/Regions/WCIntersection.h>
#include <images/Regions/ImageRegion.h>
#include <casa/OS/File.h>
#include <display/Display/Colormap.h>
#include <display/Display/ColormapDefinition.h>
#include <casa/System/Aipsrc.h>
#include <display/DisplayDatas/WedgeDD.h>
#include <casa/Exceptions/Error.h>
#include <images/Images/ImageExpr.h>
#include <images/Images/ImageExprParse.h>
#include <display/Display/Options.h>
#include <algorithm>
#include <display/functional/elements.h>

#include <imageanalysis/ImageAnalysis/ImageRegridder.h>


namespace casa { //# NAMESPACE CASA - BEGIN

	/*static std::string record_to_string( const Record &rec ) {
		ostringstream os;
		os << rec;
		std::string result = os.str( );
		// std::vector<char> r = std::for_each(result.begin( ),result.end( ),viewer::filter<char>('\n'));
		// return std::string(r.begin(),r.end());
		return std::for_each(result.begin( ),result.end( ),viewer::filter<char,std::string>('\n'));
	}*/


	template <typename T> class anylt {
	public:
		anylt( const T &r ) : rhs(r), tally_(false) { }
		anylt( const anylt &o ) : rhs(o.rhs), tally_(o.tally_) { }
		void operator(  )( const T &lhs ) {
			if ( lhs < rhs ) {
				tally_ = true;
			}
		}
		operator bool( ) const {
			return tally_;
		}

	private:
		T rhs;
		bool tally_;
	};

	template <typename T> class anyge {
	public:
		anyge( const T &r ) :  rhs(r), tally_(false) { }
		anyge( const anyge &o ) : rhs(o.rhs), tally_(o.tally_) { }
		void operator(  )( const T &lhs ) {
			if ( lhs >= rhs ) {
				tally_ = true;
			}
		}
		operator bool( ) const {
			return tally_;
		}

	private:
		T rhs;
		bool tally_;
	};


	QtDisplayData::data_to_qtdata_map_type QtDisplayData::dd_source_map;
	const String QtDisplayData::WEDGE_LABEL_CHAR_SIZE = "wedgelabelcharsize";
	const String QtDisplayData::WEDGE_YES = "Yes";
	const String QtDisplayData::COLOR_MAP = "colormap";
	bool QtDisplayData::globalColorSettings = false;

	void QtDisplayData::setGlobalColorOptions( bool global ) {
		QtDisplayData::globalColorSettings = global;
	}

	struct split_str {
		split_str( char c ) : split_char(c) { }

		operator std::vector<std::string>( ) {
			if ( accum.size( ) > 0 ) {
				values.push_back(accum);
				accum.clear( );
			}
			return values;
		}

		void operator( )( char c ) {
			if ( c == split_char ) {
				if ( accum.size( ) > 0 )
					values.push_back(accum);
				accum.clear( );
			} else {
				accum.push_back(c);
			}
		}

	private:
		char split_char;
		std::string accum;
		std::vector<std::string> values;
	};

	struct check_str {

		check_str( ) : accum(true) { }
		operator bool( ) {
			return accum;
		}

		void operator( )( const std::string &s ) {
			std::string::const_iterator offset = std::find(s.begin(),s.end(),':');
			if ( offset != s.end( ) &&
			        for_each(s.begin( ),offset,check_digit( )) &&
			        for_each(++offset,s.end( ),check_digit( )) )
				accum = accum && true;
			else
				accum = false;
		}

	private:
		struct check_digit {
			check_digit( ) : accum(true) { }
			operator bool( ) {
				return accum;
			}
			void operator( )( char c ) {
				accum = accum && isdigit(c);
			}
		private:
			bool accum;
		};
		bool accum;
	};

	struct str_to_slicer {
		str_to_slicer( ) { }
		operator Slicer( ) {
			return Slicer( IPosition(begin), IPosition(end) );
		}
		void operator( )( const std::string &s ) {
			std::string::const_iterator offset = std::find(s.begin(),s.end(),':');
			begin.push_back(for_each(s.begin( ),offset,digit( )));
			end.push_back(for_each(++offset,s.end( ),digit( )));
		}
	private:
		struct digit {
			digit( ) { }
			operator int( ) {
				return atoi(accum.c_str( ));
			}
			void operator ( )( char c ) {
				accum.push_back(c);
			}
		private:
			std::string accum;
		};

		std::vector<int> begin;
		std::vector<int> end;
	};

	QtDisplayData::QtDisplayData( QtDisplayPanelGui *panel, String path, String dataType,
	                              String displayType, const viewer::DisplayDataOptions &ddo,
	                              const viewer::ImageProperties &props ) :
	    DISPLAY_RASTER("raster"), DISPLAY_CONTOUR("contour"),
	    DISPLAY_VECTOR("vector"), DISPLAY_MARKER( "marker"),
	    panel_(panel),
		path_(path),
		dataType_(dataType),
		displayType_(displayType),
		TYPE_IMAGE( "image"),
		SKY_CATALOG( "skycatalog"), MS( "ms"),
		im_(),
		cim_(),
		dd_(0),
		clrMapName_(""),
		clrMap_(0),
		clrMapOpt_(0),
		colorBar_(0),
		colorBarDisplayOpt_(0),
		colorBarThicknessOpt_(0),
		colorBarLabelSpaceOpt_(0),
		colorBarOrientationOpt_(0),
		colorBarCharSizeOpt_(0),
		image_properties(props) {

		viewer::guiwait wait_cursor;

		QtDisplayData *regrid_to = 0;
		invertColorMap = false;
		std::string method = "";

		if(dataType=="lel") {
			name_ = path_;
			if( name_.length() > 25 )
				name_ =  path_.substr(0,15) + "..." + path_.substr(path_.length()-7,path_.length());
		} else {
			//List<QtDisplayData*> dds_ = panel_->registeredDDs();

			method = ddo["regrid"];
			name_ = Path(path_).baseName();
			QtDisplayPanel* displayPanel = panel_->displayPanel();
			if ( !displayPanel->isEmptyRegistered()) {
				if ( /*dds_.len( ) > 0 &&*/ method != "" &&
				                            (method == "N" || method == "L" || method == "C") ) {
					//ListIter<QtDisplayData*> dds(dds_);
					//dds.toStart( );
					//regrid_to = dds.getRight( );
					DisplayDataHolder::DisplayDataIterator iter = displayPanel->beginRegistered();
					regrid_to = (*iter);
				}
			}
		}

		if(isRaster()) name_ += "-"+displayType_;
		// Default; can be changed with setName()
		// (and should, if it duplicates another name).

		String stdErrMsg = "Cannot display " + name_ + " as ";
		if( isSkyCatalog()){
			stdErrMsg += SKY_CATALOG;
		}
		else stdErrMsg += displayType_ + " " + dataType_;

		errMsg_ = "";

		try {
			if ( isSkyCatalog() ) {
				dd_ = new SkyCatOverlayDD(path);
				if (dd_==0) throw(AipsError("Couldn't create skycatalog"));
			}
			else if( isMS() && isRaster() ) {
				dd_ = new MSAsRaster( path_, ddo );
			} else if(dataType_==TYPE_IMAGE || dataType_=="lel") {

				if(dataType_==TYPE_IMAGE) {

					// check for a FITS extension in the path name
					File fin(path);
					String tmp_path, ext_expr;
					tmp_path = path;
					if (!fin.exists() && !fin.isDirectory()) {
						if (!(int)path.compare(path.length()-1, 1, "]", 1) && (int)path.rfind("[", path.length()) > -1) {
							// create a string with the file path name only
							tmp_path = String(path, 0, path.rfind("[", path.length()));
							ext_expr = String(path, path.rfind("[", path.length()), path.length());
						}
					}

					// use the file path name for the opener
					switch(ImageOpener::imageType(tmp_path)) {

					case ImageOpener::AIPSPP: {

						if( imagePixelType(path_) == TpFloat ) {
							im_.reset(new PagedImage<Float>(path_, TableLock::AutoNoReadLocking));
							// regions in image...
							// Vector<String> regions = im_->regionNames( );
							// for ( int i = 0; i < regions.size( ); ++i ) {
							// 	cout << "\t\t\t\t\t(" << i << "): " << regions[i] << endl;
							// }
						} else if(imagePixelType(path_)==TpComplex) {
							cim_.reset( new PagedImage<Complex>(path_, TableLock::AutoNoReadLocking));
						} else  throw AipsError( "Only Float and Complex CASA images are supported at present.");
						break;
					}
					case ImageOpener::FITS: {
						FITSImgParser fip = FITSImgParser(tmp_path);
						if (fip.has_qualityimg() && fip.is_qualityimg(ext_expr)) {
							im_.reset(new FITSQualityImage(path));
						} else {
							im_.reset(new FITSImage(path));
						}
						break;
					}
					case ImageOpener::MIRIAD: {
						im_.reset(new MIRIADImage(path));
						break;
					}

					default: {
						File f(path);
						if(!f.exists()) throw AipsError("File not found.");
						if (f.isDirectory()) {
							//Defer image construction.
							return;
						}
						throw AipsError("Only casa, FITS and MIRIAD images are supported.");
					}
					}

					if ( im_ != 0 ) {
						if ( regrid_to && method != "" ) {
							// regrid new image to match the one provided...
							// need an option to delete temporary files on exit...
							std::string outpath = viewer::options.temporaryPath(Path(path_).baseName());
							panel_->status( "generating temporary image: " + outpath );
							panel_->logIO( ) << "generating temporary image \'" << outpath << "'" << LogIO::POST;
							ImageRegridder regridder(im_, String(outpath), regrid_to->imageInterface( ) );
							regridder.setMethod(method);
							regridder.setSpecAsVelocity(True);
							im_.reset(regridder.regrid(True));
							// std::auto_ptr<ImageInterface<Float> > imptr(im_);
						}
						std::string slice_description = ddo["slice"];
						if ( slice_description != "" && slice_description != "none" ) {
							size_t openp = slice_description.find('(');
							size_t closep = slice_description.rfind(')');
							if ( openp != string::npos && closep != string::npos && openp+1 < closep-1 ) {
								try {
									std::string str = slice_description.substr(openp+1,closep-openp-1);
									std::vector<std::string> vec = for_each(str.begin( ),str.end( ),split_str(','));
									if ( for_each(vec.begin( ),vec.end( ),check_str( )) ) {
										Slicer slicer = for_each(vec.begin( ),vec.end( ),str_to_slicer( ));
										SubImage<Float> *subim = new SubImage<Float>( (const ImageInterface<Float>&) *im_, slicer );
										//std::auto_ptr<ImageInterface<Float> > imptr(im_);
										im_.reset(subim);
									}
								} catch( const AipsError &err ) {
									panel_->status( "unnable to generate sub-image: " + err.getMesg( ) );
									panel_->logIO( ) << LogIO::WARN << "unnable to generate sub-image: " << err.getMesg( ) << LogIO::POST;
								}
							}
						}
					} else if ( cim_ == 0 ) {
						throw AipsError("Couldn't create image.");
					}
				} else {
					// Parse LEL expression to create expression-type ImageInterface.

					LatticeExprNode expr = ImageExprParse::command(path_);

					if(expr.dataType() == TpFloat) {
						im_.reset(new ImageExpr<Float>(LatticeExpr<Float>(expr), name_));
					} else if(expr.dataType() == TpComplex) {
						cim_.reset( new ImageExpr<Complex>(LatticeExpr<Complex>(expr), name_));
					} else throw AipsError("Only Float or Complex LEL expressions "
						                       "are allowed");
				}
				initImage();
			}
			// Needed (for unknown reasons) to avoid
			// blocking other users of the image file....
			// (11/07 -- locking mode changed; may no longer be necessary...).
			else throw AipsError("Unsupported data type: " + dataType_);
		}
		// (failure.. Is it best to try to delete the remains, or leave
		// them hanging?  Latter course chosen here...).

		catch (const AipsError& err) {
			errMsg_ = stdErrMsg;
			if(err.getMesg()!="") errMsg_ += ":\n  " + err.getMesg();
			else                  errMsg_ += ".";
			dd_=0;
		}

		catch (...) {
			errMsg_ = stdErrMsg + ".";
			dd_=0;
		}


		if(dd_==0) {
			emit qddError(errMsg_);
			// (error signal propagated externally, rather than throw.
			// Alternatively, caller can test newQdd->isEmpty()).
			im_.reset();
			cim_.reset();
			return;
		}


		init();
		setPlotTitle();
	}

	bool QtDisplayData::isSkyCatalog() const {
		return displayType_==SKY_CATALOG;
	}


	bool QtDisplayData::isMS() const {
		return dataType_==MS;
	}

	void QtDisplayData::setPlotTitle() {
		Record record;
		Record outRecord;
		record.define( WCAxisLabeller::PLOT_TITLE, name());
		dd_->setOptions( record, outRecord );
	}

	void QtDisplayData::setImage(std::tr1::shared_ptr<ImageInterface<Float> > img ) {
		im_=img;
	}

	void QtDisplayData::initImage() {
		uInt ndim = (im_!=0)? im_->ndim() : cim_->ndim();
		if(ndim<2) throw AipsError("Image doesn't have >=2 dimensions.");
		// (Probably won't happen).

		IPosition shape = (im_!=0)? im_->shape() : cim_->shape();
		Block<uInt> axs(ndim);

		DisplayCoordinateSystem cs = (im_!=0) ? im_->coordinates() : cim_->coordinates();

		getInitialAxes_(axs, shape, cs);
		//#dk  getInitialAxes(axs, shape);

		IPosition fixedPos(ndim);
		fixedPos = 0;

		if( isRaster()) {
			if(im_!=0) {
				if(ndim ==2){
					dd_ = new LatticeAsRaster<Float>(im_, 0, 1);
				}
				else {
					dd_ = new LatticeAsRaster<Float>(im_, axs[0], axs[1], axs[2], fixedPos, panel_ );
				}
			} else {
				if(ndim ==2) {
					dd_ = new LatticeAsRaster<Complex>(cim_, 0, 1);
				}
				else {
					dd_ = new LatticeAsRaster<Complex>(cim_, axs[0], axs[1], axs[2], fixedPos, panel_ );
				}
			}
		} else if( isContour() ) {
			if(im_!=0) {
				if(ndim ==2) {
					dd_ = new LatticeAsContour<Float>(im_, 0, 1);
				}
				else {
					dd_ = new LatticeAsContour<Float>( im_, axs[0], axs[1], axs[2], fixedPos, panel_ );
				}
			} else {
				if(ndim ==2){
					dd_ = new LatticeAsContour<Complex>(cim_, 0, 1);
				}
				else {
					dd_ = new LatticeAsContour<Complex>( cim_, axs[0], axs[1], axs[2], fixedPos, panel_ );
				}
			}
		} else if( isVector() ) {
			if(im_!=0) {
				if(ndim ==2){
					dd_ = new LatticeAsVector<Float>(im_, 0, 1);
				}
				else {
					dd_ = new LatticeAsVector<Float>(im_, axs[0], axs[1], axs[2], fixedPos);
				}
			} else {
				if(ndim ==2) {
					dd_ = new LatticeAsVector<Complex>(cim_, 0, 1);
				}
				else {
					dd_ = new LatticeAsVector<Complex>(cim_, axs[0], axs[1], axs[2], fixedPos);
				}
			}
		} else if( isMarker()) {
			if(im_!=0) {
				if(ndim ==2){
					dd_ = new LatticeAsMarker<Float>(im_, 0, 1);
				}
				else {
					dd_ = new LatticeAsMarker<Float>(im_, axs[0], axs[1], axs[2], fixedPos);
				}
			} else {
				if(ndim ==2) {
					dd_ = new LatticeAsMarker<Complex>(cim_, 0, 1);
				}
				else {
					dd_ = new LatticeAsMarker<Complex>(cim_, axs[0], axs[1], axs[2], fixedPos);
				}
			}
		} else throw AipsError("Unsupported image display type: " + displayType_);

		if(im_!=0) im_->unlock();
		else      cim_->unlock();
	}





	void QtDisplayData::init() {
		// Successful creation.
		dd_source_map.insert( data_to_qtdata_map_type::value_type(dd_,this) );

		dd_->setUIBase(0);
		// Items are numbered from zero in casaviewer as consistently
		// as possible (including, e.g., axis 'pixel' positions).  This call
		// is necessary after constructing DDs, to orient them away from
		// their old default 1-based (glish) behavior.


		// (For parsing user colormap selection via
		// getOptions() / setOptions() / Options gui).
		initColorSettings();

		//Init global color settings. If 'global' is checked, we need to update
		//a new QtDisplayData with whatever the global color settings are.
		initGlobalColorSettings();
	}

	void QtDisplayData::initGlobalColorSettings(){
		if ( globalColorSettings ){
			QtDisplayData* colorTemplateDD = this->panel_->dd();
			if ( colorTemplateDD != NULL ){
				Record options = colorTemplateDD->getOptions();
				Record globalColorOptions = getGlobalColorChangeRecord( options );
				setOptions( globalColorOptions );
			}
		}
	}

	void QtDisplayData::initColorSettings() {
		// Initialize colormap if necessary.
		if(usesClrMap_()) {
			// The CSSC has made the decision that the default colormap should
			// be a rainbow.
			String defaultCMName;
			Aipsrc::find(defaultCMName, "display.colormaps.defaultcolormap",
					             "Rainbow 2");
			// ...but fall back to "Greyscale 1" unless the above is a valid
			// ('primary') name.  ('Synonym' colormap names (like "mono") are not
			// supported at present through this QDD interface).  In case the table
			// of standard colormaps is not in the installation data repository,
			// this will also fall back to the (hard-coded) "Greyscale 1"...

			String initialCMName = "Greyscale 1";  // (Always valid/available).

			clrMapNames_ = ColormapDefinition::builtinColormapNames();

			// (Here 'builtin' means in a loaded table, or else "Greyscale 1").
			// Don't use colormap 'synonyms' like "mono" in .aipsrc or
			// QDD::setOptions()).  Note that it is possible to load a
			// custom colormap table as well, using .aipsrc (see
			// builtinColormapNames()); improvements are also planned.

			colormapnamemap::iterator cmniter = clrMapNames_.find(defaultCMName);
			if ( cmniter != clrMapNames_.end() ) {
				// defaultCMName is a valid choice -- use it.
				initialCMName = defaultCMName;
			}

			setColormap_(initialCMName);


			clrMapOpt_ = new DParameterChoice(COLOR_MAP, "Color Map",
					                                  "Name of the mapping from data values to color",
					                                  clrMapNames_, initialCMName, initialCMName, "");
		}
		// Initialization for color bar, if necessary.

		if(usesColorBar_() && clrMap_!=0) {

			colorBar_ = new WedgeDD(dd_);
			colorBar_->setColormap(clrMap_, 1.);

			Vector<String> yesNo(2);
			yesNo[0]=WEDGE_YES;
			yesNo[1]="No";

			Vector<String> vertHor(2);
			vertHor[0]="vertical";
			vertHor[1]="horizontal";

			String orientation = panel_->colorBarsVertical()? vertHor[0] : vertHor[1];

			colorBarDisplayOpt_ = new DParameterChoice(WedgeDD::WEDGE_PREFIX,
			        "display color wedge?",
			        "Whether to display a 'color bar' that indicates\n"
			        "the current mapping of colors to data values.",
			        yesNo, yesNo[1], yesNo[1], "color_wedge");

			colorBarThicknessOpt_ = new DParameterRange<Float>("wedgethickness",
			        "wedge thickness",
			        "Manual adjustment factor for thickness of colorbar.\n"
			        "Vary this if automatic thickness choice is not satisfactory.",
			        .3, 5.,  .1,  1., 1.,  "color_wedge");

			colorBarLabelSpaceOpt_ = new DParameterRange<Float>("wedgelabelspace",
			        "wedge label space",
			        "Manual adjustment factor for colorbar's label space.\n"
			        "Vary this if automatic margin choice is not satisfactory.",
			        .1, 2.,  .05,  1., 1.,  "color_wedge");

			colorBarOrientationOpt_ = new DParameterChoice("orientation",
			        "wedge orientation",
			        "Whether to display color bars vertically or horizontally.\n"
			        "(Note: orientation will be the same for all color bars\n"
			        "in all display panels).",
			        vertHor, orientation, orientation, "color_wedge");

			// (This one is just used to monitor changes to the "wedgelabelcharsize"
			// parameter during setOptions().  getOptions() uses the values
			// from colorBar_ (the WedgeDD)).
			colorBarCharSizeOpt_ = new DParameterRange<Float>(
			    WEDGE_LABEL_CHAR_SIZE, "character size", "",
			    0.2, 4.,  .05,  1.2, 1.2, "color_wedge");

			// Initialize colorBarCharSizeOpt_'s value from colorBar_.
			Record cbopts = colorBar_->getOptions();
			colorBarCharSizeOpt_->fromRecord(cbopts);


			// Initialize some colorBar_ options which depend on other objects.
			// "datamin" and "datamax" come from same same option fields (or from
			// the "minmaxhist" field) of the main dd_, as does "powercycles".
			// "orientation" comes from the central viewer object v_ (it is the
			// same for all color bars).  "dataunit" comes from dd_'s dataUnit()
			// method.

			Record cbropts, chgdopts;
			// I'd just reuse cbopts if I could, but there's no explicit way
			// to clear a Record, nor one to assign another arbitrary Record
			// to it.  (As with Vectors, the assignee must be 'compatible';
			// but who _cares_ what the record _used_ to be!  'a = b' _should_
			// mean the assignee becomes a copy of the other, regardless...).

			Record mainDDopts = dd_->getOptions();

			Float datamin=0., datamax=1.,  powercycles=0.;
			Vector<Float> minMax;
			Bool notFound;

			dd_->readOptionRecord(minMax, notFound, mainDDopts, PrincipalAxesDD::HISTOGRAM_RANGE);
			// (DisplayOptions methods like readOptionRecord() should be
			// static -- they're stateless.  Any DisplayOptions object (or
			// a newly-created one) would do here instead of dd_, which
			// just happened to be on hand...).

			if(minMax.nelements()==2) {
				cbropts.define(DisplayData::DATA_MIN, minMax[0]);
				cbropts.define(DisplayData::DATA_MAX, minMax[1]);
			} else {
				dd_->readOptionRecord(datamin, notFound, mainDDopts, DisplayData::DATA_MIN);
				if(!notFound) cbropts.define(DisplayData::DATA_MIN, datamin);
				dd_->readOptionRecord(datamax, notFound, mainDDopts, DisplayData::DATA_MAX);
				if(!notFound) cbropts.define(DisplayData::DATA_MAX, datamax);
			}

			dd_->readOptionRecord(powercycles, notFound, mainDDopts, WCPowerScaleHandler::POWER_CYCLES);
			if(!notFound) {
				cbropts.define(WCPowerScaleHandler::POWER_CYCLES, powercycles);
			}

			String dataunit = "";
			try {
				dataunit = dd_->dataUnit().getName();
			}  catch(...) {  }
			// (try because LaticePADD::dataUnit() is (stupidly) willing
			// to throw up instead of returning Unit("") as a fallback...)
			if(dataunit=="_") dataunit="";
			cbropts.define("dataunit", dataunit);

			colorBar_->setOptions(cbropts, chgdopts);

			connect( panel_, SIGNAL(colorBarOrientationChange()),
			         SLOT(setColorBarOrientation_()) );

			connect(this, SIGNAL(statsReady(const String&)),
			        panel_,  SLOT(showStats(const String&)));

			setColorBarOrientation_();
		}
	}

	std::string QtDisplayData::path( const DisplayData *d ) {
		if ( d == NULL ) return "";
		data_to_qtdata_map_type::iterator it = dd_source_map.find(d);
		if ( it == dd_source_map.end( ) ) return "";
		return it->second->path( );
	}

	Colormap* QtDisplayData::getColorMap() const {
		return clrMap_;
	}

	QtDisplayData::~QtDisplayData() {
		for (data_to_qtdata_map_type::iterator it = dd_source_map.begin( ); it != dd_source_map.end();) {
			if ( it->second == this )
				dd_source_map.erase(it++);
			else ++it;
		}
		done();
	}

	std::string QtDisplayData::description( ) const {
		return name_ + " (" + path_ + ") " + dataType_ + "/" + displayType_;
	}

//Display Type
	Bool QtDisplayData::isRaster() const {
		bool rasterData = false;
		if ( displayType_== DISPLAY_RASTER) {
			rasterData = true;
		}
		return rasterData;
	}
	Bool QtDisplayData::isContour() const {
		bool contourData = false;
		if ( displayType_==DISPLAY_CONTOUR) {
			contourData = true;
		}
		return contourData;
	}
	Bool QtDisplayData::isVector() const {
		bool vectorData = false;
		if ( displayType_==DISPLAY_VECTOR) {
			vectorData = true;
		}
		return vectorData;
	}
	Bool QtDisplayData::isMarker() const {
		bool markerData = false;
		if ( displayType_==DISPLAY_MARKER) {
			markerData = true;
		}
		return markerData;
	}

	Bool QtDisplayData::isImage() const {
		bool typeImage = false;
		if ( dataType_==TYPE_IMAGE ) {
			typeImage = true;
		}
		return typeImage;
	}

//Bool QtDisplayData::delTmpData() const {
	void QtDisplayData::delTmpData() const {
		dd_->getDelTmpData();
		return;
	}

	void QtDisplayData::setDelTmpData(Bool delTmpData) {
		dd_->setDelTmpData(delTmpData);
	}

	void QtDisplayData::done() {
		if(dd_==0) return;		// (already done).
		//emit dying(this);

		if(usesClrMap_()) {
			removeColormap_();
			for ( colormapmap::iterator iter = clrMaps_.begin( );
			        iter != clrMaps_.end( ); ++iter ) {
				delete iter->second;		// Remove/delete any existing dd colormap[s].
			}
			delete clrMapOpt_;
			clrMapOpt_=0;
		}

		if(hasColorBar()) {
			delete colorBar_;

			delete colorBarDisplayOpt_;
			delete colorBarThicknessOpt_;
			delete colorBarLabelSpaceOpt_;
			delete colorBarOrientationOpt_;
			delete colorBarCharSizeOpt_;
		}

		delete dd_;
		dd_=0;
		if(im_)  {
			im_.reset();
		}
		if(cim_) {
			cim_.reset();
		}
	}



	Record QtDisplayData::getOptions() {
		// retrieve the Record of options.  This is similar to a 'Parameter Set',
		// containing option types, default values, and meta-information,
		// suitable for building a user interface for controlling the DD.

		if(dd_==0) return Record();  //# (safety, in case construction failed.).

		Record opts = dd_->getOptions();

		// DD colormap setting was placed outside the get/setOptions framework
		// used for other DD options (for some reason).  This allows it to be
		// treated like any other dd option, at least from QDD interface...
		if(usesClrMap_()) clrMapOpt_->toRecord(opts);

		// RGB and HSV modes are not (yet) supported in the Qt viewer.
		// (Something better (transparency) may be implemented instead).
		// Remove those options unless/until supported.
		if(opts.isDefined("colormode")) opts.removeField("colormode");

		// Also, 'Histogram equalization' was not implemented in a useful way
		// and just cluttered up the interface, so it is excised here as well.
		// (Not to be confused with the glish viewer's histogram mix-max setting
		// display, which everyone wants to see re-implemented under Qt...).
		if(opts.isDefined("histoequalisation")) {
			opts.removeField("histoequalisation");
		}

		// Colorbar options
		if(hasColorBar()) {

			// In addition to the five color bar labelling options forwarded from
			// colorBar()->getOptions() below, this method also returns "wedge"
			// ("Display Color Wedge?"), "wedgethickness" ("Wedge Thickness"),
			// "wedgelabelspace" ("Wedge Label Space") and "orientation"
			// ("Wedge Orientation" (horizontal/vertical)) options itself.
			// These 9 comprise the direct user interface options which will appear
			// in the "Color Wedge" dropdown of the options window.
			//
			// "wedge", "wedgethickness" and "wedgelabelspace" are handled entirely
			// externally to WedgeDD.

			Record cbopts = colorBar()->getOptions();
			colorBarDisplayOpt_->toRecord(opts);     // "wedge" field ("Yes" / "No").
			colorBarThicknessOpt_->toRecord(opts);   // "wedgethickness" (default 1.)
			colorBarLabelSpaceOpt_->toRecord(opts);  // "wedgelabelspace" (default 1.)
			opts.mergeField(cbopts, WEDGE_LABEL_CHAR_SIZE,
			                Record::OverwriteDuplicates);
			// (Color bar panel thickness and character size are often changed
			// together; putting this here keeps the options next to each other
			// in their user interface panel).

			colorBarOrientationOpt_->toRecord(opts);  // "orientation"
			// ("vertical" / "horizontal";  default "vertical")


			// Forward only the five options begining with "wedge" ("wedgeaxistext",
			// "wedgeaxistextcolor", "wedgelabellinewidth", "wedgelabelcharsize"
			// and "wedgelabelcharfont") from color bar (WedgeDD) to user interface;
			// these all have to do with color bar labelling.  (Note that
			// "wedgelabelcharsize" has already been 'wedged in', above).

			for(uInt i=0; i<cbopts.nfields(); i++) {
				String fldname = cbopts.name(i);
				if(fldname.before(5)==WedgeDD::WEDGE_PREFIX && fldname!=WEDGE_LABEL_CHAR_SIZE) {
					opts.mergeField(cbopts, i, Record::OverwriteDuplicates);
					opts.rwSubRecord(fldname).define("dlformat", fldname);
				}
			}
		}

		return opts;
	}
	String QtDisplayData::getZAxisName(){
		Record rec = getOptions();
		String zAxisName;
		try {
			zAxisName = rec.subRecord( "zaxis").asString("value");
		}
		catch( AipsError& error ){
			//Image did not have a z-axis - no big deal.
		}
		return zAxisName;
	}



	void QtDisplayData::checkAxis( bool changeSpectralUnits ) {
		Record rec = getOptions();

		try {
			// get the axis names
			String xaxis = rec.subRecord("xaxis").asString("value");
			String yaxis = rec.subRecord("yaxis").asString("value");
			String zaxis = rec.subRecord("zaxis").asString("value");

			std::vector<int> hidden;
			char field[24];
			int index = 0;
			while ( 1 ) {
				sprintf( field, "haxis%d", ++index );
				if ( rec.fieldNumber( field ) < 0 ) {
					break;
				}
				Int haxis = rec.subRecord("haxis1").asInt("value");
				hidden.push_back(haxis);
			}
			emit axisChangedProfile(xaxis, yaxis, zaxis, hidden);
			emit axisChanged(xaxis, yaxis, zaxis, hidden);

			if ( changeSpectralUnits ){
				// get the spectral type, units, rest frequency/wavelength
				// and the frequency system //axislabelspectypeunit
				String spcRval, spcSys, spcTypeUnit;
				if (rec.fieldNumber("axislabelspectypeunit") >-1){
					spcTypeUnit = rec.subRecord("axislabelspectypeunit").asString("value");
				}
				if (rec.fieldNumber("axislabelrestvalue") >-1){
					spcRval = rec.subRecord("axislabelrestvalue").asString("value");
				}
				if (rec.fieldNumber("axislabelfrequencysystem") >-1){
					spcSys = rec.subRecord("axislabelfrequencysystem").asString("value");
				}
				emit spectrumChanged(spcTypeUnit, spcRval, spcSys);
			}
		} catch(const casa::AipsError& err) {
			errMsg_ = err.getMesg();
		} catch(...) {
			errMsg_ = "Unknown error checking axis";
		}
	}



	void QtDisplayData::setOptions(Record opts, Bool emitAll) {
		// Apply option values to the DisplayData.  Method will
		// emit optionsChanged() if other option values, limits, etc.
		// should also change as a result.
		// Set emitAll = True if the call was not initiated by the options gui
		// itself (e.g. via scripting or save-restore); that will ensure that
		// the options gui does receive all option updates (via the optionsChanged
		// signal) and updates its user interface accordingly.
		if(dd_==0) return;  // (safety, in case construction failed.).

		// dump out information about the set-options message flow...
		//cout << "\t>>== " << (emitAll ? "true" : "false") << "==>> " << opts << endl;

		Record chgdOpts;
		Bool held=False;
		try {

			//Decide if the refresh is local to this dd or if we are going to have
			//refresh all of them.
			Bool needsRefresh = dd_->setOptions(opts, chgdOpts);
			Bool cbNeedsRefresh = setColorBarOptions( opts, chgdOpts );
			if ( cbNeedsRefresh ) {
				needsRefresh = true;
			}
			// options were set ok.  (QtDDGui will
			// use it to clear status line, e.g.).

			// Beware: WedgeDD::setOptions() (unexpectedly, stupidly) alters
			// cbopts, instead of keeping all its hacks internal.
			// Don't expect the original cbopts after this call....
			// (Note: chgdcbopts is ignored (unused) for colorbar).

			// Test for changes in these, recording any new values.
			// Of these, WedgeDD processes only "wedgelabelcharsize"
			// (colorBarCharSizeOpt_ -- it was merged into cbopts, above).

			Bool reorient = false;
			if ( colorBarOrientationOpt_ != NULL ) {
				reorient = colorBarOrientationOpt_->fromRecord(opts);
			}
			Bool cbChg = false;
			if ( colorBarDisplayOpt_ != NULL ) {
				cbChg    = colorBarDisplayOpt_->fromRecord(opts);	// "wedge"
			}
			Bool cbSzChg = false;
			if ( colorBarThicknessOpt_ != NULL ) {
				cbSzChg  = colorBarThicknessOpt_->fromRecord(opts);
			}
			if ( colorBarCharSizeOpt_ != NULL ) {
				cbSzChg  = colorBarCharSizeOpt_->fromRecord(opts)   || cbSzChg;
			}
			if (colorBarLabelSpaceOpt_ != NULL ) {
				cbSzChg  = colorBarLabelSpaceOpt_->fromRecord(opts) || cbSzChg;
			}
			held=True;
			panel_->viewer()->hold();
			// (avoids redrawing more often than necessary)

			// Trigger color bar and main panel rearrangement, if necessary.
			if(reorient) {
				colorBarOrientationOpt_->toRecord(chgdOpts, True, True);
				// Make sure user interface sees this change via chgdOpts.
				Bool orientation = (colorBarOrientationOpt_->value()=="vertical");
				panel_->setColorBarOrientation(orientation);
			} else if(cbChg || (wouldDisplayColorBar() && cbSzChg) ) {
				emit colorBarChange();
			}
			if(!held) {
				held=True;
				panel_->viewer()->hold();
			}

			// Refresh all main canvases where dd_ is registered, if required
			// because of option changes (it usually is).
			// Note: the 'True' parameter to refresh() is a  sneaky/kludgy
			// part of the refresh cycle interface which is easily missed.
			// In practice what it means now is that DDs on the PrincipalAxesDD
			// branch get their drawlist cache cleared.  It has no effect
			// (on caching or otherwise) for DDs on the CachingDD branch.
			if(needsRefresh) {
				dd_->refresh(True);
			}

			held=False;
			panel_->viewer()->release();
			if(cbNeedsRefresh) {
				emit colorBarChange();
			}

			errMsg_ = "";		// Just lets anyone interested know that
			emit optionsSet();
			if ( needsRefresh && opts.nfields() > 0 ) {
				checkGlobalChange( opts );
			}

		} catch (const casa::AipsError& err) {
			errMsg_ = err.getMesg();
			cerr<<"qdd setOpts Err:"<<errMsg_<<endl;	//#dg
			if(held) {
				held=False;
				panel_->viewer()->release();
			}
			emit qddError(errMsg_);
		} catch (...) {
			errMsg_ = "Unknown error setting data options";
			cerr<<"qdd setOpts Err:"<<errMsg_<<endl;	//#dg
			if(held) {
				held=False;
				panel_->viewer()->release();
			}
			emit qddError(errMsg_);
		}
		checkAxis();

		// [Other, dependent] options the dd itself changed in
		// response.  Option guis will want to monitor this and
		// change their interface accordingly.
		//
		// The 'setanimator' sub-record of chgdOpts, if any, is not
		// an option of an individual dd, but a request to reset the
		// state of the animator (number of frames, current frame or
		// 'index').
		// (To do: This code should probably not allow explicit
		// 'zlength' or 'zindex' fields to be specified within the
		// 'setanimator' sub-record unless the dd is CS master).
		if(emitAll) {
			chgdOpts.merge(opts, Record::SkipDuplicates);
		}
		// When emitAll==True this ensures that the options gui
		// receives all option updates via the optionsChanged
		// signal, not just internally-generated ones.  For use
		// when the gui itself didn't initiate the setOptions call.
		// Note that the internal chgdOpts are a response to the
		// original opts sent into this method and are more recent;
		// they overwrite the original opts if the same option
		// is present in both.

		// emit an event to sync up GUIs based upon changes registered
		// by the display datas, e.g. update changed axis dimensionality
		// triggered in PrincipalAxesDD::setOptions( ) to keep all
		// axis dimensions unique...
		if(chgdOpts.nfields()!=0) {
			emit optionsChanged(chgdOpts);
		}


		if( opts.isDefined("Show histogram plot")) {
			emit showColorHistogram( this );
		}


		if(chgdOpts.isDefined("trackingchange")) emit trackingChange(this);
		// Tells QDP to gather and emit tracking data again.   Field
		// value is irrelevant. (There are other useful places where
		// DDs could define this field....)

	}

	Record QtDisplayData::getGlobalColorChangeRecord( Record& opts ) const {
		Record globalChangeRecord;
		//Remove all but the global options from the opts.
		if ( opts.isDefined( PrincipalAxesDD::HISTOGRAM_RANGE )){
			int histFieldId = opts.fieldNumber( PrincipalAxesDD::HISTOGRAM_RANGE );
			if ( histFieldId != -1 ) {
				if ( opts.dataType(PrincipalAxesDD::HISTOGRAM_RANGE) == TpRecord ) {
					Record rangeRecord = opts.subRecord( PrincipalAxesDD::HISTOGRAM_RANGE );
					globalChangeRecord.defineRecord( PrincipalAxesDD::HISTOGRAM_RANGE, rangeRecord );
				}
				else if (opts.dataType(PrincipalAxesDD::HISTOGRAM_RANGE) == TpArrayFloat) {
					Vector<Float> minMaxVector(opts.toArrayFloat(PrincipalAxesDD::HISTOGRAM_RANGE));
					Record rangeRecord;
					rangeRecord.define("value", minMaxVector);
					globalChangeRecord.defineRecord( PrincipalAxesDD::HISTOGRAM_RANGE, rangeRecord );
				}
				else {
					qDebug() <<"QtDisplayData::checkGlobalChange - unrecognized opts.dataType="<<opts.dataType(PrincipalAxesDD::HISTOGRAM_RANGE);
				}
			}
		}
		if ( opts.isDefined( COLOR_MAP )){
			int colorMapId = opts.fieldNumber( COLOR_MAP );
			if ( colorMapId != -1 ) {
				if ( opts.dataType( COLOR_MAP) == TpString ){
					String colorMapName = opts.asString( COLOR_MAP );
					globalChangeRecord.define( COLOR_MAP, colorMapName);
				}
				else if ( opts.dataType( COLOR_MAP)  == TpRecord ){
					Record colorRecord = opts.asRecord( COLOR_MAP );
					globalChangeRecord.defineRecord( COLOR_MAP, colorRecord );
				}
				else {
					qDebug() << "QtDisplayData::checkGlobalChange - unrecognized opts colormap type="<<opts.dataType( COLOR_MAP );
				}
			}
		}

		if ( opts.isDefined( WCPowerScaleHandler::POWER_CYCLES )){
			int powerId = opts.fieldNumber( WCPowerScaleHandler::POWER_CYCLES);
			if ( powerId != -1 ) {
				float powerValue = 0;
				if ( opts.dataType(WCPowerScaleHandler::POWER_CYCLES ) == TpRecord ) {
					Record subPowerRecord = opts.subRecord(WCPowerScaleHandler::POWER_CYCLES);
					powerValue = subPowerRecord.asFloat( "value");
				}
				else {
					powerValue = opts.asFloat( WCPowerScaleHandler::POWER_CYCLES );
				}
				Record powerRecord;
				powerRecord.define( "value", powerValue );
				globalChangeRecord.defineRecord( WCPowerScaleHandler::POWER_CYCLES, powerRecord);
			}
		}
		return globalChangeRecord;
	}

	void QtDisplayData::checkGlobalChange( Record& opts ) {
		if ( globalColorSettings ) {
			Record globalChangeRecord = getGlobalColorChangeRecord(opts);
			if ( globalChangeRecord.nfields() > 0 ) {
				emit globalOptionsChanged( this, globalChangeRecord );
			}
		}
	}

	void QtDisplayData::setInvertColorMap( bool invert ) {
		invertColorMap = invert;
		setColormap_(clrMapName_, true);
	}

	bool QtDisplayData::setColorBarOptions( Record& opts, Record& chgdOpts ) {
		//fprintf( stderr, "<0x%x>\tQtDisplayData::setColorBarOptions( ", this );
		//cerr << record_to_string(opts) << ", Record& )\n";
		Bool cbNeedsRefresh = False;
		if(usesClrMap_() && clrMapOpt_->fromRecord(opts)) {
			cbNeedsRefresh = True;
			setColormap_(clrMapOpt_->value());
		}

		// Also process change in colormap choice, if any.  This
		// was left out of the setOptions interface on the DD level.
		if(hasColorBar()) {
			// Note: In addition to the five labelling options ("wedgeaxistext",
			// "wedgeaxistextcolor" "wedgelabelcharsize", "wedgelabellinewidth"
			// and "wedgelabelcharfont"), WedgeDD::setOptions() is sensitive
			// to "orientation", "datamin", "datamax" "dataunit" and "powercycles"
			// fields.  Of the latter five, only "orientation" appears directly in
			// the "Color Wedge" dropdown, and even it is massaged externally (it
			// is ultimately controlled by QtViewerBase::setColorBarOrientation()).
			// "powercycles" shares the "Basic Settings" user interface (and the
			// setOptions field) with QDD's main DD, as do "datamin" and "datamax"
			// (these latter two are sometimes gleaned from the "minmaxhist" field
			// of LatticeAsRaster instead).
			// The "dataunit" setting is controlled internally according to what
			// is appropriate for QDD's main DD; it has no direct user interface.

			// It is not desirable to forward all of the main DD opts, esp.
			// labelling options meant for main panel only.  The following
			// code picks out and forwards the needed ones to the color bar.

			Record cbopts;	  // options to pass on to colorBar_->setOptions()
			Record chgdcbopts;  // required (but ignored) setOptions() parameter

			for(uInt i=0; i<opts.nfields(); i++) {
				String fldname = opts.name(i);
				if( (fldname.before(5)==WedgeDD::WEDGE_PREFIX && fldname!=WedgeDD::WEDGE_PREFIX) ||
				        fldname=="orientation" || fldname=="dataunit"   ||
				        fldname==WCPowerScaleHandler::POWER_CYCLES ) {		// (Forward these verbatum).

					cbopts.mergeField(opts, i, Record::OverwriteDuplicates);
				}
			}


			// Priority for datamin and datamax definition:
			//   1) "minmaxhist"          in chgdOpts  (internal request)
			//   2) "datamin", "datamax"  in chgdOpts  (internal request)
			//   3) "minmaxhist"          in opts      (user request)
			//   4) "datamin", "datamax"  in opts      (user request)

			Float datamin=0., datamax=1.;
			Vector<Float> minMax;
			Bool notFound=True, minNotFound=True, maxNotFound=True;

			dd_->readOptionRecord(minMax, notFound, chgdOpts, PrincipalAxesDD::HISTOGRAM_RANGE);
			if(minMax.nelements()==2) {
				datamin = minMax[0];
				minNotFound=False;
				datamax = minMax[1];
				maxNotFound=False;
			} else {
				dd_->readOptionRecord(datamin, minNotFound, chgdOpts, DisplayData::DATA_MIN);
				dd_->readOptionRecord(datamax, maxNotFound, chgdOpts, DisplayData::DATA_MAX);

				minMax.resize(0);
				dd_->readOptionRecord(minMax, notFound, opts, PrincipalAxesDD::HISTOGRAM_RANGE);
				if(minMax.nelements()==2) {
					if(minNotFound) {
						datamin = minMax[0];
						minNotFound=False;
					}
					if(maxNotFound) {
						datamax = minMax[1];
						maxNotFound=False;
					}
				} else {
					if(minNotFound) {
						dd_->readOptionRecord(datamin, minNotFound, opts,DisplayData::DATA_MIN);
					}
					if(maxNotFound) {
						dd_->readOptionRecord(datamax, maxNotFound, opts, DisplayData::DATA_MAX);
					}
				}
			}

			if(!minNotFound) cbopts.define(DisplayData::DATA_MIN, datamin);
			if(!maxNotFound) cbopts.define(DisplayData::DATA_MAX, datamax);

			// "dataunit" won't normally be defined as a main DD options field,
			// but the dd_ should send it out via chgdOpts if it has changed,
			// so that the color bar can be labelled correctly.

			String dataunit;
			dd_->readOptionRecord(dataunit, notFound, chgdOpts, "dataunit");
			if(!notFound) {
				if(dataunit=="_") dataunit="";
				cbopts.define("dataunit", dataunit);
			}

			if(colorBar_->setOptions(cbopts, chgdcbopts)) {
				cbNeedsRefresh = True;
			}
		}
		return cbNeedsRefresh;
	}

	void QtDisplayData::setHistogramColorMapping( float minValue, float maxValue, float powerScale  ) {
		Record histRecord;
		Record rangeRecord;
		Vector<float> values( 2 );
		values[0] = minValue;
		values[1] = maxValue;
		rangeRecord.define( "value", values );
		histRecord.defineRecord( PrincipalAxesDD::HISTOGRAM_RANGE, rangeRecord );

		Record powerScaleRecord;
		powerScaleRecord.define( "value", powerScale );
		histRecord.defineRecord(WCPowerScaleHandler::POWER_CYCLES, powerScaleRecord );

		setOptions( histRecord, true );
	}


	void QtDisplayData::setColorBarOrientation_() {
		// Set the color bar orientation option according to the master
		// value stored in the QtViewerBase (panel_->colorBarsVertical_).
		// Connected to QtViewerBase's colorBarOrientationChange() signal;
		// also called during initialization.
		Record orientation;
		orientation.define( "orientation",  panel_->colorBarsVertical()?
		                    "vertical" : "horizontal" );
		setOptions(orientation);
	}

	void QtDisplayData::emitOptionsChanged( Record changedOpts ) {
		emit optionsChanged(changedOpts);
	}

	void QtDisplayData::setColormap_(const String& clrMapName, bool invertChanged ) {
		// Set named colormap onto underlying dd (done publicly via setOptions).
		// Pass "" to remove/delete any existing colormap for the QDD.
		// In the case that no colormap is set on a dd that needs one (raster dds,
		// mainly), the drawing canvas will provide a default.
		// See ColormapDefinition.h, and the Table gui/colormaps/default.tbl (in
		// the data repository) for the list of valid default colormap names (and
		// information on creating/installing custom ones).  If an invalid name is
		// passed, an (ignorable) error message is signalled, and the dd's colormap
		// will remain unchanged.

		if(dd_==0) return;	// (safety)

		if(clrMapName==clrMapName_ && !invertChanged) return;	// (already there)

		if(clrMapName=="") {

			// Return to 'no colormap set' state.  (Does not remove old colormap,
			// if any, from set of DD's defined colormaps (clrMaps_)).

			dd_->removeColormap();
			if(hasColorBar()) colorBar_->removeColormap();

			clrMap_ = 0;
			clrMapName_ = "";
			// emit qddOK();
			return;
		}

		colormapmap::iterator cmiter = clrMaps_.find(clrMapName);
		Colormap* clrMap = (cmiter == clrMaps_.end() ? 0 : cmiter->second);

		if(clrMap==0) {

			// clrMapName not (yet) in set of DD's used colormaps:
			// see if it's a valid name.
			colormapnamemap::iterator cmniter = clrMapNames_.find(clrMapName);
			if ( cmniter != clrMapNames_.end( ) ) {
				// valid name -- create and store corresponding Colormap.
				clrMap = new Colormap(clrMapName);
				clrMaps_.insert(colormapmap::value_type(clrMapName, clrMap));
			}

			if(clrMap==0) {

				errMsg_ = "Invalid colormap name: "+clrMapName;
				// cerr<<"qddErr:"<<errMsg_<<endl;	//#dg
				emit qddError(errMsg_);
				return;
			}
		}
		clrMap->setInvertFlags( invertColorMap, invertColorMap, invertColorMap );

		// clrmap is ok to use.

		// (Colormap classes are kind of a mess under the hood, and
		// surprisingly complex (CM, CMMgr, CMDef, PCClrTables...).
		// E.g., dd_->setColormap() is _supposed_ to clean up after
		// an older Colormap you may have passed it previously, but
		// doesn't always handle it gracefully.  You must surround that
		// call with hold-release() of refresh on every canvas the dd might be
		// registered on.  This is because the process of replacing maps in
		// the PC color tables may lead to refresh/redraws in an intermediate
		// state, in which the dd may try to draw with a colormap which is not
		// really ready to be used yet...).

		panel_->viewer()->hold();

		dd_->setColormap(clrMap, 1.);
		if(hasColorBar()) colorBar_->setColormap(clrMap, 1.);

		// sets value for any currently-open data option panel
		Record cm;
		cm.define( "value", clrMapName );
		Record rec;
		rec.defineRecord(COLOR_MAP,cm);
		emit optionsChanged(rec);


		// sets value for any to-be-opened data option panel
		if ( clrMapOpt_ ) clrMapOpt_->setValue( clrMapName );
		panel_->viewer()->release();

		clrMap_ = clrMap;
		restoreColorMapName = clrMapName_;
		clrMapName_ = clrMapName;

		// emit qddOK();
	}

	void QtDisplayData::setSaturationRange( double min, double max ) {
		Record intensityRecord;
		Record rangeRecord;
		Vector<float> values( 2 );
		values[0] = min;
		values[1] = max;
		rangeRecord.define( "value", values );
		intensityRecord.defineRecord( PrincipalAxesDD::HISTOGRAM_RANGE, rangeRecord );
		setOptions( intensityRecord, true );
	}

	void QtDisplayData::removeColorMap( const String& name ) {
		std::map<String, Colormap*>::iterator iter = clrMaps_.find( name );
		if ( iter != clrMaps_.end() ) {
			clrMaps_.erase( iter );
			clrMapNames_.erase( name );
			delete (*iter).second;

			String nextColorMapName = clrMapName_;
			//We are currently using the one we are removing so we need
			//to use the restore one.
			if ( name == clrMapName_ ) {
				nextColorMapName = restoreColorMapName;
			}

			//Change the color map we are using in this class.
			delete clrMapOpt_;
			clrMapOpt_ = NULL;
			clrMapOpt_ = new DParameterChoice(COLOR_MAP, "Color Map",
			                                  "Name of the mapping from data values to color",
			                                  clrMapNames_, nextColorMapName, nextColorMapName, "");
			setColormap_( nextColorMapName );


			//Notify the QtAutoGui we have made a change in the color map.
			Record options = dd_->getOptions();
			emit optionsChanged( options);
		}
	}

	void QtDisplayData::setColorMap( Colormap* colorMap ) {
		if ( colorMap != NULL ) {
			//Put it into the map, replacing it if it already
			//exists.
			String mapName = colorMap->name();
			if ( mapName.length() == 0 ){
				return;
			}
			std::map<String, Colormap*>::iterator iter = clrMaps_.find( mapName );
			if ( iter != clrMaps_.end() ) {
				clrMaps_.erase( iter );
				delete (*iter).second;
			}
			clrMaps_[mapName] = colorMap;
			clrMapNames_.insert(colormapnamemap::value_type(mapName,true));
			//cout << "Color map names "<<clrMapNames_<<endl;

			//Set the color map we are using internally.
			delete clrMapOpt_;
			clrMapOpt_ = NULL;
			clrMapOpt_ = new DParameterChoice(COLOR_MAP, "Color Map",
			                                  "Name of the mapping from data values to color",
			                                  clrMapNames_, mapName, mapName, "");

			setColormap_(mapName );

			//Notify the auto gui we are making the change.
			Record options = getOptions();
			emit optionsChanged( options );
		}
	}

	bool QtDisplayData::isValidColormap( const QString &name ) const {
		colormapnamemap::const_iterator iter = clrMapNames_.find(name.toAscii().constData());
		return iter != clrMapNames_.end() ? true : false;
	}

// Get/set colormap shift/slope ('fiddle') and brightness/contrast
// settings.  (At present this is usually set for the PC's current
// colormap via mouse tools.  These may want to to into get/setOptions
// (thus into guis) eventually...).  Return value will be False if
// DD has no colormap [at present].

	Bool QtDisplayData::getCMShiftSlope(Vector<Float>& params) const {
		if(!hasColormap()) return False;
		Vector<Float> p = clrMap_->getShapingCoefficients();
		params.resize(p.size());	// (Should be 2.  Usual rigamarole
		params = p;			// around poor Vector operator= design...).
		return True;
	}

	Bool QtDisplayData::getCMBrtCont(Vector<Float>& params) const {
		if(!hasColormap()) return False;
		params.resize(2);
		params[0] = clrMap_->getBrightness();
		params[1] = clrMap_->getContrast();
		return True;
	}

	Bool QtDisplayData::setCMShiftSlope(const Vector<Float>& params) {
		if(!hasColormap()) return False;
		clrMap_->setShapingCoefficients(params);
		return True;
	}

	Bool QtDisplayData::setCMBrtCont(const Vector<Float>& params) {
		if(!hasColormap()) return False;
		clrMap_->setBrightness(params[0]);
		clrMap_->setContrast(params[1]);
		return True;
	}

	Bool QtDisplayData::setColormapAlpha( uInt alpha ){
		bool alphaSet = false;
		if ( hasColormap() ){
			clrMap_->setAlpha( alpha );
			alphaSet = true;
		}
		return alphaSet;
	}

	const viewer::ImageProperties &QtDisplayData::imageProperties( ) {
		if ( image_properties.ok( ) == false )
			image_properties = path_;
		return image_properties;
	}


	void QtDisplayData::unlock( ) {
		if ( im_ != 0 )  im_->unlock( );
		if ( cim_ != 0 ) cim_->unlock( );
	}


	void QtDisplayData::registerNotice(QtDisplayPanel* /*qdp*/) {
		// Called [only] by QtDisplayPanels to notify
		// the QDD that it has been registered there.

		//obs connect( qdp, SIGNAL(rectangleRegionReady(Record)),
		//obs                 SLOT(mouseRectRegionToImageRegion_(Record)) );
		//obs	// (Will pass the signal on as a true Image Region, if
		//obs	// Applicable to this type of DD).

	}

	void QtDisplayData::unregisterNotice(QtDisplayPanel* /*qdp*/) {
		// Called [only] by QtDisplayPanels to notify
		// the QDD that it is about to be unregistered there.

		//obs disconnect( qdp, SIGNAL(rectangleRegionReady(Record)),
		//obs            this,  SLOT(mouseRectRegionToImageRegion_(Record)) );

		// this should perhaps be done elsewhere, but the "unregister"
		// notice is not propagated out of the Qt layer...
		dd()->setDisplayState( DisplayData::UNDISPLAYED );
	}

	/*
	Int QtDisplayData::spectralAxis() {
	  // Return the number of the spectral axis within the DD's original
	  // image lattice and coordinate system (-1 if none).

	  if(dd_==0 || (im_==0 && cim_==0)) return -1;

	  DisplayCoordinateSystem cs= ((im_!=0)? im_->coordinates() : cim_->coordinates());

	  Int nAxes = (im_!=0)? im_->ndim() : cim_->ndim();
	  for(Int ax=0; ax<nAxes && ax<Int(cs->nWorldAxes()); ax++) {
	    Int coordno, axisincoord;
	    cs->findWorldAxis(coordno, axisincoord, ax);

	    //cout << "coordno=" << coordno << endl;
	    if(cs->showType(coordno)=="Spectral") {
	        //if (im_ != 0)
	        //   cout << "shape=" << im_->shape() << endl;
	        //if (cim_ != 0)
	        //   cout << "cshape=" << cim_->shape() << endl;
	        return ax;
	    }

	  }

	  return -1;  }
	 */

// Return the number/index of input axis name, if it is present in the
// DD's original image lattice and coordinate system list (-1 if none).
// Supported axis strings are "Spectral", "Stokes", 'Right Ascension', 'Declination'
// Note : Right Ascension and Declination are both of type 'Direction'.
	Int QtDisplayData::getAxisIndex(String axtype) {

		if(dd_==0 || (im_==0 && cim_==0)) return -1;

		DisplayCoordinateSystem cs = ( im_ != 0 )? im_->coordinates() : cim_->coordinates();

		try {

			Int nAxes = (im_!=0)? im_->ndim() : cim_->ndim();
			for(Int ax=0; ax<nAxes && ax<Int(cs.nWorldAxes()); ax++) {
				// coordno : type of coordinate
				// axisincoord : index within the coordinate list defined by coordno
				Int coordno, axisincoord;
				cs.findWorldAxis(coordno, axisincoord, ax);

				//cout << "coordno=" << coordno << "  axisincoord : " << axisincoord << "  type : " << cs.showType(coordno) << endl;

				if( cs.showType(coordno) == String("Direction") ) {
					// Check for Right Ascension and Declination
					Vector<String> axnames = (cs.directionCoordinate(coordno)).axisNames(MDirection::DEFAULT);
					AlwaysAssert( axisincoord>=0 && axisincoord < static_cast<int>(axnames.nelements()), AipsError);
					if( axnames[axisincoord] == axtype ) {
						return ax;
					}
				} else if(cs.showType(coordno)==axtype) { // Check for Stokes and Spectral
					return ax;
				}
			}//end of for ax
		}//end of try
		catch (...) {
			errMsg_ = "Unknown error converting region ***";
			// cerr<<"mse2ImgReg: "<<errMsg_<<endl;	//#dg
			// emit qddRegnError(errMsg_);
			return -1;
		}

		return -1;
	}// end of getAxisIndex



	ImageRegion* QtDisplayData::mouseToImageRegion(Record mouseRegion,
	        WorldCanvasHolder* wch,
	        Bool allChannels,Bool allPols, Bool allRAs, Bool allDECs,
	        Bool allAxes) {
		// Convert 2-D 'pseudoregion' (or 'mouse region' Record, from the region
		// mouse tools) to a full Image Region, with same dimensionality as the
		// DD's Lattice (and relative to its DisplayCoordinateSystem).
		// Return value is 0 if the conversion can't be made or does not apply
		// to this DD for any reason (ignored by non-Lattice DDs, e.g.).
		//
		// If allChannels==True, the region is extended along spectral axis, if
		// it exists (but ONLY if the spectral axis is not also on display; the
		// visible mouse region always defines region shape on the display axes).
		//
		// If allAxes is True, the region is extended over all (non-display)
		// axes (including any spectral axis; allAxes=True makes the allChannels
		// setting irrelevant).
		//
		// If both allchannels and allAxes are False (the default), the region
		// will be confined to the currently-displayed plane.
		//
		// -->If the returned ImageRegion* is non-zero, the caller
		// -->is responsible for deleting it.


		// (We can't create/return Image regions except for Image PADDs).

		if(dd_==0 || (im_==0 && cim_==0)) return 0;
		PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(dd_);
		if(padd==0) return 0;


		// 'try' instead of all the record testing, as in:
		// if(!rec.isDefined("linear") || rec.dataType("linear")!=TpRecord) return;
		// It also silently exits if unable to compute for other reasons, which is
		// the appropriate response in most cases.  In future, user feedback
		// may want to be provided for some anticipated cases (such as null region
		// or no unmasked data within the region).

		try {

			// Check DD applicability to current panel state.

			if(!padd->conformsTo(*wch->worldCanvas())) return 0;
			// (A side-effect of this is to make the padd->activeZIndex()
			// call below return a value appropriate to the wch and padd...)

			String regionType = mouseRegion.asString("type");
			if( regionType!="box" && regionType!="polygon") return 0;
			// only mouse region types supported at present


			// Gather image, dd and animation state.  (Perform sanity checks).

			Int nAxes                   = (im_!=0)? im_->ndim()  : cim_->ndim();
			IPosition shp               = (im_!=0)? im_->shape() : cim_->shape();
			DisplayCoordinateSystem cs  = (im_!=0)? im_->coordinates() : cim_->coordinates();

			//Int zIndex = mouseRegion.asInt("zindex");  // (following is preferable).
			Int zIndex            = padd->activeZIndex();
			IPosition pos         = padd->fixedPosition();
			std::vector<int> dispAxes  = padd->displayAxes();
			// dispAxes[0] and [1] contain indices of axes on display X and Y;
			// dispAxes[2] also initially contains animation axis (if it exists).

			if(nAxes==2) dispAxes.resize(2);

			if( nAxes<2 ||
			        Int(shp.size()) != nAxes ||
			        Int(pos.size()) != nAxes ||
			        std::for_each(dispAxes.begin(),dispAxes.end(),anylt<int>(0)) ||
			        std::for_each(dispAxes.begin(),dispAxes.end(),anyge<int>(nAxes)) ) {
				return 0;	// (shouldn't happen).
			}

			if( dispAxes.size() > 2 ) pos[dispAxes[2]] = zIndex;
			// (padd->fixedPosition() can't be trusted to have the correct
			// zIndex value on the animation axis; at least, not yet).

			dispAxes.resize(2);		// Now ensure that dispAxes is restricted
			// to just the axes on display X and Y,
			// (for WCPolygon definition, below).

			// unitInit() ensures that the special region Unit "pix" is defined.
			// I'm trying to use the WCBox constructor that requires Quanta to
			// be passed to it -- with "pix" units in this case.  I can't wait
			// till _after_ the ctor is called for those units to be defined!...

			WCBox dummy;		// This does the unitInit(), which is
			// WCBox::unitInit();	// private, but shouldn't be....


			// A box region (WCBox) is created for "box" or "polygon"
			// mouse region types.
			//
			// If a "box" mouse region was passed, its corners are used as
			// the WCBox corners on the display axes.
			//
			// The WCBox restricts to the plane of interest (displayed plane),
			// except if an extension is required.
			// Extensions across an axis are done only _if_ it exists _and_ is not a display axis.
			// - If allAxes is True, the region extends along all non-display axes.
			// Extensions can be done on 'allChannels', 'allPols', 'allRAs',' allDECs' if any of
			// them are non-display axes.
			//
			// For a "polygon" type mouse region, the WCBox is intersected with a
			// WCPolygon which defines the cross-section to use on the display axes.
			// (The WCPolygon by itself would not restrict the non-display axes,
			// if it were applied alone to the n-D image).

			Quantum<Double> px0(0.,"pix");
			Vector<Quantum<Double> >  blcq(nAxes, px0), trcq(nAxes, px0);
			// Establishes "pix" as a default unit (overridden
			// below in some cases).  Initializes blcq to pixel 0.

			// To extend flags along any hidden axis, first get the index of the
			//   coordinate to extend across. (if it exists)
			//   Note : extension axes must be one of the hidden axes.
			Int spaxis = -1;
			if(allChannels) spaxis = getAxisIndex(String("Spectral"));
			Int polaxis = -1;
			if(allPols) polaxis = getAxisIndex(String("Stokes"));
			Int raaxis = -1;
			if(allRAs) raaxis = getAxisIndex(String("Right Ascension"));
			Int decaxis = -1;
			if(allDECs) decaxis = getAxisIndex(String("Declination"));


			for(Int ax = 0; ax<nAxes; ax++) {

				if(ax==dispAxes[0] || ax==dispAxes[1] ||
				        allAxes || ax==spaxis || ax==polaxis || ax==raaxis || ax==decaxis) { // mask extensions
					trcq[ax].setValue(shp[ax]-1);
				}
				// Set full image extent on display axes (may be further
				// restricted below), and on non-display axes if requested.
				else  {
					blcq[ax].setValue(pos[ax]);	    // set one-pixel-wide slice on
					trcq[ax].setValue(pos[ax]);
				}
			}  // other non-displayed axes.


			// Preferable to use world coordinates on display axes, if they exist;
			// otherwise data pixel ('linear') coordinates are used ("Pix" units).

			Bool useWorldCoords = mouseRegion.isDefined("world");

			Record coords = mouseRegion.subRecord(useWorldCoords? "world" : "linear");

			Vector<String> units(2, "pix");
			if(useWorldCoords) units = coords.asArrayString("units");


			// For "box" mouse region, restrict to 2D mouse region on display axes.

			if(regionType=="box") {

				Vector<Double> blc = coords.asArrayDouble("blc"),
				               trc = coords.asArrayDouble("trc");

				for(Int i=0; i<2; i++) {		// display X (0) and Y (1)
					Int ax = dispAxes[i];		// corresp. axis index in image/cs.

					blcq[ax].setValue(blc[i]);
					trcq[ax].setValue(trc[i]);
					// Set range on these axes to mouse region extents.

					if(useWorldCoords) {
						blcq[ax].setUnit(units[i]);		// Override "pix" with
						trcq[ax].setUnit(units[i]);
					}
				}
			}	// proper world units.



			WCBox box(blcq, trcq, cs, Vector<Int>());



			if(regionType=="box") return new ImageRegion(box);
			// For "box" mouse region, we're done here.
			// (NB: caller is responsible for deletion of returned ImageRegion).



			// For "polygon" mouse region, intersect above WCBox with
			// appropriate WCPolygon.

			Vector<Double> x = coords.asArrayDouble("x"),
			               y = coords.asArrayDouble("y");
			Quantum<Vector<Double> > qx(x, units[0]), qy(y, units[1]);
			// (Note that WCBox requires Vector<Quantum>s, whereas
			// WCPolygon wants Quantum<Vector>s...).

			WCPolygon poly(qx, qy, IPosition(dispAxes), cs);
			// (dispAxes tells the WCPolygon which 2 Image/CS
			// axes the polygon applies to).

			WCIntersection flatpoly(poly, box);
			// poly on display axes, 1 data pixel wide on non-display axes
			// (but extended over spectral axis where appropriate).



			return new ImageRegion(flatpoly);

			// (Caller is responsible for deletion of returned ImageRegion).


		}	// (try)


		catch (const casa::AipsError& err) {
			errMsg_ = err.getMesg();
			// cerr<<"mse2ImgReg: "<<errMsg_<<endl;	//#dg
			// emit qddRegnError(errMsg_);
			// To do: use this routine-specific signal (do this in sOpts too)
			return 0;
		}

		catch (...) {
			errMsg_ = "Unknown error converting region";
			// cerr<<"mse2ImgReg: "<<errMsg_<<endl;	//#dg
			// emit qddRegnError(errMsg_);
			return 0;
		}


	}


	Bool QtDisplayData::printLayerStats(ImageRegion& imgReg) {
		// Compute and print statistics on DD's image for
		// given region in all layers.

		//there are several possible path here
		//(1) modify ImageRegion record then create SubImage of 1 plane
		//(2) modify ImageRegion directly then creage SubImage of 1 plane
		//(3) make SubImage of SubImage to get one plane
		//(4) pass layer index to LatticeStatistcis
		//(5) do single plane statistic right here

		if (!isRaster()) return False;

		if(im_==0) return False;

		//cout << "imgReg=" << imgReg.toRecord("") << endl;
		try {

			SubImage<Float> subImg(*im_, imgReg);
			IPosition shp = (im_!=0)? im_->shape() : cim_->shape();
			IPosition sshp = subImg.shape();
			//cout << "subImg.shape()=" << sshp << endl;
			//cout << "im_.shape()=" << shp << endl;

			PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(dd_);
			if (padd == 0)
				return False;

			std::vector<int> dispAxes = padd->displayAxes();
			//cout << "dispAxes=" << dispAxes << endl;

			Vector<Int> cursorAxes(2);
			cursorAxes(0) = dispAxes[0]; //display axis 1
			cursorAxes(1) = dispAxes[1]; //display axis 2
			//cout << "cursorAxes=" << cursorAxes << endl;;

			Int nAxes = (im_!=0)? im_->ndim()  : cim_->ndim();
			Vector<int> otherAxes(0);
			otherAxes = IPosition::otherAxes(nAxes, cursorAxes).asVector();
			//cout << "otherAxes=" << otherAxes << endl;;

			IPosition start(nAxes);
			IPosition stride(nAxes);
			IPosition end(sshp);
			start = 0;
			stride = 1;

			Record rec = dd_->getOptions();
			//cout << "dd=" << rec << endl;
			String zaxis = "";
			String haxis = "";
			Int zIndex = padd->activeZIndex();
			Int hIndex = -1;
			try {
				//String xaxis = rec.subRecord("xaxis").asString("value");
				//String yaxis = rec.subRecord("yaxis").asString("value");
				zaxis = rec.subRecord("zaxis").asString("value");
				haxis = rec.subRecord("haxis1").asString("listname");
				hIndex = rec.subRecord("haxis1").asInt("value");
				//cout << "zaxis=" << zaxis << " haxis=" << haxis << endl;
			} catch(...) {}
			//cout << "zIndex=" << zIndex << " hIndex=" << hIndex << endl;

			String zUnit, zValue;
			try {
				zUnit = rec.subRecord("axislabelspectralunit").asString("value");
			} catch(...) {}


			/*
			if (nAxes == 3 || hIndex == -1) {
			start(otherAxes[0]) = zIndex;
			end(otherAxes[0]) = zIndex;
			}
			if (nAxes == 4 && hIndex != -1) {
			if (otherAxes[0] == dispAxes[2]) {
			   start(otherAxes[0]) = zIndex;
			   end(otherAxes[0]) = zIndex;
			   start(otherAxes[1]) = hIndex;
			   end(otherAxes[1]) = hIndex;
			}
			else {
			   start(otherAxes[0]) = hIndex;
			   end(otherAxes[0]) = hIndex;
			   start(otherAxes[1]) = zIndex;
			   end(otherAxes[1]) = zIndex;
			}
			}

			//cout << "start=" << start << " end=" << end
			//     << " stride=" << stride << endl;

			IPosition cursorShape(2, sshp[dispAxes[0]], sshp[dispAxes[1]]);
			IPosition matAxes(2, dispAxes[0], dispAxes[1]);
			//cout << "cursorShape=" << cursorShape << endl;
			IPosition axisPath(4, dispAxes[0], dispAxes[1],
			                  otherAxes[0], otherAxes[1]);
			//cout << "axisPath=" << axisPath << endl;;
			LatticeStepper stepper(sshp, cursorShape, matAxes, axisPath);
			cout << "stepper ok=" << stepper.ok() << endl;
			//stepper.subSection(start, end, stride);
			//cout << "stepper section ok=" << stepper.ok() << endl;

			RO_LatticeIterator<Float> iterator(subImg, stepper);

			//Vector<Float> spectrum(nFreqs);
			//spectrum = 0.0;
			uInt channel = 0;
			uInt k = -1;
			for (iterator.reset(); !iterator.atEnd(); iterator++) {
			k++;
			Int r = k / sshp[2];
			int s = k % sshp[2];
			cout << "z=" << r << " h=" << s << endl;
			if (r != zIndex || s != hIndex)
			  continue;
			const Matrix<Float>& cursor = iterator.matrixCursor();
			cout << " cursor=" << cursor << endl;
			for (uInt col = 0; col < cursorShape(0); col++) {
			  for (uInt row = 0; row < cursorShape(1); row++) {
			  //spectrum(channel) += cursor(col, row);
			  cout << "col=" << col << " row=" << row
			       << " cursor=" << cursor(col, row) << endl;
			  }
			}
			}


			//Slicer slicer(start, end, stride);
			//SubImage<Float> twoD(subImg, slicer);
			//IPosition shp2 = twoD.shape();
			//cout << "twoD.shape()=" << shp2 << endl;
			 */

			DisplayCoordinateSystem cs = (im_ != 0) ? im_->coordinates() : cim_->coordinates();
			String unit = (im_ != 0) ? im_->units().getName() : cim_->units().getName();

			IPosition pos = padd->fixedPosition();

			ImageStatistics<Float> stats(subImg, False);
			if (!stats.setAxes(cursorAxes)) return False;
			stats.setList(True);
			String layerStats;
			Vector<String> nm = cs.worldAxisNames();
			//cout << "unit=" << zUnit << endl;

			Int zPos = -1;
			Int hPos = -1;
			for (Int k = 0; k < static_cast<int>(nm.nelements()); k++) {
				if (nm(k) == zaxis)
					zPos = k;
				if (nm(k) == haxis)
					hPos = k;
			}
			//cout << "zPos=" << zPos << " hPos=" << hPos << endl;

			String zLabel="";
			String hLabel="";
			Vector<Double> tPix,tWrld;
			tPix = cs.referencePixel();
			String tStr;
			if (zPos > -1) {
				tPix(zPos) = zIndex;
				if (!cs.toWorld(tWrld,tPix)) {
				} else {
					zLabel = ((DisplayCoordinateSystem)cs).format(tStr,
					                                       Coordinate::DEFAULT, tWrld(zPos), zPos);
					zLabel += tStr + "  ";
					if (zUnit.length()>0) {
						zValue = "Spectral_Vale="+((DisplayCoordinateSystem)cs).format(zUnit,
						         Coordinate::DEFAULT, tWrld(zPos), zPos)+zUnit+ " ";
					}
				}
			}
			if (hPos > -1) {
				tPix(hPos) = hIndex;
				//cout << "tPix=" << tPix << endl;
				if (!cs.toWorld(tWrld,tPix)) {
				} else {
					hLabel = ((DisplayCoordinateSystem)cs).format(tStr,
					                                       Coordinate::DEFAULT, tWrld(hPos), hPos);
					hLabel += tStr + "  ";
					if (zUnit.length()>0) {
						zValue = "Spectral_Vale="+((DisplayCoordinateSystem)cs).format(zUnit,
						         Coordinate::DEFAULT, tWrld(zPos), zPos)+zUnit+ " ";
					}
				}
			}
			//cout << "zLabel=" << zLabel << " hLabel=" << hLabel << endl;
			//cout << "tStr=" << tStr << endl;

			Int spInd = cs.findCoordinate(Coordinate::SPECTRAL);
			SpectralCoordinate spCoord;
			Int wSp=-1;
			if (spInd>=0) {
				wSp= (cs.worldAxes(spInd))[0];
				spCoord=cs.spectralCoordinate(spInd);
				spCoord.setVelocity();
				Double vel;
				Double restFreq = spCoord.restFrequency();
				if (downcase(zaxis).contains("freq")) {
					if (spCoord.pixelToVelocity(vel, zIndex)) {
						if (restFreq >0)
							zLabel += "Velocity=" + String::toString(vel)+"km/s  ";
						else
							zLabel += zValue;

						MFrequency::showType(spCoord.frequencySystem()) +
						"  Doppler=" +
						MDoppler::showType(spCoord.velocityDoppler()) +
						"  ";
					}
				}
				if (downcase(haxis).contains("freq")) {
					if (spCoord.pixelToVelocity(vel, hIndex)) {
						if (restFreq >0)
							hLabel += "Velocity=" + String::toString(vel) + "km/s  ";
						else
							hLabel += zValue;

						hLabel += "Frame=" +
						          MFrequency::showType(spCoord.frequencySystem()) +
						          "  Doppler=" +
						          MDoppler::showType(spCoord.velocityDoppler()) +
						          "  ";
					}
				}
			}

			//the position will have the frequency coord in it
			//Vector<Double> lin(2);
			//lin(0) = 1; lin(1) = 1;
			//Vector<Double> wld(2);
			//dd_->linToWorld(wld, lin);
			//cout << "world=" << wld << endl;
			//cout << "Tracking=" << dd_->showPosition(wld) << endl;

			Double beamArea = 0;
			ImageInfo ii = im_->imageInfo();
			GaussianBeam beam = ii.restoringBeam();
			DisplayCoordinateSystem cSys = im_->coordinates();
			String imageUnits = im_->units().getName();
			imageUnits.upcase();

			Int afterCoord = -1;
			Int dC = cSys.findCoordinate(Coordinate::DIRECTION, afterCoord);
			// use contains() not == so moment maps are dealt with nicely
			if (! beam.isNull() && dC!=-1 && imageUnits.contains("JY/BEAM")) {
				DirectionCoordinate dCoord = cSys.directionCoordinate(dC);
				Vector<String> units(2);
				units(0) = units(1) = "rad";
				dCoord.setWorldAxisUnits(units);
				Vector<Double> deltas = dCoord.increment();

				beamArea = beam.getArea("rad2") /
				           abs(deltas(0) * deltas(1));
			}

			std::string head;
			{
				std::string desc = description( );
				int pos = desc.find(" ");
				if (pos != static_cast<int>(std::string::npos)) pos += 1;
				head = desc.substr(pos);
			}

			if (zaxis != "" )
				head += zaxis + "=" + zLabel;
			if (haxis != "" )
				head += haxis + "=" + hLabel;
			head += "BrightnessUnit=" + unit;
			if (beamArea > 0) {
				head += "  BeamArea=" + String::toString(beamArea) + "\n";
			} else {
				head += "  BeamArea=Unknown\n";
			}
			//cout << "head=" << head << endl;
			//cout << "beamArea=" << beamArea
			//     << " zPos=" << zPos << " zIndex=" << zIndex
			//     << " hPos=" << hPos << " hIndex=" << hIndex << endl;
			Bool statsOk =

		    stats.getLayerStats(layerStats, beamArea, zPos, zIndex, hPos, hIndex);
			//cout << layerStats << endl;
			if (!statsOk) {
				cout << "stats not ok" << endl;
				return False;
			}

			layerStats = String(head) + layerStats;

			//cout << "done getLayerStats" << endl ;
			emit statsReady(layerStats);
			return True;

		} catch (const casa::AipsError& err) {
			errMsg_ = err.getMesg();
			//cout << "Error: " << errMsg_ << endl;
			return False;
		} catch (...) {
			errMsg_ = "Unknown error computing region statistics.";
			//cout << "Error: " << errMsg_ << endl;
			return False;
		}

	}


	Bool QtDisplayData::printRegionStats(ImageRegion& imgReg) {

		// Compute and print statistics on DD's image for given region.
		// Current plane only.  Returns False if it can't compute for
		// various reasons.

		if(im_==0) return False;
		// This code only supports QDDs containing an im_
		// (ImageInterface<Float>) at present (Complex is not supported).
		// imgReg should be conpatible with im_.  In most cases, imgReg
		// will have been provided by mouseToImageRegion(), above.

		//cout << "imgReg=" << imgReg.toRecord("") << endl;
		try {


			SubImage<Float> subImg(*im_, imgReg);

			// LogOrigin org("QtDisplayData", "imageStats()", WHERE);
			// LogIO logio(org);	//#dk this LogIO works, goes to std out.
			// ImageStatistics<Float> stats(subImg, logio);	// (verbose version)
			ImageStatistics<Float> stats(subImg, False);


			Vector<Int> cursorAxes(subImg.ndim());
			indgen(cursorAxes);		// cursorAxes = {0,1,2,...n-1}
			if(!stats.setAxes(cursorAxes)) return False;
			// This tells stats to compute just one set of statistics
			// for the entire (sub-)image.


			// Gather robust stats (for now, this is omitted as
			// too time-consuming for more than 10 megapixels).

			Array<Double> nPts;
			stats.getStatistic(nPts, LatticeStatsBase::NPTS);
			Bool computeRobust = nPts.nelements()!=0 && Vector<Double>(nPts)[0]<=10e6;
			if(computeRobust) {
				// This causes all 3 robust stats to be put into sts in the getStats()
				// call below; there's no simpler way to request this (?!)).
				Array<Double> median;	// (not really used...)
				stats.getStatistic(median, LatticeStatsBase::MEDIAN);
			}

			// Retrieve statistics into Vector sts.

			Vector<Double> sts;
			if(!stats.getStats(sts, IPosition(im_->ndim(), 0), True)) {
				// cerr<<"gS "<<stats.errorMessage()<<endl;		//#dg
				return False;
			}


			if(sts.nelements()<stats.NSTATS) return False;

			ostringstream os;
			// ostream& os=cout;

			os.unsetf(ios::scientific | ios::fixed);
			// This is the only way I know to reset numeric style to 'Normal'
			// (i.e., to choose sci. or fixed-pt on a case-by-case basis),
			// in case it had been set previously to one or the other alone.
			//
			// Note: 'ios::scientific' above (an I/O flag) is _not the same_
			// as (unqualified) 'scientific' (an 'I/O manipulator', fed into
			// the ostream, as 'left' is in the code below).  Though they serve
			// the same purpose, they use different syntax:
			// 'os.setf(ios::scientific);' is equivalent to 'os<<std::scientific;'
			//
			// The proper qualifying scope for manipulators is 'std' (std::left,
			// std::scientific, std::noshowpoint, std::endl, etc.).
			// Should you inadvertently try to use ios::scientific as a
			// manipulator, the compiler won't catch it(!); instead, it will
			// cause the next number to come out extremely weird.
			//
			// Also note: if 'Normal' mode _chooses_ scientific, and precision
			// is set at 4, you get 3 places after the decimal, 1 before;
			// if _you_ specify 'scientific', you get 4 places after the decimal,
			// 1 before....
			//
			// Also note: 'scientific' and 'setprecision()' continue to apply,
			// but 'left' (justification) and setw() (min. field width) must
			// be [re]set prior to output of _each_ numeric item.
			//
			// (c++ std. lib formatting is a bit of a pain...).

			using std::left;	// (Just to be 'clear'; unnecessary at this point).


			os<<endl<<endl<<endl<<name();

			String unit = im_->units().getName();	// brightness unit.
			if(unit!="" && unit!=" " && unit!="_") os<<"     ("<<unit<<")";

			os<<endl<<endl
			  <<"n           Std Dev     RMS         Mean        Variance    Sum"<<endl
			  <<setprecision(10)<<noshowpoint
			  <<left<<setw(10)<< sts(stats.NPTS)     <<"  "
			  <<setprecision(4)   <<showpoint
			  // <<scientific    example, here equivalent to '<<std::scientific'
			  <<left<<setw(10)<< sts(stats.SIGMA)    <<"  "
			  <<left<<setw(10)<< sts(stats.RMS)      <<"  "
			  <<left<<setw(10)<< sts(stats.MEAN)     <<"  "
			  <<left<<setw(10)<< sts(stats.VARIANCE) <<"  "
			  <<left<<setw(10)<< sts(stats.SUM)
			  <<endl<<endl;


			os<<"Flux        ";
			if(computeRobust) os<<"Med |Dev|   IntQtlRng   Median      ";
			os<<"Min         Max"<<endl;

			if(sts(stats.FLUX)!=0) os<<left<<setw(10)<< sts(stats.FLUX) <<"  ";
			else                   os<<"(no beam)   ";

			if(computeRobust) os<<left<<setw(10)<<  sts(stats.MEDABSDEVMED) <<"  "
				                    <<left<<setw(10)<<  sts(stats.QUARTILE)     <<"  "
				                    <<left<<setw(10)<<  sts(stats.MEDIAN)       <<"  ";

			os<<left<<setw(10)<<  sts(stats.MIN) <<"  "
			  <<left<<setw(10)<<  sts(stats.MAX)
			  <<endl<<endl<<endl;

			cout<<String(os);


			return True;

		}	// (try)


		catch (const casa::AipsError& err) {
			errMsg_ = err.getMesg();
			// cerr<<"mse2ImgReg: "<<errMsg_<<endl;	//#dg
			// emit qddRegnError(errMsg_);
			// To do: use this routine-specific signal (do this in sOpts too)
			return False;
		}

		catch (...) {
			errMsg_ = "Unknown error computing region statistics.";
			// cerr<<"mse2ImgReg: "<<errMsg_<<endl;	//#dg
			// emit qddRegnError(errMsg_);
			return False;
		}

	}

	String QtDisplayData::getPositionInformation( const Vector<double> world) {
		String valueStr =  dd_->showPosition(world);
		return valueStr;
	}




	pair<String,String> QtDisplayData::trackingInfo(const WCMotionEvent& ev) {
		// Returns a String with value and position information
		// suitable for a cursor tracking display.

		//if(dd_==0) return "";
		pair<string,string> trackingInfo;
		if (dd_==0 ) return trackingInfo;
		try {

			if(!ev.worldCanvas()->inDrawArea( ev.pixX(), ev.pixY() )) return trackingInfo;
			// Don't track motion off draw area (must explicitly test this now
			// (best for caller to test before trying to use this routine).

			dd_->conformsTo(ev.worldCanvas());
			// 'focus' DD on WC[H] of interest (including its zIndex).
			// If DD does not apply to the WC, we need to call showPosition()
			// and showValue() below anyway; the DD will then return the
			// appropriate number of blank lines.

			stringstream ss;
			ss << dd_->showValue(ev.world());

			// if the first string is shorter than a typical value, add spaces...
			if(ss.tellp() < 23) while(ss.tellp() < 23) ss << ' ';
			// ...otherwise add a tab
			else ss << '\t';
			trackingInfo.first = String(ss.str());

			stringstream ss2;
			ss2 << dd_->showPosition(ev.world());
			trackingInfo.second = String(ss2.str());
		} catch (const AipsError &x) {
			trackingInfo.first="";
			trackingInfo.second = "";
		}
		return trackingInfo;
	}




	void QtDisplayData::getInitialAxes_( Block<uInt>& axs, const IPosition& shape,
	                                     const DisplayCoordinateSystem &cs ) {
		// Heuristic used internally to set initial axes to display on X, Y and Z,
		// for PADDs.  (Lifted bodily from GTkDD.  Should refactor down to DD
		//  level, rather than repeat the code.  GTk layer disappearing, though).
		//
		// shape should be that of Image/Array, and have same nelements
		// as axs.  On return, axs[0], axs[1] and (if it exists) axs[2] will be axes
		// to display initially on X, Y, and animator, respectively.
		// If you pass a CS for the image, it will give special consideration to
		// Spectral axes (users expect their Image spectral axes to be on Z).

		// This kludge was designed to prevent large-size axes
		// from being placed on the animator (axs[2]) while small-sized axes
		// are displayed (axs[0], axs[1]) (at least initially).  It was
		// originally a temporary bandaid to keep single-dish data from
		// clogging up the animator with many channels, in msplot.  (see
		// http://aips2.nrao.edu/mail/aips2-visualization/229).
		// (Lifted bodily from GTkDD.  Shame on me--should refactor down to DD
		//  level, rather than repeat the code.  GTk layer disappearing though).

		uInt ndim = axs.nelements();
		for(uInt i = 0; i<ndim; i++) axs[i] = i;

		if(ndim<=2) return;


		Int spaxis = -1;	// axis number of a non-degenerate
		// spectral axis (-1 if none).

		// First, ensure that a non-degenerate Spectral axis is
		// at least on the animator (if not on display).  (Added 8/06)

		for(uInt axno=0; axno<ndim && axno<cs.nWorldAxes(); axno++) {

			Int coordno, axisincoord;
			cs.findWorldAxis(coordno, axisincoord, axno);
			// (It would be convenient if more methods in CS were in
			//  terms of 'axno' rather than 'coordno', so these two
			//  lines didn't constantly have to be repeated...).

			if( cs.showType(coordno)=="Spectral" && shape(axs[axno])>1 ) {
				spaxis = axno;
				if(spaxis>2) {
					axs[spaxis]=2;
					axs[2]=spaxis;
				}
				// Swap spectral axis onto animator.
				break;
			}
		}


		for(uInt i=0; i<3; i++) if(shape(axs[i])<=4 && axs[i]!=uInt(spaxis)) {

				for (uInt j=2; j<ndim; j++)  {
					if (shape(axs[j]) > 4) {
						uInt tmp = axs[i];
						axs[i] = axs[j];
						axs[j] = tmp;		// swap small axes for large.
						break;
					}
				}

				// This part was added (7/05) to prevent degenrerate Stokes axes
				// from displacing small Frequency axes on the animator.
				// (See defect 5148   dk).

				if (shape(axs[i]) == 1) {
					for (uInt j=2; j<ndim; j++)  {
						if (shape(axs[j]) > 1) {
							uInt tmp = axs[i];
							axs[i] = axs[j];
							axs[j] = tmp;
							// swap degenerate axis for (any) non-degenerate axis.
							break;
						}
					}
				}
			}
	}




	Display::DisplayDataType QtDisplayData::ddType() {
		// Possible valuse: Raster, Vector, Annotation, CanvasAnnotation
		if(isEmpty()) return Display::Annotation;	// (anything...)
		return dd_->classType();
	}


	Float QtDisplayData::colorBarLabelSpaceAdj() {
		// Used (by QtDisplayPanel) to compute margin space for colorbar labels.
		// It is the (pgplot) character size for colorbar labels (default 1.2)
		// times a label space 'manual adjustment' user option (default 1.).

		if(!wouldDisplayColorBar()) return 0.;

		Float charSz = 1.2;		// (Should be reset by next 3 lines).
		Bool notFound;
		Record cbOpts = colorBar_->getOptions();
		colorBar_->readOptionRecord(charSz, notFound, cbOpts,
		                            WEDGE_LABEL_CHAR_SIZE);

		charSz = max(0., min(10.,  charSz));

		Float spAdj = max(colorBarLabelSpaceOpt_->minimum(),
		                  min(colorBarLabelSpaceOpt_->maximum(),
		                      colorBarLabelSpaceOpt_->value()));

		return charSz * spAdj;
	}


	ImageRegion* QtDisplayData::mouseToImageRegion(
	    Record mouseRegion, WorldCanvasHolder* wch,
	    String& extChan, String& /*extPol*/) {

		if (dd_ == 0 || (im_ == 0 && cim_ == 0))
			return 0;

		PrincipalAxesDD* padd =
		    dynamic_cast<PrincipalAxesDD*>(dd_);

		if (padd==0)
			return 0;

		try {

			if (!padd->conformsTo(*wch->worldCanvas()))
				return 0;

			String regionType = mouseRegion.asString("type");
			if (regionType != "box" && regionType != "polygon")
				return 0;

			Int nAxes = (im_!=0)? im_->ndim()  : cim_->ndim();
			IPosition shp = (im_!=0)? im_->shape() : cim_->shape();
			DisplayCoordinateSystem cs = (im_ != 0) ? im_->coordinates() : cim_->coordinates();

			Int zIndex = padd->activeZIndex();
			IPosition pos = padd->fixedPosition();
			std::vector<int> dispAxes = padd->displayAxes();

			if (nAxes == 2)
				dispAxes.resize(2);

			if (nAxes < 2 || Int(shp.nelements()) != nAxes ||
			        Int(pos.nelements()) != nAxes ||
			        std::for_each(dispAxes.begin(),dispAxes.end(),anylt<int>(0)) ||
			        std::for_each(dispAxes.begin(),dispAxes.end(),anyge<int>(nAxes)) ) {
				return 0;
			}

			if ( dispAxes.size() > 2 )
				pos[dispAxes[2]] = zIndex;

			dispAxes.resize(2);

			WCBox dummy;

			Quantum<Double> px0(0.,"pix");
			Vector<Quantum<Double> >
			blcq(nAxes, px0), trcq(nAxes, px0);

			Int spaxis = -1;
			if (extChan.length() == 0)
				spaxis = getAxisIndex(String("Spectral"));

			for (Int ax = 0; ax < nAxes; ax++) {
				if (ax == dispAxes[0] || ax == dispAxes[1] ||
				        extChan.length() == 0 || ax == spaxis) {
					trcq[ax].setValue(shp[ax]-1);
				} else  {
					blcq[ax].setValue(pos[ax]);
					trcq[ax].setValue(pos[ax]);
				}
			}

			Bool useWorldCoords = mouseRegion.isDefined("world");
			Record coords = mouseRegion.subRecord(
			                    useWorldCoords ? "world" : "linear");

			Vector<String> units(2, "pix");
			if(useWorldCoords) units = coords.asArrayString("units");

			if (regionType=="box") {
				Vector<Double> blc = coords.asArrayDouble("blc"),
				               trc = coords.asArrayDouble("trc");

				for (Int i = 0; i < 2; i++) {
					Int ax = dispAxes[i];

					blcq[ax].setValue(blc[i]);
					trcq[ax].setValue(trc[i]);

					if (useWorldCoords) {
						blcq[ax].setUnit(units[i]);
						trcq[ax].setUnit(units[i]);
					}
				}
			}

			WCBox box(blcq, trcq, cs, Vector<Int>());

			if (regionType=="box")
				return new ImageRegion(box);

			Vector<Double> x = coords.asArrayDouble("x"),
			               y = coords.asArrayDouble("y");
			Quantum<Vector<Double> >
			qx(x, units[0]), qy(y, units[1]);

			WCPolygon poly(qx, qy, IPosition(dispAxes), cs);
			return new ImageRegion(poly);

		} catch (const casa::AipsError& err) {
			errMsg_ = err.getMesg();
			return 0;
		} catch (...) {
			errMsg_ = "Unknown error converting region";
			return 0;
		}


	}

} //# NAMESPACE CASA - END

