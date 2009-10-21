//# QtCleanPanelGui.cc:	Prototype QObject for interactive clean.
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
//# FITNESS FOR A PARTICULAR PURPOSE.	See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#				Internet email: aips2-request@nrao.edu.
//#				Postal address: AIPS++ Project Office
//#			                    National Radio Astronomy Observatory
//#			                    520 Edgemont Road
//#			                    Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <casa/iostream.h>

#include <display/QtViewer/QtCleanPanelGui.qo.h>
#include <display/DisplayDatas/DisplayData.h>
#include <display/QtViewer/QtViewer.qo.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeExpr.h> 
#include <lattices/Lattices/LCBox.h>
#include <images/Images/PagedImage.h>
#include <images/Images/SubImage.h>
#include <images/Regions/ImageRegion.h>
#include <images/Regions/RegionManager.h>
#include <images/Regions/WCBox.h>
#include <images/Regions/WCIntersection.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/iostream.h>
#include <casa/fstream.h>
#include <casa/sstream.h>	
namespace casa {

    QtCleanPanelGui::QtCleanPanelGui( QtViewer *v, QWidget *parent ) : QtDisplayPanelGui( v, parent ),
								       in_interact_mode(false), interact_id(0),
								       maskdd_(0), imagedd_(0) {

	resize(700,900);
	
	v_->autoDDOptionsShow = False;		// Prevents automatically showing 'adjust' panel.
	setStatsPrint(False);			// Turns off statistics printing.

	QDockWidget *dock = new QDockWidget(this, Qt::Widget);

	QWidget *widget = new QWidget(dock);
	if (widget->objectName().isEmpty())
	    widget->setObjectName(QString::fromUtf8("Interactive_Clean"));
	widget->setFixedSize(641, 51);
	    
	QFont smallFont;
	smallFont.setPointSize(8);

	    
	////////
	// Drawing Mode
	//
	QFrame *frame_2;
	frame_2 = new QFrame(widget);
	frame_2->setObjectName(QString::fromUtf8("frame_2"));
	frame_2->setGeometry(QRect(220, 0, 111, 51));
	frame_2->setFrameShape(QFrame::StyledPanel);
	frame_2->setFrameShadow(QFrame::Raised);
	frame_2->setLineWidth(1);
		
	addRB_ = new QRadioButton(frame_2);
	addRB_->setObjectName(QString::fromUtf8("addRB_"));
	addRB_->setGeometry(QRect(20, 4, 79, 21));
	addRB_->setChecked(True);
	addRB_->setToolTip("Set the mask under drawn region");

	eraseRB_ = new QRadioButton(frame_2);
	eraseRB_->setObjectName(QString::fromUtf8("eraseRB_"));
	eraseRB_->setGeometry(QRect(20, 28, 79, 21));
	eraseRB_->setChecked(False);
	eraseRB_->setToolTip("Erase the mask under drawn region");

	frame_2->setEnabled(false);
	default_palette = frame_2->palette( );
	input_palette = frame_2->palette( );
	input_palette.setColor( backgroundRole(), QColor( 20, 255, 64) );
// 	input_palette.setColor( backgroundRole(), QColor(122, 193, 66) );	// Qt Green
	frame_2->setAutoFillBackground(true);
	disabled_widgets.push_back(frame_2);

	////////
	// channels or planes
	// 
	QFrame *frame_4;
	frame_4 = new QFrame(widget);
	frame_4->setObjectName(QString::fromUtf8("frame_4"));
	frame_4->setGeometry(QRect(330, 0, 141, 51));
	frame_4->setFrameShape(QFrame::StyledPanel);
	    
	thisPlaneRB_ = new QRadioButton(frame_4);
	thisPlaneRB_->setObjectName(QString::fromUtf8("thisPlaneRB_"));
	thisPlaneRB_->setGeometry(QRect(10, 4, 121, 16));
	thisPlaneRB_->setChecked(True);
	thisPlaneRB_->setToolTip("Limit mask(or erasure) to the displayed plane only");
	    
	allChanRB_ = new QRadioButton(frame_4);
	allChanRB_->setObjectName(QString::fromUtf8("allChanRB_"));
	allChanRB_->setGeometry(QRect(10, 28, 113, 16));
	allChanRB_->setChecked(False);
	allChanRB_->setToolTip("Propagate mask(or erasure) to all channels");

	frame_4->setEnabled(false);
	frame_4->setAutoFillBackground(true);
	disabled_widgets.push_back(frame_4);

	////////
	// Action Buttons
	//
	QFrame *frame_3;
	frame_3 = new QFrame(widget);
	frame_3->setObjectName(QString::fromUtf8("frame_3"));
	frame_3->setGeometry(QRect(470, 0, 171, 51));
	frame_3->setFrameShape(QFrame::StyledPanel);

	QLabel *label_5 = new QLabel(frame_3);
	label_5->setObjectName(QString::fromUtf8("label_5"));
	label_5->setGeometry(QRect(10, 0, 96, 20));
	label_5->setFont(smallFont);

	maskDonePB_ = new QPushButton(frame_3);
	maskDonePB_->setObjectName(QString::fromUtf8("maskDonePB_"));
	maskDonePB_->setGeometry(QRect(120, 15, 41, 31));
	maskDonePB_->setToolTip("Continue deconvolution with current Clean region(s), then display this dialog again.");
	maskDonePB_->setIcon(QIcon(QString::fromUtf8(":/icons/view_refresh.png")));
	maskDonePB_->setIconSize(QSize(32, 32));
	maskDonePB_->setDefault(true);
	maskDonePB_->setFlat(true);

	maskNoMorePB_ = new QPushButton(frame_3);
	maskNoMorePB_->setObjectName(QString::fromUtf8("maskNoMorePB_"));
	maskNoMorePB_->setGeometry(QRect(68, 15, 41, 31));
	maskNoMorePB_->setToolTip("Apply mask edits and proceed with NON-interactive deconvolution - this dialog will not be displayed again!");
	maskNoMorePB_->setIcon(QIcon(QString::fromUtf8(":/icons/finish.png")));
	maskNoMorePB_->setIconSize(QSize(32, 32));
	maskNoMorePB_->setFlat(true);

	stopPB_ = new QPushButton(frame_3);
	stopPB_->setObjectName(QString::fromUtf8("stopPB_"));
	stopPB_->setGeometry(QRect(10, 15, 41, 31));
	stopPB_->setToolTip("Stop deconvolving now");
	stopPB_->setIcon(QIcon(QString::fromUtf8(":/icons/stop.png")));
	stopPB_->setIconSize(QSize(32, 32));
	stopPB_->setFlat(true);

	frame_3->setEnabled(false);
	frame_3->setAutoFillBackground(true);
	disabled_widgets.push_back(frame_3);

	////////
	// Iteration Control Items
	// 
	QFrame *frame;
	frame = new QFrame(widget);
	frame->setObjectName(QString::fromUtf8("frame"));
	frame->setGeometry(QRect(0, 0, 221, 51));
	frame->setFrameShape(QFrame::StyledPanel);
	frame->setFrameShadow(QFrame::Raised);

	QLabel *label = new QLabel(frame);
	label->setObjectName(QString::fromUtf8("label"));
	label->setGeometry(QRect(10, 0, 61, 20));
	label->setFont(smallFont);
	label->setAlignment(Qt::AlignCenter);

	niterED_ = new QLineEdit(frame);
	niterED_->setObjectName(QString::fromUtf8("niterED_"));
	niterED_->setGeometry(QRect(10, 20, 64, 22));
	niterED_->setMinimumSize(QSize(64, 0));
	niterED_->setMaxLength(128);
	niterED_->setToolTip("Number of iterations in each interactive loop");

	QLabel *label_2 = new QLabel(frame);
	label_2->setObjectName(QString::fromUtf8("label_2"));
	label_2->setGeometry(QRect(80, 0, 61, 20));
	label_2->setFont(smallFont);
	label_2->setAlignment(Qt::AlignCenter);

	ncyclesED_ = new QLineEdit(frame);
	ncyclesED_->setObjectName(QString::fromUtf8("ncyclesED_"));
	ncyclesED_->setGeometry(QRect(80, 20, 64, 22));
	ncyclesED_->setMinimumSize(QSize(64, 0));
	ncyclesED_->setMaxLength(128);
	ncyclesED_->setToolTip("Number of interactive loops");

	QLabel *label_3 = new QLabel(frame);
	label_3->setObjectName(QString::fromUtf8("label_3"));
	label_3->setGeometry(QRect(150, 0, 61, 20));
	label_3->setFont(smallFont);
	label_3->setAlignment(Qt::AlignCenter);

	threshED_ = new QLineEdit(frame);
	threshED_->setObjectName(QString::fromUtf8("threshED_"));
	threshED_->setGeometry(QRect(150, 20, 64, 22));
	threshED_->setMinimumSize(QSize(64, 0));
	threshED_->setMaxLength(128);
	threshED_->setToolTip("Threshold at which to stop clean; e.g 10 mJy\nClean will stop immediately if it is reached.");

	frame->setEnabled(false);
	frame->setAutoFillBackground(true);
	disabled_widgets.push_back(frame);

	// 
	//////// end iteration control items

	threshED_->setText(QApplication::translate("InteractiveClean", "8 mJy", 0, QApplication::UnicodeUTF8));
	label_5->setText(QApplication::translate("InteractiveClean", "Next Action:", 0, QApplication::UnicodeUTF8));
	label_2->setText(QApplication::translate("InteractiveClean", "cycles", 0, QApplication::UnicodeUTF8));
	label_3->setText(QApplication::translate("InteractiveClean", "threshold", 0, QApplication::UnicodeUTF8));
	label->setText(QApplication::translate("InteractiveClean", "iterations", 0, QApplication::UnicodeUTF8));
	niterED_->setText(QApplication::translate("InteractiveClean", "100", 0, QApplication::UnicodeUTF8));
	ncyclesED_->setText(QApplication::translate("InteractiveClean", "60", 0, QApplication::UnicodeUTF8));
	addRB_->setText(QApplication::translate("InteractiveClean", "Add", 0, QApplication::UnicodeUTF8));
	eraseRB_->setText(QApplication::translate("InteractiveClean", "Erase", 0, QApplication::UnicodeUTF8));
	maskNoMorePB_->setText(QString());
	stopPB_->setText(QString());
	maskDonePB_->setText(QString());
	thisPlaneRB_->setText(QApplication::translate("InteractiveClean", "Displayed Plane", 0, QApplication::UnicodeUTF8));
	allChanRB_->setText(QApplication::translate("InteractiveClean", "All Channels", 0, QApplication::UnicodeUTF8));

	dock->setWidget(widget);

	// Receives rectangle regions from the mouse tool.
	connect( displayPanel(), SIGNAL(mouseRegionReady(Record, WorldCanvasHolder*)),	
		 SLOT( newMouseRegion(Record, WorldCanvasHolder*)) );

	connect(stopPB_, SIGNAL(clicked()), this, SLOT(exitStop()));
	connect(maskDonePB_, SIGNAL(clicked()), this, SLOT(exitDone()));
	connect(maskNoMorePB_, SIGNAL(clicked()), this, SLOT(exitNoMore()));

	addDockWidget(Qt::TopDockWidgetArea, dock);

	show( );
	v_->autoDDOptionsShow = true;
    }


