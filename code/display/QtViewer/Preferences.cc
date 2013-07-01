//# Preferences.cc: dialog which allows user to set and reset viewer preferences
//# Copyright (C) 2012
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

#include <display/QtViewer/Preferences.qo.h>
#include <display/DisplayErrors.h>
#include <QLineEdit>
#include <QRegExp>

namespace casa {
	namespace viewer {

		// Fields in the table are:
		//
		//     NODE or LEAF   |   TYPE (N=node, L=leaf)   |   Default Value   |   Tool Tip   |   Validation Type
		//
		// Note that NODEs do not have default values or validation types because they do not actually
		// represent something that the user can set, but may represent something worth providing
		// intermediate tool tips for the user...
		//
		const char *Preferences::init_state[] = {
			"viewer.dpg", "N", "", "preferences for main Viewer Display Panel", "",
			"viewer.dpg.position", "N", "", "viewer dock panel positions", "",
			"viewer.dpg.position.animator", "L", "right", "in which dock (left, right, top, bottom) should the animator be shown", "L/R/T/B",
			"viewer.dpg.position.cursor_tracking", "L", "right", "in which dock (left, right, top, bottom) should the cursor tracking be shown", "L/R/T/B",
			"viewer.dpg.dimensions", "L", "1185 609", "dimensions for the main viewer panel", "2int",
			"viewer.dpg.bottomdock", "L", "untabbed", "bottom dock state (tabbed, untabbed)", "tabs",
			"viewer.dpg.position.mousetools", "L", "top", "dock position of the mouse tools", "L/R/T/B",
			"viewer.dpg.rightdock", "L", "untabbed", "right dock state (tabbed, untabbed)", "tabs",
			"viewer.dpg.position.regions", "L", "right", "in which dock (left, right, top, bottom) should the region panel be shown", "L/R/T/B",
			"viewer.dpg.datamgr.show_lel", "L", "false", "include LEL expression dialog in the viewer's Data Manager", "T/F",
			"viewer.dpg.datamgr.leave_up", "L", "false", "leave the viewer's Data Manager up after opening a data object", "T/F",
			"viewer.dpg.visible.regions", "L", "true", "should the dock widget for regions be visible", "T/F",
			"viewer.dpg.visible.cursor_tracking", "L", "true", "should the dock widget for cursor tracking be visible", "T/F",
			"viewer.dpg.visible.animator", "L", "true", "should the dock widget for cube animation be visible", "T/F",

			"viewer.iclean", "N", "", "preferences for interactive clean panel", "",
			"viewer.iclean.position", "N", "", "interactive clean dock panel positions", "",
			"viewer.iclean.position.animator", "L", "bottom", "in which dock (left, right, top, bottom) should the animator be shown", "L/R/T/B",
			"viewer.iclean.position.cursor_tracking", "L", "bottom", "in which dock (left, right, top, bottom) should the cursor tracking be shown", "L/R/T/B",
			"viewer.iclean.dimensions", "L", "700 900", "dimensions for the main viewer panel", "2int",
			"viewer.iclean.bottomdock", "L", "untabbed", "bottom dock state (tabbed, untabbed)", "tabs",
			"viewer.iclean.position.mousetools", "L", "top", "dock position of the mouse tools", "L/R/T/B",
			"viewer.iclean.rightdock", "L", "untabbed", "right dock state (tabbed, untabbed)", "tabs",
			"viewer.iclean.position.regions", "L", "bottom", "in which dock (left, right, top, bottom) should the region panel be shown", "L/R/T/B",
			"viewer.iclean.datamgr.show_lel", "L", "false", "include LEL expression dialog in the viewer's Data Manager", "T/F",
			"viewer.iclean.datamgr.leave_up", "L", "false", "leave the viewer's Data Manager up after opening a data object", "T/F",
			"viewer.iclean.visible.regions", "L", "false", "should the dock widget for regions be visible", "T/F",
			"viewer.iclean.visible.cursor_tracking", "L", "true", "should the dock widget for cursor tracking be visible", "T/F",
			"viewer.iclean.visible.animator", "L", "true", "should the dock widget for cube animation be visible", "T/F",

			"viewer.prf", "N", "", "spectral profile tool preferences", "",
			"viewer.prf.error.type", "L", "no error", "type of error displayed with extracted spectra (no error, propagated, rmse)", "N/P/R",
			"viewer.prf.plot.type", "L", "mean", "combine type to compute extracted spectra (mean, median, sum)", "M/M/S",
			"viewer.prf.freqcoord.type", "L", "radio velocity [km/s]", "spectral units for extracted spectra", "unchecked",
			"viewer.prf.collapse.type", "L", "mean", "combine type to compute collapsed image (mean, median, sum)", "M/M/S",
			"viewer.prf.collerror.type", "L", "no error", "typer of error to compute for collapsed image (no error, propagated, rmse)", "N/P/R",0
		};

