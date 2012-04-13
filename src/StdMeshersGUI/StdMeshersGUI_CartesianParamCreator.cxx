// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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
//
#include "StdMeshersGUI_CartesianParamCreator.h"

#include <SMESHGUI.h>
#include <SMESHGUI_Utils.h>
#include <SMESHGUI_HypothesesUtils.h>
#include <SMESHGUI_SpinBox.h>

// IDL includes
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

// SALOME GUI includes
#include <SalomeApp_Tools.h>
#include <SalomeApp_IntSpinBox.h>
#include <QtxComboBox.h>

// Qt includes
#include <QAbstractItemModel>
#include <QApplication>
#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QModelIndex>
#include <QRadioButton>
#include <QString>
#include <QStyleOptionViewItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTabWidget>

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
    QFrame* csFrame = new QFrame( this );
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
    connect( mySpacingTreeWdg, SIGNAL( itemSelectionChanged()), SLOT( updateButtons() ));
    connect( myCoordList,      SIGNAL( itemSelectionChanged()), SLOT( updateButtons() ));
    connect( myStepSpin,       SIGNAL( valueChanged(double)),   SLOT( onStepChange() ));
  }

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

  void GridAxisTab::onMode(int isSpacing)
  {
    mySpacingTreeWdg->setShown( isSpacing );
    myCoordList->setShown( !isSpacing );
    myStepSpin->setShown( !isSpacing );
    myStepLabel->setShown( !isSpacing );
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
      myCoordList->clear();
    }
    else
    {
      mySpacingTreeWdg->clear();
      if ( myCoordList->count() == 0 )
        myCoordList->addItem( coordToItem( 0 ));
    }
    updateButtons();
  }

  void GridAxisTab::onStepChange()
  {
    if ( fabs( myStepSpin->GetValue() ) < 1e-100 )
    {
      double delta = myStepSpin->singleStep() * ( myStep > myStepSpin->GetValue() ? -1 : +1 );
      myStepSpin->SetValue( myStepSpin->GetValue() + delta );
    }
    myStep = myStepSpin->GetValue();
  }

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

  void GridAxisTab::setCoordinates( SMESH::double_array_var coords )
  {
    myCoordList->clear();
    for ( size_t i = 0; i < coords->length(); ++i )
      myCoordList->addItem( coordToItem( coords[i] ));

    myModeGroup->button( COORD_BUT )->setChecked( true );
    onMode( COORD_BUT );
  }

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

  bool GridAxisTab::isGridBySpacing() const
  {
    return ( myModeGroup->checkedId() == SPACING_BUT );
  }

  SMESH::double_array* GridAxisTab::getCoordinates()
  {
    SMESH::double_array_var coords = new SMESH::double_array;
    coords->length( myCoordList->count() );
    for ( size_t i = 0; i < coords->length(); ++i )
      coords[i] = coordFromItem( myCoordList->item( i ) );

    return coords._retn();
  }

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

  LineDelegate::LineDelegate( QWidget* parent ):
    QItemDelegate( parent ),
    mySpacingTreeWdg( qobject_cast<QTreeWidget*>( parent )),
    myCoordList( qobject_cast<QListWidget*>( parent ))
  {
  }

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


StdMeshersGUI_CartesianParamCreator::StdMeshersGUI_CartesianParamCreator(const QString& aHypType)
  : StdMeshersGUI_StdHypothesisCreator( aHypType ),
    myThreshold( 0 )
{
  myAxisTabs[0] = 0;
  myAxisTabs[1] = 0;
  myAxisTabs[2] = 0;
}

StdMeshersGUI_CartesianParamCreator::~StdMeshersGUI_CartesianParamCreator()
{
  if ( myAxisTabs[0] ) delete myAxisTabs[0];
  if ( myAxisTabs[1] ) delete myAxisTabs[1];
  if ( myAxisTabs[2] ) delete myAxisTabs[2];
  myAxisTabs[0] = 0;
  myAxisTabs[1] = 0;
  myAxisTabs[2] = 0;
}

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

  return true;
}

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
  myThreshold->RangeStepAndValidator( 1.1, 1e+10, 1., "length_precision" );
  argGroupLayout->addWidget( myThreshold, row, 1 );
  row++;
  
  // 2)  Grid definition
  QTabWidget* tabWdg = new QTabWidget( fr );
  myAxisTabs[ 0 ] = new StdMeshersGUI::GridAxisTab( tabWdg, 0 );
  myAxisTabs[ 1 ] = new StdMeshersGUI::GridAxisTab( tabWdg, 1 );
  myAxisTabs[ 2 ] = new StdMeshersGUI::GridAxisTab( tabWdg, 2 );
  tabWdg->addTab( myAxisTabs[ 0 ], tr( "AXIS_X" ) );
  tabWdg->addTab( myAxisTabs[ 1 ], tr( "AXIS_Y" ) );
  tabWdg->addTab( myAxisTabs[ 2 ], tr( "AXIS_Z" ) );
  argGroupLayout->addWidget( tabWdg, row, 0, 1, 2 );

  return fr;
}

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
  if ( dlg() )
    dlg()->setMinimumSize( dlg()->minimumSizeHint().width(), dlg()->minimumSizeHint().height() );
}

QString StdMeshersGUI_CartesianParamCreator::storeParams() const
{
  StdMeshers::StdMeshers_CartesianParameters3D_var h =
    StdMeshers::StdMeshers_CartesianParameters3D::_narrow( hypothesis() );

  try
  {
    if( isCreation() )
      SMESH::SetName( SMESH::FindSObject( h ), myName->text().toLatin1().constData() );

    h->SetVarParameter( myThreshold->text().toLatin1().constData(), "SetSizeThreshold" );
    h->SetSizeThreshold( myThreshold->text().toDouble() );

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
  }
  catch(const SALOME::SALOME_Exception& ex)
  {
    SalomeApp_Tools::QtCatchCorbaException(ex);
  }
  return "";
}

QString StdMeshersGUI_CartesianParamCreator::helpPage() const
{
  return "cartesian_algo_page.html#cartesian_hyp_anchor";
}
