//# PlotMSPlot.cc: High level plot concept across potentially multiple objects.
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
//# $Id: $
#include <plotms/Plots/PlotMSPlot.h>

#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <plotms/Data/CacheFactory.h>
#include <plotms/Data/PlotMSCacheBase.h>
#include <plotms/Data/MSCache.h>
#include <plotms/Data/CalCache.h>
#include <QDebug>


namespace casa {

////////////////////////////
// PLOTMSPLOT DEFINITIONS //
////////////////////////////

// Static //

PlotMSPlotParameters PlotMSPlot::makeParameters(PlotMSApp* plotms) {
    PlotMSPlotParameters p(plotms->getPlotFactory());
    makeParameters(p, plotms);
    return p;    
}

void PlotMSPlot::makeParameters(PlotMSPlotParameters& params, PlotMSApp* /*plotms*/) {
    // Add data parameters if needed.
    if(params.typedGroup<PMS_PP_MSData>() == NULL)
        params.setGroup<PMS_PP_MSData>();

    // Add cache parameters if needed
    if(params.typedGroup<PMS_PP_Cache>() == NULL)
        params.setGroup<PMS_PP_Cache>();

    // Add axes parameters if needed.
    if(params.typedGroup<PMS_PP_Axes>() == NULL)
        params.setGroup<PMS_PP_Axes>();

    // Add canvas parameters if needed.
    if(params.typedGroup<PMS_PP_Canvas>() == NULL)
        params.setGroup<PMS_PP_Canvas>();

    // Add display parameters if needed.
    if(params.typedGroup<PMS_PP_Display>() == NULL)
        params.setGroup<PMS_PP_Display>();

    // Add iteration parameters if needed.
    if(params.typedGroup<PMS_PP_Iteration>() == NULL)
        params.setGroup<PMS_PP_Iteration>();
}

const uInt PlotMSPlot::PIXEL_THRESHOLD = 1000000;
const uInt PlotMSPlot::MEDIUM_THRESHOLD = 10000;
const uInt PlotMSPlot::LARGE_THRESHOLD = 1000;

// Constructors/Destructors //

PlotMSPlot::PlotMSPlot(PlotMSApp* parent) : 
  itsParent_(parent),
  itsFactory_(parent->getPlotFactory()),
  itsParams_(itsFactory_),
  itsCache_(NULL),
  iter_(0),
  iterStep_(1) { 
  
  itsCache_ = new MSCache(itsParent_);
  cacheUpdating = false;
  constructorSetup();
}

PlotMSPlot::~PlotMSPlot() {
    
    // Clean up plots
    // detachFromCanvases();

  if (itsCache_)
    delete itsCache_;
  itsCache_=NULL;
}

void PlotMSPlot::customizeAutoSymbol( const PlotSymbolPtr& baseSymbol, uInt dataSize ){
	if( baseSymbol->symbol() == PlotSymbol::AUTOSCALING) {

		if( dataSize > PIXEL_THRESHOLD ) {
			baseSymbol->setSymbol(PlotSymbol::PIXEL);
		    baseSymbol->setSize(1,1);
		}
		else if( dataSize > MEDIUM_THRESHOLD ) {
		    baseSymbol->setSymbol( PlotSymbol::CIRCLE);
		    baseSymbol->setSize(2,2);
		}
		else if( dataSize > LARGE_THRESHOLD ) {
		    baseSymbol->setSymbol( PlotSymbol::CIRCLE );
		    baseSymbol->setSize(4,4);
		}
		else {
		    baseSymbol->setSymbol( PlotSymbol::CIRCLE );
		    baseSymbol->setSize(6,6);
		}
	}
}

// Public Methods //

void PlotMSPlot::resize(PlotMSPages &pages, uInt rows, uInt cols) {
	// Resize canvases and plots
	int plotCanvasRowCount = 1;
	int plotCanvasColCount = 1;

	if ( isIteration()  ){
		plotCanvasRowCount = rows;
		plotCanvasColCount = cols;
	}

	//Number of canvases is based on the grid.
	itsCanvases_.resize( plotCanvasRowCount );
	for( int r = 0; r < plotCanvasRowCount; ++r) {
		itsCanvases_[r].resize(plotCanvasColCount);
	}

	//Number of plots is based on how many overplots we
	//are supporting (dataCount) and on the iteration count
	//over the data.
	Int plotRows = 1;
	Int plotCols = 1;
	getPlotSize( plotRows, plotCols );
	resizePlots( plotRows, plotCols );


	// Resize pages
	iterStep_ = plotCanvasRowCount * plotCanvasColCount;
	int ownedCanvasStart = this->getPageIterationCount( pages[0]);
	int pageSize = static_cast<int>(ceil( (plotCols*1.0f+ownedCanvasStart) / iterStep_));
	pages.resize( pageSize );
	for(size_t i = 0; i < pages.totalPages(); ++i) {
		pages[i].resize(rows, cols);
	}

}

String PlotMSPlot::name() const {
	const PMS_PP_MSData *data = itsParams_.typedGroup<PMS_PP_MSData>();
	const PMS_PP_Cache *cache = itsParams_.typedGroup<PMS_PP_Cache>();
	const PMS_PP_Display *display = itsParams_.typedGroup<PMS_PP_Display>();

	if(data == NULL || cache == NULL || display == NULL || !data->isSet())
		return "Over Plot";
	return display->titleFormat().getLabel(cache->xAxis(), cache->yAxis());
	//return "Over Plot for " + data->filename();
}

vector<MaskedScatterPlotPtr> PlotMSPlot::plots() const {
	if((itsPlots_.size() == 0) || (itsPlots_[0].size() == 0))
		return vector<MaskedScatterPlotPtr>();
	int index = 0;

	int dataCount = itsPlots_.size();
	int nIter = itsPlots_[0].size();

	int plotCount = nIter * dataCount;
	vector<MaskedScatterPlotPtr> v( plotCount );
	for(unsigned int i = 0; i < itsPlots_.size(); i++) {
		for(unsigned int j = 0; j < itsPlots_[i].size(); j++) {
			if(index >= plotCount) break;
			v[index] = itsPlots_[i][j];
			++index;
		}
	}
	return v;
}

vector<PlotCanvasPtr> PlotMSPlot::canvases() const {

	if(( itsCanvases_.size() == 0) || (itsCanvases_[0].size() == 0)){
		return vector<PlotCanvasPtr>();
	}
	uInt index = 0;
	uInt nIter = itsCache_->nIter(0);
	int canvasCount = std::min(nIter, uInt(itsCanvases_.size() * itsCanvases_[0].size()));
	vector<PlotCanvasPtr> v( canvasCount );
	for(uInt i = 0; i < itsCanvases_.size(); i++) {
		for(uInt j = 0; j < itsCanvases_[i].size(); j++) {
			if(index >= nIter) break;
			v[index] = itsCanvases_[i][j];
			++index;
		}
	}
	return v;
}

void PlotMSPlot::attachToCanvases() {
	Int nIter = itsCache_->nIter(0);
	if ( nIter <= 0 ){
		nIter = 1;
	}
	int canvasRows = itsCanvases_.size();
	for( int r = 0; r < canvasRows; ++r) {
		int canvasCols = itsCanvases_[r].size();
		for( int c = 0; c < canvasCols; ++c) {
			if(!itsCanvases_[r][c].null()) {
				if ( ! isIteration() ){
					//There is just one canvas for this plot, but we may
					//be adding several sets of data to it.
					int dataRowCount = itsPlots_.size();
					for ( int i = 0; i < dataRowCount; i++ ){
						int dataColCount = itsPlots_[i].size();
						for ( int j = 0; j < dataColCount; j++ ){
							itsCanvases_[r][c]->plotItem( itsPlots_[i][j]);
						}
					}
				}
				else {
					QList<PlotMSPlot*> canvasPlots = itsParent_->getPlotManager().getCanvasPlots(r,c);
					if ( canvasPlots.contains( this )){
						//For an iteration plot, there is one canvas per iteration.
						//In the case of overplotting with an iteration, we may be
						//adding several sets of data to each canvas.
						PlotMSPage page = itsParent_->getPlotManager().itsPages_.currentPage();
						int iterationIndex = getIterationIndex (r, c, page );
						//int iterationIndex = r * canvasCols + c + iter;
						if ( iterationIndex < nIter ){
							int dataRowCount = itsPlots_.size();
							for ( int i = 0; i < dataRowCount; i++ ){
								if(!itsPlots_[i][iterationIndex].null()) {
									itsCanvases_[r][c]->plotItem(itsPlots_[i][iterationIndex]);
								}
							}
						}
					}
				}
				((&*itsCanvases_[r][c]))->show();
				((&*itsCanvases_[r][c]))->setMinimumSize(5,5);
			}
		}
	}
}

void PlotMSPlot::detachFromCanvases() {
	for(uInt r = 0; r < itsCanvases_.size(); ++r) {
		for(uInt c = 0; c < itsCanvases_[r].size(); ++c) {
			if(!itsCanvases_[r][c].null()) {
				if(itsCanvases_[r][c]->numPlotItems() > 0) {
					int dataRowCount = itsPlots_.size();
					for ( int i = 0; i < dataRowCount; i++ ){
						int dataColCount = itsPlots_[i].size();
						for ( int j = 0; j < dataColCount; j++ ){
							itsCanvases_[r][c]->removePlotItem( itsPlots_[i][j]);
						}
					}
				}
				//This is necessary in scripting mode so that we don't see
				//detached canvases.
				if ( ! itsParent_->guiShown() || itsCanvases_[r][c]->numPlotItems() ==0 ){
					((&*itsCanvases_[r][c]))->hide();
				}

			}
		}
	}
}

void PlotMSPlot::dataMissing(){
	cacheUpdating = false;
	detachFromCanvases();
	initializePlot();
	releaseDrawing();
	itsCache_->clear();
}

vector<PMS::DataColumn> PlotMSPlot::getCachedData(){
	PMS_PP_Cache* cache = itsParams_.typedGroup<PMS_PP_Cache>();
	int xAxisCount = cache->numXAxes();
	int yAxisCount = cache->numYAxes();
	int count = xAxisCount + yAxisCount;
	vector<PMS::DataColumn> cdata( count );
	for( int i = 0; i < xAxisCount; ++i) {
		cdata[i] = cache->xDataColumn(i);
	}
	for( int i = xAxisCount; i < count; ++i) {
	    cdata[i] = cache->yDataColumn(i - xAxisCount);
	}
	return cdata;
}


vector<PMS::Axis> PlotMSPlot::getCachedAxes() {
	PMS_PP_Cache* c = itsParams_.typedGroup<PMS_PP_Cache>();
	int xAxisCount = c->numXAxes();
	int yAxisCount = c->numYAxes();
	int count = xAxisCount + yAxisCount;
	vector<PMS::Axis> axes( count );
	for(int i = 0; i < xAxisCount; i++){
		axes[i] = c->xAxis(i);
	}
	for(int i = xAxisCount; i < count; i++){
		uInt yIndex = i - xAxisCount;
	    axes[i] = c->yAxis(yIndex);
	}
	return axes;
}

const PlotMSPlotParameters& PlotMSPlot::parameters() const{ return itsParams_;}

PlotMSPlotParameters& PlotMSPlot::parameters() { return itsParams_; }

vector<PlotCanvasPtr> PlotMSPlot::visibleCanvases() const {
    vector<PlotCanvasPtr> v;
    vector<PlotCanvasPtr> canv = canvases();
                         // visCanv= itsParent_->getPlotter()->currentCanvases();
    //bool found = false;
    for(unsigned int i = 0; i < canv.size(); i++) {
        /*found = false;
        for(unsigned int j = 0; !found && j < visCanv.size(); j++)
            if(canv[i] == visCanv[j]) found = true;
        if(found) v.push_back(canv[i]);*/
    	bool visible = itsParent_->isVisible( canv[i] );
    	if ( visible ){
    		v.push_back( canv[i] );
    	}
    }
    return v;
}

Record PlotMSPlot::locateInfo(int plotIterIndex, const Vector<PlotRegion>& regions,
  		bool showUnflagged, bool showFlagged, bool selectAll ) const {
	Record resultRecord = itsCache_->locateInfo( plotIterIndex, regions,
			showUnflagged, showFlagged, selectAll );
	return resultRecord;
}

PlotLogMessage* PlotMSPlot::locateRange( int canvasIndex, const Vector<PlotRegion> & regions,
   		bool showUnflagged, bool showFlagged){
	int iterIndex = iter() + canvasIndex;
	PlotLogMessage* m = itsCache_->locateRange(iterIndex, regions,showUnflagged,showFlagged);
	return m;
}

PlotLogMessage* PlotMSPlot::flagRange( int canvasIndex, casa::PlotMSFlagging& flagging,
   		const Vector<PlotRegion>& regions, bool showFlagged){
	int iterIndex = iter() + canvasIndex;
	PlotLogMessage* m = itsCache_->flagRange(iterIndex, flagging, regions,showFlagged);
	return m;
}



PlotMSRegions PlotMSPlot::selectedRegions() const {
    return selectedRegions(canvases()); }

PlotMSRegions PlotMSPlot::visibleSelectedRegions() const {
    return selectedRegions(visibleCanvases()); }

bool PlotMSPlot::initializePlot(PlotMSPages& pages) {

    bool hold = allDrawingHeld();
    if(!hold){
    	holdDrawing();
    }

    // Initialize plot objects and assign canvases.
    if(!assignCanvases(pages) || !initializePlot()) {
        if(!hold){
        	releaseDrawing();
        }
        return false;
    }

    // Set up page.
    pages.setupCurrentPage();

    // Attach plot objects to their assigned canvases.
    attachToCanvases();
    
    // Update objects with set parameters.
    parameters().releaseNotification();
    parametersHaveChanged(parameters(),
            PlotMSWatchedParameters::ALL_UPDATE_FLAGS());
    
    // Draw if necessary.
    if(!hold) releaseDrawing();
    
    return true;
}

bool PlotMSPlot::updateCache() {
	PMS_PP_MSData* data = itsParams_.typedGroup<PMS_PP_MSData>();
	PMS_PP_Cache* cache = itsParams_.typedGroup<PMS_PP_Cache>();
	PMS_PP_Iteration* iter = itsParams_.typedGroup<PMS_PP_Iteration>();
	if(data == NULL || cache == NULL || iter == NULL){
		return false;
	}

	// Don't load if data isn't set or there was an error during data opening.
	if(!data->isSet()){
		return false;
	}
	// Trap bad averaging/iteration combo
	if (data->averaging().baseline() &&
			iter->iterationAxis()==PMS::ANTENNA) {
		logMessage( "Cannot iterate on Antenna if averaging over baseline, so turning off iteration.");
		iter->setIterationAxis(PMS::NONE);
	}

	// Notify the plots that the data will change
	updatePlots();

	// Set up cache loading parameters
	if(cache->numXAxes() != cache->numYAxes()){
		return false;
	}



	itsParent_->getLogger()->markMeasurement(PMS::LOG_ORIGIN,
			PMS::LOG_ORIGIN_LOAD_CACHE,
			PMS::LOG_EVENT_LOAD_CACHE);
	itsTCLParams_.endCacheLog = true;

	// Delete existing cache if it doesn't match
	if (CacheFactory::needNewCache(itsCache_, data->filename())) {
		if(itsCache_) {
			delete itsCache_;
			itsCache_ = NULL;
		}
		itsCache_ = CacheFactory::getCache(data->filename(), itsParent_);
		if(itsCache_ == NULL) {
			throw AipsError("Failed to create a new Cache object!");
		}
	}

	bool result = true;
	try {
		cacheUpdating = true;
		result = itsParent_->updateCachePlot( this,
			PlotMSPlot::cacheLoaded, true );
			
	}
	catch( AipsError& error ){
		cacheUpdating = false;
		logMessage( error.getMesg().c_str() );
		result = false;
	}
	return result;
}

bool PlotMSPlot::updateCanvas() {

	bool set = PMS_PP_RETCALL(itsParams_, PMS_PP_MSData, isSet, false);
	PMS_PP_Axes *axes = itsParams_.typedGroup<PMS_PP_Axes>();
	PMS_PP_Cache *cache = itsParams_.typedGroup<PMS_PP_Cache>();
	PMS_PP_Canvas *canv = itsParams_.typedGroup<PMS_PP_Canvas>();
	PMS_PP_Iteration *iter = itsParams_.typedGroup<PMS_PP_Iteration>();
	if(axes == NULL || cache == NULL || canv == NULL || iter == NULL ) {
		return false;
	}

	uInt nIter = itsCache_->nIter(0);
	uInt rows = itsCanvases_.size();
	for(uInt r = 0; r < rows; ++r) {
		uInt cols = itsCanvases_[r].size();
		//uInt iterationRows = iter_ + r * cols;
		/*if( iterationRows >= nIter  ){
			break;
		}*/
		for(uInt c = 0; c < cols; ++c) {
			PlotMSPage page = itsParent_->getPlotManager().itsPages_.currentPage();
			uInt iteration = getIterationIndex( r, c, page );
			if(iteration >= nIter  ){
				clearCanvasProperties( r, c );
			}
			else {
				setCanvasProperties( r, c, cache, axes, set, canv,
						rows, cols, iter, iteration );
			}
		}
	}
	return true;
}

bool PlotMSPlot::updateDisplay() {
	try {
		PMS_PP_Cache *cache = itsParams_.typedGroup<PMS_PP_Cache>();
		PMS_PP_Axes *axes = itsParams_.typedGroup<PMS_PP_Axes>();
		PMS_PP_Display *display = itsParams_.typedGroup<PMS_PP_Display>();
		if(cache == NULL || axes == NULL || display == NULL) return false;
		MaskedScatterPlotPtr plot;
		int nIter = itsCache_->nIter(0);
		if ( nIter <= 0 ){
			nIter = 1;
		}
		uInt rows = itsPlots_.size();
		for(uInt row = 0; row < rows; ++row) {
			uInt cols = itsPlots_[row].size();
			//uInt iter = iter_ + row * cols;
			//if(iter >= nIter) break;

			for(uInt col = 0; col < cols; ++col) {
				//if(iter >= nIter) break;
				// Set symbols.
				PlotSymbolPtr unflaggedSym = display->unflaggedSymbol(row);
				PlotSymbolPtr symbolUnmasked = itsParent_->createSymbol ( unflaggedSym );
				uInt dataSize = itsCache_->indexer(row,col).sizeUnmasked();
				customizeAutoSymbol( symbolUnmasked, dataSize );

				PlotSymbolPtr flaggedSym = display->flaggedSymbol(row);
				PlotSymbolPtr symbolMasked = itsParent_->createSymbol ( flaggedSym  );
				dataSize = itsCache_->indexer(row,col).sizeMasked();
				customizeAutoSymbol( symbolMasked, dataSize );

				plot = itsPlots_[row][col];
				if(plot.null()) continue;

				plot->setSymbol(symbolUnmasked);
				plot->setMaskedSymbol(symbolMasked);
				// Colorize and set data changed, if redraw is needed

				bool colorizeChanged = itsCache_->indexer(row,col).colorize(display->colorizeFlag(), display->colorizeAxis());

				if(nIter > 0 && colorizeChanged ) {
					plot->dataChanged();
				}

				// Set item axes
				plot->setAxes(axes->xAxis(row), axes->yAxis(row));

				// Set plot title
				PMS::Axis x = cache->xAxis(row);
				PMS::Axis y = cache->yAxis(row);
				vector<PMS::Axis> yAxes(1, y);
				vector<bool> yRefs(1, itsCache_->hasReferenceValue(y));
				vector<double> yRefValues(1, itsCache_->referenceValue(y));
				plot->setTitle(display->titleFormat().getLabel(
						x, yAxes,
						itsCache_->hasReferenceValue(x),
						itsCache_->referenceValue(x),
						yRefs, yRefValues ));

				//++iter;
			}
		}
	} catch(AipsError &err) {
		String errorMsg = "Could not update plot: " + err.getMesg();
		qDebug() << errorMsg.c_str();
		itsParent_->showError( errorMsg );
		return false;
	} catch(...) {
		String errorMsg = "Could not update plot, for unknown reasons!";
		qDebug() << errorMsg.c_str();
		itsParent_->showError( errorMsg );
		return false;
	}
	return true;
}

void PlotMSPlot::setColors() {
	uInt nIter = itsCache_->nIter(0);
	uInt rows = itsPlots_.size();
	itsColoredPlots_.resize(rows);
	for(uInt row = 0; row < rows; ++row) {
		uInt cols = itsPlots_[row].size();
		itsColoredPlots_[row].resize(cols);
		for(uInt col = 0; col < cols; ++col) {
			uInt iteration = row * cols + col;
			if(iteration >= nIter) break;
			itsColoredPlots_[row][col] = ColoredPlotPtr(
					dynamic_cast<ColoredPlot*>(&*itsPlots_[row][col]), false);
			if(!itsColoredPlots_[row][col].null()) {
				const vector<String> &colors = PMS::COLORS_LIST();
				for(uInt i = 0; i < colors.size(); ++i) {
					itsColoredPlots_[row][col]->setColorForBin(i ,itsFactory_->color(colors[i]));
				}
			} else {
				std::cout << "Could not convert plot (" << row << ", " << col
						<< ") into a ColoredPlot" << std::endl;
				itsParent_->showError("Could not convert a plot in a ColoredPlot");
			}
		}
	}
}

bool PlotMSPlot::updateData() {
	itsCache_->clear();
	return True;
};

void PlotMSPlot::clearCanvases() {
	int rowCount = itsCanvases_.size();
	for ( int i = 0; i < rowCount; i++ ){
		int colCount = itsCanvases_[i].size();
		for ( int j = 0; j < colCount; j++ ){
			clearCanvasProperties( i, j );
		}
	}
}

bool PlotMSPlot::isCacheUpdating() const {
	return cacheUpdating;
}

void PlotMSPlot::setCacheUpdating( bool updating ){
	cacheUpdating = updating;
}

void PlotMSPlot::updatePlots() {
	for(uInt row = 0; row < itsPlots_.size(); ++row) {
		for(uInt col = 0; col < itsPlots_[row].size(); ++col) {
			bool plottable = itsParent_->getPlotManager().isPlottable( this );
			if(!itsPlots_[row][col].null() && plottable ) {
				itsPlots_[row][col]->dataChanged();
			}
		}
	}
}

bool PlotMSPlot::updateIndexing() {
	PMS_PP_Iteration *iter = itsParams_.typedGroup<PMS_PP_Iteration>();
	PMS_PP_Axes* axes = itsParams_.typedGroup<PMS_PP_Axes>();
	bool globalX = iter->isGlobalScaleX();
	bool globalY = iter->isGlobalScaleY();
	PMS::Axis iterAxis = iter->iterationAxis();
	int dataCount = axes->numYAxes();
	//Only update if we need to.
	bool requiredUpdate = false;

	for ( int i = 0; i < dataCount; i++ ){
		bool iterationInitialized = itsCache_->isIndexerInitialized( iterAxis, globalX, globalY, i);
		if ( !iterationInitialized ){
			requiredUpdate = true;
			break;
		}
	}

	if ( requiredUpdate ){
		itsCache_->clearRanges();

		//Set up the indexer.
		for ( int i = 0; i < dataCount; i++ ){
			itsCache_->setUpIndexer(iterAxis, globalX, globalY, i);
		}
	}
	return true;
}

void PlotMSPlot::logPoints() {
	PMS_PP_Display *display = itsParams_.typedGroup<PMS_PP_Display>();
	bool showUnflagged =
			display->unflaggedSymbol()->symbol() != PlotSymbol::NOSYMBOL;
	bool showFlagged =
			display->flaggedSymbol()->symbol() != PlotSymbol::NOSYMBOL;
	bool allFlagged = false;

	stringstream ss;
	ss << "Plotting ";
	if(showUnflagged) {
		if ( itsCache_->nIter(0) > iter_ ){
                        uInt nUnflaggedPoints = itsCache_->indexer(0,iter_).sizeUnmasked(); 
			ss << nUnflaggedPoints << " unflagged" << (showFlagged ? ", " : "");
			if (nUnflaggedPoints==0) allFlagged = true;
		}
		else {
			ss << "0 unflagged" <<(showFlagged ? ", " : "");
		}
	}
	if(showFlagged) {
		if ( itsCache_->nIter(0) > iter_ ){
			ss << itsCache_->indexer(0,iter_).sizeMasked() << " flagged";
		}
		else {
			ss << "0 flagged";
		}
	}
	ss << " points.";

	itsParent_->getLogger()->postMessage(PMS::LOG_ORIGIN,
			PMS::LOG_ORIGIN_PLOT,
			ss.str(),
			PMS::LOG_EVENT_PLOT);
        if (allFlagged) {
		itsParent_->showWarning("All selected data are flagged.");
	}
	else { //clear warning
		itsParent_->clearMessage();
	}
}

void PlotMSPlot::logIter(Int iter, Int nIter) {
	if(nIter > 1) {
		stringstream ss;
		ss << "Stepping to iteration = " << iter+1
				<< " (of " << nIter << "): "
				<< itsCache_->indexer(0,iter).iterLabel();
		itsParent_->getLogger()->postMessage(PMS::LOG_ORIGIN,
				PMS::LOG_ORIGIN_PLOT,
				ss.str(),
				PMS::LOG_EVENT_PLOT);
	}
}

void PlotMSPlot::parametersHaveChanged(const PlotMSWatchedParameters& p,
        int updateFlag ) {
	if ( isCacheUpdating() ){
		return;
	}

    // Make sure it's this plot's parameters.
    if(&p != &parameters()) return;

    //A plot not to be shown.
    bool plottable = itsParent_->getPlotManager().isPlottable( this );
    if ( ! plottable ){
    	//Clear the plot
    	detachFromCanvases();
    	return;
    }
    vector<String> updates =
        PlotMSWatchedParameters::UPDATE_FLAG_NAMES(updateFlag);
    if(updates.size() == 0) return;
    
    // Log what we're going to be updating.
    stringstream ss;
    ss << "Updating: ";
    for(unsigned int i = 0; i < updates.size(); i++) {
        if(i > 0) ss << ", ";
        ss << updates[i];
    }
    ss << ".";
    itsParent_->getLogger()->postMessage(PMS::LOG_ORIGIN,
            PMS::LOG_ORIGIN_PARAMS_CHANGED, ss.str(),
            PMS::LOG_EVENT_PARAMS_CHANGED);
    int updateRedraw= updateFlag & PMS_PP::UPDATE_REDRAW;
    bool releaseWhenDone = !allDrawingHeld() && updateRedraw;
    if(releaseWhenDone){
    	holdDrawing();
    }
    
    // Update MS as needed.
    const PMS_PP_MSData* d = parameters().typedGroup<PMS_PP_MSData>();
    bool dataSuccess = d->isSet();
    if(dataSuccess && (updateFlag & PMS_PP::UPDATE_MSDATA)){
    	bool fileExists = true;
    	String fileName = d->filename();
    	ifstream ifile( fileName.c_str() );
    	if ( !ifile ){
    		fileExists = false;
    	}
    	else {
    		ifile.close();
    	}

    	if ( fileExists ){
    		dataSuccess = updateData();
    	}
    	else {
    		String errorMessage( "Please check that the file ");
    		errorMessage.append( fileName );
    		errorMessage.append( " is valid.");
    		itsParent_->getLogger()->postMessage(PMS::LOG_ORIGIN, PMS::LOG_ORIGIN_PLOT, errorMessage, PlotLogger::MSG_WARN);
    		dataSuccess = false;
    	}
    }

    // If something went wrong, clear the cache and plots.
    if(!dataSuccess) {
        itsCache_->clear();
        plotDataChanged();
    }
    
    // Let the child handle the rest of the parameter changes, and release
    // drawing if needed.
    bool result=parametersHaveChanged_(p,updateFlag,releaseWhenDone);
    if( result && releaseWhenDone){
    	//Note::this was put in because when reload was checked from the gui
    	//we were getting a segfault because the plot was redrawing before the cache
    	//was loaded from a thread.  There seems to be a mechanism in place to release
    	//the drawing later after the cache is loaded.
    	if ( ! itsParent_->guiShown() ){
    		releaseDrawing();
    	}
    }

}

void PlotMSPlot::plotDataChanged() {
    bool hold = allDrawingHeld();
    if(!hold) holdDrawing();
    
    vector<MaskedScatterPlotPtr> p = plots();
    for(unsigned int i = 0; i < p.size(); i++){
        if(!p[i].null()){
        	p[i]->dataChanged();
        }
    }
    
    if(!hold){
    	releaseDrawing();
    }
}

bool PlotMSPlot::isIteration() const {
	const PMS_PP_Iteration *iter = itsParams_.typedGroup<PMS_PP_Iteration>();
	bool iterationPlot = false;
	if ( iter != NULL ){
		iterationPlot = iter->isIteration();
	}
	return iterationPlot;
}



bool PlotMSPlot::exportToFormat(const PlotExportFormat& format) {
	vector<PlotCanvasPtr> canv = canvases();
    bool exportSuccess = true;

    //Determine how many pages we need to print.
    int pageCount = 1;
    //Store the current page.
    Int currentIter = iter();

    PlotMSExportParam& exportParams = itsParent_->getExportParameters();
    PMS::ExportRange range = exportParams.getExportRange();
    if ( range == PMS::PAGE_ALL ){
    	int iterationCount = itsCache_->nIter( 0 );
    	float divResult = (iterationCount * 1.0f) / canv.size();
    	pageCount = static_cast<int>(ceil( divResult ));
    	//If we are an iteration plot and we don't own the first few plots on the
    	//page we may need to bump the page count up by one.
    	if ( isIteration() ){
    		PlotMSPages &pages = itsParent_->getPlotManager().itsPages_;
    		PlotMSPage firstPage = pages.getFirstPage();
    		int firstPagePlotCount = getPageIterationCount( firstPage );
    		if ( firstPagePlotCount < static_cast<int>(canv.size()) ){
    			int notOwnedCount = canv.size() - firstPagePlotCount;
    			int excessSpace = (pageCount * canv.size()) - (notOwnedCount + iterationCount );
    			if ( excessSpace < 0 ){
    				pageCount = pageCount + 1;
    			}
    		}
    	}
    	firstIter();
    }


    PlotExportFormat exportFormat( format );
    String baseFileName = format.location;
    String suffix = "";
    int periodIndex = baseFileName.find_last_of( ".");
    //Remove the last '.' from the storage location.
    if ( periodIndex != static_cast<int>(String::npos) ){
    	suffix = baseFileName.substr( periodIndex, baseFileName.size() - periodIndex);
    	baseFileName = baseFileName.substr(0, periodIndex );
    }

    //Loop over all the iterations, exporting them
	waitOnCanvases();
	PlotMSPages &pages = itsParent_->getPlotManager().itsPages_;
	const String sep( "_");
    for ( int i = 0; i < pageCount; i++ ){
    	String pageStr;
    	if ( i > 0 ){
    		pageStr = String::toString( i+1 );
    	}

    	String itersInclude;
    	if (isIteration()){
    		int iterStart = this->iter_;
    		int iterEnd = getPageIterationCount( pages[i]);
    		int lastIndex = std::min( this->nIter() - iterStart, iterEnd) + iterStart;
    		int index = iterStart;

    		while ( index < lastIndex ){
    			String iterId;
    			if ( index == iterStart ){
    				iterId = itsCache_->indexer(0,index).fileLabel();
    			}
    			else {
    				iterId = itsCache_->indexer(0,index).iterValue();
    			}
    			if ( index < lastIndex - 1 ){
    				iterId = iterId + ",";
    			}
    			itersInclude = itersInclude + iterId;
    			index++;
    		}
    	}

    	String fileId;
    	if ( itersInclude.size() > 0 ){
    		fileId = sep + itersInclude;
    	}
    	if ( pageStr.size() > 0 ){
    		fileId = fileId + sep + pageStr;
    	}
    	exportFormat.location = baseFileName + fileId + suffix;
    	exportSuccess = itsParent_->exportToFormat( exportFormat );
    	waitOnCanvases();
    	if ( i < pageCount - 1 ){
    		nextIter();
    	}
    	waitOnCanvases();
    }

    //Restore the current page
    setIter( currentIter );
    return exportSuccess;
}

void PlotMSPlot::exportToFormatCancel(){
	vector<PlotCanvasPtr> canv = canvases();
	PlotOperationPtr op;
	for(unsigned int i = 0; i < canv.size(); i++) {
		if(canv[i].null()) continue;
	    op = canv[i]->operationExport();
	    if(op.null()) continue;
	    op->setCancelRequested(true);
	}
}

void PlotMSPlot::cacheLoaded_(bool wasCanceled) {
	// Ensure we fail gracefully if cache loading yielded nothing
	// or was cancelled

	if ( itsCache_ == NULL ){
		return;
	}
	if (!itsCache_->cacheReady() || wasCanceled) {
		dataMissing();
		return;
	}

	// Make this more specific than canvas-triggered
	if (itsTCLParams_.updateCanvas || itsTCLParams_.updateIteration ){
		updateIndexing();
	}

	// Reset the iterator (if data are new)
	resetIter();

	// Let the plot know that the data has been changed as needed, unless the
	// thread was canceled.
	updatePlots();


	// Update display as needed.  Put this before update canvas so
	// that the legend item keys will have the correct color.
	if(itsTCLParams_.updateDisplay){
		updateDisplay();
	}

	// Update canvas as needed.
	if(itsTCLParams_.updateCanvas){
		updateCanvas();
	}

	// Release drawing if needed.
	if(itsTCLParams_.releaseWhenDone && !isCacheUpdating() ){
		releaseDrawing();
	}


	// Log number of points as needed.
	if(itsTCLParams_.endCacheLog){
		logPoints();
	}


	// Report we are done
	if(itsTCLParams_.endCacheLog){
		itsParent_->getLogger()->releaseMeasurement();
	}
}

void PlotMSPlot::setRelease( bool b ){
	itsTCLParams_.releaseWhenDone = b;
}

void PlotMSPlot::canvasWasDisowned(PlotCanvasPtr canvas) {
    if(canvas.null()) return;

    vector<MaskedScatterPlotPtr> p = plots();
    for(unsigned int i = 0; i < p.size(); i++)
        if(!p[i].null()) canvas->removePlotItem(p[i]);
}


// Protected Methods //

bool PlotMSPlot::initializePlot() {
	Int rows = 1;
	Int cols = 1;
	getPlotSize( rows, cols );
	resizePlots( rows, cols );
	setColors();
	return true;
}

bool PlotMSPlot::parametersHaveChanged_(const PlotMSWatchedParameters &p,
		int updateFlag,
		bool releaseWhenDone) {

	if(&p != &itsParams_) {
		return false;
	}

	const PMS_PP_MSData *data = itsParams_.typedGroup<PMS_PP_MSData>();
	const PMS_PP_Iteration *iter = itsParams_.typedGroup<PMS_PP_Iteration>();
	const PMS_PP_Axes *axes = itsParams_.typedGroup<PMS_PP_Axes>();

	if(data == NULL || iter == NULL || axes == NULL ){
		return true;
	}

	itsTCLParams_.releaseWhenDone = releaseWhenDone;
	itsTCLParams_.updateCanvas = (updateFlag & PMS_PP::UPDATE_AXES) ||
			(updateFlag & PMS_PP::UPDATE_CACHE) ||
			(updateFlag & PMS_PP::UPDATE_CANVAS) ||
			(updateFlag & PMS_PP::UPDATE_ITERATION) ||
			(updateFlag & PMS_PP::UPDATE_MSDATA) || !data->isSet();

	itsTCLParams_.updateDisplay = updateFlag & PMS_PP::UPDATE_DISPLAY;
	itsTCLParams_.endCacheLog = false;


	// Clear selection if axes change
	//if(updateFlag & PMS_PP::UPDATE_AXES) {
	// Apparently UPDATE_AXES is not triggered by anything...
	// UPDATE_CACHE should be close enough for now (I hope)

	int updateCacheFlag = updateFlag & PMS_PP::UPDATE_CACHE;
	if( updateCacheFlag ) {
		for(size_t r = 0; r < itsCanvases_.size(); ++r) {
			for(size_t c = 0; c < itsCanvases_[r].size(); ++c) {
				PlotCanvasPtr plotCanvas = itsCanvases_[r][c];
				if ( ! plotCanvas.null() ){
					plotCanvas->standardMouseTools()->selectTool()->clearSelectedRects();
					plotCanvas->clearAnnotations();
					//plotCanvas->clearItems();
					plotCanvas->clearShapes();
				}
			}
		}

	}

	//See if the iteration parameters have changed.
	bool commonAxisX = iter->isCommonAxisX();
	bool commonAxisY = iter->isCommonAxisY();
	Int rows = 0;
	Int cols = 0;
	getPlotSize( rows, cols );
	PlotAxis locationAxisX = axes->xAxis();
	PlotAxis locationAxisY = axes->yAxis();
	int displayRow = iter->getGridRow();
	int displayCol = iter->getGridCol();
	int plotRows = itsPlots_.size();
	int plotCols = 0;
	if ( plotRows > 0 ){
		plotCols = itsPlots_[0].size();
	}
	bool locationChange = false;
	if ( (gridRow != displayRow || gridCol != displayCol) && gridRow != -1 ){
		locationChange = true;

		//This removes the title and axes from its previous plot location.
		QList<PlotMSPlot*> canvasPlots = itsParent_->getPlotManager().getCanvasPlots( gridRow, gridCol);

		if ( canvasPlots.size() == 1 ){
			//We are the sole occupant of the old spot (no overplotting) so we
			//erase all evidence of there being a plot
			itsParent_->getPlotManager().clearCanvas( gridRow, gridCol );
		}
		else if ( canvasPlots.size() > 1 ){
			//Just erase ourselves from the canvas.
			itsParent_->getPlotManager().itsPages_.disown( gridRow, gridCol, this );
			detachFromCanvases();
			//Tell the other plots to redraw
			for ( int i = 0; i < canvasPlots.size(); i++ ){
				if ( canvasPlots[i] != this ){
					canvasPlots[i]->parametersHaveChanged( canvasPlots[i]->parameters(),PMS_PP::UPDATE_REDRAW );
				}
			}
		}
	}

	bool updateIter = updateFlag & PMS_PP::UPDATE_ITERATION;
	itsTCLParams_.updateIteration = ( updateIter ||
			((plotRows != rows) || (plotCols != cols)) ||
					(itsParent_->isCommonAxisX() != commonAxisX) ||
					(itsParent_->isCommonAxisY() != commonAxisY) ||
					(itsParent_->getAxisLocationX() != locationAxisX) ||
					(itsParent_->getAxisLocationY() != locationAxisY) ||
					locationChange );
	itsParent_->setCommonAxes( commonAxisX, commonAxisY);
	itsParent_->setAxisLocation( locationAxisX, locationAxisY);
	gridRow = displayRow;
	gridCol = displayCol;

	//We are not plotting this particular plot so just clear it and return.
	if ( displayRow == -1 || displayCol == -1 ){
		clearCanvases();
		return true;
	}

	bool dataSet = data->isSet();
	bool updateData = (updateFlag & PMS_PP::UPDATE_MSDATA) || (updateFlag & PMS_PP::UPDATE_CACHE);

	//If the iteration count has changed, ie from an iteration to a
	//non-iteration or just a change in the iteration axis, we may need
	//to clear the cache and update it.
	if ( updateIter ){
		PMS::Axis newAxis = iter->iterationAxis();
		PMS::Axis cacheIterationAxis = itsCache_->getIterAxis();
		if ( newAxis != cacheIterationAxis ){
			updateData = true;
		}
	}


	// Update cache if needed
	bool handled = true;
	if( dataSet && updateData ) {
		try {
			handled = updateCache();
		}
		catch( AipsError& error ){
			cerr << "Could not update cache: "<<error.getMesg().c_str()<<endl;
			cacheLoaded_(false);
			handled = false;
		}
	}
	else {
		cacheLoaded_(false);
	}
	return handled;
}

void PlotMSPlot::constructorSetup() {
    PlotMSPlotParameters& params = parameters();
    params.addWatcher(this);
    // hold notification until initializePlot is called
    params.holdNotification(this);

    gridRow = -1;
    gridCol = -1;
    makeParameters(params, itsParent_);
}

bool PlotMSPlot::allDrawingHeld() {
    vector<PlotCanvasPtr> canv = canvases();
    bool allDrawingHeld = true;
    int canvasCount = canv.size();
    for(int i = 0; i < canvasCount; i++){
        if(!canv[i].null()){
        	bool canvasDrawingHeld = canv[i]->drawingIsHeld();
        	if ( !canvasDrawingHeld ){
        		allDrawingHeld = false;
        		break;
        	}
        }
    }
    return allDrawingHeld;
}

void PlotMSPlot::holdDrawing() {
   vector<PlotCanvasPtr> canv = canvases();
    for(unsigned int i = 0; i < canv.size(); i++){
    	if ( !canv[i].null() ){
    		bool canvasDrawing = canv[i]->isDrawing();
    		if ( canvasDrawing ){
    			waitOnCanvas( canv[i]);
    		}
    		canv[i]->holdDrawing();
    	}
    }
}

void PlotMSPlot::releaseDrawing() {
    vector<PlotCanvasPtr> canv = canvases();
    for(unsigned int i = 0; i < canv.size(); i++){
        if(!canv[i].null()){
        	if ( canv[i]->drawingIsHeld()){
        		canv[i]->releaseDrawing();
        	}
        }
    }
}

void PlotMSPlot::waitOnCanvas( const PlotCanvasPtr& canvas ){
	if ( !canvas.null()){
		int callIndex = 0;
		int maxCalls =  60;

		bool scriptClient = !itsParent_->guiShown();
		if ( scriptClient ){
			return;
		}
		bool canvasDrawing = canvas->isDrawing( );
	   while(  canvasDrawing && callIndex < maxCalls ){
	        usleep(1000000);
	        callIndex++;
	        canvasDrawing = canvas->isDrawing();
	    }
	}
}

void PlotMSPlot::waitOnCanvases(){
	vector<PlotCanvasPtr> canv = canvases();
		for (unsigned int i = 0; i < canv.size(); i++ ){
			if ( !canv[i].null()){
				waitOnCanvas( canv[i]);
			}
		}

}

void PlotMSPlot::waitForDrawing( bool holdDrawing ){

	vector<PlotCanvasPtr> canv = canvases();
	for (unsigned int i = 0; i < canv.size(); i++ ){
		if ( !canv[i].null()){
			waitOnCanvas( canv[i]);
			if ( holdDrawing ){
				canv[i]->holdDrawing();
			}
		}
	}
	detachFromCanvases();
}

}

