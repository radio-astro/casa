//# QtDataManager.qo.h: Qt implementation of viewer data manager widget.
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

#ifndef QTDATAMANAGER_H_
#define QTDATAMANAGER_H_
// (Trailing underscore is not a typo -- do not remove it;
// QtDataManager.ui.h uses the QTDATAMANAGER_H symbol).


#include <casa/aips.h>
#include <casa/cppconfig.h>
#include <casa/BasicSL/String.h>
#include <display/QtViewer/QtLELEdit.qo.h>
#include <display/Utilities/ImageProperties.h>
#include <display/QtViewer/SlicerGen.qo.h>

#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QColor>
#include <QHash>
#include <QWidget>
//#dk Be careful to put *.ui.h within X_enter/exit bracket too,
//#   because they'll have Qt includes.
//#   E.g. <QApplication> needs the X11 definition of 'Display'
#include <display/QtViewer/QtDataManager.ui.h>
#include <display/QtViewer/QtDataMgrMsSelect.ui.h>
#include <display/QtViewer/VOParam.ui.h>
#include <casaqt/QtDBus/dVO.h>
#include <graphics/X11/X_exit.h>
#include <display/Utilities/Lowlevel.h>
#include <set>
#include <list>

namespace casa { //# NAMESPACE CASA - BEGIN

	class QtDisplayPanelGui;
	class QtDisplayPanel;
	class QtDisplayData;
	template <class T> class ImageInterface;

	namespace viewer {
		class Region;

        namespace dvo {
            // entry of flexible VO parameters...
            class param : public QGroupBox, public Ui::VOParam {
                Q_OBJECT
                public:
                    param( QListWidget *l, QListWidgetItem *i, QWidget *parent=0 ) : QGroupBox(parent), list(l), item(i) { setupUi(this); }
                protected:
                    void mousePressEvent ( QMouseEvent * event );
                private:
                    QListWidget *list;
                    QListWidgetItem *item;
            };
        }
	}

	class QtDataManager : public QWidget, private Ui::QtDataManager {

		Q_OBJECT

	public:

		operator QObject*( ) { return dynamic_cast<QObject*>((QWidget*)this); }

		typedef std::list<std::pair<QGroupBox*,QLineEdit*> > infofield_list_t;

		QtDataManager( QtDisplayPanelGui* panel=0, const char* name=0, QWidget* parent=0 );
		~QtDataManager();

		/*   String path() const { return dir_.path().toStdString();  } */

		void updateDirectory(const std::string &);

		void showTab( std::string );
		void clearRegionMap();

	signals:

		void tableReadErrorSignal(String msg);


	protected:

        void showEvent( QShowEvent *event );
        void enterEvent( QEvent  *event );
		void showDisplayButtons(int,const QString &name=QString((const char *)0));
		void hideDisplayButtons();
		QColor getDirColor(int);
		QStringList analyseFITSImage(QString path);
		Bool isQualImg(const QString &extexpr);


		std::string guimethod_to_iamethod( const QString & );

		enum DATATYPE { UNKNOWN, IMAGE, MEASUREMENT_SET, SKY_CATALOG, RESTORE,
		                DIRECTORY, QUALIMG, CASAREGION, DS9REGION
		              };
		enum DISPLAYTYPE { RASTER, CONTOUR, VECTOR, MARKER, SKY_CAT,
		                   NEWPANEL, OLDPANEL
		                 };

		QHash<int, QString> dataType_;
		QHash<QString, int> uiDataType_;
		QHash<QString, int> displayType_;
		QHash<QString, int> uiDisplayType_;
		QVector<int>        exportTypes_;

		viewer::ImageProperties image_properties;
		infofield_list_t load_ifields;
		/*   infofield_list_t img_ifields; */

	public slots:

		void updateDisplayDatas(QtDisplayData* qdd=0, Bool autoRegister=True);

	protected slots:

		void handle_region_update(viewer::Region*,std::string);

