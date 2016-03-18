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
//  File   : StdMeshersGUI_StdHypothesisCreator.cxx
//  Author : Alexander SOLOVYOV, Open CASCADE S.A.S.
//  SMESH includes

#include "StdMeshersGUI_StdHypothesisCreator.h"

#include <SMESHGUI.h>
#include <SMESHGUI_SpinBox.h>
#include <SMESHGUI_HypothesesUtils.h>
#include <SMESHGUI_Utils.h>
#include <SMESHGUI_GEOMGenUtils.h>
#include <SMESH_TypeFilter.hxx>
#include <SMESH_NumberFilter.hxx>

#include "StdMeshersGUI_FixedPointsParamWdg.h"
#include "StdMeshersGUI_LayerDistributionParamWdg.h"
#include "StdMeshersGUI_ObjectReferenceParamWdg.h"
#include "StdMeshersGUI_PropagationHelperWdg.h"
#include "StdMeshersGUI_QuadrangleParamWdg.h"
#include "StdMeshersGUI_RadioButtonsGrpWdg.h"
#include "StdMeshersGUI_SubShapeSelectorWdg.h"

#include <SALOMEDSClient_Study.hxx>

#include <GEOM_wrap.hxx>

// SALOME GUI includes
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <SalomeApp_IntSpinBox.h>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)

// Qt includes
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QButtonGroup>

const double VALUE_MAX = 1.0e+15, // COORD_MAX
             VALUE_MAX_2  = VALUE_MAX * VALUE_MAX,
             VALUE_MAX_3  = VALUE_MAX_2 * VALUE_MAX,
             VALUE_SMALL = 1.0e-15,
             VALUE_SMALL_2 = VALUE_SMALL * VALUE_SMALL,
             VALUE_SMALL_3 = VALUE_SMALL_2 * VALUE_SMALL;

//================================================================================
/*!
 * \brief Constructor
  * \param type - hypothesis type
 */
//================================================================================