bool PlotMSPlot::firstIter() {
	Int nIter = itsCache_->nIter(0);
	if( (nIter > 1) && (iter_ != 0) ) {
		PlotMSPages &pages = itsParent_->getPlotManager().itsPages_;
		pages.firstPage();
		iter_ = 0;
		recalculateIteration();
		return true;
	}
	return false;
}

bool PlotMSPlot::prevIter() {
	Int nIter = itsCache_->nIter(0);
	if( nIter > 1 && iter_ > 0 ) {
		PlotMSPages &pages = itsParent_->getPlotManager().itsPages_;
		iter_ -= getPageIterationCount( pages.currentPage() );
		if (iter_ < 0) iter_=0;  // just in case
		pages.previousPage();
		recalculateIteration();
		return true;
	}
	return false;
}

bool PlotMSPlot::nextIter() {
	Int nIter = itsCache_->nIter(0);
	if( nIter > 1) {
		PlotMSPages &pages = itsParent_->getPlotManager().itsPages_;
		int pageIterCount = getPageIterationCount( pages.currentPage() );

		if((iter_+pageIterCount) < nIter ) {
			iter_ += pageIterCount;
			pages.nextPage();
			recalculateIteration();
			return true;
		}
	}
	return false;
}

bool PlotMSPlot::lastIter() {
	Int nIter = itsCache_->nIter(0);
	if((nIter > 0) && (iter_ < (nIter - iterStep_))) {
		PlotMSPages &pages = itsParent_->getPlotManager().itsPages_;
		int firstPageIterCount = getPageIterationCount( pages.getFirstPage() );
		iter_ = int(double(nIter-1) / iterStep_) * iterStep_;
		if(iterStep_ == 1){
			iter_ = nIter - 1;
		}
		else {
			if ( firstPageIterCount < iterStep_ ){
				iter_ = iter_ - (iterStep_ - firstPageIterCount );
			}
		}
		pages.lastPage();
		recalculateIteration();
		return true;
	}
	return false;
}

