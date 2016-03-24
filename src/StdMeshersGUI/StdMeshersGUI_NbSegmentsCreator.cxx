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

// File   : StdMeshersGUI_NbSegmentsCreator.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "StdMeshersGUI_NbSegmentsCreator.h"
#ifndef DISABLE_PLOT2DVIEWER
  #include "StdMeshersGUI_DistrPreview.h"
#endif
#include "StdMeshersGUI_DistrTable.h"
#include "StdMeshersGUI_PropagationHelperWdg.h"
#include "StdMeshersGUI_SubShapeSelectorWdg.h"

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
#include <QLabel>
#include <QGroupBox>
#include <QFrame>
#include <QLineEdit>
#include <QButtonGroup>
#include <QRadioButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>

#define SPACING 6
#define MARGIN  11

StdMeshersGUI_NbSegmentsCreator::StdMeshersGUI_NbSegmentsCreator()
: StdMeshersGUI_StdHypothesisCreator( "NumberOfSegments" ),
  myNbSeg( 0 ),
  myDistr( 0 ),
  myScale( 0 ),
  myTable( 0 ),
#ifndef DISABLE_PLOT2DVIEWER
  myPreview( 0 ),
#endif
  myExpr( 0 ),
  myConvBox( 0 ),
  myConv( 0 ),
  myLScale( 0 ),
  myLTable( 0 ),
  myLExpr( 0 ),
  myInfo( 0 ),
  myGroupLayout( 0 ),
  myTableRow( 0 ),
  myPreviewRow( 0 )
{
}

StdMeshersGUI_NbSegmentsCreator::~StdMeshersGUI_NbSegmentsCreator()
{
}

bool StdMeshersGUI_NbSegmentsCreator::checkParams( QString& msg ) const
{
  if( !SMESHGUI_GenericHypothesisCreator::checkParams( msg ) )
    return false;
  NbSegmentsHypothesisData data_old, data_new;
  readParamsFromHypo( data_old );
  readParamsFromWidgets( data_new );
  bool res = storeParamsToHypo( data_new );
  res = myNbSeg->isValid( msg, true ) && res;
  res = myScale->isValid( msg, true ) && res;
  if ( !res )
    storeParamsToHypo( data_old );
  return res;
}