  QtCleanPanelGui::~QtCleanPanelGui() { }

#define EXIT_FUNC(NAME,STR,EXTRA)						\
    void QtCleanPanelGui::NAME() {						\
	QMap<QString,QVariant> state;						\
	state["action"] = STR;							\
	state["niter"] = niterED_->text().toInt();				\
	state["ncycle"] = ncyclesED_->text().toInt();				\
	state["threshold"] = threshED_->text();					\
	state["panel"] = interact_id;						\
	for ( std::list<QWidget*>::iterator iter = disabled_widgets.begin();	\
	      iter != disabled_widgets.end(); ++iter ) {			\
	    (*iter)->setPalette( default_palette );				\
	    (*iter)->setEnabled(false);						\
	}									\
	in_interact_mode = false;						\
	if ( maskdd_ != 0 ) maskdd_->unlock( );					\
	emit interact( QVariant(state) );					\
	EXTRA									\
    }


    EXIT_FUNC(exitDone,"continue",)
    EXIT_FUNC(exitNoMore,"no more",)
    EXIT_FUNC(exitStop,"stop",hide( );)

    bool QtCleanPanelGui::supports( SCRIPTING_OPTION option ) const {
	return option == INTERACT || option == SETOPTIONS ? true : false;
    }

    QVariant QtCleanPanelGui::setoptions(const QVariant &input, int id) {
	fprintf( stderr, "\t\t>>> QtCleanPanelGui::setoptions( )\n" );
	return QVariant(true);
    }

