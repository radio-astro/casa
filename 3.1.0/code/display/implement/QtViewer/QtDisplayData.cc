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
#include <display/QtAutoGui/QtXmlRecord.h>
#include <display/DisplayDatas/DisplayData.h>
#include <display/DisplayDatas/MSAsRaster.h>
#include <images/Images/ImageInterface.h>
#include <display/DisplayDatas/LatticeAsRaster.h>
#include <display/DisplayDatas/LatticeAsContour.h>
#include <display/DisplayDatas/LatticeAsVector.h>
#include <display/DisplayDatas/LatticeAsMarker.h>
#include <display/DisplayDatas/SkyCatOverlayDD.h>
#include <casa/OS/Path.h>
#include <images/Images/PagedImage.h>
#include <images/Images/FITSImage.h>
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



namespace casa { //# NAMESPACE CASA - BEGIN


QtDisplayData::QtDisplayData(QtDisplayPanelGui *panel, String path,
			     String dataType, String displayType) :
	       panel_(panel), 
	       path_(path),
	       dataType_(dataType),
	       displayType_(displayType),
	       im_(0),
	       cim_(0),
	       dd_(0),
	       clrMapName_(""),
	       clrMap_(0),
	       clrMapOpt_(0),
	       colorBar_(0),
	       colorBarDisplayOpt_(0),
	       colorBarThicknessOpt_(0),
	       colorBarLabelSpaceOpt_(0),
	       colorBarOrientationOpt_(0),
	       colorBarCharSizeOpt_(0)  {

  if(dataType=="lel") { 
    name_ = path_;
    if(name_.length()>25) name_ =  path_.before(15) + "..." +
				   path_.from(path_.length()-7);  }
  else name_ = Path(path_).baseName();
  
  if(displayType!="raster") name_ += "-"+displayType_;
	// Default; can be changed with setName()
	// (and should, if it duplicates another name).
    
  
  String stdErrMsg = "Cannot display " + name_ + " as ";
  if(displayType_=="skycatalog") stdErrMsg += "skycatalog";
  else stdErrMsg += displayType_ + " " + dataType_;
  
  errMsg_ = "";
    
  try {
    
    if (displayType_=="skycatalog") {
      dd_ = new SkyCatOverlayDD(path);
      if (dd_==0) throw(AipsError("Couldn't create skycatalog"));  }
    

      
    else if(dataType_=="ms" && displayType_=="raster") {
      dd_ = new MSAsRaster(path_);  }
      
      

    else if(dataType_=="image" || dataType_=="lel") {
      
      if(dataType_=="image") {

        switch(ImageOpener::imageType(path)) {


	  case ImageOpener::AIPSPP: {
	  
	    if(imagePixelType(path_)==TpFloat) {
	      im_  = new PagedImage<Float>
	                 (path_, TableLock::AutoNoReadLocking);  }

	    else if(imagePixelType(path_)==TpComplex) {
	      cim_ = new PagedImage<Complex>
		         (path_, TableLock::AutoNoReadLocking);  }

	    else  throw AipsError("Only Float and Complex CASA images "
			          "are supported at present.");
	    break;  }

  
	  case ImageOpener::FITS: {
	    im_ = new FITSImage(path);
	    break;  }


	  case ImageOpener::MIRIAD: {
	    im_ = new MIRIADImage(path);
	    break;  }


	  default: { 
          
	    File f(path);
	    if(!f.exists()) throw AipsError("File not found.");

	    throw AipsError("Only casa, FITS and MIRIAD images "
			  "are supported.");  }  }


        if(im_==0 && cim_==0) throw AipsError("Couldn't create image.");  }
		// (Needed?)

      else {
      
        // Parse LEL expression to create expression-type ImageInterface.
	
        LatticeExprNode expr = ImageExprParse::command(path_);
          
	if(expr.dataType() == TpFloat) {
          im_ = new ImageExpr<Float>(LatticeExpr<Float>(expr), name_);  }
          
	else if(expr.dataType() == TpComplex) {
          cim_ = new ImageExpr<Complex>(LatticeExpr<Complex>(expr), name_);  }
         
	else throw AipsError("Only Float or Complex LEL expressions "
			     "are allowed");  }
      
 

      uInt ndim = (im_!=0)? im_->ndim() : cim_->ndim();
      if(ndim<2) throw AipsError("Image doesn't have >=2 dimensions.");
	// (Probably won't happen).

      IPosition shape = (im_!=0)? im_->shape() : cim_->shape();
      Block<uInt> axs(ndim);


      
      const CoordinateSystem* cs=0;
      try { cs = &((im_!=0)? im_->coordinates() : cim_->coordinates());  }
		// (Is there perhaps a problem retrieving
		//  a CS from FITS or miriad images?)
      catch(...) { cs = 0;  }	// (In case there is).
      
      getInitialAxes_(axs, shape, cs);
      //#dk  getInitialAxes(axs, shape);


      IPosition fixedPos(ndim);
      fixedPos = 0;
      
        
      
      if(displayType_=="raster") {
      
        if(im_!=0) {
          if(ndim ==2) dd_ = new LatticeAsRaster<Float>(im_, 0, 1);
          else dd_ = new LatticeAsRaster<Float>(im_, axs[0], axs[1], axs[2],
					        fixedPos);  }
	else {
          if(ndim ==2) dd_ = new LatticeAsRaster<Complex>(cim_, 0, 1);
          else dd_ = new LatticeAsRaster<Complex>(cim_, axs[0], axs[1],
					          axs[2], fixedPos);  }  }
    
      else if(displayType_=="contour") {
      
        if(im_!=0) {
          if(ndim ==2) dd_ = new LatticeAsContour<Float>(im_, 0, 1);
          else dd_ = new LatticeAsContour<Float>(im_, axs[0], axs[1], axs[2],
					        fixedPos);  }
	else {
          if(ndim ==2) dd_ = new LatticeAsContour<Complex>(cim_, 0, 1);
          else dd_ = new LatticeAsContour<Complex>(cim_, axs[0], axs[1],
					          axs[2], fixedPos);  }  }
    
      else if(displayType_=="vector") {
      
        if(im_!=0) {
          if(ndim ==2) dd_ = new LatticeAsVector<Float>(im_, 0, 1);
          else dd_ = new LatticeAsVector<Float>(im_, axs[0], axs[1], axs[2],
					        fixedPos);  }
	else {
          if(ndim ==2) dd_ = new LatticeAsVector<Complex>(cim_, 0, 1);
          else dd_ = new LatticeAsVector<Complex>(cim_, axs[0], axs[1],
					          axs[2], fixedPos);  }  }
    
      else if(displayType_=="marker") {
      
        if(im_!=0) {
          if(ndim ==2) dd_ = new LatticeAsMarker<Float>(im_, 0, 1);
          else dd_ = new LatticeAsMarker<Float>(im_, axs[0], axs[1], axs[2],
					        fixedPos);  }
	else {
          if(ndim ==2) dd_ = new LatticeAsMarker<Complex>(cim_, 0, 1);
          else dd_ = new LatticeAsMarker<Complex>(cim_, axs[0], axs[1],
					          axs[2], fixedPos);  }  }
    
      else throw AipsError("Unsupported image display type: "
	                     + displayType_);


      
      if(im_!=0) im_->unlock();
      else      cim_->unlock();  }
	// Needed (for unknown reasons) to avoid
	// blocking other users of the image file....
	// (11/07 -- locking mode changed; may no longer be necessary...).

    
    
    else throw AipsError("Unsupported data type: " + dataType_);  }
 
 
       
  // (failure.. Is it best to try to delete the remains, or leave
  // them hanging?  Latter course chosen here...).
  
  catch (const AipsError& err) {
    errMsg_ = stdErrMsg;
    if(err.getMesg()!="") errMsg_ += ":\n  " + err.getMesg();
    else                  errMsg_ += ".";
    dd_=0;  }
  
  catch (...) { 
    errMsg_ = stdErrMsg + ".";
    dd_=0;  }
    
  
  if(dd_==0) {
    emit qddError(errMsg_);
	// (error signal propagated externally, rather than throw.
	// Alternatively, caller can test newQdd->isEmpty()).
    im_=0;
    cim_=0;
    return;  }
  

      
  // Successful creation.
  
  dd_->setUIBase(0);  
	// Items are numbered from zero in casaviewer as consistently
	// as possible (including, e.g., axis 'pixel' positions).  This call
	// is necessary after constructing DDs, to orient them away from
	// their old default 1-based (glish) behavior.
 
  
  
  // Initialize colormap if necessary.
  
  if(usesClrMap_()) {
  
    // Check .aipsrc, otherwise use "Hot Metal 1" by default initially.
        
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
    
    
    clrMapOpt_ = new DParameterChoice("colormap", "Colormap",
                 "Name of the mapping from data values to color",
                 clrMapNames_, initialCMName, initialCMName, "");  }
	// (For parsing user colormap selection via
	// getOptions() / setOptions() / Options gui).

  
  // Initialization for color bar, if necessary.
  
  if(usesColorBar_() && clrMap_!=0) {
    
    colorBar_ = new WedgeDD;
    colorBar_->setColormap(clrMap_, 1.);

    Vector<String> yesNo(2);     yesNo[0]="Yes";
                                 yesNo[1]="No";
    
    Vector<String> vertHor(2);   vertHor[0]="vertical";
                                 vertHor[1]="horizontal";
    
    String orientation = panel_->colorBarsVertical()? vertHor[0] : vertHor[1];
    
    colorBarDisplayOpt_ = new DParameterChoice("wedge",
            "Display Color Wedge?",
            "Whether to display a 'color bar' that indicates\n"
	    "the current mapping of colors to data values.",
            yesNo, yesNo[1], yesNo[1], "Color_Wedge");
    
    colorBarThicknessOpt_ = new DParameterRange<Float>("wedgethickness",
            "Wedge Thickness",
            "Manual adjustment factor for thickness of colorbar.\n"
	    "Vary this if automatic thickness choice is not satisfactory.",
	    .3, 5.,  .1,  1., 1.,  "Color_Wedge");
    
    colorBarLabelSpaceOpt_ = new DParameterRange<Float>("wedgelabelspace",
            "Wedge Label Space",
	    "Manual adjustment factor for colorbar's label space.\n"
	    "Vary this if automatic margin choice is not satisfactory.",
	    .1, 2.,  .05,  1., 1.,  "Color_Wedge");
    
    colorBarOrientationOpt_ = new DParameterChoice("orientation",
            "Wedge Orientation",
            "Whether to display color bars vertically or horizontally.\n"
	    "(Note: orientation will be the same for all color bars\n"
	    "in all display panels).",
            vertHor, orientation, orientation, "Color_Wedge");
      
    // (This one is just used to monitor changes to the "wedgelabelcharsize"
    // parameter during setOptions().  getOptions() uses the values
    // from colorBar_ (the WedgeDD)).
    colorBarCharSizeOpt_ = new DParameterRange<Float>(
            "wedgelabelcharsize", "Character size", "",
	    0.2, 4.,  .05,  1.2, 1.2, "Color_Wedge");
    
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
    
    dd_->readOptionRecord(minMax, notFound, mainDDopts, "minmaxhist");
	// (DisplayOptions methods like readOptionRecord() should be
	// static -- they're stateless.  Any DisplayOptions object (or
	// a newly-created one) would do here instead of dd_, which
	// just happened to be on hand...).
    
    if(minMax.nelements()==2) {
      cbropts.define("datamin", minMax[0]);
      cbropts.define("datamax", minMax[1]);  }
    else {
      dd_->readOptionRecord(datamin, notFound, mainDDopts, "datamin");
      if(!notFound) cbropts.define("datamin", datamin);
      dd_->readOptionRecord(datamax, notFound, mainDDopts, "datamax");
      if(!notFound) cbropts.define("datamax", datamax);  }

    dd_->readOptionRecord(powercycles, notFound, mainDDopts, "powercycles");
    if(!notFound) cbropts.define("powercycles", powercycles);
      
    String dataunit = "";
    try { dataunit = dd_->dataUnit().getName();  }  catch(...) {  }
	// (try because LaticePADD::dataUnit() is (stupidly) willing
	// to throw up instead of returning Unit("") as a fallback...)    
    if(dataunit=="_") dataunit="";
    cbropts.define("dataunit", dataunit);
    
    
    colorBar_->setOptions(cbropts, chgdopts);
    
    
    connect( panel_, SIGNAL(colorBarOrientationChange()),
                   SLOT(setColorBarOrientation_()) );

    connect(this, SIGNAL(statsReady(const String&)),
            panel_,  SLOT(showStats(const String&)));
      
    setColorBarOrientation_();  }  }






QtDisplayData::~QtDisplayData() { 
  done();  }

String QtDisplayData::description( ) const {
  return name_ + " (" + path_ + ") " + dataType_ + "/" + displayType_;
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
    clrMapOpt_=0;  }
    
