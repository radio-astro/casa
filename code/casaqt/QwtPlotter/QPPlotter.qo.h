//# QPPlotter.qo.h: Qwt implementation of generic Plotter class.
//# Copyright (C) 2008
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
#ifndef QPPLOTTER_QO_H_
#define QPPLOTTER_QO_H_

#ifdef AIPS_HAS_QWT

#include <graphics/GenericPlotter/Plotter.h>
#include <casaqt/QwtPlotter/QPPlotter.ui.h>

#include <QtGui>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class QPExportCanvas;
class QPCanvas;
class QPAxis;
class PlotFactory;

// Implementation of Plotter for Qwt plotter.  A QWidget that can be used as a
// main window.
class QPPlotter : public QWidget, Ui::PlotterUI, public Plotter {
    Q_OBJECT
    
    friend class QPCanvas;
    friend class QPAxis;
    
public:
    // Static //

    // Provides access to a global list of Qt colors.
    static QStringList GLOBAL_COLORS;
    
    // Convenient access to class name (QPPlotter).
    static const String CLASS_NAME;

    
    // Non-Static //
    
    // Constructor that creates a plotter with a single canvas (or none for
    // NULL).  Uses the given log event flags (see PlotLogger::Event) if given,
    // and the parent QWidget if given.
    QPPlotter(QPCanvas* canvas = NULL,
              int logEventFlags = PlotLogger::NO_EVENTS,
              QWidget* parent = NULL);
    
    // Constructor that creates a plotter with the given layout (or none for
    // null).  Uses the given log event flags (see PlotLogger::Event) if given,
    // and the parent QWidget if given.
    QPPlotter(PlotCanvasLayoutPtr layout,
              int logEventFlags = PlotLogger::NO_EVENTS,
              QWidget* parent = NULL);
    
    // Destructor.
    ~QPPlotter();
    
    

    // Plotter Methods //
   
    // Implements Plotter::showGUI().
    void showGUI(bool showGUI = true);
    
    //Kludge for updating the plot when we are in Non-GUI mode.
    void updateScriptGui();
    
    // Implements Plotter::size().
    pair<int, int> size() const;
    
    // Implements Plotter::setSize().
    void setSize(int width, int height);
    
    // Implements Plotter::windowTitle().
    String windowTitle() const;
    
    // Implements Plotter::setWindowTitle().
    void setWindowTitle(const String& newTitle);
    
    // Implements Plotter::canvasAreaSize().
    pair<int, int> canvasAreaSize() const;
        
    // Implements Plotter::setCanvasSize().
    void setCanvasSize(int width, int height, bool resizeWindow = true);
    
    // Implements Plotter::displayDPI().
    int displayDPI() const;

    // Implements Plotter::isQWidget().
    bool isQWidget() const { return true; }
    
    // Implements Plotter::cursor().
    PlotCursor cursor() const;
    
    // Implements Plotter::setCursor().
    void setCursor(PlotCursor cursor);
    
    // Implements Plotter::refresh().
    void refresh();
    
    // Implements Plotter::close().
    void close();
    
    vector<QPExportCanvas*> getGridComponents();
    
    // Implements Plotter::canvasLayout().
    PlotCanvasLayoutPtr canvasLayout();
    
    // Implements Plotter::setCanvasLayout().
    void setCanvasLayout(PlotCanvasLayoutPtr layout);
    
    // Implements Plotter::canvasLayoutChanged().
    void canvasLayoutChanged(PlotCanvasLayout& layout);
    
    // Implements Plotter::dateFormat().
    const String& dateFormat() const;
    
    // Implements Plotter::setDateFormat().
    void setDateFormat(const String& dateFormat);
    
    // Implements Plotter::relativeDateFormat().
    const String& relativeDateFormat() const;
    
    // Implements Plotter::setRelativeDateFormat().
    void setRelativeDateFormat(const String& dateFormat);
    
    
    // Implements Plotter::defaultPanelShown().
    bool defaultPanelShown(DefaultPanel panel);
    
    // Implements Plotter::showDefaultPanel().
    void showDefaultPanel(DefaultPanel panel, bool show = true);
    
    // Implements Plotter::addPanel().
    int addPanel(PlotPanelPtr panel);
    
