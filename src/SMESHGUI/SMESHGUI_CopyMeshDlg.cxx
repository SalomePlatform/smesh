// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : SMESHGUI_CopyMeshDlg.cxx

#include "SMESHGUI_CopyMeshDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_Selection.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include <SMESH_Actor.h>
#include <SMESH_TypeFilter.hxx>
#include <SMDS_Mesh.hxx>

// SALOME GUI includes
#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SalomeApp_Tools.h>

// SALOME KERNEL includes
#include <SALOMEDSClient_Study.hxx>
#include <SALOMEDSClient_SObject.hxx>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>

// Qt includes
#include <QApplication>
#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSpinBox>
#include <QKeyEvent>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

/*!
  \class BusyLocker
  \brief Simple 'busy state' flag locker.
  \internal
*/

namespace
{
  class BusyLocker
  {
  public:
    //! Constructor. Sets passed boolean flag to \c true.
    BusyLocker( bool& busy ) : myBusy( busy ) { myBusy = true; }
    //! Destructor. Clear external boolean flag passed as parameter to the constructor to \c false.
    ~BusyLocker() { myBusy = false; }
  private:
    bool& myBusy; //! External 'busy state' boolean flag
  };
}

#define SPACING 6
#define MARGIN  11


//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================

SMESHGUI_CopyMeshDlg::SMESHGUI_CopyMeshDlg( SMESHGUI* theModule )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
    mySelectedObject(SMESH::SMESH_IDSource::_nil()),
    myFilterDlg(0),
    myIsApplyAndClose( false )
{
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_COPY_MESH")));
  QPixmap image2 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_COPY_MESH_WG")));

  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("SMESH_COPY_MESH_TITLE"));
  setSizeGripEnabled(true);

  QVBoxLayout* SMESHGUI_CopyMeshDlgLayout = new QVBoxLayout(this);
  SMESHGUI_CopyMeshDlgLayout->setSpacing(SPACING);
  SMESHGUI_CopyMeshDlgLayout->setMargin(MARGIN);

  /***************************************************************/
  ConstructorsBox = new QGroupBox(tr("SMESH_COPY_MESH_TITLE"), this);
  GroupConstructors = new QButtonGroup(this);
  QHBoxLayout* ConstructorsBoxLayout = new QHBoxLayout(ConstructorsBox);
  ConstructorsBoxLayout->setSpacing(SPACING);
  ConstructorsBoxLayout->setMargin(MARGIN);

  QRadioButton* RadioButton1= new QRadioButton(ConstructorsBox);
  RadioButton1->setIcon(image1);
  GroupConstructors->addButton(RadioButton1, 0);
  QRadioButton* RadioButton2= new QRadioButton(ConstructorsBox);
  RadioButton2->setIcon(image2);
  GroupConstructors->addButton(RadioButton2, 1);

  ConstructorsBoxLayout->addWidget(RadioButton1);
  ConstructorsBoxLayout->addWidget(RadioButton2);
  RadioButton1->setChecked(true);

  /***************************************************************/
  GroupArguments = new QGroupBox(tr("SMESH_ARGUMENTS"), this);
  QGridLayout* GroupArgumentsLayout = new QGridLayout(GroupArguments);
  GroupArgumentsLayout->setSpacing(SPACING);
  GroupArgumentsLayout->setMargin(MARGIN);

  myIdValidator = new SMESHGUI_IdValidator(this);

  // Controls for idSource/elements selection
  myTextLabelElements = new QLabel(tr("OBJECT_NAME"), GroupArguments);
  myLineEditElements = new QLineEdit(GroupArguments);
  myLineEditElements->setValidator(myIdValidator);
  myLineEditElements->setMaxLength(-1);
  myFilterBtn = new QPushButton( tr( "SMESH_BUT_FILTER" ), GroupArguments );
  connect(myFilterBtn,   SIGNAL(clicked()), this, SLOT(setFilters()));

  // Control for the mesh objects selection
  myIdSourceCheck = new QCheckBox(tr("SMESH_SELECT_WHOLE_MESH"), GroupArguments);

  // Name of a mesh to create
  QLabel* meshNameLabel = new QLabel(tr("NEW_NAME"), GroupArguments);
  myMeshNameEdit = new QLineEdit(GroupArguments);

  // CheckBox for copying groups
  myCopyGroupsCheck = new QCheckBox(tr("SMESH_MAKE_GROUPS"), GroupArguments);
  myCopyGroupsCheck->setChecked(true);

  // CheckBox for keeping ids ( OBSOLETE )
  myKeepIdsCheck = new QCheckBox(tr("SMESH_KEEP_IDS"), GroupArguments);
  myKeepIdsCheck->setChecked(true);
  myKeepIdsCheck->hide();

  // New geometry
  myGeomLabel = new QLabel( tr("NEW_GEOM"), GroupArguments );
  myGeomNameEdit = new QLineEdit( GroupArguments );
  myGeomNameEdit->setReadOnly(true);

  // CheckBox to reuse hypotheses
  myReuseHypCheck = new QCheckBox(tr("REUSE_HYPOTHESES"), GroupArguments);
  myReuseHypCheck->setChecked(true);

  // CheckBox to copy mesh elements
  myCopyElementsCheck = new QCheckBox(tr("COPY_ELEMENTS"), GroupArguments);
  myCopyElementsCheck->setChecked(true);

  // layout
  GroupArgumentsLayout->addWidget(myTextLabelElements,  0, 0);
  GroupArgumentsLayout->addWidget(myLineEditElements,   0, 1, 1, 5);
  GroupArgumentsLayout->addWidget(myFilterBtn,          0, 6);
  GroupArgumentsLayout->addWidget(myIdSourceCheck,      1, 0, 1, 6);
  GroupArgumentsLayout->addWidget(meshNameLabel,        2, 0);
  GroupArgumentsLayout->addWidget(myMeshNameEdit,       2, 1, 1, 5);
  GroupArgumentsLayout->addWidget(myGeomLabel,          3, 0);
  GroupArgumentsLayout->addWidget(myGeomNameEdit,       3, 1, 1, 5);
  GroupArgumentsLayout->addWidget(myCopyGroupsCheck,    4, 0, 1, 6);
  GroupArgumentsLayout->addWidget(myReuseHypCheck,      5, 0, 1, 6);
  GroupArgumentsLayout->addWidget(myCopyElementsCheck,  6, 0, 1, 6);
  // GroupArgumentsLayout->addWidget(myKeepIdsCheck,       7, 0, 1, 6);
  GroupArgumentsLayout->setRowStretch( 6, 1 );

  /***************************************************************/
  GroupButtons = new QGroupBox(this);
  QHBoxLayout* GroupButtonsLayout = new QHBoxLayout(GroupButtons);
  GroupButtonsLayout->setSpacing(SPACING);
  GroupButtonsLayout->setMargin(MARGIN);

  buttonOk = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), GroupButtons);
  buttonOk->setAutoDefault(true);
  buttonOk->setDefault(true);
  buttonApply = new QPushButton(tr("SMESH_BUT_APPLY"), GroupButtons);
  buttonApply->setAutoDefault(true);
  buttonCancel = new QPushButton(tr("SMESH_BUT_CLOSE"), GroupButtons);
  buttonCancel->setAutoDefault(true);
  buttonHelp = new QPushButton(tr("SMESH_BUT_HELP"), GroupButtons);
  buttonHelp->setAutoDefault(true);

  GroupButtonsLayout->addWidget(buttonOk);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addWidget(buttonApply);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addStretch();
  GroupButtonsLayout->addWidget(buttonCancel);
  GroupButtonsLayout->addWidget(buttonHelp);

  /***************************************************************/
  SMESHGUI_CopyMeshDlgLayout->addWidget(ConstructorsBox);
  SMESHGUI_CopyMeshDlgLayout->addWidget(GroupArguments);
  SMESHGUI_CopyMeshDlgLayout->addWidget(GroupButtons);

  /* Initialisations */
  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  // Selection filter
  myIdSourceFilter = new SMESH_TypeFilter( SMESH::IDSOURCE );

  myHelpFileName = "copy_mesh.html";

  Init();

  /* signals and slots connections */
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(buttonApply,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));

  connect(mySMESHGUI,     SIGNAL (SignalDeactivateActiveDialog()),
          this,           SLOT   (DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL (currentSelectionChanged()),
          this,           SLOT   (SelectionIntoArgument()));
  connect(mySMESHGUI,     SIGNAL (SignalCloseAllDialogs()),/* to close dialog if study change */
          this,           SLOT   (reject()));
  connect(mySMESHGUI,     SIGNAL (SignalActivatedViewManager()),
          this,           SLOT   (onOpenView()));
  connect(mySMESHGUI,     SIGNAL (SignalCloseView()),
          this,           SLOT   (onCloseView()));

  connect(myLineEditElements, SIGNAL(textChanged(const QString&)),
          this,               SLOT  (onTextChange(const QString&)));
  connect(myIdSourceCheck,    SIGNAL(toggled(bool)),
          this,               SLOT  (onSelectIdSource(bool)));
  connect(GroupConstructors,  SIGNAL(buttonClicked(int)),
          this,               SLOT  (onConstructor(int)));


  SelectionIntoArgument();
}