  if(hasColorBar()) {
    delete colorBar_;
    
    delete colorBarDisplayOpt_;
    delete colorBarThicknessOpt_;
    delete colorBarLabelSpaceOpt_;
    delete colorBarOrientationOpt_;
    delete colorBarCharSizeOpt_;  }
  
  delete dd_;  dd_=0;
  if(im_!=0)  { delete im_;  im_=0;  }
  if(cim_!=0) { delete cim_; cim_=0;  }  }  

  
    
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
    opts.removeField("histoequalisation");  }
    
  
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
    
    opts.mergeField(cbopts, "wedgelabelcharsize",
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
      if(fldname.before(5)=="wedge" && fldname!="wedgelabelcharsize") {
        opts.mergeField(cbopts, i, Record::OverwriteDuplicates);
	opts.rwSubRecord(fldname).define("dlformat", fldname);  }  }  }


  return opts;  }

  
  
  
void QtDisplayData::checkAxis() {
   //cout << "checkAxis" << endl;
   Record rec = getOptions();
   //cout << "dd=" << rec << endl;
   try {
       String xaxis = rec.subRecord("xaxis").asString("value");
       String yaxis = rec.subRecord("yaxis").asString("value");
       String zaxis = rec.subRecord("zaxis").asString("value");
       emit axisChanged(xaxis, yaxis, zaxis);
       Int haxis = rec.subRecord("haxis1").asInt("value");
       emit axisChanged4(xaxis, yaxis, zaxis, haxis);
   }
   catch(...) {
   } 
}      

  

