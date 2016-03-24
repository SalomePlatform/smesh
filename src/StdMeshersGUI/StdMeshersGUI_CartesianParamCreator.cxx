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
// File   : StdMeshersGUI_CartesianParamCreator.cxx
// Author : Open CASCADE S.A.S.

// SMESH includes
#include "StdMeshersGUI_CartesianParamCreator.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_HypothesesUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_MeshEditPreview.h"

// IDL includes
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

// SALOME GUI includes
#include <LightApp_SelectionMgr.h>
#include <QtxComboBox.h>
#include <SALOME_InteractiveObject.hxx>
#include <SALOME_ListIO.hxx>
#include <SUIT_ResourceMgr.h>
#include <SalomeApp_IntSpinBox.h>
#include <SalomeApp_Tools.h>

#include <GEOMBase.h>

#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>

// Qt includes
#include <QAbstractItemModel>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QFontMetrics>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QModelIndex>
#include <QPushButton>
#include <QRadioButton>
#include <QString>
#include <QStyleOptionViewItem>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#define SPACING 6
#define MARGIN  11

namespace StdMeshersGUI
{
  enum { COORD_BUT = 0, SPACING_BUT };

  //================================================================================
  /*!
   * \brief get spacing definition from a tree item
   */
  //================================================================================

  void getFromItem(QTreeWidgetItem * item, double& t0, double& t1, QString& fun )
  {
    if ( item )
    {
      t0 = item->text( 0 ).split(' ')[0].toDouble();
      t1 = item->data( 0, Qt::UserRole ).toDouble();
      fun = item->text( 1 );
    }
  }

  //================================================================================
  /*!
   * \brief set spacing definition to a tree item
   */
  //================================================================================

  QTreeWidgetItem* setToItem(double t0, double t1, const QString& fun, QTreeWidgetItem * item)
  {
    if ( !item ) item = new QTreeWidgetItem;
    item->setText( 0, QString( "%1 - %2" ).arg( t0 ).arg( t1 ));
    item->setData( 0, Qt::UserRole, t1 );
    item->setText( 1, fun );
    item->setFlags( item->flags() | Qt::ItemIsEditable );
    return item;
  }

  //================================================================================
  /*!
   * \brief Retrieves coordinate value from a list item
   */
  //================================================================================

  double coordFromItem( QListWidgetItem * item )
  {
    return item ? item->data( Qt::UserRole ).toDouble() : 0;
  }

  //================================================================================
  /*!
   * \brief Sets coordinate value to a list item
   */
  //================================================================================

  QListWidgetItem* coordToItem( double coord, QListWidgetItem * item )
  {
    if ( !item ) item = new QListWidgetItem;
    item->setText( QString::number( coord ));
    item->setData( Qt::UserRole, coord );
    item->setFlags( item->flags() | Qt::ItemIsEditable );
    return item;
  }

  //================================================================================
  /*!
   * \brief Constructor
   * \param theParent - Parent widget for this tab
   * 
   * Makes tab's look and feel
   */
  //================================================================================

  GridAxisTab::GridAxisTab( QWidget* theParent,const int axisIndex ):
    QFrame( theParent ), myAxisIndex( axisIndex )
  {
    // 1) Grid definition mode
    myModeGroup = new QButtonGroup( this );
    QGroupBox* modeBox = new QGroupBox( tr( "GRID_DEF_MODE" ), this );
    QHBoxLayout* modeLay = new QHBoxLayout( modeBox );
    modeLay->setMargin( MARGIN );
    modeLay->setSpacing( SPACING );

    QRadioButton* coordModeBtn = new QRadioButton( tr( "SMESH_COORDINATES" ), modeBox );
    QRadioButton* spacModeBtn  = new QRadioButton( tr( "SPACING" ), modeBox );

    modeLay->addWidget( coordModeBtn );
    modeLay->addWidget( spacModeBtn );
    myModeGroup->addButton( coordModeBtn, COORD_BUT );
    myModeGroup->addButton( spacModeBtn,  SPACING_BUT );

    // 2) Buttons + Step
    myInsertBtn = new QPushButton( tr("INSERT"), this);
    myDeleteBtn = new QPushButton( tr("SMESH_BUT_DELETE"), this);

    myStepLabel = new QLabel( tr("COORD_STEP"));
    myStepSpin  = new SMESHGUI_SpinBox( this );
    myStepSpin->setAcceptNames( false ); // No Notebook variables allowed
    myStepSpin->RangeStepAndValidator();
    myStepSpin->SetStep( 1. );
    myStepSpin->SetValue( myStep = 1. );

    // 3) Coodrinates/Spacing group
    QFrame*    csFrame = new QFrame( this );
    QVBoxLayout* scLay = new QVBoxLayout( csFrame );
    scLay->setMargin( 0 );
    scLay->setSpacing( SPACING );

    // 3.1) Spacing
    mySpacingTreeWdg = new QTreeWidget( csFrame );
    mySpacingTreeWdg->setColumnCount(2);
    mySpacingTreeWdg->setHeaderLabels( QStringList() << tr( "SMESH_RANGE" ) << QString( "f(t)" ));
    mySpacingTreeWdg->setColumnWidth( 1, 40 );
    mySpacingTreeWdg->setColumnWidth( 2, 30 );
    mySpacingTreeWdg->setItemDelegate( new LineDelegate( mySpacingTreeWdg ));
    scLay->addWidget( mySpacingTreeWdg );

    // 3.2) Coordinates
    myCoordList = new QListWidget( csFrame );
    myCoordList->setItemDelegate( new LineDelegate( myCoordList ));
    scLay->addWidget( myCoordList );

    // layouting

    QGridLayout* axisTabLayout = new QGridLayout( this );
    axisTabLayout->setMargin( MARGIN );
    axisTabLayout->setSpacing( SPACING );

    axisTabLayout->addWidget( modeBox    , 0, 0, 1, 3 );
    axisTabLayout->addWidget( myInsertBtn  , 1, 0, 1, 2 );
    axisTabLayout->addWidget( myDeleteBtn  , 2, 0, 1, 2 );
    axisTabLayout->addWidget( myStepLabel, 3, 0 );
    axisTabLayout->addWidget( myStepSpin , 3, 1 );
    axisTabLayout->addWidget( csFrame    , 1, 2, 4, 1 );

    axisTabLayout->setRowStretch( 4, 1 );

    // signals
    connect( myInsertBtn,      SIGNAL( clicked() ),             SLOT( onInsert() ));
    connect( myDeleteBtn,      SIGNAL( clicked() ),             SLOT( onDelete() ));
    connect( myModeGroup,      SIGNAL( buttonClicked ( int )),  SLOT( onMode(int)));
    connect( myModeGroup,      SIGNAL( buttonClicked ( int )),  SIGNAL( gridModeChanged(int)));
    connect( mySpacingTreeWdg, SIGNAL( itemSelectionChanged()), SLOT( updateButtons() ));
    connect( myCoordList,      SIGNAL( itemSelectionChanged()), SLOT( updateButtons() ));
    connect( myStepSpin,       SIGNAL( valueChanged(double)),   SLOT( onStepChange() ));
  }

