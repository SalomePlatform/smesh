// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_FilterLibraryDlg.cxx
// Author : Sergey LITONIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_FilterLibraryDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_FilterUtils.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_VTKUtils.h"

#include <GEOMBase.h>

// SALOME GUI includes
#include <SUIT_Session.h>
#include <SUIT_Desktop.h>
#include <SUIT_FileDlg.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIO.hxx>

// Qt includes
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QFileInfo>
#include <QDir>
#include <QKeyEvent>

#define SPACING 6
#define MARGIN  11

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
                                                      const QList<int>& theTypes,
                                                      const int theMode)
  : QDialog( parent ),
    mySMESHGUI( theModule )
{
  setModal(false);
  construct(theTypes, theMode);
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::SMESHGUI_FilterLibraryDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_FilterLibraryDlg::SMESHGUI_FilterLibraryDlg (SMESHGUI* theModule,
                                                      QWidget* parent,
                                                      const int   theType,
                                                      const int   theMode)
  : QDialog( parent ),
    mySMESHGUI( theModule )
{
  setModal(true);
  QList<int> aTypes;
  aTypes.append(theType);
  construct(aTypes, theMode);
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::construct
// Purpose : Construct dialog (called by constructor)
//=======================================================================
void SMESHGUI_FilterLibraryDlg::construct (const QList<int>& theTypes,
                                           const int theMode)
{
  myTypes = theTypes;
  myMode  = theMode;

  QVBoxLayout* aDlgLay = new QVBoxLayout(this);
  aDlgLay->setMargin(MARGIN);
  aDlgLay->setSpacing(SPACING);

  myMainFrame        = createMainFrame  (this);
  QWidget* aBtnFrame = createButtonFrame(this);

  aDlgLay->addWidget(myMainFrame);
  aDlgLay->addWidget(aBtnFrame);

  aDlgLay->setStretchFactor(myMainFrame, 1);

  myHelpFileName = "selection_filter_library_page.html";
  
  Init(myTypes, myMode);
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::createMainFrame
// Purpose : Create frame containing dialog's input fields
//=======================================================================
QWidget* SMESHGUI_FilterLibraryDlg::createMainFrame (QWidget* theParent)
{
  QWidget* aMainFrame = new QWidget(theParent);
  QGridLayout* aMainLay = new QGridLayout(aMainFrame);
  aMainLay->setMargin(0);
  aMainLay->setSpacing(SPACING);

  // library name

  QLabel* aFileLab = new QLabel(tr("LIBRARY_FILE"), aMainFrame);
  myFileName = new QLineEdit(aMainFrame);
  myOpenBtn = new QPushButton(aMainFrame);
  myOpenBtn->setIcon(SUIT_Session::session()->resourceMgr()->loadPixmap(
    "SUIT", tr("ICON_FILE_OPEN")));

  // filters list box

  QGroupBox* aFiltersGrp = new QGroupBox(tr("FILTER_NAMES"), aMainFrame);
  QGridLayout* aLay = new QGridLayout(aFiltersGrp);
  aLay->setMargin(MARGIN);
  aLay->setSpacing(SPACING);

  myListBox = new QListWidget(aFiltersGrp);

  myAddBtn    = new QPushButton(tr("ADD"), aFiltersGrp);
  myDeleteBtn = new QPushButton(tr("DELETE"), aFiltersGrp);

  aLay->addWidget(myListBox,   0, 0, 3, 1);
  aLay->addWidget(myAddBtn,    0, 1);
  aLay->addWidget(myDeleteBtn, 1, 1);
  aLay->setRowStretch(2, 5);

  // filter name

  myNameGrp = new QWidget(aMainFrame);
  QHBoxLayout* myNameGrpLayout = new QHBoxLayout(myNameGrp);
  myNameGrpLayout->setMargin(0);
  myNameGrpLayout->setSpacing(SPACING);

  myNameGrpLayout->addWidget( new QLabel(tr("FILTER_NAME"), myNameGrp) );
  myNameGrpLayout->addWidget( myName = new QLineEdit(myNameGrp) );

  // table

  myTable = new SMESHGUI_FilterTable( mySMESHGUI, aMainFrame, myTypes);
  myTable->SetEditable(myMode == EDIT);
  myTable->SetLibsEnabled(false);

  myListBox->setMinimumHeight((int)(myTable->sizeHint().height() * 0.5));
  //myListBox->setRowMode(QListWidget::FitToWidth); //VSR : TODO ???
  myListBox->setSelectionMode(QListWidget::SingleSelection);

  myOpenBtn->setAutoDefault(false);
  myAddBtn->setAutoDefault(false);
  myDeleteBtn->setAutoDefault(false);

  aMainLay->addWidget(aFileLab,    0, 0);
  aMainLay->addWidget(myFileName,  0, 1);
  aMainLay->addWidget(myOpenBtn,   0, 2);
  aMainLay->addWidget(aFiltersGrp, 1, 0, 1, 3);
  aMainLay->addWidget(myNameGrp,   2, 0, 1, 3);
  aMainLay->addWidget(myTable,     3, 0, 1, 3);

  // connect signals and slots

  connect(myFileName, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
  connect(myOpenBtn, SIGNAL(clicked()), this, SLOT(onBrowse()));

  connect(myListBox, SIGNAL(itemSelectionChanged()),
          this, SLOT(onFilterChanged()));

  connect(myAddBtn, SIGNAL(clicked()), this, SLOT(onAddBtnPressed()));
  connect(myDeleteBtn, SIGNAL(clicked()), this, SLOT(onDeleteBtnPressed()));

  connect(myName, SIGNAL(textChanged(const QString&)),
          this, SLOT(onFilterNameChanged(const QString&)));

  connect(myTable, SIGNAL(EntityTypeChanged(const int)),
          this, SLOT(onEntityTypeChanged(const int)));

  connect(myTable, SIGNAL(NeedValidation()), this, SLOT(onNeedValidation()));

  LightApp_SelectionMgr* selMgr = SMESH::GetSelectionMgr( mySMESHGUI );
  connect( selMgr, SIGNAL(currentSelectionChanged()), SLOT(onSelectionDone()));

  return aMainFrame;
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::createButtonFrame
// Purpose : Create frame containing buttons
//=======================================================================
QWidget* SMESHGUI_FilterLibraryDlg::createButtonFrame (QWidget* theParent)
{
  QGroupBox* aGrp = new QGroupBox(theParent);
  QHBoxLayout* aLay = new QHBoxLayout(aGrp);
  aLay->setMargin(MARGIN);
  aLay->setSpacing(SPACING);

  myButtons[ BTN_OK    ] = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), aGrp);
  myButtons[ BTN_Apply ] = new QPushButton(tr("SMESH_BUT_APPLY"), aGrp);

  myButtons[ BTN_Close  ] = new QPushButton(tr("SMESH_BUT_CLOSE"),  aGrp);
  myButtons[ BTN_Help   ] = new QPushButton(tr("SMESH_BUT_HELP"),   aGrp);

  aLay->addWidget(myButtons[ BTN_OK     ]);
  aLay->addSpacing(10);
  aLay->addWidget(myButtons[ BTN_Apply  ]);
  aLay->addSpacing(10);
  aLay->addStretch();
  aLay->addWidget(myButtons[ BTN_Close  ]);
  aLay->addWidget(myButtons[ BTN_Help   ]);

  connect(myButtons[ BTN_OK     ], SIGNAL(clicked()), SLOT(onOk()));
  connect(myButtons[ BTN_Close  ], SIGNAL(clicked()), SLOT(reject()));
  connect(myButtons[ BTN_Apply  ], SIGNAL(clicked()), SLOT(onApply()));
  connect(myButtons[ BTN_Help   ], SIGNAL(clicked()), SLOT(onHelp()));

  QMap<int, QPushButton*>::iterator anIter;
  for (anIter = myButtons.begin(); anIter != myButtons.end(); ++anIter)
    anIter.value()->setAutoDefault(false);

  updateMainButtons();

  return aGrp;
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::updateMainButtons
// Purpose : Update visibility of main buttons (OK, Cancel, Close ...)
//=======================================================================
void SMESHGUI_FilterLibraryDlg::updateMainButtons()
{
  myButtons[ BTN_Close  ]->show();
  if (myTypes.count() == 1) {
    myButtons[ BTN_Apply  ]->hide();
  } else {
    myButtons[ BTN_Apply  ]->show();
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
  QList<int> aTypes;
  aTypes.append(type);
  Init(aTypes, theMode);
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::Init
// Purpose : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_FilterLibraryDlg::Init (const QList<int>& theTypes,
                                      const int theMode)
{
  myMode = theMode;
  myTypes = theTypes;
  myTable->Init(theTypes);
  myCurrFilterName = "";
  myCurrFilter = -1;
  myListBox->blockSignals(true);
  myListBox->clear();
  myListBox->blockSignals(false);
  myName->clear();
  myTable->Clear();

  updateControlsVisibility();
  setEnabled(true);

  connect( mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), SLOT(onDeactivate()));
  connect( mySMESHGUI, SIGNAL(SignalCloseAllDialogs()), SLOT(reject()));

  if (myMode == ADD_TO)
  {
    setWindowTitle(tr("ADD_TO_TLT"));
    if (myFileName->text().isEmpty())
      myFileName->setText(getDefaultLibraryName());
    processNewLibrary();
  }
  else if (myMode == COPY_FROM)
  {
    setWindowTitle(tr("COPY_FROM_TLT"));
    if (myFileName->text().isEmpty())
      myFileName->setText(getDefaultLibraryName());
    processNewLibrary();
    if (myListBox->count() > 0)
      myListBox->setCurrentItem(0);
  }
  else
  {
    setWindowTitle(tr("EDIT_LIB_TLT"));
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
    SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_WRN_WARNING"),
                                 tr("LIBRARY_IS_NOT_LOADED"));
    return false;
  }

  if (myFileName->text() != myLibrary->GetFileName())
    myLibrary->SetFileName( myFileName->text().toLatin1().constData() );

  bool aResult = false;

  if (myMode == COPY_FROM || myListBox->count() == 0) {
    aResult = true;
  } else if (myMode == EDIT || myMode == ADD_TO) {
    SMESH::Filter_var aFilter = createFilter();
    if (!myListBox->selectedItems().empty() && 
        !myLibrary->Replace(myCurrFilterName.toLatin1().constData(),
                            myName->text().toLatin1().constData(),
                            aFilter.in())) {
      SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                                   tr("ERROR_OF_EDITING"));
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
    SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                                 tr("ERROR_OF_SAVING"));
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
  if (onApply()) {
    disconnect( mySMESHGUI, 0, this, 0);
    mySMESHGUI->ResetState();
    accept();
  }
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::reject
// Purpose : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_FilterLibraryDlg::reject()
{
  disconnect( mySMESHGUI, 0, this, 0);
  mySMESHGUI->ResetState();
  QDialog::reject();
}

//=================================================================================
// function : onHelp()
// purpose  :
//=================================================================================
void SMESHGUI_FilterLibraryDlg::onHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app) 
    app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString(""), myHelpFileName);
  else {
    QString platform;
#ifdef WIN32
    platform = "winapplication";
#else
    platform = "application";
#endif
    SUIT_MessageBox::warning(this, tr("WRN_WARNING"),
                             tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
                             arg(app->resourceMgr()->stringValue("ExternalBrowser", 
                                                                 platform)).
                             arg(myHelpFileName));
  }
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

  int b = theFilter.lastIndexOf("(");
  int e = theFilter.lastIndexOf(")");
  if (b != -1 && e != -1)
  {
    QString content = theFilter.mid(b + 1, e - b - 1).trimmed();
    QStringList lst = content.split(" ", QString::SkipEmptyParts);
    for (QStringList::const_iterator it = lst.begin(); it != lst.end(); ++it)
      if ((*it).indexOf(".") != -1)
        res.append((*it).trimmed());
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
  aDlg->setWindowTitle(tr("OPEN_LIBRARY"));

  //aDlg->setMode(myMode == COPY_FROM ? QFileDialogP::ExistingFile : QFileDialogP::AnyFile);
  aDlg->setFileMode(myMode == COPY_FROM ? QFileDialog::ExistingFile : QFileDialog::AnyFile);
  aDlg->setNameFilters(prepareFilters());
  aDlg->selectFile(getFileName());

  QPushButton* anOkBtn = (QPushButton*)aDlg->findChild<QPushButton*>("OK");
  if (anOkBtn != 0)
    anOkBtn->setText(tr("SMESH_BUT_OK"));

  if (aDlg->exec() != Accepted)
    return;

  QString fName = aDlg->selectedFile();

  if (fName.isEmpty())
    return;

  if (QFileInfo(fName).suffix().isEmpty())
    fName = autoExtension(fName);

  fName = QDir::toNativeSeparators(fName);
  QString prev = QDir::toNativeSeparators(getFileName());

  if (prev == fName)
    return;

  setFileName(fName);

  QListWidgetItem* item = myListBox->item( myListBox->count()-1 );
  QString aName = item ? item->text() : QString::null;
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

  myLibrary = aFilterMgr->LoadLibrary(autoExtension(getFileName()).toLatin1().constData());
  if (myLibrary->_is_nil()) {
    if (myMode == COPY_FROM) {
      SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                                   tr("ERROR_LOAD"));
      return;
    } else {
      myLibrary = aFilterMgr->CreateLibrary();
      myLibrary->SetFileName(getFileName().toLatin1().constData());
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
  myListBox->blockSignals(true);
  myListBox->clear();
  myListBox->blockSignals(false);
  myListBox->addItems(aList);
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
        SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                                     tr("EMPTY_FILTER_NAME"));
      return false;
    }

    SMESH::string_array_var aNames = myLibrary->GetAllNames();
    for (int f = 0, n = aNames->length(); f < n; f++) {
      if (aNames[ f ] == aCurrName && aNames[ f ] != myCurrFilterName) {
        if (theMess)
          SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                                       tr("ERROR_FILTER_NAME"));
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
  if (QFileInfo(fName).suffix().isEmpty())
    fName = autoExtension(fName);

  fName = QDir::toNativeSeparators(fName);

  if (QFileInfo(fName).exists()) {
    isWritable = QFileInfo(fName).isWritable();
  } else if (!theIsExistingOnly) {
    QFileInfo aDirInfo(QFileInfo(fName).absolutePath());
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
    SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_WRN_WARNING"),
                                 tr("NO_PERMISSION"));
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
// Purpose : SLOT. Called when selected filter of library is changed
//=======================================================================
void SMESHGUI_FilterLibraryDlg::onFilterChanged()
{
  QString theName = myListBox->currentItem() ? myListBox->currentItem()->text() : QString::null;
  if (myLibrary->_is_nil())
    return;

  // Save parameters of filter if it was changed

  if (!myCurrFilterName.isEmpty() && myTable->IsEditable())
  {
    if (!isValid(true))
    {
      myListBox->blockSignals(true);
      myListBox->setCurrentRow(myCurrFilter);
      myListBox->blockSignals(false);
      return;
    }

    SMESH::Filter_var aFilter = createFilter();
    myLibrary->Replace(myCurrFilterName.toLatin1().constData(), 
                       myName->text().toLatin1().constData(), 
                       aFilter);
  }

  // Fill table with filter parameters

  SMESH::Filter_var aFilter = myLibrary->Copy(theName.toLatin1().constData());
  myCurrFilterName = theName;
  myCurrFilter = myListBox->currentRow();
  myName->setText(theName);


  SMESH::Filter::Criteria_var aCriteria;

  myTable->Clear(myTable->GetType());

  if (CORBA::is_nil( aFilter ) || !aFilter->GetCriteria(aCriteria))
    return;

  for (int i = 0, n = aCriteria->length(); i < n; i++)
    myTable->AddCriterion(aCriteria[ i ], myTable->GetType());

  myTable->Update();
  updateControlsVisibility(); // IPAL19974
}

//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::onReturnPressed
// Purpose : SLOT. Called when enter button is pressed in library name field
//           Reload library
//=======================================================================
void SMESHGUI_FilterLibraryDlg::onReturnPressed()
{
  QListWidgetItem* item = myListBox->item( myListBox->count()-1 );
  QString aName = item ? item->text() : QString::null;

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
    myLibrary->Replace(myCurrFilterName.toLatin1().constData(), 
                       myName->text().toLatin1().constData(), 
                       aFilter);
  }
  myTable->Clear(myTable->GetType());

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
    SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_WRN_WARNING"),
                                 tr("LIBRARY_IS_NOT_LOADED"));
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
    ? myLibrary->Add(aName.toLatin1().constData(), aFilter)
    : myLibrary->AddEmpty(aName.toLatin1().constData(), (SMESH::ElementType)myTable->GetType());

  if (!aResult) {
    SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                                 tr("ERROR_OF_ADDING"));
  }

  updateList();
  myCurrFilterName = "";
  myCurrFilter = -1;
  setSelected(aName);

  if (theName != aName)
    SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_WARNING"),
                                 tr("ASSIGN_NEW_NAME").arg(theName).arg(aName));
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
    QString aHomeDir = QDir(QDir::home()).absolutePath();
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
    myListBox->setCurrentRow(anIndex);
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
    if (myListBox->item(i)->text() == theName)
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
    SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_WRN_WARNING"),
                                 tr("LIBRARY_IS_NOT_LOADED"));
    return;
  }

  int anIndex = getIndex(myCurrFilterName);

  if (anIndex == -1 || !myLibrary->Delete(myCurrFilterName.toLatin1().constData())) {
    SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                                 tr("ERROR_OF_DELETING"));
  } else {
    myCurrFilterName = "";
    myCurrFilter = -1;
    delete myListBox->item(anIndex);

    if (anIndex >= 1)
      myListBox->item(anIndex - 1)->setSelected(true);
    else if (anIndex == 0 && myListBox->count() > 0)
      myListBox->item(0)->setSelected(true);
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
  int aCurrItem = myListBox->currentRow();
  if (aCurrItem == -1)
    return;

  myListBox->blockSignals(true);
  myListBox->item(aCurrItem)->setText(theName);
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
      myLibrary->Replace(myCurrFilterName.toLatin1().constData(),
                         myName->text().toLatin1().constData(),
                         aFilter);
    }
  }
}


