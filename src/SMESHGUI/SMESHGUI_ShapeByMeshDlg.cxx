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

// File   : SMESHGUI_ShapeByMeshDlg.cxx
// Author : Edward AGAPOV, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_ShapeByMeshDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include <SMDS_Mesh.hxx>
#include <SMDS_MeshNode.hxx>
#include <SMESH_Actor.h>

// SALOME GEOM includes
#include <GEOMBase.h>
#include <GeometryGUI.h>
#include <GEOM_wrap.hxx>

// SALOME GUI includes
#include <LightApp_DataOwner.h>
#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_Desktop.h>
#include <SVTK_Selector.h>
#include <SVTK_ViewWindow.h>
#include <SVTK_ViewModel.h>
#include <SalomeApp_Tools.h>

// SALOME KERNEL includes
#include <SALOMEDSClient_SObject.hxx>
#include <SALOMEDS_Study.hxx>
#include <SALOMEDS_wrap.hxx>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>
#include <TopoDS_Shape.hxx>
#include <TopExp_Explorer.hxx>

// Qt includes
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QStringList>

#define SPACING 6
#define MARGIN  11

enum { EDGE = 0, FACE, VOLUME };

/*!
 * \brief Dialog to publish a sub-shape of the mesh main shape
 *        by selecting mesh elements
 */
SMESHGUI_ShapeByMeshDlg::SMESHGUI_ShapeByMeshDlg(bool isMultipleAllowed)
  : SMESHGUI_Dialog( 0, false, true, OK | Close ),
    myIsMultipleAllowed( isMultipleAllowed )
{
  setWindowTitle(tr("CAPTION"));

  QVBoxLayout* aDlgLay = new QVBoxLayout (mainFrame());
  aDlgLay->setMargin(MARGIN);
  aDlgLay->setSpacing(SPACING);

  QFrame* aMainFrame = createMainFrame  (mainFrame());

  aDlgLay->addWidget(aMainFrame);

  aDlgLay->setStretchFactor(aMainFrame, 1);
}

//=======================================================================
// function : createMainFrame()
// purpose  : Create frame containing dialog's input fields
//=======================================================================
QFrame* SMESHGUI_ShapeByMeshDlg::createMainFrame (QWidget* theParent)
{
  QFrame* aMainGrp = new QFrame(theParent);
  QGridLayout* aLayout = new QGridLayout(aMainGrp);
  aLayout->setMargin(0);
  aLayout->setSpacing(SPACING);

  // elem type
  myElemTypeBox = new QGroupBox(tr("SMESH_ELEMENT_TYPE"), aMainGrp);
  myElemTypeGroup = new QButtonGroup(aMainGrp);
  QHBoxLayout* myElemTypeBoxLayout = new QHBoxLayout(myElemTypeBox);
  myElemTypeBoxLayout->setMargin(MARGIN);
  myElemTypeBoxLayout->setSpacing(SPACING);

  QRadioButton* aEdgeRb   = new QRadioButton( tr("SMESH_EDGE"),   myElemTypeBox);
  QRadioButton* aFaceRb   = new QRadioButton( tr("SMESH_FACE"),   myElemTypeBox);
  QRadioButton* aVolumeRb = new QRadioButton( tr("SMESH_VOLUME"), myElemTypeBox);
  
  myElemTypeBoxLayout->addWidget(aEdgeRb);
  myElemTypeBoxLayout->addWidget(aFaceRb);
  myElemTypeBoxLayout->addWidget(aVolumeRb);
  myElemTypeGroup->addButton(aEdgeRb, 0);
  myElemTypeGroup->addButton(aFaceRb, 1);
  myElemTypeGroup->addButton(aVolumeRb, 2);
  aEdgeRb->setChecked(true);
  
  // element id
  QLabel* anIdLabel = new QLabel( tr("ELEMENT_ID"), aMainGrp );
  myElementId = new QLineEdit( aMainGrp );
  myElementId->setValidator( new SMESHGUI_IdValidator( theParent, 
                                                       !myIsMultipleAllowed ? 1 : 0 ) ); // 0 for any number of entities

  // shape name
  QLabel* aNameLabel = new QLabel( tr("GEOMETRY_NAME"), aMainGrp );
  myGeomName = new QLineEdit( aMainGrp );

  aLayout->addWidget(myElemTypeBox, 0, 0, 1, 2);
  aLayout->addWidget(anIdLabel,   1, 0);
  aLayout->addWidget(myElementId, 1, 1);
  aLayout->addWidget(aNameLabel,  2, 0);
  aLayout->addWidget(myGeomName,  2, 1);

  return aMainGrp;
}

//=======================================================================
// function : ~SMESHGUI_ShapeByMeshDlg()
// purpose  : Destructor
//=======================================================================
SMESHGUI_ShapeByMeshDlg::~SMESHGUI_ShapeByMeshDlg()
{
}

