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
//  File   : SMESHGUI_MeshInfosDlg.cxx
//  Author : Nicolas BARBEROU
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESHGUI_MeshInfosDlg.h"

#include "SMESHGUI.h"
#include "QAD_Application.h"
#include "QAD_Desktop.h"
#include "utilities.h"

// QT Includes
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qmap.h>

/* 
 *  Constructs a SMESHGUI_MeshInfosDlg which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
SMESHGUI_MeshInfosDlg::SMESHGUI_MeshInfosDlg( QWidget* parent,  const char* name, SALOME_Selection* Sel, bool modal, WFlags fl )
    : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
    if ( !name )
	setName( "SMESHGUI_MeshInfosDlg" );
    setCaption( tr( "SMESH_MESHINFO_TITLE"  ) );
    setSizeGripEnabled( TRUE );

    SMESHGUI_MeshInfosDlgLayout = new QVBoxLayout( this ); 
    SMESHGUI_MeshInfosDlgLayout->setSpacing( 6 );
    SMESHGUI_MeshInfosDlgLayout->setMargin( 11 );

    /****************************************************************/
    GroupBox1 = new QGroupBox( this, "GroupBox1" );
    GroupBox1->setTitle( tr( "SMESH_MESHINFO_NB1D"  ) );
    GroupBox1->setColumnLayout(0, Qt::Vertical );
    GroupBox1->layout()->setSpacing( 0 );
    GroupBox1->layout()->setMargin( 0 );
    QGridLayout* GroupBox1Layout = new QGridLayout( GroupBox1->layout() );
    GroupBox1Layout->setAlignment( Qt::AlignTop );
    GroupBox1Layout->setSpacing( 6 );
    GroupBox1Layout->setMargin( 11 );

    TextLabel11 = new QLabel( GroupBox1, "TextLabel11" );
    TextLabel11->setMinimumWidth( 100 );
    TextLabel11->setText( tr( "SMESH_MESHINFO_NODES"  ) );
    GroupBox1Layout->addWidget( TextLabel11, 0, 0 );

    TextLabel12 = new QLabel( GroupBox1, "TextLabel12" );
    TextLabel12->setMinimumWidth( 100 );
    TextLabel12->setText( tr( "SMESH_MESHINFO_EDGES"  ) );
    GroupBox1Layout->addWidget( TextLabel12, 1, 0 );

    TextLabel13 = new QLabel( GroupBox1, "TextLabel13" );
    TextLabel13->setMinimumWidth( 100 );
    GroupBox1Layout->addWidget( TextLabel13, 0, 1 );

    TextLabel14 = new QLabel( GroupBox1, "TextLabel14" );
    TextLabel14->setMinimumWidth( 100 );
    GroupBox1Layout->addWidget( TextLabel14, 1, 1 );
    SMESHGUI_MeshInfosDlgLayout->addWidget( GroupBox1 );

    /****************************************************************/
    GroupBox2 = new QGroupBox( this, "GroupBox2" );
    GroupBox2->setTitle( tr( "SMESH_MESHINFO_NB2D"  ) );
    GroupBox2->setColumnLayout(0, Qt::Vertical );
    GroupBox2->layout()->setSpacing( 0 );
    GroupBox2->layout()->setMargin( 0 );
    QGridLayout* GroupBox2Layout = new QGridLayout( GroupBox2->layout() );
    GroupBox2Layout->setAlignment( Qt::AlignTop );
    GroupBox2Layout->setSpacing( 6 );
    GroupBox2Layout->setMargin( 11 );

    TextLabel21 = new QLabel( GroupBox2, "TextLabel21" );
    TextLabel21->setMinimumWidth( 100 );
    TextLabel21->setText( tr( "SMESH_MESHINFO_TRIANGLES"  ) );
    GroupBox2Layout->addWidget( TextLabel21, 0, 0 );

    TextLabel22 = new QLabel( GroupBox2, "TextLabel22" );
    TextLabel22->setMinimumWidth( 100 );
    TextLabel22->setText( tr( "SMESH_MESHINFO_QUADRANGLES"  ) );
    GroupBox2Layout->addWidget( TextLabel22, 1, 0 );

    TextLabel23 = new QLabel( GroupBox2, "TextLabel23" );
    TextLabel23->setMinimumWidth( 100 );
    GroupBox2Layout->addWidget( TextLabel23, 0, 1 );
    
    TextLabel24 = new QLabel( GroupBox2, "TextLabel24" );
    TextLabel24->setMinimumWidth( 100 );
    GroupBox2Layout->addWidget( TextLabel24, 1, 1 );
    SMESHGUI_MeshInfosDlgLayout->addWidget( GroupBox2 );

    /****************************************************************/
    GroupBox3 = new QGroupBox( this, "GroupBox3" );
    GroupBox3->setTitle( tr( "SMESH_MESHINFO_NB3D"  ) );
    GroupBox3->setColumnLayout(0, Qt::Vertical );
    GroupBox3->layout()->setSpacing( 0 );
    GroupBox3->layout()->setMargin( 0 );
    QGridLayout* GroupBox3Layout = new QGridLayout( GroupBox3->layout() );
    GroupBox3Layout->setAlignment( Qt::AlignTop );
    GroupBox3Layout->setSpacing( 6 );
    GroupBox3Layout->setMargin( 11 );

    TextLabel31 = new QLabel( GroupBox3, "TextLabel31" );
    TextLabel31->setMinimumWidth( 100 );
    TextLabel31->setText( tr( "SMESH_MESHINFO_TETRAS"  ) );
    GroupBox3Layout->addWidget( TextLabel31, 0, 0 );

    TextLabel32 = new QLabel( GroupBox3, "TextLabel32" );
    TextLabel32->setMinimumWidth( 100 );
    TextLabel32->setText( tr( "SMESH_MESHINFO_HEXAS"  ) );
    GroupBox3Layout->addWidget( TextLabel32, 1, 0 );

    TextLabel33 = new QLabel( GroupBox3, "TextLabel33" );
    TextLabel33->setMinimumWidth( 100 );
    GroupBox3Layout->addWidget( TextLabel33, 0, 1 );

    TextLabel34 = new QLabel( GroupBox3, "TextLabel34" );
    TextLabel34->setMinimumWidth( 100 );
    GroupBox3Layout->addWidget( TextLabel34, 1, 1 );
    SMESHGUI_MeshInfosDlgLayout->addWidget( GroupBox3 );

    /****************************************************************/
    QGroupBox* GroupButtons = new QGroupBox( this, "GroupButtons" );
    GroupButtons->setColumnLayout(0, Qt::Vertical );
    GroupButtons->layout()->setSpacing( 0 );
    GroupButtons->layout()->setMargin( 0 );
    QGridLayout* GroupButtonsLayout = new QGridLayout( GroupButtons->layout() );
    GroupButtonsLayout->setAlignment( Qt::AlignTop );
    GroupButtonsLayout->setSpacing( 6 );
    GroupButtonsLayout->setMargin( 11 );

    GroupButtonsLayout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ), 0, 0 );
    buttonOk = new QPushButton( GroupButtons, "buttonOk" );
    buttonOk->setText( tr( "SMESH_BUT_OK"  ) );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    GroupButtonsLayout->addWidget( buttonOk, 0, 1 );
    GroupButtonsLayout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ), 0, 2 );
    SMESHGUI_MeshInfosDlgLayout->addWidget( GroupButtons );
    /****************************************************************/

    Init( Sel ) ; 
}

