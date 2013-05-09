//# QtNewRegionShape.cc: Widgets for creating a new region shape(s).
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
#include <QSplitter>
#include <QMessageBox>

#include <display/RegionShapes/QtNewRegionShape.qo.h>
#include <display/RegionShapes/QtRegionShapeManager.qo.h>
#include <display/RegionShapes/QtSingleRegionShape.qo.h>
#include <display/RegionShapes/RSUtils.qo.h>

namespace casa {

// QTNEWREGIONSHAPE DEFINITIONS //

	QtNewRegionShape::QtNewRegionShape(QtRegionShapeManager* manager,
	                                   bool includeComposite, bool deleteOnClose) : m_manager(manager) {
		setupUi(this);

		vector<pair<String, RegionShape*> > v = creationShapes(includeComposite);
		m_shapes.resize(v.size());

		m_widgets = new QStackedLayout(frame);
		RSPolygon* ps;
		RSComposite* cs;
		QWidget* w;
		for(unsigned int i = 0; i < v.size(); i++) {
			shapeChooser->addItem(v[i].first.c_str());
			m_shapes[i] = v[i].second;
			if((ps = dynamic_cast<RSPolygon*>(m_shapes[i])) != NULL)
				w = new QtNewRSPolygon(ps, m_manager);
			else if((cs = dynamic_cast<RSComposite*>(m_shapes[i])) != NULL)
				w = new QtNewRSComposite(cs, m_manager);
			else w = new QtEditRegionShape(m_shapes[i], m_manager);

			m_widgets->addWidget(w);
		}

		connect(shapeChooser, SIGNAL(currentIndexChanged(int)),
		        m_widgets, SLOT(setCurrentIndex(int)));
		connect(createButton, SIGNAL(clicked()), SLOT(create()));
		connect(cancelButton, SIGNAL(clicked()), SLOT(close()));
		if(deleteOnClose) setAttribute(Qt::WA_DeleteOnClose, true);
	}

	QtNewRegionShape::~QtNewRegionShape() {
		for(unsigned int i = 0; i < m_shapes.size(); i++) delete m_shapes[i];
	}

	void QtNewRegionShape::showCloseButton(bool show) {
		cancelButton->setVisible(show);
	}

	void QtNewRegionShape::create() {
		QtNewRSPolygon* p = dynamic_cast<QtNewRSPolygon*>(
		                        m_widgets->currentWidget());
		QtNewRSComposite* c = dynamic_cast<QtNewRSComposite*>(
		                          m_widgets->currentWidget());
		QtEditRegionShape* e = dynamic_cast<QtEditRegionShape*>(
		                           m_widgets->currentWidget());

		bool error = false;
		String errorReason = "One or more entered values is not valid!";
		if(p != NULL) {
			if(p->enteredCoordinatesAreValid(errorReason)) p->apply();
			else error = true;
		} else if(c != NULL) {
			if(c->enteredValuesAreValid(errorReason)) c->apply();
			else error = true;
		} else if(e != NULL) {
			if(e->enteredCoordinatesAreValid(errorReason)) e->apply();
			else error = true;
		}
		if(error) {
			QMessageBox::warning(this, "Creation Error", errorReason.c_str());
			return;
		} else {
			int i = m_widgets->currentIndex();
			emit shapeCreated(m_shapes[i]);
			m_shapes[i] = creationShape(i);
			QWidget* w = NULL;
			if(p != NULL)
				w = new QtNewRSPolygon((RSPolygon*)m_shapes[i], m_manager);
			else if(e != NULL)
				w = new QtEditRegionShape(m_shapes[i], m_manager);
			else if(c != NULL)
				w = new QtNewRSComposite((RSComposite*)m_shapes[i], m_manager);
			m_widgets->insertWidget(i, w);
			w = m_widgets->currentWidget();
			m_widgets->setCurrentIndex(i);
			m_widgets->removeWidget(w);
			delete w;
		}
	}


// QTNEWRSPOLYGON DEFINITIONS //