StdMeshersGUI_StdHypothesisCreator::StdMeshersGUI_StdHypothesisCreator( const QString& type )
  : SMESHGUI_GenericHypothesisCreator( type ), myHelperWidget( 0 )
{
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

StdMeshersGUI_StdHypothesisCreator::~StdMeshersGUI_StdHypothesisCreator()
{
}

//================================================================================
/*!
 * \brief Return widget for i-th hypothesis parameter (got from myParamWidgets)
  * \param i - index of hypothesis parameter
  * \retval QWidget* - found widget
 */
//================================================================================

QWidget* StdMeshersGUI_StdHypothesisCreator::getWidgetForParam( int i ) const
{
  QWidget* w = 0;
  if ( isCreation() ) ++i; // skip widget of 'name' parameter

  if ( i < myCustomWidgets.count() ) {
    QList<QWidget*>::const_iterator anIt  = myCustomWidgets.begin();
    QList<QWidget*>::const_iterator aLast = myCustomWidgets.end();
    for ( int j = 0 ; !w && anIt != aLast; ++anIt, ++j )
      if ( i == j )
        w = *anIt;
  }
  if ( !w ) {
    // list has no at() const, so we iterate
    QList<QWidget*>::const_iterator anIt  = widgets().begin();
    QList<QWidget*>::const_iterator aLast = widgets().end();
    for( int j = 0; !w && anIt!=aLast; anIt++, ++j ) {
      if ( i == j )
        w = *anIt;
    }
  }
  return w;
}

//================================================================================
/*!
 * \brief Allow modifing myCustomWidgets in const methods
  * \retval ListOfWidgets* - non-const pointer to myCustomWidgets
 */
//================================================================================

StdMeshersGUI_StdHypothesisCreator::ListOfWidgets*
StdMeshersGUI_StdHypothesisCreator::customWidgets() const
{
  return const_cast< ListOfWidgets* >( & myCustomWidgets );
}

//================================================================================
/*!
 * \brief Builds dlg layout
  * \retval QFrame* - the built widget
 */
//================================================================================

QFrame* StdMeshersGUI_StdHypothesisCreator::buildFrame()
{
  return buildStdFrame();
}

//================================================================================
/*!
 * \brief Initialise parameter values in controls
 */
//================================================================================

void StdMeshersGUI_StdHypothesisCreator::retrieveParams() const
{
  // buildStdFrame() sets values itself calling stdParams()

  if ( hypType().startsWith("ProjectionSource" ))
  {
    // we use this method to connect depending custom widgets
    StdMeshersGUI_ObjectReferenceParamWdg* widgetToActivate = 0;
    ListOfWidgets::const_iterator anIt = myCustomWidgets.begin();
    for ( ; anIt != myCustomWidgets.end(); anIt++)
    {
      if ( *anIt && (*anIt)->inherits("StdMeshersGUI_ObjectReferenceParamWdg"))
      {
        StdMeshersGUI_ObjectReferenceParamWdg * w1 =
          ( StdMeshersGUI_ObjectReferenceParamWdg* ) ( *anIt );
        ListOfWidgets::const_iterator anIt2 = anIt;
        for ( ++anIt2; anIt2 != myCustomWidgets.end(); anIt2++)
          if ( *anIt2 && (*anIt2)->inherits("StdMeshersGUI_ObjectReferenceParamWdg"))
          {
            StdMeshersGUI_ObjectReferenceParamWdg * w2 =
              ( StdMeshersGUI_ObjectReferenceParamWdg* ) ( *anIt2 );
            w1->AvoidSimultaneousSelection( w2 );
          }
        if ( !widgetToActivate )
          widgetToActivate = w1;
      }
    }
    if ( widgetToActivate )
      widgetToActivate->activateSelection();
  }

  if ( dlg() )
    dlg()->setMinimumSize( dlg()->minimumSizeHint().width(), dlg()->minimumSizeHint().height() );
}

namespace {

  //================================================================================
  /*!
   * \brief Widget: slider with left and right labels
   */
  //================================================================================

  class TDoubleSliderWith2Labels: public QWidget
  {
  public:
    TDoubleSliderWith2Labels( const QString& leftLabel, const QString& rightLabel,
                              const double   initValue, const double   bottom,
                              const double   top      , const double   precision,
                              QWidget *      parent=0 , const char *   name=0 )
      :QWidget(parent), _bottom(bottom), _precision(precision)
    {
      setObjectName(name);

      QHBoxLayout* aHBoxL = new QHBoxLayout(this);

      if ( !leftLabel.isEmpty() ) {
        QLabel* aLeftLabel = new QLabel( this );
        aLeftLabel->setText( leftLabel );
        aHBoxL->addWidget( aLeftLabel );
      }

      _slider = new QSlider( Qt::Horizontal, this );
      _slider->setRange( 0, toInt( top ));
      _slider->setValue( toInt( initValue ));
      aHBoxL->addWidget( _slider );

      if ( !rightLabel.isEmpty() ) {
        QLabel* aRightLabel = new QLabel( this );
        aRightLabel->setText( rightLabel );
        aHBoxL->addWidget( aRightLabel );
      }

      setLayout( aHBoxL );
    }
    double value() const { return _bottom + _slider->value() * _precision; }
    QSlider * getSlider() const { return _slider; }
    int toInt( double val ) const { return (int) ceil(( val - _bottom ) / _precision ); }
  private:
    double _bottom, _precision;
    QSlider * _slider;
  };

  //================================================================================
  /*!
   * \brief Retrieve GEOM_Object held by widget
   */
  //================================================================================

  inline GEOM::GEOM_Object_var geomFromWdg(const QWidget* wdg)
  {
    const StdMeshersGUI_ObjectReferenceParamWdg * objRefWdg =
      dynamic_cast<const StdMeshersGUI_ObjectReferenceParamWdg*>( wdg );
    if ( objRefWdg )
      return objRefWdg->GetObject< GEOM::GEOM_Object >();

    return GEOM::GEOM_Object::_nil();
  }
  //================================================================================
  /*!
   * \brief Retrieve SMESH_Mesh held by widget
   */
  //================================================================================

  inline SMESH::SMESH_Mesh_var meshFromWdg(const QWidget* wdg)
  {
    const StdMeshersGUI_ObjectReferenceParamWdg * objRefWdg =
      dynamic_cast<const StdMeshersGUI_ObjectReferenceParamWdg*>( wdg );
    if ( objRefWdg )
      return objRefWdg->GetObject< SMESH::SMESH_Mesh >();

    return SMESH::SMESH_Mesh::_nil();
  }
  //================================================================================
  /*!
   * \brief Retrieve SMESH_Mesh held by widget
   */
  //================================================================================

  inline SMESH::ListOfGroups_var groupsFromWdg(const QWidget* wdg)
  {
    SMESH::ListOfGroups_var groups = new SMESH::ListOfGroups;
    const StdMeshersGUI_ObjectReferenceParamWdg * objRefWdg =
      dynamic_cast<const StdMeshersGUI_ObjectReferenceParamWdg*>( wdg );
    if ( objRefWdg )
    {
      groups->length( objRefWdg->NbObjects() );
      for ( unsigned i = 0; i < groups->length(); ++i )
        groups[i] = objRefWdg->GetObject< SMESH::SMESH_GroupBase >(i);
    }
    return groups;
  }
  //================================================================================
  /*!
   * \brief creates a filter for selection of shapes of given dimension
    * \param dim - dimension
    * \param subShapeType - required type of sub-shapes, number of which must be \a nbSubShapes
    * \param nbSubShapes - number of sub-shapes of given type
    * \param closed - required closeness flag of a shape
    * \retval SUIT_SelectionFilter* - created filter
   */
  //================================================================================

  SUIT_SelectionFilter* filterForShapeOfDim(const int        dim,
                                            TopAbs_ShapeEnum subShapeType = TopAbs_SHAPE,
                                            const int        nbSubShapes = 0,
                                            bool             closed = false)
  {
    TColStd_MapOfInteger shapeTypes;
    switch ( dim ) {
    case 0: shapeTypes.Add( TopAbs_VERTEX ); break;
    case 1:
      if ( subShapeType == TopAbs_SHAPE ) subShapeType = TopAbs_EDGE;
      shapeTypes.Add( TopAbs_EDGE );
      shapeTypes.Add( TopAbs_COMPOUND ); // for a group
      break;
    case 2:
      if ( subShapeType == TopAbs_SHAPE ) subShapeType = TopAbs_FACE;
      shapeTypes.Add( TopAbs_FACE );
      shapeTypes.Add( TopAbs_COMPOUND ); // for a group
      break;
    case 3:
      shapeTypes.Add( TopAbs_SHELL );
      shapeTypes.Add( TopAbs_SOLID );
      shapeTypes.Add( TopAbs_COMPSOLID );
      shapeTypes.Add( TopAbs_COMPOUND );
      break;
    }
    return new SMESH_NumberFilter("GEOM", subShapeType, nbSubShapes,
                                  shapeTypes, GEOM::GEOM_Object::_nil(), closed);
  }

  //================================================================================
  /*!
   * \brief Create a widget for object selection
    * \param object - initial object
    * \param filter - selection filter
    * \retval QWidget* - created widget
   */
  //================================================================================

  QWidget* newObjRefParamWdg( SUIT_SelectionFilter* filter,
                              CORBA::Object_var     object)
  {
    StdMeshersGUI_ObjectReferenceParamWdg* w =
      new StdMeshersGUI_ObjectReferenceParamWdg( filter, 0);
    w->SetObject( object.in() );
    return w;
  }
  QWidget* newObjRefParamWdg( SUIT_SelectionFilter*    filter,
                              SMESH::string_array_var& objEntries)
  {
    StdMeshersGUI_ObjectReferenceParamWdg* w =
      new StdMeshersGUI_ObjectReferenceParamWdg( filter, 0, /*multiSel=*/true);
    //RNV: Firstly, activate selection, then set objects
    w->activateSelection();
    w->SetObjects( objEntries );
    return w;
  }

  //================================================================================
  /*!
   * \brief calls deactivateSelection() for StdMeshersGUI_ObjectReferenceParamWdg
    * \param widgetList - list of widgets
   */
  //================================================================================

  void deactivateObjRefParamWdg( QList<QWidget*>* widgetList )
  {
    StdMeshersGUI_ObjectReferenceParamWdg* w = 0;
    QList<QWidget*>::iterator anIt  = widgetList->begin();
    QList<QWidget*>::iterator aLast = widgetList->end();
    for ( ; anIt != aLast; anIt++ ) {
      if ( (*anIt) && (*anIt)->inherits( "StdMeshersGUI_ObjectReferenceParamWdg" ))
      {
        w = (StdMeshersGUI_ObjectReferenceParamWdg* )( *anIt );
        w->deactivateSelection();
      }
    }
  }
}

//================================================================================
/*!
 * \brief Check parameter values before accept()
  * \retval bool - true if OK
 */
//================================================================================

bool StdMeshersGUI_StdHypothesisCreator::checkParams( QString& msg ) const
{
  if( !SMESHGUI_GenericHypothesisCreator::checkParams( msg ) )
    return false;

  // check if object reference parameter is set, as it has no default value
  bool ok = true;
  if ( hypType().startsWith("ProjectionSource" ))
  {
    StdMeshersGUI_ObjectReferenceParamWdg* w =
      widget< StdMeshersGUI_ObjectReferenceParamWdg >( 0 );
    ok = ( w->IsObjectSelected() );
    if ( !ok ) w->SetObject( CORBA::Object::_nil() );
    int nbAssocVert = ( hypType() == "ProjectionSource1D" ? 1 : 2 );
    int nbNonEmptyAssoc = 0;
    for ( int i = 0; ok && i < nbAssocVert*2; i += 2)
    {
      QString srcV, tgtV;
      StdMeshersGUI_ObjectReferenceParamWdg* w1 =
        widget< StdMeshersGUI_ObjectReferenceParamWdg >( i+2 );
      StdMeshersGUI_ObjectReferenceParamWdg* w2 =
        widget< StdMeshersGUI_ObjectReferenceParamWdg >( i+3 );
      srcV = w1->GetValue();
      tgtV = w2->GetValue();
      ok = (( srcV.isEmpty()  && tgtV.isEmpty() ) ||
            ( !srcV.isEmpty() && !tgtV.isEmpty() /*&& srcV != tgtV*/ ));
      if ( !ok ) {
        w1->SetObject( CORBA::Object::_nil() );
        w2->SetObject( CORBA::Object::_nil() );
      }
      nbNonEmptyAssoc += !srcV.isEmpty();
    }
    if ( ok && nbNonEmptyAssoc == 1 && nbAssocVert == 2 )
    {
      // only one pair of VERTEXes is given for a FACE,
      // then the FACE must have only one VERTEX
      GEOM::GEOM_Object_var face = w->GetObject< GEOM::GEOM_Object >();

      GEOM::GEOM_Gen_var geomGen = SMESH::GetGEOMGen();
      _PTR(Study)         aStudy = SMESH::GetActiveStudyDocument();
      GEOM::GEOM_IShapesOperations_wrap shapeOp;
      if ( !geomGen->_is_nil() && aStudy )
        shapeOp = geomGen->GetIShapesOperations( aStudy->StudyId() );
      if ( !shapeOp->_is_nil() )
      {
        GEOM::ListOfLong_var vertices =
          shapeOp->GetAllSubShapesIDs (face, GEOM::VERTEX, /*isSorted=*/false);
        ok = ( vertices->length() == 1 );
      }
    }
    // Uninstall filters of StdMeshersGUI_ObjectReferenceParamWdg
    if ( ok )
      deactivateObjRefParamWdg( customWidgets() );
  }
  else if ( hypType().startsWith("ImportSource" ))
  {
    StdMeshersGUI_ObjectReferenceParamWdg* w =
      widget< StdMeshersGUI_ObjectReferenceParamWdg >( 0 );
    ok = ( w->IsObjectSelected() );
  }
  else if ( hypType() == "LayerDistribution" || hypType() == "LayerDistribution2D" )
  {
    StdMeshersGUI_LayerDistributionParamWdg* w = 
      widget< StdMeshersGUI_LayerDistributionParamWdg >( 0 );
    ok = ( w && w->IsOk() );
  }

  return ok;
}

//================================================================================
/*!
 * \brief Store params from GUI controls to a hypothesis
  * \retval QString - text representation of parameters
 */
//================================================================================

QString StdMeshersGUI_StdHypothesisCreator::storeParams() const
{
  ListOfStdParams params;
  bool res = getStdParamFromDlg( params );
  if( isCreation() )
  {
    SMESH::SetName( SMESH::FindSObject( hypothesis() ), params[0].myValue.toString().toLatin1().data() );
    params.erase( params.begin() );
  }

  QString valueStr = stdParamValues( params );
  //QStringList aVariablesList = getVariablesFromDlg();

  if( res && !params.isEmpty() )
  {
    if( hypType()=="LocalLength" )
    {
      StdMeshers::StdMeshers_LocalLength_var h =
        StdMeshers::StdMeshers_LocalLength::_narrow( hypothesis() );

      h->SetVarParameter( params[0].text(), "SetLength" );
      h->SetLength( params[0].myValue.toDouble() );
      h->SetVarParameter( params[1].text(), "SetPrecision" );
      h->SetPrecision( params[1].myValue.toDouble() );
    }
    else if( hypType()=="MaxLength" )
    {
      StdMeshers::StdMeshers_MaxLength_var h =
        StdMeshers::StdMeshers_MaxLength::_narrow( hypothesis() );

      h->SetVarParameter( params[0].text(), "SetLength" );
      h->SetLength( params[0].myValue.toDouble() );
      h->SetUsePreestimatedLength( widget< QCheckBox >( 1 )->isChecked() );
      if ( !h->HavePreestimatedLength() && !h->_is_equivalent( initParamsHypothesis() )) {
        StdMeshers::StdMeshers_MaxLength_var hInit =
          StdMeshers::StdMeshers_MaxLength::_narrow( initParamsHypothesis() );
        h->SetPreestimatedLength( hInit->GetPreestimatedLength() );
      }
    }
    else if( hypType()=="SegmentLengthAroundVertex" )
    {
      StdMeshers::StdMeshers_SegmentLengthAroundVertex_var h =
        StdMeshers::StdMeshers_SegmentLengthAroundVertex::_narrow( hypothesis() );

      h->SetVarParameter( params[0].text(), "SetLength" );
      h->SetLength( params[0].myValue.toDouble() );
    }
    else if( hypType()=="Arithmetic1D" )
    {
      StdMeshers::StdMeshers_Arithmetic1D_var h =
        StdMeshers::StdMeshers_Arithmetic1D::_narrow( hypothesis() );

      StdMeshersGUI_SubShapeSelectorWdg* w = 
        widget< StdMeshersGUI_SubShapeSelectorWdg >( 2 );

      h->SetVarParameter( params[0].text(), "SetStartLength" );
      h->SetStartLength( params[0].myValue.toDouble() );
      h->SetVarParameter( params[1].text(), "SetEndLength" );
      h->SetEndLength( params[1].myValue.toDouble() );
      if (w) {
        h->SetReversedEdges( w->GetListOfIDs() );
        h->SetObjectEntry( w->GetMainShapeEntry() );
      }
    }
    else if( hypType()=="GeometricProgression" )
    {
      StdMeshers::StdMeshers_Geometric1D_var h =
        StdMeshers::StdMeshers_Geometric1D::_narrow( hypothesis() );

      StdMeshersGUI_SubShapeSelectorWdg* w = 
        widget< StdMeshersGUI_SubShapeSelectorWdg >( 2 );

      h->SetVarParameter( params[0].text(), "SetStartLength" );
      h->SetStartLength( params[0].myValue.toDouble() );
      h->SetVarParameter( params[1].text(), "SetCommonRatio" );
      h->SetCommonRatio( params[1].myValue.toDouble() );
      if (w) {
        h->SetReversedEdges( w->GetListOfIDs() );
        h->SetObjectEntry( w->GetMainShapeEntry() );
      }
    }
    else if( hypType()=="FixedPoints1D" )
    {
      StdMeshers::StdMeshers_FixedPoints1D_var h =
        StdMeshers::StdMeshers_FixedPoints1D::_narrow( hypothesis() );

      StdMeshersGUI_FixedPointsParamWdg* w1 = 
        widget< StdMeshersGUI_FixedPointsParamWdg >( 0 );

      StdMeshersGUI_SubShapeSelectorWdg* w2 = 
        widget< StdMeshersGUI_SubShapeSelectorWdg >( 1 );

      if (w1) {
        h->SetPoints( w1->GetListOfPoints() );
        h->SetNbSegments( w1->GetListOfSegments() );
      }
      if (w2) {
        h->SetReversedEdges( w2->GetListOfIDs() );
        h->SetObjectEntry( w2->GetMainShapeEntry() );
      }
    }
    else if( hypType()=="MaxElementArea" )
    {
      StdMeshers::StdMeshers_MaxElementArea_var h =
        StdMeshers::StdMeshers_MaxElementArea::_narrow( hypothesis() );
      h->SetVarParameter( params[0].text(), "SetMaxElementArea" );
      h->SetMaxElementArea( params[0].myValue.toDouble() );
    }
    else if( hypType()=="MaxElementVolume" )
    {
      StdMeshers::StdMeshers_MaxElementVolume_var h =
        StdMeshers::StdMeshers_MaxElementVolume::_narrow( hypothesis() );

      h->SetVarParameter( params[0].text(), "SetMaxElementVolume" );
      h->SetMaxElementVolume( params[0].myValue.toDouble() );
    }
    else if( hypType()=="StartEndLength" )
    {
      StdMeshers::StdMeshers_StartEndLength_var h =
        StdMeshers::StdMeshers_StartEndLength::_narrow( hypothesis() );

      StdMeshersGUI_SubShapeSelectorWdg* w = 
        widget< StdMeshersGUI_SubShapeSelectorWdg >( 2 );

      h->SetVarParameter( params[0].text(), "SetStartLength" );
      h->SetStartLength( params[0].myValue.toDouble() );
      h->SetVarParameter( params[1].text(), "SetEndLength" );
      h->SetEndLength( params[1].myValue.toDouble() );
      if (w) {
        h->SetReversedEdges( w->GetListOfIDs() );
        h->SetObjectEntry( w->GetMainShapeEntry() );
      }
    }
    else if( hypType()=="Deflection1D" )
    {
      StdMeshers::StdMeshers_Deflection1D_var h =
        StdMeshers::StdMeshers_Deflection1D::_narrow( hypothesis() );
      h->SetVarParameter( params[0].text(), "SetDeflection" );
      h->SetDeflection( params[0].myValue.toDouble() );
    }
    else if( hypType()=="Adaptive1D" )
    {
      StdMeshers::StdMeshers_Adaptive1D_var h =
        StdMeshers::StdMeshers_Adaptive1D::_narrow( hypothesis() );
      h->SetVarParameter( params[0].text(), "SetMinSize" );
      h->SetMinSize( params[0].myValue.toDouble() );
      h->SetVarParameter( params[0].text(), "SetMaxSize" );
      h->SetMaxSize( params[1].myValue.toDouble() );
      h->SetVarParameter( params[0].text(), "SetDeflection" );
      h->SetDeflection( params[2].myValue.toDouble() );
    }
    else if( hypType()=="AutomaticLength" )
    {
      StdMeshers::StdMeshers_AutomaticLength_var h =
        StdMeshers::StdMeshers_AutomaticLength::_narrow( hypothesis() );

      h->SetVarParameter( params[0].text(), "SetFineness" );
      h->SetFineness( params[0].myValue.toDouble() );
    }
    else if( hypType()=="NumberOfLayers" )
    {
      StdMeshers::StdMeshers_NumberOfLayers_var h =
        StdMeshers::StdMeshers_NumberOfLayers::_narrow( hypothesis() );

      h->SetVarParameter( params[0].text(), "SetNumberOfLayers" );
      h->SetNumberOfLayers( params[0].myValue.toInt() );
    }
    else if( hypType()=="LayerDistribution" )
    {
      StdMeshers::StdMeshers_LayerDistribution_var h =
        StdMeshers::StdMeshers_LayerDistribution::_narrow( hypothesis() );
      StdMeshersGUI_LayerDistributionParamWdg* w = 
        widget< StdMeshersGUI_LayerDistributionParamWdg >( 0 );
      
      h->SetLayerDistribution( w->GetHypothesis() );
    }
    else if( hypType()=="NumberOfLayers2D" )
    {
      StdMeshers::StdMeshers_NumberOfLayers2D_var h =
        StdMeshers::StdMeshers_NumberOfLayers2D::_narrow( hypothesis() );

      h->SetVarParameter( params[0].text(), "SetNumberOfLayers" );
      h->SetNumberOfLayers( params[0].myValue.toInt() );
    }
    else if( hypType()=="LayerDistribution2D" )
    {
      StdMeshers::StdMeshers_LayerDistribution2D_var h =
        StdMeshers::StdMeshers_LayerDistribution2D::_narrow( hypothesis() );
      StdMeshersGUI_LayerDistributionParamWdg* w = 
        widget< StdMeshersGUI_LayerDistributionParamWdg >( 0 );
      
      h->SetLayerDistribution( w->GetHypothesis() );
    }
    else if( hypType()=="ProjectionSource1D" )
    {
      StdMeshers::StdMeshers_ProjectionSource1D_var h =
        StdMeshers::StdMeshers_ProjectionSource1D::_narrow( hypothesis() );

      h->SetSourceEdge       ( geomFromWdg ( getWidgetForParam( 0 )));
      h->SetSourceMesh       ( meshFromWdg ( getWidgetForParam( 1 )));
      h->SetVertexAssociation( geomFromWdg ( getWidgetForParam( 2 )),
                               geomFromWdg ( getWidgetForParam( 3 )));
    }
    else if( hypType()=="ProjectionSource2D" )
    {
      StdMeshers::StdMeshers_ProjectionSource2D_var h =
        StdMeshers::StdMeshers_ProjectionSource2D::_narrow( hypothesis() );

      h->SetSourceFace       ( geomFromWdg ( getWidgetForParam( 0 )));
      h->SetSourceMesh       ( meshFromWdg ( getWidgetForParam( 1 )));
      h->SetVertexAssociation( geomFromWdg ( getWidgetForParam( 2 )), // src1
                               geomFromWdg ( getWidgetForParam( 4 )), // src2
                               geomFromWdg ( getWidgetForParam( 3 )), // tgt1
                               geomFromWdg ( getWidgetForParam( 5 ))); // tgt2
    }
    else if( hypType()=="ProjectionSource3D" )
    {
      StdMeshers::StdMeshers_ProjectionSource3D_var h =
        StdMeshers::StdMeshers_ProjectionSource3D::_narrow( hypothesis() );

      h->SetSource3DShape    ( geomFromWdg ( getWidgetForParam( 0 )));
      h->SetSourceMesh       ( meshFromWdg ( getWidgetForParam( 1 )));
      h->SetVertexAssociation( geomFromWdg ( getWidgetForParam( 2 )), // src1
                               geomFromWdg ( getWidgetForParam( 4 )), // src2
                               geomFromWdg ( getWidgetForParam( 3 )), // tgt1
                               geomFromWdg ( getWidgetForParam( 5 ))); // tgt2
    }
    else if( hypType()=="ImportSource1D" )
    {
      StdMeshers::StdMeshers_ImportSource1D_var h =
        StdMeshers::StdMeshers_ImportSource1D::_narrow( hypothesis() );

      SMESH::ListOfGroups_var groups = groupsFromWdg( getWidgetForParam( 0 ));
      h->SetSourceEdges( groups.in() );
      QCheckBox* toCopyMesh   = widget< QCheckBox >( 1 );
      QCheckBox* toCopyGroups = widget< QCheckBox >( 2 );
      h->SetCopySourceMesh( toCopyMesh->isChecked(), toCopyGroups->isChecked());
    }
    else if( hypType()=="ImportSource2D" )
    {
      StdMeshers::StdMeshers_ImportSource2D_var h =
        StdMeshers::StdMeshers_ImportSource2D::_narrow( hypothesis() );

      SMESH::ListOfGroups_var groups = groupsFromWdg( getWidgetForParam( 0 ));
      h->SetSourceFaces( groups.in() );
      QCheckBox* toCopyMesh   = widget< QCheckBox >( 1 );
      QCheckBox* toCopyGroups = widget< QCheckBox >( 2 );
      h->SetCopySourceMesh( toCopyMesh->isChecked(), toCopyGroups->isChecked());
    }
    else if( hypType()=="ViscousLayers" )
    {
      StdMeshers::StdMeshers_ViscousLayers_var h =
        StdMeshers::StdMeshers_ViscousLayers::_narrow( hypothesis() );

      h->SetVarParameter  ( params[0].text(), "SetTotalThickness" );
      h->SetTotalThickness( params[0].myValue.toDouble() );
      h->SetVarParameter  ( params[1].text(), "SetNumberLayers" );
      h->SetNumberLayers  ( params[1].myValue.toInt() );
      h->SetVarParameter  ( params[2].text(), "SetStretchFactor" );
      h->SetStretchFactor ( params[2].myValue.toDouble() );
      h->SetMethod (( StdMeshers::VLExtrusionMethod ) params[3].myValue.toInt() );

      if ( StdMeshersGUI_SubShapeSelectorWdg* idsWg =
           widget< StdMeshersGUI_SubShapeSelectorWdg >( 5 ))
      {
        h->SetFaces( idsWg->GetListOfIDs(), params[4].myValue.toInt() );
      }
    }
    else if( hypType()=="ViscousLayers2D" )
    {
      StdMeshers::StdMeshers_ViscousLayers2D_var h =
        StdMeshers::StdMeshers_ViscousLayers2D::_narrow( hypothesis() );

      h->SetVarParameter  ( params[0].text(), "SetTotalThickness" );
      h->SetTotalThickness( params[0].myValue.toDouble() );
      h->SetVarParameter  ( params[1].text(), "SetNumberLayers" );
      h->SetNumberLayers  ( params[1].myValue.toInt() );
      h->SetVarParameter  ( params[2].text(), "SetStretchFactor" );
      h->SetStretchFactor ( params[2].myValue.toDouble() );

      if ( StdMeshersGUI_SubShapeSelectorWdg* idsWg =
           widget< StdMeshersGUI_SubShapeSelectorWdg >( 4 ))
      {
        h->SetEdges( idsWg->GetListOfIDs(), params[3].myValue.toInt() );
      }
    }
    // else if( hypType()=="QuadrangleParams" )
    // {
    //   StdMeshers::StdMeshers_QuadrangleParams_var h =
    //     StdMeshers::StdMeshers_QuadrangleParams::_narrow( hypothesis() );
    //   StdMeshersGUI_SubShapeSelectorWdg* w1 =
    //     widget< StdMeshersGUI_SubShapeSelectorWdg >( 0 );
    //   StdMeshersGUI_QuadrangleParamWdg* w2 =
    //     widget< StdMeshersGUI_QuadrangleParamWdg >( 1 );
    //   if (w1 && w2) {
    //     if (w1->GetListSize() > 0) {
    //       h->SetTriaVertex(w1->GetListOfIDs()[0]); // getlist must be called once
    //       const char * entry = w1->GetMainShapeEntry();
    //       h->SetObjectEntry(entry);
    //     }
    //     h->SetQuadType(StdMeshers::QuadType(w2->GetType()));
    //   }
    // }
  }
  return valueStr;
}

//================================================================================
/*!
 * \brief Return parameter values as SMESHGUI_GenericHypothesisCreator::StdParam
 * \param p - list of parameters
 * \retval bool - success flag
 *
 * Is called from SMESHGUI_GenericHypothesisCreator::buildStdFrame().
 * Parameters will be shown using "standard" controls:
 *   Int by QtxIntSpinBox
 *   Double by SMESHGUI_SpinBox
 *   String by QLineEdit
 * getCustomWidget() allows to redefine control for a parameter
 */
//================================================================================

bool StdMeshersGUI_StdHypothesisCreator::stdParams( ListOfStdParams& p ) const
{
  bool res = true;
  SMESHGUI_GenericHypothesisCreator::StdParam item;

  p.clear();
  customWidgets()->clear();
  if( isCreation() )
  {
    HypothesisData* data = SMESH::GetHypothesisData( hypType() );
    item.myName = tr( "SMESH_NAME" );
    item.myValue = data ? hypName() : QString();
    p.append( item );
    customWidgets()->append(0);
  }

  SMESH::SMESH_Hypothesis_var hyp = initParamsHypothesis();
  //SMESH::ListOfParameters_var aParameters = hyp->GetLastParameters();

  if( hypType()=="LocalLength" )
  {
    StdMeshers::StdMeshers_LocalLength_var h =
      StdMeshers::StdMeshers_LocalLength::_narrow( hyp );

    item.myName = tr("SMESH_LOCAL_LENGTH_PARAM");
    if(!initVariableName( hyp, item, "SetLength"))
      item.myValue = h->GetLength();
    p.append( item );     

    item.myName = tr("SMESH_LOCAL_LENGTH_PRECISION");
    if(!initVariableName( hyp, item, "SetPrecision"))
      item.myValue = h->GetPrecision(); 
    p.append( item );
  }
  else if( hypType()=="MaxLength" )
  {
    StdMeshers::StdMeshers_MaxLength_var h =
      StdMeshers::StdMeshers_MaxLength::_narrow( hyp );
    // try to set a right preestimated length to edited hypothesis
    bool noPreestimatedAtEdition = false;
    if ( !isCreation() ) {
      StdMeshers::StdMeshers_MaxLength_var initHyp =
        StdMeshers::StdMeshers_MaxLength::_narrow( initParamsHypothesis(true) );
      noPreestimatedAtEdition =
        ( initHyp->_is_nil() || !initHyp->HavePreestimatedLength() );
      if ( !noPreestimatedAtEdition )
        h->SetPreestimatedLength( initHyp->GetPreestimatedLength() );
    }

    item.myName = tr("SMESH_LOCAL_LENGTH_PARAM");
    if(!initVariableName( hyp, item, "SetLength"))
      item.myValue = h->GetLength();
    p.append( item );
    customWidgets()->append(0);

    item.myName = tr("SMESH_USE_PREESTIMATED_LENGTH");
    p.append( item );
    QCheckBox* aQCheckBox = new QCheckBox(dlg());
    if ( !noPreestimatedAtEdition && h->HavePreestimatedLength() ) {
      aQCheckBox->setChecked( h->GetUsePreestimatedLength() );
      connect( aQCheckBox, SIGNAL(  stateChanged(int) ), this, SLOT( onValueChanged() ) );
    }
    else {
      aQCheckBox->setChecked( false );
      aQCheckBox->setEnabled( false );
    }
    customWidgets()->append( aQCheckBox );
  }
  else if( hypType()=="SegmentLengthAroundVertex" )
  {
    StdMeshers::StdMeshers_SegmentLengthAroundVertex_var h =
      StdMeshers::StdMeshers_SegmentLengthAroundVertex::_narrow( hyp );

    item.myName = tr("SMESH_LOCAL_LENGTH_PARAM");
    if(!initVariableName( hyp, item, "SetLength"))
      item.myValue = h->GetLength();
    
    p.append( item );
  }
  else if( hypType()=="Arithmetic1D" )
  {
    StdMeshers::StdMeshers_Arithmetic1D_var h =
      StdMeshers::StdMeshers_Arithmetic1D::_narrow( hyp );

    item.myName = tr( "SMESH_START_LENGTH_PARAM" );
    if(!initVariableName( hyp, item, "SetStartLength" ))
      item.myValue = h->GetLength( true );
    p.append( item );

    customWidgets()->append (0);

    item.myName = tr( "SMESH_END_LENGTH_PARAM" );
    if(!initVariableName( hyp, item, "SetEndLength" ))
      item.myValue = h->GetLength( false );
    p.append( item );

    customWidgets()->append (0);

    item.myName = tr( "SMESH_REVERSED_EDGES" );
    p.append( item );

    customWidgets()->append ( makeReverseEdgesWdg( h->GetReversedEdges(), h->GetObjectEntry() ));
  }

  else if( hypType()=="GeometricProgression" )
  {
    StdMeshers::StdMeshers_Geometric1D_var h =
      StdMeshers::StdMeshers_Geometric1D::_narrow( hyp );

    item.myName = tr( "SMESH_START_LENGTH_PARAM" );
    if(!initVariableName( hyp, item, "SetStartLength" ))
      item.myValue = h->GetStartLength();
    p.append( item );

    customWidgets()->append (0);

    item.myName = tr( "SMESH_COMMON_RATIO" );
    if(!initVariableName( hyp, item, "SetCommonRatio" ))
      item.myValue = h->GetCommonRatio();
    p.append( item );

    customWidgets()->append (0);

    item.myName = tr( "SMESH_REVERSED_EDGES" );
    p.append( item );

    customWidgets()->append ( makeReverseEdgesWdg( h->GetReversedEdges(), h->GetObjectEntry() ));
  }

  else if( hypType()=="FixedPoints1D" )
  {
    StdMeshers::StdMeshers_FixedPoints1D_var h =
      StdMeshers::StdMeshers_FixedPoints1D::_narrow( hyp );

    item.myName = tr( "SMESH_FIXED_POINTS" );
    p.append( item );

    StdMeshersGUI_FixedPointsParamWdg* aFixedPointsWidget =
      new StdMeshersGUI_FixedPointsParamWdg();

    if ( !isCreation() ) {
      aFixedPointsWidget->SetListOfPoints( h->GetPoints() );
      aFixedPointsWidget->SetListOfSegments( h->GetNbSegments() );
    }
    customWidgets()->append( aFixedPointsWidget );

    item.myName = tr( "SMESH_REVERSED_EDGES" );
    p.append( item );

    customWidgets()->append ( makeReverseEdgesWdg( h->GetReversedEdges(), h->GetObjectEntry() ));
  }


  else if( hypType()=="MaxElementArea" )
  {
    StdMeshers::StdMeshers_MaxElementArea_var h =
      StdMeshers::StdMeshers_MaxElementArea::_narrow( hyp );

    item.myName = tr( "SMESH_MAX_ELEMENT_AREA_PARAM" );
    if(!initVariableName( hyp, item, "SetMaxElementArea" ))
      item.myValue = h->GetMaxElementArea();
    p.append( item );

  }
  else if( hypType()=="MaxElementVolume" )
  {
    StdMeshers::StdMeshers_MaxElementVolume_var h =
      StdMeshers::StdMeshers_MaxElementVolume::_narrow( hyp );

    item.myName = tr( "SMESH_MAX_ELEMENT_VOLUME_PARAM" );
    if(!initVariableName( hyp, item, "SetMaxElementVolume" ))
      item.myValue = h->GetMaxElementVolume();
    p.append( item );
  }
  else if( hypType()=="StartEndLength" )
  {
    StdMeshers::StdMeshers_StartEndLength_var h =
      StdMeshers::StdMeshers_StartEndLength::_narrow( hyp );

    item.myName = tr( "SMESH_START_LENGTH_PARAM" );

    if(!initVariableName( hyp, item, "SetStartLength" ))
      item.myValue = h->GetLength( true );
    p.append( item );
    customWidgets()->append(0);

    item.myName = tr( "SMESH_END_LENGTH_PARAM" );
    if(!initVariableName( hyp, item, "SetEndLength" ))
      item.myValue = h->GetLength( false );
    p.append( item );
    customWidgets()->append(0);

    item.myName = tr( "SMESH_REVERSED_EDGES" );
    p.append( item );

    customWidgets()->append ( makeReverseEdgesWdg( h->GetReversedEdges(), h->GetObjectEntry() ));
  }
  else if( hypType()=="Deflection1D" )
  {
    StdMeshers::StdMeshers_Deflection1D_var h =
      StdMeshers::StdMeshers_Deflection1D::_narrow( hyp );

    item.myName = tr( "SMESH_DEFLECTION1D_PARAM" );
    if(!initVariableName( hyp, item, "SetDeflection" ))
      item.myValue = h->GetDeflection();
    p.append( item );
  }
  else if( hypType()=="Adaptive1D" )
  {
    StdMeshers::StdMeshers_Adaptive1D_var h =
      StdMeshers::StdMeshers_Adaptive1D::_narrow( hyp );

    item.myName = tr( "SMESH_MIN_SIZE" );
    if(!initVariableName( hyp, item, "SetMinSize" ))
      item.myValue = h->GetMinSize();
    p.append( item );

    item.myName = tr( "SMESH_MAX_SIZE" );
    if(!initVariableName( hyp, item, "SetMaxSize" ))
      item.myValue = h->GetMaxSize();
    p.append( item );

    item.myName = tr( "SMESH_DEFLECTION1D_PARAM" );
    if(!initVariableName( hyp, item, "SetDeflection" ))
      item.myValue = h->GetDeflection();
    p.append( item );
  }
  else if( hypType()=="AutomaticLength" )
  {
    StdMeshers::StdMeshers_AutomaticLength_var h =
      StdMeshers::StdMeshers_AutomaticLength::_narrow( hyp );

    item.myName = tr( "SMESH_FINENESS_PARAM" );
    //item.myValue = h->GetFineness();
    p.append( item );
    SMESHGUI_SpinBox* _autoLengthSpinBox = new SMESHGUI_SpinBox(dlg());
    _autoLengthSpinBox->RangeStepAndValidator(0, 1, 0.01, "length_precision");
    _autoLengthSpinBox->SetValue(h->GetFineness());
    customWidgets()->append( _autoLengthSpinBox);
  }
  else if( hypType()=="NumberOfLayers" )
  {
    StdMeshers::StdMeshers_NumberOfLayers_var h =
      StdMeshers::StdMeshers_NumberOfLayers::_narrow( hyp );

    item.myName = tr( "SMESH_NUMBER_OF_LAYERS" );
    if(!initVariableName( hyp, item, "SetNumberOfLayers" ))     
      item.myValue = (int) h->GetNumberOfLayers();
    p.append( item );
  }
  else if( hypType()=="LayerDistribution" )
  {
    StdMeshers::StdMeshers_LayerDistribution_var h =
      StdMeshers::StdMeshers_LayerDistribution::_narrow( hyp );
    
    item.myName = tr( "SMESH_LAYERS_DISTRIBUTION" ); p.append( item );
    initVariableName( hyp, item, "SetLayerDistribution" );
    customWidgets()->append ( new StdMeshersGUI_LayerDistributionParamWdg
                              ( h, h->GetLayerDistribution(), hypName(), dlg() ));
  }
  else if( hypType()=="NumberOfLayers2D" )
  {
    StdMeshers::StdMeshers_NumberOfLayers2D_var h =
      StdMeshers::StdMeshers_NumberOfLayers2D::_narrow( hyp );
    
    item.myName = tr( "SMESH_NUMBER_OF_LAYERS" );
    if(!initVariableName( hyp, item, "SetNumberOfLayers" ))     
      item.myValue = (int) h->GetNumberOfLayers();
    p.append( item );
  }
  else if( hypType()=="LayerDistribution2D" )
  {
    StdMeshers::StdMeshers_LayerDistribution2D_var h =
      StdMeshers::StdMeshers_LayerDistribution2D::_narrow( hyp );

    item.myName = tr( "SMESH_LAYERS_DISTRIBUTION" ); p.append( item );
    initVariableName( hyp, item, "SetLayerDistribution" );
    customWidgets()->append ( new StdMeshersGUI_LayerDistributionParamWdg
                              ( h, h->GetLayerDistribution(), hypName(), dlg() ));
  }
  else if( hypType()=="ProjectionSource1D" )
  {
    StdMeshers::StdMeshers_ProjectionSource1D_var h =
      StdMeshers::StdMeshers_ProjectionSource1D::_narrow( hyp );

    item.myName = tr( "SMESH_SOURCE_EDGE" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( filterForShapeOfDim( 1 ),
                                               h->GetSourceEdge()));
    item.myName = tr( "SMESH_SOURCE_MESH" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( new SMESH_TypeFilter( SMESH::MESH ),
                                               h->GetSourceMesh()));
    item.myName = tr( "SMESH_SOURCE_VERTEX" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( filterForShapeOfDim( 0 ),
                                               h->GetSourceVertex()));
    item.myName = tr( "SMESH_TARGET_VERTEX" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( filterForShapeOfDim( 0 ),
                                               h->GetTargetVertex()));
  }
  else if( hypType()=="ProjectionSource2D" )
  {
    StdMeshers::StdMeshers_ProjectionSource2D_var h =
      StdMeshers::StdMeshers_ProjectionSource2D::_narrow( hyp );

    item.myName = tr( "SMESH_SOURCE_FACE" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( filterForShapeOfDim( 2 ),
                                               h->GetSourceFace()));
    item.myName = tr( "SMESH_SOURCE_MESH" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( new SMESH_TypeFilter( SMESH::MESH ),
                                               h->GetSourceMesh()));
    item.myName = tr( "SMESH_SOURCE_VERTEX1" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( filterForShapeOfDim( 0 ),
                                               h->GetSourceVertex( 1 )));
    item.myName = tr( "SMESH_TARGET_VERTEX1" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( filterForShapeOfDim( 0 ),
                                               h->GetTargetVertex( 1 )));
    item.myName = tr( "SMESH_SOURCE_VERTEX2" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( filterForShapeOfDim( 0 ),
                                               h->GetSourceVertex( 2 )));
    item.myName = tr( "SMESH_TARGET_VERTEX2" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( filterForShapeOfDim( 0 ),
                                               h->GetTargetVertex( 2 )));
  }
  else if( hypType()=="ProjectionSource3D" )
  {
    StdMeshers::StdMeshers_ProjectionSource3D_var h =
      StdMeshers::StdMeshers_ProjectionSource3D::_narrow( hyp );

    item.myName = tr( "SMESH_SOURCE_3DSHAPE" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( filterForShapeOfDim( 3, TopAbs_FACE, 6, true ),
                                               h->GetSource3DShape()));
    item.myName = tr( "SMESH_SOURCE_MESH" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( new SMESH_TypeFilter( SMESH::MESH ),
                                               h->GetSourceMesh()));
    item.myName = tr( "SMESH_SOURCE_VERTEX1" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( filterForShapeOfDim( 0 ),
                                               h->GetSourceVertex( 1 )));
    item.myName = tr( "SMESH_TARGET_VERTEX1" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( filterForShapeOfDim( 0 ),
                                               h->GetTargetVertex( 1 )));
    item.myName = tr( "SMESH_SOURCE_VERTEX2" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( filterForShapeOfDim( 0 ),
                                               h->GetSourceVertex( 2 )));
    item.myName = tr( "SMESH_TARGET_VERTEX2" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( filterForShapeOfDim( 0 ),
                                               h->GetTargetVertex( 2 )));
  }
  else if( hypType()=="ImportSource1D" )
  {
    StdMeshers::StdMeshers_ImportSource1D_var h =
      StdMeshers::StdMeshers_ImportSource1D::_narrow( hyp );

    SMESH::string_array_var groupEntries = h->GetSourceEdges();
    CORBA::Boolean toCopyMesh, toCopyGroups;
    h->GetCopySourceMesh(toCopyMesh, toCopyGroups);

    item.myName = tr( "SMESH_SOURCE_EDGES" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( new SMESH_TypeFilter( SMESH::GROUP_EDGE ), 
                                                groupEntries));

    item.myName = tr( "SMESH_COPY_MESH" ); p.append( item );
    QCheckBox* aQCheckBox = new QCheckBox(dlg());
    aQCheckBox->setChecked( toCopyMesh );
    connect( aQCheckBox, SIGNAL(  stateChanged(int) ), this, SLOT( onValueChanged() ));
    customWidgets()->append( aQCheckBox );

    item.myName = tr( "SMESH_TO_COPY_GROUPS" ); p.append( item );
    aQCheckBox = new QCheckBox(dlg());
    aQCheckBox->setChecked( toCopyGroups );
    aQCheckBox->setEnabled( toCopyMesh );
    customWidgets()->append( aQCheckBox );
  }
  else if( hypType()=="ImportSource2D" )
  {
    StdMeshers::StdMeshers_ImportSource2D_var h =
      StdMeshers::StdMeshers_ImportSource2D::_narrow( hyp );

    SMESH::string_array_var groupEntries = h->GetSourceFaces();
    CORBA::Boolean toCopyMesh, toCopyGroups;
    h->GetCopySourceMesh(toCopyMesh, toCopyGroups);

    item.myName = tr( "SMESH_SOURCE_FACES" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( new SMESH_TypeFilter( SMESH::GROUP_FACE ), 
                                                groupEntries));

    item.myName = tr( "SMESH_COPY_MESH" ); p.append( item );
    QCheckBox* aQCheckBox = new QCheckBox(dlg());
    aQCheckBox->setChecked( toCopyMesh );
    connect( aQCheckBox, SIGNAL(  stateChanged(int) ), this, SLOT( onValueChanged() ));
    customWidgets()->append( aQCheckBox );

    item.myName = tr( "SMESH_COPY_GROUPS" ); p.append( item );
    aQCheckBox = new QCheckBox(dlg());
    aQCheckBox->setChecked( toCopyGroups );
    aQCheckBox->setEnabled( toCopyMesh );
    customWidgets()->append( aQCheckBox );
  }
  else if( hypType()=="ViscousLayers" )
  {
    StdMeshers::StdMeshers_ViscousLayers_var h =
      StdMeshers::StdMeshers_ViscousLayers::_narrow( hyp );

    item.myName = tr( "SMESH_TOTAL_THICKNESS" );
    if(!initVariableName( hyp, item, "SetTotalThickness" ))
      item.myValue = h->GetTotalThickness();
    p.append( item );
    customWidgets()->append (0);

    item.myName = tr( "SMESH_NUMBER_OF_LAYERS" );
    if(!initVariableName( hyp, item, "SetNumberLayers" ))
      item.myValue = h->GetNumberLayers();
    p.append( item );
    customWidgets()->append (0);

    item.myName = tr( "SMESH_STRETCH_FACTOR" );
    if(!initVariableName( hyp, item, "SetStretchFactor" ))
      item.myValue = h->GetStretchFactor();
    p.append( item );
    customWidgets()->append (0);

    item.myName = tr( "EXTRUSION_METHOD" );
    p.append( item );
    StdMeshersGUI_RadioButtonsGrpWdg* methodWdg = new StdMeshersGUI_RadioButtonsGrpWdg("");
    methodWdg->setButtonLabels ( QStringList()
                                 << tr("EXTMETH_SURF_OFFSET_SMOOTH")
                                 << tr("EXTMETH_FACE_OFFSET")
                                 << tr("EXTMETH_NODE_OFFSET"),
                                 QStringList()
                                 << tr("ICON_EXTMETH_SURF_OFFSET_SMOOTH")
                                 << tr("ICON_EXTMETH_FACE_OFFSET")
                                 << tr("ICON_EXTMETH_NODE_OFFSET"));
    methodWdg->setChecked( (int) h->GetMethod() );
    customWidgets()->append( methodWdg );

    QString aMainEntry = SMESHGUI_GenericHypothesisCreator::getMainShapeEntry();
    QString aSubEntry  = SMESHGUI_GenericHypothesisCreator::getShapeEntry();
    if ( !aMainEntry.isEmpty() )
    {
      item.myName = tr( "TO_IGNORE_FACES_OR_NOT" );
      p.append( item );

      StdMeshersGUI_RadioButtonsGrpWdg* ignoreWdg = new StdMeshersGUI_RadioButtonsGrpWdg("");
      ignoreWdg->setButtonLabels ( QStringList()
                                   << tr("NOT_TO_IGNORE_FACES")
                                   << tr("TO_IGNORE_FACES") );
      ignoreWdg->setChecked( h->GetIsToIgnoreFaces() );
      connect(ignoreWdg->getButtonGroup(),SIGNAL(buttonClicked(int)),this,SLOT(onValueChanged()));
      customWidgets()->append( ignoreWdg );

      item.myName =
        tr( h->GetIsToIgnoreFaces() ? "SMESH_FACES_WO_LAYERS" : "SMESH_FACES_WITH_LAYERS" );
      p.append( item );

      StdMeshersGUI_SubShapeSelectorWdg* idsWg =
        new StdMeshersGUI_SubShapeSelectorWdg(0,TopAbs_FACE);

      idsWg->SetGeomShapeEntry( aSubEntry, aMainEntry );
      if ( idsWg->SetListOfIDs( h->GetFaces() ))
      {
        idsWg->ShowPreview( true );
      }
      else
      {
        SUIT_MessageBox::warning( dlg(),tr( "SMESH_WRN_WARNING" ),tr( "BAD_FACES_WARNING" ));
        idsWg->setEnabled( false );
      }
      customWidgets()->append ( idsWg );
    }
  }
  else if( hypType()=="ViscousLayers2D" )
  {
    StdMeshers::StdMeshers_ViscousLayers2D_var h =
      StdMeshers::StdMeshers_ViscousLayers2D::_narrow( hyp );

    item.myName = tr( "SMESH_TOTAL_THICKNESS" );
    if(!initVariableName( hyp, item, "SetTotalThickness" ))
      item.myValue = h->GetTotalThickness();
    p.append( item );
    customWidgets()->append (0);

    item.myName = tr( "SMESH_NUMBER_OF_LAYERS" );
    if(!initVariableName( hyp, item, "SetNumberLayers" ))
      item.myValue = h->GetNumberLayers();
    p.append( item );
    customWidgets()->append (0);

    item.myName = tr( "SMESH_STRETCH_FACTOR" );
    if(!initVariableName( hyp, item, "SetStretchFactor" ))
      item.myValue = h->GetStretchFactor();
    p.append( item );
    customWidgets()->append (0);

    QString aMainEntry = SMESHGUI_GenericHypothesisCreator::getMainShapeEntry();
    QString aSubEntry  = SMESHGUI_GenericHypothesisCreator::getShapeEntry();
    if ( !aMainEntry.isEmpty() )
    {
      item.myName = tr("TO_IGNORE_EDGES_OR_NOT");
      p.append( item );

      StdMeshersGUI_RadioButtonsGrpWdg* ignoreWdg = new StdMeshersGUI_RadioButtonsGrpWdg("");
      ignoreWdg->setButtonLabels ( QStringList()
                                   << tr("NOT_TO_IGNORE_EDGES")
                                   << tr("TO_IGNORE_EDGES") );
      ignoreWdg->setChecked( h->GetIsToIgnoreEdges() );
      connect(ignoreWdg->getButtonGroup(),SIGNAL(buttonClicked(int)),this,SLOT(onValueChanged()));
      customWidgets()->append( ignoreWdg );

      item.myName =
        tr( h->GetIsToIgnoreEdges() ? "SMESH_EDGES_WO_LAYERS" : "SMESH_EDGES_WITH_LAYERS" );
      p.append( item );

      StdMeshersGUI_SubShapeSelectorWdg* idsWg =
        new StdMeshersGUI_SubShapeSelectorWdg(0,TopAbs_EDGE);

      idsWg->SetGeomShapeEntry( aSubEntry, aMainEntry );
      if ( idsWg->SetListOfIDs( h->GetEdges() ))
      {
        idsWg->ShowPreview( true );
      }
      else
      {
        SUIT_MessageBox::warning( dlg(),tr( "SMESH_WRN_WARNING" ),tr( "BAD_EDGES_WARNING" ));
        idsWg->setEnabled( false );
      }
      customWidgets()->append ( idsWg );
    }
  }
  else
    res = false;
  return res;
}

//================================================================================
/*!
 * \brief tune "standard" control
 *  \param w - control widget
 *  \param int - parameter index
 */
//================================================================================

void StdMeshersGUI_StdHypothesisCreator::attuneStdWidget (QWidget* w, const int) const
{
  SMESHGUI_SpinBox* sb = w->inherits( "SMESHGUI_SpinBox" ) ? ( SMESHGUI_SpinBox* )w : 0;
  if ( sb )
  {
    if( hypType()=="LocalLength" )
    {
      if (sb->objectName() == tr("SMESH_LOCAL_LENGTH_PARAM"))
        sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, "length_precision" );
      else if (sb->objectName() == tr("SMESH_LOCAL_LENGTH_PRECISION"))
        sb->RangeStepAndValidator( 0.0, 1.0, 0.05, "len_tol_precision" );
    }
    else if( hypType()=="Arithmetic1D" )
    {
      sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, "parametric_precision" );
    }
    else if( hypType()=="GeometricProgression" )
    {
      if (sb->objectName() == tr("SMESH_START_LENGTH_PARAM"))
        sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, "length_precision" );
      else if (sb->objectName() == tr("SMESH_COMMON_RATIO"))
        sb->RangeStepAndValidator( -VALUE_MAX, VALUE_MAX, 0.5, "len_tol_precision" );
    }
    else if( hypType()=="MaxLength" )
    {
      sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, "length_precision" );
      sb->setEnabled( !widget< QCheckBox >( 1 )->isChecked() );
      sb->setMinimumWidth( 150 );
    }
    else if( hypType()=="MaxElementArea" )
    {
      sb->RangeStepAndValidator( VALUE_SMALL_2, VALUE_MAX_2, 1.0, "area_precision" );
    }
    else if( hypType()=="MaxElementVolume" )
    {
      sb->RangeStepAndValidator( VALUE_SMALL_3, VALUE_MAX_3, 1.0, "vol_precision" );
    }
    else if( hypType()=="StartEndLength" )
    {
      sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, "length_precision" );
    }
    else if( hypType()=="Deflection1D" )
    {
      sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, "parametric_precision" );
    }
    else if( hypType()=="Adaptive1D" )
    {
      sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, "length_precision" );
    }
    else if( hypType().startsWith( "ViscousLayers" ))
    {
      if (sb->objectName() == tr("SMESH_STRETCH_FACTOR"))
        sb->RangeStepAndValidator( 1.0, VALUE_MAX, 0.1, "parametric_precision" );
      else
        sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, "length_precision" );
    }
    else // default validator for possible ancestors
    {
      sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, "length_precision" );
    }
  }
  else if ( SalomeApp_IntSpinBox* sb = qobject_cast< SalomeApp_IntSpinBox* >( w ))
  {
    if ( hypType().startsWith( "NumberOfLayers" ) ||
         hypType().startsWith( "ViscousLayers" ))
    {
      sb->setMinimum( 1 );
    }
  }
}

