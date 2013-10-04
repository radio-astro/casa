//# QtSingleRegionShape.cc: Classes for viewing/editing a single RegionShape.
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
#include <display/RegionShapes/QtSingleRegionShape.qo.h>

#include <display/RegionShapes/QtRegionShapeManager.qo.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>
#include <display/RegionShapes/RegionShapes.h>

#include <limits>
#include <math.h>

namespace casa {

// QTSINGLEREGIONSHAPE DEFINITIONS //

	QtSingleRegionShape::QtSingleRegionShape(RegionShape* shape,
	        QtRegionShapeManager* parent, bool repErr, RegionShape* comp) :
		m_shape(shape), m_compositeParent(comp), m_parent(parent),
		m_panel(parent->panel()), m_shouldDelete(parent == NULL) {
		setupUi(this);

		setupGUI();

		if(comp == NULL) {
			registerShape();
			if(repErr && shape->hadDrawingError() &&
			        !shape->lastDrawingError().empty())
				parent->showDetailedError(QtRegionShapeManager::DRAWERRORMSG,
				                          shape->lastDrawingError());
		} else {
			titleBox->setEnabled(false);
			deleteButton->setEnabled(false);
		}

		connect(titleBox, SIGNAL(clicked(bool)), SLOT(showHide_(bool)));
		connect(editButton, SIGNAL(clicked()), SLOT(edit()));
		connect(deleteButton, SIGNAL(clicked()), SLOT(deleteShape()));
	}

	QtSingleRegionShape::~QtSingleRegionShape() {
		if(m_compositeParent == NULL) {
			if(m_panel->isRegistered(m_shape)) unregisterShape();
			if(m_shouldDelete) delete m_shape;
		}
	}

	RegionShape* QtSingleRegionShape::shape() {
		return m_shape;
	}
	RegionShape* QtSingleRegionShape::compositeParent() {
		return m_compositeParent;
	}
	QtRegionShapeManager* QtSingleRegionShape::manager() {
		return m_parent;
	}
	QtDisplayPanel* QtSingleRegionShape::panel() {
		return m_panel;
	}

	void QtSingleRegionShape::setShouldDelete(bool shouldDelete) {
		m_shouldDelete = m_compositeParent == NULL && shouldDelete;
	}

	void QtSingleRegionShape::refresh() {
		setupGUI();

		RSComposite* comp = dynamic_cast<RSComposite*>(m_shape);
		if(comp != NULL) {
			// also refresh children
			vector<RegionShape*> children = comp->children();
			QtSingleRegionShape* child;
			for(unsigned int i = 0; i < children.size(); i++) {
				child = m_parent->shapeWidget(children[i]);
				if(child != NULL) child->refresh();
			}
		}
	}

	bool QtSingleRegionShape::isShown() const {
		return titleBox->isChecked();
	}

	void QtSingleRegionShape::showHide(bool show, bool reportErrors) {
		if(!titleBox->isEnabled()) return;
		titleBox->blockSignals(true);
		titleBox->setChecked(show);
		if(show) {
			if(!m_panel->isRegistered(m_shape)) {
				registerShape();
				if(m_shape->hadDrawingError() && m_compositeParent == NULL &&
				        reportErrors && !m_shape->lastDrawingError().empty()) {
					m_parent->showDetailedError(QtRegionShapeManager::DRAWERRORMSG,
					                            m_shape->lastDrawingError());
				}
			}
		} else unregisterShape();
		titleBox->blockSignals(false);
	}

	void QtSingleRegionShape::edit() {
		QtEditRegionShape* e = new QtEditRegionShape(this);
		//e->setWindowModality(Qt::ApplicationModal);
		e->setVisible(true);
		e->raise();

		// disable input to manager until editor is closed
		m_parent->setEnabled(false);
		connect(e, SIGNAL(destroyed(QObject*)), m_parent, SLOT(enable()));
	}

	void QtSingleRegionShape::setupGUI() {
		titleBox->setText(m_shape->title().c_str());
	}

	void QtSingleRegionShape::registerShape() {
		if(!m_panel->isRegistered(m_shape) && m_compositeParent == NULL)
			m_panel->registerRegionShape(m_shape);
	}

	void QtSingleRegionShape::unregisterShape() {
		if(m_compositeParent == NULL) m_panel->unregisterRegionShape(m_shape);
	}

	void QtSingleRegionShape::deleteShape() {
		m_parent->removeShape(m_shape, m_shouldDelete);
	}


// QTRSOPTION DEFINITIONS //

