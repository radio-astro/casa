//# QtDataManager.cc: Qt implementation of viewer data manager widget.
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
#include <display/QtViewer/QtDataManager.qo.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/region/Region.qo.h>
#include <display/Display/Options.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableInfo.h>
#include <images/Images/FITSImgParser.h>
#include <display/QtViewer/SlicerGen.qo.h>
#include "fitsio.h"
#include <casa/BasicSL/String.h>
#include <casa/OS/File.h>
#include <casa/iostream.h>
#include <casa/fstream.h>
#include <casa/Exceptions/Error.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>

#include <images/Images/PagedImage.h>		/*** needed for global imagePixelType( ) ***/
#include <images/Images/ImageFITSConverter.h>
#include <sstream>

#include <casa/Utilities/GenSort.h>

#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QMessageBox>
#include <QDirModel>
#include <QHash>
#include <QSettings>
#include <QTextStream>
#include <graphics/X11/X_exit.h>

#include <casa/cppconfig.h>

using std::map;
using std::set;
using std::get;

namespace casa { //# NAMESPACE CASA - BEGIN

	struct dismiss_button_t : public QPushButton {
		dismiss_button_t( QTableWidgetItem *i, QString u, QString t, QString s, QWidget *parent=0 ) : QPushButton(parent), item(i), url(u),
                                                                                                      file_type(t), service(s) { }
		~dismiss_button_t( ) { }
		QTableWidgetItem *item;
		QString url;
		QString path;
		QString file_type;
		QString display_type;
		QString service;
	};

// forward declare...
	namespace viewer {
		std::string canonical_path( const std::string & );

        namespace dvo {
            void param::mousePressEvent ( QMouseEvent * event ) {
                 list->setCurrentItem(item);
                 edit->setFocus( );
            }

        }
	}

	QtDataManager::QtDataManager(QtDisplayPanelGui* panel, const char *name, QWidget *parent ) :
		QWidget(parent), parent_(parent), panel_(panel),
		ms_selection(new Ui::QtDataMgrMsSelect), rc(viewer::getrc()),
        slice_available(true), regrid_available(true), dvo("edu.nrao.casa.dVO", "/casa/dVO", QDBusConnection::sessionBus( ), 0),
		vo_current_action(0), vo_action_timeout(new QTimer( )), vo_actions_are_enabled(true) {

		setWindowTitle(name);

		setupUi(this);

        setupVO( );

		connect( vo_action_timeout, SIGNAL(timeout( )), SLOT(vo_action_timed_out( )) );

		slice_gen = new viewer::SlicerGen( );
		slice_gen->initialize(slice_frame);
		slice_frame->setFrameStyle(QFrame::NoFrame);
        connect( slice_gen, SIGNAL(stateChange(bool)), SLOT(enable_disable_regrid(bool)) );

		ms_selection->setupUi(ms_selection_scroll_widget);
		connect(cancelButton_, SIGNAL(clicked( )), SLOT(close( )));
		connect(tabs, SIGNAL(currentChanged(int)), SLOT(changeTabContext(int)));

		// start out with the regrid combo-box hidden...
		regrid->hide( );
        connect( regrid_method, SIGNAL(currentIndexChanged(const QString&)), SLOT(enable_disable_slice(const QString&)) );

		load_ifields.push_back(infofield_list_t::value_type(load_ibox11,load_itext11));
		load_ifields.push_back(infofield_list_t::value_type(load_ibox12,load_itext12));
		load_ifields.push_back(infofield_list_t::value_type(load_ibox21,load_itext21));
		load_ifields.push_back(infofield_list_t::value_type(load_ibox22,load_itext22));
		load_ifields.push_back(infofield_list_t::value_type(load_ibox31,load_itext31));
		load_ifields.push_back(infofield_list_t::value_type(load_ibox32,load_itext32));
		load_ifields.push_back(infofield_list_t::value_type(load_ibox41,load_itext41));
		load_ifields.push_back(infofield_list_t::value_type(load_ibox42,load_itext42));

		// 	img_ifields.push_back(infofield_list_t::value_type(img_ibox11,img_itext11));
		// 	img_ifields.push_back(infofield_list_t::value_type(img_ibox12,img_itext12));
		// 	img_ifields.push_back(infofield_list_t::value_type(img_ibox21,img_itext21));
		// 	img_ifields.push_back(infofield_list_t::value_type(img_ibox22,img_itext22));
		// 	img_ifields.push_back(infofield_list_t::value_type(img_ibox31,img_itext31));
		// 	img_ifields.push_back(infofield_list_t::value_type(img_ibox32,img_itext32));
		// 	img_ifields.push_back(infofield_list_t::value_type(img_ibox41,img_itext41));
		// 	img_ifields.push_back(infofield_list_t::value_type(img_ibox42,img_itext42));

#if defined(__APPLE__)
		QFont field_font( "Lucida Grande", 10 );
#else
		QFont field_font( "Sans Serif", 7 );
#endif
		for ( infofield_list_t::iterator it = load_ifields.begin( ); it != load_ifields.end( ); ++it ) {
			(*it).first->setTitle(" ");
			(*it).second->clear( );
			(*it).second->setFont( field_font );
		}

		// 	for ( infofield_list_t::iterator it = img_ifields.begin( ); it != img_ifields.end( ); ++it ) {
		// 	    (*it).first->setTitle(" ");
		// 	    (*it).second->clear( );
		// 	    (*it).second->setFont( field_font );
		// 	}

		std::string show_lel = rc.get("viewer." + panel_->rcid() + ".datamgr.show_lel");
		if ( show_lel != "true" && show_lel != "false" ) {
			rc.put( "viewer." + panel_->rcid() + ".datamgr.show_lel", "false" );
			show_lel = "false";
		}
		std::string do_show_slice = rc.get("viewer." + panel_->rcid() + ".datamgr.show_slice");
		if ( do_show_slice != "true" && do_show_slice != "false" ) {
			rc.put( "viewer." + panel_->rcid() + ".datamgr.show_slice", "false" );
			do_show_slice = "false";
		}

		std::string leave_up = rc.get("viewer." + panel_->rcid() + ".datamgr.leave_up");
		if ( leave_up != "true" && leave_up != "false" ) {
			rc.put( "viewer." + panel_->rcid() + ".datamgr.leave_up", "true" );
			leave_up = "true";
		}

		// Tue Sep  6 12:17:21 EDT 2011
		// don't think people really use the lel entry, and while it's removal wasn't
		// required (with the addition of the ms selection to the "loading options"),
		// originally the lel expression was supposed to be replace with ms selection
		// options... so we'll remove it and see what happens <drs>...
		// Fri Nov 18 22:39:26 UTC 2011
		// some complaints make it reasonable to re-enable it as an option <drs>...
		if ( show_lel == "false" ) {
			lelGB_->hide( );
			showLEL_->setChecked( false );
		} else {
			showLEL_->setChecked( true );
		}

		if ( do_show_slice == "false" ) {
			slice_frame->hide( );
			show_slice->setChecked( false );
		} else {
			show_slice->setChecked( true );
		}

		connect(showLEL_, SIGNAL(clicked(bool)), SLOT(showlelButtonClicked(bool)));
		connect(show_slice, SIGNAL(clicked(bool)), SLOT(showSliceButtonClicked(bool)));

		//updateButton_->setEnabled(false);	//#dk until this works.

		lelEdit_->setToolTip( "Enter an image expression, such as\n"
		                      "'clean.im' - 'dirty.im'.  For details, see:\n"
		                      "aips2.nrao.edu/docs/notes/223/223.html" );

		hideDisplayButtons();

		uiDataType_["Unknown"]          = UNKNOWN;
		uiDataType_["Image"]            = IMAGE;
		uiDataType_["Measurement Set"]  = MEASUREMENT_SET;
		uiDataType_["Sky Catalog"]      = SKY_CATALOG;
		uiDataType_["Directory"]        = DIRECTORY;
		uiDataType_["FITS Image"]       = IMAGE;
		uiDataType_["FITS Ext."]        = IMAGE;
		uiDataType_["Quality Ext."]     = QUALIMG;
		uiDataType_["Miriad Image"]     = IMAGE;
		uiDataType_["Gipsy"]            = IMAGE;
		uiDataType_["Restore File"]     = RESTORE;
		uiDataType_["CASA Region File"] = CASAREGION;
		uiDataType_["DS9 Region File"] = DS9REGION;

		dataType_[UNKNOWN]              = "unknown";
		dataType_[IMAGE]                = "image";
		dataType_[QUALIMG]              = "image";
		dataType_[MEASUREMENT_SET]      = "ms";
		dataType_[SKY_CATALOG]          = "skycatalog";
		dataType_[RESTORE]              = "restore";
		dataType_[CASAREGION]           = "casa region";
		dataType_[DS9REGION]           = "ds9 region";

		uiDisplayType_["raster image"]  = RASTER;
		uiDisplayType_["contour map"]   = CONTOUR;
		uiDisplayType_["vector map"]    = VECTOR;
		uiDisplayType_["marker map"]    = MARKER;
		uiDisplayType_["sky catalog"]   = SKY_CAT;
		uiDisplayType_["old window"]    = OLDPANEL;
		uiDisplayType_["new window"]    = NEWPANEL;

		displayType_["raster"]          = RASTER;
		displayType_["contour"]         = CONTOUR;
		displayType_["vector"]          = VECTOR;
		displayType_["marker"]          = MARKER;
		displayType_["skycatalog"]      = SKY_CAT;
		displayType_["oldpanel"]        = OLDPANEL;
		displayType_["newpanel"]        = NEWPANEL;

		// define the accepted export types
		exportTypes_ = QVector<int>(1, IMAGE);

		leaveOpen_->setToolTip( "Uncheck to close this window after "
		                        "data and display type selection.\n"
		                        "Use 'Open' button/menu on Display Panel to show it again." );

		if ( leave_up == "true" ) {
			leaveOpen_->setChecked(true);
		} else {
			leaveOpen_->setChecked(false);
		}
		connect(leaveOpen_, SIGNAL(clicked(bool)), SLOT(leaveopenButtonClicked(bool)));

		buildDirTree();

		connect(rasterButton_,      SIGNAL(clicked()), SLOT(createButtonClicked()));
		connect(contourButton_,     SIGNAL(clicked()), SLOT(createButtonClicked()));
		connect(vectorButton_,      SIGNAL(clicked()), SLOT(createButtonClicked()));
		connect(markerButton_,      SIGNAL(clicked()), SLOT(createButtonClicked()));
		connect(catalogButton_,     SIGNAL(clicked()), SLOT(createButtonClicked()));
		connect(oldPanelButton_,    SIGNAL(clicked()), SLOT(restoreToOld_()));
		connect(newPanelButton_,    SIGNAL(clicked()), SLOT(restoreToNew_()));
		connect(updateButton_,      SIGNAL(clicked()), SLOT(buildDirTree()));
		connect(regionButton_,      SIGNAL(clicked()), SLOT(load_regions_clicked()));

		//  connect(registerCheck,      SIGNAL(clicked()), SLOT(registerClicked()));

		connect(load_directory,     SIGNAL(returnPressed()), SLOT(returnPressed()));
		connect(image_directory,    SIGNAL(returnPressed()), SLOT(returnPressed()));
		connect(region_directory,    SIGNAL(returnPressed()), SLOT(returnPressed()));

		connect( load_tree_widget_, SIGNAL(itemSelectionChanged()),            SLOT(changeItemSelection()));
		connect( load_tree_widget_, SIGNAL(itemClicked(QTreeWidgetItem*,int)),SLOT(clickItem(QTreeWidgetItem*)));
		connect( load_tree_widget_, SIGNAL(itemExpanded(QTreeWidgetItem*)),   SLOT(expandItem(QTreeWidgetItem*)));

		connect( image_file_list_, SIGNAL(itemSelectionChanged()),            SLOT(changeItemSelection()));
		connect( image_file_list_, SIGNAL(itemClicked(QTreeWidgetItem*,int)),SLOT(clickItem(QTreeWidgetItem*)));
		connect( image_file_list_, SIGNAL(itemExpanded(QTreeWidgetItem*)),   SLOT(expandItem(QTreeWidgetItem*)));

		connect( region_file_list, SIGNAL(itemSelectionChanged()),            SLOT(changeItemSelection()));
		connect( region_file_list, SIGNAL(itemClicked(QTreeWidgetItem*,int)),SLOT(clickItem(QTreeWidgetItem*)));
		connect( region_file_list, SIGNAL(itemExpanded(QTreeWidgetItem*)),   SLOT(expandItem(QTreeWidgetItem*)));

		connect(panel_,             SIGNAL(createDDFailed(String, String, String, String)), SLOT(showDDCreateError_(String)));

		connect(lelEdit_,           SIGNAL(gotFocus(QFocusEvent*)),       SLOT(lelGotFocus_()));

		connect(img_output_name,    SIGNAL(textChanged(const QString & )), SLOT(img_output_target_changed(const QString &)));
		connect(image_directory,    SIGNAL(textChanged(const QString & )), SLOT(img_output_target_changed(const QString &)));
		connect(save_casa_img,      SIGNAL(toggled(bool)),                 SLOT(img_output_target_changed(bool)));
		connect(save_fits_img,      SIGNAL(toggled(bool)),                 SLOT(img_output_target_changed(bool)));
		connect(img_do_save,        SIGNAL(clicked( )),                    SLOT(img_do_output( )));

		connect(region_output_name, SIGNAL(textChanged(const QString & )), SLOT(region_output_target_changed(const QString &)));
		connect(region_directory,   SIGNAL(textChanged(const QString & )), SLOT(region_output_target_changed(const QString &)));
		connect(save_casa_region,   SIGNAL(toggled(bool)),                 SLOT(region_output_target_changed(bool)));
		connect(save_ds9_region,    SIGNAL(toggled(bool)),                 SLOT(region_output_target_changed(bool)));
		connect(region_do_save,     SIGNAL(clicked( )),                    SLOT(region_do_output( )));

		save_ds9_csys->setDisabled(true);
		connect(save_ds9_region,    SIGNAL(toggled(bool)),                 SLOT(region_ds9_csys_disable(bool)));
	}