		// validator class declarations...
		class TrueFalseValidator : public QValidator {
		public:
			TrueFalseValidator( QObject *parent=0 ) : QValidator(parent) { }
			State validate( QString &input, int & ) const {
				QString t("true"), f("false");
				QString val = input.toLower( );
				if ( val == t || val == f ) return Acceptable;
				if ( t.contains(val) || f.contains(val) ) return Intermediate;
				return Invalid;
			}
		};

		class LRTBValidator : public QValidator {
		public:
			LRTBValidator( QObject *parent=0 ) : QValidator(parent) { }
			State validate( QString &input, int & ) const {
				QString l("left"), r("right"), t("top"), b("bottom");
				QString val = input.toLower( );
				if ( val == l || val == r || val == t || val == b ) return Acceptable;
				if ( l.contains(val) || r.contains(val) || t.contains(val) || b.contains(val) )
					return Intermediate;
				return Invalid;
			}
		};

		class TabValidator : public QValidator {
		public:
			TabValidator( QObject *parent=0 ) : QValidator(parent) { }
			State validate( QString &input, int & ) const {
				QString t("tabbed"), u("untabbed");
				QString val = input.toLower( );
				if ( val == t || val == u ) return Acceptable;
				if ( t.contains(val) || u.contains(val) ) return Intermediate;
				return Invalid;
			}
		};

		class IntPairValidator : public QValidator {
		public:
			IntPairValidator( QObject *parent=0 ) : QValidator(parent) { }
			State validate( QString &input, int & ) const {
				QRegExp intpair("(\\d+)\\s+(\\d+)");
				if ( intpair.exactMatch(input) ) return Acceptable;
				QRegExp intval("\\d+");
				QRegExp space("\\s+");
				if ( input.contains(intval) || input.contains(space) ) return Intermediate;
				return Invalid;
			}
		};

		class CombineTypeValidator : public QValidator {
		public:
			CombineTypeValidator( QObject *parent=0 ) : QValidator(parent) { }
			State validate( QString &input, int & ) const {
				QString mean("mean"), median("median"), sum("sum");
				QString val = input.toLower( );
				if ( val == mean || val == median || val == sum ) return Acceptable;
				if ( mean.contains(val) || median.contains(val) || sum.contains(val) ) return Intermediate;
				return Invalid;
			}
		};

		class ErrorTypeValidator : public QValidator {
		public:
			ErrorTypeValidator( QObject *parent=0 ) : QValidator(parent) { }
			State validate( QString &input, int & ) const {
				QString noerror("no error"), propa("propagated"), rmse("rmse");
				QString val = input.toLower( );
				if ( val == noerror || val == propa || val == rmse ) return Acceptable;
				if ( noerror.contains(val) || propa.contains(val) || rmse.contains(val) ) return Intermediate;
				return Invalid;
			}
		};

