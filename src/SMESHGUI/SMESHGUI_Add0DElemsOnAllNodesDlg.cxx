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
// File      : SMESHGUI_Add0DElemsOnAllNodesDlg.cxx
// Created   : Fri Oct 19 15:51:24 2012
// Author    : Edward AGAPOV (eap)

#include "SMESHGUI_Add0DElemsOnAllNodesDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESH_LogicalFilter.hxx"
#include "SMESH_TypeFilter.hxx"

#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_MessageBox.h>
#include <SVTK_Selector.h>
#include <SalomeApp_Tools.h>

// Qt includes
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)


#define SPACING 6
#define MARGIN  11

enum { SEL_OBJECT, SEL_ELEMENTS, SEL_NODES };

//================================================================================
/*!
 * \brief Dialog Constructor
 */
//================================================================================

SMESHGUI_Add0DElemsOnAllNodesDlg::SMESHGUI_Add0DElemsOnAllNodesDlg()
  : SMESHGUI_Dialog( 0, false, true ),
    myIDValidator( this )
{
  setWindowTitle( tr( "CAPTION" ) );

  // Seletction type radio buttons

  QGroupBox* selTypeGrBox = new QGroupBox( mainFrame() );
  //
  QRadioButton*    objBtn = new QRadioButton( tr( "SMESH_SUBMESH_GROUP"),selTypeGrBox );
  QRadioButton*   elemBtn = new QRadioButton( tr( "SMESH_ELEMENTS" ),selTypeGrBox );
  QRadioButton*   nodeBtn = new QRadioButton( tr( "SMESH_NODES" ),selTypeGrBox );
  //
  QHBoxLayout* selTypeLay = new QHBoxLayout( selTypeGrBox );
  selTypeLay->setMargin(MARGIN);
  selTypeLay->setSpacing(SPACING);
  selTypeLay->addWidget( objBtn  );
  selTypeLay->addWidget( elemBtn );
  selTypeLay->addWidget( nodeBtn );
  objBtn->setChecked(true);
  //
  mySelTypeBtnGrp         = new QButtonGroup( mainFrame() );
  mySelTypeBtnGrp->addButton( objBtn , SEL_OBJECT );
  mySelTypeBtnGrp->addButton( elemBtn, SEL_ELEMENTS );
  mySelTypeBtnGrp->addButton( nodeBtn, SEL_NODES );

  // Label, Select Btn, LineEdit, Filter Btn

  setObjectPixmap( "SMESH", tr( "ICON_SELECT" ) );
  createObject( tr( "SMESH_NAME" ), mainFrame(), 0 );

  myFilterBtn = new QPushButton( tr( "SMESH_BUT_FILTER" ), mainFrame() );

  myDuplicateElemsChkBox = new QCheckBox( tr( "SMESH_DUPLICATE_0D" ), mainFrame() );

  // List of groups

  myGroupBox = new QGroupBox( tr( "SMESH_ADD_TO_GROUP" ), mainFrame() );
  myGroupBox->setCheckable( true );

  myGroupLabel = new QLabel( tr( "SMESH_GROUP" ), myGroupBox );
  myGroupListCmBox = new QComboBox( myGroupBox );
  myGroupListCmBox->setEditable( true );
  myGroupListCmBox->setInsertPolicy( QComboBox::NoInsert );

  QHBoxLayout* groupsLayout = new QHBoxLayout( myGroupBox );
  groupsLayout->setSpacing(SPACING);
  groupsLayout->setMargin(MARGIN);
  groupsLayout->addWidget( myGroupLabel );
  groupsLayout->addWidget( myGroupListCmBox, 1 );

  // Main layout

  QGridLayout* aLay = new QGridLayout( mainFrame() );
  aLay->setMargin(MARGIN);
  aLay->setSpacing(SPACING);
  //
  aLay->addWidget( selTypeGrBox,          0, 0, 1, 5  );
  //
  aLay->addWidget( objectWg( 0, Label  ), 1, 0 );
  aLay->addWidget( objectWg( 0, Btn    ), 1, 1 );
  aLay->addWidget( objectWg( 0, Control), 1, 2, 1, 2 );
  aLay->addWidget( myFilterBtn,           1, 4 );
  aLay->addWidget( myDuplicateElemsChkBox,2, 0 );
  //
  aLay->addWidget( myGroupBox,            3, 0, 1, 5 );

  // Signals

  connect( myGroupBox,      SIGNAL( toggled( bool )),     SLOT( onGroupChecked() ));
  connect( mySelTypeBtnGrp, SIGNAL( buttonClicked(int) ), SLOT( onSelTypeChange(int)));

  onSelTypeChange( SEL_OBJECT );
}

//================================================================================
/*!
 * \brief SLOT to enable/disable groups
 */