  //================================================================================
  /*!
   * \brief SLOT onInsert
   */
  //================================================================================

  void GridAxisTab::onInsert()
  {
    if ( isGridBySpacing() )
    {
      QTreeWidgetItem * item = mySpacingTreeWdg->currentItem();
      if ( !item ) item = mySpacingTreeWdg->topLevelItem( 0 );
      int i = mySpacingTreeWdg->indexOfTopLevelItem( item );

      double t0, t1; QString fun;
      getFromItem( item, t0, t1, fun );
      double t = 0.5 * ( t0 + t1 );
      setToItem( t0, t, fun, item );

      item = setToItem( t, t1, fun );
      if ( i == mySpacingTreeWdg->topLevelItemCount()-1 )
        mySpacingTreeWdg->addTopLevelItem( item );
      else
        mySpacingTreeWdg->insertTopLevelItem( i+1, item );
      mySpacingTreeWdg->setCurrentItem( item );
    }
    else
    {
      if ( myCoordList->count() == 0 )
      {
        myCoordList->addItem( coordToItem( 0 ));
      }
      else
      {
        double coord = coordFromItem( myCoordList->currentItem() ) + myStep;
        int i = myCoordList->currentRow();
        while ( i > 0 && coordFromItem( myCoordList->item( i-1 )) > coord )
          --i;
        while ( i < myCoordList->count() && coordFromItem( myCoordList->item( i )) < coord )
          ++i;
        const double tol = 1e-6;
        const bool isSame = 
          ( i < myCoordList->count() && coordFromItem( myCoordList->item( i )) - coord < tol ) ||
          ( i > 0 && coord - coordFromItem( myCoordList->item( i-1 )) < tol );
        if ( !isSame )
          myCoordList->insertItem( i, coordToItem( coord ));
        else if ( myStep < 0 )
          --i;
        myCoordList->setCurrentRow( i );
      }
    }
    updateButtons();
  }

  //================================================================================
  /*!
   * \brief SLOT onDelete
   */
  //================================================================================

  void GridAxisTab::onDelete()
  {
    if ( isGridBySpacing() )
    {
      QList<QTreeWidgetItem *> selItems = mySpacingTreeWdg->selectedItems();
      QTreeWidgetItem * item;
      foreach ( item, selItems )
      {
        int i = mySpacingTreeWdg->indexOfTopLevelItem( item );
        if ( i == 0 ) continue; 
        QTreeWidgetItem* prevItem = mySpacingTreeWdg->topLevelItem( i-1 );

        double t0, t1, t2; QString fun;
        getFromItem( item, t1, t2, fun );
        getFromItem( prevItem, t0, t1, fun );
        delete item;

        setToItem( t0, t2, fun, prevItem );
      }
    }
    else
    {
      if ( myCoordList->count() > 2 )
        if ( QListWidgetItem * item = myCoordList->currentItem() )
          delete item;
    }
    updateButtons();
  }

  //================================================================================
  /*!
   * \brief SLOT onMode
   */
  //================================================================================

  void GridAxisTab::onMode(int isSpacing)
  {
    mySpacingTreeWdg->setVisible( isSpacing );
    myCoordList->setVisible( !isSpacing );
    myStepSpin->setVisible( !isSpacing );
    myStepLabel->setVisible( !isSpacing );
    if ( isSpacing )
    {
      if ( mySpacingTreeWdg->topLevelItemCount() == 0 )
      {
        QString spacing( "1" );
        if ( myCoordList->count() > 1 )
        {
          double c1 = coordFromItem( myCoordList->item( 1 ));
          double c0 = coordFromItem( myCoordList->item( 0 ));
          spacing = QString::number( c1 - c0 );
        }
        mySpacingTreeWdg->addTopLevelItem( setToItem( 0., 1., spacing ) );
      }
      //myCoordList->clear();
    }
    else
    {
      //mySpacingTreeWdg->clear();
      if ( myCoordList->count() == 0 )
        myCoordList->addItem( coordToItem( 0 ));
    }
    updateButtons();
  }

  //================================================================================
  /*!
   * \brief SLOT onStepChange
   */
  //================================================================================

  void GridAxisTab::onStepChange()
  {
    if ( fabs( myStepSpin->GetValue() ) < 1e-100 )
    {
      double delta = myStepSpin->singleStep() * ( myStep > myStepSpin->GetValue() ? -1 : +1 );
      myStepSpin->SetValue( myStepSpin->GetValue() + delta );
    }
    myStep = myStepSpin->GetValue();
  }

  //================================================================================
  /*!
   * \brief Enables/disables buttons
   */
  //================================================================================

  void GridAxisTab::updateButtons()
  {
    bool insertEnable = false, deleteEnable = false;
    if ( isGridBySpacing() )
    {
      insertEnable = true;
      const int nbSelected = mySpacingTreeWdg->selectedItems().count();
      if ( nbSelected > 0 )
      {
        // we delete a current range by uniting it with the previous
        int i = mySpacingTreeWdg->indexOfTopLevelItem(  mySpacingTreeWdg->currentItem() );
        deleteEnable = ( i > 0 );
      }
    }
    else
    {
      const int nbSelected = myCoordList->selectedItems().count();
      insertEnable = ( nbSelected || myCoordList->count() < 2 );
      deleteEnable = ( nbSelected && myCoordList->count() > 2 );
    }
    myInsertBtn->setEnabled( insertEnable );
    myDeleteBtn->setEnabled( deleteEnable );
  }

  //================================================================================
  /*!
   * \brief Inserts coordinates into myCoordList
   */
  //================================================================================

  void GridAxisTab::setCoordinates( SMESH::double_array_var coords )
  {
    myCoordList->clear();
    for ( size_t i = 0; i < coords->length(); ++i )
      myCoordList->addItem( coordToItem( coords[i] ));

    myModeGroup->button( COORD_BUT )->setChecked( true );
    onMode( COORD_BUT );
  }

  //================================================================================
  /*!
   * \brief Sets spacing got from hypothesis
   */
  //================================================================================