	QtDataManager::~QtDataManager() {
		delete vo_action_timeout;
	}


	void QtDataManager::showTab( std::string tab ) {
		QString qtab = QString::fromStdString(tab);
		for ( int i=0; i < tabs->count( ); ++i ) {
			if ( tabs->tabText(i) == qtab ) {
				tabs->setCurrentIndex(i);
				break;
			}
		}
	}


	void QtDataManager::clickItem(QTreeWidgetItem* item) {
		// make sure a directory was clicked
		if(item!=0 && item->text(1)=="Directory") {

			// get the text
			QString iText = item->text(0);

			// if there is a text go to that directory
			if (iText.size()>0) {
				updateDirectory(item->text(0).toStdString( ));
			} else if (iText.size()==0) {
				// if there is no text
				// get the top-level index
				int index = load_tree_widget_->indexOfTopLevelItem (item );

				// go to "home" or "root"
				if (index == 0)
					updateDirectory(QDir::homePath( ).toStdString( ));
				else if (index == 1)
					updateDirectory(QDir::rootPath( ).toStdString( ));
			}
		}
	}


	static int findNumberOfFITSImageExt( QString path ) {
		fitsfile *fptr;
		int status = 0;
		fits_open_file( &fptr, path.toAscii( ).constData( ), READONLY, &status );
		if ( status != 0 ) {
			fits_report_error(stderr, status);
			return -1;
		}
		int number_hdus = 0;
		fits_get_num_hdus( fptr, &number_hdus, &status );
		if ( status != 0 ) {
			fits_report_error(stderr, status);
			fits_close_file( fptr, &status );
			return -1;
		}
		int number_images = 0;
		if ( number_hdus > 0 ) {
			int type = 0;
			fits_movabs_hdu( fptr, 1, &type, &status );
			if ( status != 0 ) {
				fits_report_error(stderr, status);
				fits_close_file( fptr, &status );
				return -1;
			}
			if ( type == IMAGE_HDU ) ++number_images;
			for ( int i=2; i <= number_hdus; ++i ) {
				type = 0;
				fits_movrel_hdu(fptr, 1, &type, &status);
				if ( status != 0 ) {
					fits_report_error(stderr, status);
					fits_close_file( fptr, &status );
					return -1;
				}
				if ( type == IMAGE_HDU ) ++number_images;
			}
		}

		fits_close_file( fptr, &status );
		if ( status != 0 ) {
			fits_report_error(stderr, status);
			return -1;
		}
		return number_images;
	}


	void QtDataManager::expandItem(QTreeWidgetItem* item) {
		if ( item->text(1) == "FITS Image" && item->childCount( ) == 1 ) {
			// check whether its a FITS image and prevent second generation children
			if(item!=0 && item->text(1)=="FITS Image" && !item->text(0).endsWith("]")) {
				delete item->takeChild(0);

				if ( tab_info.size( ) == 0 ) init_tab_info( );
				tab_state ts = tab_info[tabs->currentIndex( )];

				QString path = ts.dir( )->path( ) + "/" +  item->text(0);

				// get a list of all extensions with data
				QStringList extList = analyseFITSImage(path);

				// if there is more than one extension
				if (extList.size()>1) {
					QTreeWidgetItem *childItem;
					int dType = uiDataType_[item->text(1)];

					// add the extensions as child items
					for (int j = 0; j < extList.size(); j+=2) {
						childItem = new QTreeWidgetItem(item);
						childItem->setText(0, extList.at(j));
						childItem->setText(1, extList.at(j+1));
						dType = uiDataType_[childItem->text(1)];
						childItem->setTextColor(1, getDirColor(dType));
					}
				}
				load_tree_widget_->resizeColumnToContents(0);
			}
		}
	}


	void QtDataManager::updateDirectory(const std::string &str) {
		if ( tab_info.size( ) == 0 ) init_tab_info( );
		tab_state ts = tab_info[tabs->currentIndex( )];

		QString qstr = QString::fromStdString(str);
		QDir saved = *ts.dir( );
		if( ! ts.dir( )->cd(qstr)) {
			QMessageBox::warning( this, tr("QtDataManager"), tr("No such directory:\n %1").arg(qstr));
			*ts.dir( ) = saved;
		}
		if(ts.dir( )->entryList( ).size( ) == 0) {
			QMessageBox::warning(this, tr("QtDataManager"), tr("Could not enter the directory:\n %1").arg(ts.dir( )->path( )));
			*ts.dir( ) = saved;
		}
		ts.dir( )->makeAbsolute( );
		ts.dirline( )->setText(ts.dir( )->cleanPath(ts.dir( )->path( )));

		std::string newpath = ts.dirline( )->text().toStdString();
		panel_->selectedDMDir = newpath;
		buildDirTree( newpath );
	}


	void QtDataManager::load_tab_notify( const std::string &value, tab_state &ts ) {
		if ( value == "building" ) {
			if ( ts.tree( )->columnCount( ) == 1 ) {
				ts.updateDir( );
				QStringList lbl;
				lbl << "input file" << "type";
				ts.tree( )->setColumnCount(2);
				ts.tree( )->setHeaderLabels(lbl);
			}
			lelEdit_->deactivate();
			hideDisplayButtons();
			ts.tree( )->setFocus(Qt::OtherFocusReason);
		} else if ( value == "selection" ) {
			QList<QTreeWidgetItem *> lst = load_tree_widget_->selectedItems();
			if (!lst.empty()) {
				QTreeWidgetItem *item = (QTreeWidgetItem*)(lst.at(0));
				showDisplayButtons(uiDataType_[item->text(1)],item->text(0));
				lelEdit_->deactivate();
				update_regrid_options( );
				update_slice_options(uiDataType_[item->text(1)], item->text(0));
			}

		}
	}

	void QtDataManager::clearRegionMap(){
		typedef QtDisplayPanelGui::region_list_t region_list_t;
		region_list_t regions = panel_->regions( );
		if ( regions.size() > 0 ){
			for ( region_map_t::iterator it=region_to_treeitem.begin( ); it != region_to_treeitem.end( ); ++it ) {
				//          fprintf( stderr, "\t>>>>>> 0x%x\n", it->first );
				disconnect( it->first, 0, this, 0 );
			}
		}

		treeitem_to_region.clear( );
		region_to_treeitem.clear( );
	}

	void QtDataManager::update_region_list( tab_state &ts ) {
		typedef QtDisplayPanelGui::region_list_t region_list_t;
		clearRegionMap();
		ts.dtree( )->clear( );
		ostringstream oss;
		oss << fixed << setprecision(0);
		region_list_t regions = panel_->regions( );
		for ( region_list_t::iterator it=regions.begin( ); it != regions.end( ); ++it ) {
			QTreeWidgetItem *item = new QTreeWidgetItem(ts.dtree( ));
			item->setText( 0, QString::fromStdString((*it)->name( )) );
			item->setText( 1, QString::fromStdString((*it)->lineColor( )) );
			double x,y;
			(*it)->pixelCenter(x,y);
			oss.str("");
			oss << x << "," << y;
			item->setText( 2, QString::fromStdString(oss.str( )) );
			if ( (*it)->marked( ) )
				item->setCheckState(0,Qt::Checked);
			else
				item->setCheckState(0,Qt::Unchecked);

			treeitem_to_region.insert(treeitem_map_t::value_type(item,*it));
			region_to_treeitem.insert(region_map_t::value_type(*it,item));
			// connect(*it,        SIGNAL(selectionChanged(viewer::Region*,bool)),  this, SLOT(region_selection_change(viewer::Region*,bool)));
		}
		ts.dtree( )->resizeColumnToContents(0);
	}

	void QtDataManager::region_selection_change( viewer::Region *rgn, bool selected ) {
		region_map_t::iterator it = region_to_treeitem.find(rgn);
		if ( it != region_to_treeitem.end( ) ) {
			if ( selected )
				it->second->setCheckState(0,Qt::Checked);
			else
				it->second->setCheckState(0,Qt::Unchecked);
		}
	}

	void QtDataManager::region_item_state_change( QTreeWidgetItem *item, int /*col*/ ) {
#if 0
		if ( item && col > 0 && item->isSelected( ) ) {
			item->setSelected(false);
			if ( item->checkState(0) == Qt::Checked )
				item->setCheckState(0,Qt::Unchecked);
			else
				item->setCheckState(0,Qt::Checked);
			// 	    return;
		}
#endif
		treeitem_map_t::iterator it = treeitem_to_region.find(item);
		if ( it != treeitem_to_region.end( ) ) {
			if ( item->checkState(0) == Qt::Checked )
				it->second->mark( true );
			else
				it->second->mark( false );
		}
#if 0
		region_output_target_changed( );
#endif
	}


	void QtDataManager::update_dd_list( tab_state &ts ) {
		// retrieve a copy of the current DD list.
		//List<QtDisplayData*> qdds_ = panel_->dds();
		//if ( qdds_.len( ) == 0 ) {
		if ( panel_->isEmptyDD()) {
			ts.notifyErr(this,"no data to save");
			return;
		}

		// iterate over all list members
		display_datas.clear( );
		ts.dtree( )->clear( );
		//for(ListIter<QtDisplayData*> qdds(qdds_); !qdds.atEnd(); qdds++) {
		//QtDisplayData* qdd = qdds.getRight();
		DisplayDataHolder::DisplayDataIterator iter = panel_->beginDD();
		while ( iter != panel_->endDD()) {
			QtDisplayData* qdd = (*iter);
			iter++;
			if ( qdd != NULL ) {
				int type    = dataType_.key(QString::fromStdString(qdd->dataType( )));
				if (exportTypes_.contains(type)) {
					// 		dtype  = displayType_.key(QString::fromStdString(qdd->displayType()));

					QString fileName    = QString((qdd->name()).c_str());
					QString filePath    = QString((qdd->path()).c_str());

					if (filePath.endsWith("]") && filePath.lastIndexOf(".fits[") >0) {
						int lIndex=filePath.lastIndexOf(".fits[");
						filePath.replace(lIndex, filePath.size()-lIndex, ".fits");
					}


					QTreeWidgetItem *fileItem = new QTreeWidgetItem(ts.dtree( ));
					fileItem->setText(0, fileName);
					fileItem->setToolTip(0, filePath);
					// 		fileItem->setText(1, uiDataType_.key(type));
					fileItem->setText(1, QString::fromStdString(qdd->displayType( )));
					fileItem->setTextColor(1, getDirColor(type));
					display_datas.insert(display_data_map_t::value_type(fileName,qdd));
				}
			}
		}
		ts.dtree( )->resizeColumnToContents(0);

		// activate the last entry
		if (ts.dtree( )->topLevelItemCount()>0) {
			ts.dtree( )->setCurrentItem (ts.dtree( )->topLevelItem(ts.dtree( )->topLevelItemCount()-1));
		}
	}