//================================================================================
/*!
 * \brief Return dlg title
  * \retval QString - title string
 */
//================================================================================

QString StdMeshersGUI_StdHypothesisCreator::caption() const
{
  return tr( QString( "SMESH_%1_TITLE" ).arg( hypTypeName( hypType() ) ).toLatin1().data() );
}

//================================================================================
/*!
 * \brief return pixmap for dlg icon
  * \retval QPixmap - 
 */
//================================================================================

QPixmap StdMeshersGUI_StdHypothesisCreator::icon() const
{
  QString hypIconName = tr( QString( "ICON_DLG_%1" ).arg( hypTypeName( hypType() ) ).toLatin1().data() );
  return SMESHGUI::resourceMgr()->loadPixmap( "SMESH", hypIconName );
}

//================================================================================
/*!
 * \brief Return hypothesis type name to show in dlg
  * \retval QString - 
 */
//================================================================================

QString StdMeshersGUI_StdHypothesisCreator::type() const
{
  return tr( QString( "SMESH_%1_HYPOTHESIS" ).arg( hypTypeName( hypType() ) ).toLatin1().data() );
}

//================================================================================
/*!
 * \brief String to insert in "SMESH_%1_HYPOTHESIS" to get hypothesis type name
 * from message resouce file
  * \param t - hypothesis type
  * \retval QString - result string
 */