	QtRSOption::QtRSOption(RegionShape::OptionType type, const String& name,
	                       const RSOption& value, const vector<String>& choices) : m_type(type),
		m_string(NULL), m_choice(NULL), m_double(NULL), m_bool(NULL),
		m_choices(choices) {
		RSUtils::setupLayout(this);

		switch(type) {
		case RegionShape::STRING:
			addWidget(new QLabel(String(name + ": ").c_str()));
			m_string = new QLineEdit(value.asString().c_str());
			addWidget(m_string);
			break;

		case RegionShape::STRINGCHOICE: {
			addWidget(new QLabel(String(name + ": ").c_str()));
			m_choice = new QComboBox();
			unsigned int j = choices.size();
			for(unsigned int i = 0; i < choices.size(); i++) {
				if(choices[i] == value.asString()) j = i;
				m_choice->addItem(choices[i].c_str());
			}
			if(j < choices.size()) m_choice->setCurrentIndex(j);
			addWidget(m_choice);
			break;
		}

		case RegionShape::STRINGARRAY: {
			addWidget(new QLabel(String(name + ": ").c_str()));
			QLineEdit* e;
			for(unsigned int i = 0; i < value.asStringArray().size(); i++) {
				e = new QLineEdit(value.asStringArray()[i].c_str());
				m_stringArray.push_back(e);
				addWidget(e);
			}
			m_lessButton = new QToolButton();
			m_lessButton->setText("<<");
			m_lessButton->setEnabled(m_stringArray.size() > 0);
			connect(m_lessButton, SIGNAL(clicked()), SLOT(lessArray()));
			addWidget(m_lessButton);
			m_moreButton = new QToolButton();
			m_moreButton->setText(">>");
			connect(m_moreButton, SIGNAL(clicked()), SLOT(moreArray()));
			addWidget(m_moreButton);
			addStretch();
			break;
		}

		case RegionShape::STRINGCHOICEARRAY: {
			addWidget(new QLabel(String(name + ": ").c_str()));
			QComboBox* b;
			unsigned int k = choices.size();
			for(unsigned int i = 0; i < value.asStringArray().size(); i++) {
				b = new QComboBox();
				k = choices.size();
				for(unsigned int j = 0; j < choices.size(); j++) {
					if(choices[j] == value.asStringArray()[i]) k = j;
					b->addItem(choices[j].c_str());
				}
				if(k < choices.size()) b->setCurrentIndex(k);
				m_choiceArray.push_back(b);
				addWidget(b);
			}
			m_lessButton = new QToolButton();
			m_lessButton->setText("<<");
			m_lessButton->setEnabled(m_choiceArray.size() > 0);
			connect(m_lessButton, SIGNAL(clicked()), SLOT(lessArray()));
			addWidget(m_lessButton);
			m_moreButton = new QToolButton();
			m_moreButton->setText(">>");
			connect(m_moreButton, SIGNAL(clicked()), SLOT(moreArray()));
			addWidget(m_moreButton);
			addStretch();
			break;
		}

		case RegionShape::DOUBLE:
			addWidget(new QLabel(String(name + ": ").c_str()));
			m_double = new QDoubleSpinBox();
			m_double->setMinimum(std::numeric_limits<double>::min());
			m_double->setMaximum(std::numeric_limits<double>::max());
			m_double->setValue(value.asDouble());
			addWidget(m_double);
			break;

		case RegionShape::BOOL:
			m_bool = new QCheckBox(name.c_str());
			m_bool->setChecked(value.asBool());
			addWidget(m_bool);
			break;
		}
	}

	QtRSOption::~QtRSOption() { }

	RSOption QtRSOption::value() const {
		switch(m_type) {
		case RegionShape::STRING:
			return RSOption(m_string->text().toStdString());
		case RegionShape::STRINGCHOICE:
			return RSOption(m_choice->currentText().toStdString());
		case RegionShape::STRINGARRAY: {
			vector<String> v(m_stringArray.size());
			for(unsigned int i = 0; i < m_stringArray.size(); i++)
				v[i] = m_stringArray[i]->text().toStdString();
			return RSOption(v);
		}
		case RegionShape::STRINGCHOICEARRAY: {
			vector<String> v(m_choiceArray.size());
			for(unsigned int i = 0; i < m_choiceArray.size(); i++)
				v[i] = m_choiceArray[i]->currentText().toStdString();
			return RSOption(v);
		}
		case RegionShape::DOUBLE:
			return RSOption(m_double->value());
		case RegionShape::BOOL:
			return RSOption(m_bool->isChecked());

		default:
			return RSOption("");
		}
	}