	void QtDataManager::image_tab_error( const std::string &value, tab_state & ) {
		img_output_error->setText(QString::fromStdString(value));
		img_do_save->setEnabled(false);
	}

	void QtDataManager::image_tab_notify( const std::string &value, tab_state &ts ) {
		if ( value == "building" ) {
			if ( ts.tree( )->columnCount( ) == 1 ) {
				ts.updateDir( );
				QStringList lbl;
				lbl << "output file" << "type";
				ts.tree( )->setColumnCount(2);
				ts.tree( )->setHeaderLabels(lbl);
			}
			if ( ts.dtree( )->columnCount( ) == 1 ) {
				QStringList lbl;
				lbl << "image name" << "type";
				ts.dtree( )->setColumnCount(2);
				ts.dtree( )->setHeaderLabels(lbl);
				update_dd_list( ts );
			}
		} else if ( value == "selection" ) {
			QList<QTreeWidgetItem *> lst = ts.tree( )->selectedItems();
			if (!lst.empty()) {
				QTreeWidgetItem *item = (QTreeWidgetItem*)(lst.at(0));
				if ( item->text(1) == "Directory" ) {
					ts.updateDir( );
				} else {
					ts.outFileLine( )->setText( item->text(0) );
				}
			}
		} else if ( value == "update data" ) {
			update_dd_list( ts );
			img_output_target_changed( );
		}
	}

	void QtDataManager::region_tab_error( const std::string &value, tab_state & ) {
		region_output_error->setText(QString::fromStdString(value));
		region_do_save->setEnabled(false);
	}

	void QtDataManager::handle_region_update(viewer::Region*,std::string) {
		for ( int i=0; i < tabs->count( ); ++i ) {
			if ( tabs->tabText(i) == "save region" ) {
				update_region_list( tab_info[i] );
				break;
			}
		}
	}

	void QtDataManager::region_tab_notify( const std::string &value, tab_state &ts ) {
		if ( value == "building" ) {
			if ( ts.tree( )->columnCount( ) == 1 ) {
				ts.updateDir( );
				QStringList lbl;
				lbl << "output file" << "type";
				ts.tree( )->setColumnCount(2);
				ts.tree( )->setHeaderLabels(lbl);
				connect( panel_, SIGNAL(regionChange(viewer::Region*,std::string )), SLOT(handle_region_update(viewer::Region*,std::string)));
			}
			if ( ts.dtree( )->columnCount( ) == 1 ) {
				QStringList lbl;
				lbl << "region type" << "color" << "pixel center";
				ts.dtree( )->setColumnCount(3);
				ts.dtree( )->setHeaderLabels(lbl);
				update_region_list( ts );
				connect(ts.dtree( ),SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(region_item_state_change(QTreeWidgetItem*,int)));
			}
		} else if ( value == "selection" ) {
			QList<QTreeWidgetItem *> lst = ts.tree( )->selectedItems();
			if (!lst.empty()) {
				QTreeWidgetItem *item = (QTreeWidgetItem*)(lst.at(0));
				if ( item->text(1) == "Directory" ) {
					ts.updateDir( );
				} else {
					if ( item->text(1) == "CASA Region File" )
						save_casa_region->setChecked(true);
					else if ( item->text(1) == "DS9 Region File" )
						save_ds9_region->setChecked(true);
					ts.outFileLine( )->setText( item->text(0) );
				}
			}
		} else if ( value == "update data" ) {
			update_region_list( ts );
			//img_output_target_changed( );
		}
	}

	void QtDataManager::updateDisplayDatas(QtDisplayData*, Bool ) {
		tab_info_map_t::iterator it=tab_info.find(tabs->currentIndex( ));
		if ( it != tab_info.end( ) ) it->second.notify(this,"update data");
	}

	void QtDataManager::buildDirTree( std::string /*newpath*/ ) {

		if ( tab_info.size( ) == 0 ) init_tab_info( );
		tab_state ts = tab_info[tabs->currentIndex( )];

		if ( ts.tree( ) == 0 ) {
			return;
		}
		ts.tree( )->clear();

		panel_->selectedDMDir = ts.dirStr( );
		ts.notify(this,"building");

		QTreeWidgetItem *dirItem;
		ts.dir( )->makeAbsolute( );
		QStringList entryList = ts.dir( )->entryList();

		// create and add an item for the home directory
		QString type = "Directory";
		int dType = uiDataType_[type];
		dirItem = new QTreeWidgetItem();
		dirItem->setIcon(0, QIcon(":/icons/home_folder.png"));
		dirItem->setToolTip(0, QString("Home directory"));
		dirItem->setText(1, type);
		dirItem->setTextColor(1, getDirColor(dType));
		ts.tree( )->insertTopLevelItem (0, dirItem );

		// create and add an item for the root directory
		type = "Directory";
		dType = uiDataType_[type];
		dirItem = new QTreeWidgetItem();
		dirItem->setIcon(0, QIcon(":/icons/root_folder.png"));
		dirItem->setToolTip(0, QString("Root directory"));
		dirItem->setText(1, type);
		dirItem->setTextColor(1, getDirColor(dType));
		ts.tree( )->insertTopLevelItem (1, dirItem );


		QTreeWidgetItem *selection = 0;
		QtDisplayData *dd = panel_->dd( );
		QString ddpath;
		if ( dd ) {
			QFileInfo path(QString::fromStdString(dd->path( )));
			ddpath = path.canonicalFilePath( );
		}

		for (int i = 0; i < entryList.size(); i++) {
			QString it = entryList.at(i);
			if (it.compare(".") > 0) {
				QString path = ts.dir( )->path( ) + "/" +  entryList.at(i);
				type = panel_->viewer( )->fileType(path.toStdString()).chars();
				dType = uiDataType_[type];

				if (dType!=UNKNOWN) {
					if ( ts.filter( ).find(dType) != ts.filter( ).end( ) )
						continue;
					dirItem = new QTreeWidgetItem(ts.tree( ));
					dirItem->setText(0, it);
					dirItem->setText(1, type);
					dirItem->setTextColor(1, getDirColor(dType));
					if ( type == "FITS Image" && findNumberOfFITSImageExt( path ) > 1 ) {
						QTreeWidgetItem *childItem = new QTreeWidgetItem(dirItem);
						childItem->setText(0, "");
						childItem->setText(1, "");
						childItem->setTextColor(1, getDirColor(dType));
					}
					if ( selection == 0 && dd && ddpath == QFileInfo(path).canonicalFilePath( ) )
						selection = dirItem;
				}
			}
		}

		// QSettings settings("NRAO", "casa");
		// //cout << "dir_.path()=" << dir_.path().toStdString() << endl;
		// settings.setValue("lastDir", dir_.path());

		ts.tree( )->resizeColumnToContents(0);
		// load_tree_widget_->setColumnWidth(0, 200);   // (only suppotted in Qt 4.2+)

		if ( selection ) {
			ts.tree( )->setCurrentItem(selection);
			changeItemSelection( );
			ts.tree( )->scrollToItem(selection,QAbstractItemView::PositionAtCenter);
		}

	}


	void QtDataManager::changeItemSelection() {
        regrid_available = slice_available = true;
		if ( tab_info.size( ) == 0 ) init_tab_info( );
		tab_state ts = tab_info[tabs->currentIndex( )];
		ts.notify(this,"selection");
	}

	struct strip_chars {
		// should generalize to strip 'chars'...
		strip_chars( const char */*chars*/ ) { }
		strip_chars(const strip_chars &other) : str(other.str) { }
		operator std::string( ) const {
			return str;
		}
		void operator( )( const char &c ) {
			if ( c != '[' && c != ']' ) str += c;
		}
	private:
		std::string str;
	};

	struct max_ftor {
		max_ftor( ) : max(-FLT_MAX) { }
		max_ftor( const max_ftor &other ) : max(other.max) { }
		operator float( ) const {
			return max;
		}
		void operator( )( float f ) {
			if ( f > max ) max = f;
		}
	private:
		float max;
	};

	struct min_ftor {
		min_ftor( ) : min(FLT_MAX) { }
		min_ftor( const min_ftor &other ) : min(other.min) { }
		operator float( ) const {
			return min;
		}
		void operator( )( float f ) {
			if ( f < min ) min = f;
		}
	private:
		float min;
	};

    void QtDataManager::showEvent( QShowEvent *event ) {
        QWidget::showEvent(event);
        updateVOstatus( );
    }
    void QtDataManager::enterEvent( QEvent  *event ) {
        QWidget::enterEvent(event);
        updateVOstatus( );
    }

	void QtDataManager::showDisplayButtons(int ddtp,const QString &name) {
		hideDisplayButtons();

		if ( tab_info.size( ) == 0 ) init_tab_info( );
		tab_state ts = tab_info[tabs->currentIndex( )];

		switch (ddtp) {
		case IMAGE :
			rasterButton_->show();
			contourButton_->show();
			vectorButton_->show();
			markerButton_->show();
			if ( load_tree_widget_->currentItem() && dataType_.value(uiDataType_[load_tree_widget_->currentItem()->text(1)]) == "image" )
				load_info_box->show();
			if ( ! name.isNull( ) ) {
				std::string path = (ts.dir( )->path( ) + "/" + name).toStdString( );
				fill_image_info( path );
			}
			break;
		case MEASUREMENT_SET :
			rasterButton_->show();
			ms_selection_box->show();
			break;
		case SKY_CATALOG:
			catalogButton_->show();
			break;
		case RESTORE:
			oldPanelButton_->show();
			newPanelButton_->show();
			break;
		case QUALIMG:
			rasterButton_->show();
			contourButton_->show();
			vectorButton_->show();
			markerButton_->show();
			break;
		case CASAREGION:
			if ( panel_->useNewRegions( ) ) {
				if ( panel_->isEmptyDD()) {
					//if ( panel_->nDDs( ) == 0 )
					regionButton_->setDisabled(true);
				} else {
					regionButton_->setDisabled(false);
				}
				regionButton_->show( );
			}
			break;
		case DS9REGION:
			if ( panel_->useNewRegions( ) ) {
				//if ( panel_->nDDs( ) == 0 )
				if ( panel_->isEmptyDD() ) {
					regionButton_->setDisabled(true);
				} else {
					regionButton_->setDisabled(false);
				}
				regionButton_->show( );
			}
			break;
		}
	}

	QColor QtDataManager::getDirColor(int ddtp) {
		QColor clr;
		switch (ddtp) {
		case IMAGE:
			clr = Qt::darkGreen;
			break;
		case MEASUREMENT_SET:
			clr = Qt::darkBlue;
			break;
		case SKY_CATALOG:
			clr = Qt::darkCyan;
			break;
		case RESTORE:
			clr = QColor(255,43,45);
			break;
		case DIRECTORY:
			clr = Qt::black;
			break;
		case QUALIMG:
			clr = Qt::darkRed;
			break;
		case CASAREGION:
			clr = Qt::darkYellow;
			break;
		case DS9REGION:
			clr = QColor(255,153,51);
			break;
		case UNKNOWN:
		default:
			clr = Qt::darkMagenta;
		}

		return clr;
	}

	void QtDataManager::hideDisplayButtons() {
		rasterButton_->hide();
		contourButton_->hide();
		vectorButton_->hide();
		markerButton_->hide();
		catalogButton_->hide();
		oldPanelButton_->hide();
		newPanelButton_->hide();
		regionButton_->hide();
		ms_selection_box->hide();
		load_info_box->hide();
		// 	img_info_box->hide();
	}


	void QtDataManager::returnPressed() {
		if ( tab_info.size( ) == 0 ) init_tab_info( );
		tab_state ts = tab_info[tabs->currentIndex( )];
		updateDirectory(ts.dirStr( ));
	}