		int Preferences::tokenize( tokenized_type &tokenized ) {
			char sep[] = ".";
			int buf_size = 256;
			char *buf = (char*) malloc(buf_size*sizeof(char));
			int rc_count = 0;

			// for use in setting up validation routines...
			TrueFalseValidator   *tfval      = new TrueFalseValidator( );
			LRTBValidator        *lrtbval    = new LRTBValidator( );
			TabValidator         *tabval     = new TabValidator( );
			IntPairValidator     *intpairval = new IntPairValidator( );
			CombineTypeValidator *combtypeval= new CombineTypeValidator( );
			ErrorTypeValidator   *errtypeval = new ErrorTypeValidator( );

			for ( const char **p=init_state; *p; p+=init_fields ) {
				// store tips for nodes & leaves...
				tips.insert(tooltips_type::value_type(p[0],p[tip_offset]));

				// only leaves represent actual rc values...
				if ( p[role_offset][0] != 'L' ) continue;

				++rc_count;

				std::string validate_type(p[validate_offset]);
				if ( validate_type == "L/R/T/B" )
					validate_map.insert(validate_map_type::value_type(p[0],lrtbval));
				else if ( validate_type == "2int" )
					validate_map.insert(validate_map_type::value_type(p[0],intpairval));
				else if ( validate_type == "tabs" )
					validate_map.insert(validate_map_type::value_type(p[0],tabval));
				else if ( validate_type == "T/F" )
					validate_map.insert(validate_map_type::value_type(p[0],tfval));
				else if ( validate_type == "M/M/S" )
					validate_map.insert(validate_map_type::value_type(p[0],combtypeval));
				else if ( validate_type == "N/P/R" )
					validate_map.insert(validate_map_type::value_type(p[0],errtypeval));

				// store system defaults...
				defaults.insert(rcvalues_type::value_type(p[0],p[default_offset]));

				int size = strlen(p[0]) + 1;
				if ( size > buf_size ) {
					while ( size > buf_size ) buf_size *= 2;
					buf = (char*) realloc( buf, buf_size*sizeof(char) );
				}
				memcpy( buf, p[0], buf_size );
				std::vector<std::string> tokens;
				for( char *val = strtok(buf,sep); val; val = strtok(NULL, sep) ) {
					tokens.push_back(val);
				}
				tokenized.insert(tokenized_type::value_type(p[0],tokens));
			}

			free(buf);
			return rc_count;
		}

		int Preferences::countbranches( const tokenized_type &tokenized, branch_count_type &branch_count ) {
			std::set<std::string> roots;
			// calculate node branch count...
			for ( tokenized_type::const_iterator it = tokenized.begin( ); it != tokenized.end( ); ++it ) {
				std::string parent;
				for ( std::vector<std::string>::const_iterator tokit = it->second.begin(); tokit != it->second.end(); ++tokit ) {
					if ( parent.size( ) == 0 ) {
						parent = *tokit;
						roots.insert(parent);
						continue;
					}
					branch_count_type::iterator fpit = branch_count.find(parent);
					if ( fpit == branch_count.end( ) ) {
						branch_count.insert(branch_count_type::value_type(parent,std::set<std::string>()));
						fpit = branch_count.find(parent);
					}
					fpit->second.insert(*tokit);
					parent = parent + "." + *tokit;
				}
				branch_count.insert(branch_count_type::value_type(parent,std::set<std::string>()));
			}
			return roots.size( );
		}


		int Preferences::find_tree_depth( bool trim_root, const tokenized_type &tokenized, const branch_count_type &branch_count ) {
			int depth = 0;
			for ( tokenized_type::const_iterator it = tokenized.begin( ); it != tokenized.end( ); ++it ) {
				int cur_depth = 0;
				std::vector<std::string>::const_iterator tokit = it->second.begin();
				std::vector<std::string>::const_iterator tokit_end = it->second.end();

				if ( tokit == tokit_end ) continue;

				std::string root;
				std::string prefix;

				// trim off root node (because there's a single root)...
				if ( trim_root )
					prefix = *tokit++ + ".";

				for ( ; tokit != tokit_end; ++tokit ) {
					std::string node = root + prefix + *tokit;
					branch_count_type::const_iterator fpit = branch_count.find(node);
					if ( fpit == branch_count.end( ) ) {
						break;
					}
					int count = fpit->second.size( );
					if ( count == 1 ) {
						// combine nodes...
						prefix = prefix + *tokit + ".";
						continue;
					} else if ( count > 1 ) {
						// branching node...
						root = root + prefix + *tokit + ".";
						prefix = "";
						++cur_depth;
					}
				}
				// tree depth (minus top level)...
				if ( cur_depth > depth )
					depth = cur_depth;
			}

			return depth+1;
		}