    QVariant QtCleanPanelGui::start_interact( const QVariant &input, int id ) {
	if ( ! in_interact_mode ) {
	    in_interact_mode = true;
	    interact_id = id;
	    for ( std::list<QWidget*>::iterator iter = disabled_widgets.begin();
		  iter != disabled_widgets.end(); ++iter ) {
		(*iter)->setPalette( input_palette );
		(*iter)->setEnabled(true);
	    }
	    raise( );
	    return QVariant(true);
	} else {
	    return QVariant(false);
	}
    }


    void QtCleanPanelGui::addedData( QString type, QtDisplayData *dd ) {
	if ( type == "contour" ) {
	    maskdd_ = dd;
	    ImageInterface<Float> *maskim=maskdd_->imageInterface();
	    csys_p=maskim->coordinates();
	    Int dirIndex=csys_p.findCoordinate(Coordinate::DIRECTION);
	    dirCoord_p=csys_p.directionCoordinate(dirIndex);
	} else if ( type == "raster" ) {
	    imagedd_ = dd;
	    Record opts;
	    opts.define("axislabelswitch", True);
	    imagedd_->setOptions(opts); 
	}
    }

    void QtCleanPanelGui::closeEvent(QCloseEvent *event) {
	// v_->exiting( ) is set when the gui user clicks quit() from
	// the menu bar... however, for the clean panel gui, we should
	// probably only exit when the script/c++ user invokes
	// closeMainPanel( ) via the close( ) slot on the
	// QtDBusViewerAdaptor... which sets isOverridedClose( )...
//	if ( ! v_->exiting( ) && ! isOverridedClose( ) ) {
	if ( ! isOverridedClose( ) ) {

	  
	  event->ignore( );
	  //hide( );	  
	  //That is what is meant most probably at this stage
	  exitStop();

	} else {
	    QtDisplayPanelGui::closeEvent(event);
	}
    }