	void QtDataManager::createButtonClicked() {

		if ( tab_info.size( ) == 0 ) init_tab_info( );
		tab_state ts = tab_info[tabs->currentIndex( )];

		QPushButton* button = dynamic_cast<QPushButton*>(sender());

		if(panel_==0 || button==0) return;

		String path, datatype, displaytype;

		displaytype = (displayType_.key(uiDisplayType_[button->text()])).toStdString();

		if(lelEdit_->isActive()) {
			// Display LEL expression.
			path = lelEdit_->text().trimmed().toStdString();
			datatype = "lel";
		} else if (load_tree_widget_->currentItem() > 0) {
			// Display selected file.
			path = (ts.dir( )->path( ) + "/" + load_tree_widget_->currentItem()->text(0)).toStdString( );
			datatype = dataType_.value(uiDataType_[load_tree_widget_->currentItem()->text(1)]).toStdString();
		}

		if(path=="" || datatype=="" || displaytype=="") return;

		viewer::DisplayDataOptions ddo;
		if ( datatype == "ms" ) {
			if ( ms_selection->select_field->text( ) != "" )
				ddo.insert( "field", ms_selection->select_field->text( ).toStdString( ) );
			if ( ms_selection->select_spw->text( ) != "" )
				ddo.insert( "spw", ms_selection->select_spw->text( ).toStdString( ) );
			if ( ms_selection->select_time->text( ) != "" )
				ddo.insert( "time", ms_selection->select_time->text( ).toStdString( ) );
			if ( ms_selection->select_uvrange->text( ) != "" )
				ddo.insert( "uvrange", ms_selection->select_uvrange->text( ).toStdString( ) );
			if ( ms_selection->select_antenna->text( ) != "" )
				ddo.insert( "antenna", ms_selection->select_antenna->text( ).toStdString( ) );
			if ( ms_selection->select_scan->text( ) != "" )
				ddo.insert( "scan", ms_selection->select_scan->text( ).toStdString( ) );
			if ( ms_selection->select_corr->text( ) != "" )
				ddo.insert( "corr", ms_selection->select_corr->text( ).toStdString( ) );
			if ( ms_selection->select_array->text( ) != "" )
				ddo.insert( "array", ms_selection->select_array->text( ).toStdString( ) );
			if ( ms_selection->select_msexpr->text( ) != "" )
				ddo.insert( "msexpr", ms_selection->select_msexpr->text( ).toStdString( ) );
		}

		// pass along regridding information to QtDisplayPanelGui...
		if ( load_tree_widget_ != 0 && load_tree_widget_->currentItem( ) != 0 &&
		        dataType_.value(uiDataType_[load_tree_widget_->currentItem()->text(1)]) == "image" ) {
			ddo.insert( "regrid", guimethod_to_iamethod(regrid_method->currentText( )) );
			if ( slice_gen->sliceReady( ) )
				ddo.insert( "slice", slice_gen->getSliceRep( ) );
			else
				ddo.insert( "slice", "none" );
		}

		panel_->createDD( path, datatype, displaytype, True, -1,
				false, false, false, ddo, image_properties );

		if(!leaveOpen_->isChecked()) close();  // (will hide dialog for now).
	}


	void QtDataManager::load_regions_clicked( ) {
		QPushButton* button = dynamic_cast<QPushButton*>(sender());

		if(panel_==0 || button==0) return;

		if ( tab_info.size( ) == 0 ) init_tab_info( );
		tab_state ts = tab_info[tabs->currentIndex( )];

		std::string path, datatype, displaytype;

		displaytype = (displayType_.key(uiDisplayType_[button->text()])).toStdString();

		if (load_tree_widget_->currentItem() > 0) {

			// Display selected file.
			path = (ts.dir( )->path( ) + "/" + load_tree_widget_->currentItem()->text(0)).toStdString( );
			datatype = dataType_.value(uiDataType_[load_tree_widget_->currentItem()->text(1)]).toStdString();
		}

		if(path=="" || datatype=="" || displaytype=="") return;

		panel_->loadRegions( path, datatype );

		if(!leaveOpen_->isChecked()) close();  // (will hide dialog for now).
	}

//<drs> Duplicate code for this functionality in QtDBusViewerAdaptor::restore(...)
//      should try to find a way to make this available from QtViewer
	void QtDataManager::restoreToOld_() {
		// Restore viewer state to existing panel.
		// Use the first empty panel, or if none, the first panel.
		std::list<QtDisplayPanelGui*> dps = panel_->viewer()->openDPs();
		for ( std::list<QtDisplayPanelGui*>::iterator iter = dps.begin( ); iter != dps.end( ); ++iter ) {
			QtDisplayPanelGui* dp = *iter;
			//if(dp->displayPanel()->registeredDDs().len()==0) {
			if ( dp->displayPanel()->isEmptyRegistered()) {
				restoreTo_(dp->displayPanel());		// restore to first empty panel, if any...
				return;
			}
		}

		if ( dps.size( ) > 0 ) {
			restoreTo_(dps.front( )->displayPanel( ));		// ...else, restore to first panel, if any...
			return;
		}

		restoreToNew_();		// ...else, restore to a new panel.
	}



//<drs> Duplicate code for this functionality in QtDBusViewerAdaptor::restore(...)
//      should try to find a way to make this available from QtViewer
	void QtDataManager::restoreToNew_() {
		// Create new display panel, restore viewer state to it.
		panel_->viewer()->createDPG();

		std::list<QtDisplayPanelGui*> dps = panel_->viewer()->openDPs();
		if ( dps.size( ) > 0 ) {
			// Newly-created dp should be the last one on the list...
			restoreTo_( dps.back( )->displayPanel());
		}
	}




//<drs> Duplicate code for this functionality in QtDBusViewerAdaptor::restore(...)
//      should try to find a way to make this available from QtViewer
	void QtDataManager::restoreTo_(QtDisplayPanel* dp) {
		if ( tab_info.size( ) == 0 ) init_tab_info( );
		tab_state ts = tab_info[tabs->currentIndex( )];

		// Restore viewer state to given panel.

		// Collect the name of the selected restore file.
		String filename = ( ts.dir( )->path( ) + "/" + load_tree_widget_->currentItem()->text(0) ).toStdString( );

		Bool ok = dp->restorePanelState(filename);

		// (will hide open dialog for now, if requested).
		if(ok && !leaveOpen_->isChecked()) close();

	}


	void QtDataManager::lelGotFocus_() {
		load_tree_widget_->clearSelection();
		load_info_box->hide( );
		showDisplayButtons(IMAGE);
	}

	void QtDataManager::showDDCreateError_(String errMsg) {
		// For now, just send to cerr.  (To do: put this on a status line).
		cerr<<endl<<errMsg<<endl;
	}

	QStringList QtDataManager::analyseFITSImage(QString path) {
		QString qdelim="<delim>";
		QString qualMark="<qualimg>";
		QString fitsMark="<fitsimg>";

		QStringList typedExtlist;

		// create a parser object and get the String information on
		// the extensions with data
		FITSImgParser fip = FITSImgParser(String(path.toStdString()));
		String extstring = fip.get_extlist_string("<delim>", "<qualimg>", "<fitsimg>");

		// convert the String to a QString;
		// split into a list of QStrings
		QString qextstring = QString(extstring.c_str());
		QStringList extlist = qextstring.split(qdelim, QString::SkipEmptyParts);

		// create a list with the type plus
		// the extension name
		for (int j = 0; j < extlist.size(); j++) {
			QString ext = extlist.at(j);
			if (ext.contains(qualMark)) {
				typedExtlist << ext.remove(qualMark);
				typedExtlist << "Quality Ext.";
			} else if (ext.contains(fitsMark)) {
				typedExtlist << ext.remove(fitsMark);
				typedExtlist << "FITS Ext.";
			} else {
				typedExtlist << ext;
			}
		}

		// return the QString list
		return typedExtlist;
	}

	Bool QtDataManager::isQualImg(const QString &/*extexpr*/) {
		return True;
	}

// finally, this string is examined by LatticeSlice1D<T>::stringToMethod,
// which only looks at the first letter...
	std::string QtDataManager::guimethod_to_iamethod( const QString &type ) {
		if ( type == "bicubic" ) return "C";
		if ( type == "bilinear" ) return "L";
		if ( type == "nearest" ) return "N";
		return "";
	}

	void QtDataManager::showlelButtonClicked( bool clicked ) {
		if ( clicked ) {
			lelGB_->show( );
			rc.put( "viewer." + panel_->rcid() + ".datamgr.show_lel", "true" );
		} else {
			lelGB_->hide( );
			rc.put( "viewer." + panel_->rcid() + ".datamgr.show_lel", "false" );
		}
	}
	void QtDataManager::showSliceButtonClicked( bool clicked ) {
		if ( clicked ) {
			slice_frame->show( );
			rc.put( "viewer." + panel_->rcid() + ".datamgr.show_slice", "true" );
		} else {
			slice_frame->hide( );
			rc.put( "viewer." + panel_->rcid() + ".datamgr.show_slice", "false" );
		}
	}
	void QtDataManager::leaveopenButtonClicked( bool clicked ) {
		if ( clicked ) {
			rc.put( "viewer." + panel_->rcid() + ".datamgr.leave_up", "true" );
		} else {
			rc.put( "viewer." + panel_->rcid() + ".datamgr.leave_up", "false" );
		}
	}


	void QtDataManager::update_regrid_options( ) {

		// start out with the regrid combo-box hidden...
		regrid->hide( );

		// hide regrid option unless an image file is selected...
		if ( load_tree_widget_ == 0 || load_tree_widget_->currentItem( ) == 0 ||
		        dataType_.value(uiDataType_[load_tree_widget_->currentItem()->text(1)]) != "image" )
			return;

		QtDisplayData *cdd = 0;
		//if ( panel_->nDDs( ) == 0 || (cdd = panel_->dd( )) == 0 ) return;
		if ( panel_->isEmptyDD() || (cdd = panel_->dd()) == 0 ) return;

		const viewer::ImageProperties &cproperties = cdd->imageProperties( );
		if ( cproperties.ok( ) == false || cproperties.hasSpectralAxis( ) == false ) return;

		// at this point the already-loaded image has a spectral axis...
		// show the combo-box, but disable it...
		regrid->show( );
		regrid_method->setCurrentIndex(0);
		regrid->setDisabled(true);

        std::vector<double> controlling_velocities(cproperties.velocities( ));
        if ( controlling_velocities.size( ) > 0 ) {
             std::vector<double> new_velocities(image_properties.velocities( ));
             if ( new_velocities.size( ) > 0 ) {
                  bool matched = true;
                  //unsigned int COUNT=0;
                  for ( std::vector<double>::iterator newiter=new_velocities.begin( ), controliter=controlling_velocities.begin( );
                        newiter != new_velocities.end( ) && controliter != controlling_velocities.end( );
                        ++newiter, ++controliter ) {
                       if ( *newiter != *controliter ) {
                            matched = false;
                            break;
                       }
                  }
                  if ( matched != true ) {
                       regrid->setDisabled(false);
                  }
             }
        }
        if ( regrid_available == false )
            regrid->setDisabled(true);
	}

	void QtDataManager::update_slice_options( int ddtp, const QString & name ) {
		if ( tab_info.size() == 0 ) init_tab_info();
		tab_state ts = tab_info[tabs->currentIndex()];
		if ( ddtp == IMAGE ) {
			std::string path = (ts.dir()->path()+"/"+name).toStdString();
			slice_gen->enable(path);
		} else {
			slice_gen->disable();
		}
        if ( slice_available == false )
            slice_gen->disable( );
	}