  void GridAxisTab::setSpacing( SMESH::string_array_var funs, SMESH::double_array_var points )
  {
    mySpacingTreeWdg->clear();
    if ( funs->length() == points->length() - 1 )
    {
      for ( size_t i = 1; i < points->length(); ++i )
        mySpacingTreeWdg->addTopLevelItem
          ( setToItem( points[i-1], points[i], (const char*) funs[i-1] ));
    }
    myModeGroup->button( SPACING_BUT )->setChecked( true );
    onMode( SPACING_BUT );
  }

  //================================================================================
  /*!
   * \brief Checks grid definintion mode
   */
  //================================================================================

  bool GridAxisTab::isGridBySpacing() const
  {
    return ( myModeGroup->checkedId() == SPACING_BUT );
  }

  //================================================================================
  /*!
   * \brief Returns coordinates to set to a hypothesis
   */
  //================================================================================

  SMESH::double_array* GridAxisTab::getCoordinates()
  {
    SMESH::double_array_var coords = new SMESH::double_array;
    coords->length( myCoordList->count() );
    for ( size_t i = 0; i < coords->length(); ++i )
      coords[i] = coordFromItem( myCoordList->item( i ) );

    return coords._retn();
  }

  //================================================================================
  /*!
   * \brief Returms spacing to set to a hypothesis
   */
  //================================================================================

  void GridAxisTab::getSpacing(SMESH::string_array_out funs,
                               SMESH::double_array_out points) const
  {
    funs =  new SMESH::string_array();
    points = new SMESH::double_array();
    funs->length( mySpacingTreeWdg->topLevelItemCount() );
    points->length( mySpacingTreeWdg->topLevelItemCount() + 1 );
    double t0, t1; QString fun;
    for ( size_t i = 0; i < funs->length(); ++i )
    {
      QTreeWidgetItem* item = mySpacingTreeWdg->topLevelItem( i );
      getFromItem( item, t0, t1, fun );
      points[i] = t0;
      funs[i] = fun.toLatin1().constData();
    }
    points[ points->length()-1 ] = 1.0;
  }


  //================================================================================
  /*!
   * \brief Verifies parameters
   */
  //================================================================================

  bool GridAxisTab::checkParams(QString& msg, SMESH::SMESH_Hypothesis_var& hyp) const
  {
    if ( isGridBySpacing() )
    {
      if ( mySpacingTreeWdg->topLevelItemCount() == 0 )
        return false; // how could it be?
      StdMeshers::StdMeshers_CartesianParameters3D_var h =
        StdMeshers::StdMeshers_CartesianParameters3D::_narrow( hyp );
      SMESH::string_array_var funs;
      SMESH::double_array_var points;
      getSpacing( funs.out(), points.out() );
      try {
        const char* axisName[3] = { "X", "Y", "Z" };
        SMESH::double_array_var coords =
          h->ComputeCoordinates(0.,1., funs, points, axisName[ myAxisIndex ]);
      }
      catch ( const SALOME::SALOME_Exception& ex ) {
        msg = (const char*) ex.details.text;
        return false;
      }
    }
    else
    {
      return myCoordList->count() > 1;
    }
    return true;
  }

  //================================================================================
  /*!
   * \brief LineDelegate constructor
   */
  //================================================================================

  LineDelegate::LineDelegate( QWidget* parent ):
    QItemDelegate( parent ),
    mySpacingTreeWdg( qobject_cast<QTreeWidget*>( parent )),
    myCoordList( qobject_cast<QListWidget*>( parent ))
  {
  }

  //================================================================================
  /*!
   * \brief Creates an editor depending on a current item
   */
  //================================================================================

  QWidget* LineDelegate::createEditor( QWidget*                    parent,
                                       const QStyleOptionViewItem& opt,
                                       const QModelIndex&          index) const
  {
    QWidget* w = 0;
    if ( mySpacingTreeWdg )
    {
      if ( index.column() == 0 &&
           index.row() != mySpacingTreeWdg->topLevelItemCount()-1 )
      {
        SMESHGUI_SpinBox* sb = new SMESHGUI_SpinBox( parent );
        sb->setAcceptNames( false ); // No Notebook variables allowed
        sb->setFrame( false );
        w = sb;
      }
      if ( index.column() == 1 ) {
        w = new QLineEdit( parent );
      }
    }
    else
    {
      SMESHGUI_SpinBox* sb = new SMESHGUI_SpinBox( parent );
      sb->setAcceptNames( false ); // No Notebook variables allowed
      sb->setFrame( false );
      const double tol = 1e-5;
      double from = index.row() ? coordFromItem( myCoordList->item( index.row()-1 ))+tol : -1e+6;
      double to = index.row() == myCoordList->count()-1 ? 1e+6 : coordFromItem( myCoordList->item( index.row()+1 ))-tol;
      sb->RangeStepAndValidator( from, to, 0.01 );
      w = sb;
    }
    return w;
  }

  //================================================================================
  /*!
   * \brief Limit value range in the spin of a neighbor range
   */
  //================================================================================

  void LineDelegate::setEditorData ( QWidget * editor, const QModelIndex & index ) const
  {
    if ( mySpacingTreeWdg && index.column() == 0 )
    {
      double t0, t1, t2=1.0; QString fun;
      QTreeWidgetItem* item = mySpacingTreeWdg->topLevelItem( index.row() );
      getFromItem( item, t0, t1, fun );
      if ( index.row() != mySpacingTreeWdg->topLevelItemCount()-1 )
      {
        item = mySpacingTreeWdg->topLevelItem( index.row()+1 );
        getFromItem( item, t1, t2, fun );
      }
      const double tol = 1e-3;
      SMESHGUI_SpinBox* sb = qobject_cast<SMESHGUI_SpinBox*>( editor );
      sb->RangeStepAndValidator( t0 + tol, t2 - tol, 0.01 );
      sb->SetValue( t1 );
    }
    else
    {
      QItemDelegate::setEditorData( editor, index );
    }
  }

  //================================================================================
  /*!
   * \brief
   */
  //================================================================================

  void LineDelegate::setModelData( QWidget*            editor,
                                   QAbstractItemModel* model,
                                   const QModelIndex&  index ) const
  {
    if ( mySpacingTreeWdg )
    {
      if ( index.column() == 0 )
      {
        if ( index.row() != mySpacingTreeWdg->topLevelItemCount()-1 )
        {
          SMESHGUI_SpinBox* sb = qobject_cast<SMESHGUI_SpinBox*>( editor );
          double t0, t1, t = sb->GetValue(); QString fun;

          QTreeWidgetItem* item = mySpacingTreeWdg->topLevelItem( index.row() );
          getFromItem( item, t0, t1, fun );
          setToItem( t0, t, fun, item );

          item = mySpacingTreeWdg->topLevelItem( index.row() + 1 );
          getFromItem( item, t0, t1, fun );
          setToItem( t, t1, fun, item );
        }
      }
      else if ( !qobject_cast<QLineEdit*>(editor)->text().trimmed().isEmpty() )
      {
        QItemDelegate::setModelData( editor, model, index );
      }
    }
    else
    {
      SMESHGUI_SpinBox* sb = qobject_cast<SMESHGUI_SpinBox*>( editor );
      coordToItem( sb->GetValue(), myCoordList->item( index.row() ));
    }
  }

} // namespace StdMeshersGUI