	QtNewRSPolygon::QtNewRSPolygon(RSPolygon* poly, QtRegionShapeManager* m) :
		QWidget(), m_polygon(poly) {
		QHBoxLayout* topLayout = RSUtils::hlayout(this);
		QSplitter* splitter = new QSplitter(Qt::Horizontal, this);

		m_coordFrame = new QFrame();
		m_coordFrame->setFrameShape(QFrame::NoFrame);
		QVBoxLayout* l = RSUtils::vlayout(m_coordFrame);

		m_coordList = new QListWidget();
		m_coordList->setSelectionMode(QAbstractItemView::SingleSelection);
		l->addWidget(m_coordList);

		QHBoxLayout* l2 = RSUtils::hlayout();

		l2->addStretch();
		QToolButton* button = new QToolButton();
		button->setText("^");
		connect(button, SIGNAL(clicked()), SLOT(listMoveUp()));
		l2->addWidget(button);
		button = new QToolButton();
		button->setText("V");
		connect(button, SIGNAL(clicked()), SLOT(listMoveDown()));
		l2->addWidget(button);
		button = new QToolButton();
		button->setText("X");
		connect(button, SIGNAL(clicked()), SLOT(listDelete()));
		l2->addWidget(button);
		l2->addStretch();
		l->addLayout(l2);
		splitter->addWidget(m_coordFrame);

		QFrame* frame = new QFrame();
		frame->setFrameShape(QFrame::NoFrame);
		l2 = RSUtils::hlayout(frame);

		l2->addWidget(new QLabel("("));
		m_coordXEdit = new QLineEdit("0");
		l2->addWidget(m_coordXEdit);
		l2->addWidget(new QLabel(","));
		m_coordYEdit = new QLineEdit("0");
		l2->addWidget(m_coordYEdit);
		l2->addWidget(new QLabel(")"));
		QPushButton* button2 = new QPushButton("Add Vertex");
		connect(button2, SIGNAL(clicked()), SLOT(addCoordinates()));
		l2->addWidget(button2);

		m_editor = new QtEditRegionShape(poly, m, frame, true, false);
		splitter->addWidget(m_editor);
		m_coordFrame->hide();

		topLayout->addWidget(splitter);
	}

	QtNewRSPolygon::~QtNewRSPolygon() { }

	bool QtNewRSPolygon::enteredCoordinatesAreValid(String& reason) const {
		if(m_enteredCoords.size() < 2) {
			reason = "Cannot have < 2 polygon vertices!";
			return false;
		}

		String posUnit = m_editor->chosenPositionUnit(),
		       p1 = posUnit, p2 = posUnit;
		if(posUnit == QtEditRegionShape::SEXAGESIMAL) {
			p1 = RSValue::HMS;
			p2 = RSValue::DMS;
		}
		RSValue val;
		for(unsigned int i = 0; i < m_enteredCoords.size(); i++) {
			if(!RSValue::convertBetween(m_enteredCoords[i].first, p1, val) ||
			        !RSValue::convertBetween(m_enteredCoords[i].second, p2, val)) {
				reason = "Invalid value given at index "+ String::toString(i) +".";
				return false;
			}
		}
		return true;
	}

	void QtNewRSPolygon::apply() {
		String posUnit = m_editor->chosenPositionUnit(),
		       system = m_editor->chosenCoordinateSystem(),
		       p1 = posUnit, p2 = posUnit;
		if(posUnit == QtEditRegionShape::SEXAGESIMAL) {
			p1 = RSValue::HMS;
			p2 = RSValue::DMS;
		}

		m_polygon->setIsWorld(system != RSUtils::PIXEL);
		if(m_polygon->isWorld()) {
			MDirection::Types t;
			if(MDirection::getType(t, system)) m_polygon->setWorldSystem(t);
		}

		RSValue x1, y1, x2, y2;
		vector<double> x, y;
		for(unsigned int i = 0; i < m_enteredCoords.size(); i++) {
			if(RSValue::convertBetween(m_enteredCoords[i].first, p1, x1) &&
			        RSValue::convertBetween(x1, x2, RegionShape::UNIT) &&
			        RSValue::convertBetween(m_enteredCoords[i].second, p2, y1) &&
			        RSValue::convertBetween(y1, y2, RegionShape::UNIT)) {
				x.push_back(x2.val.getValue());
				y.push_back(y2.val.getValue());
			}
		}

		m_polygon->setPolygonCoordinates(Vector<double>(x), Vector<double>(y));

		m_editor->apply();
	}

	void QtNewRSPolygon::addCoordinates() {
		QString x = m_coordXEdit->text(), y = m_coordYEdit->text();
		m_enteredCoords.push_back(pair<QString, QString>(x, y));
		QString disp = "(" + x + ", " + y + ")";
		m_coordList->addItem(disp);
		m_coordXEdit->setText("");
		m_coordYEdit->setText("");
		m_coordFrame->show();
	}