bool PlotMSPlot::setIter( int index ){
	Int nIter = itsCache_->nIter(0);
	bool successful = false;
	if( nIter > 1 && index < nIter && index >= 0) {
		PlotMSPages &pages = itsParent_->getPlotManager().itsPages_;
		pages.setCurrentPageNum( index );
		iter_ = index;
		
		recalculateIteration();
		successful = true;
	}
	return successful;
}

bool PlotMSPlot::resetIter() {
	Int nIter = itsCache_->nIter(0);
	if(nIter > 0 ) {
		PlotMSPages &pages = itsParent_->getPlotManager().itsPages_;
		pages.firstPage();
		iter_ = 0;
		recalculateIteration();
		return true;
	}
	return false;
}

void PlotMSPlot::recalculateIteration( ) {
	bool drawingHeld = allDrawingHeld();
	if ( !drawingHeld ){
		this->holdDrawing();
	}
	int nIter = itsCache_->nIter(0);
		if ( nIter <= 0 ){
			nIter = 1;
		}


	detachFromCanvases();
	if(itsTCLParams_.updateIteration  || isIteration()) {
		PlotMSPages &pages = itsParent_->getPlotManager().itsPages_;
		assignCanvases(pages);
	}

	//Add for CAS-6928/CAS-7014.  Tools were not being reset when
	//the iteration plot page changed.
	itsParent_->resetTools();

	//Put the data into the plot
	uInt rows = itsPlots_.size();
	for(uInt r = 0; r < rows; ++r) {
		uInt cols = itsPlots_[r].size();
		for(uInt c = 0; c < cols; ++c) {
			int iterationIndex = c;
			if(iterationIndex >= nIter ){
				break;
			}
			logIter(iterationIndex, nIter);
			PlotMaskedPointDataPtr data(&(itsCache_->indexer(r,c)), false);
			itsPlots_[r][c] = itsFactory_->maskedPlot(data);
		}
	}

	setColors();
	itsTCLParams_.updateDisplay = true;

	//Update display should come before update canvas so that the
	//legend items get the correct color.
	updateDisplay();
	updateCanvas();

	attachToCanvases();
	updatePlots();
	if ( !isCacheUpdating() && !drawingHeld ){
		releaseDrawing();
	}

	logPoints();

}