	void QtDataManager::fill_image_info( const std::string &path ) {

		if ( tab_info.size( ) == 0 ) init_tab_info( );
		tab_state ts = tab_info[tabs->currentIndex( )];

		for ( infofield_list_t::iterator it = ts.infoFields( )->begin( ); it != ts.infoFields( )->end( ); ++it ) {
			(*it).first->hide( );
		}

		image_properties = path;
		if ( image_properties.ok( ) == false ) return;

		infofield_list_t::iterator it = ts.infoFields( )->begin( );
		(*it).first->show( );
		(*it).first->setTitle("shape");
		std::ostringstream buf;
		buf << image_properties.shape( );
		std::string shape_str = buf.str( );
		(*it).second->setText(QString::fromStdString(std::for_each(shape_str.begin(),shape_str.end(),strip_chars("[]"))));
		buf.str("");
		(*it).second->setCursorPosition(0);
		++it;

		std::vector<std::string> beamvec = image_properties.medianRestoringBeamAsStr( );
		if ( beamvec.size( ) == 3 ) {
			(*it).first->show( );
			if ( image_properties.nBeams( ) > 1 )
				(*it).first->setTitle("median restoring beam");
			else
				(*it).first->setTitle("restoring beam");
			std::string beam = beamvec[0] + ", " + beamvec[1] + ", " + beamvec[2];
			(*it).second->setText(QString::fromStdString(beam));
			(*it).second->setCursorPosition(0);
			++it;
		} else {
			++it;	// align ra/dec & freq/velo (may need to be made smarter)
		}

		if ( image_properties.hasDirectionAxis( ) ) {
			std::vector<std::string> ra_range = image_properties.raRangeAsStr( );
			std::vector<std::string> dec_range = image_properties.decRangeAsStr( );
			if ( ra_range.size( ) == 2 && dec_range.size( ) == 2 ) {
				(*it).first->show( );
				(*it).first->setTitle(QString::fromStdString(image_properties.directionType( )) + " right ascension" );
				std::string ra_str = ra_range[0] + ", " + ra_range[1];
				(*it).second->setText(QString::fromStdString(ra_str));
				(*it).second->setCursorPosition(0);
				++it;
				(*it).first->show( );
				(*it).first->setTitle(QString::fromStdString(image_properties.directionType( )) + " declination" );
				std::string dec_str = dec_range[0] + ", " + dec_range[1];
				(*it).second->setText(QString::fromStdString(dec_str));
				(*it).second->setCursorPosition(0);
				++it;
			} else {
				(*it).first->show( );
				(*it).first->setTitle("direction type");
				(*it).second->setText(QString::fromStdString(image_properties.directionType( )));
				(*it).second->setCursorPosition(0);
				++it;
			}

		}

		if ( image_properties.hasSpectralAxis( ) ) {
			if ( image_properties.frequencies( ).size( ) > 0 ) {
				(*it).first->show( );
				(*it).first->setTitle("frequency range");
				buf.str("");
				buf << image_properties.frequencies( ).front( ) << ", " <<
                     image_properties.frequencies( ).back( ) << " " << image_properties.frequencyUnits( );
				(*it).second->setText(QString::fromStdString(buf.str( )));
				(*it).second->setCursorPosition(0);
				++it;
			}

			if ( image_properties.velocities( ).size( ) > 0 ) {
				(*it).first->show( );
				(*it).first->setTitle("velocity range");
				buf.str("");
				buf << image_properties.velocities( ).front( ) << ", " <<
                     image_properties.velocities( ).back( ) << " km/s" ;
				(*it).second->setText(QString::fromStdString(buf.str( )));
				(*it).second->setCursorPosition(0);
				++it;
			}
		}
	}

	void QtDataManager::changeTabContext( int index ) {
		buildDirTree();
		updateDisplayDatas(0,false);
	}

	void QtDataManager::init_tab_info( ) {
		if ( tab_info.size( ) != 0 ) return;
		for ( int i=0; i < tabs->count( ); ++i ) {
			if ( tabs->tabText(i) == "load" ) {
				tab_info.insert(tab_info_map_t::value_type(i,tab_state( load_directory, load_tree_widget_, set<int>( ),
				                load_info_box, &load_ifields, &QtDataManager::load_tab_notify )));
			} else if ( tabs->tabText(i) == "save image" ) {
				set<int> filter;
				filter.insert(CASAREGION);
				filter.insert(DS9REGION);
				filter.insert(MEASUREMENT_SET);
				filter.insert(SKY_CATALOG);
				filter.insert(RESTORE);
				tab_info.insert(tab_info_map_t::value_type(i,tab_state( image_directory, image_file_list_, filter,
				                0, 0, &QtDataManager::image_tab_notify,
				                image_data_list_, img_output_name,
				                &QtDataManager::image_tab_error )));
			} else if ( tabs->tabText(i) == "save region" ) {
				set<int> filter;
				filter.insert(IMAGE);
				filter.insert(MEASUREMENT_SET);
				filter.insert(SKY_CATALOG);
				filter.insert(IMAGE);
				filter.insert(QUALIMG);
				filter.insert(RESTORE);
				tab_info.insert(tab_info_map_t::value_type(i,tab_state( region_directory, region_file_list, filter,
				                0, 0, &QtDataManager::region_tab_notify,
				                region_data_list, region_output_name,
				                &QtDataManager::region_tab_error )));
			}
		}
	}

	void QtDataManager::img_output_target_changed( const QString &txt ) {

		if ( img_output_name->text( ) != last_image_extension_tweak_string ) {
			if ( txt.endsWith(".fits") && save_fits_img->isChecked( ) == false ) {
				last_image_extension_tweak_string = img_output_name->text( );
				save_fits_img->setChecked(true);
				return;
			} else if ( txt.endsWith(".image") && save_casa_img->isChecked( ) == false ) {
				last_image_extension_tweak_string = img_output_name->text( );
				save_casa_img->setChecked(true);
				return;
			}
		}

		validation_msg msg( output_validation( image_directory->text( ).trimmed( ), img_output_name->text( ).trimmed( ),
		                                       save_fits_img->isChecked( ) ? validation_msg::FITS : validation_msg::CASA ) );
		switch (msg.valid) {
		case validation_msg::VALID:
			img_output_error->setStyleSheet("color: green");
			img_do_save->setEnabled(true);
			break;
		case validation_msg::INVALID:
			img_output_error->setStyleSheet("color: red");
			img_do_save->setEnabled(false);
			break;
		case validation_msg::WARN:
			img_output_error->setStyleSheet("color: orange");
			img_do_save->setEnabled(true);
			break;
		}
		img_output_error->setText( msg.msg + msg.path );
	}


	void QtDataManager::img_do_output( ) {

		// insures that we have an acceptable output path and data to save...
		validation_msg msg( output_validation( image_directory->text( ).trimmed( ), img_output_name->text( ).trimmed( ),
		                                       save_fits_img->isChecked( ) ? validation_msg::FITS : validation_msg::CASA ) );

		if ( msg.valid == validation_msg::INVALID ) {
			img_output_error->setStyleSheet("color: red");
			img_output_error->setText( msg.msg + msg.path );
			img_do_save->setEnabled(false);
			return;
		}

		QTreeWidgetItem *item = image_data_list_->currentItem( );
		if ( item == 0 ) {
			img_output_error->setStyleSheet("color: red");
			img_output_error->setText( "no data to save" );
			img_do_save->setEnabled(false);
			return;
		}

		QtDisplayData *qdd = display_datas[item->text(0)];

		shared_ptr<ImageInterface<Float> > img = qdd->imageInterface();
		if (!img) {
			img_output_error->setStyleSheet("color: red");
			img_output_error->setText( "cannot export data, complex images cannot be exported" );
			img_do_save->setEnabled(false);
			return;
		}

		if ( msg.path.toStdString( ) == viewer::canonical_path(qdd->path( )) ) {
			img_output_error->setStyleSheet("color: red");
			img_output_error->setText( "error: input and output are the same" );
			img_do_save->setEnabled(false);
			return;
		}

		bool OK = false;
		switch ( msg.output_format ) {
		case validation_msg::CASA:
			OK = export_to_casa(img.get(),msg.path.toStdString( ));
			break;
		case validation_msg::FITS:
			OK = export_to_fits(img.get(),msg.path.toStdString( ));
			break;
		default:
			img_output_error->setStyleSheet("color: red");
			img_output_error->setText( "export failed" );
			img_do_save->setEnabled(false);
			OK = false;
			break;
		}

		if( OK && ! leaveOpen_->isChecked() ) close();  // (will hide dialog for now).
	}


	void QtDataManager::region_ds9_csys_disable( bool checked ) {
		if ( checked )
			save_ds9_csys->setDisabled(false);
		else
			save_ds9_csys->setDisabled(true);
	}


	void QtDataManager::region_output_target_changed( const QString &txt ) {

		if ( QObject::sender( ) == save_casa_region ) {
			if ( save_casa_region->isChecked( ) && save_ds9_region->isChecked( ) )
				save_ds9_region->setChecked(false);
		} else if ( QObject::sender( ) == save_ds9_region ) {
			if ( save_casa_region->isChecked( ) && save_ds9_region->isChecked( ) )
				save_casa_region->setChecked(false);
		}


		if ( region_output_name->text( ) != last_region_extension_tweak_string ) {
			if ( txt.endsWith(".crtf") && save_casa_region->isChecked( ) == false ) {
				last_region_extension_tweak_string = region_output_name->text( );
				save_casa_region->setChecked(true);
				return;
			}
		}

		validation_msg msg( output_validation( region_directory->text( ).trimmed( ), region_output_name->text( ).trimmed( ),
		                                       save_ds9_region->isChecked( ) ? validation_msg::DS9 : validation_msg::CRTF ) );
		switch (msg.valid) {
		case validation_msg::VALID:
			region_output_error->setStyleSheet("color: green");
			region_do_save->setEnabled(true);
			break;
		case validation_msg::INVALID:
			region_output_error->setStyleSheet("color: red");
			region_do_save->setEnabled(false);
			break;
		case validation_msg::WARN:
			region_output_error->setStyleSheet("color: orange");
			region_do_save->setEnabled(true);
			break;
		}
		region_output_error->setText( msg.msg + msg.path );
	}

	void QtDataManager::region_do_output( ) {

		// insures that we have an acceptable output path and data to save...
		validation_msg msg( output_validation( region_directory->text( ).trimmed( ), region_output_name->text( ).trimmed( ),
		                                       save_ds9_region->isChecked( ) ? validation_msg::DS9 : validation_msg::CRTF ) );

		if ( msg.valid == validation_msg::INVALID ) {
			region_output_error->setStyleSheet("color: red");
			region_output_error->setText( msg.msg + msg.path );
			region_do_save->setEnabled(false);
			return;
		}

		std::list<viewer::QtRegionState*> region_list;
		for ( int i=0; i < region_data_list->topLevelItemCount( );  ++i) {
			QTreeWidgetItem *item = region_data_list->topLevelItem(i);
			if ( item->checkState(0) == Qt::Checked ) {
				region_list.push_back(treeitem_to_region[item]->state( ));
			}
		}

		if ( region_list.size( ) == 0 ) {
			region_output_error->setStyleSheet("color: red");
			region_output_error->setText( "no regions for output" );
			region_do_save->setEnabled(false);
			return;
		}

		if ( msg.output_format != validation_msg::CRTF &&
		        msg.output_format != validation_msg::DS9 ) {
			region_output_error->setStyleSheet("color: red");
			region_output_error->setText( "bad region output format" );
			region_do_save->setEnabled(false);
			return;
		}

		std::string err = panel_->outputRegions( region_list, msg.path.toStdString( ),
		                  msg.output_format == validation_msg::DS9 ? "ds9" : "crtf",
		                  save_ds9_csys->currentText( ).toStdString( ) );
		if ( err.size( ) == 0 ) {
			region_output_error->setStyleSheet("color: blue");
			region_output_error->setText("success...");
			region_do_save->setEnabled(false);

			if(!leaveOpen_->isChecked()) close();  // (will hide dialog for now).

		} else {
			region_output_error->setStyleSheet("color: red");
			region_output_error->setText( QString::fromStdString(err) );
			region_do_save->setEnabled(false);
		}

	}

	namespace viewer {
		template <typename T>
		struct join {
			join( ) : dosep(false) { }
			join( const T &s ) : dosep(true), sep(s) { }
			join( const join<T> &o ) : dosep(o.dosep), sep(o.sep), accum(o.accum) { }
			~join( ) { }
			void operator( )( const T &c ) {
				if ( accum.size( ) > 0 ) {
					if ( dosep )
						accum = accum + sep + c;
					else
						accum = accum + c;
				} else {
					accum = c;
				}
			}

			operator T( ) {
				return accum;
			}

