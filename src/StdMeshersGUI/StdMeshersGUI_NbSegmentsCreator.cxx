
#include "StdMeshersGUI_NbSegmentsCreator.h"
#include "StdMeshersGUI_DistrTable.h"
#include "StdMeshersGUI_DistrPreview.h"

#include <SMESHGUI_Utils.h>
#include <SMESHGUI_HypothesesUtils.h>

#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include <SalomeApp_Tools.h>

#include <QtxIntSpinBox.h>
#include <QtxComboBox.h>
#include <QtxDblValidator.h>
#include <SMESHGUI_SpinBox.h>

#include <qlabel.h>
#include <qgroupbox.h>
#include <qframe.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

StdMeshersGUI_NbSegmentsCreator::StdMeshersGUI_NbSegmentsCreator()
: StdMeshersGUI_StdHypothesisCreator( "NumberOfSegments" ),
  myNbSeg( 0 ),
  myDistr( 0 ),
  myScale( 0 ),
  myTable( 0 ),
  myPreview( 0 ),
  myExpr( 0 ),
  myConv( 0 ),
  myLScale( 0 ),
  myLTable( 0 ),
  myLExpr( 0 ),
  myLConv( 0 ),
  myInfo( 0 ),
  myGroupLayout( 0 ),
  myTableRow( 0 ),
  myPreviewRow( 0 )
{
}

StdMeshersGUI_NbSegmentsCreator::~StdMeshersGUI_NbSegmentsCreator()
{
}

bool StdMeshersGUI_NbSegmentsCreator::checkParams() const
{
  NbSegmentsHypothesisData data_old, data_new;
  readParamsFromHypo( data_old );
  readParamsFromWidgets( data_new );
  bool res = storeParamsToHypo( data_new );
  storeParamsToHypo( data_old );
  return res;
}

QFrame* StdMeshersGUI_NbSegmentsCreator::buildFrame()
{
  QFrame* fr = new QFrame( 0, "myframe" );
  QVBoxLayout* lay = new QVBoxLayout( fr, 5, 0 );

  QGroupBox* GroupC1 = new QGroupBox( fr, "GroupC1" );
  lay->addWidget( GroupC1 );

  StdMeshers::StdMeshers_NumberOfSegments_var h =
    StdMeshers::StdMeshers_NumberOfSegments::_narrow( hypothesis() );
  myPreview = new StdMeshersGUI_DistrPreview( GroupC1, h.in() );

  GroupC1->setTitle( tr( "SMESH_ARGUMENTS"  ) );
  GroupC1->setColumnLayout(0, Qt::Vertical );
  GroupC1->layout()->setSpacing( 0 );
  GroupC1->layout()->setMargin( 0 );
  myGroupLayout = new QGridLayout( GroupC1->layout() );
  myGroupLayout->setAlignment( Qt::AlignTop );
  myGroupLayout->setSpacing( 6 );
  myGroupLayout->setMargin( 11 );
  myGroupLayout->setColStretch( 0, 0 );
  myGroupLayout->setColStretch( 1, 1 );

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
  myNbSeg = new QtxIntSpinBox( GroupC1 );
  myNbSeg->setMinValue( 1 );
  myNbSeg->setMaxValue( 9999 );
  myGroupLayout->addWidget( myNbSeg, row, 1 );
  row++;

  // 2)  type of distribution
  myGroupLayout->addWidget( new QLabel( tr( "SMESH_DISTR_TYPE" ), GroupC1 ), row, 0 );
  myDistr = new QtxComboBox( GroupC1 );
  QStringList types;
  types.append( QObject::tr( "SMESH_DISTR_REGULAR" ) );
  types.append( QObject::tr( "SMESH_DISTR_SCALE"   ) );
  types.append( QObject::tr( "SMESH_DISTR_TAB"     ) );
  types.append( QObject::tr( "SMESH_DISTR_EXPR"    ) );
  myDistr->insertStringList( types );
  myGroupLayout->addWidget( myDistr, row, 1 );
  row++;

  // 3)  scale
  myGroupLayout->addWidget( myLScale = new QLabel( tr( "SMESH_NB_SEGMENTS_SCALE_PARAM" ), GroupC1 ), row, 0 );
  myScale = new SMESHGUI_SpinBox( GroupC1 );
  myScale->RangeStepAndValidator( 1E-5, 1E+5, 0.1, 6 );
  myGroupLayout->addWidget( myScale, row, 1 );
  row++;

  myInfo = new QLabel( tr( "SMESH_FUNC_DOMAIN" ), GroupC1 );
  myGroupLayout->addMultiCellWidget( myInfo, row, row, 0, 1 );
  row++;
  
  // 4)  table
  myGroupLayout->addWidget( myLTable = new QLabel( tr( "SMESH_TAB_FUNC" ), GroupC1 ), row, 0 );
  myTable = new StdMeshersGUI_DistrTableFrame( GroupC1 );
  myGroupLayout->addWidget( myTable, row, 1 );
  myGroupLayout->setRowStretch( row, 1 );
  myTableRow = row;
  row++;

  // 5)  expression
  myGroupLayout->addWidget( myLExpr = new QLabel( tr( "SMESH_EXPR_FUNC" ), GroupC1 ), row, 0 );
  myExpr = new QLineEdit( GroupC1 );
  myGroupLayout->addWidget( myExpr, row, 1 );
  row++;

  // 6)  conversion (radiogroup)
  myGroupLayout->addWidget( myLConv = new QLabel( tr( "SMESH_CONV_MODE" ), GroupC1 ), row, 0 );
  myConv = new QButtonGroup( GroupC1 );
  myConv->setExclusive( true );
  myConv->setColumnLayout( 0, Qt::Vertical );
  QGridLayout* convLay = new QGridLayout( myConv->layout() );
  convLay->addWidget( new QRadioButton( tr( "SMESH_EXP_MODE" ), myConv ), 0, 0 );
  convLay->addWidget( myCutNeg = new QRadioButton( tr( "SMESH_CUT_NEG_MODE" ), myConv ), 1, 0 );
  myGroupLayout->addWidget( myConv, row, 1 );
  row++;

  // 7) distribution preview
  myGroupLayout->addMultiCellWidget( myPreview, row, row, 0, 1 );
  myGroupLayout->setRowStretch( row, 1 );
  myPreviewRow = row;
  row++;

  connect( myNbSeg, SIGNAL( valueChanged( const QString& ) ), this, SLOT( onValueChanged() ) );
  connect( myDistr, SIGNAL( activated( int ) ), this, SLOT( onValueChanged() ) );
  connect( myTable, SIGNAL( valueChanged( int, int ) ), this, SLOT( onValueChanged() ) );
  connect( myExpr,  SIGNAL( textChanged( const QString& ) ), this, SLOT( onValueChanged() ) );
  connect( myConv,  SIGNAL( clicked( int ) ), this, SLOT( onValueChanged() ) );

  return fr;
}