//================================================================================

QString StdMeshersGUI_StdHypothesisCreator::hypTypeName( const QString& t ) const
{
  static QMap<QString,QString>  types;
  if( types.isEmpty() )
  {
    types.insert( "LocalLength", "LOCAL_LENGTH" );
    types.insert( "NumberOfSegments", "NB_SEGMENTS" );
    types.insert( "MaxElementArea", "MAX_ELEMENT_AREA" );
    types.insert( "MaxElementVolume", "MAX_ELEMENT_VOLUME" );
    types.insert( "StartEndLength", "START_END_LENGTH" );
    types.insert( "Deflection1D", "DEFLECTION1D" );
    types.insert( "Adaptive1D", "ADAPTIVE1D" );
    types.insert( "Arithmetic1D", "ARITHMETIC_1D" );
    types.insert( "GeometricProgression", "GEOMETRIC_1D" );
    types.insert( "FixedPoints1D", "FIXED_POINTS_1D" );
    types.insert( "AutomaticLength", "AUTOMATIC_LENGTH" );
    types.insert( "ProjectionSource1D", "PROJECTION_SOURCE_1D" );
    types.insert( "ProjectionSource2D", "PROJECTION_SOURCE_2D" );
    types.insert( "ProjectionSource3D", "PROJECTION_SOURCE_3D" );
    types.insert( "ImportSource1D", "IMPORT_SOURCE_1D" );
    types.insert( "ImportSource2D", "IMPORT_SOURCE_2D" );
    types.insert( "NumberOfLayers", "NUMBER_OF_LAYERS" );
    types.insert( "LayerDistribution", "LAYER_DISTRIBUTION" );
    types.insert( "NumberOfLayers2D", "NUMBER_OF_LAYERS_2D" );
    types.insert( "LayerDistribution2D", "LAYER_DISTRIBUTION" );
    types.insert( "SegmentLengthAroundVertex", "SEGMENT_LENGTH_AROUND_VERTEX" );
    types.insert( "MaxLength", "MAX_LENGTH" );
    types.insert( "ViscousLayers", "VISCOUS_LAYERS" );
    types.insert( "ViscousLayers2D", "VISCOUS_LAYERS" );
    types.insert( "QuadrangleParams", "QUADRANGLE_PARAMS" );
    types.insert( "CartesianParameters3D", "CARTESIAN_PARAMS" );
  }

  QString res;
  if( types.contains( t ) )
    res = types[ t ];

  return res;
}