		private:
			bool dosep;
			T sep;
			T accum;
		};


// this should be expanded with another functor to resolve internal
// symlinks in a separated path... but for now (in the QtDataManager)
// I'll just use the Qt canonical path function...
		struct split_path {
			std::string prefix( ) const {
				return root == IS_ROOT ? std::string(1,sep) : std::string( );
			}
			std::string separator( ) const {
				return std::string(1,sep);
			}
			split_path( bool strip_dotdot=true ) : strip(strip_dotdot), root(UNINITIALIZED), size(128), off(0),
				buf((char*) malloc(size*sizeof(char))) {
				buf[off] = '\0';
			}
			split_path( const split_path &o ) : strip(o.strip), root(o.root), size(o.size),
				off(o.off), buf((char*) malloc(size*sizeof(char))),
				separated(o.separated) {
				memcpy(buf,o.buf,off);
			}
			~split_path( ) {
				free(buf);
			}
			void operator( )( char c ) throw (std::invalid_argument) {
				if ( root == UNINITIALIZED )
					root = (c == sep ? IS_ROOT : NOT_ROOT);
				if ( c == sep ) {
					if ( off > 0 ) {
						buf[off] = '\0';
						if ( strip ) {
							if ( *buf == '.' ) {
								if ( *(buf+1) == '\0' ) {
									off = 0;
									return;
								} else if ( *(buf+1) == '.' && *(buf+2) == '\0' ) {
									if ( separated.size( ) > 0 && separated.back() != ".." ) {
										off = 0;
										separated.pop_back( );
										return;
									} else if ( root == IS_ROOT ) {
										throw std::invalid_argument("stripping '..' goes above root directory");
									}
								}
							}
						}
						separated.push_back(buf);
						off = 0;
					}
				} else {		/******* may strip out embeded '\0' here *******/
					if ( off >= size - 1 ) {
						size *= 2;
						buf = (char*) realloc( buf, size*sizeof(char) );
					}
					buf[off++] = c;
				}
			}
			operator std::list<std::string>( ) throw (std::invalid_argument) {
				if ( off != 0 ) {
					buf[off] = '\0';
					if ( strip ) {
						if ( *buf == '.' ) {
							if ( *(buf+1) == '\0' ) {
								off = 0;
							} else if ( *(buf+1) == '.' && *(buf+2) == '\0' ) {
								if ( separated.size( ) > 0 ) {
									off = 0;
									separated.pop_back( );
								} else if ( root == IS_ROOT ) {
									throw std::invalid_argument("stripping '..' goes above root directory");
								}
							}
						}
					}
					if ( off != 0 ) {
						off = 0;
						separated.push_back(buf);
					}
				}
				return separated;
			}

			operator std::string( ) throw (std::invalid_argument) {
				if ( off != 0 ) {
					buf[off] = '\0';
					if ( strip ) {
						if ( *buf == '.' ) {
							if ( *(buf+1) == '\0' ) {
								off = 0;
							} else if ( *(buf+1) == '.' && *(buf+2) == '\0' ) {
								if ( separated.size( ) > 0 ) {
									off = 0;
									separated.pop_back( );
								} else if ( root == IS_ROOT ) {
									throw std::invalid_argument("stripping '..' goes above root directory");
								}
							}
						}
					}
					if ( off != 0 ) {
						off = 0;
						separated.push_back(buf);
					}
				}
				if ( root == IS_ROOT )
					return std::string(1,sep) + (std::string) (std::for_each(separated.begin(), separated.end( ), join<std::string>(std::string(1,sep))));
				else
					return std::for_each(separated.begin(), separated.end( ), join<std::string>(std::string(1,sep)));
			}

		private:
			static const char sep = '/';
			bool strip;
			enum { UNINITIALIZED, IS_ROOT, NOT_ROOT } root;		// path starts at root?
			size_t size;
			size_t off;
			char *buf;
			std::list<std::string> separated;
		};


		std::string readlink( const std::string &path ) {
			int buffer_size = 128;
			char *buffer = new char[buffer_size+1];
			int nchars = ::readlink( path.c_str( ), buffer, buffer_size );
			while ( nchars == buffer_size ) {
				buffer_size *= 2;
				delete [] buffer;
				buffer = new char[buffer_size+1];
				nchars = ::readlink( path.c_str( ), buffer, buffer_size );
			}
			std::string result;
			if ( nchars > 0 ) {
				buffer[nchars] = '\0';
				result = buffer;
			}
			delete [] buffer;
			return result;
		}

		std::string canonical_path( const std::string &path ) {
			split_path split = std::for_each(path.begin(),path.end(),split_path( ));
			if ( split.prefix( ) != split.separator( ) ) {
				char wd[PATH_MAX+1];
				if ( getcwd(wd,PATH_MAX+1) != NULL ) {
					// split already has '.' and '..' stripped out...
					// if new path == old path, probably things have run off the rails...
					std::string recurse = wd + split.separator( ) + (std::string) split;
					if ( recurse != path )
						return canonical_path( recurse );
				}
			}
			std::list<std::string> sep = split;
			std::string prefix = split.prefix( );
			std::string accum;
			struct stat statbuf;
			for ( std::list<std::string>::iterator it = sep.begin( ); it != sep.end( ); ++it ) {
				std::string cur = accum + split.separator( ) + *it;
				if ( lstat( cur.c_str( ), &statbuf ) == 0 && S_ISLNK(statbuf.st_mode) ) {
					std::string lnk = readlink( cur );
					if ( lnk[0] == split.separator( )[0] ) {
						return canonical_path(lnk + split.separator( ) + (std::string) std::for_each( ++it, sep.end( ), join<std::string>(split.separator( ))));
					} else {
						return canonical_path(accum + split.separator( ) + lnk + split.separator( ) + (std::string) std::for_each( ++it, sep.end( ), join<std::string>(split.separator( ))));
					}
				}
				accum = cur;
			}
			return accum;
		}
	}


	QString canonical_path( const QString &qs ) {
		return QString::fromStdString(viewer::canonical_path(qs.toStdString( )));
	}

	QtDataManager::validation_msg QtDataManager::output_validation( const QString &dir, const QString &path, validation_msg::output_format_t format) const {

		if ( format == validation_msg::CASA || format == validation_msg::FITS ) {
			if ( image_data_list_->currentItem( ) == 0 )
				return validation_msg("",format,validation_msg::INVALID,"no data to save");
		} else if ( format == validation_msg::CRTF || format == validation_msg::DS9 ) {
			int regions_selected = 0;
			for ( int i=0; i < region_data_list->topLevelItemCount( );  ++i) {
				if ( region_data_list->topLevelItem(i)->checkState(0) == Qt::Checked ) {
					regions_selected += 1;
				}
			}
			if ( regions_selected == 0 )
				return validation_msg("",format,validation_msg::INVALID,"no data to save");
		}

		if ( path.isEmpty( ) )
			return validation_msg("",format,validation_msg::INVALID,"no file specified");

		QString combined_path = (path.startsWith("/") ? path : (dir + "/" + path));
		QFileInfo f(combined_path);

		if ( f.exists( ) ) {
			if ( ! f.isWritable( ) ) {
				return validation_msg(canonical_path(combined_path),format,validation_msg::INVALID,"not writable: ");
			} else if ( format == validation_msg::CASA && ! f.isDir( ) ) {
				return validation_msg(canonical_path(combined_path),format,validation_msg::INVALID,"exists & not a dir: ");
			} else if ( ( format == validation_msg::CRTF || format == validation_msg::DS9 ||
			              format == validation_msg::FITS ) && ! f.isFile( ) ) {
				return validation_msg(canonical_path(combined_path),format,validation_msg::INVALID,"exists & not a file: ");
			} else {
				return validation_msg(canonical_path(combined_path),format,validation_msg::WARN,"will overwrite: ");
			}
		} else {
			QFileInfo dir(f.dir( ).path( ));
			if ( ! dir.isWritable( ) ) {
				return validation_msg(canonical_path(combined_path),format,validation_msg::INVALID,"not writable: ");
			} else {
				return validation_msg(canonical_path(combined_path),format,validation_msg::VALID,"writing to: ");
			}
		}
		return validation_msg(canonical_path(combined_path),format,validation_msg::INVALID,"internal error: ");
	}


	bool QtDataManager::export_to_fits(ImageInterface<Float>* img, std::string outFile) {

		// thats the default values for the call "ImageFITSConverter::ImageToFITS"
		String error;
		uInt memoryInMB(64);
		Bool preferVelocity(True);
		Bool opticalVelocity(True);
		Int BITPIX(-32);
		Float minPix(1.0);
		Float maxPix(-1.0);
		Bool allowOverwrite(False);
		Bool degenerateLast(False);
		Bool verbose(True);
		Bool stokesLast(False);
		Bool preferWavelength(False);
		Bool preferAirWavelength(False);
		String origin("CASA Viewer / FITS export");

		// overwrite was confirmed
		allowOverwrite = True;
		getSpectralCoordFlags(img, preferVelocity, opticalVelocity, preferWavelength, preferAirWavelength);

		// overwrite the default "origin" if already
		// in the metadata
		const TableRecord miscInfo=img->miscInfo();
		if (miscInfo.isDefined("origin") && miscInfo.dataType("origin")==TpString) {
			origin = miscInfo.asString(String("origin"));
		}

		// export the image to FITS
		ImageFITSConverter::ImageToFITS(
		    error,
		    *img,
		    outFile,
		    memoryInMB,
		    preferVelocity,
		    opticalVelocity,
		    BITPIX,
		    minPix,
		    maxPix,
		    allowOverwrite,
		    degenerateLast,
		    verbose,
		    stokesLast,
		    preferWavelength,
		    preferAirWavelength,
		    origin
		);

		//
		if (error.size()>0) {
			img_output_error->setStyleSheet("color: red");
			img_output_error->setText( "error: \""+ QString(error.c_str()) );
			img_do_save->setEnabled(false);
			return false;
		} else {
			img_output_error->setStyleSheet("color: blue");
			img_output_error->setText("success...");
			img_do_save->setEnabled(false);
			return true;
		}
	}

	void QtDataManager::getSpectralCoordFlags(const ImageInterface<Float>* img, Bool &preferVelocity, Bool &opticalVelocity, Bool &preferWavelength, Bool &preferAirWavelength) {
		// check for a spectral axis
		if (!(img->coordinates().hasSpectralAxis()))
			return;

		SpectralCoordinate spcCoord = img->coordinates().spectralCoordinate();
		SpectralCoordinate::SpecType spcType=spcCoord.nativeType();

		switch (spcType) {
		case SpectralCoordinate::FREQ:
			preferVelocity      = False;
			opticalVelocity     = False;
			preferWavelength    = False;
			preferAirWavelength = False;
			break;
		case SpectralCoordinate::VRAD:
			preferVelocity      = True;
			opticalVelocity     = False;
			preferWavelength    = False;
			preferAirWavelength = False;
			break;
		case SpectralCoordinate::VOPT:
			preferVelocity      = True;
			opticalVelocity     = True;
			preferWavelength    = False;
			preferAirWavelength = False;
			break;
		case SpectralCoordinate::BETA:
			preferVelocity      = False;
			opticalVelocity     = False;
			preferWavelength    = False;
			preferAirWavelength = False;
			break;
		case SpectralCoordinate::WAVE:
			preferVelocity      = False;
			opticalVelocity     = False;
			preferWavelength    = True;
			preferAirWavelength = False;
			break;
		case SpectralCoordinate::AWAV:
			preferVelocity      = False;
			opticalVelocity     = False;
			preferWavelength    = True;
			preferAirWavelength = True;
			break;
		default:
			preferVelocity      = False;
			opticalVelocity     = False;
			preferWavelength    = False;
			preferAirWavelength = False;
		}

	}

	bool QtDataManager::export_to_casa(ImageInterface<Float> *img, String outFile) {
		try {
			// from: synthesis/TransformMachines/Utils.h
			// still to be copied over:
			// regions, history??
			PagedImage<Float> newPagedImage(img->shape(), img->coordinates(), outFile);
			LatticeExpr<Float> le(*img);
			newPagedImage.copyData(le);

			// check for a mask
			if (img->isMasked()) {
				// get the default mask name
				const String maskName = img->getDefaultMask();

				// create a mask in the output image
				if (maskName.size()>0)
					newPagedImage.makeMask(maskName, True, True);
				else
					newPagedImage.makeMask("default", True, True);

				// copy the mask over
				(newPagedImage.pixelMask()).copyData(img->pixelMask());
				//img->pixelMask().copyDataTo(newPagedImage.pixelMask());
			}

			// copy ImageInfo and MiscInfo
			newPagedImage.setImageInfo(img->imageInfo());
			newPagedImage.setMiscInfo(img->miscInfo());

			//Added for CAS-5216.  When the units are from a collapsed image and
			//are nonstandard, such as 'Jy/beam.km/sec' they were not getting saved
			//through normal means.
			Unit imageUnits = img->units();
			newPagedImage.setUnits( imageUnits );

			img_output_error->setStyleSheet("color: blue");
			img_output_error->setText("success...");
			img_do_save->setEnabled(false);
			return true;

		} catch (AipsError x) {
			img_output_error->setStyleSheet("color: red");
			img_output_error->setText( "error: \""+ QString((x.getMesg()).c_str()) );
			img_do_save->setEnabled(false);
			return false;
		}
	}