    void QtCleanPanelGui::newMouseRegion(Record mouseRegion, WorldCanvasHolder* wch) {
	// Slot connected to the display panel's 'new mouse region' signal.
	// Accumulates [/ displays] selected boxes.

	// See QtMouseTools.cc for the current format of mouseRegion.
	// It is pretty much the same as the glish one at present, but
	// is not set in stone yet, and could be altered if desired.

	// To do: use the more-complete (true Region) info returned
	// by the specific image DD of interest instead.

	Float value=addRB_->isChecked()? 1.0 : 0.0;

	if( maskdd_ && imagedd_ && maskdd_->imageInterface() != 0 ){
	    //Lets see if we can change the mask
	    ImageRegion* imagereg=0;
	    try{
		imagereg=imagedd_->mouseToImageRegion( mouseRegion, wch, allChanRB_->isChecked() );

		ImageInterface<Float> *maskim=maskdd_->imageInterface();
		//Write the region as text...will need to add a box/toggle
		//to the viewer for that
		writeRegionText(*imagereg, maskim->name(), value);

		SubImage<Float> partToMask(*maskim, *imagereg, True);
		LatticeRegion latReg=imagereg->toLatticeRegion(csys_p, maskim->shape());
		ArrayLattice<Bool> pixmask(latReg.get());
		LatticeExpr<Float> myexpr(iif(pixmask, value, partToMask) );
		partToMask.copyData(myexpr);
		((*maskdd_).dd())->refresh(True);

	    } catch(...) { }

	    if(imagereg !=0) delete imagereg;
	}
	// Clears mouse tool drawings.	(Maskdd should show accumulated clean boxes instead).
	displayPanel()->resetSelectionTools();
    }

    void QtCleanPanelGui::writeRegionText(const ImageRegion& imageReg, const String& filename, Float value){
	//Write text of region
	if ( imagedd_ == 0 || imagedd_->imageInterface()==0) return;

	Record regRec(imageReg.asWCRegion().toLCRegion(csys_p,imagedd_->imageInterface()->shape())->toRecord(""));
	Vector<Float> x;
	Vector<Float> y;

	if(regRec.isDefined("name") && regRec.asString("name")=="LCIntersection"){
	    regRec=regRec.asRecord("regions").asRecord(0).asRecord("region");
	}

	if(regRec.isDefined("name") && regRec.asString("name")=="LCBox"){
	    x.resize();
	    y.resize();
	    x=Vector<Float>(regRec.asArrayFloat("blc"));
	    y=Vector<Float>(regRec.asArrayFloat("trc"));
	    x.resize(2,True);
	    y.resize(2,True);	
	}

	if(regRec.isDefined("name") && regRec.asString("name")=="LCPolygon"){
	    x.resize();
	    y.resize();
	    x=Vector<Float>(regRec.asArrayFloat("x"));
	    y=Vector<Float>(regRec.asArrayFloat("y"));	
	}

	if(regRec.isDefined("oneRel") && regRec.asBool("oneRel")){
	    x-=Float(1.0);
	    y-=Float(1.0);
	}		

	String textfile=filename+".text";
	ofstream outfile(textfile.c_str(), ios::out | ios::app);
	if(outfile){
	    outfile.precision(6);
	    outfile << floor(x)<<"		" << floor(y) << "		 "<< Int(value) << endl;
	}
	//End of write text
    }

} //# NAMESPACE CASA - END
