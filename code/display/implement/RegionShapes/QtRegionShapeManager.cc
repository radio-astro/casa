//# QtRegionShapeManager.cc: Classes for managing/loading region shapes.
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
//# $Id$
#include <display/RegionShapes/QtRegionShapeManager.qo.h>
#include <display/RegionShapes/QtSingleRegionShape.qo.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>
#include <display/RegionShapes/RSFileReaderWriter.h>
#include <display/RegionShapes/RegionShapes.h>
#include <display/RegionShapes/QtNewRegionShape.qo.h>
#include <display/RegionShapes/RSUtils.qo.h>
#include <display/RegionShapes/XMLFileReaderWriter.h>

namespace casa {

// QTREGIONSHAPEMANAGER DEFINITIONS //

const String QtRegionShapeManager::DRAWERRORMSG = "Errors were reported during"
                                                  " drawing.";

const QString QtRegionShapeManager::HIDDEN         = "hidden";
const QString QtRegionShapeManager::LAST_DIRECTORY = "last_directory";
const QString QtRegionShapeManager::LAST_FILE      = "last_file";
const QString QtRegionShapeManager::LAST_FORMAT    = "last_format";
const QString QtRegionShapeManager::WINDOW_VISIBLE = "window_visible";

QtRegionShapeManager::QtRegionShapeManager(QtDisplayPanel* panel) :
        QWidget(NULL), m_panel(panel) {
    setupUi(this);
    scrollFrame->setFrameStyle(QFrame::NoFrame);
    scrollFrame->setWidget(frame);
    scrollFrame->setWidgetResizable(true);
    QVBoxLayout* l = RSUtils::vlayout(frame);
    l->addStretch();

    connect(showHideBox, SIGNAL(clicked(bool)), SLOT(showHideAll(bool)));
    connect(loadButton, SIGNAL(clicked()), SLOT(load()));
    connect(saveButton, SIGNAL(clicked()), SLOT(save()));
    connect(newButton, SIGNAL(clicked()), SLOT(newShape()));
    connect(deleteAllButton, SIGNAL(clicked()), SLOT(deleteAll()));
    connect(dismissButton, SIGNAL(clicked()), SLOT(dismiss()));
}

QtRegionShapeManager::~QtRegionShapeManager() { }


QtDisplayPanel* QtRegionShapeManager::panel() const { return m_panel; }

void QtRegionShapeManager::saveState(QDomDocument& document) {
    vector<RegionShape*> v = shapes(true);
    QDomElement elem = XMLFileReaderWriter::shapesToDOM(document, v);
    QString hidden; QTextStream ss(&hidden);
    for(unsigned int i = 0; i < v.size(); i++) {
        bool found = false;
        for(unsigned int j = 0; j < m_shapes.size(); j++) {
            if(v[i] == m_shapes[j]->shape()) {
                ss << (m_shapes[j]->isShown() ? "0" : "1");
                found = true;
                break;
            }
        }
        if(!found) ss << "1";
        if(i < v.size() - 1) ss << ' ';
    }
    ss.flush();
    elem.setAttribute(HIDDEN,         hidden);
    elem.setAttribute(LAST_DIRECTORY, m_lastDirectory.c_str());
    elem.setAttribute(LAST_FILE,      m_lastFile.c_str());
    elem.setAttribute(LAST_FORMAT,    m_lastFormat.c_str());
    elem.setAttribute(WINDOW_VISIBLE, isVisible() ? "1" : "0");
    document.documentElement().appendChild(elem);
}

void QtRegionShapeManager::restoreState(QDomDocument& elem) {
    deleteAll();
    QDomElement docElem = elem.documentElement();
    if(docElem.isNull()) return;
    QDomElement shapes = docElem.firstChildElement(
                         XMLFileReaderWriter::SHAPE_OPTIONS);
    if(shapes.isNull()) return;
    vector<RegionShape*> v = XMLFileReaderWriter::DOMToShapes(shapes);
    addShapes(v);
    
    if(shapes.hasAttribute(HIDDEN)) {
        QStringList hidden = shapes.attribute(HIDDEN).split(QRegExp("\\s+"));
        for(int i = 0; i < (int)v.size() && i < hidden.size(); i++) {
            bool h = hidden[i] != "0" && hidden[i].toLower() != "f" &&
                     hidden[i].toLower() != "false";
            for(unsigned int j = 0; j < m_shapes.size(); j++) {
                if(v[i] == m_shapes[j]->shape()) {
                    m_shapes[j]->showHide(!h, false);
                    break;
                }
            }
        }
    }
    
    // if attribute isn't given, it will return blank
    m_lastDirectory = shapes.attribute(LAST_DIRECTORY).toStdString();
    m_lastFile =      shapes.attribute(LAST_FILE).toStdString();
    m_lastFormat =    shapes.attribute(LAST_FORMAT).toStdString();
    
    if(shapes.hasAttribute(WINDOW_VISIBLE)) {
        QString str = shapes.attribute(WINDOW_VISIBLE).trimmed().toLower();
        setVisible(str != "0" && str != "f" && str != "false");
    }
}

unsigned int QtRegionShapeManager::numShapes(bool includeComposites) const {
    unsigned int count = m_shapes.size();
    for(unsigned int i = 0; i < m_shapes.size(); i++) {
        if(m_shapes[i]->compositeParent() != NULL) count--;
        if(!includeComposites && dynamic_cast<RSComposite*>(
           m_shapes[i]->shape()) != NULL) count--;
    }
    return count;
}

vector<RegionShape*> QtRegionShapeManager::shapes(bool incComp) const {
    vector<RegionShape*> v;
    for(unsigned int i = 0; i < m_shapes.size(); i++) {
        if(m_shapes[i]->compositeParent() == NULL && (incComp ||
           dynamic_cast<RSComposite*>(m_shapes[i]->shape()) == NULL)) {
            v.push_back(m_shapes[i]->shape());
        }
    }
    return v;
}

QtSingleRegionShape* QtRegionShapeManager::shapeWidget(RegionShape* s) const {
    if(s == NULL) return NULL;
    for(unsigned int i = 0; i < m_shapes.size(); i++)
        if(m_shapes[i]->shape() == s) return m_shapes[i];
    return NULL;
}

void QtRegionShapeManager::showSimpleError(const String& msg, bool w) const {
    QString title = "Region Shape Error", text(msg.c_str());
    QWidget* t = const_cast<QtRegionShapeManager*>(this);
    if(w) QMessageBox::warning(t, title, text);
    else  QMessageBox::critical(t, title, text);
}

void QtRegionShapeManager::showDetailedError(const String& message,
        const String& details, bool warn) const {
    QString title = "Region Shape Error", text(message.c_str()),
            dtext(details.c_str());
    QMessageBox::Icon icon = warn? QMessageBox::Warning: QMessageBox::Critical;
    QWidget* t = const_cast<QtRegionShapeManager*>(this);
    QMessageBox msg(icon, title, text, QMessageBox::Ok, t);
    msg.setDetailedText(dtext);
    msg.exec();
}


void QtRegionShapeManager::addShape(RegionShape* shape) {
    addShape(shape, NULL);
}

void QtRegionShapeManager::addShapes(const vector<RegionShape*>& shapes) {
    vector<RegionShape*> parents(shapes.size(), NULL);
    addShapes(shapes, parents);
}

void QtRegionShapeManager::removeShape(RegionShape* shape, bool delShape) {
    if(shape == NULL) return;
    unsigned int i = 0;
    for(; i < m_shapes.size(); i++)
        if(m_shapes[i]->shape() == shape) break;
    if(i == m_shapes.size()) return;
    
    QtSingleRegionShape* r;
    for(unsigned int j = i + 1; j < m_shapes.size(); j++) {
        // delete composite children
        if(m_shapes[j]->compositeParent() == shape) {
            r = m_shapes[j];
            m_shapes.erase(m_shapes.begin() + j);
            ((QVBoxLayout*)frame->layout())->removeWidget(r);
            delete r;
            j--;
        } else break;
    }
    r = m_shapes[i];
    m_shapes.erase(m_shapes.begin() + i);
    ((QVBoxLayout*)frame->layout())->removeWidget(r);
    r->setShouldDelete(delShape);
    delete r;
    if(m_shapes.size() == 0) {
        showHideBox->setEnabled(false);
        saveButton->setEnabled(false);
        deleteAllButton->setEnabled(false);
    }
}

void QtRegionShapeManager::deleteAll() {
    // clear widgets
    QVBoxLayout *l = dynamic_cast<QVBoxLayout*>(frame->layout( ));
    while(l->count() > 1) l->removeItem(l->itemAt(0));

    // delete shapes
    m_panel->hold();
    for(unsigned int i = 0; i < m_shapes.size(); i++)
        delete m_shapes[i];
    m_panel->release();
    m_shapes.clear();
    
    showHideBox->setEnabled(false);
    saveButton->setEnabled(false);
    deleteAllButton->setEnabled(false);
}


void QtRegionShapeManager::addShape(RegionShape* shape, RegionShape* comp) {
    if(shape == NULL) return;
    QtSingleRegionShape* r = new QtSingleRegionShape(shape, this, false, comp);
    m_shapes.push_back(r);
    ((QVBoxLayout*)frame->layout())->insertWidget(m_shapes.size() - 1, r);
    if(comp == NULL && shape->hadDrawingError() &&
       !shape->lastDrawingError().empty()) {
        showDetailedError(DRAWERRORMSG, shape->lastDrawingError());
    }
    
    // if composite with independent children, add them too
    RSComposite* c = dynamic_cast<RSComposite*>(shape);
    if(comp == NULL && c != NULL && !c->childrenAreDependent()) {
        vector<RegionShape*> children = c->children(),
                         parent = vector<RegionShape*>(children.size(), shape);
        addShapes(children, parent);
    }
    
    showHideBox->setEnabled(true);
    saveButton->setEnabled(true);
    deleteAllButton->setEnabled(true);
}

void QtRegionShapeManager::addShapes(const vector<RegionShape*>& shapes,
        const vector<RegionShape*>& parents) {
    vector<RegionShape*> compositeParents(parents);
    if(compositeParents.size() != shapes.size())
        compositeParents.resize(shapes.size(), NULL);
    RSComposite* c; vector<RegionShape*> children, parent;
    m_panel->hold();
    for(unsigned int i = 0; i < shapes.size(); i++) {
        if(shapes[i] == NULL) continue;
        QtSingleRegionShape* r = new QtSingleRegionShape(shapes[i], this,
                false, compositeParents[i]);
        m_shapes.push_back(r);
        ((QVBoxLayout*)frame->layout())->insertWidget(m_shapes.size() - 1, r);
        c = dynamic_cast<RSComposite*>(shapes[i]);
        if(compositeParents[i]==NULL && c!=NULL && !c->childrenAreDependent()){
            children = c->children();
            parent = vector<RegionShape*>(children.size(), shapes[i]);
            addShapes(children, parent);
        }
    }
    m_panel->release();
    
    // check for errors
    m_panel->hold();
    stringstream ss;
    for(unsigned int i = 0; i < shapes.size(); i++) {
        if(compositeParents[i] == NULL && shapes[i]->hadDrawingError()) {
            RSUtils::appendUniqueMessage(ss, shapes[i]->lastDrawingError());
            m_shapes[m_shapes.size() - shapes.size() + i]->showHide(false);
        }
    }
    m_panel->release();
    
    QCoreApplication::processEvents();
    if(ss.str().size() > 0)
        showDetailedError(DRAWERRORMSG, ss.str());
    showHideBox->setEnabled(true);
    saveButton->setEnabled(true);
    deleteAllButton->setEnabled(true);
}

void QtRegionShapeManager::showHideAll(bool show) {
    m_panel->hold();
    for(unsigned int i = 0; i < m_shapes.size(); i++)
        m_shapes[i]->showHide(show, false);
    m_panel->release();
    if(show) {
        stringstream ss;
        for(unsigned int i = 0; i < m_shapes.size(); i++) {
            if(m_shapes[i]->shape()->hadDrawingError() &&
               !m_shapes[i]->shape()->lastDrawingError().empty()) {
                if(ss.str().size() > 0) ss << "\n";
                ss << m_shapes[i]->shape()->lastDrawingError();
            }
        }
        if(ss.str().size() > 0)
            showDetailedError(DRAWERRORMSG,ss.str());
    }
}

void QtRegionShapeManager::load() {
    RSFileReader* reader = QtRSFileLoader::getFileReader(&m_lastFile,
                           &m_lastFormat, &m_lastDirectory);
    if(reader == NULL) return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    vector<RegionShape*> shapes;
    bool result = reader->read(shapes);
    if(!result && reader->lastError().isFatal())
        showDetailedError("One or more fatal errors occurred during file "
                          "loading.", reader->lastError().error(), false);
    else {
        if(!result) showDetailedError("One or more warnings occurred during "
                                "file loading.", reader->lastError().error());
        addShapes(shapes);
    }

    delete reader;
    QApplication::restoreOverrideCursor();
}

void QtRegionShapeManager::save() {
    RSFileWriter* writer = QtRSFileSaver::getFileWriter(&m_lastFile,
                           &m_lastFormat, &m_lastDirectory);
    if(writer == NULL) return;
    
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    
    vector<RegionShape*> shapes;
    for(unsigned int i = 0; i < m_shapes.size(); i++)
        if(m_shapes[i]->compositeParent() == NULL)
            shapes.push_back(m_shapes[i]->shape());
    
    bool result = writer->write(shapes);
    if(!result) {
        if(writer->lastError().isFatal())
            showDetailedError("One or more fatal errors occurred during file "
                              "writing.", writer->lastError().error(), false);
        else
            showDetailedError("One or more warnings occurred during file "
                              "writing.", writer->lastError().error(), true);
    }
    
    delete writer;
    QApplication::restoreOverrideCursor();
}

void QtRegionShapeManager::newShape() {
    QtNewRegionShape* nrs = new QtNewRegionShape(this, true, true);
    nrs->setVisible(true);
    setEnabled(false);
    connect(nrs, SIGNAL(shapeCreated(RegionShape*)),
            SLOT(addShape(RegionShape*)));
    connect(nrs, SIGNAL(shapeCreated(RegionShape*)), nrs, SLOT(close()));
    connect(nrs, SIGNAL(destroyed()), SLOT(enable()));
}

void QtRegionShapeManager::dismiss() {
    setVisible(false);
}


// QTRSFILELOADER DEFINITIONS //

QtRSFileLoader::QtRSFileLoader(String file, String format, String dir) :
        QDialog(NULL), m_lastDir(dir.c_str()) {
    setupUi(this);
    
    // set up format chooser
    vector<String> formats = RSFileReaderWriter::supportedTypesStrings();
    for(unsigned int i = 0; i < formats.size(); i++)
        formatChooser->addItem(formats[i].c_str());

    fileLocation->setText(file.c_str());
    if(!format.empty()) {
        for(int i = 0; i < formatChooser->count(); i++) {
            if(formatChooser->itemText(i) == format.c_str()) {
                formatChooser->setCurrentIndex(i);
                break;
            }
        }
    }
    
    connect(browseButton, SIGNAL(clicked()), SLOT(browse()));
    connect(buttonBox, SIGNAL(accepted()), SLOT(ok()));
    connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
}

QtRSFileLoader::~QtRSFileLoader() { }


String QtRSFileLoader::getFilename() {
    return QFileInfo(fileLocation->text()).absoluteFilePath().toStdString();
}

String QtRSFileLoader::getDirectory() {
    return QFileInfo(fileLocation->text()).absoluteDir().path().toStdString();
}

String QtRSFileLoader::getFormat() {
    return formatChooser->currentText().toStdString();
}

void QtRSFileLoader::browse() {
    QString format = formatChooser->currentText();
    QString ext(RSFileReaderWriter::extensionForType(
            RSFileReaderWriter::supportedType(format.toStdString())).c_str());
    QString file = QFileDialog::getOpenFileName(this, "Open "+format+" file",
            m_lastDir, format + " Regions (*." + ext + ")");
    if(!file.isEmpty()) fileLocation->setText(file);
}

void QtRSFileLoader::ok() {
    QFile file(fileLocation->text());
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        QMessageBox::critical(this, "Load Error", "File "+fileLocation->text()+
                              " could not be opened for reading!");
    else accept();
}


RSFileReader* QtRSFileLoader::getFileReader(String* file, String* format,
        String* directory) {
    String fi = (file != NULL) ? *file : "",
           fo = (format != NULL) ? *format : "",
           di = (directory != NULL) ? *directory : "";
    QtRSFileLoader loader(fi, fo, di);
    int ret = loader.exec();
    if(ret == QDialog::Rejected) return NULL;

    fi = loader.getFilename();
    fo = loader.getFormat();
    di = loader.getDirectory();
    if(file != NULL) *file = fi;
    if(format != NULL) *format = fo;
    if(directory != NULL) *directory = di;
    
    RSFileReaderWriter::SupportedType type =
        RSFileReaderWriter::supportedType(fo);
    RSFileReader* reader = RSFileReaderWriter::readerForType(type);
    if(reader != NULL) reader->setFile(fi);
    return reader;
}


// QTRSFILESAVER DEFINITIONS //

QtRSFileSaver::QtRSFileSaver(String file, String format, String dir) :
        QDialog(NULL), m_lastDir(dir.c_str()) {
    setupUi(this);
    
    // set up format chooser
    vector<String> formats = RSFileReaderWriter::supportedTypesStrings();
    for(unsigned int i = 0; i < formats.size(); i++)
        formatChooser->addItem(formats[i].c_str());
    
    fileLocation->setText(file.c_str());
    
    m_formatOptions = new QStackedLayout(optionsFrame);
    RSUtils::setupLayout(m_formatOptions);
    
    QWidget* w;
    for(unsigned int i = 0; i < formats.size(); i++) {
        w = RSFileReaderWriter::optionsWidgetForType(
                RSFileReaderWriter::supportedType(formats[i]));
        m_formatOptions->addWidget(w);
    }
    
    formatChooser->setCurrentIndex(0);
    m_formatOptions->setCurrentIndex(0);
    
    optionsFrame->setVisible(false);
    
    connect(browseButton, SIGNAL(clicked()), SLOT(browse()));
    connect(formatChooser, SIGNAL(currentIndexChanged(int)),
            m_formatOptions, SLOT(setCurrentIndex(int)));
    connect(optionsButton, SIGNAL(toggled(bool)), SLOT(showHideOptions(bool)));
    connect(buttonBox, SIGNAL(accepted()), SLOT(ok()));
    connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
    
    if(!format.empty()) {
        for(int i = 0; i < formatChooser->count(); i++) {
            if(formatChooser->itemText(i) == format.c_str()) {
                formatChooser->setCurrentIndex(i);
                break;
            }
        }
    }
}

QtRSFileSaver::~QtRSFileSaver() { }


String QtRSFileSaver::getFilename() {
    return QFileInfo(fileLocation->text()).absoluteFilePath().toStdString();
}

String QtRSFileSaver::getDirectory() {
    return QFileInfo(fileLocation->text()).absoluteDir().path().toStdString();
}

String QtRSFileSaver::getFormat() {
    return formatChooser->currentText().toStdString();
}

QWidget* QtRSFileSaver::getOptions() {
    return m_formatOptions->currentWidget();
}

void QtRSFileSaver::browse() {
    QString format = formatChooser->currentText();
    QString ext(RSFileReaderWriter::extensionForType(
            RSFileReaderWriter::supportedType(format.toStdString())).c_str());
    QString file = QFileDialog::getSaveFileName(this, "Save "+ format +" file",
            m_lastDir, format + " Regions (*." + ext + ")");
    if(!file.isEmpty()) fileLocation->setText(file);
}

void QtRSFileSaver::showHideOptions(bool show) {
    optionsFrame->setVisible(show);
    optionsButton->setText(show ? "-" : "+");
}

void QtRSFileSaver::ok() {
    QFile file(fileLocation->text());
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        QMessageBox::critical(this, "Save Error", "File "+fileLocation->text()+
                              " could not be opened for writing!");
    else accept();
}


RSFileWriter* QtRSFileSaver::getFileWriter(String* file, String* format,
        String* directory) {
    String fi = (file != NULL) ? *file : "",
           fo = (format != NULL) ? *format : "",
           di = (directory != NULL) ? *directory : "";
    QtRSFileSaver saver(fi, fo, di);
    if(saver.exec() == QDialog::Rejected) return NULL;
    
    fi = saver.getFilename();
    fo = saver.getFormat();
    di = saver.getDirectory();
    if(file != NULL) *file = fi;
    if(format != NULL) *format = fo;
    if(directory != NULL) *directory = di;
    
    QWidget* options = saver.getOptions();    
    RSFileReaderWriter::SupportedType type =
        RSFileReaderWriter::supportedType(fo);
    RSFileWriter* writer = RSFileReaderWriter::writerForType(type);
    
    if(writer != NULL) {
        writer->setFile(fi);
        writer->setOptions(options);
    }
    
    return writer;
}

}