	void QtNewRSPolygon::listMoveUp() {
		if(m_coordList->count() == 0 || m_coordList->currentRow() <= 0) return;
		int i = m_coordList->currentRow();
		pair<QString, QString> p = m_enteredCoords[i - 1];
		m_enteredCoords[i - 1] = m_enteredCoords[i];
		m_enteredCoords[i] = p;
		QListWidgetItem* w = m_coordList->takeItem(i);
		m_coordList->insertItem(i - 1, w);
		m_coordList->setCurrentRow(i - 1);
	}

	void QtNewRSPolygon::listMoveDown() {
		if(m_coordList->count() == 0 || m_coordList->currentRow() < 0 ||
		        m_coordList->currentRow() >= m_coordList->count() - 1) return;
		int i = m_coordList->currentRow();
		pair<QString, QString> p = m_enteredCoords[i + 1];
		m_enteredCoords[i + 1] = m_enteredCoords[i];
		m_enteredCoords[i] = p;
		QListWidgetItem* w = m_coordList->takeItem(i);
		m_coordList->insertItem(i + 1, w);
		m_coordList->setCurrentRow(i + 1);
	}

	void QtNewRSPolygon::listDelete() {
		if(m_coordList->count() == 0 || m_coordList->currentRow() < 0) return;
		int i = m_coordList->currentRow();
		m_enteredCoords.erase(m_enteredCoords.begin() + i);
		delete m_coordList->takeItem(i);
		m_coordList->setCurrentRow(-1);
		if(m_coordList->count() == 0) m_coordFrame->hide();
	}


// QTNEWRSCOMPOSITE DEFINITIONS //

	QtNewRSComposite::QtNewRSComposite(RSComposite* comp, QtRegionShapeManager* m):
		m_manager(m), m_composite(comp) {
		m_layout = new QStackedLayout(this);
		m_layout->addWidget(initFirstScreen());
		m_layout->addWidget(initSecondScreen1());
		m_layout->addWidget(initSecondScreen2());

		m_dependentLabel2 = new QLabel();
		QtEditRegionShape* e = new QtEditRegionShape(m_composite, m_manager,
		        m_dependentLabel2, false, false);
		m_layout->addWidget(e);
	}

	QtNewRSComposite::~QtNewRSComposite() { }

	bool QtNewRSComposite::enteredValuesAreValid(String& reason) const {
		if(m_layout->currentIndex() <= 0) {
			reason = "Must enter initial parameters!";
			return false;
		}
		if(m_children.size() == 0 && m_moveShapes.size() == 0) {
			reason = "Composite must have one or more children!";
			return false;
		}
		if(m_children.size() > 0) {
			bool error = false;
			String errorReason;
			int i = 1;
			for(; i < m_editWidgets->count() - 2; i++) {
				if(!dynamic_cast<QtEditRegionShape*>(m_editWidgets->widget(i))->
				        enteredCoordinatesAreValid(errorReason)) {
					error = true;
					break;
				}
			}
			if(error) {
				reason = "The child at index " + String::toString(i - 1) +
				         "reported:\n" + errorReason;
				return false;
			}
		}

		return true;
	}

	void QtNewRSComposite::apply() {
		String str;
		if(!enteredValuesAreValid(str)) return;

		if(m_moveShapes.size() > 0) {
			// remove from manager and add to children
			for(unsigned int i = 0; i < m_moveShapes.size(); i++)
				m_manager->removeShape(m_moveShapes[i], false);
			m_children = m_moveShapes;

			// apply composite edit
			dynamic_cast<QtEditRegionShape*>(m_layout->widget(3))->apply();
		} else {
			// apply all edits to children
			for(int i = 1; i < m_editWidgets->count() - 2; i++) {
				dynamic_cast<QtEditRegionShape*>(m_editWidgets->widget(i))->apply();
			}
			// apply composite edit
			dynamic_cast<QtEditRegionShape*>(m_editWidgets->widget(0))->apply();
		}

		// add to composite
		m_composite->addShapes(m_children);
	}

