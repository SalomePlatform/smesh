//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org
//
//
//
//  File   : SMESHGUI_FilterLibraryDlg.cxx
//  Author : Sergey LITONIN
//  Module : SMESH

#include "SMESHGUI_FilterLibraryDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_FilterUtils.h"

#include "SUIT_Session.h"
#include "SUIT_Desktop.h"
#include "SUIT_FileDlg.h"

// QT Includes
#include <qapplication.h>
#include <qmessagebox.h>
#include <qframe.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>

#define SPACING 5
#define MARGIN  10

/*!
 *  Class       : SMESHGUI_FilterLibraryDlg::Dialog
 *  Description : Dialog for opening filter library
 */

class SMESHGUI_FilterLibraryDlg::Dialog : public SUIT_FileDlg
{
 public:
  Dialog(QWidget* theParent, const bool theToOpen);
  virtual ~Dialog();

 protected:
  virtual bool acceptData();
};

SMESHGUI_FilterLibraryDlg::Dialog::Dialog (QWidget*   theParent,
                                           const bool theToOpen)
     : SUIT_FileDlg(theParent, theToOpen)
{
}

SMESHGUI_FilterLibraryDlg::Dialog::~Dialog()
{
}

bool SMESHGUI_FilterLibraryDlg::Dialog::acceptData()
{
//  if (mode() != QFileDialogP::AnyFile)
//    return SUIT_FileDlg::acceptData();

  return true;
}