	void QtRSOption::moreArray() {
		if(m_type == RegionShape::STRINGARRAY) {
			QLineEdit* e = new QLineEdit();
			insertWidget(m_stringArray.size() + 1, e);
			m_stringArray.push_back(e);
		} else {
			QComboBox* b = new QComboBox();
			for(unsigned int i = 0; i < m_choices.size(); i++)
				b->addItem(m_choices[i].c_str());
			insertWidget(m_choiceArray.size() + 1, b);
			m_choiceArray.push_back(b);
		}
		m_lessButton->setEnabled(true);
	}

	void QtRSOption::lessArray() {
		if(m_type == RegionShape::STRINGARRAY) {
			QLineEdit* e = m_stringArray[m_stringArray.size() - 1];
			m_stringArray.erase(m_stringArray.begin() + m_stringArray.size() - 1);
			removeWidget(e);
			delete e;
			m_lessButton->setEnabled(m_stringArray.size() > 0);
		} else {
			QComboBox* b = m_choiceArray[m_choiceArray.size() - 1];
			m_choiceArray.erase(m_choiceArray.begin() + m_choiceArray.size() - 1);
			removeWidget(b);
			delete b;
			m_lessButton->setEnabled(m_choiceArray.size() > 0);
		}
	}


// QTEDITREGIONSHAPE DEFINITIONS //

	QtEditRegionShape::QtEditRegionShape(QtSingleRegionShape* s) : QWidget(NULL),
		m_creation(false), m_shape(s), m_cShape(NULL), m_manager(s->manager()) {
		init();
	}

	QtEditRegionShape::QtEditRegionShape(RegionShape* shape,
	                                     QtRegionShapeManager* manager, QWidget* coordWidget, bool showPos,
	                                     bool showSize, String applyButtonText): QWidget(NULL), m_creation(true),
		m_shape(NULL), m_cShape(shape), m_manager(manager) {
		init(coordWidget, showPos, showSize, applyButtonText);
	}

	QtEditRegionShape::~QtEditRegionShape() { }

	String QtEditRegionShape::chosenCoordinateSystem() const {
		return coordSystem->currentText().toStdString();
	}

	String QtEditRegionShape::chosenPositionUnit() const {
		return coordPositionUnit->currentText().toStdString();
	}

	String QtEditRegionShape::chosenSizeUnit() const {
		return coordSizeUnit->currentText().toStdString();
	}

	bool QtEditRegionShape::enteredCoordinatesAreValid(String& reason) const {
		if(m_coordEdits.size() == 0) return true;

		String posUnit = coordPositionUnit->currentText().toStdString(),
		       sizeUnit = coordSizeUnit->currentText().toStdString();
		String pUnit = posUnit;
		RSValue val;
		bool ok;
		for(unsigned int i = 0; i < m_coordEdits.size(); i++) {
			if(m_coordTypes[i] == RegionShape::POSITION) {
				if(posUnit == SEXAGESIMAL)
					pUnit = i % 2 == 0 ? RSValue::HMS : RSValue::DMS;
				if(!RSValue::convertBetween(m_coordEdits[i]->text(), pUnit, val)) {
					reason = "Invalid " + posUnit + " value given at coordinate "
					         "index " + String::toString(i) + ".";
					return false;
				}
			} else if(m_coordTypes[i] == RegionShape::SIZE) {
				if(!RSValue::convertBetween(m_coordEdits[i]->text(),sizeUnit,val)) {
					reason = "Invalid " + sizeUnit + " value given at coordinate "
					         "index " + String::toString(i) + ".";
					return false;
				}
			} else {
				m_coordEdits[i]->text().toDouble(&ok);
				if(!ok) {
					reason = "Invalid double value given at coordinate index " +
					         String::toString(i) + ".";
					return false;
				}
			}
		}
		return true;
	}