		void createButtonClicked();
		void clickItem(QTreeWidgetItem* item);
		void expandItem(QTreeWidgetItem* item);
		void changeItemSelection();
		void returnPressed();
		void buildDirTree( std::string newdir="" );
		void lelGotFocus_();
		void load_regions_clicked( );

		// Restore viewer state to existing panel.
		// Use the first empty panel, or if none, the first panel.
		void restoreToOld_();

		// Create new display panel, restore viewer state to it.
		void restoreToNew_();

		// Restore viewer state to given panel.
		void restoreTo_(QtDisplayPanel* dp);


		void showDDCreateError_(String);

		void leaveopenButtonClicked( bool checked );
		void showlelButtonClicked( bool checked );
		void showSliceButtonClicked( bool checked );

		void changeTabContext(int);

		void img_output_target_changed(const QString &s="");
		void img_output_target_changed(bool) {
			img_output_target_changed( );
		}

		void region_ds9_csys_disable(bool);
		void region_output_target_changed(const QString &s="");
		void region_output_target_changed(bool) {
			region_output_target_changed( );
		}

		void img_do_output( );
		void region_do_output( );

		void region_item_state_change(QTreeWidgetItem*,int);
		void region_selection_change( viewer::Region *rgn, bool selected );

        //---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---
        //  VO controls...
        //---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---
        void addVOParam( );
        void removeVOParam( );
        //---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---

    private slots:
         void enable_disable_slice( const QString & );
         void enable_disable_regrid( bool );
	private:

		typedef std::map<QString,QtDisplayData*> display_data_map_t;
		display_data_map_t display_datas;

		typedef std::map<QTreeWidgetItem*,viewer::Region*> treeitem_map_t;
		treeitem_map_t treeitem_to_region;
		typedef std::map<viewer::Region*,QTreeWidgetItem*> region_map_t;
		region_map_t region_to_treeitem;

		struct validation_msg {
			enum output_format_t { FITS, CASA, CRTF, DS9 };
			enum valid_t { VALID, INVALID, WARN };
			validation_msg( const QString &s, output_format_t f, valid_t v, const QString &m ) : path(s), output_format(f), valid(v), msg(m) { }
			validation_msg( const validation_msg &o ) : path(o.path), output_format(o.output_format), valid(o.valid), msg(o.msg) { }
			validation_msg( ) : path(/* QString::Null */), output_format(CASA), valid(INVALID), msg(/* QString::Null */) {
				fprintf( stderr, "HERE<3>:::: " );
			}
			validation_msg operator=(const validation_msg &o ) {
				path = o.path;
				output_format = o.output_format;
				valid = o.valid;
				msg = o.msg;
				return *this;
			}
			QString canonical_path( const QString & );
			QString path;
			output_format_t output_format;
			valid_t valid;
			QString msg;
		};

		validation_msg output_validation( const QString &, const QString &, validation_msg::output_format_t ) const;

		class tab_state {
		public:
			typedef void (QtDataManager::*notify_func_t)( const std::string &, tab_state & );
			tab_state( ) : dir_entry_(0), tree_(0), dir_(new QDir), notify_func_(0),
				info_frame_(0), info_fields_(0), dtree_(0),
				output_name_line_(0), error_func_(0) {
				dir_->setFilter( QDir::AllDirs | QDir::Files);
				dir_->setSorting(QDir::Name);
			}
			tab_state( const tab_state &o ) : filtered_types_(o.filtered_types_), dir_entry_(o.dir_entry_),
				tree_(o.tree_), dir_(o.dir_), notify_func_(o.notify_func_),
				info_frame_(o.info_frame_), info_fields_(o.info_fields_),
				dtree_(o.dtree_), output_name_line_(o.output_name_line_),
				error_func_(o.error_func_) { }
			tab_state( QLineEdit *le, QTreeWidget *t, std::set<int> s, QFrame *info_frame=0,
			           infofield_list_t *info_fields=0, notify_func_t f=0,
			           QTreeWidget *d=0, QLineEdit *on=0, notify_func_t oerr=0 ) :
				filtered_types_(s), dir_entry_(le), tree_(t), dir_(new QDir),
				notify_func_(f), info_frame_(info_frame), info_fields_(info_fields),
				dtree_(d), output_name_line_(on), error_func_(oerr) {
				dir_->setFilter( QDir::AllDirs | QDir::Files);
				dir_->setSorting(QDir::Name);
			}
			const std::set<int> &filter( ) const {
				return filtered_types_;
			}
			void updateDir( ) {
				if ( dir_entry_ ) {
					dir_entry_->setText(dir_->currentPath( ));
				}
			}