/*!
 *  Class       : SMESHGUI_FilterLibraryDlg
 *  Description : Dialog to specify filters for VTK viewer
 */

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::SMESHGUI_FilterLibraryDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_FilterLibraryDlg::SMESHGUI_FilterLibraryDlg (SMESHGUI* theModule,
                                                      QWidget* parent,
                                                      const QValueList<int>& theTypes,
                                                      const int              theMode,
                                                      const char*            theName)
     : QDialog( parent, theName, true, WStyle_Customize |
                WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu ),
     mySMESHGUI( theModule )
{
  construct(theTypes, theMode);
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::SMESHGUI_FilterLibraryDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_FilterLibraryDlg::SMESHGUI_FilterLibraryDlg (SMESHGUI* theModule,
                                                      QWidget* parent,
                                                      const int   theType,
                                                      const int   theMode,
                                                      const char* theName)
     : QDialog( parent, theName, true, WStyle_Customize |
                WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu ),
     mySMESHGUI( theModule )
{
  QValueList<int> aTypes;
  aTypes.append(theType);
  construct(aTypes, theMode);
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::construct
// Purpose : Construct dialog (called by constructor)
//=======================================================================
void SMESHGUI_FilterLibraryDlg::construct (const QValueList<int>& theTypes,
                                           const int theMode)
{
  myTypes = theTypes;
  myMode  = theMode;

  QVBoxLayout* aDlgLay = new QVBoxLayout(this, MARGIN, SPACING);

  myMainFrame        = createMainFrame  (this);
  QFrame* aBtnFrame  = createButtonFrame(this);

  aDlgLay->addWidget(myMainFrame);
  aDlgLay->addWidget(aBtnFrame);

  aDlgLay->setStretchFactor(myMainFrame, 1);

  Init(myTypes, myMode);
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::createMainFrame
// Purpose : Create frame containing dialog's input fields
//=======================================================================
QFrame* SMESHGUI_FilterLibraryDlg::createMainFrame (QWidget* theParent)
{
  QGroupBox* aMainFrame = new QGroupBox(1, Qt::Horizontal, theParent);
  aMainFrame->setFrameStyle(QFrame::NoFrame);
  aMainFrame->setInsideMargin(0);

  // library name

  QGroupBox* aGrp = new QGroupBox(1, Qt::Vertical, aMainFrame);
  aGrp->setFrameStyle(QFrame::NoFrame);
  aGrp->setInsideMargin(0);

  new QLabel(tr("LIBRARY_FILE"), aGrp);
  myFileName = new QLineEdit(aGrp);
  myOpenBtn = new QPushButton(aGrp);
  myOpenBtn->setPixmap(SUIT_Session::session()->resourceMgr()->loadPixmap(
    "SUIT", tr("ICON_FILE_OPEN")));

  // filters list box

  aGrp = new QGroupBox(1, Qt::Vertical, tr("FILTER_NAMES"), aMainFrame);
  QFrame* aFrame = new QFrame(aGrp);
  myListBox = new QListBox(aFrame);
  myAddBtn = new QPushButton(tr("ADD"), aFrame);
  myDeleteBtn = new QPushButton(tr("DELETE"), aFrame);

  QGridLayout* aLay = new QGridLayout(aFrame, 3, 2, 0, 5);
  aLay->addMultiCellWidget(myListBox, 0, 2, 0, 0);
  aLay->addWidget(myAddBtn, 0, 1);
  aLay->addWidget(myDeleteBtn, 1, 1);
  QSpacerItem* aVSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  aLay->addItem(aVSpacer, 2, 1);

  // filter name

  myNameGrp = new QGroupBox(1, Qt::Vertical, aMainFrame);
  myNameGrp->setFrameStyle(QFrame::NoFrame);
  myNameGrp->setInsideMargin(0);
  new QLabel(tr("FILTER_NAME"), myNameGrp);
  myName = new QLineEdit(myNameGrp);

  // table

  myTable = new SMESHGUI_FilterTable( mySMESHGUI, aMainFrame, myTypes);
  myTable->SetEditable(myMode == EDIT);
  myTable->SetLibsEnabled(false);

  myListBox->setMinimumHeight((int)(myTable->sizeHint().height() * 0.5));
  myListBox->setRowMode(QListBox::FitToWidth);
  myListBox->setSelectionMode(QListBox::Single);

  myOpenBtn->setAutoDefault(false);
  myAddBtn->setAutoDefault(false);
  myDeleteBtn->setAutoDefault(false);

  // connect signals and slots

  connect(myFileName, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
  connect(myOpenBtn , SIGNAL(clicked()), this, SLOT(onBrowse()));

  connect(myListBox, SIGNAL(highlighted(const QString&)),
           this, SLOT(onFilterChanged(const QString&)));

  connect(myAddBtn, SIGNAL(clicked()), this, SLOT(onAddBtnPressed()));
  connect(myDeleteBtn, SIGNAL(clicked()), this, SLOT(onDeleteBtnPressed()));

  connect(myName, SIGNAL(textChanged(const QString&)),
           this, SLOT(onFilterNameChanged(const QString&)));

  connect(myTable, SIGNAL(EntityTypeChanged(const int)),
           this, SLOT(onEntityTypeChanged(const int)));

  connect(myTable, SIGNAL(NeedValidation()), this, SLOT(onNeedValidation()));

  return aMainFrame;
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::createButtonFrame
// Purpose : Create frame containing buttons
//=======================================================================
QFrame* SMESHGUI_FilterLibraryDlg::createButtonFrame (QWidget* theParent)
{
  QGroupBox* aGrp = new QGroupBox(1, Qt::Vertical, theParent);

  myButtons[ BTN_OK    ] = new QPushButton(tr("SMESH_BUT_OK"   ), aGrp);
  myButtons[ BTN_Apply ] = new QPushButton(tr("SMESH_BUT_APPLY"), aGrp);

  QLabel* aLbl = new QLabel(aGrp);
  aLbl->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

  myButtons[ BTN_Cancel ] = new QPushButton(tr("SMESH_BUT_CANCEL"), aGrp);
  myButtons[ BTN_Close  ] = new QPushButton(tr("SMESH_BUT_CLOSE"), aGrp);

  connect(myButtons[ BTN_OK     ], SIGNAL(clicked()), SLOT(onOk()));
  connect(myButtons[ BTN_Cancel ], SIGNAL(clicked()), SLOT(onClose()));
  connect(myButtons[ BTN_Close  ], SIGNAL(clicked()), SLOT(onClose()));
  connect(myButtons[ BTN_Apply  ], SIGNAL(clicked()), SLOT(onApply()));

  QMap<int, QPushButton*>::iterator anIter;
  for (anIter = myButtons.begin(); anIter != myButtons.end(); ++anIter)
    anIter.data()->setAutoDefault(false);

  updateMainButtons();

  return aGrp;
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::updateMainButtons
// Purpose : Update visibility of main buttons (OK, Cancel, Close ...)
//=======================================================================
void SMESHGUI_FilterLibraryDlg::updateMainButtons()
{
  if (myTypes.count() == 1) {
    myButtons[ BTN_Cancel ]->show();
    myButtons[ BTN_Apply  ]->hide();
    myButtons[ BTN_Close  ]->hide();
  } else {
    myButtons[ BTN_Cancel ]->hide();
    myButtons[ BTN_Apply  ]->show();
    myButtons[ BTN_Close  ]->show();
  }
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::~SMESHGUI_FilterLibraryDlg
// Purpose : Destructor
//=======================================================================
SMESHGUI_FilterLibraryDlg::~SMESHGUI_FilterLibraryDlg()
{
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::Init
// Purpose : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_FilterLibraryDlg::Init (const int type, const int theMode)
{
  QValueList<int> aTypes;
  aTypes.append(type);
  Init(aTypes, theMode);
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::Init
// Purpose : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_FilterLibraryDlg::Init (const QValueList<int>& theTypes,
                                      const int theMode)
{
  myMode = theMode;
  myTypes = theTypes;
  myTable->Init(theTypes);
  myCurrFilterName = "";
  myCurrFilter = -1;
  myListBox->clear();
  myName->clear();
  myTable->Clear();

  updateControlsVisibility();
  setEnabled(true);

  connect( mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), SLOT(onDeactivate()));
  connect( mySMESHGUI, SIGNAL(SignalCloseAllDialogs()), SLOT(onClose()));

  if (myMode == ADD_TO)
  {
    setCaption(tr("ADD_TO_TLT"));
    if (myFileName->text().isEmpty())
      myFileName->setText(getDefaultLibraryName());
    processNewLibrary();
  }
  else if (myMode == COPY_FROM)
  {
    setCaption(tr("COPY_FROM_TLT"));
    if (myFileName->text().isEmpty())
      myFileName->setText(getDefaultLibraryName());
    processNewLibrary();
    if (myListBox->count() > 0)
      myListBox->setCurrentItem(0);
  }
  else
  {
    setCaption(tr("EDIT_LIB_TLT"));
    if (myFileName->text().isEmpty())
      myFileName->setText(getDefaultLibraryName());
    processNewLibrary();
    if (myListBox->count() > 0)
      myListBox->setCurrentItem(0);
  }

  this->show();

  updateMainButtons();
  isPermissionValid(false);
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::updateControlsVisibility
// Purpose : Update visibility of controls in accordance with myMode
//=======================================================================
void SMESHGUI_FilterLibraryDlg::updateControlsVisibility()
{
  if (myMode == ADD_TO)
  {
    myNameGrp->show();
    myNameGrp->setEnabled(true);
    myAddBtn->hide();
    myDeleteBtn->hide();
    myTable->SetEditable(false);
  }
  else if (myMode == COPY_FROM)
  {
    myNameGrp->hide();
    myNameGrp->setEnabled(false);
    myAddBtn->hide();
    myDeleteBtn->hide();
    myTable->SetEditable(false);
  }
  else if (myMode == EDIT)
  {
    myNameGrp->show();
    myNameGrp->setEnabled(true);
    myAddBtn->show();
    myDeleteBtn->show();
    myTable->SetEditable(true);
  }

  qApp->processEvents();
  updateGeometry();
  adjustSize();
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::onApply
// Purpose : SLOT called when "Apply" button pressed.
//=======================================================================
bool SMESHGUI_FilterLibraryDlg::onApply()
{
  if (!isValid(true) || !isPermissionValid(false))
    return false;

  if (myLibrary->_is_nil()) {
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_WRN_WARNING"),
                             tr("LIBRARY_IS_NOT_LOADED"), QMessageBox::Ok);
    return false;
  }

  const char* aName = myFileName->text().latin1();
  if (strcmp(myLibrary->GetFileName(), aName) != 0)
    myLibrary->SetFileName(aName);

  bool aResult = false;

  if (myMode == COPY_FROM || myListBox->count() == 0) {
    aResult = true;
  } else if (myMode == EDIT || myMode == ADD_TO) {
    SMESH::Filter_var aFilter = createFilter();
    if (!myLibrary->Replace(myCurrFilterName, myName->text(), aFilter.in())) {
      QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                               tr("ERROR_OF_EDITING"), QMessageBox::Ok);
      aResult = false;
    }
    else
      aResult = true;
  }

  if (aResult && myMode != COPY_FROM)
    aResult = myLibrary->Save();

  if (aResult) {
    char* aFileName = myLibrary->GetFileName();
    getDefaultLibraryName() = QString(aFileName);
    delete aFileName;
  } else if (myMode != COPY_FROM) {
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                             tr("ERROR_OF_SAVING"), QMessageBox::Ok);
  } else {
  }

  return aResult;
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::onOk
// Purpose : SLOT called when "Ok" button pressed.
//           Assign filters VTK viewer and close dialog
//=======================================================================
void SMESHGUI_FilterLibraryDlg::onOk()
{
  if (onApply())
  {
    disconnect( mySMESHGUI, 0, this, 0);
    mySMESHGUI->ResetState();
    accept();
  }
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::onClose
// Purpose : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_FilterLibraryDlg::onClose()
{
  disconnect( mySMESHGUI, 0, this, 0);
  mySMESHGUI->ResetState();
  reject();
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::onDeactivate
// Purpose : SLOT called when dialog must be deativated
//=======================================================================
void SMESHGUI_FilterLibraryDlg::onDeactivate()
{
  setEnabled(false);
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::enterEvent
// Purpose : Event filter
//=======================================================================
void SMESHGUI_FilterLibraryDlg::enterEvent(QEvent*)
{
  setEnabled(true);
}

//=================================================================================
// function : closeEvent()
// purpose  : Close dialog
//=================================================================================
void SMESHGUI_FilterLibraryDlg::closeEvent(QCloseEvent* e)
{
  onClose();
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::getFileName
// Purpose : Get file name
//=======================================================================
QString SMESHGUI_FilterLibraryDlg::getFileName() const
{
  return myFileName != 0 ? myFileName->text() : QString("");
}

//================================================================
// Function : setFileName
// Purpose  : Set file name to line edit
//================================================================
void SMESHGUI_FilterLibraryDlg::setFileName(const QString& txt, const bool autoExt)
{
  if (myFileName == 0)
    return;
  myFileName->setText(autoExt ? autoExtension(txt) : txt);
}

//================================================================
// Function : autoExtension
// Purpose  : Append extension to the file name
//================================================================
QString SMESHGUI_FilterLibraryDlg::autoExtension(const QString& theFileName) const
{
  QString anExt = theFileName.section('.', -1);
  return anExt != "xml" && anExt != "XML" ? theFileName + ".xml" : theFileName;
}

//================================================================
// Function : filterWildCards
// Purpose  :
//================================================================
QStringList SMESHGUI_FilterLibraryDlg::filterWildCards(const QString& theFilter) const
{
  QStringList res;

  int b = theFilter.findRev("(");
  int e = theFilter.findRev(")");
  if (b != -1 && e != -1)
  {
    QString content = theFilter.mid(b + 1, e - b - 1).stripWhiteSpace();
    QStringList lst = QStringList::split(" ", content);
    for (QStringList::const_iterator it = lst.begin(); it != lst.end(); ++it)
      if ((*it).find(".") != -1)
        res.append((*it).stripWhiteSpace());
  }
  return res;
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::prepareFilters
// Purpose : Prepare filters for dialog
//=======================================================================
QStringList SMESHGUI_FilterLibraryDlg::prepareFilters() const
{
  static QStringList aList;
  if (aList.isEmpty())
  {
    aList.append(tr("XML_FILT"));
    //aList.append(tr("ALL_FILES_FILTER"));
  }

  return aList;
}

//================================================================
// Function : onBrowse
// Purpose  : SLOT. Display "Open file" dialog for chosing library name
//================================================================
void SMESHGUI_FilterLibraryDlg::onBrowse()
{
  Dialog* aDlg = new Dialog(this, true);
  aDlg->setCaption(tr("OPEN_LIBRARY"));

  //aDlg->setMode(myMode == COPY_FROM ? QFileDialogP::ExistingFile : QFileDialogP::AnyFile);
  aDlg->setMode(myMode == COPY_FROM ? QFileDialog::ExistingFile : QFileDialog::AnyFile);
  aDlg->setFilters(prepareFilters());
  aDlg->setSelection(getFileName());

  QPushButton* anOkBtn = (QPushButton*)aDlg->child("OK", "QPushButton");
  if (anOkBtn != 0)
    anOkBtn->setText(tr("SMESH_BUT_OK"));

  if (aDlg->exec() != Accepted)
    return;

  QString fName = aDlg->selectedFile();

  if (fName.isEmpty())
    return;

  if (QFileInfo(fName).extension().isEmpty())
    fName = autoExtension(fName);

  fName = QDir::convertSeparators(fName);
  QString prev = QDir::convertSeparators(getFileName());

  if (prev == fName)
    return;

  setFileName(fName);

  QString aName = myListBox->text(myListBox->count() - 1);
  processNewLibrary();

  if (myMode == ADD_TO)
  {
    myTable->Copy((SMESHGUI_FilterTable*)parentWidget());
    myCurrFilterName = "";
    myCurrFilter = -1;
    addFilterToLib(aName);
  }

  isPermissionValid(false);
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::processNewLibrary
// Purpose : SLOT. Calleds when file name changed
//=======================================================================
void SMESHGUI_FilterLibraryDlg::processNewLibrary()
{
  SMESH::FilterManager_var aFilterMgr = SMESH::GetFilterManager();
  if (aFilterMgr->_is_nil())
    return;

  myLibrary = aFilterMgr->LoadLibrary(autoExtension(getFileName()));
  if (myLibrary->_is_nil()) {
    if (myMode == COPY_FROM) {
      QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                               tr("ERROR_LOAD"), QMessageBox::Ok);
      return;
    } else {
      myLibrary = aFilterMgr->CreateLibrary();
      myLibrary->SetFileName(getFileName().latin1());
    }
  }

  updateList();
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::updateList
// Purpose : Fill list box with filter names
//=======================================================================
void SMESHGUI_FilterLibraryDlg::updateList()
{
  QStringList aList;
  SMESH::string_array_var aNames = myLibrary->GetNames((SMESH::ElementType)myTable->GetType());
  for (int i = 0, n = aNames->length(); i < n; i++)
    aList.append(QString(aNames[ i ]));
  myListBox->clear();
  myListBox->insertStringList(aList);
  if (myListBox->count() == 0)
  {
    myTable->Clear(myTable->GetType());
    myName->clear();
    myName->setEnabled(false);
    myTable->SetEnabled(false);
  }
  else
  {
    myName->setEnabled(true);
    myTable->SetEnabled(true);
    if (myListBox->count())
    {
      myCurrFilterName = "";
      myListBox->setCurrentItem(0);
    }
  }
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::isNameValid
// Purpose : Verify validity if entered data
//=======================================================================
bool SMESHGUI_FilterLibraryDlg::isNameValid(const bool theMess) const
{
  // verify validity of filter name
  if (myName->isEnabled() && !myCurrFilterName.isEmpty()) {
    QString aCurrName = myName->text();
    if (aCurrName.isEmpty()) {
      if (theMess)
        QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                                 tr("EMPTY_FILTER_NAME"), QMessageBox::Ok);
      return false;
    }

    SMESH::string_array_var aNames = myLibrary->GetAllNames();
    for (int f = 0, n = aNames->length(); f < n; f++) {
      if (aNames[ f ] == aCurrName && aNames[ f ] != myCurrFilterName) {
        if (theMess)
          QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                                   tr("ERROR_FILTER_NAME"), QMessageBox::Ok);
        return false;
      }
    }
  }

  return true;
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::isPermissionValid
// Purpose : Verify write permission on file
//=======================================================================
bool SMESHGUI_FilterLibraryDlg::isPermissionValid(const bool theIsExistingOnly)
{
  if (myMode == COPY_FROM)
    return true;

  // Verify write permission
  bool isWritable = false;

  QString fName(myFileName->text());
  if (QFileInfo(fName).extension().isEmpty())
    fName = autoExtension(fName);

  fName = QDir::convertSeparators(fName);

  if (QFileInfo(fName).exists()) {
    isWritable = QFileInfo(fName).isWritable();
  } else if (!theIsExistingOnly) {
    QFileInfo aDirInfo(QFileInfo(fName).dirPath(true));
    isWritable = aDirInfo.isWritable();
    /*if (QDir(QFileInfo(fName).dirPath(true)).exists() ||
         QDir().mkdir(QFileInfo(fName).dirPath(true)))
    {
      QFile aFile(fName);
      if (aFile.open(IO_WriteOnly))
        isWritable = true;
      else
        aFile.close();
    }
    */
  } else {
    isWritable = true;
  }

  if (!isWritable) {
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_WRN_WARNING"),
                             tr("NO_PERMISSION"), QMessageBox::Ok);
    return false;
  }

  return true;
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::isValid
// Purpose : Verify validity if entered data
//=======================================================================
bool SMESHGUI_FilterLibraryDlg::isValid(const bool theMess) const
{
  // verify validity of table
  if (!myTable->IsValid(theMess) || !isNameValid(theMess))
    return false;
  else
    return true;
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::onFilterChanged
// Purpose : SLOT. Called when selected filter of library  changed
//=======================================================================
void SMESHGUI_FilterLibraryDlg::onFilterChanged(const QString& theName)
{
  if (myLibrary->_is_nil())
    return;

  // Save parameters of filter if it was changed

  if (!myCurrFilterName.isEmpty() && myTable->IsEditable())
  {
    if (!isValid(true))
    {
      myListBox->blockSignals(true);
      myListBox->setCurrentItem(myCurrFilter);
      myListBox->blockSignals(false);
      return;
    }

    SMESH::Filter_var aFilter = createFilter();
    myLibrary->Replace(myCurrFilterName.latin1(), myName->text().latin1(), aFilter);
  }

  // Fill table with filter parameters

  SMESH::Filter_var aFilter = myLibrary->Copy(theName);
  myCurrFilterName = theName;
  myCurrFilter = myListBox->currentItem();
  myName->setText(theName);


  SMESH::Filter::Criteria_var aCriteria;

  myTable->Clear(myTable->GetType());

  if (!aFilter->GetCriteria(aCriteria))
    return;

  for (int i = 0, n = aCriteria->length(); i < n; i++)
    myTable->AddCriterion(aCriteria[ i ], myTable->GetType());

  myTable->Update();
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::onReturnPressed
// Purpose : SLOT. Called when enter button is pressed in library name field
//           Reload library
//=======================================================================
void SMESHGUI_FilterLibraryDlg::onReturnPressed()
{
  QString aName = myListBox->text(myListBox->count() - 1);

  processNewLibrary();

  if (myMode == ADD_TO)
  {
    myTable->Copy((SMESHGUI_FilterTable*)parentWidget());
    myCurrFilterName = "";
    myCurrFilter = -1;
    addFilterToLib(aName);
  }

  isPermissionValid(false);
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::enableMainButtons
// Purpose : Update state of "OK", "Cancel" buttons
//=======================================================================
void SMESHGUI_FilterLibraryDlg::enableMainButtons()
{
  /*bool isEnabled = isValid(false);
  if (myButtons.contains(BTN_OK))
    myButtons[ BTN_OK ]->setEnabled(isEnabled);
  else if (myButtons.contains(BTN_Apply))
    myButtons[ BTN_OK ]->setEnabled(isEnabled);
  if (myButtons.contains(BTN_Cancel))
    myButtons[ BTN_Cancel ]->setEnabled(isEnabled);
  else if (myButtons.contains(BTN_Close))
    myButtons[ BTN_Cancel ]->setEnabled(isEnabled);
    */
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::createFilter
// Purpose : Cerate filter in accordance with library
//=======================================================================
SMESH::Filter_ptr SMESHGUI_FilterLibraryDlg::createFilter(const int theType)
{
  int n = myTable->NumRows(theType);

  SMESH::Filter::Criteria_var aCriteria = new SMESH::Filter::Criteria;
  aCriteria->length(n);

  for (int i = 0; i < n; i++)
  {
    SMESH::Filter::Criterion aCriterion = SMESHGUI_FilterDlg::createCriterion();
    myTable->GetCriterion(i, aCriterion);
    aCriteria[ i ] = aCriterion;
  }

  SMESH::FilterManager_var aFilterMgr = SMESH::GetFilterManager();
  SMESH::Filter_var aFilter = aFilterMgr->CreateFilter();
  aFilter->SetCriteria(aCriteria.in());

  return aFilter._retn();
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::onAddBtnPressed
// Purpose : SLOT. Called when "Add" button pressed
//           Add new filter to the end of library
//=======================================================================
void SMESHGUI_FilterLibraryDlg::onAddBtnPressed()
{
  // Save parameters of filter if it was changed
  if (!myCurrFilterName.isEmpty() && myTable->IsEditable())
  {
    if (!isValid(true))
      return;

    SMESH::Filter_var aFilter = createFilter();
    myLibrary->Replace(myCurrFilterName.latin1(), myName->text().latin1(), aFilter);
  }

  addFilterToLib(getDefaultFilterName());
}

//=======================================================================
// name    : onAddBtnPressed()
// Purpose : SLOT. Called when "Add" button pressed
//           Add new filter to the end of library
//=======================================================================
void SMESHGUI_FilterLibraryDlg::addFilterToLib (const QString& theName)
{
  if (myLibrary->_is_nil()) {
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_WRN_WARNING"),
                             tr("LIBRARY_IS_NOT_LOADED"), QMessageBox::Ok);
    return;
  }

  // create filter
  SMESH::Filter_var aFilter = createFilter();

  // if name of filter already exist in the library assign default name for the filter
  QString aName(theName);
  SMESH::string_array_var aNames = myLibrary->GetAllNames();
  for (int i = 0, n = aNames->length(); i < n; i++)
    if (aName == QString(aNames[ i ]))
    {
      aName = getDefaultFilterName();
      break;
    }

  // add new filter in library
  bool aResult = !aFilter->GetPredicate()->_is_nil()
    ? myLibrary->Add(aName.latin1(), aFilter)
    : myLibrary->AddEmpty(aName.latin1(), (SMESH::ElementType)myTable->GetType());

  if (!aResult) {
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                             tr("ERROR_OF_ADDING"), QMessageBox::Ok);
  }

  updateList();
  myCurrFilterName = "";
  myCurrFilter = -1;
  setSelected(aName);

  if (theName != aName)
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_WARNING"),
                             tr("ASSIGN_NEW_NAME").arg(theName).arg(aName), QMessageBox::Ok);
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::getDefaultLibraryName
// Purpose : Get default library name
//=======================================================================
QString& SMESHGUI_FilterLibraryDlg::getDefaultLibraryName() const
{
  static QString aName;
  if (aName.isEmpty())
  {
    QString aHomeDir = QDir(QDir::home()).absPath();
    aName = aHomeDir + "/" + tr ("LIB_NAME");
  }
  return aName;
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::getDefaultFilterName
// Purpose : Create default filter name
//=======================================================================
QString SMESHGUI_FilterLibraryDlg::getDefaultFilterName() const
{
  QString aName;

  if (myTable->GetType() == SMESH::NODE)
    aName = tr("NODE");
  else if (myTable->GetType() == SMESH::EDGE)
    aName = tr("EDGE");
  else if (myTable->GetType() == SMESH::FACE)
    aName = tr("FACE");
  else if (myTable->GetType() == SMESH::VOLUME)
    aName = tr("VOLUME");
  else if (myTable->GetType() == SMESH::ALL)
    aName = tr("ELEMENT");
  else
    aName = tr("SELECTION");

  aName += tr("FILTER");


  QMap< QString, int > anAllNames;
  SMESH::string_array_var aNames = myLibrary->GetAllNames();
  for(int i = 0, n = aNames->length(); i < n; i++)
    anAllNames[ QString(aNames[ i ]) ] = -1;

  bool isNotValid = true;
  int k = 1;
  QString aNewName;
  while (isNotValid)
  {
    isNotValid = false;
    aNewName = aName + "_" + QString("%1").arg(k);
    if (anAllNames.contains(aNewName))
    {
      isNotValid = true;
      k++;
    }
  }

  return aNewName;
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::setSelected
// Purpose : set selected item in list box containing filters
//=======================================================================
bool SMESHGUI_FilterLibraryDlg::setSelected(const QString& theName)
{
  int anIndex = getIndex(theName);
  if (anIndex != -1)
  {
    myListBox->setCurrentItem(anIndex);
    myCurrFilterName = theName;
    myCurrFilter = anIndex;
  }
  return anIndex != -1;
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::getIndex
// Purpose : Get index of the filter in list box
//=======================================================================
int SMESHGUI_FilterLibraryDlg::getIndex(const QString& theName) const
{
  for (int i = 0, n = myListBox->count(); i < n; i++)
    if (myListBox->text(i) == theName)
      return i;
  return -1;
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::onDeleteBtnPressed
// Purpose : SLOT. Called when "Delete" button pressed
//=======================================================================
void SMESHGUI_FilterLibraryDlg::onDeleteBtnPressed()
{
  if (myLibrary->_is_nil()) {
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_WRN_WARNING"),
                             tr("LIBRARY_IS_NOT_LOADED"), QMessageBox::Ok);
    return;
  }

  int anIndex = getIndex(myCurrFilterName);

  if (anIndex == -1 || !myLibrary->Delete(myCurrFilterName.latin1())) {
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                             tr("ERROR_OF_DELETING"), QMessageBox::Ok);
  } else {
    myCurrFilterName = "";
    myCurrFilter = -1;
    myListBox->removeItem(anIndex);

    if (anIndex >= 1)
      myListBox->setSelected(anIndex - 1, true);
    else if (anIndex == 0 && myListBox->count() > 0)
      myListBox->setSelected(0, true);
    else
      myTable->Clear();
  }

  myTable->SetEnabled(myListBox->count() > 0);
  if (myListBox->count() == 0) {
    myName->setText("");
    myName->setEnabled(false);
  }
}

//=======================================================================
// name    : onFilterNameChanged()
// Purpose : SLOT. Called when name of filter changed
//           Change filter name in list box
//=======================================================================
void SMESHGUI_FilterLibraryDlg::onFilterNameChanged (const QString& theName)
{
  int aCurrItem = myListBox->currentItem();
  if (aCurrItem == -1)
    return;

  myListBox->blockSignals(true);
  myListBox->changeItem(theName, aCurrItem);
  myListBox->blockSignals(false);
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::SetTable
// Purpose : Set table
//=======================================================================
void SMESHGUI_FilterLibraryDlg::SetTable(const SMESHGUI_FilterTable* theTable)
{
  myTable->Copy(theTable);
  myName->setText(getDefaultFilterName());
  addFilterToLib(myName->text());
  myTable->Update();
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::GetTable
// Purpose : Get table
//=======================================================================
const SMESHGUI_FilterTable* SMESHGUI_FilterLibraryDlg::GetTable() const
{
  return myTable;
}


//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::onEntityTypeChanged
// Purpose : SLOT. Called when entiyt type changed
//=======================================================================
void SMESHGUI_FilterLibraryDlg::onEntityTypeChanged(const int theType)
{
  if (myLibrary->_is_nil())
    return;

  myName->clear();
  myCurrFilterName = "";
  myCurrFilter = -1;
  updateList();
  if (myListBox->count())
    myListBox->setCurrentItem(0);
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::onNeedValidation
// Purpose :
//=======================================================================
void SMESHGUI_FilterLibraryDlg::onNeedValidation()
{
  if (!myCurrFilterName.isEmpty())
  {
    bool valid = isValid(true);
    myTable->SetValidity(valid);

    if (valid)
    {
      SMESH::Filter_var aFilter = createFilter(myTable->GetType());
      myLibrary->Replace(myCurrFilterName.latin1(), myName->text().latin1(), aFilter);
    }
  }
}