//=======================================================================
// name    : SMESHGUI_FilterLibraryDlg::onSelectionDone
// Purpose : SLOT called when selection changed.
//           If current cell corresponds to the threshold value of
//           BelongToGeom criterion name of selected object is set in this cell
//=======================================================================

void SMESHGUI_FilterLibraryDlg::onSelectionDone()
{
  SALOME_ListIO aList;
  if( LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr() )
    aSel->selectedObjects( aList );

  int aRow, aCol;
  if (aList.Extent() != 1 || !myTable->CurrentCell(aRow, aCol))
    return;

  const int type = myTable->GetCriterionType(aRow);
  QList<int> types;
  types << SMESH::FT_BelongToGeom      << SMESH::FT_BelongToPlane 
        << SMESH::FT_BelongToCylinder  << SMESH::FT_BelongToGenSurface
        << SMESH::FT_LyingOnGeom       << SMESH::FT_CoplanarFaces
        << SMESH::FT_ConnectedElements << SMESH::FT_BelongToMeshGroup;
  if ( !types.contains( type ))
    return;

  Handle(SALOME_InteractiveObject) anIO = aList.First();
  switch ( type )
  {
  case SMESH::FT_CoplanarFaces: // get ID of a selected mesh face
    {
      QString aString;
      int nbElems = SMESH::GetNameOfSelectedElements(SMESH::GetSelector(), anIO, aString);
      if (nbElems == 1)
        myTable->SetThreshold(aRow, aString);
      break;
    }
  case SMESH::FT_ConnectedElements: // get either VERTEX or a node ID
    {
      GEOM::GEOM_Object_var anObj = SMESH::IObjectToInterface<GEOM::GEOM_Object>(anIO);
      if (!anObj->_is_nil())
      {
        myTable->SetThreshold(aRow, GEOMBase::GetName(anObj));
        myTable->SetID       (aRow, anIO->getEntry());
      }
      else
      {
        QString aString;
        int nbElems = SMESH::GetNameOfSelectedElements(SMESH::GetSelector(), anIO, aString);
        if (nbElems == 1)
          myTable->SetThreshold(aRow, aString);
      }
      break;
    }
  case SMESH::FT_BelongToMeshGroup: // get a group name and IOR
    {
      myTable->SetThreshold(aRow, anIO->getName() );
    }
  default: // get a GEOM object
    {
      GEOM::GEOM_Object_var anObj = SMESH::IObjectToInterface<GEOM::GEOM_Object>(anIO);
      if (!anObj->_is_nil())
      {
        myTable->SetThreshold(aRow, GEOMBase::GetName(anObj));
        myTable->SetID       (aRow, anIO->getEntry());
      }
    }
  }
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_FilterLibraryDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    onHelp();
  }
}