QFrame* StdMeshersGUI_NbSegmentsCreator::buildFrame()
{
  QFrame* fr = new QFrame();
  fr->setMinimumWidth(460);

  QVBoxLayout* lay = new QVBoxLayout( fr );
  lay->setMargin( 0 );
  lay->setSpacing( SPACING );

  QGroupBox* GroupC1 = new QGroupBox( tr( "SMESH_ARGUMENTS" ), fr );
  lay->addWidget( GroupC1 );

  StdMeshers::StdMeshers_NumberOfSegments_var h =
    StdMeshers::StdMeshers_NumberOfSegments::_narrow( hypothesis() );

  myGroupLayout = new QGridLayout( GroupC1 );
  myGroupLayout->setSpacing( SPACING );
  myGroupLayout->setMargin( MARGIN );
  myGroupLayout->setColumnStretch( 0, 0 );
  myGroupLayout->setColumnStretch( 1, 1 );

  int row = 0;
  // 0)  name
  myName = 0;
  if( isCreation() )
  {
    myName = new QLineEdit( GroupC1 );
    myGroupLayout->addWidget( new QLabel( tr( "SMESH_NAME" ), GroupC1 ), row, 0 );
    myGroupLayout->addWidget( myName, row, 1 );
    row++;
  }


  // 1)  number of segments
  myGroupLayout->addWidget( new QLabel( tr( "SMESH_NB_SEGMENTS_PARAM" ), GroupC1 ), row, 0 );
  myNbSeg = new SalomeApp_IntSpinBox( GroupC1 );
  myNbSeg->setMinimum( 1 );
  myNbSeg->setMaximum( 9999 );
  myGroupLayout->addWidget( myNbSeg, row, 1 );
  row++;

  
  // 2)  type of distribution
  myGroupLayout->addWidget( new QLabel( tr( "SMESH_DISTR_TYPE" ), GroupC1 ), row, 0 );
  myDistr = new QtxComboBox( GroupC1 );
  QStringList types;
  types.append( tr( "SMESH_DISTR_REGULAR" ) );
  types.append( tr( "SMESH_DISTR_SCALE"   ) );
  types.append( tr( "SMESH_DISTR_TAB"     ) );
  types.append( tr( "SMESH_DISTR_EXPR"    ) );
  myDistr->addItems( types );
  myGroupLayout->addWidget( myDistr, row, 1 );
  row++;

  
  // 3)  scale
  myGroupLayout->addWidget( myLScale = new QLabel( tr( "SMESH_NB_SEGMENTS_SCALE_PARAM" ), GroupC1 ), row, 0 );
  myScale = new SMESHGUI_SpinBox( GroupC1 );
  myScale->RangeStepAndValidator( 1E-5, 1E+5, 0.1, "parametric_precision" );
  myGroupLayout->addWidget( myScale, row, 1 );
  row++;

  
  // 4) Distribution definition
  QGridLayout* myDistLayout = new QGridLayout(GroupC1);
  myGroupLayout->addLayout( myDistLayout, row, 0, 1, 2 );
  myGroupLayout->setRowStretch( row, 1 );
  row ++;

       // a)  expression
  QHBoxLayout* myExprLayout = new QHBoxLayout(GroupC1);
  myExprLayout->addWidget( myLExpr = new QLabel( "f(t)=", GroupC1 ), 0);
  myExpr = new QLineEdit( GroupC1 );
  myExprLayout->addWidget( myExpr,1);
  myDistLayout->addLayout(myExprLayout,1 ,0);
  myDistLayout->setRowStretch(2, 1);

       // b)  warning
  myInfo = new QLabel( tr( "SMESH_FUNC_DOMAIN" ), GroupC1 );
  myDistLayout->addWidget( myInfo, 0, 0, 1, 2);
  
       // c)  table
  myTable = new StdMeshersGUI_DistrTableFrame( GroupC1 );
  myTable->setMinimumHeight(220);
  myDistLayout->addWidget( myTable, 1, 0, 2, 1 );

#ifndef DISABLE_PLOT2DVIEWER
       // d) preview
  myPreview = new StdMeshersGUI_DistrPreview( GroupC1, h.in() );  
  myDistLayout->addWidget( myPreview, 1, 1, 2, 1 );
#endif
  
  // 5)  conversion (radiogroup)
  myConvBox = new QGroupBox( tr( "SMESH_CONV_MODE" ), GroupC1 );
  myConv = new QButtonGroup( GroupC1 );

  QHBoxLayout* convLay = new QHBoxLayout( myConvBox );
  convLay->setMargin( MARGIN );
  convLay->setSpacing( SPACING );

  QRadioButton* rbExp = new QRadioButton( tr( "SMESH_EXP_MODE" ), myConvBox );
  QRadioButton* myCutNeg = new QRadioButton( tr( "SMESH_CUT_NEG_MODE" ), myConvBox );

  convLay->addWidget( rbExp );
  convLay->addWidget( myCutNeg );
  myConv->addButton( rbExp, 0 );
  myConv->addButton( myCutNeg, 1 );

  myGroupLayout->addWidget( myConvBox, row, 0, 1, 2 );
  row++;


  // 6) reverse edge parameters
  myReversedEdgesBox = new QGroupBox(tr( "SMESH_REVERSED_EDGES" ), fr);
  QHBoxLayout* edgeLay = new QHBoxLayout( myReversedEdgesBox );

  myDirectionWidget = new StdMeshersGUI_SubShapeSelectorWdg( myReversedEdgesBox );
  QString aGeomEntry = getShapeEntry();
  QString aMainEntry = getMainShapeEntry();
  if ( aGeomEntry == "" )
    aGeomEntry = h->GetObjectEntry();
  myDirectionWidget->SetGeomShapeEntry( aGeomEntry, aMainEntry );
  myDirectionWidget->SetListOfIDs( h->GetReversedEdges() );
  edgeLay->addWidget( myDirectionWidget );

  lay->addWidget( myReversedEdgesBox );
  lay->setStretchFactor( GroupC1, 1);
  lay->setStretchFactor( myReversedEdgesBox, 1);

  myReversedEdgesHelper = 0;
  if ( !aGeomEntry.isEmpty() || !aMainEntry.isEmpty() )
  {
    myReversedEdgesHelper = new StdMeshersGUI_PropagationHelperWdg( myDirectionWidget, fr, false );
    lay->addWidget( myReversedEdgesHelper );
    lay->setStretchFactor( myReversedEdgesHelper, 1 );
  }

  connect( myNbSeg, SIGNAL( valueChanged( const QString& ) ), this, SLOT( onValueChanged() ) );
  connect( myDistr, SIGNAL( activated( int ) ), this, SLOT( onValueChanged() ) );
  connect( myTable, SIGNAL( valueChanged( int, int ) ), this, SLOT( onValueChanged() ) );
  connect( myExpr,  SIGNAL( textChanged( const QString& ) ), this, SLOT( onValueChanged() ) );
  connect( myConv,  SIGNAL( buttonClicked( int ) ), this, SLOT( onValueChanged() ) );

  onValueChanged();

  return fr;
}