	void QtEditRegionShape::apply() {
		QApplication::setOverrideCursor(Qt::WaitCursor);
		bool changed = false;
		RegionShape* shape = m_creation ? m_cShape : m_shape->shape();

		if(m_creation) {
			shape->setIsWorld(coordSystem->currentIndex() <
			                  (coordSystem->count() - 1));
			if(shape->isWorld()) {
				MDirection::Types t;
				if(MDirection::getType(t,coordSystem->currentText().toStdString()))
					shape->setWorldSystem(t);
			}
		}

		// coordinates
		vector<double> oldVals = shape->coordParameterValues();
		vector<double> vals(m_coordEdits.size());
		bool allValid = true;
		bool ok;
		double d;
		for(unsigned int i = 0; i < m_coordEdits.size(); i++) {
			d = convertToRS(i, ok);
			if(ok) vals[i] = d;
			else {
				allValid = false;
				vals[i] = oldVals[i];
			}
		}

		bool isPixel = coordSystem->currentIndex() == coordSystem->count() - 1;
		if(allValid) {
			if((isPixel && !shape->isWorld()) || (!isPixel && shape->isWorld() &&
			                                      coordSystem->currentText() == QString(MDirection::showType(
			                                              shape->worldSystem()).c_str()))) {
				// same system
				for(unsigned int i = 0; !changed && i < vals.size(); i++)
					if(vals[i] != oldVals[i]) changed = true;
				if(changed) shape->setCoordParameters(vals);
			} else {
				// different system
				changed = true;
				shape->setCoordParameters(vals,
				                          coordSystem->currentText().toStdString());
			}
		} else {
			String reason = "One or more coordinate values was not valid!";
			enteredCoordinatesAreValid(reason);
			QMessageBox::warning(this, "Editing Error", reason.c_str());
		}

		// custom options
		if(m_optWidgets.size() > 0) {
			vector<RSOption> opts(m_optWidgets.size());
			for(unsigned int i = 0; i < m_optWidgets.size(); i++)
				opts[i] = m_optWidgets[i]->value();
			vector<RSOption> vals = shape->optionValues();
			bool ochanged = false;
			for(unsigned int i = 0; !ochanged && i < vals.size(); i++)
				if(vals[i] != opts[i]) ochanged = true;

			if(ochanged) {
				changed = true;
				shape->setOptionValues(opts);
			}
		}

		// color
		String color = m_lineColor->getColor();
		if(color.empty()) color = shape->lineColor();
		if(color != shape->lineColor() && !color.empty()) {
			shape->setLineColor(color);
			changed = true;
		}

		// width
		int width = widthSpinner->value();
		if(width != shape->lineWidth()) {
			shape->setLineWidth(width);
			changed = true;
		}

		// style
		RegionShape::LineStyle style = RegionShape::convLineStyle(
		                                   styleChooser->currentText().toStdString());
		if(style != shape->lineStyle()) {
			shape->setLineStyle(style);
			changed = true;
		}

		// text
		String text = labelEdit->text().toStdString();
		if(text != shape->text()) {
			shape->setText(text);
			changed = true;
		}

		bool isText = dynamic_cast<RSText*>(shape) != NULL;

		// font
		String font = fontChooser->currentText().toStdString();
		int size = fontSize->value();
		bool bold = boldBox->isChecked(), italic = italicBox->isChecked();
		String labelColor = color;
		if(labelOverride->isChecked()) {
			labelColor = m_labelColor->getColor();
			if(labelColor.empty()) shape->label().color();
		}
		RegionShapeLabel label = shape->label();
		if(font != label.font() || size != label.size() || bold!= label.isBold() ||
		        italic != label.isItalic() || labelColor != label.color()) {
			label.setFont(font);
			label.setSize(size);
			label.setBold(bold);
			label.setItalic(italic);
			label.setColor(labelColor);
			shape->setLabel(label);
			changed = true;
		}
		if(isText) shape->setLineColor(labelColor); // also set line color for text

		// linethrough
		if(ltBox->isChecked()) {
			if(ltOverride->isChecked()) {
				style = RegionShape::convLineStyle(
				            ltStyleChooser->currentText().toStdString());
				color = m_ltColor->getColor();
				if(color.empty()) color = shape->linethroughColor();
				width = ltWidth->value();
			}
			if(!shape->linethrough() || shape->linethroughColor() != color ||
			        shape->linethroughWidth() != width ||
			        style != shape->linethroughStyle()) {
				changed = true;
				shape->setLinethrough(true, color, width, style);
			}
		} else if(shape->linethrough()) {
			shape->setLinethrough(false);
			changed = true;
		}

		if(changed && !m_creation) {
			if(m_shape->compositeParent() == NULL) {
				if(m_shape->panel()->isRegistered(m_shape->shape())) {
					m_shape->unregisterShape();
					m_shape->registerShape();
				}
			} else {
				shape = m_shape->compositeParent();
				if(m_shape->panel()->isRegistered(shape)) {
					m_shape->panel()->unregisterRegionShape(shape);
					m_shape->panel()->registerRegionShape(shape);
				}
			}
			m_shape->refresh();
			setupGui();
		}
		QApplication::restoreOverrideCursor();
	}