//=======================================================================
//function : getCustomWidget
//purpose  : is called from buildStdFrame()
//=======================================================================

QWidget* StdMeshersGUI_StdHypothesisCreator::getCustomWidget( const StdParam & param,
                                                              QWidget*         parent,
                                                              const int        index) const
{
  QWidget* w = 0;
  if ( index < customWidgets()->count() ) {
    w = customWidgets()->at( index );
    if ( w ) {
      w->setParent( parent );
      w->move( QPoint( 0, 0 ) );
    }
  }
  return w;
}

//================================================================================
/*!
 * \brief Set param value taken from a custom widget
  * \param param - SMESHGUI_GenericHypothesisCreator::StdParam structure
  * \param widget - widget presenting param
  * \retval bool - success flag
 * 
 * this method is called from getStdParamFromDlg()
 */
//================================================================================

bool StdMeshersGUI_StdHypothesisCreator::getParamFromCustomWidget( StdParam & param,
                                                                   QWidget*   widget) const
{
  if ( hypType()=="AutomaticLength" ) {
    SMESHGUI_SpinBox* w = dynamic_cast<SMESHGUI_SpinBox*>( widget );
    if ( w ) {
      param.myValue = w->GetValue();
      return true;
    }
  }
  if ( hypType() == "MaxLength" ) {
    param.myValue = "";
    return true;
  }
  if ( widget->inherits( "StdMeshersGUI_ObjectReferenceParamWdg" ))
  {
    // show only 1st reference value
    if ( true /*widget == getWidgetForParam( 0 )*/) {
      const StdMeshersGUI_ObjectReferenceParamWdg * w =
        static_cast<const StdMeshersGUI_ObjectReferenceParamWdg*>( widget );
      param.myValue = w->GetValue();
    }
    return true;
  }
  if ( widget->inherits( "StdMeshersGUI_LayerDistributionParamWdg" ))
  {
    const StdMeshersGUI_LayerDistributionParamWdg * w =
      static_cast<const StdMeshersGUI_LayerDistributionParamWdg*>( widget );
    param.myValue = w->GetValue();
    return true;
  }
  if ( widget->inherits( "StdMeshersGUI_SubShapeSelectorWdg" ))
  {
    const StdMeshersGUI_SubShapeSelectorWdg * w =
      static_cast<const StdMeshersGUI_SubShapeSelectorWdg*>( widget );
    param.myValue = w->GetValue();
    return true;
  }
  // if ( widget->inherits( "StdMeshersGUI_QuadrangleParamWdg" ))
  // {
  //   param.myValue = "QuadType";
  //   return true;
  // }
  if ( widget->inherits( "StdMeshersGUI_FixedPointsParamWdg" ))
  {
    const StdMeshersGUI_FixedPointsParamWdg * w =
      static_cast<const StdMeshersGUI_FixedPointsParamWdg*>( widget );
    param.myValue = w->GetValue();
    return true;
  }
  if ( widget->inherits( "QCheckBox" ))
  {
    //const QCheckBox * w = static_cast<const QCheckBox*>( widget );
    //param.myValue = w->isChecked();
    return true;
  }
  if ( widget->inherits( "StdMeshersGUI_RadioButtonsGrpWdg" ))
  {
    const StdMeshersGUI_RadioButtonsGrpWdg * w =
      static_cast<const StdMeshersGUI_RadioButtonsGrpWdg*>( widget );
    param.myValue = w->checkedId();
    return true;
  }
  return false;
}