//================================================================================

void SMESHGUI_Add0DElemsOnAllNodesDlg::onGroupChecked( bool on )
{
  myGroupLabel->setEnabled( on );
  myGroupListCmBox->setEnabled( on );
}

//================================================================================
/*!
 * \brief SLOT to enable/disable groups
 */
//================================================================================

void SMESHGUI_Add0DElemsOnAllNodesDlg::onSelTypeChange( int selType )
{
  setNameIndication( 0, selType == SEL_OBJECT ? OneName : ListOfNames );
  setReadOnly      ( 0, selType == SEL_OBJECT );

  QLabel* label = qobject_cast< QLabel* >( objectWg(0, Label ));
  switch( selType ) {
  case SEL_OBJECT:   label->setText( tr("SMESH_NAME")); break;
  case SEL_ELEMENTS: label->setText( tr("ELEMENT_IDS")); break;
  case SEL_NODES:    label->setText( tr("NODE_IDS")); break;
  default:;
  }
  QLineEdit* lineEdit = qobject_cast< QLineEdit* >( objectWg(0, Control ));
  lineEdit->setText("");
  lineEdit->setValidator( selType == SEL_OBJECT ? 0 : & myIDValidator );

  myFilterBtn->setEnabled( selType != SEL_OBJECT );

  emit selTypeChanged( selType );
}

//================================================================================
/*!
 * \brief Return type of selected object: [SEL_OBJECT, SEL_ELEMENTS, SEL_NODES]
 */
//================================================================================

int SMESHGUI_Add0DElemsOnAllNodesDlg::getSelectionType() const
{
  return mySelTypeBtnGrp->checkedId();
}

//================================================================================
/*!
 * \brief Checks consistency of data
 */
//================================================================================

bool SMESHGUI_Add0DElemsOnAllNodesDlg::isValid()
{
  if( myGroupBox->isChecked() && myGroupListCmBox->currentText().isEmpty() ) {
    SUIT_MessageBox::warning( this, tr( "SMESH_WRN_WARNING" ), tr( "GROUP_NAME_IS_EMPTY" ) );
    return false;
  }
  return true;
}

//================================================================================
/*!
 * \brief Operation Constructor
 */
//================================================================================