	void QtEditRegionShape::init(QWidget* coordWidget, bool sPos, bool sUnit,
	                             String applyButtonText) {
		setupUi(this);
		ltFrame->setEnabled(false);

		RegionShape* shape = m_creation ? m_cShape : m_shape->shape();
		bool isText = dynamic_cast<RSText*>(shape) != NULL;

		// color
		m_lineColor = RSUtils::colorWidget(true, "", colorFrame);

		// line styles
		vector<RegionShape::LineStyle> styles = RegionShape::allLineStyles();
		QString style;
		for(unsigned int i = 0; i < styles.size(); i++) {
			style = RegionShape::convLineStyle(styles[i]).c_str();
			styleChooser->addItem(style);
			ltStyleChooser->addItem(style);
		}

		// coord system chooser
		vector<String> v = systems();
		unsigned int i = 0;
		for(; i < v.size(); i++)
			coordSystem->addItem(v[i].c_str());
		QString s;
		if(!m_creation) {
			s = (!shape->isWorld() ? RSUtils::PIXEL :
			     MDirection::showType(shape->worldSystem())).c_str();
		} else {
			ConstListIter<WorldCanvas*> wcs = m_manager->panel()->panelDisplay()->
			                                  myWCLI;
			if(wcs.len() > 0) {
				if(wcs.getRight()->hasCS()) {
					s = MDirection::showType(RSUtils::worldSystem(
					                             wcs.getRight())).c_str();
				}
			}
		}
		if(s.isEmpty()) s = MDirection::showType(MDirection::J2000).c_str();
		for(i = 0; i < v.size(); i++) {
			if(coordSystem->itemText(i).compare(s, Qt::CaseInsensitive) == 0)
				break;
		}
		if(i < v.size()) coordSystem->setCurrentIndex(i);
		else {
			coordSystem->addItem(UNKNOWN.c_str());
			coordSystem->setCurrentIndex(i);
			coordSystem->setEnabled(false);
		}

		// coord position unit chooser
		v = positionUnits();
		for(i = 0; i < v.size(); i++)
			coordPositionUnit->addItem(v[i].c_str());
		s = RegionShape::UNIT.c_str();
		for(i = 0; i < v.size(); i++) {
			if(coordPositionUnit->itemText(i).compare(s, Qt::CaseInsensitive) == 0)
				break;
		}
		if(i < v.size()) coordPositionUnit->setCurrentIndex(i);
		else {
			coordPositionUnit->addItem(UNKNOWN.c_str());
			coordPositionUnit->setCurrentIndex(i);
			coordPositionUnit->setEnabled(false);
		}
		if(!m_creation && !shape->isWorld()) coordPositionUnit->setEnabled(false);

		// coord size unit chooser
		v = sizeUnits();
		for(i = 0; i < v.size(); i++)
			coordSizeUnit->addItem(v[i].c_str());
		for(i = 0; i < v.size(); i++) {
			if(coordSizeUnit->itemText(i).compare(s, Qt::CaseInsensitive) == 0)
				break;
		}
		if(i < v.size()) coordSizeUnit->setCurrentIndex(i);
		else {
			coordSizeUnit->addItem(UNKNOWN.c_str());
			coordSizeUnit->setCurrentIndex(i);
			coordSizeUnit->setEnabled(false);
		}
		if(!m_creation && !shape->isWorld()) coordSizeUnit->setEnabled(false);

		m_coordTypes = shape->coordParameterTypes();
		bool hasPosition = sPos;
		bool hasSize = sUnit;
		if(!m_creation || coordWidget == NULL) {
			hasPosition = false;
			hasSize = false;
			for(i = 0; i < m_coordTypes.size(); i++) {
				hasPosition |= m_coordTypes[i] == RegionShape::POSITION ||
				               m_coordTypes[i] == RegionShape::ANGLE;
				hasSize |= m_coordTypes[i] == RegionShape::SIZE;
				if(hasPosition && hasSize) break;
			}
		}
		if(!hasPosition) {
			coordSystem->close();
			positionLabel->close();
			coordPositionUnit->close();
		}
		if(!hasSize) {
			sizeLabel->close();
			coordSizeUnit->close();
		}
		if(m_creation) {
			precisionLabel->close();
			precisionSpinner->close();
			coordResetFrame->close();
		}
		if(!hasPosition && !hasSize && m_creation) {
			chooserFrame->close();
			coordLine->close();
		}

		m_lastSystem = coordSystem->currentText().toStdString();
		m_lastPosUnit = coordPositionUnit->currentText().toStdString();
		m_lastSizeUnit = coordSizeUnit->currentText().toStdString();

		// coord values
		QGridLayout* layout = new QGridLayout(coordFrame);
		RSUtils::setupLayout(layout);

		if(!m_creation || coordWidget == NULL) {
			vector<double> cv = shape->coordParameterValues();
			vector<String> cn = shape->coordParameterNames();
			QHBoxLayout* l;
			QLineEdit* e;
			s = "%1";

			QString str;
			QLabel* lab;
			for(i = 0; i < cn.size(); i++) {
				str = cn[i].c_str() + QString(":");
				if(i % 2 == 1) str = "  " + str;
				lab = new QLabel(str);
				lab->setAlignment((Qt::Alignment)(Qt::AlignRight | Qt::AlignVCenter));
				layout->addWidget(lab, (i / 2), (i % 2) * 2);

				l = RSUtils::hlayout();
				if(cv[i] == (int)cv[i]) e = new QLineEdit(s.arg((int)cv[i]));
				else                    e = new QLineEdit(s.arg(cv[i], 0, 'f'));
				l->addWidget(e);
				if(m_coordTypes[i] == RegionShape::PIXEL)
					l->addWidget(new QLabel("px"));
				else if(m_coordTypes[i] == RegionShape::ANGLE)
					l->addWidget(new QLabel("°"));
				layout->addLayout(l, (i / 2), ((i % 2) * 2) + 1);
				m_coordEdits.push_back(e);
			}
		} else {
			layout->addWidget(coordWidget, 0, 0);
		}

		// linethrough color
		m_ltColor = RSUtils::colorWidget(true, "red", ltColorFrame);

		// label color
		m_labelColor = RSUtils::colorWidget(true, "", labelColorFrame);

		// custom options
		vector<String> optNames = shape->optionNames();
		if(optNames.size() == 0) {
			tabWidget->removeTab(tabWidget->indexOf(customTab));
		} else {
			QString title = (shape->oneWordType() + " O&ptions").c_str();
			QStringList split = title.split(QRegExp("\\s+"));
			for(int i = 0; i < split.size(); i++)
				if(split[i].size() > 0) split[i][0] = split[i][0].toUpper();
			title = split.join(" ");
			tabWidget->setTabText(tabWidget->indexOf(customTab), title);

			m_optTypes = shape->optionTypes();
			vector<vector<String> > optChoices = shape->optionChoices();
			vector<RSOption> optVals = shape->optionValues();

			layout = new QGridLayout(optionFrame);
			RSUtils::setupLayout(layout);

			QtRSOption* o;
			unsigned int row = 0;
			bool firstCol = true;
			int colSpan = 1;
			for(unsigned int i = 0; i < optNames.size(); i++) {
				colSpan = 1;
				if(m_optTypes[i] == RegionShape::STRINGARRAY ||
				        m_optTypes[i] == RegionShape::STRINGCHOICEARRAY) {
					if(!firstCol) row++;
					firstCol = true;
					colSpan = 2;
				}
				o = new QtRSOption(m_optTypes[i], optNames[i], optVals[i],
				                   (i < optChoices.size()) ? optChoices[i] : vector<String>());
				RSUtils::setupLayout(o);
				m_optWidgets.push_back(o);
				layout->addLayout(o, row, firstCol ? 0 : 1, 1, colSpan);
				if(m_optTypes[i] == RegionShape::STRINGARRAY ||
				        m_optTypes[i] == RegionShape::STRINGCHOICEARRAY) {
					row++;
				} else if(firstCol) {
					firstCol = false;
				} else {
					row++;
					firstCol = true;
				}
			}
		}

		connect(coordSystem, SIGNAL(currentIndexChanged(int)),
		        SLOT(coordSystemChanged()));
		connect(coordPositionUnit, SIGNAL(currentIndexChanged(int)),
		        SLOT(coordSystemChanged()));
		connect(coordSizeUnit, SIGNAL(currentIndexChanged(int)),
		        SLOT(coordSystemChanged()));
		connect(precisionSpinner, SIGNAL(valueChanged(int)),
		        SLOT(coordSystemChanged()));
		connect(coordResetButton, SIGNAL(clicked()), SLOT(coordReset()));
		connect(ltBox, SIGNAL(toggled(bool)), SLOT(linethrough()));
		connect(ltOverride, SIGNAL(toggled(bool)), SLOT(linethrough()));

		if(!m_creation) {
			connect(closeButton, SIGNAL(clicked()), SLOT(close()));
			setAttribute(Qt::WA_DeleteOnClose, true);
		} else {
			if(applyButtonText.empty()) applyButton->close();
			else applyButton->setText(applyButtonText.c_str());
			closeButton->close();
		}
		connect(applyButton, SIGNAL(clicked()), SLOT(applySlot()));

		if(isText) {
			labelOverride->setChecked(true);
			labelOverride->setVisible(false);
			optionsFrame->setVisible(false);
			optionsLine->setVisible(false);
		}

		setupGui();
	}