namespace
{
  const double theAngTol = M_PI / 180.;

  //================================================================================
  /*!
   * \brief Set variables to groups of spin boxes
   */
  //================================================================================

  void setText( const QString& vars, SMESHGUI_SpinBox** spins )
  {
    QStringList varList = vars.split( ':' );
    for ( int i = 0; i < 3 && i < varList.count(); ++i )
      if ( !varList[i].isEmpty() )
        spins[i]->setText( varList[i] );
  }
  
  //================================================================================
  /*!
   * \brief Computes more 2 axes by one
   *  \param [in] iOk - index of a given axis
   *  \param [in,out] dirs - directions of 3 axes
   */
  //================================================================================

  void get3Dirs( int iOk, gp_XYZ dirs[3] )
  {
    dirs[ ( iOk+1 ) % 3 ] = dirs[ iOk ];

    if ( Abs( dirs[ iOk ].Y() ) < 1e-100 &&
         Abs( dirs[ iOk ].Z() ) < 1e-100 )
      // dirs[ iOk ] || OX
      dirs[ ( iOk+1 ) % 3 ].SetY( dirs[ iOk ].Y() + 1. );
    else
      dirs[ ( iOk+1 ) % 3 ].SetX( dirs[ iOk ].X() + 1. );

    dirs[( iOk+2 ) % 3] = dirs[ iOk ] ^ dirs[ ( iOk+1 ) % 3 ];
    dirs[( iOk+1 ) % 3] = dirs[ ( iOk+2 ) % 3 ] ^ dirs[ iOk ];
  }

  //================================================================================
  /*!
   * \brief Returns a minimal width of a SpinBox depending on a precision type
   */
  //================================================================================

  int getMinWidth( const char* precisionType )
  {
    int nb = SMESHGUI::resourceMgr()->integerValue( "SMESH", precisionType, -3 );
    QString s;
    s.fill('0', qAbs(nb)+7 );
    QLineEdit le;
    QFontMetrics metrics( le.font() );
    return metrics.width( s );
  }
}

//================================================================================
/*!
 * \brief StdMeshersGUI_CartesianParamCreator constructor
 */
//================================================================================

StdMeshersGUI_CartesianParamCreator::StdMeshersGUI_CartesianParamCreator(const QString& aHypType)
  : StdMeshersGUI_StdHypothesisCreator( aHypType ),
    myThreshold( 0 )
{
  myAxisTabs[0] = 0;
  myAxisTabs[1] = 0;
  myAxisTabs[2] = 0;

  myAxesPreview = new SMESHGUI_MeshEditPreview( SMESH::GetViewWindow( SMESHGUI::GetSMESHGUI() ));
  myAxesPreview->SetArrowShapeAndNb( /*nbArrows=*/3,
                                     /*headLength=*/0.1,
                                     /*headRadius=*/0.01,
                                     /*start=*/0.,
                                     /*labels=*/"XYZ");

  myDirTic[0] = myDirTic[1] = myDirTic[2] = 0;
}

//================================================================================
/*!
 * \brief StdMeshersGUI_CartesianParamCreator destructor
 */
//================================================================================

StdMeshersGUI_CartesianParamCreator::~StdMeshersGUI_CartesianParamCreator()
{
  if ( myAxisTabs[0] ) delete myAxisTabs[0];
  if ( myAxisTabs[1] ) delete myAxisTabs[1];
  if ( myAxisTabs[2] ) delete myAxisTabs[2];
  myAxisTabs[0] = 0;
  myAxisTabs[1] = 0;
  myAxisTabs[2] = 0;

  delete myAxesPreview;
}

//================================================================================
/*!
 * \brief Validate parameters
 */
//================================================================================

bool StdMeshersGUI_CartesianParamCreator::checkParams( QString& msg ) const
{
  if( !SMESHGUI_GenericHypothesisCreator::checkParams( msg ) )
    return false;

  if ( myName && myName->text().trimmed().isEmpty() )
  {
    msg = tr("SMESH_WRN_EMPTY_NAME");
    return false;
  }
  if ( ! myThreshold->isValid( msg, true ))
    return false;

  SMESH::SMESH_Hypothesis_var hyp = hypothesis();
  if ( !myAxisTabs[0]->checkParams( msg, hyp )) return false;
  if ( !myAxisTabs[1]->checkParams( msg, hyp )) return false;
  if ( !myAxisTabs[2]->checkParams( msg, hyp )) return false;

  StdMeshersGUI_CartesianParamCreator* me = (StdMeshersGUI_CartesianParamCreator*) this;
  if ( !me->updateAxesPreview() )
  {
    msg = tr("INVALID_AXES_DIR");
    return false;
  }

  return true;
}

//================================================================================
/*!
 * \brief Create widgets
 */
//================================================================================

