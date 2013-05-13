//# Preferences.qo.h: dialog which allows user to set and reset viewer preferences
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

#ifndef QTVIEWER_PREFERENCES_H_
#define QTVIEWER_PREFERENCES_H_

#include <set>
#include <string>
#include <QSignalMapper>
#include <QtGui/QDialog>
#include <display/QtViewer/Preferences.ui.h>
#include <display/Utilities/Lowlevel.h>

namespace casa {
	namespace viewer {

		class Preferences : public QDialog, protected Ui::Preferences {
			Q_OBJECT
		public:
			Preferences( QWidget *parent=0 );
			~Preferences( );

		protected slots:
			void reset_edits( );
			void cancel_edits( );
			void apply_edits( );

		protected:
			void showEvent( QShowEvent* );

		private:
			typedef std::map<std::string,std::vector<std::string> > tokenized_type;
			typedef std::map<std::string,std::set<std::string> > branch_count_type;

			void init( );

			// returns the number of configuration options...
			int tokenize( tokenized_type & );
			// returns the number of root nodes...
			int countbranches( const tokenized_type &, branch_count_type & );
			// calculate tree depth...
			int find_tree_depth( bool, const tokenized_type &, const branch_count_type & );
			static const char *init_state[];

			QSignalMapper *sigmap;
			typedef std::map<std::string,QValidator*> validate_map_type;
			validate_map_type validate_map;

			typedef std::map<std::string,std::string> rcvalues_type;
			typedef std::map<std::string,std::string> tooltips_type;
			typedef std::map<std::string,QLineEdit*> rcedits_type;
			// tool tips for leaves & nodes...
			tooltips_type tips;
			// initial viewer default values...
			rcvalues_type defaults;
			// rc values to edit boxes...
			rcedits_type edits;

			const int init_fields;
			const int tip_offset;
			const int role_offset;
			const int default_offset;
			const int validate_offset;

			// connection to rc file
			Casarc &rc;
		};

	}
}

#endif