	QWidget* QtNewRSComposite::initFirstScreen() {
		QFrame* frame = new QFrame();
		frame->setFrameShape(QFrame::NoFrame);
		QVBoxLayout* l = RSUtils::vlayout(frame);
		l->addStretch();

		QHBoxLayout* l2 = RSUtils::hlayout();
		l2->addStretch();
		l2->addWidget(new QLabel("Children are:"));
		QRadioButton* rb = new QRadioButton("Independent");
		rb->setChecked(true);
		l2->addWidget(rb);
		l2->addWidget(m_dependentChildren = new QRadioButton("Dependent"));
		l2->addStretch();
		l->addLayout(l2);

		QLabel* lb = new QLabel("Independent children can be edited individually, "
		                        "whereas a composite with dependent children is "
		                        "treated as a single shape.");
		lb->setWordWrap(true);
		l->addWidget(lb);

		QFrame* frame2 = new QFrame();
		frame2->setFrameShape(QFrame::NoFrame);
		l2 = RSUtils::hlayout(frame2);
		l2->addStretch();
		l2->addWidget(new QLabel("Add:"));
		m_newShapes = new QRadioButton("New Shapes");
		m_newShapes->setChecked(true);
		l2->addWidget(m_newShapes);
		l2->addWidget(new QRadioButton("Existing Shapes"));
		l2->addStretch();
		l->addWidget(frame2);
		if(m_manager->numShapes(false) == 0) frame2->close();
		l->addStretch();

		l2 = RSUtils::hlayout();
		l2->addStretch();
		QPushButton* b = new QPushButton("Next");
		connect(b, SIGNAL(clicked()), SLOT(next()));
		l2->addWidget(b);
		l->addLayout(l2);

		return frame;
	}

	QWidget* QtNewRSComposite::initSecondScreen1() {
		QSplitter* splitter = new QSplitter();

		QFrame* frame = new QFrame();
		frame->setFrameShape(QFrame::NoFrame);
		QVBoxLayout* vl = RSUtils::vlayout(frame);
		m_newShapeList = new QListWidget();
		m_newShapeList->setSelectionMode(QAbstractItemView::SingleSelection);
		vl->addWidget(m_newShapeList);

		QHBoxLayout* hl = RSUtils::hlayout();
		hl->addStretch();
		QToolButton* b = new QToolButton();
		b->setText("^");
		connect(b, SIGNAL(clicked()), SLOT(nListUp()));
		hl->addWidget(b);
		b = new QToolButton();
		b->setText("V");
		connect(b, SIGNAL(clicked()), SLOT(nListDown()));
		hl->addWidget(b);
		b = new QToolButton();
		b->setText("X");
		connect(b, SIGNAL(clicked()), SLOT(nListDelete()));
		hl->addWidget(b);
		hl->addStretch();
		vl->addLayout(hl);
		splitter->addWidget(frame);

		frame = new QFrame();
		frame->setFrameShape(QFrame::NoFrame);
		vl = RSUtils::vlayout(frame);
		hl = RSUtils::hlayout();
		hl->addStretch();
		hl->addWidget(new QLabel("Edit:"));
		m_editChooser = new QComboBox();
		m_editChooser->addItem("composite");
		m_editChooser->addItem("[new shape]");
		hl->addWidget(m_editChooser);
		hl->addStretch();
		vl->addLayout(hl);
		vl->addWidget(RSUtils::hline());

		// edit widgets
		QFrame* frame2 = new QFrame();
		frame2->setFrameShape(QFrame::NoFrame);
		m_editWidgets = new QStackedLayout(frame2);
		connect(m_editChooser, SIGNAL(currentIndexChanged(int)),
		        m_editWidgets, SLOT(setCurrentIndex(int)));

		// edit widgets: composite
		m_dependentLabel1 = new QLabel();
		m_editWidgets->addWidget(new QtEditRegionShape(m_composite, m_manager,
		                         m_dependentLabel1, false, false));

		// edit widgets: new shape
		QtNewRegionShape* nrs = new QtNewRegionShape(m_manager, false, false);
		nrs->showCloseButton(false);
		connect(nrs, SIGNAL(shapeCreated(RegionShape*)),
		        SLOT(nAddShape(RegionShape*)));
		m_editWidgets->addWidget(nrs);
		vl->addWidget(frame2);
		splitter->addWidget(frame);

		return splitter;
	}