void QtDisplayData::setOptions(Record opts, Bool emitAll) {
  // Apply option values to the DisplayData.  Method will
  // emit optionsChanged() if other option values, limits, etc.
  // should also change as a result.
  // Set emitAll = True if the call was not initiated by the options gui
  // itself (e.g. via scripting or save-restore); that will assure that
  // the options gui does receive all option updates (via the optionsChanged
  // signal) and updates its user interface accordingly.
  

  if(dd_==0) return;  // (safety, in case construction failed.).

  // dump out information about the set-options message flow...
//cout << "\t>>== " << (emitAll ? "true" : "false") << "==>> " << opts << endl;

  Record chgdOpts;
  
  Bool held=False;
  
  try { 
  
    Bool needsRefresh = dd_->setOptions(opts, chgdOpts);
    Bool cbNeedsRefresh = False;
    
    if(usesClrMap_() && clrMapOpt_->fromRecord(opts)) {
      needsRefresh = cbNeedsRefresh = True;
      setColormap_(clrMapOpt_->value());  }
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
        if( (fldname.before(5)=="wedge" && fldname!="wedge") ||
	     fldname=="orientation" || fldname=="dataunit"   ||
	     fldname=="powercycles" ) {		// (Forward these verbatum).

          cbopts.mergeField(opts, i, Record::OverwriteDuplicates);  }  }
      
      
      // Priority for datamin and datamax definition: 
      //   1) "minmaxhist"          in chgdOpts  (internal request)
      //   2) "datamin", "datamax"  in chgdOpts  (internal request)
      //   3) "minmaxhist"          in opts      (user request)
      //   4) "datamin", "datamax"  in opts      (user request)
    
      Float datamin=0., datamax=1.;
      Vector<Float> minMax;
      Bool notFound=True, minNotFound=True, maxNotFound=True;

      dd_->readOptionRecord(minMax, notFound, chgdOpts, "minmaxhist");
      if(minMax.nelements()==2) {
        datamin = minMax[0];   minNotFound=False;
        datamax = minMax[1];   maxNotFound=False;  }
      
      else {
        
        dd_->readOptionRecord(datamin, minNotFound, chgdOpts, "datamin");
        dd_->readOptionRecord(datamax, maxNotFound, chgdOpts, "datamax");
    
        minMax.resize(0);
        dd_->readOptionRecord(minMax, notFound, opts, "minmaxhist");
        if(minMax.nelements()==2) {
          if(minNotFound) { datamin = minMax[0];  minNotFound=False;  }
          if(maxNotFound) { datamax = minMax[1];  maxNotFound=False;  }  }
        
	else {
          if(minNotFound) {
            dd_->readOptionRecord(datamin, minNotFound, opts,
	                         "datamin");  }
          if(maxNotFound) { 
            dd_->readOptionRecord(datamax, maxNotFound, opts,
	                         "datamax");  }  }  }
      
      
      if(!minNotFound) cbopts.define("datamin", datamin);
      if(!maxNotFound) cbopts.define("datamax", datamax);

    
      // "dataunit" won't normally be defined as a main DD options field,
      // but the dd_ should send it out via chgdOpts if it has changed,
      // so that the color bar can be labelled correctly.
    
      String dataunit;
      dd_->readOptionRecord(dataunit, notFound, chgdOpts, "dataunit");
      if(!notFound) {
        if(dataunit=="_") dataunit="";
        cbopts.define("dataunit", dataunit);  }
      
    
    
      
      if(colorBar_->setOptions(cbopts, chgdcbopts)) cbNeedsRefresh = True;
	// Beware: WedgeDD::setOptions() (unexpectedly, stupidly) alters
	// cbopts, instead of keeping all its hacks internal.
	// Don't expect the original cbopts after this call....
	// (Note: chgdcbopts is ignored (unused) for colorbar).
    
    
    
      // Test for changes in these, recording any new values.
      // Of these, WedgeDD processes only "wedgelabelcharsize"
      // (colorBarCharSizeOpt_ -- it was merged into cbopts, above).
      
      Bool reorient = colorBarOrientationOpt_->fromRecord(opts);
      Bool cbChg    = colorBarDisplayOpt_->fromRecord(opts);	// "wedge"
      Bool cbSzChg  = colorBarThicknessOpt_->fromRecord(opts);
           cbSzChg  = colorBarCharSizeOpt_->fromRecord(opts)   || cbSzChg;
           cbSzChg  = colorBarLabelSpaceOpt_->fromRecord(opts) || cbSzChg;
          

      held=True;   panel_->viewer()->hold();
	// (avoids redrawing more often than necessary)
      
      
      // Trigger color bar and main panel rearrangement, if necessary.
      
      if(reorient) {
	colorBarOrientationOpt_->toRecord(chgdOpts, True, True);
		// Make sure user interface sees this change via chgdOpts.
        Bool orientation = (colorBarOrientationOpt_->value()=="vertical");
        
	panel_->setColorBarOrientation(orientation);  }
    
      else if(cbChg || (wouldDisplayColorBar() && cbSzChg)) {
        
	emit colorBarChange();  }
    
    
      // Trigger redraw of color bar, if necessary.
      
      if(cbNeedsRefresh) colorBar_->refresh();  }

    
    
    if(!held) { held=True;  panel_->viewer()->hold();  }
    
    
    // Refresh all main canvases where dd_ is registered, if required
    // because of option changes (it usually is).
    // Note: the 'True' parameter to refresh() is a  sneaky/kludgy
    // part of the refresh cycle interface which is easily missed.
    // In practice what it means now is that DDs on the PrincipalAxesDD
    // branch get their drawlist cache cleared.  It has no effect
    // (on caching or otherwise) for DDs on the CachingDD branch.
     
    if(needsRefresh)   dd_->refresh(True);

    
    held=False;  panel_->viewer()->release();     
    
    errMsg_ = "";		// Just lets anyone interested know that
    emit optionsSet();  }	// options were set ok.  (QtDDGui will
				// use it to clear status line, e.g.).

  catch (const casa::AipsError& err) {
    errMsg_ = err.getMesg();
    //cerr<<"qdd setOpts Err:"<<errMsg_<<endl;	//#dg
    if(held) { held=False;  panel_->viewer()->release();  }
    emit qddError(errMsg_);  }
  
  catch (...) { 
    errMsg_ = "Unknown error setting data options";
    //cerr<<"qdd setOpts Err:"<<errMsg_<<endl;	//#dg
    if(held) { held=False;  panel_->viewer()->release();  }
    emit qddError(errMsg_);  }

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

  if(emitAll) chgdOpts.merge(opts, Record::SkipDuplicates);
	// When emitAll==True this assures that the options gui
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
  if(chgdOpts.nfields()!=0) emit optionsChanged(chgdOpts);
    
  if(chgdOpts.isDefined("trackingchange")) emit trackingChange(this);  }
	// Tells QDP to gather and emit tracking data again.   Field
	// value is irrelevant. (There are other useful places where
	// DDs could define this field....)