Int PlotMSPlot::nIter() {
	Int iterationCount = 0;
	if ( itsCache_ != NULL ){
		iterationCount = itsCache_->nIter(0);
	}
	return iterationCount;
}

int PlotMSPlot::getPageIterationCount( const PlotMSPage& page ) {
	int rows = itsCanvases_.size();
	int cols = 0;
	if ( rows > 0 ){
		cols = itsCanvases_[0].size();
	}
	int iterationCanvasCount = getIterationIndex(rows,cols,page);
	iterationCanvasCount = iterationCanvasCount - iter_;
	return iterationCanvasCount;
}

void PlotMSPlot::updateLocation(){

	PlotMSPages &pages = itsParent_->getPlotManager().itsPages_;

	//Initializes the canvases for this plot
	assignCanvases(pages);

	//Put the plot data on the canvas.
	attachToCanvases();


	//For scripting mode, we get plots without axes if the call is not preset.
	if ( !itsParent_->guiShown()  ){
		//Put the plot axis on the canvas.
		updateCanvas();
	}

}

PlotMSRegions PlotMSPlot::selectedRegions(
		const vector<PlotCanvasPtr>& canvases) const {
	PlotMSRegions r;
	PMS::Axis x = (PMS::Axis)PMS_PP_RETCALL(itsParams_, PMS_PP_Cache,
			xAxis, 0);
	PMS::Axis y = (PMS::Axis)PMS_PP_RETCALL(itsParams_, PMS_PP_Cache,
			yAxis, 0);

	for(uInt i = 0; i < canvases.size(); ++i) {
		r.addRegions(x, y, canvases[i]);
	}
	return r;
}