	QWidget* QtNewRSComposite::initSecondScreen2() {
		QFrame* topFrame = new QFrame();
		topFrame->setFrameShape(QFrame::NoFrame);
		QVBoxLayout* topVL = RSUtils::vlayout(topFrame);

		QSplitter* splitter = new QSplitter();

		QFrame* frame = new QFrame();
		frame->setFrameShape(QFrame::NoFrame);
		QVBoxLayout* vl = RSUtils::vlayout(frame);

		QHBoxLayout* hl = RSUtils::hlayout();
		hl->addStretch();
		hl->addWidget(new QLabel("Existing Shapes"));
		hl->addStretch();
		vl->addLayout(hl);

		m_existingList = new QListWidget();
		m_existingList->setSelectionMode(QAbstractItemView::SingleSelection);
		vl->addWidget(m_existingList);

		m_existingShapes = m_manager->shapes(false);
		for(unsigned int i = 0; i < m_existingShapes.size(); i++)
			m_existingList->addItem(m_existingShapes[i]->title().c_str());

		hl = RSUtils::hlayout();
		hl->addStretch();
		QPushButton* b = new QPushButton(">>");
		connect(b, SIGNAL(clicked()), SLOT(eAddShape()));
		hl->addWidget(b);
		hl->addStretch();
		vl->addLayout(hl);
		splitter->addWidget(frame);

		frame = new QFrame();
		frame->setFrameShape(QFrame::NoFrame);
		vl = RSUtils::vlayout(frame);

		hl = RSUtils::hlayout();
		hl->addStretch();
		hl->addWidget(new QLabel("Composite Shapes"));
		hl->addStretch();
		vl->addLayout(hl);

		m_moveList = new QListWidget();
		m_moveList->setSelectionMode(QAbstractItemView::SingleSelection);
		vl->addWidget(m_moveList);

		hl = RSUtils::hlayout();
		hl->addStretch();
		b = new QPushButton("<<");
		connect(b, SIGNAL(clicked()), SLOT(eRemoveShape()));
		hl->addWidget(b);
		b = new QPushButton("^");
		connect(b, SIGNAL(clicked()), SLOT(eListUp()));
		hl->addWidget(b);
		b = new QPushButton("V");
		connect(b, SIGNAL(clicked()), SLOT(eListDown()));
		hl->addWidget(b);
		hl->addStretch();
		vl->addLayout(hl);
		splitter->addWidget(frame);
		topVL->addWidget(splitter);

		hl = RSUtils::hlayout();
		hl->addStretch();
		b = new QPushButton("Accept and Edit Composite Properties");
		connect(b, SIGNAL(clicked()), SLOT(next()));
		hl->addWidget(b);
		topVL->addLayout(hl);

		return topFrame;
	}

	void QtNewRSComposite::updateLists() {
		RegionShape* s;
		if(m_moveList->count() != (int)m_moveShapes.size()) {
			m_existingList->clear();
			bool found = false;
			for(unsigned int i = 0; i < m_existingShapes.size(); i++) {
				s = m_existingShapes[i];
				found = false;
				for(unsigned int j = 0; j < m_moveShapes.size(); j++) {
					if(m_moveShapes[j] == s) {
						found = true;
						break;
					}
				}
				if(!found) m_existingList->addItem(s->title().c_str());
			}
		}
		m_moveList->clear();
		for(unsigned int i = 0; i < m_moveShapes.size(); i++) {
			s = m_moveShapes[i];
			m_moveList->addItem(s->title().c_str());
		}
	}

	unsigned int QtNewRSComposite::adjustedIndex(int row) {
		if(row < 0) return 0;
		int index = -1;
		bool found = false;
		for(unsigned int i = 0; i < m_existingShapes.size(); i++) {
			found = false;
			for(unsigned int j = 0; j < m_moveShapes.size(); j++) {
				if(m_existingShapes[i] == m_moveShapes[j]) {
					found = true;
					break;
				}
			}
			if(!found) index++;
			if(index == row) return i;
		}
		return 0;
	}

	void QtNewRSComposite::next() {
		if(m_layout->currentIndex() == 0) {
			m_composite->setChildrenAreDependent(m_dependentChildren->isChecked());
			QString label = "(children are " + QString(
			                    m_composite->childrenAreDependent() ? "dependent" :
			                    "independent") + ")";
			m_dependentLabel1->setText(label);
			m_dependentLabel2->setText(label);
			if(!m_newShapes->isVisible() || m_newShapes->isChecked())
				m_layout->setCurrentIndex(1);
			else m_layout->setCurrentIndex(2);
		} else if(m_layout->currentIndex() == 2) {
			m_layout->setCurrentIndex(3);
		}
	}