void QtDisplayData::setColorBarOrientation_() {
  // Set the color bar orientation option according to the master
  // value stored in the QtViewerBase (panel_->colorBarsVertical_).
  // Connected to QtViewerBase's colorBarOrientationChange() signal;
  // also called during initialization.
  Record orientation;
  orientation.define( "orientation",  panel_->colorBarsVertical()?
                                      "vertical" : "horizontal" );
  setOptions(orientation);  }

void QtDisplayData::emitOptionsChanged( Record changedOpts ) {
    emit optionsChanged(changedOpts);
}

void QtDisplayData::setColormap_(const String& clrMapName) {
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
  
  if(clrMapName==clrMapName_) return;	// (already there)
  
  if(clrMapName=="") {
    
    // Return to 'no colormap set' state.  (Does not remove old colormap,
    // if any, from set of DD's defined colormaps (clrMaps_)).
    
    dd_->removeColormap();
    if(hasColorBar()) colorBar_->removeColormap();
    
    clrMap_ = 0;
    clrMapName_ = "";
    // emit qddOK();
    return;  }
    
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
      return;  }  }

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
  rec.defineRecord("colormap",cm);
  emit optionsChanged(rec);
  // sets value for any to-be-opened data option panel
  if ( clrMapOpt_ ) clrMapOpt_->setValue( clrMapName );
  
  panel_->viewer()->release();

    
  clrMap_ = clrMap;
  clrMapName_ = clrMapName;

  // emit qddOK();
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
  return True;  }