			void notify( QtDataManager *mgr, const std::string &s ) {
				if ( notify_func_ ) (mgr->*notify_func_)(s,*this);
			}
			void notifyErr( QtDataManager *mgr, const std::string &s ) {
				if ( error_func_ ) (mgr->*error_func_)(s,*this);
			}

			QTreeWidget *tree( ) {
				return tree_;
			}
			QFrame *infoFrame( ) {
				return info_frame_;
			}
			infofield_list_t *infoFields( ) {
				return info_fields_;
			}

			QTreeWidget *dtree( ) {
				return dtree_;
			}
			QLineEdit *outFileLine( ) {
				return output_name_line_;
			}

			shared_ptr<QDir> dir( ) {
				return dir_;
			}
			QLineEdit *dirline( ) {
				return dir_entry_;
			}
			std::string dirStr( ) const {
				return dir_entry_->text( ).toStdString( );
			}
		private:
			std::set<int> filtered_types_;
			QLineEdit *dir_entry_;
			QTreeWidget *tree_;
			shared_ptr<QDir> dir_;
			notify_func_t notify_func_;
			QFrame *info_frame_;
			infofield_list_t *info_fields_;

			QTreeWidget *dtree_;
			QLineEdit *output_name_line_;
			notify_func_t error_func_;

		};

		void update_regrid_options( );
		void update_slice_options( int ddtp,const QString &name );
		void fill_image_info( const std::string &/*path*/ );

		QWidget *parent_;
		QtDisplayPanelGui* panel_;
		Ui::QtDataMgrMsSelect *ms_selection;

		// avoid setting the export type more than once
		QString last_image_extension_tweak_string;
		QString last_region_extension_tweak_string;

		// connection to rc file
		Casarc &rc;

		typedef std::map<int,tab_state> tab_info_map_t;
		tab_info_map_t tab_info;
		void init_tab_info( );

		void load_tab_notify( const std::string &value, tab_state & );

		void update_dd_list( tab_state &ts );
		void image_tab_notify( const std::string &value, tab_state & );
		void image_tab_error( const std::string &value, tab_state & );

		void update_region_list( tab_state &ts );
		void region_tab_notify( const std::string &value, tab_state & );
		void region_tab_error( const std::string &value, tab_state & );

		bool export_to_fits(ImageInterface<Float> *img, std::string outFile);
		bool export_to_casa(ImageInterface<Float> *img, String outFile);
		void getSpectralCoordFlags(const ImageInterface<Float>* img, Bool &preferVelocity, Bool &opticalVelocity, Bool &preferWavelength, Bool &preferAirWavelength);

		viewer::SlicerGen *slice_gen;

        bool slice_available;
        bool regrid_available;

        void setupVO( );
        // returns true if everything is OK...
        bool updateVOstatus( );
        QPushButton *new_vo_dismiss_button( QTableWidgetItem*, QString, QString );
        bool collect_vo_parameters( double &ra, double &dec, double &ra_size, double &dec_size, QVariantMap &params );
        void vo_flag_missing_param( QLineEdit *widget );
        // (enabled, gui label, vo parameter name,vo parameter default,entry box)
        typedef std::tuple<bool,QString,QString,QString,viewer::dvo::param*> vo_param_t;
        std::vector<vo_param_t> voparameters;
        edu::nrao::casa::dVO dvo;
        std::set<QWidget*> dvo_missing_parameters;