QFrame* StdMeshersGUI_CartesianParamCreator::buildFrame()
{
  QFrame* fr = new QFrame();
  //fr->setMinimumWidth(460);

  QVBoxLayout* lay = new QVBoxLayout( fr );
  lay->setMargin( 0 );
  lay->setSpacing( SPACING );

  QGroupBox* GroupC1 = new QGroupBox( tr( "SMESH_ARGUMENTS" ), fr );
  lay->addWidget( GroupC1 );

  StdMeshers::StdMeshers_NumberOfSegments_var h =
    StdMeshers::StdMeshers_NumberOfSegments::_narrow( hypothesis() );

  QGridLayout* argGroupLayout = new QGridLayout( GroupC1 );
  argGroupLayout->setSpacing( SPACING );
  argGroupLayout->setMargin( MARGIN );
  argGroupLayout->setColumnStretch( 0, 0 );
  argGroupLayout->setColumnStretch( 1, 1 );

  int row = 0;
  // 0)  name
  myName = 0;
  if( isCreation() )
  {
    myName = new QLineEdit( GroupC1 );
    argGroupLayout->addWidget( new QLabel( tr( "SMESH_NAME" ), GroupC1 ), row, 0 );
    argGroupLayout->addWidget( myName, row, 1 );
    row++;
  }

  // 1)  threshold
  argGroupLayout->addWidget( new QLabel( tr( "THRESHOLD" ), GroupC1 ), row, 0 );
  myThreshold = new SMESHGUI_SpinBox( GroupC1 );
  myThreshold->setAcceptNames( false ); // No Notebook variables allowed
  myThreshold->RangeStepAndValidator( 1.00001, 1e+10, 1., "length_precision" );
  argGroupLayout->addWidget( myThreshold, row, 1 );
  row++;
  
  // 2)  "Implement edges"
  myAddEdges = new QCheckBox( tr("ADD_EDGES"), GroupC1 );
  argGroupLayout->addWidget( myAddEdges, row, 0, 1, 2 );
  row++;

  // 3)  Grid definition
  QTabWidget* tabWdg = new QTabWidget( fr );
  myAxisTabs[ 0 ] = new StdMeshersGUI::GridAxisTab( tabWdg, 0 );
  myAxisTabs[ 1 ] = new StdMeshersGUI::GridAxisTab( tabWdg, 1 );
  myAxisTabs[ 2 ] = new StdMeshersGUI::GridAxisTab( tabWdg, 2 );
  tabWdg->addTab( myAxisTabs[ 0 ], tr( "AXIS_X" ) );
  tabWdg->addTab( myAxisTabs[ 1 ], tr( "AXIS_Y" ) );
  tabWdg->addTab( myAxisTabs[ 2 ], tr( "AXIS_Z" ) );
  argGroupLayout->addWidget( tabWdg, row, 0, 1, 2 );
  row++;

  QPixmap aPix = SMESHGUI::resourceMgr()->loadPixmap("SMESH", tr("ICON_SELECT"));

  // 4) Fixed point
  myFixedPointGrp = new QGroupBox( tr("FIXED_POINT"), fr );
  myFixedPointGrp->setCheckable( true );
  //QPushButton* pointBtn = new QPushButton( QIcon(aPix), "", myFixedPointGrp );
  QLabel* pXLbl = new QLabel( tr("SMESH_X"), myFixedPointGrp );
  QLabel* pYLbl = new QLabel( tr("SMESH_Y"), myFixedPointGrp );
  QLabel* pZLbl = new QLabel( tr("SMESH_Z"), myFixedPointGrp );
  for ( int i = 0; i < 3; ++i )
  {
    myPointSpin[i] = new SMESHGUI_SpinBox( myFixedPointGrp );
    myPointSpin[i]->RangeStepAndValidator( -1e20, 1e20, 10 );
    myPointSpin[i]->SetValue( 0. );
  }
  QHBoxLayout* aFixedPointLay = new QHBoxLayout( myFixedPointGrp );
  aFixedPointLay->addWidget( pXLbl, 0, Qt::AlignRight );
  aFixedPointLay->addWidget( myPointSpin[0], 1 );
  aFixedPointLay->addWidget( pYLbl, 0, Qt::AlignRight );
  aFixedPointLay->addWidget( myPointSpin[1], 1 );
  aFixedPointLay->addWidget( pZLbl, 0, Qt::AlignRight );
  aFixedPointLay->addWidget( myPointSpin[2], 1 );
  argGroupLayout->addWidget( myFixedPointGrp, row, 0, 1, 2 );
  row++;

  // 5) Axes direction
  QGroupBox* axesDirGrp = new QGroupBox( tr("AXES_DIRECTION"), fr );
  QGridLayout* axisDirLay = new QGridLayout( axesDirGrp );
  axisDirLay->setSpacing( SPACING );
  axisDirLay->setMargin( MARGIN );
  axisDirLay->setColumnStretch( 0, 2 );
  // is orthogonal
  myOrthogonalChk = new QCheckBox( tr("ORTHOGONAL_AXES"), axesDirGrp );
  axisDirLay->addWidget( myOrthogonalChk, 0, 0, 1, 7 );
  // axes
  QLabel* axisLbl[3];
  axisLbl[0] = new QLabel( tr( "AXIS_X"), axesDirGrp );
  axisLbl[1] = new QLabel( tr( "AXIS_Y"), axesDirGrp );
  axisLbl[2] = new QLabel( tr( "AXIS_Z"), axesDirGrp );
  QLabel* dLbl[3];
  myAxisBtnGrp = new QButtonGroup( axesDirGrp );
  // get spin width
  const char * const precisionType = "len_tol_precision";
  int minWidth = getMinWidth( precisionType );
  for ( int i = 0; i < 3; ++i )
  {
    QPushButton* axisBtn = new QPushButton( QIcon(aPix), "", axesDirGrp );
    axisBtn->setCheckable( true );
    myAxisBtnGrp->addButton( axisBtn, i );
    myXDirSpin[i] = new SMESHGUI_SpinBox( axesDirGrp );
    myYDirSpin[i] = new SMESHGUI_SpinBox( axesDirGrp );
    myZDirSpin[i] = new SMESHGUI_SpinBox( axesDirGrp );
    myXDirSpin[i]->RangeStepAndValidator( -1, 1, 0.1, precisionType );
    myYDirSpin[i]->RangeStepAndValidator( -1, 1, 0.1, precisionType );
    myZDirSpin[i]->RangeStepAndValidator( -1, 1, 0.1, precisionType );
    myXDirSpin[i]->setMinimumWidth( minWidth );
    myYDirSpin[i]->setMinimumWidth( minWidth );
    myZDirSpin[i]->setMinimumWidth( minWidth );
    dLbl[0] = new QLabel( tr("SMESH_DX"), axesDirGrp );
    dLbl[1] = new QLabel( tr("SMESH_DY"), axesDirGrp );
    dLbl[2] = new QLabel( tr("SMESH_DZ"), axesDirGrp );
    axisDirLay->addWidget( axisLbl[i],    i+1, 0 );
    axisDirLay->addWidget( axisBtn,       i+1, 1 );
    axisDirLay->addWidget( dLbl[0],       i+1, 2 );
    axisDirLay->addWidget( dLbl[1],       i+1, 4 );
    axisDirLay->addWidget( dLbl[2],       i+1, 6 );
    axisDirLay->addWidget( myXDirSpin[i], 1, 3+i*2 );
    axisDirLay->addWidget( myYDirSpin[i], 2, 3+i*2 );
    axisDirLay->addWidget( myZDirSpin[i], 3, 3+i*2 );
  }
  axisDirLay->setColumnStretch( 3, 10 );
  axisDirLay->setColumnStretch( 5, 10 );
  axisDirLay->setColumnStretch( 7, 10 );

  // set optimal axes
  QPushButton* optimBtn = new QPushButton( tr("OPTIMAL_AXES"), axesDirGrp );
  QPushButton* resetBtn = new QPushButton( tr("RESET_AXES"), axesDirGrp );
  axisDirLay->addWidget( optimBtn, 4, 0, 1, 4 );
  axisDirLay->addWidget( resetBtn, 4, 4, 1, 4 );

  argGroupLayout->addWidget( axesDirGrp, row, 0, 1, 2 );
  row++;

  // Signals

  LightApp_SelectionMgr* selMgr = SMESH::GetSelectionMgr( SMESHGUI::GetSMESHGUI() );

  connect( selMgr,          SIGNAL( currentSelectionChanged()), SLOT( onSelectionChange()));
  connect( myOrthogonalChk, SIGNAL( toggled(bool)),             SLOT( onOrthogonalAxes(bool)));
  connect( optimBtn,        SIGNAL( clicked(bool)),             SLOT( onOptimalAxes(bool)));
  connect( resetBtn,        SIGNAL( clicked(bool)),             SLOT( onResetAxes(bool)));
  for ( int i = 0; i < 3; ++i )
  {
    connect( myXDirSpin[i], SIGNAL(valueChanged   (const QString&)),
             this,          SLOT  (onAxisDirChange(const QString&)) );
    connect( myYDirSpin[i], SIGNAL(valueChanged   (const QString&)),
             this,          SLOT  (onAxisDirChange(const QString&)) );
    connect( myZDirSpin[i], SIGNAL(valueChanged   (const QString&)),
             this,          SLOT  (onAxisDirChange(const QString&)) );
    connect( myAxisTabs[i], SIGNAL(gridModeChanged(int)),
             this,          SLOT  (onGridModeChanged(int)));
  }

  // Show axes
  myAxesLen = 120; // default trihedron size is 100
  myOrigin[0] = myOrigin[1] = myOrigin[2] = 0.;
  TopoDS_Shape shape;
  QString shapeEntry = getMainShapeEntry();
  if ( !shapeEntry.isEmpty() )
  {
    // find origin
    Handle(SALOME_InteractiveObject) io =
      new SALOME_InteractiveObject( shapeEntry.toStdString().c_str(), "GEOM" );
    GEOM::GEOM_Object_var geomObj = SMESH::IObjectToInterface<GEOM::GEOM_Object>( io );
    if ( GEOMBase::GetShape( geomObj, shape ) && !shape.IsNull())
    {
      Bnd_Box box;
      BRepBndLib::Add( shape, box );
      double max[3];
      if ( !box.IsVoid() )
      {
        box.Get( myOrigin[0], myOrigin[1], myOrigin[2], max[0], max[1], max[2] );
        gp_Pnt o( myOrigin[0], myOrigin[1], myOrigin[2] );
        gp_Pnt x( max[0], max[1], max[2] );
        myAxesLen = o.Distance( x );

        double step = 1e20;
        while ( step > myAxesLen / 5 )
          step /= 10;
        myPointSpin[0]->SetStep( step );
        myPointSpin[1]->SetStep( step );
        myPointSpin[2]->SetStep( step );
      }
    }
  }
  myAxisBtnGrp->button(0)->setEnabled( !shape.IsNull() );
  myAxisBtnGrp->button(1)->setEnabled( !shape.IsNull() );
  myAxisBtnGrp->button(2)->setEnabled( !shape.IsNull() );
  optimBtn->setEnabled( !shape.IsNull() );

  updateAxesPreview();

  return fr;
}