		void Preferences::init( ) {

			// tokenize...
			tokenized_type tokenized;
			/*int rc_count = */
			tokenize(tokenized);

			// calculate per node branch count...
			branch_count_type branch_count;
			int root_count = countbranches( tokenized, branch_count );

			const bool combine_root_and_first = (root_count == 1);
			const int tree_depth = find_tree_depth( combine_root_and_first, tokenized, branch_count );

			treeWidget->setColumnCount(tree_depth+1);	// +1 for the editline

			// construct preference tree...
			typedef std::map<std::string,QTreeWidgetItem*> treewidget_map_type;
			treewidget_map_type widgets;
			for ( tokenized_type::iterator it = tokenized.begin( ); it != tokenized.end( ); ++it ) {
				std::string root;
				std::string prefix;
				int cur_depth = 0;

				std::vector<std::string>::iterator tokit = it->second.begin();
				std::vector<std::string>::iterator tokit_end = it->second.end();

				if ( tokit == tokit_end ) continue;

				if ( combine_root_and_first )
					prefix = *tokit++ + ".";

				for ( ; tokit != tokit_end; ++tokit ) {
					std::string node = prefix + *tokit;
					std::string fully_qualified;
					if ( root.size( ) != 0 )
						fully_qualified = root + "." + node;
					else
						fully_qualified = node;
					branch_count_type::iterator fpit = branch_count.find(fully_qualified);
					if ( fpit == branch_count.end( ) ) {
						break;
					}
					int count = fpit->second.size( );
					if ( count == 1 ) {
						prefix = node + ".";
						continue;
					} else if ( count > 1 ) {
						if ( root.size( ) == 0 ) {
							treewidget_map_type::iterator it = widgets.find(fully_qualified);
							if ( it == widgets.end( ) && widgets.find(fully_qualified) == widgets.end( ) ) {
								QTreeWidgetItem *cur_widget = new QTreeWidgetItem(treeWidget);
								widgets.insert(treewidget_map_type::value_type(fully_qualified,cur_widget));
								cur_widget->setText(cur_depth, QString::fromStdString(node));
								tooltips_type::iterator tip = tips.find(fully_qualified);
								if ( tip != tips.end( ) )
									cur_widget->setToolTip(cur_depth,QString::fromStdString(tip->second));
							}
							++cur_depth;
							root = node;
						} else {
							treewidget_map_type::iterator it = widgets.find(root);
							if ( it == widgets.end( ) ) break;
							if ( widgets.find(fully_qualified) == widgets.end( ) ) {
								QTreeWidgetItem *cur_widget = new QTreeWidgetItem(it->second);
								widgets.insert(treewidget_map_type::value_type(fully_qualified,cur_widget));
								cur_widget->setText(cur_depth, QString::fromStdString(node));
								tooltips_type::iterator tip = tips.find(fully_qualified);
								if ( tip != tips.end( ) )
									cur_widget->setToolTip(cur_depth,QString::fromStdString(tip->second));
							}
							++cur_depth;
							root = root + "." + node;
						}
						prefix = "";
					} else {
						rcvalues_type::iterator dflt = defaults.find(fully_qualified);
						if ( dflt == defaults.end( ) ) break;
						if ( root.size( ) == 0 ) {
							QTreeWidgetItem *cur_widget = new QTreeWidgetItem(treeWidget);
							widgets.insert(treewidget_map_type::value_type(fully_qualified,cur_widget));
							cur_widget->setText(cur_depth, QString::fromStdString(node));
							QLineEdit *edit = new QLineEdit(treeWidget);
							edit->setText(QString::fromStdString(dflt->second));
							validate_map_type::iterator valit = validate_map.find(fully_qualified);
							if ( valit != validate_map.end( ) )
								edit->setValidator(valit->second);
							edits.insert(rcedits_type::value_type(fully_qualified,edit));
							treeWidget->setItemWidget(cur_widget,cur_depth+1,edit);
							tooltips_type::iterator tip = tips.find(fully_qualified);
							if ( tip != tips.end( ) ) {
								cur_widget->setToolTip(cur_depth,QString::fromStdString(tip->second));
								edit->setToolTip(QString::fromStdString(tip->second));
							}
						} else {
							treewidget_map_type::iterator it = widgets.find(root);
							if ( it == widgets.end( ) ) break;
							if ( widgets.find(fully_qualified) == widgets.end( ) ) {
								QTreeWidgetItem *cur_widget = new QTreeWidgetItem(it->second);
								widgets.insert(treewidget_map_type::value_type(fully_qualified,cur_widget));
								cur_widget->setText(cur_depth, QString::fromStdString(node));
								QLineEdit *edit = new QLineEdit(treeWidget);
								edit->setText(QString::fromStdString(dflt->second));
								validate_map_type::iterator valit = validate_map.find(fully_qualified);
								if ( valit != validate_map.end( ) )
									edit->setValidator(valit->second);
								edits.insert(rcedits_type::value_type(fully_qualified,edit));
								treeWidget->setItemWidget(cur_widget,cur_depth+1,edit);
								tooltips_type::iterator tip = tips.find(fully_qualified);
								if ( tip != tips.end( ) ) {
									cur_widget->setToolTip(cur_depth,QString::fromStdString(tip->second));
									edit->setToolTip(QString::fromStdString(tip->second));
								}
							}
						}
						prefix = "";
					}
				}
			}
		}