        std::map<QString,int> vo_label2col;
        std::vector<QString> vo_urls;
        std::vector<QString> vo_labels;
        std::vector<QString> vo_labels_tip;

        QColor vo_default_bg;
		struct dvo_working_item {
			int id;
			unsigned int count;
			bool operator<( const dvo_working_item &other ) const { return id < other.id; }
			operator int( ) const { return id; }
			dvo_working_item( int i ) : id(i), count(0) { }
			dvo_working_item( QDBusPendingReply<int> i ) : id(i), count(0) { }
			// set find etc. yeilds constant iterators (even though changes to
			// the set item may have no effect on set ordering...)
			void increment( ) const { ((dvo_working_item*)this)->count += 1; }
		};
		std::set<dvo_working_item> dvo_working_set;
		QPushButton *vo_current_action;

		void vo_clear_status_delayed( int seconds );
		void vo_action_with_timeout( int id, int seconds, QString msg );
		bool vo_action_with_timeout_active( ) const { return vo_action_timeout->isActive( ); }
		void vo_action_with_timeout_reset( ) {
			if ( vo_action_with_timeout_active( ) ) vo_action_timeout->start( );
		}
		void vo_action_with_timeout_complete( );

    private slots:

		void vo_service_select( const QString &service ) { vo_selected_service = service; }
		void vo_selection_changed( );

		void vo_clear_table( ) {
			vo_table->clear( );
			vo_table->setRowCount( 0 );
			vo_init_columns( );
			vo_label2col.clear( );
			vo_urls.clear( );
			vo_labels.clear( );
			vo_labels_tip.clear( );
		}
        void vo_launch_query( );
		void vo_fetch_data( );
        void vo_clear_param( );
        void vo_query_begin( int, const QString&, const QtStringMap& );
        void vo_query_complete( int, const QString&, const QtStringMap& );
        void vo_query_description( int, const QString&, const QtStringMap& );
        void vo_query_row( int, const QString&, const QtStringMap& );
		void vo_error( int, const QString&, const QString& );
		void vo_dismiss_row( );
		void vo_fetch_complete( int, QString );
		void vo_fetch_progress(int,QString,double,double,double,double);

		void vo_clear_status( );
		void vo_disable_actions( );
		void vo_enable_actions( );
		void vo_action_timed_out( );

	private:
		void error( const QString &msg ) {
			vo_status->setStyleSheet("color: red");
			vo_status->setText( msg );
		}
		void status( const QString &msg ) {
			vo_status->setStyleSheet("color: black");
			vo_status->setText( msg );
		}
		void warning( const QString &msg ) {
			vo_status->setStyleSheet("color: orange");
			vo_status->setText( msg );
		}

		void vo_init_columns( ) {
			vo_table->setColumnCount( 2 );
			vo_table->setColumnWidth(0,30);
			QTableWidgetItem *first = new QTableWidgetItem( "" );
			vo_table->setHorizontalHeaderItem(0,first);
			QTableWidgetItem *second = new QTableWidgetItem( "1" );
			vo_table->setHorizontalHeaderItem(1,second);
		}

        QDoubleValidator *ra_val;
        QDoubleValidator *dec_val;
        QDoubleValidator *ra_size_val;
        QDoubleValidator *dec_size_val;

		QTimer *vo_action_timeout;
		int vo_action_timeout_id;
		QString vo_action_timeout_msg;
		bool vo_actions_are_enabled;

		std::map<QString,QString> vo_service_name_to_url;
		QString vo_selected_service;
		std::vector<int> vo_selected_rows;

		friend void lambda_dsoc_test_pre_( QtDataManager& );
	};

} //# NAMESPACE CASA - END

#endif
