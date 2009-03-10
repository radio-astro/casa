//# QPPlotter.cc: Qwt implementation of generic Plotter class.
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
#ifdef AIPS_HAS_QWT

#include <casaqt/QwtPlotter/QPPlotter.qo.h>

#include <graphics/GenericPlotter/PlotCanvasLayout.h>
#include <casaqt/QwtPlotter/QPCanvas.qo.h>
#include <casaqt/QwtPlotter/QPFactory.h>
#include <casaqt/QwtPlotter/QPPanel.qo.h>

#include <ctime>

namespace casa {

// TODO PlotCanvas: way of providing custom tick labels for axes, print to
//      printer, custom layer
// TODO PlotData: possible indexing bug in PlotRasterDataImpl
// TODO PlotLogger: log messages for function entering, object
//      creation/deletion
// TODO QPCanvas: pixmap cache for stack
// TODO QPCurve: test which is faster: setting the pen multiple times and
//      going through the data once, or setting the pen twice and going through
//      the data twice.  also test which is faster: multiple for loops or
//      single for loop with if statements
// TODO QPPlotter: find way of continuing pen style, reduce number of draws in
//      qwtplottertest
// TODO QPRasterPlot: different origins, fix contour line offset problem,
//      printing to PS
// TODO QPSymbol: character width

///////////////////////////
// QPPLOTTER DEFINITIONS //
///////////////////////////

// Static //

QStringList QPPlotter::GLOBAL_COLORS = QColor::colorNames();

const String QPPlotter::CLASS_NAME = "QPPlotter";

bool QPPlotter::__initColors = QPPlotter::initColors();

bool QPPlotter::initColors() {
    // remove transparent color
    QString trans = "transparent";
    for(int i = 0; i < GLOBAL_COLORS.size(); i++) {
        if(GLOBAL_COLORS[i] == trans) {
            GLOBAL_COLORS.removeAt(i);
            break;
        }
    }
    
    // seed random number generator
    srand(time(NULL));

    return true;
}


// Constructors/Destructors //

QPPlotter::QPPlotter(QPCanvas* canvas, int logEventFlags, QWidget* parent) :
        QWidget(parent), m_layout(NULL), m_emitResize(true),
        m_logEvents(logEventFlags) {
    initialize();
    
    if(canvas != NULL) {    
        m_layout = new PlotLayoutSingle(canvas);
        setupCanvasFrame();
    }
}

QPPlotter::QPPlotter(PlotCanvasLayoutPtr layout, int logEventFlags,
        QWidget* parent) : QWidget(parent), m_layout(layout),
        m_emitResize(true), m_logEvents(logEventFlags) {
    initialize();
    
    if(!m_layout.null()) {
        bool valid = m_layout->isValid();
        
        if(valid) {
            vector<PlotCanvasPtr> canvases = layout->allCanvases();
            QPCanvas* c;
            for(unsigned int i = 0; i < canvases.size(); i++) {
                c = dynamic_cast<QPCanvas*>(canvases[i].operator->());
                if(c == NULL) {
                    valid = false;
                    break;
                }
            }
        }
        
        if(valid) setupCanvasFrame();
    }
}

QPPlotter::~QPPlotter() { }


// Public Methods //

void QPPlotter::showGUI(bool showGUI) {
    setVisible(showGUI);
}

pair<int, int> QPPlotter::size() const {
    QSize s = QWidget::size();
    return pair<int, int>(s.width(), s.height());
}
void QPPlotter::setSize(int width, int height) { resize(width, height); }

String QPPlotter::windowTitle() const {
    return QWidget::windowTitle().toStdString(); }
void QPPlotter::setWindowTitle(const String& newTitle) {
    QWidget::setWindowTitle(newTitle.c_str()); }

pair<int, int> QPPlotter::canvasAreaSize() const {
    return pair<int, int>(canvasFrame->width(), canvasFrame->height()); }

void QPPlotter::setCanvasSize(int width, int height, bool resizeWindow) {
    if(!m_layout.null() && !m_layout->canvas().null()) {
        QPCanvas& canv = dynamic_cast<QPCanvas&>(*m_layout->canvas());
        canv.setFixedSize(width, height);
        
        if(resizeWindow) {
            bool b = m_emitResize;
            m_emitResize = false;        
            resize(minimumSizeHint());
            m_emitResize = b;
        }
    }
}

int QPPlotter::displayDPI() const {
    if(logicalDpiX() < logicalDpiY()) return logicalDpiX();
    else return logicalDpiY();
}

PlotCursor QPPlotter::cursor() const {
    return QPOptions::cursor(QWidget::cursor().shape()); }
void QPPlotter::setCursor(PlotCursor cursor) {
    QWidget::setCursor(QPOptions::cursor(cursor)); }

void QPPlotter::refresh() {     
    if(!m_layout.null()) {
        vector<PlotCanvasPtr> v = m_layout->allCanvases();
        for(unsigned int i = 0; i < v.size(); i++)
            v[i]->refresh();
    }
    
    update();
}

void QPPlotter::close() { QWidget::close(); }


PlotCanvasLayoutPtr QPPlotter::canvasLayout() { return m_layout; }
void QPPlotter::setCanvasLayout(PlotCanvasLayoutPtr layout) {
    if(layout != m_layout) {
        m_canvasTools.clear();
        m_layout = layout;
        setupCanvasFrame();
        
        if(!m_layout.null()) {
            showDefaultPanel(HAND_TOOLS, handBox->isVisible());
            showDefaultPanel(EXPORT_TOOLS, exportBox->isVisible());
        }
    }
}

void QPPlotter::canvasLayoutChanged(PlotCanvasLayout& layout) {
    if(&layout == &(*m_layout)) {
        // check if just the spacing changed
        PlotLayoutGrid* g = dynamic_cast<PlotLayoutGrid*>(&layout);
        if(g != NULL) {
            QGridLayout* qg= dynamic_cast<QGridLayout*>(canvasFrame->layout());
            if(g->spacing() != (unsigned int)qg->spacing() &&
               g->rows() == (unsigned int)qg->rowCount() &&
               g->cols() == (unsigned int)qg->columnCount()) {
                PlotGridCoordinate c(0, 0);
                bool same = true;
                int row = 0, col = 0, tmp = 0;
                for(int i = 0; same && i < qg->count(); i++) {
                    qg->getItemPosition(i, &row, &col, &tmp, &tmp);
                    c.row = row; c.col = col;
                    if(dynamic_cast<QWidget*>(&*g->canvasAt(c)) !=
                       dynamic_cast<QWidget*>(qg->itemAt(i))) same = false;
                }
                if(same) {
                    // the only thing changed was the spacing, so don't call
                    // setupCanvasFrame() which redoes the whole layout.
                    qg->setSpacing(g->spacing());
                    return;
                }
            }
        }
        m_canvasTools.clear();
        setupCanvasFrame();
    }
}


bool QPPlotter::defaultPanelShown(DefaultPanel panel) {
    switch(panel) {
    case HAND_TOOLS: return toolFrame->isVisible() && handBox->isVisible();
    case EXPORT_TOOLS: return toolFrame->isVisible() && exportBox->isVisible();
    
    default: return false;
    }
}

void QPPlotter::showDefaultPanel(DefaultPanel panel, bool show) {
    switch(panel) {
    case HAND_TOOLS:
        handBox->setVisible(show);
        
        if(show) toolFrame->setVisible(true);
        else if(!exportBox->isVisible()) toolFrame->setVisible(false);
        
        if(!m_layout.null()) {        
            PlotStandardMouseToolGroup::Tool tool =
                PlotStandardMouseToolGroup::NONE;        
            if(show) {
                if(handSelectButton->isChecked())
                    tool = PlotStandardMouseToolGroup::SELECT;
                else if(handZoomButton->isChecked())
                    tool = PlotStandardMouseToolGroup::ZOOM;
                else if(handPanButton->isChecked())
                    tool = PlotStandardMouseToolGroup::PAN;
            }

            for(unsigned int i = 0; i < m_canvasTools.size(); i++)
                m_canvasTools[i]->setActiveTool(tool);
        }
        break;
        
    case EXPORT_TOOLS:
        exportBox->setVisible(show);
        
        if(show) toolFrame->setVisible(true);
        else if(!handBox->isVisible()) toolFrame->setVisible(false);
        
        break;
    }
}

int QPPlotter::addPanel(PlotPanelPtr panel) {
    if(panel.null()) return -1;
    for(unsigned int i = 0; i < m_panels.size(); i++)
        if(m_panels[i] == panel) return i;
    
    QPPanel* p = dynamic_cast<QPPanel*>(&*panel);
    if(p == NULL) return -1;
    
    QVBoxLayout* l = dynamic_cast<QVBoxLayout*>(panelFrame->layout());
    l->addWidget(p->asQFrame());
    panelFrame->setVisible(true);
  
    m_panels.push_back(panel);
    return m_panels.size() - 1;
}

vector<PlotPanelPtr> QPPlotter::allPanels() { return m_panels; }
unsigned int QPPlotter::numPanels() { return m_panels.size(); }

PlotPanelPtr QPPlotter::getPanel(int index) {
    if(index < 0 || (unsigned int)index >= m_panels.size())
        return PlotPanelPtr();
    else return m_panels[index];
}

int QPPlotter::panelIndex(PlotPanelPtr panel) {
    if(panel.null()) return -1;
    for(unsigned int i = 0; i < m_panels.size(); i++)
        if(m_panels[i] == panel) return i;
    return -1;
}

void QPPlotter::clearPanels() {
    panelFrame->setVisible(false);
    QVBoxLayout* l = dynamic_cast<QVBoxLayout*>(panelFrame->layout());
    while(l->count() > 0) l->removeItem(l->itemAt(0));
    m_panels.clear();
}

void QPPlotter::removePanel(PlotPanelPtr panel) {
    if(panel.null()) return;
    
    unsigned int i;
    for(i = 0; i < m_panels.size(); i++)
        if(m_panels[i] == panel) break;
    if(i == m_panels.size()) return;
    else removePanel(i);
}

void QPPlotter::removePanel(int i) {
    if(i < 0 || (unsigned int)i >= m_panels.size()) return;

    if(m_panels.size() == 1) panelFrame->setVisible(false);
    QFrame* f = dynamic_cast<QPPanel*>(&*m_panels[i])->asQFrame();
    panelFrame->layout()->removeWidget(f);
    m_panels.erase(m_panels.begin() + i);
}

void QPPlotter::removeLastPanel() { removePanel(m_panels.size() - 1); }


PlotFactory* QPPlotter::implementationFactory() const {
    return new QPFactory(); }

bool QPPlotter::exportToFile(const PlotExportFormat& format) {
    return QPCanvas::exportPlotter(this, format); }

String QPPlotter::fileChooserDialog(const String& title,
        const String& directory) {
    return QFileDialog::getSaveFileName(this, title.c_str(),
            directory.c_str()).toStdString();
}


int QPPlotter::logEventFlags() const { return m_logEvents; }
void QPPlotter::setLogEventFlags(int flags) { m_logEvents = flags; }


void QPPlotter::registerResizeHandler(PlotResizeEventHandlerPtr handler) {
    if(!handler.null()) m_resizeHandlers.push_back(handler); }
vector<PlotResizeEventHandlerPtr> QPPlotter::allResizeHandlers() const {
    return m_resizeHandlers; }

void QPPlotter::unregisterResizeHandler(PlotResizeEventHandlerPtr handler) {
    for(unsigned int i = 0; i < m_resizeHandlers.size(); i++) {
        if(m_resizeHandlers[i] == handler) {
            m_resizeHandlers.erase(m_resizeHandlers.begin() + i);
            break;
        }
    }
}


const QWidget* QPPlotter::canvasWidget() const { return canvasFrame; }
QWidget* QPPlotter::canvasWidget() { return canvasFrame; }

QSize QPPlotter::sizeHint() const { return QSize(); }
QSize QPPlotter::minimumSizeHint() const { return QSize(); }


// Protected Methods //

void QPPlotter::resizeEvent(QResizeEvent* event) {
    if(m_resizeHandlers.size() > 0 && m_emitResize) {
        QSize o = event->oldSize(), n = event->size();
        PlotResizeEvent e(this, o.width(), o.height(), n.width(), n.height());
        for(unsigned int i = 0; i < m_resizeHandlers.size(); i++)
            m_resizeHandlers[i]->handleResize(e);
        event->accept();
    } else event->ignore();
}


// Private Methods //

void QPPlotter::setupCanvasFrame() {
    if(m_layout.null()) {
        if(canvasFrame->layout() != NULL) delete canvasFrame->layout();
        return;
    }
    
    // remove the previous Qt layout, but do NOT delete the canvas children
    // as this will be taken care of by our smart pointers (hopefully)
    if(canvasFrame->layout() != NULL) {
        while((canvasFrame->layout()->takeAt(0)) != 0);
        delete canvasFrame->layout();
    }
    
    m_layout->attach(this);
    
    // add canvases to frame
    PlotLayoutSingle* l = dynamic_cast<PlotLayoutSingle*>(&(*m_layout));
    PlotLayoutGrid* g = dynamic_cast<PlotLayoutGrid*>(&(*m_layout));
    if(l != NULL) {
        // single layout
        QHBoxLayout* hbl = new QHBoxLayout(canvasFrame);
#if QT_VERSION >= 0x040300
        hbl->setContentsMargins(0, 0, 0, 0);
#else
        hbl->setMargin(0);
#endif
        hbl->addWidget(dynamic_cast<QPCanvas*>(l->canvas().operator->()));
    } else if(g != NULL) {
        // grid layout
        QGridLayout* qgl = new QGridLayout(canvasFrame);
#if QT_VERSION >= 0x040300
        qgl->setContentsMargins(0, 0, 0, 0);
#else
        qgl->setMargin(0);
#endif
        qgl->setSpacing(g->spacing());
        
        PlotGridCoordinate coord(0, 0);
        QPCanvas* c;
        for(unsigned int i = 0; i < g->rows(); i++) {
            coord.row = i;
            for(unsigned int j = 0; j < g->cols(); j++) {
                coord.col = j;
                c = dynamic_cast<QPCanvas*>(g->canvasAt(coord).operator->());
                qgl->addWidget(c, i, j);
            }
        }
    } else {
        // error?
    }
    
    // Update standard mouse tools.
    vector<PlotCanvasPtr> canvases = m_layout->allCanvases();
    m_canvasTools.resize(canvases.size());
    for(unsigned int i = 0; i < canvases.size(); i++) {
        m_canvasTools[i] = canvases[i]->standardMouseTools();
        m_canvasTools[i]->trackerTool()->setDrawText(true);
        m_canvasTools[i]->setActive(true);
        m_canvasTools[i]->setBlocking(true);
    }
    
    // Set the parent of the canvases to this plotter.
    for(unsigned int i = 0; i < canvases.size(); i++)
        dynamic_cast<QPCanvas&>(*canvases[i]).setQPPlotter(this);
}

void QPPlotter::initialize() {
    setupUi(this);
    
    toolFrame->setVisible(false);
    handBox->setVisible(false);
    exportBox->setVisible(false);
    
    setVisible(false);
    
    // Add legend positions to chooser
    Vector<String> p = PlotCanvas::allLegendPositionStrings();
    for(unsigned int i = 0; i < p.size(); i++)
        legendChooser->addItem(p[i].c_str());
    
    // Add formats to chooser
    vector<String> f = PlotExportFormat::supportedFormatStrings();
    for(unsigned int i = 0; i < f.size(); i++)
        exportFormat->addItem(f[i].c_str());
    
    // Set up panel frame
    panelFrame->setVisible(false);
    QVBoxLayout* l = new QVBoxLayout(panelFrame);
#if QT_VERSION >= 0x040300
    l->setContentsMargins(0, 0, 0, 0);
#else
    l->setMargin(0);
#endif
    l->setSpacing(3);
    
    // Connect widgets.
    connect(handZoomButton, SIGNAL(toggled(bool)),
            this, SLOT(handToolChanged(bool)));
    connect(handSelectButton, SIGNAL(toggled(bool)),
            this, SLOT(handToolChanged(bool)));
    connect(handPanButton, SIGNAL(toggled(bool)),
            this, SLOT(handToolChanged(bool)));
    connect(handNoneButton, SIGNAL(toggled(bool)),
            this, SLOT(handToolChanged(bool)));
    connect(trackerBox, SIGNAL(toggled(bool)),
            this, SLOT(trackerTurned(bool)));
    connect(legendBox, SIGNAL(toggled(bool)),
            this, SLOT(legendTurned(bool)));
    connect(legendChooser, SIGNAL(currentIndexChanged(int)),
            this, SLOT(legendPositionChanged()));
    connect(exportButton, SIGNAL(clicked()), this, SLOT(exportCanvases()));
}


// Private Slots //

void QPPlotter::handToolChanged(bool on) {
    if(!on || m_layout.null()) return;
    
    PlotStandardMouseToolGroup::Tool tool = PlotStandardMouseToolGroup::NONE;
    if(handZoomButton->isChecked()) tool = PlotStandardMouseToolGroup::ZOOM;
    else if(handSelectButton->isChecked())
        tool = PlotStandardMouseToolGroup::SELECT;
    else if(handPanButton->isChecked()) tool = PlotStandardMouseToolGroup::PAN;
    
    for(unsigned int i = 0; i < m_canvasTools.size(); i++)
        m_canvasTools[i]->setActiveTool(tool);
}

void QPPlotter::trackerTurned(bool on) {
    if(m_layout.null()) return;
    
    vector<PlotCanvasPtr> canvases = m_layout->allCanvases();
    for(unsigned int i = 0; i < canvases.size(); i++)
        canvases[i]->standardMouseTools()->turnTracker(on);
}

void QPPlotter::legendTurned(bool on) {
    if(m_layout.null()) return;
    
    PlotCanvas::LegendPosition a = PlotCanvas::legendPosition(
            legendChooser->currentText().toStdString());
    vector<PlotCanvasPtr> canvases = m_layout->allCanvases();
    for(unsigned int i = 0; i < canvases.size(); i++)
        canvases[i]->showLegend(on, a);
}

void QPPlotter::exportCanvases() {
    PlotExportFormat format(PlotExportFormat::exportFormat(
            exportFormat->currentText().toStdString()), "");
    format.resolution = (exportScreen->isChecked())? PlotExportFormat::SCREEN :
                                                     PlotExportFormat::HIGH;
    if(exportDPIbox->isChecked()) format.dpi = exportDPI->value();
    if(exportSizeBox->isChecked()) {
        format.width = exportWidth->value();
        format.height = exportHeight->value();
    }
    
    QString title = "Export to ";
    title += exportFormat->currentText();
    title += " file.";
    QString filename = QFileDialog::getSaveFileName(this, title);
    if(filename.isEmpty()) return;
    format.location = filename.toStdString();
    
    exportToFile(format);
}

}

#endif