void StdMeshersGUI_NbSegmentsCreator::retrieveParams() const
{
  NbSegmentsHypothesisData data;
  readParamsFromHypo( data );

  if( myName )
    myName->setText( data.myName );
  if(data.myNbSegVarName.isEmpty())
    myNbSeg->setValue( data.myNbSeg );
  else
    myNbSeg->setText( data.myNbSegVarName );
  
  myDistr->setCurrentIndex( data.myDistrType );
  if(data.myScaleVarName.isEmpty())
    myScale->setValue( data.myScale );
  else
    myScale->setText( data.myScaleVarName );
  myConv->button( data.myConv )->setChecked( true );
  myTable->setFuncMinValue(myConv->checkedId()==0 ? -1E20 : 0);
  myTable->setData( data.myTable );
  myExpr->setText( data.myExpr );

  if ( dlg() )
    dlg()->setMinimumSize( dlg()->minimumSizeHint().width(), dlg()->minimumSizeHint().height() );
}

QString StdMeshersGUI_NbSegmentsCreator::storeParams() const
{
  NbSegmentsHypothesisData data;
  readParamsFromWidgets( data );
  storeParamsToHypo( data );
    
  QString valStr = QString::number( data.myNbSeg ) += "; ";

  enum DistrType
  {
    Regular, //!< equidistant distribution
    Scale,   //!< scale distribution
    TabFunc, //!< distribution with density function presented by table
    ExprFunc //!< distribution with density function presented by expression
  };
  bool hasConv = false;
  switch ( data.myDistrType ) {
  case Regular :
    valStr += tr("SMESH_DISTR_REGULAR");
    break;
  case Scale   :
    valStr += tr("SMESH_NB_SEGMENTS_SCALE_PARAM") + " = " + QString::number( data.myScale );
    break;
  case TabFunc : {
    //valStr += tr("SMESH_TAB_FUNC");
    bool param = true;
    for( CORBA::ULong i = 0; i < data.myTable.length(); i++, param = !param ) {
      if ( param )
        valStr += "[";
      valStr += QString::number( data.myTable[ i ]);
      valStr += ( param ? "," : "]" );
    }
    hasConv = true;
    break;
  }
  case ExprFunc:
    valStr += data.myExpr;
    hasConv = true;
    break;
  }
  if ( hasConv )
  {
    if ( data.myConv )
      valStr += "; " + tr("SMESH_CUT_NEG_MODE");
    else
      valStr += "; " + tr("SMESH_EXP_MODE");
  }

  return valStr;
}

bool StdMeshersGUI_NbSegmentsCreator::readParamsFromHypo( NbSegmentsHypothesisData& h_data ) const
{
  StdMeshers::StdMeshers_NumberOfSegments_var h =
    StdMeshers::StdMeshers_NumberOfSegments::_narrow( initParamsHypothesis() );

  h_data.myName = hypName();

  h_data.myNbSeg = (int) h->GetNumberOfSegments();

  CORBA::String_var aVaribaleName = h->GetVarParameter( "SetNumberOfSegments" );
  h_data.myNbSegVarName = aVaribaleName.in();

  int distr = (int) h->GetDistrType();
  h_data.myDistrType = distr;
  h_data.myScale = distr==1 ? h->GetScaleFactor() : 1.0;
  
  if(distr==1) {
    aVaribaleName = h->GetVarParameter( "SetScaleFactor" );
    h_data.myScaleVarName = aVaribaleName.in();
  }
  else 
    h_data.myScaleVarName = QString("");

  if( distr==2 )
  {
    SMESH::double_array* a = h->GetTableFunction();
    h_data.myTable = *a;
    delete a;
  }
  else
  {
    SMESH::double_array& a = h_data.myTable;
    // by default, constant table function f(t)=1
    a.length( 4 );
    a[0] = 0.0; a[1] = 1.0;
    a[2] = 1.0; a[3] = 1.0; 
  }

  h_data.myExpr = distr==3 ? h->GetExpressionFunction() : "1";
  h_data.myConv = distr==2 || distr==3 ? h->ConversionMode() : 1; /*cut negative by default*/

  return true;
}