//================================================================================
/*!
 * \brief Tranfer parameters from hypothesis to widgets
 */
//================================================================================

void StdMeshersGUI_CartesianParamCreator::retrieveParams() const
{
  StdMeshers::StdMeshers_CartesianParameters3D_var h =
    StdMeshers::StdMeshers_CartesianParameters3D::_narrow( initParamsHypothesis() );

  if( myName )
    myName->setText( hypName() );

  QString varName = getVariableName( "SetSizeThreshold" );
  if ( varName.isEmpty() )
    myThreshold->setValue( h->GetSizeThreshold() );
  else
    myThreshold->setText( varName );

  myAddEdges->setChecked( h->GetToAddEdges() );

  // grid definition
  for ( int ax = 0; ax < 3; ++ax )
  {
    if ( h->IsGridBySpacing( ax ))
    {
      SMESH::string_array_var funs;
      SMESH::double_array_var intPoints;
      h->GetGridSpacing( funs.out(), intPoints.out(), ax );
      myAxisTabs[ax]->setSpacing( funs, intPoints );
    }
    else
    {
      SMESH::double_array_var coords = h->GetGrid( ax );
      myAxisTabs[ax]->setCoordinates( coords );
    }
  }

  // fixed point
  SMESH::PointStruct fp;
  StdMeshersGUI_CartesianParamCreator* me = (StdMeshersGUI_CartesianParamCreator*) this;
  if ( h->GetFixedPoint( fp ))
  {
    me->myPointSpin[0]->SetValue( fp.x );
    me->myPointSpin[1]->SetValue( fp.y );
    me->myPointSpin[2]->SetValue( fp.z );
    setText( getVariableName("GetFixedPoint"), &me->myPointSpin[0] );
    myFixedPointGrp->setChecked( true );
  }
  else
  {
    myFixedPointGrp->setChecked( false );
  }

  // axes directions
  SMESHGUI_SpinBox** spins[3] = { &me->myXDirSpin[0], &me->myYDirSpin[0], &me->myZDirSpin[0] };
  SMESH::DirStruct axisDir[3];
  h->GetAxesDirs( axisDir[0],
                  axisDir[1],
                  axisDir[2]);
  QString vars = getVariableName("GetAxesDirs");
  for ( int i = 0; i < 3; ++i )
  {
    spins[i][0]->SetValue( axisDir[i].PS.x );
    spins[i][1]->SetValue( axisDir[i].PS.y );
    spins[i][2]->SetValue( axisDir[i].PS.z );
    setText( vars, spins[i] );

    // cut off 3 used vars
    if ( !vars.isEmpty() )
    {
      int ind = -1;
      for ( int j = 0; j < 3; ++j )
        if (( ind = vars.indexOf(':', ind+1 )) < 0 )
          break;
      if ( ind < 0 )
        vars.clear();
      else
        vars.remove( 0, ind+1 );
    }
  }

  if ( dlg() )
    dlg()->setMinimumSize( dlg()->minimumSizeHint().width(),
                           dlg()->minimumSizeHint().height() );
}

//================================================================================
/*!
 * \brief Tranfer parameters from widgets to hypothesis
 */
//================================================================================

