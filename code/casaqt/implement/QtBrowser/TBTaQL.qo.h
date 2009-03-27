//# TBTaQL.qo.h: GUI for entering a TaQL command.
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
//# $Id: $
#ifndef TBTAQL_H_
#define TBTAQL_H_

#include <casaqt/QtBrowser/TBTaQL.ui.h>

#include <QtGui>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

// <summary>
// GUI for entering a TaQL command.
// </summary>
//
// <synopsis>
// A TBTaQL widget contains two basic parts: the builder and the command text
// edit.  The builder contains GUI components that allow the user to visually
// put together a TaQL command; once the command is built, the actual comamnd
// can be generated.  The command can also be manually edited.  When the
// command is accepted, a signal is emitted with the command in String format;
// it is the parent/caller's responsibility to collect the command.
// NOTE: at this time, only the builder for the SELECT command has been
// implemented (and not a very advanced implementation, at that).
// </synopsis>

class TBTaQL : public QDialog, Ui::TaQL {
    Q_OBJECT

public:
    // Available commands.  Makes use of the QFlags for easy combining of
    // multiple Command enums into one Command enum.
    // <group>
    enum Command {
        SELECT = 0x1,
        UPDATE = 0x2,
        INSERT = 0x4,
        DELETE = 0x8,
        CALC = 0x16,
        CREATE = 0x32
    };
    Q_DECLARE_FLAGS(Commands, Command);
    // </group>
    
    // Returns the String representation of the given command.
    static String command(Command c) {
        switch(c) {
        case SELECT: return "SELECT";
        case UPDATE: return "UPDATE";
        case INSERT: return "INSERT";
        case DELETE: return "DELETE";
        case CALC: return "CALC";
        case CREATE: return "CREATE";
        default: return "";
        }
    }
    
    // Constructor which optionally takes the parent and the commands to show
    // in the builder command chooser.  If parent is NULL, the TBTaQL is
    // presented as a QDialog.
    TBTaQL(QWidget* parent = NULL, Commands commands = SELECT);
    
    ~TBTaQL();
    
signals:
    // This signal is emitted whenever the user has accepted the entered TaQL
    // command.
    void command(String command);
    
    // This signal is emitted whenever the user presses the "Close" button.
    // NOTE: if this TBTaQL is in dialog mode, it will close itself; if it is
    // in widget mode, the parent is responsible for closing it.  This signal
    // is not emitted if in dialog mode.
    void closeRequested();
    
private slots:
    // Slot for the "browse" button.  Opens a file browser and puts the result
    // in the location text edit.
    void doBrowse();

    // Slot for the "generate" button.  Generates the TaQL command from the
    // state of the builder widgets.
    void doGenerate();
    
    // Slot for the "accept" button.  Emits the command() signal and, if the
    // TBTaQL is in dialog mode, closes the dialog.
    void doAccept();
    
    // Slot for the "close" button.  If the TBTaQL is in dialog mode, closes
    // the dialog; otherwise, emits the closeRequested() signal.
    void doClose();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TBTaQL::Commands)

}

#endif /*TBTAQL_H_*/