//================================================================================
/*!
 * \brief called when operation cancelled
 */
//================================================================================

void StdMeshersGUI_StdHypothesisCreator::onReject()
{
  if ( hypType().startsWith("ProjectionSource" ) ||
       hypType().startsWith("ImportSource" ))
  {
    // Uninstall filters of StdMeshersGUI_ObjectReferenceParamWdg
    deactivateObjRefParamWdg( customWidgets() );
  }
}

//================================================================================
/*!
 * \brief Update widgets dependent on paramWidget
 */
//================================================================================

void StdMeshersGUI_StdHypothesisCreator::valueChanged( QWidget* paramWidget)
{
  if ( hypType() == "MaxLength" && paramWidget == getWidgetForParam(1) )
  {
    getWidgetForParam(0)->setEnabled( !widget< QCheckBox >( 1 )->isChecked() );
    if ( !getWidgetForParam(0)->isEnabled() ) {
      StdMeshers::StdMeshers_MaxLength_var h =
        StdMeshers::StdMeshers_MaxLength::_narrow( initParamsHypothesis() );
      widget< QtxDoubleSpinBox >( 0 )->setValue( h->GetPreestimatedLength() );
    }
  }
  else if ( hypType().startsWith("ImportSource") && paramWidget == getWidgetForParam(1) )
  {
    QCheckBox* toCopyMesh   = (QCheckBox*) paramWidget;
    QCheckBox* toCopyGroups = widget< QCheckBox >( 2 );
    if ( !toCopyMesh->isChecked() )
    {
      toCopyGroups->setChecked( false );
      toCopyGroups->setEnabled( false );
    }
    else
    {
      toCopyGroups->setEnabled( true );
    }
  }
  else if ( hypType().startsWith( "ViscousLayers" ) && paramWidget->inherits("QButtonGroup"))
  {
    int widgetNumber = hypType() == "ViscousLayers2D" ? 3 : 4;
    if ( QLabel* label = getLabel( widgetNumber + 1 ) )
    {
      bool toIgnore = widget< StdMeshersGUI_RadioButtonsGrpWdg >( widgetNumber )->checkedId();
      if ( hypType() == "ViscousLayers2D" )
        label->setText( tr( toIgnore ? "SMESH_EDGES_WO_LAYERS" : "SMESH_EDGES_WITH_LAYERS" ));
      else
        label->setText( tr( toIgnore ? "SMESH_FACES_WO_LAYERS" : "SMESH_FACES_WITH_LAYERS" ));
    }
  }
}