SMESHGUI_Add0DElemsOnAllNodesOp::SMESHGUI_Add0DElemsOnAllNodesOp()
  :SMESHGUI_SelectionOp(),
   myDlg( new SMESHGUI_Add0DElemsOnAllNodesDlg ),
   myFilterDlg( 0 )
{
  myHelpFileName = "adding_nodes_and_elements_page.html#adding_0delems_on_all_nodes_anchor";

  connect( myDlg,              SIGNAL( selTypeChanged(int) ), SLOT( onSelTypeChange(int)));
  connect( myDlg->myFilterBtn, SIGNAL( clicked()),            SLOT( onSetFilter() ));
  connect( myDlg->myGroupBox,  SIGNAL( clicked(bool)),        SLOT( updateButtons() ));
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

SMESHGUI_Add0DElemsOnAllNodesOp::~SMESHGUI_Add0DElemsOnAllNodesOp()
{
  if ( myFilterDlg ) {
    myFilterDlg->setParent( 0 );
    delete myFilterDlg;
    myFilterDlg = 0;
  }
}

//================================================================================
/*!
 * \brief Start
 */
//================================================================================

void SMESHGUI_Add0DElemsOnAllNodesOp::startOperation()
{
  SMESHGUI_SelectionOp::startOperation();

  myDlg->myGroupBox->setChecked( false );
  myDlg->activateObject( 0 );
  myDlg->show();

  selectionDone();
}

//================================================================================
/*!
 * \brief Treat changed selection
 */
//================================================================================

void SMESHGUI_Add0DElemsOnAllNodesOp::selectionDone()
{
  if (myFilterDlg && myFilterDlg->isVisible()) return; // filter dgl active
  if (!myDlg->myGroupBox->isEnabled())         return; // inactive

  myIO.Nullify();
  updateButtons();

  SALOME_ListIO aList;
  selectionMgr()->selectedObjects( aList );
  if ( aList.Extent() == 1 ) {
    myIO = aList.First();
  }
  else {
    myDlg->setObjectText( 0, ""); // it clears the selection
    return;
  }
  QString ids;
  switch ( myDlg->getSelectionType() ) {
  case SEL_OBJECT:
    SMESHGUI_SelectionOp::selectionDone();
    break;
  case SEL_ELEMENTS:
    SMESH::GetNameOfSelectedElements( selector(), myIO, ids );
    myDlg->setObjectText( 0, ids );
    break;
  case SEL_NODES:
    SMESH::GetNameOfSelectedNodes( selector(), myIO, ids );
    myDlg->setObjectText( 0, ids );
    break;
  default:;
  }

  // fill the list of existing groups
  myDlg->myGroupListCmBox->clear();
  myDlg->myGroupListCmBox->addItem( QString() );
  if ( !myIO.IsNull() && myIO->hasEntry())
  {
    SMESH::SMESH_Mesh_var mesh = SMESH::GetMeshByIO( myIO );
    _PTR(SObject)       meshSO = SMESH::ObjectToSObject( mesh );
    _PTR(SObject) group0DRoot;
    if ( meshSO && meshSO->FindSubObject( SMESH::Tag_0DElementsGroups, group0DRoot ))
    {
      _PTR(Study)              aStudy = SMESH::GetActiveStudyDocument();
      _PTR(ChildIterator) group0DIter = aStudy->NewChildIterator( group0DRoot );
      for ( ; group0DIter->More(); group0DIter->Next() )
      {
        _PTR(SObject) groupSO = group0DIter->Value();
        std::string groupName = groupSO->GetName();
        if ( !groupName.empty() )
          myDlg->myGroupListCmBox->addItem( groupName.c_str() );
      }
    }
    // enable buttons
    updateButtons();
  }
}

//=======================================================================
//function : updateButtons
//purpose  : enable [Apply]
//=======================================================================

void SMESHGUI_Add0DElemsOnAllNodesOp::updateButtons()
{
  bool ok = false;

  if (( !myIO.IsNull() && myIO->hasEntry() && !myDlg->objectText( 0 ).isEmpty() ) &&
      ( !myDlg->myGroupBox->isChecked() || !myDlg->myGroupListCmBox->currentText().isEmpty() ))
  {
    SMESH::SMESH_Mesh_var mesh = SMESH::GetMeshByIO( myIO );
    if ( !mesh->_is_nil() )
    {
      if ( myDlg->getSelectionType() == SEL_OBJECT )
        ok = true;
      else
      {
        QString        ids = myDlg->objectText( 0 );
        QStringList idList = ids.split( " ", QString::SkipEmptyParts );
        const bool  isElem = ( myDlg->getSelectionType() == SEL_ELEMENTS );
        QStringList::iterator idIt = idList.begin();
        for ( ; idIt != idList.end() && !ok; ++idIt )
          ok = ( mesh->GetElementType( idIt->toLong(), isElem ) != SMESH::ALL );
      }
    }
  }

  myDlg->button( QtxDialog::Apply )->setEnabled( ok );
  myDlg->button( QtxDialog::OK    )->setEnabled( ok );
}

//================================================================================
/*!
 * \brief Return a filter of objects
 */
//================================================================================

SUIT_SelectionFilter* SMESHGUI_Add0DElemsOnAllNodesOp::createFilter( const int ) const
{
  if ( myDlg->getSelectionType() == SEL_OBJECT )
  {
    // Create a filter of objects: any IDSource except the group of 0D elements

    QList<SUIT_SelectionFilter*> filters;
    filters.push_back( new SMESH_TypeFilter( SMESH::GROUP_0D ));
    SMESH_LogicalFilter* not0DGroup = new SMESH_LogicalFilter( filters,
                                                               SMESH_LogicalFilter::LO_NOT,
                                                               /*takeOwnership=*/true);
    filters[0] = not0DGroup;
    filters.push_back( new SMESH_TypeFilter( SMESH::IDSOURCE ));
    return new SMESH_LogicalFilter( filters,
                                    SMESH_LogicalFilter::LO_AND,
                                    /*takeOwnership=*/true);
  }
  return 0;
}

//================================================================================
/*!
 * \brief Makes its main job
 */
//================================================================================

bool SMESHGUI_Add0DElemsOnAllNodesOp::onApply()
{
  if ( !myDlg->isValid() )
    return false;

  // get a mesh
  SMESH::IDSource_wrap meshObject;
  SMESH::SMESH_Mesh_var      mesh;
  if ( !myIO.IsNull() )
  {
    CORBA::Object_var obj = SMESH::IObjectToObject( myIO );
    meshObject = SMESH::SMESH_IDSource::_narrow( obj );
    if ( !meshObject->_is_nil() )
    {
      mesh = meshObject->GetMesh();
      meshObject->Register();
    }
  }
  if ( mesh->_is_nil() )
  {
    SUIT_MessageBox::warning( myDlg, tr( "SMESH_WRN_WARNING" ), tr( "SMESH_BAD_SELECTION" ) );
    return false;
  }

  try {
    SMESH::SMESH_MeshEditor_var editor = mesh->GetMeshEditor();

    // make SMESH_IDSource holding IDs of selected elements
    if ( myDlg->getSelectionType() != SEL_OBJECT )
    {
      QString elemIDs = myDlg->objectText( 0 );
      QStringList idList = elemIDs.split( " ", QString::SkipEmptyParts );
      if ( idList.count() == 0 )
      {
        SUIT_MessageBox::warning( myDlg, tr( "SMESH_WRN_WARNING" ), tr( "SMESH_BAD_SELECTION" ) );
        return false;
      }
      SMESH::long_array_var idArray = new SMESH::long_array;
      idArray->length( idList.count() );
      QStringList::iterator idIt = idList.begin();
      for ( int i = 0; idIt != idList.end(); ++idIt, ++i )
        idArray[i] = idIt->toLong();
      SMESH::ElementType elemType =
        myDlg->getSelectionType() == SEL_NODES ? SMESH::NODE : SMESH::ALL;
      meshObject = editor->MakeIDSource( idArray, elemType );
    }

    // Create 0D elements

    int prevNb0D = mesh->Nb0DElements();

    QString groupName = myDlg->myGroupListCmBox->currentText();
    SMESH::SMESH_IDSource_var newObj =
      editor->Create0DElementsOnAllNodes( meshObject, groupName.toLatin1().data(),
                                          myDlg->myDuplicateElemsChkBox->isChecked() );

    int newNb0D = mesh->Nb0DElements() - prevNb0D;
    SUIT_MessageBox::information( myDlg, tr( "SMESH_INFORMATION" ),
                                  tr( "NB_NEW_0D" ).arg( newNb0D ),
                                  SUIT_MessageBox::Ok, SUIT_MessageBox::Ok);
  }
  catch ( const SALOME::SALOME_Exception& S_ex ) {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
    return false;
  }
  catch (...) {
    return false;
  }

  // clear selection

  if ( myDlg->getSelectionType() == SEL_OBJECT )
  {
    SALOME_ListIO aList;
    selectionMgr()->setSelectedObjects( aList, /*append=*/false );
  }
  else
  {
    selector()->ClearIndex();
  }
  selectionDone();

  SMESH::UpdateView();
  SMESHGUI::Modified();

  if ( myDlg->myGroupBox->isChecked() )
    SMESHGUI::GetSMESHGUI()->updateObjBrowser();

  return true;
}

//================================================================================
/*!
 * \brief Sets selection mode
 */
//================================================================================

void SMESHGUI_Add0DElemsOnAllNodesOp::onSelTypeChange(int selType)
{
  switch ( selType ) {
  case SEL_OBJECT:   setSelectionMode( ActorSelection ); break;
  case SEL_ELEMENTS: setSelectionMode( CellSelection );  break;
  case SEL_NODES:    setSelectionMode( NodeSelection );  break;
  }

  if ( selType != SEL_OBJECT )
    connect( myDlg, SIGNAL( objectChanged( int, const QStringList& )),
             this,  SLOT  ( onTextChanged( int, const QStringList& )));
  else
    disconnect( myDlg, SIGNAL( objectChanged( int, const QStringList& )),
                this,  SLOT  ( onTextChanged( int, const QStringList& )));

  connect( myDlg->myGroupListCmBox, SIGNAL( editTextChanged(const QString & )),
           this,                    SLOT(   updateButtons() ));

  selectionDone();
}

//================================================================================
/*!
 * \brief Install filters
 */
//================================================================================

void SMESHGUI_Add0DElemsOnAllNodesOp::onSetFilter()
{
  SMESH::SMESH_Mesh_var mesh = SMESH::GetMeshByIO( myIO );
  if ( mesh->_is_nil()) {
    SUIT_MessageBox::critical( myDlg, tr("SMESH_ERROR"), tr("NO_MESH_SELECTED"));
    return;
  }
  QList<int> types;
  if ( myDlg->getSelectionType() == SEL_NODES ) {
    types.push_back( SMESH::NODE );
  }
  else if ( myDlg->getSelectionType() == SEL_ELEMENTS ) {
    types.push_back( SMESH::EDGE );
    types.push_back( SMESH::FACE );
    types.push_back( SMESH::VOLUME );
  }
  else
    return;
  if ( !myFilterDlg )
    myFilterDlg = new SMESHGUI_FilterDlg( getSMESHGUI(), SMESH::ALL );

  myFilterDlg->Init( types );
  myFilterDlg->SetSelection();
  myFilterDlg->SetMesh( mesh );
  myFilterDlg->SetSourceWg( myDlg->objectWg( 0, LightApp_Dialog::Control ));

  myFilterDlg->show();
}

//=======================================================================
//function : onTextChanged
//purpose  : SLOT called when the user types IDs
//=======================================================================

void SMESHGUI_Add0DElemsOnAllNodesOp::onTextChanged( int obj, const QStringList& text )
{
  SMESHGUI_SelectionOp::onTextChanged( obj, text );
  updateButtons();
}
