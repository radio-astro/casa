#ifndef REGION_QTREGIONSTATS_H_
#define REGION_QTREGIONSTATS_H_

#include <QtGui/QGroupBox>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QVBoxLayout>
#include <casa/BasicSL/String.h>
#include <display/region/QtImageRegionStats.ui.h>
#include <display/region/QtMsRegionStats.ui.h>
#include <display/region/SlicerStats.ui.h>
#include <display/region/PVLineStats.ui.h>
#include <display/region/RegionInfo.h>


class QStackedWidget;

namespace casa {
	namespace viewer {

		class Region;

		namespace qt {

			typedef std::list<std::pair<QGroupBox*,QLabel*> > statfield_list_t;

			class stats_t : public QGroupBox {
				Q_OBJECT
			public:
				stats_t( QWidget *parent ) : QGroupBox("",parent) { }
				void setLabels( const std::string &label, const std::string &desc );
				virtual RegionInfo::InfoTypes type( ) const = 0;
				virtual QPushButton *next( ) = 0;
				virtual bool updateStatisticsInfo( shared_ptr<casa::viewer::RegionInfo> ) {
					return false;
				}
			protected:
				std::string description_;
				std::string label_;
			};

			class image_stats_t : public stats_t, public Ui::QtImageRegionStats {
				Q_OBJECT
			public:
				image_stats_t( statfield_list_t &fields, QWidget *parent=0 );
				RegionInfo::InfoTypes type( ) const {
					return RegionInfo::ImageInfoType;
				}
				virtual QPushButton *next( ) {
					return next_button;
				}
			};

			class ms_stats_t : public stats_t, public Ui::QtMsRegionStats {
				Q_OBJECT
			public:
				ms_stats_t( statfield_list_t &fields, QWidget *parent=0 );
				RegionInfo::InfoTypes type( ) const {
					return RegionInfo::MsInfoType;
				}
				virtual QPushButton *next( ) {
					return next_button;
				}
			};

			class SliceStats : public stats_t, public Ui::SlicerStats {
				Q_OBJECT
			public:
				SliceStats( QWidget *parent=0 );
				RegionInfo::InfoTypes type( ) const {
					return RegionInfo::SliceInfoType;
				}
				virtual QPushButton *next( ) {
					return next_button;
				}
				QWidget* getPlotHolder() {
					return plotHolderWidget;
				}
				bool updateStatisticsInfo( shared_ptr<casa::viewer::RegionInfo> );
			signals:
				void show1DSliceTool();
			};

			class pvline_stats_t : public stats_t, public Ui::PVLineStats {
				Q_OBJECT
			public:
				pvline_stats_t( QWidget *parent=0 );
				RegionInfo::InfoTypes type( ) const {
					return RegionInfo::PVLineInfoType;
				}
				virtual QPushButton *next( ) {
					return next_button;
				}
				bool updateStatisticsInfo( shared_ptr<casa::viewer::RegionInfo> );
			protected:
				void enterEvent (QEvent *);
				void showEvent ( QShowEvent * );

			signals:
				void createPVImage(const std::string&,const std::string&,int);
				void setPVInfo(int width);

			private slots:
				void create_pv_image( );
			};
		}


		class QtRegionStats : public QWidget {
			Q_OBJECT
		public:
			QtRegionStats( QWidget *parent=0 );
			~QtRegionStats( );

			void disableNextButton( );
			void reset( );
#if OLDSTUFF
			void addstats( std::list<std::pair<String,String> > *stats );
#endif

			void updateStatistics( shared_ptr<casa::viewer::RegionInfo> stats, Region* region = NULL );

			void setCenterBackground(QString background);

			void setNext( QStackedWidget *, QtRegionStats * );

			bool updateStatisticsInfo( shared_ptr<casa::viewer::RegionInfo> );

		protected slots:
			void go_next( );

		protected:
			qt::statfield_list_t fields;

			QStackedWidget *container_;
			QtRegionStats *next_;

		private:
			QVBoxLayout *layout_;
			qt::stats_t *new_stats_box( RegionInfo::InfoTypes type, Region* region, const string& label );
			qt::stats_t *stats_box_;
			std::string description_;
			std::string label_;
		};
	}
}

#endif