Bool QtDisplayData::getCMBrtCont(Vector<Float>& params) const {
  if(!hasColormap()) return False;
  params.resize(2);
  params[0] = clrMap_->getBrightness();
  params[1] = clrMap_->getContrast();
  return True;  }

Bool QtDisplayData::setCMShiftSlope(const Vector<Float>& params) {
  if(!hasColormap()) return False;
  clrMap_->setShapingCoefficients(params);
  return True;  }

Bool QtDisplayData::setCMBrtCont(const Vector<Float>& params) {
  if(!hasColormap()) return False;
  clrMap_->setBrightness(params[0]);
  clrMap_->setContrast(params[1]);
  return True;  }


void QtDisplayData::unlock( ) {
    if ( im_ != 0 ) im_->unlock( );
    if ( cim_ != 0 ) cim_->unlock( );
}


void QtDisplayData::registerNotice(QtDisplayPanel* qdp) {
  // Called [only] by QtDisplayPanels to notify
  // the QDD that it has been registered there.
  
  //obs connect( qdp, SIGNAL(rectangleRegionReady(Record)),
  //obs                 SLOT(mouseRectRegionToImageRegion_(Record)) );
  //obs	// (Will pass the signal on as a true Image Region, if
  //obs	// Applicable to this type of DD).

}