bool PlotMSPlot::assignCanvases(PlotMSPages &pages) {
	if(pages.totalPages() == 0) {
		pages.insertPage();
		pages.firstPage();
	}

	//Resize based on the row and column count
	PlotMSParameters params = itsParent_->getParameters();
	int rows = params.getRowCount();
	int cols = params.getColCount();
	resize( pages, rows, cols );
	int currentPage = pages.currentPageNumber();
	PlotMSPage& page = pages[currentPage];

	const PMS_PP_Iteration* iterParams = itsParams_.typedGroup<PMS_PP_Iteration>();
	int rowIndex = 0;
	int colIndex = 0;
	if ( iterParams != NULL ){
		rowIndex = iterParams->getGridRow();
		colIndex = iterParams->getGridCol();
	}

	page.disown( this );
	if ( rowIndex >= 0 && colIndex >= 0 ){
		//Find a canvas for this plot.
		for(int r = 0; r < rows; ++r) {
			bool assigned = false;
			for(int c = 0; c < cols; ++c) {
				if ( isIteration() ){
					if ( (r > rowIndex) || (r == rowIndex && c>=colIndex) || currentPage > 0 ){
						if( !page.isOwned(r, c)) {
							page.setOwner(r, c, this);
							itsCanvases_[r][c] = page.canvas(r, c);
						}
					}
				}
				else {
					//If it is not an iteration plot, there is just
					//one canvas for this plot.
					if ( rowIndex == r && colIndex == c){
						//page.disown( this );
						page.setOwner(r, c, this);
						itsCanvases_[0][0] = page.canvas(r,c);
						assigned = true;
						break;
					}

				}
			}
			if ( assigned ){
				break;
			}
		}

	}
	page.setupPage();
	return true;
}