QString StdMeshersGUI_CartesianParamCreator::storeParams() const
{
  StdMeshers::StdMeshers_CartesianParameters3D_var h =
    StdMeshers::StdMeshers_CartesianParameters3D::_narrow( hypothesis() );

  try
  {
    if( isCreation() )
      SMESH::SetName( SMESH::FindSObject( h ), myName->text().toLatin1().constData() );

    // threshold
    h->SetVarParameter( myThreshold->text().toLatin1().constData(), "SetSizeThreshold" );
    h->SetSizeThreshold( myThreshold->text().toDouble() );
    h->SetToAddEdges( myAddEdges->isChecked() );

    // grid
    for ( int ax = 0; ax < 3; ++ax )
    {
      if ( myAxisTabs[ax]->isGridBySpacing())
      {
        SMESH::double_array_var intPoints;
        SMESH::string_array_var funs;
        myAxisTabs[ax]->getSpacing( funs.out(), intPoints.out() );
        h->SetGridSpacing( funs, intPoints, ax );
      }
      else
      {
        SMESH::double_array_var coords = myAxisTabs[ax]->getCoordinates();
        h->SetGrid( coords, ax );
      }
    }

    // fixed point
    QStringList params;
    params << myPointSpin[0]->text();
    params << myPointSpin[1]->text();
    params << myPointSpin[2]->text();
    h->SetVarParameter( params.join(":").toLatin1().constData(), "SetFixedPoint" );
    params.clear();

    SMESH::PointStruct ps;
    ps.x = myPointSpin[0]->GetValue();
    ps.y = myPointSpin[1]->GetValue();
    ps.z = myPointSpin[2]->GetValue();
    h->SetFixedPoint( ps, !myFixedPointGrp->isEnabled() || !myFixedPointGrp->isChecked() );

    // axes directions
    SMESHGUI_SpinBox* const * spins[3] = { &myXDirSpin[0], &myYDirSpin[0], &myZDirSpin[0] };
    for ( int ax = 0; ax < 3; ++ax )
    {
      params << spins[ax][0]->text();
      params << spins[ax][1]->text();
      params << spins[ax][2]->text();
    }
    h->SetVarParameter( params.join(":").toLatin1().constData(), "SetAxesDirs" );

    SMESH::DirStruct axDir[3];
    for ( int ax = 0; ax < 3; ++ax )
    {
      axDir[ax].PS.x = spins[ax][0]->GetValue();
      axDir[ax].PS.y = spins[ax][1]->GetValue();
      axDir[ax].PS.z = spins[ax][2]->GetValue();
    }
    h->SetAxesDirs( axDir[0], axDir[1], axDir[2] );

  }
  catch(const SALOME::SALOME_Exception& ex)
  {
    SalomeApp_Tools::QtCatchCorbaException(ex);
  }
  return "";
}

//================================================================================
/*!
 * \brief Returns a name of help page
 */
//================================================================================

QString StdMeshersGUI_CartesianParamCreator::helpPage() const
{
  return "cartesian_algo_page.html#cartesian_hyp_anchor";
}

//================================================================================
/*!
 * \brief Show axes if they are OK
 */
//================================================================================

bool StdMeshersGUI_CartesianParamCreator::updateAxesPreview()
{
  bool isOk = true;
  gp_Ax1 axes[3];
  SMESHGUI_SpinBox** spins[3] = { &myXDirSpin[0], &myYDirSpin[0], &myZDirSpin[0] };
  for ( int i = 0; i < 3 && isOk; ++i )
  {
    gp_XYZ dir( spins[i][0]->GetValue(),
                spins[i][1]->GetValue(),
                spins[i][2]->GetValue());
    if (( isOk = ( dir.Modulus() > 1e-100 )))
      axes[i].SetDirection( gp_Dir( dir ));

    axes[i].SetLocation ( gp_Pnt( myOrigin[0],
                                  myOrigin[1],
                                  myOrigin[2]));
  }
  gp_Vec norm01 = axes[0].Direction().XYZ() ^ axes[1].Direction().XYZ();
  gp_Vec norm12 = axes[1].Direction().XYZ() ^ axes[2].Direction().XYZ();
  if ( isOk )
    isOk = ( !axes[0].Direction().IsParallel( axes[1].Direction(), theAngTol ) &&
             !axes[1].Direction().IsParallel( axes[2].Direction(), theAngTol ) &&
             !axes[2].Direction().IsParallel( axes[0].Direction(), theAngTol ) &&
             !norm01.IsParallel( norm12, theAngTol ) );
  if ( isOk )
    myAxesPreview->SetArrows( axes, myAxesLen );

  myAxesPreview->SetVisibility( isOk );

  return isOk;
}

//================================================================================
/*!
 * \brief Makes axes orthogonal if necessary
 */
//================================================================================

void StdMeshersGUI_CartesianParamCreator::onOrthogonalAxes(bool isOrtho)
{
  if ( !isOrtho )
  {
    updateAxesPreview();
    return;
  }

  std::multimap< int, int > ageOfAxis;
  gp_XYZ dirs[3];
  SMESHGUI_SpinBox** spins[3] = { &myXDirSpin[0], &myYDirSpin[0], &myZDirSpin[0] };
  int nbOk = 0, isOk;
  for ( int iAx = 0; iAx < 3; ++iAx )
  {
    dirs[iAx].SetCoord( spins[iAx][0]->GetValue(),
                        spins[iAx][1]->GetValue(),
                        spins[iAx][2]->GetValue());
    if (( isOk = ( dirs[iAx].Modulus() > 1e-100 )))
      ageOfAxis.insert( std::make_pair( myDirTic[iAx], iAx ));
    else
      ageOfAxis.insert( std::make_pair( -1, iAx ));
    nbOk += isOk;
  }
  switch ( nbOk )
  {
  case 0:
  {
    dirs[0].SetCoord( 1, 0, 0 );
    dirs[1].SetCoord( 0, 1, 0 );
    dirs[2].SetCoord( 0, 0, 1 );
    break;
  }
  case 1:
  {
    int iOk = ageOfAxis.rbegin()->second;
    get3Dirs( iOk, dirs );
    break;
  }
  default:
    std::multimap< int, int >::reverse_iterator ag2ax = ageOfAxis.rbegin();
    int iOk1 = ag2ax->second;
    int iOk2 = (++ag2ax)->second;
    int iKo  = (++ag2ax)->second;
    if ( gp_Vec( dirs[ iOk1 ]).IsParallel( gp_Vec( dirs[ iOk2 ]), theAngTol ))
      std::swap( iOk2, iKo );
    if ( gp_Vec( dirs[ iOk1 ]).IsParallel( gp_Vec( dirs[ iOk2 ]), theAngTol ))
    {
      get3Dirs( iOk1, dirs );
    }
    else
    {
      dirs[ iKo  ] = dirs[ iOk1 ] ^ dirs[ iOk2 ];
      dirs[ iOk2 ] = dirs[ iKo  ] ^ dirs[ iOk1 ];
      if ( ( iOk1+1 ) % 3 != iOk2 )
        dirs[ iKo ].Reverse();
    }
  }

  for ( int iAx = 0; iAx < 3; ++iAx )
  {
    double size = dirs[iAx].Modulus();
    if ( size > 1e-100 )
      dirs[iAx] /= size;
    for (int i = 0; i < 3; ++i )
    {
      bool isBlocked = spins[iAx][i]->blockSignals( true );
      spins[iAx][i]->SetValue( dirs[iAx].Coord( i+1 ));
      spins[iAx][i]->blockSignals( isBlocked );
    }
  }

  updateAxesPreview();
}

