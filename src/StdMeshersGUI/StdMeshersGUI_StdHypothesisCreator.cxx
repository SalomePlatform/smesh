//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//  File   : StdMeshersGUI_StdHypothesisCreator.cxx
//  Author : Alexander SOLOVYOV, Open CASCADE S.A.S.
//  SMESH includes

#include "StdMeshersGUI_StdHypothesisCreator.h"

#include <SMESHGUI.h>
#include <SMESHGUI_SpinBox.h>
#include <SMESHGUI_HypothesesUtils.h>
#include <SMESHGUI_Utils.h>

#include <SMESH_TypeFilter.hxx>
#include <SMESH_NumberFilter.hxx>

#include "StdMeshersGUI_FixedPointsParamWdg.h"
#include "StdMeshersGUI_LayerDistributionParamWdg.h"
#include "StdMeshersGUI_ObjectReferenceParamWdg.h"
#include "StdMeshersGUI_QuadrangleParamWdg.h"
#include "StdMeshersGUI_SubShapeSelectorWdg.h"

#include <SALOMEDSClient_Study.hxx>

// SALOME GUI includes
#include <SUIT_ResourceMgr.h>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

// Qt includes
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>

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
: SMESHGUI_GenericHypothesisCreator( type )
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
   * \brief creates a filter for selection of shapes of given dimension
    * \param dim - dimension
    * \param subShapeType - required type of subshapes, number of which must be \a nbSubShapes
    * \param nbSubShapes - number of subshapes of given type
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
    for ( int i = 0; ok && i < nbAssocVert; i += 2)
    {
      QString srcV, tgtV;
      StdMeshersGUI_ObjectReferenceParamWdg* w1 =
        widget< StdMeshersGUI_ObjectReferenceParamWdg >( i+2 );
      StdMeshersGUI_ObjectReferenceParamWdg* w2 =
        widget< StdMeshersGUI_ObjectReferenceParamWdg >( i+3 );
      srcV = w1->GetValue();
      tgtV = w2->GetValue();
      ok = (( srcV.isEmpty()  && tgtV.isEmpty() ) ||
            ( !srcV.isEmpty() && !tgtV.isEmpty() && srcV != tgtV ));
      if ( !ok ) {
        w1->SetObject( CORBA::Object::_nil() );
        w2->SetObject( CORBA::Object::_nil() );
      }
    }

    // Uninstall filters of StdMeshersGUI_ObjectReferenceParamWdg
    if ( ok )
      deactivateObjRefParamWdg( customWidgets() );
  }
  else if ( hypType() == "LayerDistribution" || hypType() == "LayerDistribution2D" )
  {
    StdMeshersGUI_LayerDistributionParamWdg* w = 
      widget< StdMeshersGUI_LayerDistributionParamWdg >( 0 );
    ok = ( w && w->IsOk() );
  }
  else if ( hypType() == "QuadrangleParams" )
  {
    //StdMeshersGUI_SubShapeSelectorWdg* w =
    //  widget< StdMeshersGUI_SubShapeSelectorWdg >( 0 );
    //ok = ( w->GetListSize() > 0 );
    //StdMeshersGUI_QuadrangleParamWdg* w =
    //  widget< StdMeshersGUI_QuadrangleParamWdg >( 1 );
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
  QStringList aVariablesList = getVariablesFromDlg();

  if( res && !params.isEmpty() )
  {
    if( hypType()=="LocalLength" )
    {
      StdMeshers::StdMeshers_LocalLength_var h =
        StdMeshers::StdMeshers_LocalLength::_narrow( hypothesis() );

      h->SetLength( params[0].myValue.toDouble() );
      h->SetParameters(aVariablesList.join(":").toLatin1().constData());
      h->SetPrecision( params[1].myValue.toDouble() );
      h->SetParameters(aVariablesList.join(":").toLatin1().constData());
    }
    else if( hypType()=="MaxLength" )
    {
      StdMeshers::StdMeshers_MaxLength_var h =
        StdMeshers::StdMeshers_MaxLength::_narrow( hypothesis() );

      h->SetLength( params[0].myValue.toDouble() );
      h->SetParameters(aVariablesList.join(":").toLatin1().constData());
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

      h->SetLength( params[0].myValue.toDouble() );
      h->SetParameters(aVariablesList.join(":").toLatin1().constData());
    }
    else if( hypType()=="Arithmetic1D" )
    {
      StdMeshers::StdMeshers_Arithmetic1D_var h =
        StdMeshers::StdMeshers_Arithmetic1D::_narrow( hypothesis() );

      StdMeshersGUI_SubShapeSelectorWdg* w = 
        widget< StdMeshersGUI_SubShapeSelectorWdg >( 2 );

      h->SetStartLength( params[0].myValue.toDouble() );
      h->SetParameters(aVariablesList.join(":").toLatin1().constData());
      h->SetEndLength( params[1].myValue.toDouble() );
      h->SetParameters(aVariablesList.join(":").toLatin1().constData());
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
        h->SetParameters(aVariablesList.join(":").toLatin1().constData());
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
      h->SetParameters(aVariablesList.join(":").toLatin1().constData());
      h->SetMaxElementArea( params[0].myValue.toDouble() );
    }
    else if( hypType()=="MaxElementVolume" )
    {
      StdMeshers::StdMeshers_MaxElementVolume_var h =
        StdMeshers::StdMeshers_MaxElementVolume::_narrow( hypothesis() );

      h->SetMaxElementVolume( params[0].myValue.toDouble() );
      h->SetParameters(aVariablesList.join(":").toLatin1().constData());
    }
    else if( hypType()=="StartEndLength" )
    {
      StdMeshers::StdMeshers_StartEndLength_var h =
        StdMeshers::StdMeshers_StartEndLength::_narrow( hypothesis() );

      StdMeshersGUI_SubShapeSelectorWdg* w = 
        widget< StdMeshersGUI_SubShapeSelectorWdg >( 2 );

      h->SetStartLength( params[0].myValue.toDouble() );
      h->SetParameters(aVariablesList.join(":").toLatin1().constData());
      h->SetEndLength( params[1].myValue.toDouble() );
      h->SetParameters(aVariablesList.join(":").toLatin1().constData());
      if (w) {
        h->SetReversedEdges( w->GetListOfIDs() );
        h->SetObjectEntry( w->GetMainShapeEntry() );
      }
    }
    else if( hypType()=="Deflection1D" )
    {
      StdMeshers::StdMeshers_Deflection1D_var h =
        StdMeshers::StdMeshers_Deflection1D::_narrow( hypothesis() );
      h->SetParameters(aVariablesList.join(":").toLatin1().constData());
      h->SetDeflection( params[0].myValue.toDouble() );
    }
    else if( hypType()=="AutomaticLength" )
    {
      StdMeshers::StdMeshers_AutomaticLength_var h =
        StdMeshers::StdMeshers_AutomaticLength::_narrow( hypothesis() );

      h->SetFineness( params[0].myValue.toDouble() );
    }
    else if( hypType()=="NumberOfLayers" )
    {
      StdMeshers::StdMeshers_NumberOfLayers_var h =
        StdMeshers::StdMeshers_NumberOfLayers::_narrow( hypothesis() );

      h->SetNumberOfLayers( params[0].myValue.toInt() );
      h->SetParameters(aVariablesList.join(":").toLatin1().constData());
    }
    else if( hypType()=="LayerDistribution" )
    {
      StdMeshers::StdMeshers_LayerDistribution_var h =
        StdMeshers::StdMeshers_LayerDistribution::_narrow( hypothesis() );
      StdMeshersGUI_LayerDistributionParamWdg* w = 
        widget< StdMeshersGUI_LayerDistributionParamWdg >( 0 );
      
      h->SetLayerDistribution( w->GetHypothesis() );
      h->SetParameters(w->GetHypothesis()->GetParameters());
      w->GetHypothesis()->ClearParameters();
    }
    else if( hypType()=="NumberOfLayers2D" )
    {
      StdMeshers::StdMeshers_NumberOfLayers2D_var h =
        StdMeshers::StdMeshers_NumberOfLayers2D::_narrow( hypothesis() );

      h->SetNumberOfLayers( params[0].myValue.toInt() );
      h->SetParameters(aVariablesList.join(":").toLatin1().constData());
    }
    else if( hypType()=="LayerDistribution2D" )
    {
      StdMeshers::StdMeshers_LayerDistribution2D_var h =
        StdMeshers::StdMeshers_LayerDistribution2D::_narrow( hypothesis() );
      StdMeshersGUI_LayerDistributionParamWdg* w = 
        widget< StdMeshersGUI_LayerDistributionParamWdg >( 0 );
      
      h->SetLayerDistribution( w->GetHypothesis() );
      h->SetParameters(w->GetHypothesis()->GetParameters());
      w->GetHypothesis()->ClearParameters();
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
    else if( hypType()=="QuadrangleParams" )
    {
      StdMeshers::StdMeshers_QuadrangleParams_var h =
        StdMeshers::StdMeshers_QuadrangleParams::_narrow( hypothesis() );
      StdMeshersGUI_SubShapeSelectorWdg* w1 =
        widget< StdMeshersGUI_SubShapeSelectorWdg >( 0 );
      StdMeshersGUI_QuadrangleParamWdg* w2 =
        widget< StdMeshersGUI_QuadrangleParamWdg >( 1 );
      if (w1 && w2) {
        if (w1->GetListSize() > 0) {
          h->SetTriaVertex(w1->GetListOfIDs()[0]); // getlist must be called once
          const char * entry = w1->GetMainShapeEntry();
          h->SetObjectEntry(entry);
        }
        h->SetQuadType(StdMeshers::QuadType(w2->GetType()));
      }
    }
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
  SMESH::ListOfParameters_var aParameters = hyp->GetLastParameters();

  if( hypType()=="LocalLength" )
  {
    StdMeshers::StdMeshers_LocalLength_var h =
      StdMeshers::StdMeshers_LocalLength::_narrow( hyp );
    
    item.myName = tr("SMESH_LOCAL_LENGTH_PARAM");
    if(!initVariableName(aParameters,item,0))
      item.myValue = h->GetLength();
    p.append( item );     
    
    item.myName = tr("SMESH_LOCAL_LENGTH_PRECISION");
    if(!initVariableName(aParameters,item,1))
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
    if(!initVariableName(aParameters,item,0))
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
    if(!initVariableName(aParameters,item,0))
      item.myValue = h->GetLength();
    
    p.append( item );
  }
  else if( hypType()=="Arithmetic1D" )
  {
    StdMeshers::StdMeshers_Arithmetic1D_var h =
      StdMeshers::StdMeshers_Arithmetic1D::_narrow( hyp );

    item.myName = tr( "SMESH_START_LENGTH_PARAM" );
    if(!initVariableName(aParameters,item,0))
      item.myValue = h->GetLength( true );
    p.append( item );

    customWidgets()->append (0);

    item.myName = tr( "SMESH_END_LENGTH_PARAM" );
    if(!initVariableName(aParameters,item,1))
      item.myValue = h->GetLength( false );
    p.append( item );

    customWidgets()->append (0);

    item.myName = tr( "SMESH_REVERSED_EDGES" );
    p.append( item );

    StdMeshersGUI_SubShapeSelectorWdg* aDirectionWidget =
      new StdMeshersGUI_SubShapeSelectorWdg();
    QString aGeomEntry = SMESHGUI_GenericHypothesisCreator::getShapeEntry();
    QString aMainEntry = SMESHGUI_GenericHypothesisCreator::getMainShapeEntry();
    if ( aGeomEntry == "" )
      aGeomEntry = h->GetObjectEntry();

    aDirectionWidget->SetGeomShapeEntry( aGeomEntry );
    aDirectionWidget->SetMainShapeEntry( aMainEntry );
    aDirectionWidget->SetListOfIDs( h->GetReversedEdges() );
    aDirectionWidget->showPreview( true );
    customWidgets()->append ( aDirectionWidget );
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

    StdMeshersGUI_SubShapeSelectorWdg* aDirectionWidget =
      new StdMeshersGUI_SubShapeSelectorWdg();
    QString anEntry = SMESHGUI_GenericHypothesisCreator::getShapeEntry();
    QString aMainEntry = SMESHGUI_GenericHypothesisCreator::getMainShapeEntry();
    if ( anEntry == "" )
      anEntry = h->GetObjectEntry();
    aDirectionWidget->SetGeomShapeEntry( anEntry );
    aDirectionWidget->SetMainShapeEntry( aMainEntry );
    aDirectionWidget->SetListOfIDs( h->GetReversedEdges() );
    aDirectionWidget->showPreview( true );
    customWidgets()->append ( aDirectionWidget );
  }


  else if( hypType()=="MaxElementArea" )
  {
    StdMeshers::StdMeshers_MaxElementArea_var h =
      StdMeshers::StdMeshers_MaxElementArea::_narrow( hyp );

    item.myName = tr( "SMESH_MAX_ELEMENT_AREA_PARAM" );
    if(!initVariableName(aParameters,item,0))
      item.myValue = h->GetMaxElementArea();
    p.append( item );
    
  }
  else if( hypType()=="MaxElementVolume" )
  {
    StdMeshers::StdMeshers_MaxElementVolume_var h =
      StdMeshers::StdMeshers_MaxElementVolume::_narrow( hyp );

    item.myName = tr( "SMESH_MAX_ELEMENT_VOLUME_PARAM" );
    if(!initVariableName(aParameters,item,0))
      item.myValue = h->GetMaxElementVolume();
    p.append( item );
  }
  else if( hypType()=="StartEndLength" )
  {
    StdMeshers::StdMeshers_StartEndLength_var h =
      StdMeshers::StdMeshers_StartEndLength::_narrow( hyp );

    item.myName = tr( "SMESH_START_LENGTH_PARAM" );

    if(!initVariableName(aParameters,item,0)) 
      item.myValue = h->GetLength( true );
    p.append( item );
    customWidgets()->append(0);

    item.myName = tr( "SMESH_END_LENGTH_PARAM" );
    if(!initVariableName(aParameters,item,1)) 
      item.myValue = h->GetLength( false );
    p.append( item );
    customWidgets()->append(0);

    item.myName = tr( "SMESH_REVERSED_EDGES" );
    p.append( item );

    StdMeshersGUI_SubShapeSelectorWdg* aDirectionWidget =
      new StdMeshersGUI_SubShapeSelectorWdg();
    QString anEntry = SMESHGUI_GenericHypothesisCreator::getShapeEntry();
    QString aMainEntry = SMESHGUI_GenericHypothesisCreator::getMainShapeEntry();
    if ( anEntry == "" )
      anEntry = h->GetObjectEntry();
    aDirectionWidget->SetGeomShapeEntry( anEntry );
    aDirectionWidget->SetMainShapeEntry( aMainEntry );
    aDirectionWidget->SetListOfIDs( h->GetReversedEdges() );
    aDirectionWidget->showPreview( true );
    customWidgets()->append ( aDirectionWidget );
  }
  else if( hypType()=="Deflection1D" )
  {
    StdMeshers::StdMeshers_Deflection1D_var h =
      StdMeshers::StdMeshers_Deflection1D::_narrow( hyp );
    
    item.myName = tr( "SMESH_DEFLECTION1D_PARAM" );
    if(!initVariableName(aParameters,item,0)) 
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
    if(!initVariableName(aParameters,item,0))     
      item.myValue = (int) h->GetNumberOfLayers();
    p.append( item );
  }
  else if( hypType()=="LayerDistribution" ) {
    StdMeshers::StdMeshers_LayerDistribution_var h =
      StdMeshers::StdMeshers_LayerDistribution::_narrow( hyp );
    
    item.myName = tr( "SMESH_LAYERS_DISTRIBUTION" ); p.append( item );
    
    //Set into not published hypo last variables
    QStringList aLastVarsList;
    for(int i = 0;i<aParameters->length();i++) 
      aLastVarsList.append(QString(aParameters[i].in()));

    if(!aLastVarsList.isEmpty())
      h->GetLayerDistribution()->SetLastParameters(aLastVarsList.join(":").toLatin1().constData());
    
    customWidgets()->append
      ( new StdMeshersGUI_LayerDistributionParamWdg( h->GetLayerDistribution(), hypName(), dlg()));
  }
  else if( hypType()=="NumberOfLayers2D" ) {
    StdMeshers::StdMeshers_NumberOfLayers2D_var h =
      StdMeshers::StdMeshers_NumberOfLayers2D::_narrow( hyp );
    
    item.myName = tr( "SMESH_NUMBER_OF_LAYERS" );
    if(!initVariableName(aParameters,item,0))     
      item.myValue = (int) h->GetNumberOfLayers();
    p.append( item );
  }
  else if( hypType()=="LayerDistribution2D" ) {
    StdMeshers::StdMeshers_LayerDistribution2D_var h =
      StdMeshers::StdMeshers_LayerDistribution2D::_narrow( hyp );

    item.myName = tr( "SMESH_LAYERS_DISTRIBUTION" ); p.append( item );
    
    //Set into not published hypo last variables
    QStringList aLastVarsList;
    for(int i = 0;i<aParameters->length();i++) 
      aLastVarsList.append(QString(aParameters[i].in()));

    if(!aLastVarsList.isEmpty())
      h->GetLayerDistribution()->SetLastParameters(aLastVarsList.join(":").toLatin1().constData());
    
    customWidgets()->append
      ( new StdMeshersGUI_LayerDistributionParamWdg( h->GetLayerDistribution(), hypName(), dlg()));
  }
  else if( hypType()=="ProjectionSource1D" )
  {
    StdMeshers::StdMeshers_ProjectionSource1D_var h =
      StdMeshers::StdMeshers_ProjectionSource1D::_narrow( hyp );

    item.myName = tr( "SMESH_SOURCE_EDGE" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( filterForShapeOfDim( 1 ),
                                               h->GetSourceEdge()));
    item.myName = tr( "SMESH_SOURCE_MESH" ); p.append( item );
    customWidgets()->append( newObjRefParamWdg( new SMESH_TypeFilter( MESH ),
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
    customWidgets()->append( newObjRefParamWdg( new SMESH_TypeFilter( MESH ),
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
    customWidgets()->append( newObjRefParamWdg( new SMESH_TypeFilter( MESH ),
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
  else if (hypType() == "QuadrangleParams")
  {
    StdMeshers::StdMeshers_QuadrangleParams_var h =
      StdMeshers::StdMeshers_QuadrangleParams::_narrow(hyp);

    item.myName = tr("SMESH_BASE_VERTEX");
    p.append(item);

    StdMeshersGUI_SubShapeSelectorWdg* aDirectionWidget =
      new StdMeshersGUI_SubShapeSelectorWdg();
    aDirectionWidget->SetMaxSize(1);
    aDirectionWidget->SetSubShType(TopAbs_VERTEX);
    QString anEntry = SMESHGUI_GenericHypothesisCreator::getShapeEntry();
    QString aMainEntry = SMESHGUI_GenericHypothesisCreator::getMainShapeEntry();
    if (anEntry == "")
      anEntry = h->GetObjectEntry();
    aDirectionWidget->SetGeomShapeEntry(anEntry);
    aDirectionWidget->SetMainShapeEntry(aMainEntry);
    if (!isCreation()) {
      SMESH::long_array_var aVec = new SMESH::long_array;
      int vertID = h->GetTriaVertex();
      if (vertID > 0) {
        aVec->length(1);
        aVec[0] = vertID;
        aDirectionWidget->SetListOfIDs(aVec);
      }
    }
    aDirectionWidget->showPreview(true);

    item.myName = tr("SMESH_QUAD_TYPE");
    p.append(item);

    StdMeshersGUI_QuadrangleParamWdg* aTypeWidget =
      new StdMeshersGUI_QuadrangleParamWdg();
    if (!isCreation()) {
      aTypeWidget->SetType(int(h->GetQuadType()));
    }

    customWidgets()->append(aDirectionWidget);
    customWidgets()->append(aTypeWidget);
  }
  else
    res = false;
  return res;
}

//================================================================================
/*!
 * \brief tune "standard" control
  * \param w - control widget
  * \param int - parameter index
 */
//================================================================================

void StdMeshersGUI_StdHypothesisCreator::attuneStdWidget (QWidget* w, const int) const
{
  SMESHGUI_SpinBox* sb = w->inherits( "SMESHGUI_SpinBox" ) ? ( SMESHGUI_SpinBox* )w : 0;
  if( hypType()=="LocalLength" &&  sb )
  {
    if (sb->objectName() == tr("SMESH_LOCAL_LENGTH_PARAM"))
      sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, "length_precision" );
    else if (sb->objectName() == tr("SMESH_LOCAL_LENGTH_PRECISION"))
      sb->RangeStepAndValidator( 0.0, 1.0, 0.05, "len_tol_precision" );
  }
  else if( hypType()=="Arithmetic1D" && sb )
  {
    sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, "parametric_precision" );
  }
  else if( hypType()=="MaxLength" && sb )
  {
    sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, "length_precision" );
    sb->setEnabled( !widget< QCheckBox >( 1 )->isChecked() );
  }
  else if( hypType()=="MaxElementArea" && sb )
  {
    sb->RangeStepAndValidator( VALUE_SMALL_2, VALUE_MAX_2, 1.0, "area_precision" );
  }
  else if( hypType()=="MaxElementVolume" && sb )
  {
    sb->RangeStepAndValidator( VALUE_SMALL_3, VALUE_MAX_3, 1.0, "vol_precision" );
  }
  else if( hypType()=="StartEndLength" && sb )
  {
    sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, "length_precision" );
  }
  else if( hypType()=="Deflection1D" && sb )
  {
    sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, "parametric_precision" );
  }
  else if ( sb ) // default validator for possible ancestors
  {
    sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, "length_precision" );
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
    types.insert( "Arithmetic1D", "ARITHMETIC_1D" );
    types.insert( "FixedPoints1D", "FIXED_POINTS_1D" );
    types.insert( "AutomaticLength", "AUTOMATIC_LENGTH" );
    types.insert( "ProjectionSource1D", "PROJECTION_SOURCE_1D" );
    types.insert( "ProjectionSource2D", "PROJECTION_SOURCE_2D" );
    types.insert( "ProjectionSource3D", "PROJECTION_SOURCE_3D" );
    types.insert( "NumberOfLayers", "NUMBER_OF_LAYERS" );
    types.insert( "LayerDistribution", "LAYER_DISTRIBUTION" );
    types.insert( "NumberOfLayers2D", "NUMBER_OF_LAYERS_2D" );
    types.insert( "LayerDistribution2D", "LAYER_DISTRIBUTION" );
    types.insert( "SegmentLengthAroundVertex", "SEGMENT_LENGTH_AROUND_VERTEX" );
    types.insert( "MaxLength", "MAX_LENGTH" );
    types.insert( "QuadrangleParams", "QUADRANGLE_PARAMS" );
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
  if ( widget->inherits( "StdMeshersGUI_QuadrangleParamWdg" ))
  {
    //const StdMeshersGUI_QuadrangleParamWdg * w =
    //  static_cast<const StdMeshersGUI_QuadrangleParamWdg*>( widget );
    param.myValue = "QuadType";
    return true;
  }
  if ( widget->inherits( "StdMeshersGUI_FixedPointsParamWdg" ))
  {
    const StdMeshersGUI_FixedPointsParamWdg * w =
      static_cast<const StdMeshersGUI_FixedPointsParamWdg*>( widget );
    param.myValue = w->GetValue();
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
  if ( hypType().startsWith("ProjectionSource" ))
  {
    // Uninstall filters of StdMeshersGUI_ObjectReferenceParamWdg
    deactivateObjRefParamWdg( customWidgets() );
  }
}

//================================================================================
/*!
 * \brief 
 */
//================================================================================

void StdMeshersGUI_StdHypothesisCreator::valueChanged( QWidget* paramWidget)
{
  if ( hypType() == "MaxLength" && paramWidget == getWidgetForParam(1) ) {
    getWidgetForParam(0)->setEnabled( !widget< QCheckBox >( 1 )->isChecked() );
    if ( !getWidgetForParam(0)->isEnabled() ) {
      StdMeshers::StdMeshers_MaxLength_var h =
        StdMeshers::StdMeshers_MaxLength::_narrow( initParamsHypothesis() );
      widget< QtxDoubleSpinBox >( 0 )->setValue( h->GetPreestimatedLength() );
    }
  }
}

//================================================================================
/*!
 *
 */
//================================================================================

bool StdMeshersGUI_StdHypothesisCreator::initVariableName(SMESH::ListOfParameters_var theParameters, 
                                                          StdParam &theParams, 
                                                          int order) const
{
  QString aVaribaleName = (theParameters->length() > order) ? QString(theParameters[order].in()) : QString("");
  theParams.isVariable = !aVaribaleName.isEmpty();
  if(theParams.isVariable) 
    theParams.myValue = aVaribaleName;

  return theParams.isVariable;
}