void PlotMSPlot::resizePlots( int rows, int cols ){
	itsPlots_.resize( rows );
	for ( int r = 0; r < rows; ++r) {
		itsPlots_[r].resize( cols );
		for ( int c = 0; c < cols; ++c) {
			//Put empty data into the plot.
			PlotMaskedPointDataPtr data(&(itsCache_->indexer0()), false);
			itsPlots_[r][c] = itsFactory_->maskedPlot(data);

			// We want to execute this loop at least once to fill in
			// a single plot scenario; but after that, if there are
			// no iterations, break out
			//if(iter >= nIter) break;
		}
		//if(iter >= nIter) break;
	}
}

void PlotMSPlot::getPlotSize( Int& rows, Int& cols ){
	rows = 1;
	cols = 1;

	//Number of plots is based on how many overplots we
	//are supporting (dataCount) and on the iteration count
	//over the data.
	const PMS_PP_Axes *axes = itsParams_.typedGroup<PMS_PP_Axes>();
	if ( axes != NULL ){
		rows = axes->numYAxes();
	}

	int iterationCount = itsCache_->nIter(0);
	if ( iterationCount > 0 ){
		cols = iterationCount;
	}
}

int PlotMSPlot::getIterationIndex( int r, int c, const PlotMSPage& page ){
	int iterationIndex = iter_;
	bool found = false;
	int rows = page.canvasRows();
	int cols = page.canvasCols();
	for ( int i = 0; i < rows; i++ ){
		for ( int j = 0; j <= cols; j++ ){
			if ( i == r && j == c ){
				found =true;
				break;
			}
			else {
				bool ownsCanvas = page.isOwner(i,j, this);
				if ( ownsCanvas ){
					iterationIndex++;
				}
			}
		}
		if ( found ){
			break;
		}
	}
	return iterationIndex;
}