	void QtNewRSComposite::nAddShape(RegionShape* shape) {
		m_children.push_back(shape);
		m_newShapeList->addItem(shape->title().c_str());
		m_editChooser->insertItem(m_editChooser->count()-1,shape->title().c_str());
		m_editWidgets->insertWidget(m_editWidgets->count()-1,new QtEditRegionShape(
		                                shape, m_manager));
	}

	void QtNewRSComposite::nListUp() {
		if(m_newShapeList->count()== 0 || m_newShapeList->currentRow()<= 0) return;
		int i = m_newShapeList->currentRow();
		RegionShape* s = m_children[i];
		m_children[i] = m_children[i - 1];
		m_children[i - 1] = s;
		QListWidgetItem* w = m_newShapeList->takeItem(i);
		m_newShapeList->insertItem(i - 1, w);
		m_newShapeList->setCurrentRow(i - 1);
		QWidget* widget = m_editWidgets->widget(i + 1);
		m_editWidgets->removeWidget(widget);
		m_editWidgets->insertWidget(i, widget);
		QString t = m_editChooser->itemText(i + 1);
		m_editChooser->removeItem(i + 1);
		m_editChooser->insertItem(i, t);
		m_editChooser->setCurrentIndex(i);
	}

	void QtNewRSComposite::nListDown() {
		if(m_newShapeList->count() == 0 || m_newShapeList->currentRow() < 0 ||
		        m_newShapeList->currentRow() >= m_newShapeList->count() - 1) return;
		int i = m_newShapeList->currentRow();
		RegionShape* s = m_children[i];
		m_children[i] = m_children[i + 1];
		m_children[i + 1] = s;
		QListWidgetItem* w = m_newShapeList->takeItem(i);
		m_newShapeList->insertItem(i + 1, w);
		m_newShapeList->setCurrentRow(i + 1);
		QWidget* widget = m_editWidgets->widget(i + 1);
		m_editWidgets->removeWidget(widget);
		m_editWidgets->insertWidget(i + 2, widget);
		QString t = m_editChooser->itemText(i + 1);
		m_editChooser->removeItem(i + 1);
		m_editChooser->insertItem(i + 2, t);
		m_editChooser->setCurrentIndex(i + 2);
	}

	void QtNewRSComposite::nListDelete() {
		if(m_newShapeList->count()== 0 || m_newShapeList->currentRow() < 0) return;
		int i = m_newShapeList->currentRow();
		RegionShape* s = m_children[i];
		m_children.erase(m_children.begin() + i);
		delete s;
		delete m_newShapeList->takeItem(i);
		m_newShapeList->setCurrentRow(-1);
		m_editChooser->removeItem(i + 1);
		QWidget* w = m_editWidgets->widget(i + 1);
		m_editWidgets->removeWidget(w);
		delete w;
	}

	void QtNewRSComposite::eAddShape() {
		if(m_existingList->currentRow() == -1) return;
		int row = m_existingList->currentRow();
		unsigned int i = adjustedIndex(row);
		m_moveShapes.push_back(m_existingShapes[i]);
		updateLists();
	}

	void QtNewRSComposite::eRemoveShape() {
		if(m_moveList->currentRow() == -1) return;
		int i = m_moveList->currentRow();
		m_moveShapes.erase(m_moveShapes.begin() + i);
		updateLists();
	}

	void QtNewRSComposite::eListUp() {
		if(m_moveList->count() == 0 || m_moveList->currentRow() < 1) return;
		int i = m_moveList->currentRow();
		RegionShape* s = m_moveShapes[i];
		m_moveShapes[i] = m_moveShapes[i - 1];
		m_moveShapes[i - 1] = s;
		updateLists();
	}

	void QtNewRSComposite::eListDown() {
		if(m_moveList->count() == 0 || m_moveList->currentRow() < 0 ||
		        m_moveList->currentRow() >= m_moveList->count() - 1) return;
		int i = m_moveList->currentRow();
		RegionShape* s = m_moveShapes[i];
		m_moveShapes[i] = m_moveShapes[i + 1];
		m_moveShapes[i + 1] = s;
		updateLists();
	}

}