void QtDisplayData::unregisterNotice(QtDisplayPanel* qdp) {
  // Called [only] by QtDisplayPanels to notify
  // the QDD that it is about to be unregistered there.
  
  //obs disconnect( qdp, SIGNAL(rectangleRegionReady(Record)),
  //obs            this,  SLOT(mouseRectRegionToImageRegion_(Record)) );

}



Int QtDisplayData::spectralAxis() {
  // Return the number of the spectral axis within the DD's original
  // image lattice and coordinate system (-1 if none).
  
  if(dd_==0 || (im_==0 && cim_==0)) return -1;
  
  const CoordinateSystem* cs=0;
  try { cs = &((im_!=0)? im_->coordinates() : cim_->coordinates());  }
  catch(...) { cs = 0;  }	// (necessity of try-catch is doubtful...).
  if(cs==0) return -1;
    
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



ImageRegion* QtDisplayData::mouseToImageRegion(Record mouseRegion,
                                               WorldCanvasHolder* wch,
					       Bool allChannels,
					       Bool allAxes) {
  // Convert 2-D 'pseudoregion' (or 'mouse region' Record, from the region
  // mouse tools) to a full Image Region, with same dimensionality as the
  // DD's Lattice (and relative to its CoordinateSystem).
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
    
    if(!padd->conformsTo(*wch)) return 0;
	// (A side-effect of this is to make the padd->activeZIndex()
	// call below return a value appropriate to the wch and padd...)
    
    String regionType = mouseRegion.asString("type");
    if( regionType!="box" && regionType!="polygon") return 0;
	// only mouse region types supported at present

    
    // Gather image, dd and animation state.  (Perform sanity checks).
    
    Int nAxes                   = (im_!=0)? im_->ndim()  : cim_->ndim();
    IPosition shp               = (im_!=0)? im_->shape() : cim_->shape();
    const CoordinateSystem& cs  = (im_!=0)? im_->coordinates() : 
					    cim_->coordinates();
					   
  //Int zIndex = mouseRegion.asInt("zindex");  // (following is preferable).
    Int zIndex            = padd->activeZIndex();
    IPosition pos         = padd->fixedPosition();
    Vector<Int> dispAxes  = padd->displayAxes();
	// dispAxes[0] and [1] contain indices of axes on display X and Y;
	// dispAxes[2] also initially contains animation axis (if it exists).
    
    if(nAxes==2) dispAxes.resize(2, True);
    
    if( nAxes<2 ||
        Int(shp.nelements()) != nAxes ||
        Int(pos.nelements()) != nAxes ||
        anyLT(dispAxes, 0) ||
	anyGE(dispAxes, nAxes) ) return 0;	// (shouldn't happen).
    
    if(dispAxes.nelements()>2u) pos[dispAxes[2]] = zIndex;
	// (padd->fixedPosition() can't be trusted to have the correct
	// zIndex value on the animation axis; at least, not yet).
    
    dispAxes.resize(2, True);	// Now assure that dispAxes is restricted
				// to just the axes on display X and Y,
				// (for WCPolygon definition, below).
          
    // unitInit() assures that the special region Unit "pix" is defined.
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
    // except that if allChannels==True, it also extends along the
    // spectral axis (_if_ it exists _and_ is not a display axis); or if
    // allAxes is True, the region extends along all non-display axes. 
    // 
    // For a "polygon" type mouse region, the WCBox is intersected with a
    // WCPolygon which defines the cross-section to use on the display axes.
    // (The WCPolygon by itself would not restrict the non-display axes,
    // if it were applied alone to the n-D image).
    
    Quantum<Double> px0(0.,"pix");
    Vector<Quantum<Double> >  blcq(nAxes, px0), trcq(nAxes, px0);
		// Establishes "pix" as a default unit (overridden
		// below in some cases).  Initializes blcq to pixel 0.

    Int spaxis = -1;
    if(allChannels) spaxis = spectralAxis();
	// Retrieve number of spectral axis (if any), for allChannels case.
      
    for(Int ax = 0; ax<nAxes; ax++) {
      
      if(ax==dispAxes[0] || ax==dispAxes[1] || allAxes || ax==spaxis) {
        trcq[ax].setValue(shp[ax]-1);  } 
		// Set full image extent on display axes (may be further
		// restricted below), and on non-display axes if requested.
      else  {
        blcq[ax].setValue(pos[ax]);	    // set one-pixel-wide slice on
        trcq[ax].setValue(pos[ax]);  }   }  // other non-displayed axes.
    
  
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
	  trcq[ax].setUnit(units[i]);  }  }  }	// proper world units.

    
    
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
    return 0;  }
  
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

  if (displayType_!="raster") return False;

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

    Vector<Int> dispAxes = padd->displayAxes();
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
    }
    catch(...){}
    //cout << "zIndex=" << zIndex << " hIndex=" << hIndex << endl;
    
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

    const CoordinateSystem& cs = 
      (im_ != 0) ? im_->coordinates() : cim_->coordinates();
    String unit =  
      (im_ != 0) ? im_->units().getName() : cim_->units().getName();

    IPosition pos = padd->fixedPosition();

    ImageStatistics<Float> stats(subImg, False);
    if (!stats.setAxes(cursorAxes)) return False;
    stats.setList(True);
    String layerStats;
    Vector<String> nm = cs.worldAxisNames();
    //cout << "nm=" << nm << endl;

    Int zPos = -1;
    Int hPos = -1;
    for (Int k = 0; k < nm.nelements(); k++) {
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
          zLabel = ((CoordinateSystem)cs).format(tStr, 
                 Coordinate::DEFAULT, tWrld(zPos), zPos);
          zLabel += tStr + "  ";
       }
    }
    if (hPos > -1) {
       tPix(hPos) = hIndex;
       //cout << "tPix=" << tPix << endl;
       if (!cs.toWorld(tWrld,tPix)) {
       } else {
          hLabel = ((CoordinateSystem)cs).format(tStr, 
                 Coordinate::DEFAULT, tWrld(hPos), hPos);
          hLabel += tStr + "  ";
       }
    }
    //cout << "zLabel=" << zLabel << " hLabel=" << hLabel << endl;
    //cout << "tStr=" << tStr << endl;

    Int spInd = cs.findCoordinate(Coordinate::SPECTRAL);
    SpectralCoordinate spCoord;
    Int wSp=-1;
    if (spInd>=0){
       wSp= (cs.worldAxes(spInd))[0];
       spCoord=cs.spectralCoordinate(spInd);
       spCoord.setVelocity();
       Double vel;
       if (downcase(zaxis).contains("freq")) {
          if (spCoord.pixelToVelocity(vel, zIndex)) {
             zLabel += "Velocity=" + String::toString(vel) + 
                  "km/s  Frame=" + 
                  MFrequency::showType(spCoord.frequencySystem()) + 
                  "  Doppler=" +
                  MDoppler::showType(spCoord.velocityDoppler()) + 
                  "  ";
          } 
       }
       if (downcase(haxis).contains("freq")) {
          if (spCoord.pixelToVelocity(vel, hIndex)) {
             hLabel += "Velocity=" + String::toString(vel) + 
                  "km/s  Frame=" + 
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
   Vector<Quantum<Double> > beam = ii.restoringBeam();
   CoordinateSystem cSys = im_->coordinates();
   String imageUnits = im_->units().getName();
   imageUnits.upcase();

   Int afterCoord = -1;
   Int dC = cSys.findCoordinate(Coordinate::DIRECTION, afterCoord);
   // use contains() not == so moment maps are dealt with nicely
   if (beam.nelements()==3 && dC!=-1 && imageUnits.contains("JY/BEAM")) {
      DirectionCoordinate dCoord = cSys.directionCoordinate(dC);
      Vector<String> units(2);
      units(0) = units(1) = "rad";
      dCoord.setWorldAxisUnits(units);
      Vector<Double> deltas = dCoord.increment();

      Double major = beam(0).getValue(Unit("rad"));
      Double minor = beam(1).getValue(Unit("rad"));
      beamArea = C::pi/(4*log(2)) * major * minor / 
                 abs(deltas(0) * deltas(1));
    }

    String head = description().after(" ") + "\n";   
    if (zaxis != "" )
        head += zaxis + "=" + zLabel;  
    if (haxis != "" )
        head += haxis + "=" + hLabel; 
    head += "BrightnessUnit=" + unit;
    if (beamArea > 0) {
        head += "  BeamArea=" + String::toString(beamArea) + "\n"; 
    }
    else {
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



    layerStats = head + layerStats;

    //cout << "done getLayerStats" << endl ;
    emit statsReady(layerStats);
    return True;

  }
  catch (const casa::AipsError& err) {
    errMsg_ = err.getMesg();
    //cout << "Error: " << errMsg_ << endl;
    return False;  
  }
  catch (...) { 
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
      stats.getStatistic(median, LatticeStatsBase::MEDIAN);  }
    
    // Retrieve statistics into Vector sts.
    
    Vector<Double> sts;
    if(!stats.getStats(sts, IPosition(im_->ndim(), 0), True)) {
      // cerr<<"gS "<<stats.errorMessage()<<endl;		//#dg
      return False;  }
    
    
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
    return False;  }
  
  catch (...) { 
    errMsg_ = "Unknown error computing region statistics.";
    // cerr<<"mse2ImgReg: "<<errMsg_<<endl;	//#dg
    // emit qddRegnError(errMsg_);
    return False;  }
    
}





String QtDisplayData::trackingInfo(const WCMotionEvent& ev) {
  // Returns a String with value and position information
  // suitable for a cursor tracking display.
  
  if(dd_==0) return "";
  
  try {
    
    if(!ev.worldCanvas()->inDrawArea( ev.pixX(), ev.pixY() )) return "";
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
    
    ss << dd_->showPosition(ev.world());
    
    return String(ss.str());  } 
  
  
  catch (const AipsError &x) { return "";  }  }

 

  
void QtDisplayData::getInitialAxes_(Block<uInt>& axs, const IPosition& shape,
				    const CoordinateSystem* cs) {
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
  if(cs!=0) {
    
    // First, assure that a non-degenerate Spectral axis is
    // at least on the animator (if not on display).  (Added 8/06)
    
    for(uInt axno=0; axno<ndim && axno<cs->nWorldAxes(); axno++) {
      
      Int coordno, axisincoord;
      cs->findWorldAxis(coordno, axisincoord, axno);
      // (It would be convenient if more methods in CS were in
      //  terms of 'axno' rather than 'coordno', so these two
      //  lines didn't constantly have to be repeated...).

      if( cs->showType(coordno)=="Spectral" && shape(axs[axno])>1 ) {
	spaxis = axno;
	if(spaxis>2) { axs[spaxis]=2; axs[2]=spaxis;  }
	// Swap spectral axis onto animator.
	break;  }  }  }
    
    
  
  for(uInt i=0; i<3; i++) if(shape(axs[i])<=4 && axs[i]!=uInt(spaxis)) {
      
    for (uInt j=2; j<ndim; j++)  {
      if (shape(axs[j]) > 4) {
	uInt tmp = axs[i]; 
	axs[i] = axs[j]; 
	axs[j] = tmp;		// swap small axes for large.
	break;  }  }
	   
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
	  break;  }  }  }  }  }