		Preferences::Preferences( QWidget *parent ) : QDialog(parent), sigmap(new QSignalMapper(this)),
			init_fields(5), tip_offset(3), role_offset(1),
			default_offset(2), validate_offset(4),
			rc(viewer::getrc()) {

			setupUi(this);

			// see if the configuration has a multiple of 3 strings...
			if ( (((sizeof(init_state)-1) / sizeof(const char *)) % init_fields) != 0 )
				throw casa::viewer::internal_error("size issue with initialization of preferences");

			init( );

			connect( cancel, SIGNAL(clicked()), this, SLOT(cancel_edits( )) );
			connect( reset, SIGNAL(clicked()), this, SLOT(reset_edits( )) );
			connect( apply, SIGNAL(clicked()), this, SLOT(apply_edits( )) );

		}

		Preferences::~Preferences( ) { }

		void Preferences::showEvent( QShowEvent* ) {
			for ( rcedits_type::iterator it=edits.begin( ); it != edits.end( ); ++it ) {
				std::string val;
				if ( rc.get(it->first, val) ) {
					it->second->setText(QString::fromStdString(val));
				}
			}
		}

		void Preferences::cancel_edits( ) {
			hide( );
			for ( rcedits_type::iterator it=edits.begin( ); it != edits.end( ); ++it ) {
				rcvalues_type::iterator dflt = defaults.find(it->first);
				if (dflt == defaults.end( )) continue;
				it->second->setText(QString::fromStdString(dflt->second));
			}
		}

		void Preferences::reset_edits( ) {
			hide( );
			for ( rcedits_type::iterator it=edits.begin( ); it != edits.end( ); ++it ) {
				rcvalues_type::iterator dflt = defaults.find(it->first);
				if (dflt == defaults.end( )) continue;
				it->second->setText(QString::fromStdString(dflt->second));
				rc.put(it->first,dflt->second);
			}
		}

		void Preferences::apply_edits( ) {
			hide( );
			for ( rcedits_type::iterator it=edits.begin( ); it != edits.end( ); ++it ) {
				rc.put(it->first,it->second->text( ).toStdString( ));
			}
		}

	}

}