/*  
 *  Destroys the object and frees any allocated resources
 */
SMESHGUI_MeshInfosDlg::~SMESHGUI_MeshInfosDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_MeshInfosDlg::Init( SALOME_Selection* Sel )
{  
  mySelection = Sel ;

  mySMESHGUI = SMESHGUI::GetSMESHGUI() ;
  myStudy = mySMESHGUI->GetActiveStudy()->getStudyDocument();

  Engines::Component_var comp = QAD_Application::getDesktop()->getEngine("FactoryServer", "SMESH");
  myCompMesh = SMESH::SMESH_Gen::_narrow(comp);

  int nbSel = mySelection->IObjectCount();

  TextLabel13->setText( "0" );
  TextLabel14->setText( "0" );
  TextLabel23->setText( "0" );
  TextLabel24->setText( "0" );
  TextLabel33->setText( "0" );
  TextLabel34->setText( "0" );

  //gets the selected mesh
  if ( nbSel == 1 ) {
    Handle(SALOME_InteractiveObject) IObject = mySelection->firstIObject();
    Standard_Boolean res;
    myMesh = mySMESHGUI->ConvertIOinMesh( IObject, res );
    if ( res )
      DumpMeshInfos();
  }

  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  /* signals and slots connections */
  connect( buttonOk,    SIGNAL( clicked() ),                      this, SLOT( ClickOnOk() ) );
  connect( mySMESHGUI,  SIGNAL( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;
  connect( mySelection, SIGNAL( currentSelectionChanged() ),      this, SLOT( SelectionIntoArgument() ) );
  /* to close dialog if study change */
  connect( mySMESHGUI,  SIGNAL ( SignalCloseAllDialogs() ),       this, SLOT( ClickOnOk() ) ) ;
 
  /* Move widget on the botton right corner of main widget */
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ; /* Displays Dialog */ 

  return ;
}

//=================================================================================
// function : genEdgeKey
// purpose  : edge counting helper;
//            packs two long integers into one 8-byte value (treated as double by the caller);
//            the order of arguments is insignificant
//=================================================================================
void genEdgeKey(long a, long b, void* key)
{
  long* lKey = (long*)key;
  *lKey     = (a < b) ? a : b;
  *(++lKey) = (a < b) ? b : a;
}

//=================================================================================
// function : DumpMeshInfos()
// purpose  :
//=================================================================================
void SMESHGUI_MeshInfosDlg::DumpMeshInfos()
{
  int nbOfNodes = myMesh->NbNodes();
  int nbOfEdges = myMesh->NbEdges();
  int nbOfTriangles = myMesh->NbTriangles();
  int nbOfQuadrangles = myMesh->NbQuadrangles();
  int nbOfTetras = myMesh->NbTetras();
  int nbOfHexas = myMesh->NbHexas();

  /*
  int nbOfNodes = 0 ;
  int nbOfEdges = 0 ;
  int nbOfTriangles = 0 ;
  int nbOfQuadrangles = 0 ;
  int nbOfTetras = 0 ;
  int nbOfHexas = 0 ;
  int nbCells = 0 ;
  int CellType = 0 ;
  QMap<double, char> aMapOfEdges;

  Standard_Boolean result;
  SMESH_Actor* MeshActor = mySMESHGUI->FindActor(myMesh, result, true);

  if ( result ) {
    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( MeshActor->DataSource );
    vtkPoints *Pts = ugrid->GetPoints();
    nbOfNodes = Pts->GetNumberOfPoints();
    int nbCells = ugrid->GetNumberOfCells();

    for ( int i = 0; i < nbCells; i++ ) {
      vtkCell* cellPtr = ugrid->GetCell(i);
      CellType = cellPtr->GetCellType();
      switch (CellType)
	{
	case 3: //Edges
	  {
	    nbOfEdges++;
	    break;
	  }
	case 5: //Triangles
	  {
	    nbOfTriangles++;
	    
	    for (int edgeNum = 0; edgeNum < 3; edgeNum++) {
	      vtkCell* edgePtr = cellPtr->GetEdge(edgeNum);
	      double anEdgeKey;
	      genEdgeKey(edgePtr->GetPointId(0), edgePtr->GetPointId(1), &anEdgeKey);
	      if (!aMapOfEdges.contains(anEdgeKey)) {
		nbOfEdges++;
		aMapOfEdges.insert(anEdgeKey, 0);
	      }
	    }
	    break;
	  }
	case 9: //Quadrangles
	  {
	    nbOfQuadrangles++;
	    
	    for (int edgeNum = 0; edgeNum < 4; edgeNum++) {
	      vtkCell* edgePtr = cellPtr->GetEdge(edgeNum);
	      double anEdgeKey;
	      genEdgeKey(edgePtr->GetPointId(0), edgePtr->GetPointId(1), &anEdgeKey);
	      if (!aMapOfEdges.contains(anEdgeKey)) {
		nbOfEdges++;
		aMapOfEdges.insert(anEdgeKey, 0);
	      }
	    }
	    break;
	  }
	case 10: //Tetraedras
	  {
	    nbOfTetras++;

	    for (int edgeNum = 0; edgeNum < 6; edgeNum++) {
	      vtkCell* edgePtr = cellPtr->GetEdge(edgeNum);
	      double anEdgeKey;
	      genEdgeKey(edgePtr->GetPointId(0), edgePtr->GetPointId(1), &anEdgeKey);
	      if (!aMapOfEdges.contains(anEdgeKey)) {
		nbOfEdges++;
		aMapOfEdges.insert(anEdgeKey, 0);
	      }
	    }
	    break;
	  }
	case 12: //Hexahedras
	  {
	    nbOfHexas++;

	    for (int edgeNum = 0; edgeNum < 12; edgeNum++) {
	      vtkCell* edgePtr = cellPtr->GetEdge(edgeNum);
	      double anEdgeKey;
	      genEdgeKey(edgePtr->GetPointId(0), edgePtr->GetPointId(1), &anEdgeKey);
	      if (!aMapOfEdges.contains(anEdgeKey)) {
		nbOfEdges++;
		aMapOfEdges.insert(anEdgeKey, 0);
	      }
	    }
	    break;
	  }
	}
    }
  }
  */
  TextLabel13->setText( tr( "%1" ).arg(nbOfNodes) );
  TextLabel14->setText( tr( "%1" ).arg(nbOfEdges) );
  TextLabel23->setText( tr( "%1" ).arg(nbOfTriangles) );
  TextLabel24->setText( tr( "%1" ).arg(nbOfQuadrangles) );
  TextLabel33->setText( tr( "%1" ).arg(nbOfTetras) );
  TextLabel34->setText( tr( "%1" ).arg(nbOfHexas) );
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_MeshInfosDlg::ClickOnOk()
{
  disconnect( mySelection, 0, this, 0 );
  mySMESHGUI->ResetState() ;
  reject() ;
  return ;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection has changed
//=================================================================================
void SMESHGUI_MeshInfosDlg::SelectionIntoArgument()
{
  TextLabel13->setText( "0" );
  TextLabel14->setText( "0" );
  TextLabel23->setText( "0" );
  TextLabel24->setText( "0" );
  TextLabel33->setText( "0" );
  TextLabel34->setText( "0" );

  int nbSel = mySelection->IObjectCount();
  if ( nbSel == 1 ) {
    Handle(SALOME_InteractiveObject) IObject = mySelection->firstIObject();
    Standard_Boolean res;
    myMesh = mySMESHGUI->ConvertIOinMesh( IObject, res );
    if ( res ) 
      DumpMeshInfos();
  }
  return ; 
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MeshInfosDlg::closeEvent( QCloseEvent* e )
{
  disconnect( mySelection, 0, this, 0 );
  mySMESHGUI->ResetState() ;
  reject() ;
  return ;
}


//=================================================================================
// function : enterEvent()
// purpose  : when mouse enter onto the QWidget
//=================================================================================
void SMESHGUI_MeshInfosDlg::enterEvent( QEvent *  )
{
  ActivateThisDialog() ;
}


//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_MeshInfosDlg::DeactivateActiveDialog()
{
  disconnect( mySelection, 0, this, 0 );

  return ;
}


//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_MeshInfosDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate any active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog() ;

  return ;
}