	void QtEditRegionShape::setupGui() {
		RegionShape* shape = m_creation ? m_cShape : m_shape->shape();

		// title
		if(!m_creation) titleLabel->setText(shape->title().c_str());
		else            titleLabel->setText(String("new "+shape->type()).c_str());

		// color
		m_lineColor->setColor(shape->lineColor());

		// width
		widthSpinner->setValue((int)(shape->lineWidth() + 0.5));

		// style
		QString style = RegionShape::convLineStyle(shape->lineStyle()).c_str();
		for(int i = 0; i < styleChooser->count(); i++) {
			if(styleChooser->itemText(i) == style) {
				styleChooser->setCurrentIndex(i);
				break;
			}
		}

		// text
		labelEdit->setText(shape->text().c_str());

		// font
		QFont font(shape->label().font().c_str(),
		           (int)(shape->label().size() + 0.5));
		fontChooser->setCurrentFont(font);
		fontSize->setValue(font.pointSize());
		boldBox->setChecked(shape->label().isBold());
		italicBox->setChecked(shape->label().isItalic());
		m_labelColor->setColor(shape->label().color());

		bool isText = dynamic_cast<RSText*>(shape) != NULL;
		if(!isText)
			labelOverride->setChecked(shape->label().color()!= shape->lineColor());

		// linethrough
		ltBox->setChecked(shape->linethrough());
		if(ltBox->isChecked()) {
			m_ltColor->setColor(shape->linethroughColor());

			ltWidth->setValue((int)(shape->linethroughWidth() + 0.5));

			style = RegionShape::convLineStyle(shape->linethroughStyle()).c_str();
			for(int i = 0; i < ltStyleChooser->count(); i++) {
				if(ltStyleChooser->itemText(i) == style) {
					ltStyleChooser->setCurrentIndex(i);
					break;
				}
			}
			ltOverride->setChecked(shape->linethroughColor()!= shape->lineColor()||
			                       shape->linethroughWidth() != shape->lineWidth() ||
			                       shape->linethroughStyle() != shape->lineStyle());
		} else {
			m_ltColor->setColor("red");
			ltWidth->setValue(1);
			ltStyleChooser->setCurrentIndex(0);
		}
	}