//================================================================================
/*!
 * \brief Increment myDirTic and update the preview of axes
 */
//================================================================================

void StdMeshersGUI_CartesianParamCreator::onAxisDirChange(const QString&)
{
  QObject* changedSpin = sender();
  SMESHGUI_SpinBox** spins[3] = { &myXDirSpin[0], &myYDirSpin[0], &myZDirSpin[0] };
  for ( int iAx = 0; iAx < 3; ++iAx )
    if ( spins[iAx][0] == changedSpin ||
         spins[iAx][1] == changedSpin ||
         spins[iAx][2] == changedSpin )
    {
      myDirTic[ iAx ] = 1 + Max( Max( myDirTic[0], myDirTic[1] ), myDirTic[2] );
      break;
    }

  onOrthogonalAxes( myOrthogonalChk->isChecked() );
}

//================================================================================
/*!
 * \brief Sets axis direction by a selected EDGE
 */
//================================================================================

void StdMeshersGUI_CartesianParamCreator::onSelectionChange()
{
  int iAxis = myAxisBtnGrp->checkedId();
  if ( iAxis < 0 )
    return;

  SALOME_ListIO aList;
  SMESHGUI::GetSMESHGUI()->selectionMgr()->selectedObjects(aList);

  TopoDS_Shape edge, shape;
  for( SALOME_ListIteratorOfListIO anIt( aList ); anIt.More(); anIt.Next() )
  {
    GEOM::GEOM_Object_var go = SMESH::IObjectToInterface<GEOM::GEOM_Object>( anIt.Value() );
    if ( GEOMBase::GetShape( go, shape ) && shape.ShapeType() == TopAbs_EDGE )
    {
      if ( !edge.IsNull() )
        return; // several EDGEs selected
      edge = shape;
    }
  }
  if ( edge.IsNull() )
    return;

  TopoDS_Shape vv[2];
  TopoDS_Iterator vIt( edge );
  for ( ; vIt.More() && vv[1].IsNull(); vIt.Next() )
    vv[ !vv[0].IsNull() ] = vIt.Value();

  gp_Pnt pp[2];
  if ( !GEOMBase::VertexToPoint( vv[0], pp[0] ) ||
       !GEOMBase::VertexToPoint( vv[1], pp[1] ))
    return;

  SMESHGUI_SpinBox** spins[3] = { &myXDirSpin[0], &myYDirSpin[0], &myZDirSpin[0] };

  gp_Vec newDir( pp[0], pp[1] );
  gp_Vec curDir( spins[iAxis][0]->GetValue(),
                 spins[iAxis][1]->GetValue(),
                 spins[iAxis][2]->GetValue());
  if ( newDir * curDir < 0 )
    newDir.Reverse();

  double size = newDir.Magnitude();
  if ( size < 1e-100 )
    return;
  newDir /= size;

  for (int i = 0; i < 3; ++i )
  {
    bool isBlocked = spins[iAxis][i]->blockSignals( true );
    spins[iAxis][i]->SetValue( newDir.Coord( i+1 ));
    spins[iAxis][i]->blockSignals( isBlocked );
  }
  myDirTic[ iAxis ] = 1 + Max( Max( myDirTic[0], myDirTic[1] ), myDirTic[2] );

  onOrthogonalAxes( myOrthogonalChk->isChecked() );
}

//================================================================================
/*!
 * \brief Sets axes at which number of hexahedra is maximal
 */
//================================================================================

void StdMeshersGUI_CartesianParamCreator::onOptimalAxes(bool)
{
  StdMeshers::StdMeshers_CartesianParameters3D_var h =
    StdMeshers::StdMeshers_CartesianParameters3D::_narrow( hypothesis() );
  if ( h->_is_nil() )
    return;

  QString shapeEntry = getMainShapeEntry();
  if ( shapeEntry.isEmpty() )
    return;

  Handle(SALOME_InteractiveObject) io =
    new SALOME_InteractiveObject( shapeEntry.toStdString().c_str(), "GEOM" );
  GEOM::GEOM_Object_var geomObj = SMESH::IObjectToInterface<GEOM::GEOM_Object>( io );
  if ( geomObj->_is_nil() )
    return;

  SMESH::DirStruct axDirs[3];
  h->ComputeOptimalAxesDirs( geomObj,
                             myOrthogonalChk->isChecked(),
                             axDirs[0],
                             axDirs[1],
                             axDirs[2]);

  SMESHGUI_SpinBox** spins[3] = { &myXDirSpin[0], &myYDirSpin[0], &myZDirSpin[0] };
  for ( int iAx = 0; iAx < 3; ++iAx )
  {
    double coords[3] = { axDirs[iAx].PS.x, axDirs[iAx].PS.y, axDirs[iAx].PS.z };
    for (int i = 0; i < 3; ++i )
    {
      bool isBlocked = spins[iAx][i]->blockSignals( true );
      spins[iAx][i]->SetValue( coords[ i ]);
      spins[iAx][i]->blockSignals( isBlocked );
    }
  }
  updateAxesPreview();
}

//================================================================================
/*!
 * \brief Sets axes || to the axes of global CS
 */
//================================================================================

void StdMeshersGUI_CartesianParamCreator::onResetAxes(bool)
{
  SMESHGUI_SpinBox** spins[3] = { &myXDirSpin[0], &myYDirSpin[0], &myZDirSpin[0] };
  for ( int iAx = 0; iAx < 3; ++iAx )
  {
    for (int i = 0; i < 3; ++i )
    {
      bool isBlocked = spins[iAx][i]->blockSignals( true );
      spins[iAx][i]->SetValue( iAx == i ? 1. : 0. );
      spins[iAx][i]->blockSignals( isBlocked );
    }
    myDirTic[iAx] = 0;
  }
  updateAxesPreview();
}

//================================================================================
/*!
 * \brief SLOT called when the grid definintion mode changes
 */
//================================================================================

void StdMeshersGUI_CartesianParamCreator::onGridModeChanged(int)
{
  bool haveSpacing = ( myAxisTabs[0]->isGridBySpacing() ||
                       myAxisTabs[1]->isGridBySpacing() ||
                       myAxisTabs[2]->isGridBySpacing() );

  myFixedPointGrp->setEnabled( haveSpacing );
}