Display::DisplayDataType QtDisplayData::ddType() {
  // Possible valuse: Raster, Vector, Annotation, CanvasAnnotation
  if(isEmpty()) return Display::Annotation;	// (anything...)
  return dd_->classType();  }
  

Float QtDisplayData::colorBarLabelSpaceAdj() {
  // Used (by QtDisplayPanel) to compute margin space for colorbar labels.
  // It is the (pgplot) character size for colorbar labels (default 1.2)
  // times a label space 'manual adjustment' user option (default 1.).
    
  if(!wouldDisplayColorBar()) return 0.;
    
  Float charSz = 1.2;		// (Should be reset by next 3 lines).
  Bool notFound;
  Record cbOpts = colorBar_->getOptions();
  colorBar_->readOptionRecord(charSz, notFound, cbOpts,
				"wedgelabelcharsize");
    
  charSz = max(0., min(10.,  charSz));
    
  Float spAdj = max(colorBarLabelSpaceOpt_->minimum(),
                min(colorBarLabelSpaceOpt_->maximum(),
	            colorBarLabelSpaceOpt_->value()));

  return charSz * spAdj;  }


ImageRegion* QtDisplayData::mouseToImageRegion(
     Record mouseRegion, WorldCanvasHolder* wch,
     String& extChan, String& extPol) {
  
  if (dd_ == 0 || (im_ == 0 && cim_ == 0)) 
     return 0;

  PrincipalAxesDD* padd = 
     dynamic_cast<PrincipalAxesDD*>(dd_);

  if (padd==0) 
     return 0;
  
  try {  
    
    if (!padd->conformsTo(*wch)) 
       return 0;
    
    String regionType = mouseRegion.asString("type");
    if (regionType != "box" && regionType != "polygon") 
       return 0;

    Int nAxes = (im_!=0)? im_->ndim()  : cim_->ndim();
    IPosition shp = (im_!=0)? im_->shape() : cim_->shape();
    const CoordinateSystem& cs = 
      (im_ != 0) ? im_->coordinates() : cim_->coordinates();
					   
    Int zIndex = padd->activeZIndex();
    IPosition pos = padd->fixedPosition();
    Vector<Int> dispAxes = padd->displayAxes();

    if (nAxes == 2) 
       dispAxes.resize(2, True);
    
    if (nAxes < 2 || Int(shp.nelements()) != nAxes ||
        Int(pos.nelements()) != nAxes ||
        anyLT(dispAxes, 0) || anyGE(dispAxes, nAxes)) 
       return 0;
    
    if (dispAxes.nelements() > 2u) 
       pos[dispAxes[2]] = zIndex;
    
    dispAxes.resize(2, True);
    
    WCBox dummy;
    
    Quantum<Double> px0(0.,"pix");
    Vector<Quantum<Double> >  
         blcq(nAxes, px0), trcq(nAxes, px0);

    Int spaxis = -1;
    if (extChan.length() == 0) 
       spaxis = spectralAxis();
      
    for (Int ax = 0; ax < nAxes; ax++) {
      if (ax == dispAxes[0] || ax == dispAxes[1] || 
          extChan.length() == 0 || ax == spaxis) {
        trcq[ax].setValue(shp[ax]-1);  
      } 
      else  {
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
    
  }
  catch (const casa::AipsError& err) {
    errMsg_ = err.getMesg();
    return 0;  
  }
  catch (...) { 
    errMsg_ = "Unknown error converting region";
    return 0;
  }
  

}

} //# NAMESPACE CASA - END