	double QtEditRegionShape::convertToRS(int index, bool& ok) const {
		bool isPixel = coordSystem->currentIndex() == coordSystem->count() - 1;
		RSValue from, to;
		if(!isPixel && m_coordTypes[index] == RegionShape::POSITION) {
			String p = coordPositionUnit->currentText().toStdString();
			// sexagesimal: HMS for even, DMS for odd
			if(p == SEXAGESIMAL) {
				if(index % 2 == 0) p = RSValue::HMS;
				else               p = RSValue::DMS;
			}
			ok = RSValue::convertBetween(m_coordEdits[index]->text(), p, from) &&
			     RSValue::convertBetween(from, to, RegionShape::UNIT);
			return to.val.getValue();
		} else if(!isPixel && m_coordTypes[index] == RegionShape::SIZE) {
			ok = RSValue::convertBetween(m_coordEdits[index]->text(),
			                             coordSizeUnit->currentText().toStdString(), from) &&
			     RSValue::convertBetween(from, to, RegionShape::UNIT);
			return to.val.getValue();
		} else {
			return m_coordEdits[index]->text().toDouble(&ok);
		}
	}

	void QtEditRegionShape::displayCoordinates(const vector<double>& coords) {
		bool isPixel = coordSystem->currentIndex() == coordSystem->count() - 1;
		String p = coordPositionUnit->currentText().toStdString(),
		       s = coordSizeUnit->currentText().toStdString(),
		       p2 = p;
		QString qstr;
		RSValue from(0, RegionShape::UNIT), to;
		for(unsigned int i = 0; i < m_coordEdits.size(); i++) {
			from.val.setValue(coords[i]);
			if(!isPixel && (m_coordTypes[i] == RegionShape::POSITION ||
			                m_coordTypes[i] == RegionShape::SIZE)) {
				if(m_coordTypes[i] == RegionShape::POSITION) {
					// sexagesimal: HMS for even, DMS for odd
					if(p == SEXAGESIMAL) {
						if(i % 2 == 0) p2 = RSValue::HMS;
						else           p2 = RSValue::DMS;
					}
					RSValue::convertBetween(from, to, p2);
				} else {
					RSValue::convertBetween(from, to, s);
				}
				RSValue::convertBetween(to, qstr, precisionSpinner->value());
			} else {
				qstr = QString("%1").arg(coords[i], 0, 'f',
				                         precisionSpinner->value());
			}
			m_coordEdits[i]->setText(qstr);
		}
	}