    // Implements Plotter::allPanels().
    vector<PlotPanelPtr> allPanels();
    
    // Implements Plotter::numPanels().
    unsigned int numPanels();
    
    // Implements Plotter::getPanel().
    PlotPanelPtr getPanel(int index);
    
    // Implements Plotter::panelIndex().
    int panelIndex(PlotPanelPtr panel);
    
    // Implements Plotter::clearPanels().
    void clearPanels();
    
    // Implements Plotter::removePanel().
    void removePanel(PlotPanelPtr annotation);
    
    // Implements Plotter::removePanel().
    void removePanel(int id);
    
    // Implements Plotter::removeLastPanel().
    void removeLastPanel();
    
    
    // Implements Plotter::implementation().
    Implementation implementation() const { return QWT; }
    
    // Implements Plotter::implementationFactory().
    PlotFactory* implementationFactory() const;
    
    // Implements Plotter::exportToFile().
    bool exportToFile(const PlotExportFormat& format);
    
    // Implements Plotter::fileChooserDialog().
    String fileChooserDialog(const String& title = "File Chooser",
                             const String& directory = "");

    
    // Implements Plotter::registerResizeHandler().
    void registerResizeHandler(PlotResizeEventHandlerPtr handler);
    
    // Implements Plotter::allResizeHandlers().
    vector<PlotResizeEventHandlerPtr> allResizeHandlers() const;
    
    // Implements Plotter::unregisterResizeHandlers().
    void unregisterResizeHandler(PlotResizeEventHandlerPtr handler);
    
    
    // Returns the frame used to hold the canvases.
    // <group>
    const QWidget* canvasWidget() const;
    QWidget* canvasWidget();
    // </group>
    
    // Overrides QWidget::sizeHint() to return an invalid size.
    QSize sizeHint() const;
    
    // Overrides QWidget::minimumSizeHint() to return an invalid size.
    QSize minimumSizeHint() const;
    
    virtual bool exportPlot(const PlotExportFormat& format );

	//Return the number of rows and columns in the current grid.
    int getRowCount();
    int getColCount();
protected:
    // For catching resize events.
    void resizeEvent(QResizeEvent* event);
    
    // Logs the given object creation/destruction event, if needed.
    void logObject(const String& className, void* address, bool creation,
            const String& message = String());
    
    // Logs the given method enter/exit event, if needed.
    void logMethod(const String& className, const String& methodName,
            bool entering, const String& message = String());
    
private:
    // Canvas layout.
    PlotCanvasLayoutPtr m_layout;
    
    // Standard tools for canvases.
    vector<PlotStandardMouseToolGroupPtr> m_canvasTools;
    
    // Registered handlers.
    vector<PlotResizeEventHandlerPtr> m_resizeHandlers;
    
    // Panels.
    vector<PlotPanelPtr> m_panels;
    
    // Flag for whether a resize event should be emitted.
    bool m_emitResize;
    
    // Date formats.
    // <group>
    String m_dateFormat;
    String m_relativeDateFormat;
    // </group>
    
    QList<QPAxis*> externalAxes;
    
    // Sets up the canvas QFrame for the current layout.
    void setupCanvasFrame();
    
    // Initializes GUI (to be called from constructors).
    void initialize();
    
    void clearExternalAxes();
    void emptyLayout();

    // Static //
    
    // Used to initialize GLOBAL_COLORS.
    // <group>
    static bool __initColors;    
    static bool initColors();
    // </group>
    
    //Returns true if the given axis is drawn by the Qwt plot; false if
    //the drawing is external and custom.
    // <group>
    bool isLeftAxisInternal() const;
    bool isBottomAxisInternal() const;
    bool isRightAxisInternal() const;
    // </group>
private slots:
    // Default panel: hand tool changed.
    void handToolChanged(bool on);
    
    // Default panel: tracker toggled.
    void trackerTurned(bool on);
    
    // Default panel: legend toggled.
    void legendTurned(bool on);
    
    // Default panel: legend position changed.
    void legendPositionChanged() { legendTurned(legendBox->isChecked()); }
    
    // Default panel: export button pushed.
    void exportCanvases();
};

}

#endif

#endif /*QPPLOTTER_QO_H_*/