bool StdMeshersGUI_NbSegmentsCreator::storeParamsToHypo( const NbSegmentsHypothesisData& h_data ) const
{
  StdMeshers::StdMeshers_NumberOfSegments_var h =
    StdMeshers::StdMeshers_NumberOfSegments::_narrow( hypothesis() );

  bool ok = true;
  try
  {
    if( isCreation() )
      SMESH::SetName( SMESH::FindSObject( h ), h_data.myName.toLatin1().data() );

    h->SetVarParameter( h_data.myNbSegVarName.toLatin1().constData(), "SetNumberOfSegments" );
    h->SetNumberOfSegments( h_data.myNbSeg );
    
    int distr = h_data.myDistrType;
    if ( distr == 0 )
      h->SetDistrType( distr ); // this is actually needed at non-uniform -> uniform switch
    if( distr==1 ) {
      h->SetVarParameter( h_data.myScaleVarName.toLatin1().constData(), "SetScaleFactor" );
      h->SetScaleFactor( h_data.myScale );
    }
    if( distr==2 || distr==3 )
      h->SetConversionMode( h_data.myConv );

    if( distr==1 || distr==2 || distr==3 ) {
      h->SetReversedEdges( myDirectionWidget->GetListOfIDs() );
      h->SetObjectEntry( myDirectionWidget->GetMainShapeEntry() );
    }

    if( distr==2 )
      h->SetTableFunction( h_data.myTable );

    if( distr==3 )
      h->SetExpressionFunction( h_data.myExpr.toLatin1().data() );
    //setting of function must follow after setConversionMode, because otherwise
    //the function will be checked with old conversion mode, so that it may occurs
    //unexpected errors for user
  }
  catch(const SALOME::SALOME_Exception& ex)
  {
    SalomeApp_Tools::QtCatchCorbaException(ex);
    ok = false;
  }
  return ok;
}

bool StdMeshersGUI_NbSegmentsCreator::readParamsFromWidgets( NbSegmentsHypothesisData& h_data ) const
{
  h_data.myName      = myName ? myName->text() : "";
  h_data.myNbSeg     = myNbSeg->value();
  h_data.myNbSegVarName =  myNbSeg->text();
  h_data.myScaleVarName =  myScale->text();
  h_data.myDistrType = myDistr->currentIndex();
  h_data.myConv      = myConv->checkedId();
  h_data.myScale     = myScale->value();
  myTable->data( h_data.myTable );
  h_data.myExpr      = myExpr->text();
  return true;
}

void StdMeshersGUI_NbSegmentsCreator::onValueChanged()
{
  int distr = myDistr->currentIndex();

/*  if( distr==2 ) //table func
    myCutNeg->setText( tr( "SMESH_NO_CONV" ) );
  else if( distr==3 )
    myCutNeg->setText( tr( "SMESH_CUT_NEG_MODE" ) );*/

  if( distr==2 && sender()==myConv ) //table func
  {
    myTable->setFuncMinValue( myConv->checkedId()==0 ? -1E20 : 0 );
    SMESH::double_array arr;
    myTable->data( arr );
    myTable->setData( arr ); //update data in table
  }

  myScale->setVisible( distr==1 );
  myLScale->setVisible( distr==1 );
  myReversedEdgesBox->setVisible( distr!=0 );
  if ( myReversedEdgesHelper ) {
    myReversedEdgesHelper->Clear();
    myReversedEdgesHelper->setVisible( distr!=0 );
  }
  myDirectionWidget->ShowPreview( distr!=0 );

  bool isFunc = distr==2 || distr==3;
#ifndef DISABLE_PLOT2DVIEWER
  myPreview->setVisible( isFunc );
#endif
  myConvBox->setVisible( isFunc );
  
  myTable->setVisible( distr==2 );
  myExpr->setVisible( distr==3 );
  myLExpr->setVisible( distr==3 );
  myInfo->setVisible( distr==3);

#ifndef DISABLE_PLOT2DVIEWER
  //change of preview
  int nbSeg = myNbSeg->value();
  if( distr==2 ) //preview for table-described function
  {
    SMESH::double_array a;
    myTable->data( a );
    myPreview->setParams( a, nbSeg, false );
  }
  else if( distr==3 ) //preview for analytic-described function
    myPreview->setParams( myExpr->text(), nbSeg, 100, false );

  if( isFunc )
    myPreview->setConversion( StdMeshersGUI_DistrPreview::Conversion( myConv->checkedId() ) );
#endif

  if ( (QtxComboBox*)sender() == myDistr && dlg() ) {
    QApplication::instance()->processEvents();
    myGroupLayout->invalidate();
    dlg()->layout()->invalidate();
    dlg()->updateGeometry();
    dlg()->setMinimumSize( dlg()->minimumSizeHint() );
    dlg()->resize( dlg()->minimumSize() );
    QApplication::instance()->processEvents();
  }
}