//=================================================================================
// function : ~SMESHGUI_CopyMeshDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================

SMESHGUI_CopyMeshDlg::~SMESHGUI_CopyMeshDlg()
{
  if ( myFilterDlg )
  {
    myFilterDlg->setParent( 0 );
    delete myFilterDlg; myFilterDlg = 0;
  }
  if ( myIdSourceFilter )
  {
    if ( mySelectionMgr )
      mySelectionMgr->removeFilter( myIdSourceFilter );
    delete myIdSourceFilter; myIdSourceFilter=0;
  }
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_CopyMeshDlg::Init (bool ResetControls)
{
  myBusy = false;

  if ( !isWithGeomMode() )
    myMeshNameEdit->setText( SMESH::UniqueMeshName("Mesh"));

  if ( ResetControls )
  {
    myLineEditElements->clear();
    myNbOkElements = 0;

    buttonOk->setEnabled(false);
    buttonApply->setEnabled(false);

    myActor = 0;
    myMesh = SMESH::SMESH_Mesh::_nil();

    myIdSourceCheck->setChecked(true);

    onConstructor( 0 );
  }
}

//=======================================================================
//function : onConstructor
//purpose  : switch operation mode
//=======================================================================

void SMESHGUI_CopyMeshDlg::onConstructor( int withGeom )
{
  myGeomLabel        ->setVisible( withGeom );
  myGeomNameEdit     ->setVisible( withGeom );
  myReuseHypCheck    ->setVisible( withGeom );
  myCopyElementsCheck->setVisible( withGeom );
  myFilterBtn        ->setVisible( !withGeom );
  myIdSourceCheck    ->setVisible( !withGeom );

  if ( !withGeom )
    myMeshNameEdit->setText( SMESH::UniqueMeshName("Mesh"));

  onSelectIdSource( /*toSelectMesh=*/ myIdSourceCheck->isChecked() || withGeom );
}

//=======================================================================
//function : getErrorMsg
//purpose  : Return an error message and entries of invalid smesh object
//=======================================================================

QString SMESHGUI_CopyMeshDlg::getErrorMsg( SMESH::string_array_var theInvalidEntries,
                                           QStringList &           theEntriesToBrowse )
{
  if ( theInvalidEntries->length() == 0 )
    return tr("OPERATION_FAILED");

  // theInvalidEntries - SObject's that hold geometry objects whose
  // counterparts are not found in the newGeometry, followed by SObject's
  // holding mesh sub-objects that are invalid because they depend on a not found
  // preceding sub-shape

  QString msg = tr("SUBSHAPES_NOT_FOUND_MSG") + "\n";

  QString objString;
  for ( CORBA::ULong i = 0; i < theInvalidEntries->length(); ++i )
  {
    _PTR(SObject) so = SMESH::getStudy()->FindObjectID( theInvalidEntries[i].in() );

    int objType = SMESHGUI_Selection::type( theInvalidEntries[i].in() );
    if ( objType < 0 ) // geom object
    {
      objString += "\n";
      if ( so )
        objString += so->GetName().c_str();
      else
        objString += theInvalidEntries[i].in(); // it's something like "FACE #2"
    }
    else // smesh object
    {
      theEntriesToBrowse.push_back( theInvalidEntries[i].in() );

      objString += "\n   ";
      switch ( objType ) {
      case SMESH::MESH:
        objString += tr("SMESH_MESH"); break;
      case SMESH::HYPOTHESIS:
        objString += tr("SMESH_HYPOTHESIS"); break;
      case SMESH::ALGORITHM:
        objString += tr("SMESH_ALGORITHM"); break;
      case SMESH::SUBMESH_VERTEX:
      case SMESH::SUBMESH_EDGE:
      case SMESH::SUBMESH_FACE:
      case SMESH::SUBMESH_SOLID:
      case SMESH::SUBMESH_COMPOUND:
      case SMESH::SUBMESH:
        objString += tr("SMESH_SUBMESH"); break;
      case SMESH::GROUP:
        objString += tr("SMESH_GROUP"); break;
      default:;
      }
      objString += " \"";
      if ( so )
        objString += so->GetName().c_str();
      objString += "\" (";
      objString += theInvalidEntries[i].in();
      objString += ")";
    }
  }
  if ( !objString.isEmpty() )
    msg += objString;

  return msg;
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================

bool SMESHGUI_CopyMeshDlg::ClickOnApply()
{
  if ( SMESHGUI::isStudyLocked() )
    return false;

  if( !isValid() )
    return false;

  QStringList anEntryList;
  bool toShowObjects = isApplyAndClose();
  try
  {
    SUIT_OverrideCursor aWaitCursor;

    SMESH::IDSource_wrap aPartToCopy;
    if ( myIdSourceCheck->isChecked())
    {
      aPartToCopy = mySelectedObject;
      aPartToCopy->Register();
    }
    else
    {
      QStringList aListElementsId = myLineEditElements->text().split(" ", QString::SkipEmptyParts);
      SMESH::smIdType_array_var anElementsId = new SMESH::smIdType_array;
      anElementsId->length(aListElementsId.count());
      for (int i = 0; i < aListElementsId.count(); i++)
        anElementsId[i] = aListElementsId[i].toInt();

      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
      aPartToCopy = aMeshEditor->MakeIDSource( anElementsId, SMESH::ALL );
    }
    QByteArray meshName = myMeshNameEdit->text().toUtf8();
    bool toCopyGroups = ( myCopyGroupsCheck->isChecked() );
    bool toReuseHyps  = ( myReuseHypCheck->isChecked() );
    bool toCopyElems  = ( myCopyElementsCheck->isChecked() );
    bool toKeepIDs    = ( myKeepIdsCheck->isChecked() );

    SMESH::SMESH_Gen_var gen = SMESHGUI::GetSMESHGen();
    SMESH::SMESH_Mesh_var newMesh;
    if ( isWithGeomMode() )
    {
      SMESH::SMESH_Mesh_var       srcMesh = mySelectedObject->GetMesh();
      SMESH::ListOfGroups_var     newGroups;
      SMESH::submesh_array_var    newSubmeshes;
      SMESH::ListOfHypothesis_var newHypotheses;
      SMESH::string_array_var     invalidEntries;
      CORBA::Boolean ok = gen->CopyMeshWithGeom( srcMesh, myNewGeometry,
                                                 meshName.constData(),
                                                 toCopyGroups, toReuseHyps, toCopyElems,
                                                 newMesh.out(),
                                                 newGroups.out(),
                                                 newSubmeshes.out(),
                                                 newHypotheses.out(),
                                                 invalidEntries.out() );
      if ( !ok )
      {
        if ( invalidEntries->length() > 0 )
          toShowObjects = true;
        SUIT_MessageBox::warning( this,
                                  tr("SMESH_WRN_WARNING"),
                                  getErrorMsg( invalidEntries, anEntryList ));
      }
    }
    else
    {
      newMesh = gen->CopyMesh(aPartToCopy, meshName.constData(), toCopyGroups, toKeepIDs);
    }
    if ( !newMesh->_is_nil() )
      if ( _PTR(SObject) aSObject = SMESH::ObjectToSObject( newMesh ) )
      {
        anEntryList.append( aSObject->GetID().c_str() );

        if ( isWithGeomMode() )
          SMESH::SetName( aSObject, meshName );
      }
  }
  catch(const SALOME::SALOME_Exception & S_ex)
  {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  }
  catch (...)
  {
  }

  mySMESHGUI->updateObjBrowser(true);
  SMESHGUI::Modified();

  if( LightApp_Application* anApp =
      dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
    anApp->browseObjects( anEntryList, toShowObjects );

  Init(false);
  SelectionIntoArgument();

  return true;
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_CopyMeshDlg::ClickOnOk()
{
  setIsApplyAndClose( true );
  if( ClickOnApply() )
    reject();
}

//=================================================================================
// function : reject()
// purpose  :
//=================================================================================
void SMESHGUI_CopyMeshDlg::reject()
{
  disconnect(mySelectionMgr, 0, this, 0);
  if ( mySelectionMgr )
    mySelectionMgr->removeFilter( myIdSourceFilter );
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( ActorSelection );
  mySMESHGUI->ResetState();
  QDialog::reject();
}

//=================================================================================
// function : onOpenView()
// purpose  :
//=================================================================================
void SMESHGUI_CopyMeshDlg::onOpenView()
{
  if ( mySelector ) {
    SMESH::SetPointRepresentation(false);
  }
  else {
    mySelector = SMESH::GetViewWindow( mySMESHGUI )->GetSelector();
    ActivateThisDialog();
  }
}

//=================================================================================
// function : onCloseView()
// purpose  :
//=================================================================================
void SMESHGUI_CopyMeshDlg::onCloseView()
{
  DeactivateActiveDialog();
  mySelector = 0;
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_CopyMeshDlg::ClickOnHelp()
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
// function : onTextChange()
// purpose  :
//=======================================================================

void SMESHGUI_CopyMeshDlg::onTextChange (const QString& theNewText)
{
  QLineEdit* send = (QLineEdit*)sender();

  if (myBusy) return;
  BusyLocker lock( myBusy );

  //if (send == myLineEditElements)
  myNbOkElements = 0;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  // highlight entered elements
  SMDS_Mesh* aMesh = 0;
  if (myActor)
    aMesh = myActor->GetObject()->GetMesh();

  QStringList aListId = theNewText.split(" ", QString::SkipEmptyParts);
  if (myActor && aMesh)
  {
    SVTK_TVtkIDsMap newIndices;
    if (send == myLineEditElements) {
      for (int i = 0; i < aListId.count(); i++)
        if ( const SMDS_MeshElement * e = aMesh->FindElement(aListId[ i ].toInt()))
        {
          newIndices.Add(e->GetID());
        }
    }
    myNbOkElements = newIndices.Extent();

    Handle(SALOME_InteractiveObject) anIO = myActor->getIO();
    mySelector->AddOrRemoveIndex( anIO, newIndices, false );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->highlight( anIO, true, true );
  }
  else
  {
    myNbOkElements = aListId.count();
  }

  if (myNbOkElements) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================

void SMESHGUI_CopyMeshDlg::SelectionIntoArgument()
{
  if (myBusy) return;
  if (myFilterDlg && myFilterDlg->isVisible()) return; // filter dlg active
  if (!GroupButtons->isEnabled()) return;              // inactive

  BusyLocker lock( myBusy );

  // clear
  myActor = 0;
  QString aString = "";

  myNbOkElements = 0;
  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);
  myFilterBtn->setEnabled(false);

  // get selected mesh or geometry
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);
  int nbSel = aList.Extent();
  if (nbSel != 1)
    return;

  Handle(SALOME_InteractiveObject) IO = aList.First();

  SMESH::SMESH_Mesh_var mesh = SMESH::GetMeshByIO(IO);
  GEOM::GEOM_Object_var geom = SMESH::GetGeom(IO);

  if ( !mesh->_is_nil() )
  {
    myMesh  = mesh;
    myActor = SMESH::FindActorByEntry(IO->getEntry());
    if (!myActor)
      myActor = SMESH::FindActorByObject(myMesh);

    mySelectedObject = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( IO );
    if ( mySelectedObject->_is_nil() )
      return;

    if ( isWithGeomMode() ) // only mesh selection allowed
    {
      myMesh = SMESH::SMESH_Mesh::_narrow( mySelectedObject );
      if ( myMesh->_is_nil() )
      {
        myLineEditElements->setText("");
        return;
      }
    }
  }
  else if ( !geom->_is_nil() )
  {
    myNewGeometry = geom;
  }
  else
    return;

  if (myIdSourceCheck->isChecked() || isWithGeomMode() )
  {
    SMESH::GetNameOfSelectedIObjects( mySelectionMgr, aString );
    if ( aString.isEmpty() ) aString = " ";
    else                     aString = aString.trimmed(); // issue 0021327
  }
  else
  {
    SMESH::GetNameOfSelectedElements( mySelector, IO, aString );
    myNbOkElements = aString.size();
    myFilterBtn->setEnabled(true);
  }

  bool ok = !aString.isEmpty();
  if ( !mesh->_is_nil() )
  {
    myLineEditElements->setText( aString );
    if ( isWithGeomMode() )
      myMeshNameEdit->setText( aString );
  }
  else if ( !geom->_is_nil() )
  {
    myGeomNameEdit->setText( aString );
  }

  if ( ok && isWithGeomMode() )
    ok = ( !myMesh->_is_nil() && myMesh->HasShapeToMesh() && !myNewGeometry->_is_nil() );

  buttonOk->setEnabled(ok);
  buttonApply->setEnabled(ok);
}

//=======================================================================
//function : onSelectIdSource
//purpose  :
//=======================================================================
void SMESHGUI_CopyMeshDlg::onSelectIdSource (bool toSelectMesh)
{
  if ( isWithGeomMode() )
    myTextLabelElements->setText(tr("SMESH_MESH"));
  else if ( toSelectMesh )
    myTextLabelElements->setText(tr("OBJECT_NAME"));
  else
    myTextLabelElements->setText(tr("ELEM_IDS"));

  if (toSelectMesh) {
    myLineEditElements->clear();
  }

  mySelectionMgr->clearFilters();
  if ( !isWithGeomMode() )
    mySelectionMgr->installFilter(myIdSourceFilter);
  SMESH::SetPointRepresentation(false);

  if (toSelectMesh) {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( ActorSelection );
    myLineEditElements->setReadOnly(true);
    myLineEditElements->setValidator(0);
  }
  else
  {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( CellSelection );
    myLineEditElements->setReadOnly(false);
    myLineEditElements->setValidator(myIdValidator);
    onTextChange(myLineEditElements->text());
  }

  SelectionIntoArgument();
}

//=================================================================================
// function : isValid
// purpose  :
//=================================================================================

bool SMESHGUI_CopyMeshDlg::isValid()
{
  bool ok = false;
  if ( myIdSourceCheck->isChecked() || isWithGeomMode() )
  {
    ok = ( !mySelectedObject->_is_nil() );
    if ( isWithGeomMode() )
      ok = ok && ( !myNewGeometry->_is_nil() );
  }
  else
  {
    ok = ( myNbOkElements > 0 );
  }

  return ok;
}

//=======================================================================
//function : isWithGeomMode
//purpose  : Return true if the mode is "with geometry"
//=======================================================================

bool SMESHGUI_CopyMeshDlg::isWithGeomMode()
{
  return ( GroupConstructors->checkedId() == 1 );
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_CopyMeshDlg::DeactivateActiveDialog()
{
  if (ConstructorsBox->isEnabled()) {
    ConstructorsBox->setEnabled(false);
    GroupArguments->setEnabled(false);
    GroupButtons->setEnabled(false);
    mySMESHGUI->ResetState();
    mySMESHGUI->SetActiveDialogBox(0);
    if ( mySelectionMgr )
      mySelectionMgr->removeFilter( myIdSourceFilter );
  }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_CopyMeshDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  ConstructorsBox->setEnabled(true);
  GroupArguments->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  onSelectIdSource( myIdSourceCheck->isChecked() );

  SelectionIntoArgument();
}


//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_CopyMeshDlg::enterEvent (QEvent*)
{
  if ( !ConstructorsBox->isEnabled() ) {
    SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
    if ( aViewWindow && !mySelector ) {
      mySelector = aViewWindow->GetSelector();
    }
    ActivateThisDialog();
  }
}
//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_CopyMeshDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    ClickOnHelp();
  }
}

//=================================================================================
// function : setFilters()
// purpose  : SLOT. Called when "Filter" button pressed.
//=================================================================================
void SMESHGUI_CopyMeshDlg::setFilters()
{
  if(myMesh->_is_nil()) {
    SUIT_MessageBox::critical(this,
                              tr("SMESH_ERROR"),
                              tr("NO_MESH_SELECTED"));
   return;
  }
  if ( !myFilterDlg )
    myFilterDlg = new SMESHGUI_FilterDlg( mySMESHGUI, SMESH::ALL );

  QList<int> types;
  if ( myMesh->NbEdges()     ) types << SMESH::EDGE;
  if ( myMesh->NbFaces()     ) types << SMESH::FACE;
  if ( myMesh->NbVolumes()   ) types << SMESH::VOLUME;
  if ( myMesh->NbBalls()     ) types << SMESH::BALL;
  if ( myMesh->Nb0DElements()) types << SMESH::ELEM0D;
  if ( types.count() > 1 )     types << SMESH::ALL;

  myFilterDlg->Init( types );
  myFilterDlg->SetSelection();
  myFilterDlg->SetMesh( myMesh );
  myFilterDlg->SetSourceWg( myLineEditElements );

  myFilterDlg->show();
}

//================================================================
// function : setIsApplyAndClose
// Purpose  : Set value of the flag indicating that the dialog is
//            accepted by Apply & Close button
//================================================================
void SMESHGUI_CopyMeshDlg::setIsApplyAndClose( const bool theFlag )
{
  myIsApplyAndClose = theFlag;
}

//================================================================
// function : isApplyAndClose
// Purpose  : Get value of the flag indicating that the dialog is
//            accepted by Apply & Close button
//================================================================
bool SMESHGUI_CopyMeshDlg::isApplyAndClose() const
{
  return myIsApplyAndClose;
}