void PlotMSPlot::logMessage( const QString& msg ) const {
	if ( itsParent_ != NULL ){
		stringstream ss;
		ss << msg.toStdString().c_str();
		itsParent_->getLogger()->postMessage(PMS::LOG_ORIGIN,
					PMS::LOG_ORIGIN_PLOT,
					ss.str(),
					PMS::LOG_EVENT_PLOT);
	}
}

void PlotMSPlot::clearCanvasProperties( int row, int col){
	PlotCanvasPtr canvas = itsCanvases_[row][col];
	if(canvas.null()){
		return;
	}
	canvas->showAllAxes( false );
	canvas->setTitle( "" );
	canvas->setCommonAxes( false, false );
}

void PlotMSPlot::setCanvasProperties (int row, int col,
		PMS_PP_Cache* cacheParams, PMS_PP_Axes* axesParams,
		bool set, PMS_PP_Canvas *canvParams, uInt rows, uInt cols,
		PMS_PP_Iteration *iter, uInt iteration) {

	PlotCanvasPtr canvas = itsCanvases_[row][col];
	if(canvas.null()){
		return;
	}

	// Show/hide axes
	canvas->showAllAxes(false);

	//ShowX and showY determine whether axes are visible at
	//all.  For visible axes, there is the option of sharing
	//them (common) or for each plot to manage its own.
	bool commonX = iter->isCommonAxisX();
	bool commonY = iter->isCommonAxisY();
	canvas->setCommonAxes( commonX, commonY );
	bool showX = set && canvParams->xAxisShown();
	bool showY = set && canvParams->yAxisShown();
	PlotAxis cx = axesParams->xAxis();
	canvas->showAxis(cx, showX);
	int yAxisCount = axesParams->numYAxes();
	for ( int i = 0; i < yAxisCount; i++ ){
		PlotAxis cy = axesParams->yAxis( i );
		canvas->showAxis(cy, showY);
	}

	// Set axes scales
	PMS::Axis x = cacheParams->xAxis();
	canvas->setAxisScale(cx, PMS::axisScale(x));
	for ( int i = 0; i < yAxisCount; i++ ){
		PMS::Axis y = cacheParams->yAxis( i );
		PlotAxis cy = axesParams->yAxis( i );
		canvas->setAxisScale(cy, PMS::axisScale(y));
	}

	// Set reference values
	bool xref = itsCache_->hasReferenceValue(x);
	double xrefval = itsCache_->referenceValue(x);
	canvas->setAxisReferenceValue(cx, xref, xrefval);
	for ( int i = 0; i < yAxisCount; i++ ){
		PMS::Axis y = cacheParams->yAxis( i );
		PlotAxis cy = axesParams->yAxis( i );
		bool yref = itsCache_->hasReferenceValue(y);
		double yrefval = itsCache_->referenceValue(y);
		canvas->setAxisReferenceValue(cy, yref, yrefval);
	}


	// Legend
	canvas->showLegend(set && canvParams->legendShown(), canvParams->legendPosition());

	// Title font
	PlotFontPtr font = canvas->titleFont();
	font->setPointSize(std::max(16. - rows*cols+1., 8.));
	font->setBold(true);
	canvas->setTitleFont(font);
	int gridRow = iter->getGridRow();
	int gridCol = iter->getGridCol();
	QList<PlotMSPlot*> canvasPlots  = itsParent_->getPlotManager().getCanvasPlots( gridRow, gridCol );

	int canvasPlotCount = canvasPlots.size();
	canvas->clearAxesLabels();

	//x-axis label
	if(set) {
		PMS::DataColumn xDataColumn = cacheParams->xDataColumn();
		String xLabelSingle = canvParams->xLabelFormat().getLabel(x, xref, xrefval, xDataColumn);
		canvas->setAxisLabel(cx, xLabelSingle);
		PlotFontPtr xFont = canvas->axisFont(cx);
		xFont->setPointSize(std::max(12. - rows*cols+1., 8.));
		canvas->setAxisFont(cx, xFont);
	}

	String yLabelLeft;
	String yLabelRight;
	for ( int j = 0; j < canvasPlotCount; j++ ){
		// Set axes labels
		PlotMSPlotParameters plotParams = canvasPlots[j]->parameters();
		PMS_PP_Cache *plotCacheParams = plotParams.typedGroup<PMS_PP_Cache>();
		PMS_PP_Axes * plotAxisParams = plotParams.typedGroup<PMS_PP_Axes>();
		if ( plotCacheParams == NULL || plotAxisParams == NULL ){
			continue;
		}
		if(set) {
			int plotYAxisCount = plotAxisParams->numYAxes();
			for ( int i = 0; i < plotYAxisCount; i++ ){
				PMS::Axis y = plotCacheParams->yAxis( i );
				PlotAxis cy = plotAxisParams->yAxis( i );
				bool yref = itsCache_->hasReferenceValue(y);
				double yrefval = itsCache_->referenceValue(y);
				PMS::DataColumn yDataColumn = plotCacheParams->yDataColumn(i);
				String yLabelSingle = canvParams->yLabelFormat( ).getLabel(y, yref, yrefval, yDataColumn );
				if ( cy == Y_LEFT ){
					if ( yLabelLeft.size() > 0 ){
						yLabelLeft.append( ", ");
					}
					yLabelLeft.append( yLabelSingle );
				}
				else {
					if ( yLabelRight.size() > 0 ){
						yLabelRight.append( ", ");
					}
					yLabelRight.append( yLabelSingle );
				}
			}
		}
	}
	if ( yLabelLeft.size() > 0 ){
		canvas->setAxisLabel(Y_LEFT, yLabelLeft);
		PlotFontPtr yFont = canvas->axisFont( Y_LEFT);
		yFont->setPointSize(std::max(12. - rows*cols+1., 8.));
		canvas->setAxisFont(Y_LEFT, yFont);
	}
	if ( yLabelRight.size() > 0 ){
		canvas->setAxisLabel(Y_RIGHT, yLabelRight);
		PlotFontPtr yFont = canvas->axisFont( Y_RIGHT);
		yFont->setPointSize(std::max(12. - rows*cols+1., 8.));
		canvas->setAxisFont(Y_RIGHT, yFont);
	}

	// Custom axes ranges
	canvas->setAxesAutoRescale(true);
	if ( set ){
		if ( axesParams->xRangeSet() ){
			canvas->setAxisRange(cx, axesParams->xRange());
		}
		for ( int i = 0; i < yAxisCount; i++ ){
			if ( axesParams->yRangeSet(i) ){
				PlotAxis cy = axesParams->yAxis( i );
				canvas->setAxisRange(cy, axesParams->yRange(i));
			}
		}
	}

	// Title
	bool resetTitle = set || (iter->iterationAxis() != PMS::NONE);
	String iterTxt;
	if(iter->iterationAxis() != PMS::NONE && itsCache_->nIter(0) > 0) {
		iterTxt = itsCache_->indexer(0,iteration).iterLabel();
	}
	String title = "";
	if(resetTitle) {
		PMS::DataColumn xDataColumn = cacheParams->xDataColumn();
		vector<PMS::Axis> yAxes;
		vector<bool> yRefs;
		vector<double> yRefVals;
		vector<PMS::DataColumn> yDatas;
		for ( int j = 0; j < canvasPlotCount; j++ ){
			PlotMSPlotParameters plotParams = canvasPlots[j]->parameters();
			PMS_PP_Cache* plotCacheParams = plotParams.typedGroup<PMS_PP_Cache>();
			PMS_PP_Axes* plotAxisParams = plotParams.typedGroup<PMS_PP_Axes>();
			if ( plotCacheParams == NULL || plotAxisParams == NULL ){
				continue;
			}
			PlotMSCacheBase& plotCacheBase = canvasPlots[j]->cache();
			int plotYAxisCount = plotAxisParams->numYAxes();
			for ( int i = 0; i < plotYAxisCount; i++ ){
				yAxes.push_back(plotCacheParams->yAxis( i ));
				yRefs.push_back(plotCacheBase.hasReferenceValue(yAxes[i]));
				yRefVals.push_back(plotCacheBase.referenceValue(yAxes[i]));
				yDatas.push_back(plotCacheParams->yDataColumn( i ) );
			}
		}
		title = canvParams->titleFormat().getLabel(x, yAxes, xref,
				xrefval, yRefs, yRefVals, xDataColumn, yDatas)
				+ " " + iterTxt;
		canvas->setTitle(title);
	}

	// Grids
	canvas->showGrid(canvParams->gridMajorShown(), canvParams->gridMinorShown(),
			canvParams->gridMajorShown(), canvParams->gridMinorShown());

	PlotLinePtr major_line =
			itsFactory_->line(canvParams->gridMajorLine());
	if(!canvParams->gridMajorShown()) {
		major_line->setStyle(PlotLine::NOLINE);
	}
	canvas->setGridMajorLine(major_line);

	PlotLinePtr minor_line =
			itsFactory_->line(canvParams->gridMinorLine());
	if(!canvParams->gridMinorShown()) {
		minor_line->setStyle(PlotLine::NOLINE);
	}
	canvas->setGridMinorLine(minor_line);
}