void StdMeshersGUI_NbSegmentsCreator::retrieveParams() const
{
  NbSegmentsHypothesisData data;
  readParamsFromHypo( data );

  if( myName )
    myName->setText( data.myName );
  myNbSeg->setValue( data.myNbSeg );
  myDistr->setCurrentItem( data.myDistrType );
  myScale->setValue( data.myScale );
  myConv->setButton( data.myConv );
  myTable->table()->setData( data.myTable );
  myExpr->setText( data.myExpr );
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
    for( int i=0; i < data.myTable.length(); i++, param = !param ) {
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
    if ( data.myConv )
      valStr += "; " + tr("SMESH_CUT_NEG_MODE");
    else
      valStr += "; " + tr("SMESH_EXP_MODE");

  return valStr;
}

bool StdMeshersGUI_NbSegmentsCreator::readParamsFromHypo( NbSegmentsHypothesisData& h_data ) const
{
  StdMeshers::StdMeshers_NumberOfSegments_var h =
    StdMeshers::StdMeshers_NumberOfSegments::_narrow( initParamsHypothesis() );

  HypothesisData* data = SMESH::GetHypothesisData( hypType() );
  h_data.myName = isCreation() && data ? data->Label : "";

  h_data.myNbSeg = (int) h->GetNumberOfSegments();
  int distr = (int) h->GetDistrType();
  h_data.myDistrType = distr;
  h_data.myScale = distr==1 ? h->GetScaleFactor() : 1.0;
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
      SMESH::SetName( SMESH::FindSObject( h ), h_data.myName.latin1() );

    h->SetNumberOfSegments( h_data.myNbSeg );
    int distr = h_data.myDistrType;
    h->SetDistrType( distr );

    if( distr==1 )
      h->SetScaleFactor( h_data.myScale );

    if( distr==2 || distr==3 )
      h->SetConversionMode( h_data.myConv );

    if( distr==2 )
      h->SetTableFunction( h_data.myTable );

    if( distr==3 )
      h->SetExpressionFunction( h_data.myExpr.latin1() );
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
  h_data.myDistrType = myDistr->currentItem();
  h_data.myConv      = myConv->id( myConv->selected() );
  h_data.myScale     = myScale->value();
  myTable->table()->data( h_data.myTable );
  h_data.myExpr      = myExpr->text();
  return true;
}

void StdMeshersGUI_NbSegmentsCreator::onValueChanged()
{
  int distr = myDistr->currentItem();

/*  if( distr==2 ) //table func
    myCutNeg->setText( tr( "SMESH_NO_CONV" ) );
  else if( distr==3 )
    myCutNeg->setText( tr( "SMESH_CUT_NEG_MODE" ) );*/

  if( distr==2 && sender()==myConv ) //table func
  {
    myTable->table()->funcValidator()->setBottom( myConv->id( myConv->selected() )==0 ? -1E20 : 0 );
    SMESH::double_array arr;
    myTable->table()->data( arr );
    myTable->table()->setData( arr ); //update data in table
  }

  myScale->setShown( distr==1 );
  myLScale->setShown( distr==1 );

  bool isFunc = distr==2 || distr==3;
  myPreview->setShown( isFunc );
  myGroupLayout->setRowStretch( myPreviewRow, isFunc ? 1 : 0 );

  myConv->setShown( isFunc );
  myLConv->setShown( isFunc );

  if( distr==2 )
    myTable->show();
  else
    myTable->hide();
  myLTable->setShown( distr==2 );
  myGroupLayout->setRowStretch( myTableRow, distr==2 ? 1 : 0 );

  myExpr->setShown( distr==3 );
  myLExpr->setShown( distr==3 );
  myInfo->setShown( isFunc );

  //change of preview
  int nbSeg = myNbSeg->value();
  if( distr==2 ) //preview for table-described function
  {
    SMESH::double_array a;
    myTable->table()->data( a );
    myPreview->setParams( a, nbSeg, false );
  }
  else if( distr==3 ) //preview for analytic-described function
    myPreview->setParams( myExpr->text(), nbSeg, 100, false );

  if( isFunc )
    myPreview->setConversion( StdMeshersGUI_DistrPreview::Conversion( myConv->id( myConv->selected() ) ) );
}