void SMESHGUI_ShapeByMeshDlg:: setMultipleAllowed( bool isAllowed )
{
  myIsMultipleAllowed = isAllowed;
}

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================
SMESHGUI_ShapeByMeshOp::SMESHGUI_ShapeByMeshOp(bool isMultipleAllowed):
  myIsMultipleAllowed(isMultipleAllowed)
{
  if ( GeometryGUI::GetGeomGen()->_is_nil() )// check that GEOM_Gen exists
    GeometryGUI::InitGeomGen();

  myDlg = new SMESHGUI_ShapeByMeshDlg(myIsMultipleAllowed);

  connect(myDlg->myElemTypeGroup, SIGNAL(buttonClicked(int)), SLOT(onTypeChanged(int)));
  connect(myDlg->myElementId, SIGNAL(textChanged(const QString&)), SLOT(onElemIdChanged(const QString&)));
}


//=======================================================================
// function : startOperation()
// purpose  : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_ShapeByMeshOp::startOperation()
{
  //SetMesh( SMESH::SMESH_Mesh::_nil() );
  myIsManualIdEnter = false;

  SMESHGUI_SelectionOp::startOperation();

  //activateSelection(); // set filters
  onSelectionDone(); // desable/enable [ OK ]

  myDlg->show();
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================
SMESHGUI_ShapeByMeshOp::~SMESHGUI_ShapeByMeshOp()
{
  if ( myDlg )
    delete myDlg;
}

//================================================================================
/*!
 * \brief Gets dialog of this operation
 * \retval LightApp_Dialog* - pointer to dialog of this operation
 */
//================================================================================
LightApp_Dialog* SMESHGUI_ShapeByMeshOp::dlg() const
{
  return myDlg;
}

SMESH::SMESH_Mesh_ptr SMESHGUI_ShapeByMeshOp::GetMesh()
{
  return myMesh;
}

//=======================================================================
// function : GetShape()
// purpose  : Return published sub-shape, it must be UnRegister()ed!
//=======================================================================
GEOM::GEOM_Object_ptr SMESHGUI_ShapeByMeshOp::GetShape()
{
  return myGeomObj.in();
}

//=======================================================================
// function : SetMesh()
// purpose  : Set mesh to dialog
//=======================================================================

void SMESHGUI_ShapeByMeshOp::SetMesh (SMESH::SMESH_Mesh_ptr thePtr)
{
  myMesh    = SMESH::SMESH_Mesh::_duplicate(thePtr);
  myGeomObj = GEOM::GEOM_Object::_nil();
  myHasSolids = false;

  std::vector< bool > hasElement (myDlg->myElemTypeGroup->buttons().count(), false);
  if (!myMesh->_is_nil() )
    {
      //     _PTR(SObject) aSobj = SMESH::FindSObject(myMesh.in());
      //     SUIT_DataOwnerPtr anIObj (new LightApp_DataOwner(aSobj->GetID().c_str()));

      std::vector< int > nbShapes( TopAbs_SHAPE, 0 );
      int shapeDim = 0; // max dim with several shapes
      //if ( /*mySelectionMgr*/ selectionMgr()->isOk(anIObj) ) // check that the mesh has a valid shape
      {
        _PTR(SObject) aSO = SMESH::FindSObject(myMesh.in());
        GEOM::GEOM_Object_var mainShape = SMESH::GetGeom(aSO);
        if ( !mainShape->_is_nil() ) 
          {
            TopoDS_Shape aShape;
            if ( GEOMBase::GetShape(mainShape, aShape))
              {
                TopAbs_ShapeEnum types[4] = { TopAbs_EDGE, TopAbs_FACE, TopAbs_SHELL, TopAbs_SOLID };
                for ( int dim = 4; dim > 0; --dim ) {
                  TopAbs_ShapeEnum type = types[ dim - 1 ];
                  TopAbs_ShapeEnum avoid = ( type == TopAbs_SHELL ) ? TopAbs_SOLID : TopAbs_SHAPE;
                  TopExp_Explorer exp( aShape, type, avoid );
                  for ( ; nbShapes[ type ] < 2 && exp.More(); exp.Next() )
                    ++nbShapes[ type ];
                  if ( nbShapes[ type ] > 1 ) {
                    shapeDim = dim;
                    break;
                  }
                }
              }
          }
      }
      if (shapeDim > 0)
        {
          if ( nbShapes[ TopAbs_SHELL ] + nbShapes[ TopAbs_SOLID ] > 1 )
            shapeDim = 3;
          hasElement[ EDGE ]   = shapeDim > 0 && myMesh->NbEdges();
          hasElement[ FACE ]   = shapeDim > 1 && myMesh->NbFaces();
          hasElement[ VOLUME ] = shapeDim > 2 && myMesh->NbVolumes();
        }
      myHasSolids = nbShapes[ TopAbs_SOLID ];
    }

  // disable inexistant elem types
  for ( int i = 0; i < myDlg->myElemTypeGroup->buttons().count(); ++i ) {
    if ( QAbstractButton* button = myDlg->myElemTypeGroup->button( i ) )
      button->setEnabled( hasElement[ i ] );
  }
  myDlg->myElementId->setEnabled( hasElement[ EDGE ] );
  myDlg->myGeomName-> setEnabled( hasElement[ EDGE ] );

  setElementID("");
}

//=======================================================================
// function : commitOperation()
// purpose  : called when "Ok" button pressed.
//=======================================================================

void SMESHGUI_ShapeByMeshOp::commitOperation()
{
  SMESHGUI_SelectionOp::commitOperation();
  try {
    QStringList aListId = myDlg->myElementId->text().split( " ", QString::SkipEmptyParts);
    if (aListId.count() == 1)
    {
      int elemID = (aListId.first()).toInt();
      // GEOM_Object is published -> no need to UnRegister()
      myGeomObj = GEOM::GEOM_Object::_duplicate
        (SMESHGUI::GetSMESHGen()->GetGeometryByMeshElement
         ( myMesh.in(), elemID, myDlg->myGeomName->text().toLatin1().constData()) );
    }
    else
    {
      GEOM::GEOM_Gen_var geomGen = SMESH::GetGEOMGen();
      _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();

      if (geomGen->_is_nil() || !aStudy)
        return;

      GEOM::GEOM_IShapesOperations_wrap aShapesOp =
        geomGen->GetIShapesOperations(aStudy->StudyId());
      if (aShapesOp->_is_nil() )
        return;

      TopAbs_ShapeEnum aGroupType = TopAbs_SHAPE;

      std::map<int, GEOM::GEOM_Object_wrap> aGeomObjectsMap;
      GEOM::GEOM_Object_wrap aGeomObject;

      GEOM::GEOM_Object_var aMeshShape = myMesh->GetShapeToMesh();

      for ( int i = 0; i < aListId.count(); i++ )
      {
        aGeomObject = // received object need UnRegister()!
          SMESHGUI::GetSMESHGen()->FindGeometryByMeshElement(myMesh.in(), aListId[i].toInt());

        if (aGeomObject->_is_nil()) continue;

        int anId = aShapesOp->GetSubShapeIndex(aMeshShape, aGeomObject);
        if (aShapesOp->IsDone() && !aGeomObjectsMap.count(anId) )
        {
          aGeomObjectsMap[anId] = aGeomObject;

          TopAbs_ShapeEnum aSubShapeType = (TopAbs_ShapeEnum)aGeomObject->GetShapeType();
          if (i == 0)
            aGroupType = aSubShapeType;
          else if (aSubShapeType != aGroupType)
            aGroupType = TopAbs_SHAPE;
        }
      }

      int aNumberOfGO = aGeomObjectsMap.size();
      if (aNumberOfGO == 1)
      {
        aGeomObject = (*aGeomObjectsMap.begin()).second;
      }
      else if (aNumberOfGO > 1)
      {
        GEOM::GEOM_IGroupOperations_wrap aGroupOp =
          geomGen->GetIGroupOperations(aStudy->StudyId());
        if(aGroupOp->_is_nil())
          return;

        GEOM::ListOfGO_var aGeomObjects = new GEOM::ListOfGO();
        aGeomObjects->length( aNumberOfGO );

        int i = 0;
        std::map<int, GEOM::GEOM_Object_wrap>::iterator anIter;
        for (anIter = aGeomObjectsMap.begin(); anIter!=aGeomObjectsMap.end(); anIter++)
          aGeomObjects[i++] = GEOM::GEOM_Object::_duplicate( (*anIter).second.in() );

        //create geometry group
        aGeomObject = aGroupOp->CreateGroup(aMeshShape, aGroupType);
        aGroupOp->UnionList(aGeomObject, aGeomObjects);

        if (!aGroupOp->IsDone())
          return;
      }

      // publish the GEOM object in study
      QString aNewGeomGroupName ( myDlg->myGeomName->text() );
      SALOMEDS::Study_var aStudyVar = _CAST(Study,aStudy)->GetStudy();
      SALOMEDS::SObject_wrap aNewGroupSO =
        geomGen->AddInStudy( aStudyVar, aGeomObject, 
                             aNewGeomGroupName.toLatin1().data(), aMeshShape);

      // get a GEOM_Object already published, which doesn't need UnRegister()
      CORBA::Object_var obj = aNewGroupSO->GetObject();
      myGeomObj = GEOM::GEOM_Object::_narrow( obj );
    }
  }
  catch (const SALOME::SALOME_Exception& S_ex) {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  }
  catch (...) {
  }

}

bool SMESHGUI_ShapeByMeshOp::onApply()
{
  return true;
}

//=======================================================================
// function : onSelectionDone()
// purpose  : SLOT called when selection changed. Enable/desable [ OK ]
//=======================================================================
void SMESHGUI_ShapeByMeshOp::onSelectionDone()
{
  myDlg->setButtonEnabled( false, QtxDialog::OK );
  setElementID("");

  try {
    SALOME_ListIO aList;
    selectionMgr()->selectedObjects(aList);
    if (!myIsMultipleAllowed && aList.Extent() != 1)
      return;

    SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(aList.First());
    if (aMesh->_is_nil() || myMesh->_is_nil() || aMesh->GetId() != myMesh->GetId() )
      return;

    QString aString;
    int nbElems = SMESH::GetNameOfSelectedElements(selector(),//myViewWindow->GetSelector(),
                                                   aList.First(), aString);
    if (nbElems > 0) {
      if (!myIsMultipleAllowed && nbElems != 1 )
        return;
      setElementID( aString );
      myDlg->setButtonEnabled( true, QtxDialog::OK );
    }
  } catch (...) {
  }
}

//=======================================================================
// function : activateSelection()
// purpose  : Activate selection in accordance with current pattern type
//=======================================================================
void SMESHGUI_ShapeByMeshOp::activateSelection()
{
  selectionMgr()->clearFilters();
  //SMESH::SetPointRepresentation(false);

  myDlg->myGeomName->setText("");

  QString geomName;
  Selection_Mode mode = EdgeSelection;
  switch ( myDlg->myElemTypeGroup->checkedId() ) {
  case EDGE  :
    mode = EdgeSelection;   geomName = tr("GEOM_EDGE"); break;
  case FACE  :
    mode = FaceSelection;   geomName = tr("GEOM_FACE"); break;
  case VOLUME:
    mode = VolumeSelection; geomName = tr(myHasSolids ? "GEOM_SOLID" : "GEOM_SHELL"); break;
  default: return;
  }
  if ( selectionMode() != mode )
    setSelectionMode( mode );

  myDlg->myGeomName->setText( GEOMBase::GetDefaultName( geomName ));
}

//=======================================================================
//function : onTypeChanged
//purpose  : SLOT. Called when element type changed.
//=======================================================================

void SMESHGUI_ShapeByMeshOp::onTypeChanged (int theType)
{
  setElementID("");
  activateSelection();
}

//=======================================================================
//function : onTypeChanged
//purpose  : SLOT. Called when element id is entered
//           Highlight the element whose Ids the user entered manually
//=======================================================================

void SMESHGUI_ShapeByMeshOp::onElemIdChanged(const QString& theNewText)
{
  myDlg->setButtonEnabled( false, QtxDialog::OK );

  if ( myIsManualIdEnter && !myMesh->_is_nil() )
  {
    if ( SMESH_Actor* actor = SMESH::FindActorByObject(myMesh) )
    {
      if ( SMDS_Mesh* aMesh = actor->GetObject()->GetMesh() )
      {
        SMDSAbs_ElementType type = SMDSAbs_Edge;
        switch ( myDlg->myElemTypeGroup->checkedId() ) {
        case EDGE  : type = SMDSAbs_Edge;   break;
        case FACE  : type = SMDSAbs_Face;   break;
        case VOLUME: type = SMDSAbs_Volume; break;
        default: return;
        }
        TColStd_MapOfInteger newIndices;
        QStringList aListId = theNewText.split( " ", QString::SkipEmptyParts);
        for ( int i = 0; i < aListId.count(); i++ ) {
          if ( const SMDS_MeshElement * e = aMesh->FindElement( aListId[ i ].toInt() ))
            if ( e->GetType() == type )
              newIndices.Add( e->GetID() );
        }
        
        if ( !newIndices.IsEmpty() )
        {
          if (!myIsMultipleAllowed && newIndices.Extent() != 1)
            return;
          if ( SVTK_Selector* s = selector() ) {
            s->AddOrRemoveIndex( actor->getIO(), newIndices, false );
            viewWindow()->highlight( actor->getIO(), true, true );
            myDlg->setButtonEnabled( true, QtxDialog::OK );
          }
        }
      }
    }
  }
}

//=======================================================================
//function : setElementID
//purpose  : programmatically set element id
//=======================================================================

void SMESHGUI_ShapeByMeshOp::setElementID(const QString& theText)
{
  myIsManualIdEnter = false;
  myDlg->myElementId->setText(theText);
  myIsManualIdEnter = true;
}