//================================================================================
/*!
 *
 */
//================================================================================

bool StdMeshersGUI_StdHypothesisCreator::initVariableName(SMESH::SMESH_Hypothesis_var theHyp, 
                                                          StdParam &                  theParams, 
                                                          const char*                 theMethod) const
{
  QString aVaribaleName = getVariableName( theMethod );
  theParams.isVariable = !aVaribaleName.isEmpty();
  if (theParams.isVariable)
    theParams.myValue = aVaribaleName;

  return theParams.isVariable;
}

//================================================================================
/*!
 * \brief Creates two widgets used to define reversed edges for some 1D hypotheses
 *  \param [in] edgeIDs - ids of reversed edges to set to the widgets
 *  \param [in] shapeEntry - entry of a sub-shape of a sub-mesh if any
 *  \return QWidget* - new StdMeshersGUI_SubShapeSelectorWdg; 
 *          new StdMeshersGUI_PropagationHelperWdg is stored in \a myHelperWidget field.
 */
//================================================================================

QWidget*
StdMeshersGUI_StdHypothesisCreator::makeReverseEdgesWdg( SMESH::long_array_var edgeIDs,
                                                         CORBA::String_var     shapeEntry) const
{
  QString aGeomEntry = SMESHGUI_GenericHypothesisCreator::getShapeEntry();
  QString aMainEntry = SMESHGUI_GenericHypothesisCreator::getMainShapeEntry();
  if ( aGeomEntry.isEmpty() && shapeEntry.in() )
    aGeomEntry = shapeEntry.in();

  StdMeshersGUI_SubShapeSelectorWdg* wdg = new StdMeshersGUI_SubShapeSelectorWdg();
  wdg->SetGeomShapeEntry( aGeomEntry, aMainEntry );
  wdg->SetListOfIDs( edgeIDs );
  wdg->ShowPreview( true );

  if ( !aGeomEntry.isEmpty() || !aMainEntry.isEmpty() )
    const_cast<StdMeshersGUI_StdHypothesisCreator*>( this )->
      myHelperWidget = new StdMeshersGUI_PropagationHelperWdg( wdg );

  return wdg;
}