	void QtEditRegionShape::coordSystemChanged() {
		bool isPixel = coordSystem->currentIndex() == coordSystem->count() - 1,
		     wasPixel = m_lastSystem == RSUtils::PIXEL;
		coordPositionUnit->setEnabled(!isPixel);
		coordSizeUnit->setEnabled(!isPixel);
		positionLabel->setEnabled(!isPixel);
		sizeLabel->setEnabled(!isPixel);

		if(m_creation) return;

		String p = m_lastPosUnit;
		RSValue from, to;
		m_manager->panel()->hold();
		vector<double> oldCoords = m_shape->shape()->coordParameterValues(),
		               newCoords;

		// preserve any entered changes -- note: this is not the best/most
		// efficient way of doing this, but it seems to work reasonably, so..

		// get the entered coordinates in the previous system/units
		for(unsigned int i = 0; i < m_coordEdits.size(); i++) {
			if(!wasPixel && (m_coordTypes[i] == RegionShape::POSITION ||
			                 m_coordTypes[i] == RegionShape::SIZE)) {
				if(m_coordTypes[i] == RegionShape::POSITION) {
					// sexagesimal: HMS for even, DMS for odd
					if(m_lastPosUnit == SEXAGESIMAL) {
						if(i % 2 == 0) p = RSValue::HMS;
						else           p = RSValue::DMS;
					}
					RSValue::convertBetween(m_coordEdits[i]->text(), p, from);
				} else {
					RSValue::convertBetween(m_coordEdits[i]->text(),
					                        m_lastSizeUnit, from);
				}
				RSValue::convertBetween(from, to, RegionShape::UNIT);
			} else {
				RSValue::convertBetween(m_coordEdits[i]->text(), RSValue::DEG, to);
			}
			newCoords.push_back(to.val.getValue());
		}

		// temporarily set the shape's coordinates to these coordinates and then
		// get these values in the new system
		m_shape->shape()->setCoordParameters(newCoords, m_lastSystem);
		String t = coordSystem->currentText().toStdString();
		newCoords = m_shape->shape()->coordParameterValues(t);

		// update the text edits accordingly
		displayCoordinates(newCoords);

		// restore original coords and release panel
		m_shape->shape()->setCoordParameters(oldCoords);
		m_manager->panel()->release();

		m_lastSystem = coordSystem->currentText().toStdString();
		m_lastPosUnit = coordPositionUnit->currentText().toStdString();
		m_lastSizeUnit = coordSizeUnit->currentText().toStdString();
	}

	void QtEditRegionShape::coordReset() {
		if(m_creation) return;

		coordSystem->blockSignals(true);
		coordPositionUnit->blockSignals(true);
		coordSizeUnit->blockSignals(true);
		if(!m_shape->shape()->isWorld()) {
			coordSystem->setCurrentIndex(coordSystem->count() - 1);
		} else {
			QString sys  = MDirection::showType(
			                   m_shape->shape()->worldSystem()).c_str(),
			               unit = RegionShape::UNIT.c_str();
			if(coordSystem->isVisible()) {
				for(int i = 0; i < coordSystem->count() - 1; i++) {
					if(coordSystem->itemText(i) == sys) {
						coordSystem->setCurrentIndex(i);
						break;
					}
				}
			}
			if(coordPositionUnit->isVisible()) {
				for(int i = 0; i < coordPositionUnit->count() - 1; i++) {
					if(coordPositionUnit->itemText(i) == unit) {
						coordPositionUnit->setCurrentIndex(i);
						break;
					}
				}
			}
			if(coordSizeUnit->isVisible()) {
				for(int i = 0; i < coordSizeUnit->count() - 1; i++) {
					if(coordSizeUnit->itemText(i) == unit) {
						coordSizeUnit->setCurrentIndex(i);
						break;
					}
				}
			}
		}

		coordSystemChanged();
		displayCoordinates(m_shape->shape()->coordParameterValues());
		coordSystem->blockSignals(false);
		coordPositionUnit->blockSignals(false);
		coordSizeUnit->blockSignals(false);
	}

	void QtEditRegionShape::linethrough() {
		ltFrame->setEnabled(ltBox->isChecked() && ltOverride->isChecked());
	}


// Static Members //

	const String QtEditRegionShape::SEXAGESIMAL = "sexagesimal";
	const String QtEditRegionShape::UNKNOWN     = "UNKNOWN";

}