    void QtDataManager::enable_disable_slice( const QString &text ) {
        if ( text != "none" ) {
            slice_available = false;
            slice_gen->disable( );
        } else {
            bool old_state = slice_available;
            slice_available = true;
            if ( old_state == false ) {
                 tab_state ts = tab_info[tabs->currentIndex( )];
                 QList<QTreeWidgetItem *> lst = ts.tree( )->selectedItems();
                 if (!lst.empty()) {
                      QTreeWidgetItem *item = (QTreeWidgetItem*)(lst.at(0));
                      update_slice_options(uiDataType_[item->text(1)], item->text(0));
                 }
            }
        }
    }
    void QtDataManager::enable_disable_regrid( bool slicing ) {
        if ( slicing ) {
            regrid_available = false;
            regrid->setDisabled(true);
        } else {
            bool old_state = regrid_available;
            regrid_available = true;
            if ( old_state == false )
                update_regrid_options( );
        }
    }

	struct vo_service_config {
		// name of service => pre-flight configuration
		function<void(QtDataManager &mgr)> pre_flight;
		function<const char **( )> flex_params;
		vo_service_config( function<void(QtDataManager &mgr)> pref, function<const char **( )> flex ) : pre_flight(pref), flex_params(flex) { }

	};

	void lambda_dsoc_test_pre_( QtDataManager &dm ) {
		dm.ra_size_val->setRange( 0, 360 );
        dm.vo_ra_size->setText("360.0");
        dm.vo_dec_size->setText("360.0");
        dm.vo_dec->setText("0.0");
        dm.vo_ra->setText("180.0");

	}
	const char **lambda_dsoc_test_flex_( ) {
		static const char *result[] = { "Band", "BAND", "1.0E-8/5",
										"Time", "TIME", "1990-07-04/2014",
										"Mode", "MODE", "archival",
										"Collection", "COLLECTION", "alma,jvla,vla", 0 };
		return result;
	}

    void QtDataManager::setupVO( ) {

		// initialize the default service name...
		vo_selected_service = vo_service->currentText( );

		const char *vo_service_service_map[] = { "NRAO/DSOC Test Service", "http://vaosa-vm1.aoc.nrao.edu/ivoa-dal/siapv2-vao",
												 "Chandra X-ray Observatory Data Archive", "http://cda.harvard.edu/cxcsiap/queryImages?",
												 "Chandra Source Catalog", "http://cda.harvard.edu/cscsiap/queryImages?",
												 0, 0 };

        const char *vo_labels_to_params_defaults[] = { "Band", "BAND", "1.0E-8/5",
                                                       "Time", "TIME", "1990-07-04/2014",
                                                       "Mode", "MODE", "archival",
                                                       "Collection", "COLLECTION", "alma,jvla,vla",
													   0, 0, 0 };

		for ( const char **name=vo_service_service_map, **url=vo_service_service_map+1; *name; name+=2, url+=2 ) {
			vo_service_name_to_url[*name] = *url;
		}

        // save the default background for QLineEdit...
        QPalette pal = vo_ra->palette( );
        vo_default_bg = pal.color( vo_ra->backgroundRole() );

        // set up QLineEdit validators...
		//  >>>>>>=========================> move definitions back to here from end of class once we have lambda functions...............
        /*QDoubleValidator * */ra_val = new QDoubleValidator(this);
        /*QDoubleValidator * */dec_val = ra_val;
        /*QDoubleValidator * */ra_size_val = new QDoubleValidator(this);;
        /*QDoubleValidator * */dec_size_val = ra_size_val;

        vo_ra->setValidator(ra_val);
        vo_dec->setValidator(dec_val);
        vo_ra_size->setValidator(ra_size_val);
        vo_dec_size->setValidator(dec_size_val);

		lambda_dsoc_test_pre_(*this);

        for ( const char **valp=vo_labels_to_params_defaults; *valp; valp += 3 ) {
            voparameters.push_back(vo_param_t(false, valp[0], valp[1], valp[2], NULL));
            // user actions fills vo_flex_params_entry
            vo_flex_params->addItem(valp[0]);
        }

		connect( vo_clear, SIGNAL(pressed( )), SLOT(vo_clear_table( )) );
        connect( vo_query, SIGNAL(pressed( )), SLOT(vo_launch_query( )) );
		connect( vo_raster, SIGNAL(pressed( )), SLOT(vo_fetch_data( )) );
		connect( vo_contour, SIGNAL(pressed( )), SLOT(vo_fetch_data( )) );

        connect( voaddb, SIGNAL(pressed( )), this, SLOT(addVOParam( )) );
        connect( voremoveb, SIGNAL(pressed( )), this, SLOT(removeVOParam( )) );

        connect( vo_ra, SIGNAL(textChanged(QString)), this, SLOT(vo_clear_param( )) );
        connect( vo_ra_size, SIGNAL(textChanged(QString)), this, SLOT(vo_clear_param( )) );
        connect( vo_dec, SIGNAL(textChanged(QString)), this, SLOT(vo_clear_param( )) );
        connect( vo_dec_size, SIGNAL(textChanged(QString)), this, SLOT(vo_clear_param( )) );

		connect( vo_service, SIGNAL(currentIndexChanged(QString)), this, SLOT(vo_service_select(QString)) );

        connect( &dvo, SIGNAL(query_begin(int,QString,QtStringMap)), this, SLOT(vo_query_begin(int,QString,QtStringMap)) );
        connect( &dvo, SIGNAL(query_data(int,QString,QtStringMap)), this, SLOT(vo_query_row(int,QString,QtStringMap)) );
        connect( &dvo, SIGNAL(query_description(int,QString,QtStringMap)), this, SLOT(vo_query_description(int,QString,QtStringMap)) );
        connect( &dvo, SIGNAL(query_end(int,QString,QtStringMap)), this, SLOT(vo_query_complete(int,QString,QtStringMap)) );
		connect( &dvo, SIGNAL(query_error(int,QString,QString)), this, SLOT(vo_error(int,QString,QString)) );
		connect( &dvo, SIGNAL(fetch_complete(int,QString)), this, SLOT(vo_fetch_complete(int,QString)) );
		connect( &dvo, SIGNAL(fetch_progress(int,QString,double,double,double,double)),
				 this, SLOT(vo_fetch_progress(int,QString,double,double,double,double)) );
		connect( &dvo, SIGNAL(fetch_error(int,QString,QString)), this, SLOT(vo_error(int,QString,QString)) );

        vo_flex_params->setCurrentRow(0);
        vo_table->setSelectionBehavior(QAbstractItemView::SelectRows);
		connect( vo_table, SIGNAL(itemSelectionChanged( )), SLOT(vo_selection_changed( )) );

		vo_init_columns( );

    }

     bool QtDataManager::updateVOstatus( ) {
		static bool dvo_service_down = false;
        if ( dvo.isValid( ) ) {
			vo_enable_actions( );
			if ( dvo_service_down ) vo_clear_status( );
			dvo_service_down = false;
			return true;
        } else {
			vo_disable_actions( );
			error( "DBus Virtual Observatory Service is NOT available..." );
			dvo_service_down = true;
			return false;
        }
    }

    void QtDataManager::vo_clear_param( ) {
        QWidget *sender = static_cast<QWidget*>(sender);
        if ( sender && dvo_missing_parameters.find(sender) != dvo_missing_parameters.end( ) ) {
            QPalette pal = sender->palette( );
            pal.setColor( sender->backgroundRole(), vo_default_bg );
            sender->setPalette(pal);
        }
    }

    void QtDataManager::vo_query_begin( int count, const QString &service, const QtStringMap &values ) {
		vo_action_with_timeout_reset( );
        if ( values["QUERY_STATUS"] == "OK" ) {
			QString url_base = values["URL base"];
			QString url_full = values["URL full"];
		} else {
			qDebug() << "failed query to " << values["URL base"];
		}
    }

    static QStringList add_to_qstringlist( const QStringList &l, const QString &v ) {
        QStringList result(l);
        result << v;
        return result;
    }

    static std::string to_string( const std::pair<QString,int>& data) {
        std::ostringstream str;
        str << data.first.toStdString( ) << ", " << data.second;
        return str.str();
    }

    void QtDataManager::vo_query_description( int count, const QString &service, const QtStringMap &values ) {
		qDebug( ) << "------------------------------------------- description -------------------------------------------";
		qDebug( ) << service;
		qDebug( ) << "---------------------------------------------------------------------------------------------------";
		qDebug( ) << values;
		vo_action_with_timeout_reset( );
        int initial_columns = vo_labels.size( );
        vector<QString>::size_type cur_row = vo_urls.size( );
        bool found_url = false;
        for ( QtStringMap::const_iterator it=values.begin( ); it != values.end( ); ++it ) {
            if ( it.key( ) == "access_url" || it.key( ) == "accref" )
                 found_url = true;
            else if ( vo_label2col.find(it.key( )) == vo_label2col.end( ) ) {
                 vo_labels.push_back(it.key( ));
                 // column #0 has the delete row button...
                 // so column numbering for info starts at column #1...
                 vo_label2col[it.key( )] = vo_labels.size( );
				 QStringList desc = it.value( ).split(":@:");
				 vo_labels_tip.push_back(desc.last( ));
            }
        }
        if ( found_url && vo_labels.size( ) != initial_columns ) {
			int size = std::min(vo_labels.size( ),vo_labels_tip.size( ));
            vo_table->setColumnCount(size+1);
			for ( int i = 0; i < size; ++i ) {
				QTableWidgetItem *item = new QTableWidgetItem( vo_labels[i] );
				item->setToolTip(vo_labels_tip[i]);
				vo_table->setHorizontalHeaderItem(i+1,item);
			}
        }
    }

    void QtDataManager::vo_query_complete( int id, const QString &service, const QtStringMap &values ) {
		// std::copy( dvo_working_set.begin(), dvo_working_set.end(), std::ostream_iterator<int>( std::cout, "\n" ));
		set<dvo_working_item>::iterator iter = dvo_working_set.find(id);
		if ( iter != dvo_working_set.end( ) ) dvo_working_set.erase(iter);
		vo_action_with_timeout_complete( );
    }

    void QtDataManager::vo_error( int id, const QString &service, const QString &err ) {
		error( err );
		set<dvo_working_item>::iterator iter = dvo_working_set.find(id);
		if ( iter != dvo_working_set.end( ) ) dvo_working_set.erase(iter);
		vo_action_with_timeout_complete( );
    }

    void QtDataManager::vo_query_row( int id, const QString &service, const QtStringMap &values ) {
		vo_action_with_timeout_reset( );
		set<dvo_working_item>::iterator iter = dvo_working_set.find(id);
		if ( dvo_working_set.find(id) == dvo_working_set.end( ) ) return;
		if ( iter->count < 3 ) {
			qDebug( ) << "-------------------------------------------      row    -------------------------------------------";
			qDebug( ) << service;
			qDebug( ) << "---------------------------------------------------------------------------------------------------";
			qDebug( ) << values;
		}
		const int truncate_length = 500;
		if ( iter->count >= truncate_length ) {
			warning( QString("Truncating query results after %1 entries...").arg(truncate_length) );
			dvo.cancel(id);
			dvo_working_set.erase(iter);
			vo_action_with_timeout_complete( );
			return;
		}
		iter->increment( );

        int row = vo_table->rowCount();
        vo_table->insertRow(row);
        QString url;
		QString type;
		bool skip_row = false;
		QTableWidgetItem *reference_item = 0;
        for ( QtStringMap::const_iterator it=values.begin( ); it != values.end( ); ++it ) {
            if ( it.key( ) == "access_url" || it.key( ) == "accref" ) {
                 url = it.value( );
            } else {
				if ( it.key( ) == "access_format" || it.key( ) == "imgfmt" ) {
					type = it.value( );
					if ( type != "image/fits" ) {
						// drop any non-fits rows...
						skip_row = true;
						break;
					}
				}
                std::map<QString,int>::iterator col = vo_label2col.find(it.key( ));
                if ( col != vo_label2col.end( ) ) {
                     QTableWidgetItem *cell = new QTableWidgetItem(it.value( ));
					 if ( reference_item == 0 ) reference_item = cell;
					 cell->setFlags( cell->flags() ^ Qt::ItemIsEditable );
                     vo_table->setItem(row,col->second,cell);
                }
            }
        }

		if ( reference_item && ! url.isEmpty( ) && ! skip_row ) {
			vo_table->setCellWidget(row,0,new_vo_dismiss_button(reference_item,url,type));
		} else vo_table->removeRow( row );

    }

	void QtDataManager::vo_fetch_complete( int id, QString path ) {
		set<dvo_working_item>::iterator iter = dvo_working_set.find(id);
		if ( iter == dvo_working_set.end( ) ) return;
		dvo_working_set.erase(iter);
		if ( vo_current_action ) {
			dismiss_button_t *disb = static_cast<dismiss_button_t*>(vo_current_action);
			if ( disb && disb->path.isEmpty( ) == false ) {
				panel_->createDD( disb->path.toStdString( ), "image",
								  disb->display_type.toStdString( ), True, -1,
								  false, false, false/*, ddo, image_properties*/ );
			}
			vo_clear_status_delayed(3);
		} else vo_clear_status( );
		vo_action_with_timeout_complete( );
	}

	void QtDataManager::vo_fetch_progress( int id, QString path, double total_size, double total_done, double ultotal, double uldone ) {
		// if ( dvo_working_set.find(id) == dvo_working_set.end( ) ) return;
		status( QString("%1% (of %2MB) complete").arg(total_done/total_size * 100.0,0,'f',0).arg(total_size/(1024.0*1024.0),0,'f',1) );
		vo_action_with_timeout_reset( );
	}

    void QtDataManager::vo_flag_missing_param( QLineEdit *widget ) {
         QPalette pal = widget->palette();
         pal.setColor(widget->backgroundRole(), Qt::yellow);
         widget->setPalette(pal);
         dvo_missing_parameters.insert(widget);
    }

    bool QtDataManager::collect_vo_parameters( double &ra, double &dec, double &ra_size, double &dec_size, QVariantMap &params ) {
        bool OK = true;

        // ra
        QString ra_str = vo_ra->text( );
        if ( ra_str.size( ) <= 0 ) {
            OK = false;
            vo_flag_missing_param(vo_ra);
        } else ra = ra_str.toDouble( );

        // dec
        QString dec_str = vo_dec->text( );
        if ( dec_str.size( ) <= 0 ) {
            OK = false;
            vo_flag_missing_param(vo_dec);
        } else dec = dec_str.toDouble( );

        // ra size
        QString ra_size_str = vo_ra_size->text( );
        if ( ra_size_str.size( ) <= 0 ) {
            OK = false;
            vo_flag_missing_param(vo_ra_size);
        } else ra_size = ra_size_str.toDouble( );

        // dec size
        QString dec_size_str = vo_dec_size->text( );
        if ( dec_size_str.size( ) <= 0 ) {
            OK = false;
            vo_flag_missing_param(vo_dec_size);
        } else dec_size = dec_size_str.toDouble( );

        // flexible parameters...
        for ( int i=0; i < voparameters.size( ); ++i ) {
            if ( get<0>(voparameters[i]) ) {
                viewer::dvo::param *param = get<4>(voparameters[i]);
                if ( param && param->edit->text( ).size( ) <= 0 ) {
                    OK = false;
                    vo_flag_missing_param(param->edit);
                } else params.insert(get<2>(voparameters[i]),param->edit->text( ));
            }
        }
        return OK;
    }

    void QtDataManager::vo_launch_query( ) {
        if ( ! updateVOstatus( ) ) return;
        double ra, dec, ra_size, dec_size;
        QVariantMap flex_params;
        collect_vo_parameters( ra, dec, ra_size, dec_size, flex_params );
		QStringList vos;
		vos.push_back(vo_service_name_to_url[vo_selected_service]);
		int id = dvo.query(ra,dec,ra_size,dec_size, "image/fits", flex_params,vos);
        dvo_working_set.insert(id);
		vo_action_with_timeout( id, 5, "VO query" );
    }

	void QtDataManager::vo_selection_changed( ) {
		vo_selected_rows.clear( );
		QList<QTableWidgetSelectionRange> range = vo_table->selectedRanges( );
		for ( QList<QTableWidgetSelectionRange>::iterator it = range.begin( ); it != range.end( ); ++it ) {
			for ( int r=it->topRow( ); r <= it->bottomRow( ); ++r )vo_selected_rows.push_back(r);
		}
		if ( vo_selected_rows.size( ) > 0 ) {
			vo_contour->setEnabled( true );
			vo_raster->setEnabled( true );
		} else {
			vo_contour->setEnabled( false );
			vo_raster->setEnabled( false );
		}
	}

	void QtDataManager::vo_fetch_data( ) {
		if ( vo_table->currentRow( ) < 0 )
			error( "No row is currently selected..." );
		else {
			QWidget *widget = vo_table->cellWidget(vo_table->currentRow( ),0);
			if ( widget ) {
				dismiss_button_t *disb = static_cast<dismiss_button_t*>(widget);
				if ( disb && disb->url.size( ) > 0 ) {
					QPushButton *button = dynamic_cast<QPushButton*>(sender());
					if ( button ) {
						QString qname(disb->file_type);
						qname.replace('/','_');
						qname.push_front("dvo_");
						disb->path = QString::fromStdString(viewer::options.temporaryPath( qname.toStdString( ) ));
						disb->display_type = button->text( );
						vo_current_action = disb;
						qDebug() << "fetch url: " << disb->url;
						qDebug() << "fetch out: " << disb->path;
						int id = dvo.fetch(disb->url,disb->path,false);
						dvo_working_set.insert( id );
						vo_action_with_timeout( id, 5, "VO fetch" );
						// disable buttons, set timer (for timeout... reset with each progress event), etc.
					}
				}
			}
		}
	}

	void QtDataManager::vo_dismiss_row( ) {
		QPushButton *sender = static_cast<QPushButton*>(QObject::sender( ));
		if ( sender ) {
			dismiss_button_t *disb = static_cast<dismiss_button_t*>(sender);
			if ( disb ) {
				int row = vo_table->row(disb->item);
				if ( row >= 0 ) {
					vo_table->removeRow( row );
					if ( row == 0 ) vo_table->selectRow( 1 );
					else vo_table->selectRow(row);
				}
			}
		}
	}

    void QtDataManager::addVOParam( ) {
        // how many "available to activate" flex params...
        int count = vo_flex_params->count( );
        if ( count == 0 ) return;
        // fetch the current "available" flex param...
        int cur = vo_flex_params->currentRow( );
        QListWidgetItem *item = vo_flex_params->item(cur);
        if ( ! item ) return;
        QString current = item->text( );
        // find state for current param...
        for ( int i=0; i < voparameters.size( ); ++i ) {
            if ( current == get<1>(voparameters[i]) ) {
                int entries = vo_flex_params_entry->count( );
                // remove current param from the "available to activate" list...
                vo_flex_params->removeItemWidget(item);
                delete item;
                // create new flex param entry widget...
                QListWidgetItem *newitem = new QListWidgetItem( );
                vo_flex_params_entry->addItem(newitem);
                viewer::dvo::param *param = new viewer::dvo::param( vo_flex_params_entry, newitem );
                connect( param->edit, SIGNAL(textChanged(QString)), this, SLOT(vo_clear_param( )) );
                get<4>(voparameters[i]) = param;
                // if the size hint is not set, then the group boxes pile up...
                newitem->setSizeHint(get<4>(voparameters[i])->minimumSizeHint());
                vo_flex_params_entry->setItemWidget(newitem,get<4>(voparameters[i]));
                // mark the parameter as active...
                get<0>(voparameters[i]) = true;
                // set the focus to the line edit widget...
                get<4>(voparameters[i])->setFocus( );
                // set the title and default value...
                get<4>(voparameters[i])->setTitle(get<1>(voparameters[i]));
                get<4>(voparameters[i])->edit->setText(get<3>(voparameters[i]));
                // set the new active parameter entry as the active item...
                vo_flex_params_entry->setCurrentItem(newitem);
                int choices = vo_flex_params->count( );
                // enable the "remove" button if this is the first "enabled" flex param...
                if ( entries == 0 ) voremoveb->setEnabled(true);
                // disable the "add" button if this is the last parameter available for activation...
                if ( choices == 0 ) voaddb->setEnabled(false);
                break;
            }
        }

    }

    void QtDataManager::removeVOParam( ) {
        // how many active flex params...
        int count = vo_flex_params_entry->count( );
        if ( count == 0 ) return;
        // fetch current flex param...
        int cur = vo_flex_params_entry->currentRow( );
        QListWidgetItem *item = vo_flex_params_entry->item(cur);
        if ( ! item ) return;
        viewer::dvo::param *param = static_cast<viewer::dvo::param*>(vo_flex_params_entry->itemWidget(item));
        QString current = param->title( );
        // find state for current param...
        for ( int i=0; i < voparameters.size( ); ++i ) {
            if ( current == get<1>(voparameters[i]) ) {
                int choices = vo_flex_params->count( );
                // remove current flex param...
                vo_flex_params_entry->removeItemWidget(item);
                delete item;
                // add choice back in the "available to activate" list...
                vo_flex_params->addItem(get<1>(voparameters[i]));
                // clear param widget pointer...
                get<4>(voparameters[i]) = 0;
                // mark parameter as not active...
                get<0>(voparameters[i]) = false;
                // update current row among the remaining "active" entries...
                int entries = vo_flex_params_entry->count( );
                if ( entries > cur ) vo_flex_params_entry->setCurrentRow( cur );
                else if ( entries > 0 ) vo_flex_params_entry->setCurrentRow( cur-1 );
                else voremoveb->setEnabled(false);
                // enable the "activate" button if the "available to activate"
                // list was initially empty...
                if ( choices == 0 ) voaddb->setEnabled(true);
                // update the current row in the "available to activate" list...
                vo_flex_params->setCurrentRow(choices);
                break;
            }
        }
    }

    QPushButton *QtDataManager::new_vo_dismiss_button( QTableWidgetItem *i, QString url, QString type ) {
        QSizePolicy sizepolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		QPushButton *dismiss = new dismiss_button_t(i,url,type,vo_selected_service,vo_table);
		connect( dismiss, SIGNAL(pressed( )), SLOT(vo_dismiss_row( )) );
        dismiss->setObjectName(QString::fromUtf8("dismiss"));
        sizepolicy.setHeightForWidth(dismiss->sizePolicy().hasHeightForWidth());
        dismiss->setSizePolicy(sizepolicy);
        dismiss->setMaximumSize(QSize(25, 25));
        dismiss->setFlat(true);
        dismiss->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
        dismiss->setToolTip(QApplication::translate("QtDataManager", "delete this row", 0, QApplication::UnicodeUTF8));
        return dismiss;
    }

	void QtDataManager::vo_clear_status( ) { status( "" ); }
	void QtDataManager::vo_clear_status_delayed( int seconds ) {
		QTimer::singleShot( seconds * 1000, this, SLOT(vo_clear_status( )) );
	}

	void QtDataManager::vo_action_with_timeout( int id, int seconds, QString msg ) {
		QApplication::setOverrideCursor(Qt::WaitCursor);
		vo_action_timeout_id = id;
		vo_action_timeout_msg = msg;
		vo_disable_actions( );
		vo_action_timeout->setInterval( seconds*1000 );
		vo_action_timeout->start( );
	}

	void QtDataManager::vo_disable_actions( ) {
		vo_contour->setEnabled(false);
		vo_raster->setEnabled(false);
		vo_query->setEnabled(false);
		vo_clear->setEnabled(false);
		vo_actions_are_enabled = false;
	}
	void QtDataManager::vo_enable_actions( ) {
		vo_action_timeout->stop( );
		vo_action_timeout->setInterval( 0 );
		if ( vo_selected_rows.size( ) > 0 ) {
			vo_contour->setEnabled(true);
			vo_raster->setEnabled(true);
		}
		vo_query->setEnabled(true);
		vo_clear->setEnabled(true);
		vo_actions_are_enabled = true;
	}

	void QtDataManager::vo_action_timed_out( ) {
		vo_action_with_timeout_complete( );
		error( vo_action_timeout_msg + " timed out..." );
		updateVOstatus( );
	}

	void QtDataManager::vo_action_with_timeout_complete( ) {
		QApplication::restoreOverrideCursor();
		vo_enable_actions( );
		vo_action_timeout_id = 0;
		vo_action_timeout_msg = "";
	}

} //# NAMESPACE CASA - END
