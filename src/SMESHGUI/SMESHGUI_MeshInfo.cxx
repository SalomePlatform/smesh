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

#include "SMESHGUI_MeshInfo.h"

#include "SMDS_Mesh.hxx"
#include "SMESH_Actor.h"
#include "SMESHGUI.h"
#include "SMESHGUI_FilterUtils.h"
#include "SMESHGUI_IdPreview.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include <SUIT_FileDlg.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SVTK_ViewWindow.h>
#include <LightApp_SelectionMgr.h>

#include <SALOMEDSClient_Study.hxx>

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemDelegate>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTextStream>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

////////////////////////////////////////////////////////////////////////////////
/// \class Field
/// \brief Field widget.
/// \internal
////////////////////////////////////////////////////////////////////////////////

class Field : public QLabel
{
public:
  Field( QWidget*, const QString& = QString() );
  bool event( QEvent* );
};

/*!
  \brief Constructor.
  \param parent Parent widget.
  \param name Field name. Defaults to null string.
*/
Field::Field( QWidget* parent, const QString& name ): QLabel( parent )
{
  setFrameStyle( QLabel::StyledPanel | QLabel::Sunken );
  setAlignment( Qt::AlignCenter );
  setAutoFillBackground( true );
  QPalette pal = palette();
  QColor base = QApplication::palette().color( QPalette::Active, QPalette::Base );
  pal.setColor( QPalette::Window, base );
  setPalette( pal );
  setMinimumWidth( 60 );
  if ( !name.isEmpty() )
    setObjectName( name );
}

/*!
  \brief Event handler. Redefined from QLabel.
*/
bool Field::event( QEvent* e )
{
  if ( e->type() == QEvent::DynamicPropertyChange )
  {
    QDynamicPropertyChangeEvent* ce = (QDynamicPropertyChangeEvent*)e;
    if ( ce->propertyName() == "value" && property( "value" ).isValid() )
    {
      setText( QString::number( property( "value" ).toInt() ) );
      setProperty( "value", QVariant() );
      return true;
    }
  }
  return QLabel::event( e );
}

////////////////////////////////////////////////////////////////////////////////
/// \class TreeItemCreator
/// \brief Generic tree item creator.
/// \internal
////////////////////////////////////////////////////////////////////////////////

class TreeItemCreator
{
public:
  TreeItemCreator() {}
  virtual ~TreeItemCreator() {}
  virtual QTreeWidgetItem* createItem( QTreeWidgetItem*, int ) = 0;
};

////////////////////////////////////////////////////////////////////////////////
// General purpose services.
////////////////////////////////////////////////////////////////////////////////

namespace
{
  const int SPACING = 6;
  const int MARGIN = 9;

  enum
  {
    Bold = 0x01,
    Italic = 0x02,
    AllColumns = 0x04,
    Expanded = 0x08,
    Editable = 0x10
  };

  enum
  {
    GroupsId = 100,
    SubMeshesId
  };

  enum
  {
    TypeRole = Qt::UserRole + 10,
    IdRole,
  };

  enum
  {
    NodeConnectivity = 100,
    ElemConnectivity,
  };

  /*!
    \brief Get item's depth in the tree.
    \param item Tree widget item.
    \return Item's depth in the tree widget (top-level item has zero depth).
    \internal
  */
  int itemDepth( QTreeWidgetItem* item )
  {
    QList<QTreeWidgetItem*> parents;
    parents << item;
    while ( parents.last()->parent() )
      parents << parents.last()->parent();
    return parents.size()-1;
  }

  /*!
    \brief Get chunk size.
    \return Chunk size.
    \internal
  */
  int blockSize()
  {
    return 10;
  }

  /*!
    \brief Get spacer.
    \return Spacer string.
    \internal
  */
  QString spacing()
  {
    static int size = 1;
    static QChar spacer = ' ';
    return QString( size, spacer );
  }

  /*!
    \brief Get indent.
    \param length Indent size. Defaults to 1.
    \return Indentation string.
    \internal
  */
  QString indent( int length = 1 )
  {
    static int size = 4;
    static QChar spacer = ' ';
    return QString( size * length, spacer );
  }

  /*!
    \brief Get indent.
    \param spacer Spacer.
    \param length Indent size. Defaults to 1.
    \return Indentation string.
    \internal
  */
  QString indent( const QString& spacer, uint length = 1 )
  {
    QString result;
    while( length-- > 0 )
      result += spacer;
    return result;
  }

  /*!
    \brief Get group separator.
    \param length Length of ruler (number of symbols). Defaults to 80.
    \return Ruler string.
    \internal
  */
  QString ruler( int length = 80 )
  {
    static QChar ruler = '-';
    return QString( length, ruler );
  }

  /*!
    \brief Get text value from label.
    \param w Widget (QLabel).
    \return Value string.
    \internal
  */
  QString widgetValue( QWidget* w )
  {
    QString v;
    if ( qobject_cast<QLabel*>( w ) )
      v = qobject_cast<QLabel*>( w )->text();
    return v;
  }

  /*!
    \brief Get font for given options.
    \param font Initial font.
    \param options Font attributes.
    \return Font.
  */
  QFont fontFromOptions( const QFont& font, int options )
  {
    QFont f = font;
    f.setBold( options & Bold );
    f.setItalic( options & Italic );
    return f;
  }

  /*!
    \brief Set font attributes to given widget.
    \param w Widget.
    \param options Font attributes.
  */
  void setFontAttributes( QWidget* w, int options )
  {
    if ( w )
      w->setFont( fontFromOptions( w->font(), options ) );
  }

  /*!
    \brief Set attributes to given tree item.
    \param item Tree widget item.
    \param options Item attributes.
  */
  void setTreeItemAttributes( QTreeWidgetItem* item, int options )
  {
    if ( item && item->treeWidget() )
    {
      for ( int i = 0; i < item->treeWidget()->columnCount(); i++ )
      {
        if ( i == 0 || options & AllColumns )
          item->setFont( i, fontFromOptions( item->font( 0 ), options) );
      }
    }
    if ( options & Expanded )
      item->setExpanded( true );
    if ( options & Editable )
      item->setFlags( item->flags() | Qt::ItemIsEditable );
  }

  /*!
    \brief Create label.
    \param parent Parent widget.
    \param options Label options. Defaults to 0 (none).
    \return New label.
  */
  QLabel* createLabel( QWidget* parent, int options = 0 )
  {
    QLabel* lab = new QLabel( parent );
    setFontAttributes( lab, options );
    return lab;
  }

  /*!
    \brief Create label.
    \param text Label text.
    \param parent Parent widget.
    \param options Label options. Defaults to 0 (none).
    \return New label.
  */
  QLabel* createLabel( const QString& text, QWidget* parent, int options = 0 )
  {
    QLabel* lab = createLabel( parent, options );
    lab->setText( text );
    return lab;
  }

  /*!
    \brief Create information field.
    \param parent Parent widget.
    \param name Field's object. Default to null string.
    \return New field.
  */
  QLabel* createField( QWidget* parent, const QString& name = QString() )
  {
    return new Field( parent, name );
  }

  /*!
    \brief Create information field.
    \param parent Parent widget.
    \param options Label options.
    \param name Field's object. Default to null string.
    \return New field.
  */
  QLabel* createField( QWidget* parent, int options, const QString& name = QString() )
  {
    QLabel* field = createField( parent, name );
    setFontAttributes( field, options );
    return field;
  }

  /*!
    \brief Create ruler.
    \param parent Parent widget.
    \param orientation Ruler orientation. Defaults to horizontal.
    \return New ruler.
  */
  QWidget* createSeparator( QWidget* parent, Qt::Orientation orientation = Qt::Horizontal )
  {
    QFrame* line = new QFrame( parent );
    line->setFrameShape( orientation == Qt::Horizontal ? QFrame::HLine : QFrame::HLine );
    line->setFrameShadow( QFrame::Sunken );
    return line;
  }

  /*!
    \brief Decorate text as bold.
    \param text Initial text.
    \return Decorated text.
  */
  QString bold( const QString& text )
  {
    return QString("<b>%1</b>").arg( text );
  }

  /*!
    \brief Format connectivity data to string representation.
    \param connectivity Connectivity map.
    \param type Element type or face index if negative
    \return Stringified representation of the connectivity.
  */
  QString formatConnectivity( SMESH::Connectivity connectivity, int type )
  {
    QStringList str;
    QString     result;
    bool isNodal = ( type == SMDSAbs_Node || type < 0 );
    type = qAbs( type );
    if ( connectivity.contains( type ))
    {
      QList<int> elements = connectivity[ type ];
      if ( !isNodal ) // order of nodes is important
        qSort( elements );
      foreach( int id, elements )
        str << QString::number( id );

      // wrap IDs into an html link, to be treated by QTextBrowser used by SMESHGUI_SimpleElemInfo
      QString s = str.join( " " );
      result = ( "<a href = \"" +                                 // URL:
                 QString( isNodal ? "nodes" : "elems" ) + "://" + // protocol
                 QString( "host.com/") + // QUrl return nothing if host missing
                 s + "\">" +                                      // path
                 s + "</a>" );                                    // anchor text
    }
    return result;
  }
} // end of anonymous namespace

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_Info
/// \brief Base widget for all information panes.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
  \param parent Parent widget. Defaults to 0.
*/
SMESHGUI_Info::SMESHGUI_Info( QWidget* parent ): QWidget( parent )
{
}

////////////////////////////////////////////////////////////////////////////////
/// \class ExtraWidget
/// \brief Auxiliary widget to browse between chunks of information.
/// \internal
////////////////////////////////////////////////////////////////////////////////

class ExtraWidget : public QWidget
{
public:
  ExtraWidget( QWidget*, bool = false );
  void updateControls( int, int );

public:
  QLabel* current;
  QPushButton* prev;
  QPushButton* next;
  bool brief;
};

/*
  \brief Constructor.
  \param parent Parent widget.
  \param briefSummary Show summary in brief format. Defaults to \c false.
*/
ExtraWidget::ExtraWidget( QWidget* parent, bool briefSummary ): QWidget( parent ), brief( briefSummary )
{
  current = new QLabel( this );
  current->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  prev = new QPushButton( "<<", this );
  next = new QPushButton( ">>", this );
  QHBoxLayout* hbl = new QHBoxLayout( this );
  hbl->setMargin( 0 );
  hbl->setSpacing( SPACING );
  hbl->addStretch();
  hbl->addWidget( current );
  hbl->addWidget( prev );
  hbl->addWidget( next );
}

/*
  \brief Update controls.
  \param total Total number of items.
  \param index Current index.
*/
void ExtraWidget::updateControls( int total, int index )
{
  setVisible( total > blockSize() );
  QString format = brief ? QString( "%1-%2 / %3" ) : SMESHGUI_MeshInfoDlg::tr( "X_FROM_Y_ITEMS_SHOWN" );
  current->setText( format.arg( index*blockSize()+1 ).arg( qMin( index*blockSize()+blockSize(), total ) ).arg( total ) );
  prev->setEnabled( index > 0 );
  next->setEnabled( (index+1)*blockSize() < total );
}

////////////////////////////////////////////////////////////////////////////////
/// \class DumpFileDlg
/// \brief Standard Save File dialog box, customized for dump info operation.
/// \internal
////////////////////////////////////////////////////////////////////////////////

class DumpFileDlg : public SUIT_FileDlg
{
  QMap<int, QCheckBox*> myControls;
public:
  DumpFileDlg( QWidget*, bool = true );
  bool isChecked( int ) const;
  void setChecked( int, bool );
};

/*!
  \brief Constructor.
  \param parent Parent widget.
  \param showControls Show additional controls. Defaults to \c true.
  \internal
*/
DumpFileDlg::DumpFileDlg( QWidget* parent, bool showControls ): SUIT_FileDlg( parent, false, true, true )
{
  if ( showControls )
  {
    QWidget* hB = new QWidget( this );
    myControls[SMESHGUI_MeshInfoDlg::BaseInfo] = new QCheckBox( SMESHGUI::tr( "PREF_DUMP_BASE_INFO" ), hB );
    myControls[SMESHGUI_MeshInfoDlg::ElemInfo] = new QCheckBox( SMESHGUI::tr( "PREF_DUMP_ELEM_INFO" ), hB );
    myControls[SMESHGUI_MeshInfoDlg::AddInfo] = new QCheckBox( SMESHGUI::tr( "PREF_DUMP_ADD_INFO" ),  hB );
    myControls[SMESHGUI_MeshInfoDlg::CtrlInfo] = new QCheckBox( SMESHGUI::tr( "PREF_DUMP_CTRL_INFO" ), hB );

    QGridLayout* layout = new QGridLayout( hB );
    layout->setMargin( 0 );
    layout->addWidget( myControls[SMESHGUI_MeshInfoDlg::BaseInfo], 0, 0 );
    layout->addWidget( myControls[SMESHGUI_MeshInfoDlg::ElemInfo], 0, 1 );
    layout->addWidget( myControls[SMESHGUI_MeshInfoDlg::AddInfo], 1, 0 );
    layout->addWidget( myControls[SMESHGUI_MeshInfoDlg::CtrlInfo], 1, 1 );

    addWidgets( 0, hB, 0 );
  }
}

/*!
  \brief Get control's value.
  \param option Control identifier.
  \return Control value.
  \internal
*/
bool DumpFileDlg::isChecked( int option ) const
{
  return myControls.contains( option ) ? myControls[option]->isChecked() : false;
}

/*!
  \brief Set control's initial value.
  \param option Control identifier.
  \param value Control value.
  \internal
*/
void DumpFileDlg::setChecked( int option, bool value )
{
  if ( myControls.contains( option ) ) 
    myControls[option]->setChecked( value );
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_BaseInfo
/// \brief Show basic information on selected object.
///
/// Displays the base information about selected object: mesh, sub-mesh, group
/// or arbitrary ID source.
/// \todo Hide unnecessary widgets (e.g. for mesh group).
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
  \param parent Parent widget. Defaults to 0.
*/
SMESHGUI_BaseInfo::SMESHGUI_BaseInfo( QWidget* parent ): SMESHGUI_Info( parent )
{
  QGridLayout* l = new QGridLayout( this );
  l->setMargin( MARGIN );
  l->setSpacing( SPACING );

  // object info
  // - name
  addWidget( createLabel( tr( "NAME_LAB" ), this, Bold ), iName, iLabel );
  addWidget( createField( this, "meshName" ), iName, iSingle, 4 )->setMinimumWidth( 150 );
  // - type
  addWidget( createLabel( tr( "OBJECT_LAB" ), this, Bold ), iObject, iLabel );
  addWidget( createField( this, "meshType" ), iObject, iSingle, 4 )->setMinimumWidth( 150 );
  // - --- (separator)
  addWidget( createSeparator( this ), iObjectEnd, iLabel, 5 );

  // node info
  // - info
  addWidget( createLabel( tr( "NODES_LAB" ), this, Bold ), iNodes, iLabel );
  addWidget( createField( this, "nbNodes" ), iNodes, iTotal );
  // - --- (separator)
  addWidget( createSeparator( this ), iNodesEnd, iLabel, 5 );

  // element info
  // - title
  addWidget( createLabel( tr( "ELEMENTS_LAB" ), this, Bold ), iElementsTitle, iLabel );
  addWidget( createLabel( tr( "TOTAL_LAB" ), this, Italic ), iElementsTitle, iTotal );
  addWidget( createLabel( tr( "LINEAR_LAB" ), this, Italic ), iElementsTitle, iLinear );
  addWidget( createLabel( tr( "QUADRATIC_LAB" ), this, Italic ), iElementsTitle, iQuadratic );
  addWidget( createLabel( tr( "BI_QUADRATIC_LAB" ), this, Italic ), iElementsTitle, iBiQuadratic );
  // - --- (separator)
  addWidget( createSeparator( this ), iElementsTitleEnd, iTotal, 4 );
  // - summary
  addWidget( createField( this, "totalNbElems" ), iElementsTotal, iTotal );
  addWidget( createField( this, "totalNbLinearElems" ), iElementsTotal, iLinear );
  addWidget( createField( this, "totalNbQuadraticElems" ), iElementsTotal, iQuadratic );
  addWidget( createField( this, "totalNbBiQuadraticElems" ), iElementsTotal, iBiQuadratic );
  // - --- (separator)
  addWidget( createSeparator( this ), iElementsTotalEnd, iTotal, 4 );
  // - 0D elements info
  addWidget( createLabel( tr( "0D_LAB" ), this, Bold | Italic ), i0D, iLabel );
  addWidget( createField( this, "nb0D" ), i0D, iTotal );
  // - --- (separator)
  addWidget( createSeparator( this ), i0DEnd, iTotal, 4 );
  // - balls info
  addWidget( createLabel( tr( "BALL_LAB" ), this, Bold | Italic ), iBalls, iLabel );
  addWidget( createField( this, "nbBall" ), iBalls, iTotal );
  // - --- (separator)
  addWidget( createSeparator( this ), iBallsEnd, iTotal, 4 );
  // - 1D elements info
  addWidget( createLabel( tr( "1D_LAB" ), this, Bold | Italic ), i1D, iLabel );
  addWidget( createField( this, "nb1D" ), i1D, iTotal );
  addWidget( createField( this, "nbLinear1D" ), i1D, iLinear );
  addWidget( createField( this, "nbQuadratic1D" ), i1D, iQuadratic );
  // - --- (separator)
  addWidget( createSeparator( this ), i1DEnd, iTotal, 4 );
  // - 2D elements info
  // --+ summary
  addWidget( createLabel( tr( "2D_LAB" ), this, Bold | Italic ), i2D, iLabel );
  addWidget( createField( this, "nb2D" ), i2D, iTotal );
  addWidget( createField( this, "nbLinear2D" ), i2D, iLinear );
  addWidget( createField( this, "nbQuadratic2D" ), i2D, iQuadratic );
  addWidget( createField( this, "nbBiQuadratic2D" ), i2D, iBiQuadratic );
  // --+ triangles
  addWidget( createLabel( tr( "TRIANGLES_LAB" ), this, Italic ), i2DTriangles, iLabel );
  addWidget( createField( this, Italic, "nbTriangle" ), i2DTriangles, iTotal );
  addWidget( createField( this, Italic, "nbLinearTriangle" ), i2DTriangles, iLinear );
  addWidget( createField( this, Italic, "nbQuadraticTriangle" ), i2DTriangles, iQuadratic );
  addWidget( createField( this, Italic, "nbBiQuadraticTriangle" ), i2DTriangles, iBiQuadratic );
  // --+ quadrangles
  addWidget( createLabel( tr( "QUADRANGLES_LAB" ), this, Italic ), i2DQuadrangles, iLabel );
  addWidget( createField( this, Italic, "nbQuadrangle" ), i2DQuadrangles, iTotal );
  addWidget( createField( this, Italic, "nbLinearQuadrangle" ), i2DQuadrangles, iLinear );
  addWidget( createField( this, Italic, "nbQuadraticQuadrangle" ), i2DQuadrangles, iQuadratic );
  addWidget( createField( this, Italic, "nbBiQuadraticQuadrangle" ), i2DQuadrangles, iBiQuadratic );
  // --+ polygons
  addWidget( createLabel( tr( "POLYGONS_LAB" ), this, Italic ), i2DPolygons, iLabel );
  addWidget( createField( this, Italic, "nbPolygon" ), i2DPolygons, iTotal );
  addWidget( createField( this, Italic, "nbLinearPolygon" ), i2DPolygons, iLinear );
  addWidget( createField( this, Italic, "nbQuadraticPolygon" ), i2DPolygons, iQuadratic );
  // - --- (separator)
  addWidget( createSeparator( this ), i2DEnd, iTotal, 4 );
  // - 3D elements info
  // --+ summary
  addWidget( createLabel( tr( "3D_LAB" ), this, Bold | Italic ), i3D, iLabel );
  addWidget( createField( this, "nb3D" ), i3D, iTotal );
  addWidget( createField( this, "nbLinear3D" ), i3D, iLinear );
  addWidget( createField( this, "nbQuadratic3D" ), i3D, iQuadratic );
  addWidget( createField( this, "nbBiQuadratic3D" ), i3D, iBiQuadratic );
  // --+ tetras
  addWidget( createLabel( tr( "TETRAHEDRONS_LAB" ), this, Italic ), i3DTetrahedrons, iLabel );
  addWidget( createField( this, Italic, "nbTetrahedron" ), i3DTetrahedrons, iTotal );
  addWidget( createField( this, Italic, "nbLinearTetrahedron" ), i3DTetrahedrons, iLinear );
  addWidget( createField( this, Italic, "nbQudraticTetrahedron" ), i3DTetrahedrons, iQuadratic );
  // --+ hexas
  addWidget( createLabel( tr( "HEXAHEDONRS_LAB" ), this, Italic ), i3DHexahedrons, iLabel );
  addWidget( createField( this, Italic, "nbHexahedron" ), i3DHexahedrons, iTotal );
  addWidget( createField( this, Italic, "nbLinearHexahedron" ), i3DHexahedrons, iLinear );
  addWidget( createField( this, Italic, "nbQuadraticHexahedron" ), i3DHexahedrons, iQuadratic );
  addWidget( createField( this, Italic, "nbBiQuadraticHexahedron" ), i3DHexahedrons, iBiQuadratic );
  // --+ pyramids
  addWidget( createLabel( tr( "PYRAMIDS_LAB" ), this, Italic ), i3DPyramids, iLabel );
  addWidget( createField( this, Italic, "nbPyramid" ), i3DPyramids, iTotal );
  addWidget( createField( this, Italic, "nbLinearPyramid" ), i3DPyramids, iLinear );
  addWidget( createField( this, Italic, "nbQuadraticPyramid" ), i3DPyramids, iQuadratic );
  // --+ prisms
  addWidget( createLabel( tr( "PRISMS_LAB" ), this, Italic ), i3DPrisms, iLabel );
  addWidget( createField( this, Italic, "nbPrism" ), i3DPrisms, iTotal );
  addWidget( createField( this, Italic, "nbLinearPrism" ), i3DPrisms, iLinear );
  addWidget( createField( this, Italic, "nbQuadraticPrism" ), i3DPrisms, iQuadratic );
  addWidget( createField( this, Italic, "nbBiQuadraticPrism" ), i3DPrisms, iBiQuadratic );
  // --+ hexagonal prisms
  addWidget( createLabel( tr( "HEX_PRISMS_LAB" ), this, Italic ), i3DHexaPrisms, iLabel );
  addWidget( createField( this, Italic, "nbHexagonalPrism" ), i3DHexaPrisms, iTotal );
  // --+ polyhedrons
  addWidget( createLabel( tr( "POLYHEDRONS_LAB" ), this, Italic ), i3DPolyhedrons, iLabel );
  addWidget( createField( this, Italic, "nbPolyhedron" ), i3DPolyhedrons, iTotal );

  // load button
  QPushButton* loadBtn = new QPushButton( tr( "BUT_LOAD_MESH" ), this );
  loadBtn->setAutoDefault( true );
  connect( loadBtn, SIGNAL( clicked() ), this, SLOT( loadMesh() ) );
  addWidget( loadBtn, iEnd, iSingle, 4 );

  // additional layout set-up
  l->setColumnStretch( iLabel, 0 );
  l->setColumnStretch( iTotal, 5 );
  l->setColumnStretch( iLinear, 5 );
  l->setColumnStretch( iQuadratic, 5 );
  l->setColumnStretch( iBiQuadratic, 5 );
  l->setRowStretch( iElementsEnd, 5 );

  // set initial values
  clear();
}

/*!
  \brief Destructor.
*/
SMESHGUI_BaseInfo::~SMESHGUI_BaseInfo()
{
}

/*!
  \brief Show information on given object.
  \param proxy Object to show information on (mesh, sub-mesh, group, ID source).
*/
void SMESHGUI_BaseInfo::showInfo( const SMESH::SelectionProxy& proxy )
{
  // reset panel
  clear();

  // then fill panel with data if object is not null
  if ( proxy )
  {
    myProxy = proxy;

    SMESH::MeshInfo info = proxy.meshInfo();

    // object info
    // - name
    widget( iName, iSingle )->setProperty( "text", proxy.name() );
    // - type
    QString typeName;
    SMESH::SelectionProxy::Type type = proxy.type();
    if ( type == SMESH::SelectionProxy::Mesh )
    {
      typeName = tr( "OBJECT_MESH" );
    }
    else if ( type == SMESH::SelectionProxy::Submesh )
    {
      typeName = tr( "OBJECT_SUBMESH" );
    }
    else if ( type >= SMESH::SelectionProxy::Group )
    {
      switch( proxy.groupElementType() )
      {
      case SMESH::NODE:   typeName = tr( "OBJECT_GROUP_NODES"   ); break;
      case SMESH::EDGE:   typeName = tr( "OBJECT_GROUP_EDGES"   ); break;
      case SMESH::FACE:   typeName = tr( "OBJECT_GROUP_FACES"   ); break;
      case SMESH::VOLUME: typeName = tr( "OBJECT_GROUP_VOLUMES" ); break;
      case SMESH::ELEM0D: typeName = tr( "OBJECT_GROUP_0DELEMS" ); break;
      case SMESH::BALL:   typeName = tr( "OBJECT_GROUP_BALLS"   ); break;
      default:            typeName = tr( "OBJECT_GROUP"         ); break;
      }
      QString subType;
      if ( type == SMESH::SelectionProxy::GroupStd )
        subType = tr( "OBJECT_GROUP_STANDALONE" );
      else if ( type == SMESH::SelectionProxy::GroupGeom )
        subType = tr( "OBJECT_GROUP_GEOM" );
      else if ( type == SMESH::SelectionProxy::GroupFilter )
        subType = tr( "OBJECT_GROUP_FILTER" );
      if ( !subType.isEmpty() )
        typeName = QString( "%1 %2" ).arg( typeName, subType );
    }
    widget( iObject, iSingle )->setProperty( "text", typeName );

    // node info
    uint nbNodes = info[SMDSEntity_Node];
    widget( iNodes, iTotal )->setProperty( "value", nbNodes );

    // element info
    // - 0D elements info
    uint nb0d = info[SMDSEntity_0D];
    widget( i0D, iTotal )->setProperty( "value", nb0d );
    // - balls info
    uint nbBalls = info[SMDSEntity_Ball];
    widget( iBalls, iTotal )->setProperty( "value", nbBalls );
    // - 1D elements info
    uint nb1dLin = info[SMDSEntity_Edge];
    uint nb1dQua = info[SMDSEntity_Quad_Edge];
    uint nb1d = nb1dLin + nb1dQua;
    widget( i1D, iLinear )->setProperty( "value", nb1dLin );
    widget( i1D, iQuadratic )->setProperty( "value", nb1dQua );
    widget( i1D, iTotal )->setProperty( "value", nb1d );
    // - 2D elements info
    // --+ triangles
    uint nbTriLin = info[SMDSEntity_Triangle];
    uint nbTriQua = info[SMDSEntity_Quad_Triangle];
    uint nbTriBiq = info[SMDSEntity_BiQuad_Triangle];
    uint nbTri = nbTriLin + nbTriQua + nbTriBiq;
    widget( i2DTriangles, iLinear )->setProperty( "value", nbTriLin );
    widget( i2DTriangles, iQuadratic )->setProperty( "value", nbTriQua );
    widget( i2DTriangles, iBiQuadratic )->setProperty( "value", nbTriBiq );
    widget( i2DTriangles, iTotal )->setProperty( "value", nbTri );
    // --+ quadrangles
    uint nbQuadLin = info[SMDSEntity_Quadrangle];
    uint nbQuadQua = info[SMDSEntity_Quad_Quadrangle];
    uint nbQuadBiq = info[SMDSEntity_BiQuad_Quadrangle];
    uint nbQuad = nbQuadLin + nbQuadQua + nbQuadBiq;
    widget( i2DQuadrangles, iLinear )->setProperty( "value", nbQuadLin );
    widget( i2DQuadrangles, iQuadratic )->setProperty( "value", nbQuadQua );
    widget( i2DQuadrangles, iBiQuadratic )->setProperty( "value", nbQuadBiq );
    widget( i2DQuadrangles, iTotal )->setProperty( "value", nbQuad );
    // --+ polygons
    uint nbPolyLin = info[SMDSEntity_Polygon];
    uint nbPolyQua = info[SMDSEntity_Quad_Polygon];
    uint nbPoly = nbPolyLin + nbPolyQua;
    widget( i2DPolygons, iLinear )->setProperty( "value", nbPolyLin );
    widget( i2DPolygons, iQuadratic )->setProperty( "value", nbPolyQua );
    widget( i2DPolygons, iTotal )->setProperty( "value", nbPoly );
    // --+ summary
    uint nb2dLin = nbTriLin + nbQuadLin + nbPolyLin;
    uint nb2dQua = nbTriQua + nbQuadQua + nbPolyQua;
    uint nb2dBiq = nbTriBiq + nbQuadBiq;
    uint nb2d = nb2dLin + nb2dQua + nb2dBiq;
    widget( i2D, iLinear )->setProperty( "value", nb2dLin );
    widget( i2D, iQuadratic )->setProperty( "value", nb2dQua );
    widget( i2D, iBiQuadratic )->setProperty( "value", nb2dBiq );
    widget( i2D, iTotal )->setProperty( "value", nb2d );
    // - 3D elements info
    // --+ tetras
    uint nbTetraLin = info[SMDSEntity_Tetra];
    uint nbTetraQua = info[SMDSEntity_Quad_Tetra];
    uint nbTetra = nbTetraLin + nbTetraQua;
    widget( i3DTetrahedrons, iLinear )->setProperty( "value", nbTetraLin );
    widget( i3DTetrahedrons, iQuadratic )->setProperty( "value", nbTetraQua );
    widget( i3DTetrahedrons, iTotal )->setProperty( "value", nbTetra );
    // --+ hexas
    uint nbHexaLin = info[SMDSEntity_Hexa];
    uint nbHexaQua = info[SMDSEntity_Quad_Hexa];
    uint nbHexaBiq = info[SMDSEntity_TriQuad_Hexa];
    uint nbHexa = nbHexaLin + nbHexaQua + nbHexaBiq;
    widget( i3DHexahedrons, iLinear )->setProperty( "value", nbHexaLin );
    widget( i3DHexahedrons, iQuadratic )->setProperty( "value", nbHexaQua );
    widget( i3DHexahedrons, iBiQuadratic )->setProperty( "value", nbHexaBiq );
    widget( i3DHexahedrons, iTotal )->setProperty( "value", nbHexa );
    // --+ pyramids
    uint nbPyraLin = info[SMDSEntity_Pyramid];
    uint nbPyraQua = info[SMDSEntity_Quad_Pyramid];
    uint nbPyra = nbPyraLin + nbPyraQua;
    widget( i3DPyramids, iLinear )->setProperty( "value", nbPyraLin );
    widget( i3DPyramids, iQuadratic )->setProperty( "value", nbPyraQua );
    widget( i3DPyramids, iTotal )->setProperty( "value", nbPyra );
    // --+ prisms
    uint nbPentaLin = info[SMDSEntity_Penta];
    uint nbPentaQua = info[SMDSEntity_Quad_Penta];
    uint nbPentaBiq = info[SMDSEntity_BiQuad_Penta];
    uint nbPenta = nbPentaLin + nbPentaQua + nbPentaBiq;
    widget( i3DPrisms, iLinear )->setProperty( "value", nbPentaLin );
    widget( i3DPrisms, iQuadratic )->setProperty( "value", nbPentaQua );
    widget( i3DPrisms, iBiQuadratic )->setProperty( "value", nbPentaBiq );
    widget( i3DPrisms, iTotal )->setProperty( "value", nbPenta );
    // --+ hexagonal prisms
    uint nbHexaPri = info[SMDSEntity_Hexagonal_Prism];
    widget( i3DHexaPrisms, iTotal )->setProperty( "value", nbHexaPri );
    // --+ polyhedrons
    uint nbPolyhedra = info[SMDSEntity_Polyhedra];
    widget( i3DPolyhedrons, iTotal )->setProperty( "value", nbPolyhedra );
    // --+ summary
    uint nb3dLin = nbTetraLin + nbHexaLin + nbPyraLin + nbPentaLin + nbHexaPri + nbPolyhedra;
    uint nb3dQua = nbTetraQua + nbHexaQua + nbPyraQua + nbPentaQua;
    uint nb3dBiq = nbHexaBiq + nbPentaBiq;
    uint nb3d = nb3dLin + nb3dQua + nb3dBiq;
    widget( i3D, iLinear )->setProperty( "value", nb3dLin );
    widget( i3D, iQuadratic )->setProperty( "value", nb3dQua );
    widget( i3D, iBiQuadratic )->setProperty( "value", nb3dBiq );
    widget( i3D, iTotal )->setProperty( "value", nb3d );
    // - summary
    uint nbElemLin = nb1dLin + nb2dLin + nb3dLin;
    uint nbElemQua = nb1dQua + nb2dQua + nb3dQua;
    uint nbElemBiq = nb2dBiq + nb3dBiq;
    uint nbElem = nb0d + nbBalls + nb1d + nb2d + nb3d;
    widget( iElementsTotal, iLinear )->setProperty( "value", nbElemLin );
    widget( iElementsTotal, iQuadratic )->setProperty( "value", nbElemQua );
    widget( iElementsTotal, iBiQuadratic )->setProperty( "value", nbElemBiq );
    widget( iElementsTotal, iTotal )->setProperty( "value", nbElem );

    // show 'Load' button if data was not loaded yet
    widget( iEnd, iSingle )->setVisible( !proxy.isValid() );

    // until data is loaded from study file, type of elements in a sub-mesh or group
    // can be undefined in some cases
    if ( !proxy.isValid() )
    {
      // two cases are possible:
      // 1. type of 2D or 3D elements is unknown but their nb is OK (for a sub-mesh)
      // 2. there is no info at all (for a group on geom or on filter)
      if ( info.count( SMDSEntity_Node, SMDSEntity_Last ) > 0 ) // believe it is a sub-mesh
      {
        if ( nb2dLin + nb2dQua + nb2dBiq > 0 )
        {
          // we know nothing about triangles, quadranges and polygons
          for ( int i = i2DTriangles; i < i2DEnd; i++ )
          {
            for ( int j = iTotal; j < iNbColumns; j++ )
            {
              if ( widget( i, j ) )
                widget( i, j )->setProperty( "text", "?" );
            }
          }
          // we don't know if elements are linear, quadratic or bi-quadratic
          for ( int j = iLinear; j < iNbColumns; j++ )
          {
            if ( widget( i2D, j ) )
              widget( i2D, j )->setProperty( "text", "?" );
            if ( widget( iElementsTotal, j ) )
              widget( iElementsTotal, j )->setProperty( "text", "?" );
          }
        }
        else if ( nb3dLin + nb3dQua + nb3dBiq > 0 )
        {
          // we know nothing about tetras, hexas, etc.
          for ( int i = i3DTetrahedrons; i < i3DEnd; i++ )
          {
            for ( int j = iTotal; j < iNbColumns; j++ )
            {
              if ( widget( i, j ) )
                widget( i, j )->setProperty( "text", "?" );
            }
          }
          // we don't know if elements are linear, quadratic or bi-quadratic
          for ( int j = iLinear; j < iNbColumns; j++ )
          {
            if ( widget( i3D, j ) )
              widget( i3D, j )->setProperty( "text", "?" );
            if ( widget( iElementsTotal, j ) )
              widget( iElementsTotal, j )->setProperty( "text", "?" );
          }
        }
      }
      else
      {
        // we know nothing about nodes :(
        widget( iNodes, iTotal )->setProperty( "text", "?" );
        // we know nothing about elements :(
        for ( int i = iElementsTotal; i < iElementsEnd; i++ )
        {
          for ( int j = iTotal; j < iNbColumns; j++ )
          {
            if ( widget( i, j ) )
              widget( i, j )->setProperty( "text", "?" );
          }
        }
      }
    }
  }
}

/*!
  \brief Update information in panel.
*/
void SMESHGUI_BaseInfo::updateInfo()
{
  showInfo( myProxy );
}

/*!
  \brief Load mesh from a study file.
*/
void SMESHGUI_BaseInfo::loadMesh()
{
  SUIT_OverrideCursor wc;
  if ( myProxy )
  {
    myProxy.load();
    updateInfo();
  }
}

/*!
  \brief Reset panel (clear all data).
*/
void SMESHGUI_BaseInfo::clear()
{
  // - name
  widget( iName, iSingle )->setProperty( "text", QString() );
  // - type
  widget( iObject, iSingle )->setProperty( "text", QString() );
  // - nodes
  widget( iNodes, iTotal )->setProperty( "value", 0 );
  // - elements
  for ( int i = iElementsTotal; i < iElementsEnd; i++ )
  {
    for ( int j = iTotal; j < iNbColumns; j++ )
    {
      if ( widget( i, j ) )
        widget( i, j )->setProperty( "value", 0 );
    }
  }
  // hide 'Load' button
  widget( iEnd, iSingle )->setVisible( false );
}

/*!
  \brief Register widget in a grid.
  \param w Widget being added.
  \param row Row index in a grid.
  \param column Column index in a grid.
  \param colspan Number of columns to span in a grid. Defaults to 1.
  \return Just added widget.
*/
QWidget* SMESHGUI_BaseInfo::addWidget( QWidget* w, int row, int column, int colspan )
{
  if ( !myWidgets.contains( row ) )
    myWidgets[row] = wlist();
  myWidgets[row][column] = w;
  dynamic_cast<QGridLayout*>( layout() )->addWidget( w, row, column, 1, colspan );
  return w;
}

/*!
  \brief Get registered widget.
  \param row Row index in a grid.
  \param column Column index in a grid.
  \return Widget stored in a given grid cell (0 if there's no widget).
*/
QWidget* SMESHGUI_BaseInfo::widget( int row, int column ) const
{
  return myWidgets.contains( row ) && myWidgets[row].contains( column ) ? myWidgets[row][column] : 0;
}

/*!
  \brief Get text value from registered widget.
  \param row Row index in a grid.
  \param column Column index in a grid.
  \return Value string (empty string if no label in given cell).
*/
QString SMESHGUI_BaseInfo::value( int row, int column ) const
{
  return widgetValue( widget( row, column ) );
}

/*!
  \brief Show/hide group(s) of widgets.
  \param startRow Starting grid row.
  \param lastRow Last grid row.
  \param on Visibility flag.
*/
void SMESHGUI_BaseInfo::setFieldsVisible( int startRow, int lastRow, bool on )
{
  startRow = qMax( 0, startRow );
  lastRow = qMin( lastRow, (int)iEnd );
  for ( int i = startRow; i <= lastRow; i++ )
  {
    wlist widgets = myWidgets[i];
    foreach ( QWidget* w, widgets )
      w->setVisible( on );
  }
}

/*!
  \brief Write information from panel to output stream.
  \param out Text stream output.
*/
void SMESHGUI_BaseInfo::saveInfo( QTextStream& out )
{
  // title
  QString title = tr( "BASE_INFO" );
  out << ruler( title.size() ) << endl;
  out << title << endl;
  out << ruler( title.size() ) << endl;
  out << endl;

  // object info
  // - name
  out << tr( "NAME_LAB" ) << spacing() << value( iName, iSingle ) << endl;
  // - type
  out << tr( "OBJECT_LAB" ) << spacing() << value( iObject, iSingle ) << endl;
  // - --- (separator)
  out << endl;

  // node info
  out << tr( "NODES_LAB" ) << spacing() << value( iNodes, iTotal ) << endl;
  // - --- (separator)
  out << endl;

  // element info
  QString lin = tr( "LINEAR_LAB" ) + ":" + spacing();
  QString qua = tr( "QUADRATIC_LAB" ) + ":" + spacing();
  QString biq = tr( "BI_QUADRATIC_LAB" ) + ":" + spacing();
  // - summary
  out << tr( "ELEMENTS_LAB" ) << spacing() << value( iElementsTotal, iTotal ) << endl;
  out << indent(1) << lin << value( iElementsTotal, iLinear ) << endl;
  out << indent(1) << qua << value( iElementsTotal, iQuadratic ) << endl;
  out << indent(1) << biq << value( iElementsTotal, iBiQuadratic ) << endl;
  // - --- (separator)
  out << endl;
  // - 0D elements info
  out << indent(1) << tr( "0D_LAB" ) << spacing() << value( i0D, iTotal ) << endl;
  // - --- (separator)
  out << endl;
  // - balls info
  out << indent(1) << tr( "BALL_LAB" ) << spacing() << value( iBalls, iTotal ) << endl;
  // - --- (separator)
  out << endl;
  // - 1D elements info
  out << indent(1) << tr( "1D_LAB" ) << spacing() << value( i1D, iTotal ) << endl;
  out << indent(2) << lin << value( i1D, iLinear ) << endl;
  out << indent(2) << qua << value( i1D, iQuadratic ) << endl;
  // - --- (separator)
  out << endl;
  // - 2D elements info
  // - summary
  out << indent(1) << tr( "2D_LAB" ) << spacing() << value( i2D, iTotal ) << endl;
  out << indent(2) << lin << value( i2D, iLinear ) << endl;
  out << indent(2) << qua << value( i2D, iQuadratic ) << endl;
  out << indent(2) << biq << value( i2D, iBiQuadratic ) << endl;
  // - --- (separator)
  out << endl;
  // --+ triangles
  out << indent(2) << tr( "TRIANGLES_LAB" ) << spacing() << value( i2DTriangles, iTotal ) << endl;
  out << indent(3) << lin << value( i2DTriangles, iLinear ) << endl;
  out << indent(3) << qua << value( i2DTriangles, iQuadratic ) << endl;
  out << indent(3) << biq << value( i2DTriangles, iBiQuadratic ) << endl;
  // --+ quadrangles
  out << indent(2) << tr( "QUADRANGLES_LAB" ) << spacing() << value( i2DQuadrangles, iTotal ) << endl;
  out << indent(3) << lin << value( i2DQuadrangles, iLinear ) << endl;
  out << indent(3) << qua << value( i2DQuadrangles, iQuadratic ) << endl;
  out << indent(3) << biq << value( i2DQuadrangles, iBiQuadratic ) << endl;
  // --+ polygons
  out << indent(2) << tr( "POLYGONS_LAB" ) << spacing() << value( i2DPolygons, iTotal ) << endl;
  out << indent(3) << lin << value( i2DPolygons, iLinear ) << endl;
  out << indent(3) << qua << value( i2DPolygons, iQuadratic ) << endl;
  // - --- (separator)
  out << endl;
  // - 3D elements info
  // --+ summary
  out << indent(1) << tr( "3D_LAB" ) << spacing() << value( i3D, iTotal ) << endl;
  out << indent(2) << lin << value( i3D, iLinear ) << endl;
  out << indent(2) << qua << value( i3D, iQuadratic ) << endl;
  out << indent(2) << biq << value( i3D, iBiQuadratic ) << endl;
  // - --- (separator)
  out << endl;
  // --+ tetras
  out << indent(2) << tr( "TETRAHEDRONS_LAB" ) << spacing() << value( i3DTetrahedrons, iTotal ) << endl;
  out << indent(3) << lin << value( i3DTetrahedrons, iLinear ) << endl;
  out << indent(3) << qua << value( i3DTetrahedrons, iQuadratic ) << endl;
  // --+ hexas
  out << indent(2) << tr( "HEXAHEDONRS_LAB" ) << spacing() << value( i3DHexahedrons, iTotal ) << endl;
  out << indent(3) << lin << value( i3DHexahedrons, iLinear ) << endl;
  out << indent(3) << qua << value( i3DHexahedrons, iQuadratic ) << endl;
  out << indent(3) << biq << value( i3DHexahedrons, iBiQuadratic ) << endl;
  // --+ pyramids
  out << indent(2) << tr( "PYRAMIDS_LAB" ) << spacing() << value( i3DPyramids, iTotal ) << endl;
  out << indent(3) << lin << value( i3DPyramids, iLinear ) << endl;
  out << indent(3) << qua << value( i3DPyramids, iQuadratic ) << endl;
  // --+ prisms
  out << indent(2) << tr( "PRISMS_LAB" ) << spacing() << value( i3DPrisms, iTotal ) << endl;
  out << indent(3) << lin << value( i3DPrisms, iLinear ) << endl;
  out << indent(3) << qua << value( i3DPrisms, iQuadratic ) << endl;
  out << indent(3) << biq << value( i3DPrisms, iBiQuadratic ) << endl;
  // --+ hexagonal prisms
  out << indent(2) << tr( "HEX_PRISMS_LAB" ) << spacing() << value( i3DHexaPrisms, iTotal ) << endl;
  // --+ polyhedrons
  out << indent(2) << tr( "POLYHEDRONS_LAB" ) << spacing() << value( i3DPolyhedrons, iTotal ) << endl;
  // - --- (separator)
  out << endl;
}

////////////////////////////////////////////////////////////////////////////////
/// \class InfoWriter
/// \brief Base info writer class.
/// \internal
////////////////////////////////////////////////////////////////////////////////

class InfoWriter
{
protected:
  int myPrecision;
  bool myRecursive;
public:
  InfoWriter( bool = false );
  void write( const QString&, bool = false );
  void write( const QString&, const QString&, bool = false );
  void write( const QString&, int, bool = false );
  void write( const QString&, double, bool = false );
  void write( const QString&, const SMESH::XYZ&, bool = false );
  virtual void indent() {}
  virtual void unindent() {}
  virtual void separator() {}
protected:
  virtual void put( const QString&, const QString&, bool = false ) = 0;
};

InfoWriter::InfoWriter( bool r ): myRecursive(r)
{
  myPrecision = SMESHGUI::resourceMgr()->integerValue( "SMESH", "length_precision", 6 );
}

void InfoWriter::write( const QString& key, bool emphasize )
{
  put( key, QString(), emphasize );
}

void InfoWriter::write( const QString& key, const QString& value, bool emphasize )
{
  put( key, value, emphasize );
}

void InfoWriter::write( const QString& key, int value, bool emphasize )
{
  put( key, QString::number( value ), emphasize );
}

void InfoWriter::write( const QString& key, double value, bool emphasize )
{
  put( key, QString::number( value, myPrecision > 0 ? 'f' : 'g', qAbs( myPrecision ) ), emphasize );
}

void InfoWriter::write( const QString& key, const SMESH::XYZ& value, bool emphasize )
{
  if ( myRecursive )
  {
    write( key, emphasize );
    indent();
    write( "X", value.x() );
    write( "Y", value.y() );
    write( "Z", value.z() );
    unindent(); 
  }
  else
  {
    QStringList vl;
    vl << QString::number( value.x(), myPrecision > 0 ? 'f' : 'g', qAbs( myPrecision ) );
    vl << QString::number( value.y(), myPrecision > 0 ? 'f' : 'g', qAbs( myPrecision ) );
    vl << QString::number( value.z(), myPrecision > 0 ? 'f' : 'g', qAbs( myPrecision ) );
    put( key, vl.join( ", " ), emphasize );
  }
}

////////////////////////////////////////////////////////////////////////////////
/// \class SimpleWriter
/// \brief Base text writer.
/// \internal
////////////////////////////////////////////////////////////////////////////////

class SimpleWriter: public InfoWriter
{
protected:
  int myIndent;
public:
  SimpleWriter();
  void indent();
  void unindent();
  void separator();
protected:
  void put( const QString&, const QString&, bool );
  virtual QString spacer() const;
  virtual QString decorate( const QString& ) const;
  virtual void dumpLine( const QString& ) = 0;
};

SimpleWriter::SimpleWriter(): InfoWriter(false), myIndent(0)
{
}

void SimpleWriter::indent()
{
  myIndent += 1;
}

void SimpleWriter::unindent()
{
  myIndent = qMax( myIndent-1, 0 );
}

void SimpleWriter::separator()
{
  write( "" );
}

QString SimpleWriter::spacer() const
{
  return " ";
}

QString SimpleWriter::decorate( const QString& s ) const
{
  return s;
}

void SimpleWriter::put( const QString& key, const QString& value, bool emphasize )
{
  QString line;
  line += ::indent( spacer(), myIndent*4 );
  line += decorate( key );
  if ( !value.isEmpty() )
  {
    line += ":";
    line += emphasize ? decorate( value ) : value;
  }
  dumpLine( line );
}

////////////////////////////////////////////////////////////////////////////////
/// \class StreamWriter
/// \brief Writer for QTextStream.
/// \internal
////////////////////////////////////////////////////////////////////////////////

class StreamWriter: public SimpleWriter
{
  QTextStream& myOut;
public:
  StreamWriter( QTextStream& );
protected:
  void dumpLine( const QString& );
};

StreamWriter::StreamWriter( QTextStream& out ): SimpleWriter(), myOut(out)
{
}

void StreamWriter::dumpLine( const QString& line )
{
  myOut << line;
  myOut << endl;
}

////////////////////////////////////////////////////////////////////////////////
/// \class TextWriter
/// \brief Writer for QTextBrowser.
/// \internal
////////////////////////////////////////////////////////////////////////////////

class TextWriter: public SimpleWriter
{
  QTextBrowser* myInfo;
public:
  TextWriter( QTextBrowser* );
protected:
  QString spacer() const;
  QString decorate( const QString& ) const;
  void dumpLine( const QString& );
};

TextWriter::TextWriter( QTextBrowser* w ): SimpleWriter(), myInfo(w)
{
}

QString TextWriter::spacer() const
{
  return "&nbsp;";
}

QString TextWriter::decorate( const QString& s ) const
{
  return bold( s );
}

void TextWriter::dumpLine( const QString& line )
{
  myInfo->append( line );
}

////////////////////////////////////////////////////////////////////////////////
/// \class TreeWriter
/// \brief Writer for QTreeWidget.
/// \internal
////////////////////////////////////////////////////////////////////////////////

class TreeWriter: public InfoWriter
{
  QTreeWidget* myInfo;
  QTreeWidgetItem* myCurrentItem;
  TreeItemCreator* myCreator;
public:
  TreeWriter( QTreeWidget*, TreeItemCreator* );
  ~TreeWriter();
  void indent();
  void unindent();
protected:
  void put( const QString&, const QString&, bool = false );
};

TreeWriter::TreeWriter( QTreeWidget* w, TreeItemCreator* c ):
  InfoWriter(true), myInfo(w), myCurrentItem(0), myCreator(c)
{
}

TreeWriter::~TreeWriter()
{
  delete myCreator;
}

void TreeWriter::put( const QString& key, const QString& value, bool emphasize )
{
  //std::string sss = myCurrentItem ? myCurrentItem->text(0).toStdString() : "";
  int options = Bold;
  if ( emphasize ) options |= AllColumns;
  QTreeWidgetItem* item = myCreator->createItem( myCurrentItem, options );
  item->setText( 0, key );
  if ( !value.isEmpty() )
  {
    QString val = value;
    if ( value.startsWith( "<a href" )) // connectivity encoded as: <a href = "nodes://host/1 2">1 2</a>
    {
      int role = ( value[11] == 'n' ) ? NodeConnectivity : ElemConnectivity;
      val = value.mid( value.lastIndexOf( '>', -5 ) + 1 ); // ==>   1 2</a>
      val.chop( 4 );
      item->setData( 1, TypeRole, role );
    }
    item->setText( 1, val );
  }
}

void TreeWriter::indent()
{
  QTreeWidgetItem* item = myCurrentItem ? myCurrentItem : myInfo->invisibleRootItem();
  if ( item->childCount() > 0 )
    myCurrentItem = item->child( item->childCount()-1 );
}

void TreeWriter::unindent()
{
  if ( myCurrentItem )
    myCurrentItem = myCurrentItem->parent();
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_ElemInfo
/// \brief Base class for the mesh element information widget.
///
/// Displays the detail information about given mesh node(s) or element(s).
/// Default class does not provide working implementation but onle general
/// functionalities; main work is done in sub-classes.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
  \param parent Parent widget. Defaults to 0.
*/
SMESHGUI_ElemInfo::SMESHGUI_ElemInfo( QWidget* parent ): SMESHGUI_Info( parent ), myWhat( ShowNone )
{
  myFrame = new QWidget( this );
  myExtra = new ExtraWidget( this );

  QVBoxLayout* vbl = new QVBoxLayout( this );
  vbl->setMargin( 0 );
  vbl->setSpacing( SPACING );
  vbl->addWidget( myFrame );
  vbl->addWidget( myExtra );

  connect( myExtra->prev, SIGNAL( clicked() ), this, SLOT( showPrevious() ) );
  connect( myExtra->next, SIGNAL( clicked() ), this, SLOT( showNext() ) );

  clear();
}

/*!
  \brief Destructor.
*/
SMESHGUI_ElemInfo::~SMESHGUI_ElemInfo()
{
}

/*!
  \brief Show information on given node / element.
  \param proxy Object to compute information on (mesh, sub-mesh, group, ID source).
  \param id Mesh node / element ID.
  \param isElement If \c true, show element info; otherwise show node info.
*/
void SMESHGUI_ElemInfo::showInfo( const SMESH::SelectionProxy& proxy, uint id, bool isElement )
{
  QSet<uint> ids;
  ids << id;
  showInfo( proxy, ids, isElement );
}

/*!
  \brief Show information on given nodes / elements.
  \param proxy Object to compute information on (mesh, sub-mesh, group, ID source).
  \param ids Mesh nodes / elements IDs.
  \param isElement If \c true, show element info; otherwise show node info.
*/
void SMESHGUI_ElemInfo::showInfo( const SMESH::SelectionProxy& proxy, QSet<uint> ids, bool isElement )
{
  if ( !proxy )
  {
    clear();
    return;
  }

  QList<uint> newIds = ids.toList();
  qSort( newIds );
  int what = isElement ? ShowElements : ShowNodes;
  
  if ( myProxy == proxy && myIDs == newIds && myWhat == what )
    return;
  
  myProxy = proxy;
  myProxy.refresh(); // try to re-initialize actor

  clear();

  myIDs = newIds;
  myWhat = what;
  myIndex = 0;
  
  updateControls();
  information( myIDs.mid( myIndex*blockSize(), blockSize() ) );
}

/*!
  \brief Show information on given group.
  \param proxy Object to compute information on (group).
*/
void SMESHGUI_ElemInfo::showInfo( const SMESH::SelectionProxy& proxy )
{
  if ( !proxy || proxy.groupElementType() == SMESH::ALL ) // null proxy or not a group
  {
    clear();
    return;
  }

  showInfo( proxy, proxy.ids(), proxy.groupElementType() != SMESH::NODE );
}

/*!
  \brief Reset panel (clear all data).
*/
void SMESHGUI_ElemInfo::clear()
{
  myIDs.clear();
  myIndex = 0;
  clearInternal();
  updateControls();
}

/*!
  \brief Get central area widget.
  \return Central widget.
*/
QWidget* SMESHGUI_ElemInfo::centralWidget() const
{
  return myFrame;
}

/*!
  \brief Get current mesh proxy object information is shown on.
  \return Current proxy.
*/
SMESH::SelectionProxy SMESHGUI_ElemInfo::proxy() const
{
  return myProxy;
}

/*!
  \brief Get current info mode.
  \return Current panel mode.
*/
int SMESHGUI_ElemInfo::what() const
{
  return myWhat;
}

/*!
  \brief Get title for given element type.
  \param type Mesh element type.
  \param multiple Use plural form. Defaults to \c false.
  \return Element type's title.
*/
QString SMESHGUI_ElemInfo::type2str( int type, bool multiple )
{
  QString title;
  switch ( type )
  {
  case SMDSAbs_Edge:
    title = multiple ? tr( "EDGES" ) : tr( "EDGE" ) ; break;
  case SMDSAbs_Face:
    title = multiple ? tr( "FACES" ) : tr( "FACE" ); break;
  case SMDSAbs_Volume:
    title = multiple ? tr( "VOLUMES" ) : tr( "VOLUME" ); break;
  case SMDSAbs_0DElement:
    title = multiple ? tr( "0D_ELEMENTS" ) : tr( "0D_ELEMENT" ); break;
  case SMDSAbs_Ball:
    title = multiple ? tr( "BALL_ELEMENTS" ) : tr( "BALL" ); break;
  default:
    break;
  }
  return title;
}

/*!
  \brief Get title for given shape type.
  \param type Shape type.
  \return Shape type's title.
*/
QString SMESHGUI_ElemInfo::stype2str( int type )
{
  QString title;
  switch ( type )
  {
  case GEOM::VERTEX:
    title = tr( "GEOM_VERTEX" ); break;
  case GEOM::EDGE:
    title = tr( "GEOM_EDGE" ); break;
  case GEOM::FACE:
    title = tr( "GEOM_FACE" ); break;
  case GEOM::SOLID:
  default:
    title = tr( "GEOM_SOLID" ); break;
    break;
  }
  return title;
}

/*!
  \brief Get title for given element type.
  \param type Mesh element type.
  \return Element type's title.
*/
QString SMESHGUI_ElemInfo::etype2str( int type )
{
  QString title;
  switch ( type )
  {
  case SMESH::Entity_0D:
    title = tr( "SMESH_ELEM0D" ); break;
  case SMESH::Entity_Edge:
    title = tr( "SMESH_EDGE" ); break;
  case SMESH::Entity_Quad_Edge:
    title = tr( "SMESH_QUADRATIC_EDGE" ); break;
  case SMESH::Entity_Triangle:
    title = tr( "SMESH_TRIANGLE" ); break;
  case SMESH::Entity_Quad_Triangle:
    title = tr( "SMESH_QUADRATIC_TRIANGLE" ); break;
  case SMESH::Entity_BiQuad_Triangle:
    title = tr( "SMESH_BIQUADRATIC_TRIANGLE" ); break;
  case SMESH::Entity_Quadrangle:
    title = tr( "SMESH_QUADRANGLE" ); break;
  case SMESH::Entity_Quad_Quadrangle:
    title = tr( "SMESH_QUADRATIC_QUADRANGLE" ); break;
  case SMESH::Entity_BiQuad_Quadrangle:
    title = tr( "SMESH_BIQUADRATIC_QUADRANGLE" ); break;
  case SMESH::Entity_Polygon:
    title = tr( "SMESH_POLYGON" ); break;
  case SMESH::Entity_Quad_Polygon:
    title = tr( "SMESH_QUADRATIC_POLYGON" ); break;
  case SMESH::Entity_Tetra:
    title = tr( "SMESH_TETRAHEDRON" ); break;
  case SMESH::Entity_Quad_Tetra:
    title = tr( "SMESH_QUADRATIC_TETRAHEDRON" ); break;
  case SMESH::Entity_Pyramid:
    title = tr( "SMESH_PYRAMID" ); break;
  case SMESH::Entity_Quad_Pyramid:
    title = tr( "SMESH_QUADRATIC_PYRAMID" ); break;
  case SMESH::Entity_Hexa:
    title = tr( "SMESH_HEXAHEDRON" ); break;
  case SMESH::Entity_Quad_Hexa:
    title = tr( "SMESH_QUADRATIC_HEXAHEDRON" ); break;
  case SMESH::Entity_TriQuad_Hexa:
    title = tr( "SMESH_TRIQUADRATIC_HEXAHEDRON" ); break;
  case SMESH::Entity_Penta:
    title = tr( "SMESH_PENTA" ); break;
  case SMESH::Entity_Quad_Penta:
    title = tr( "SMESH_QUADRATIC_PENTAHEDRON" ); break;
  case SMESH::Entity_BiQuad_Penta:
    title = tr( "SMESH_BIQUADRATIC_PENTAHEDRON" ); break;
  case SMESH::Entity_Hexagonal_Prism:
    title = tr( "SMESH_HEX_PRISM" ); break;
  case SMESH::Entity_Polyhedra:
    title = tr( "SMESH_POLYEDRON" ); break;
  case SMESH::Entity_Quad_Polyhedra:
    title = tr( "SMESH_QUADRATIC_POLYEDRON" ); break;
  case SMESH::Entity_Ball:
    title = tr( "SMESH_BALL" ); break;
  default:
    break;
  }
  return title;
}

/*!
  \brief Get title for given quality control.
  \param type Mesh control type.
  \return Quality control's title.
*/
QString SMESHGUI_ElemInfo::ctrl2str( int control )
{
  QString title;
  switch ( control )
  {
  case SMESH::FT_AspectRatio:
    title = tr( "ASPECTRATIO_ELEMENTS" ); break;
  case SMESH::FT_AspectRatio3D:
    title = tr( "ASPECTRATIO_3D_ELEMENTS" ); break;
  case SMESH::FT_Warping:
    title = tr( "WARP_ELEMENTS" ); break;
  case SMESH::FT_MinimumAngle:
    title = tr( "MINIMUMANGLE_ELEMENTS" ); break;
  case SMESH::FT_Taper:
    title = tr( "TAPER_ELEMENTS" ); break;
  case SMESH::FT_Skew:
    title = tr( "SKEW_ELEMENTS" ); break;
  case SMESH::FT_Area:
    title = tr( "AREA_ELEMENTS" ); break;
  case SMESH::FT_Volume3D:
    title = tr( "VOLUME_3D_ELEMENTS" ); break;
  case SMESH::FT_MaxElementLength2D:
    title = tr( "MAX_ELEMENT_LENGTH_2D" ); break;
  case SMESH::FT_MaxElementLength3D:
    title = tr( "MAX_ELEMENT_LENGTH_3D" ); break;
  case SMESH::FT_Length:
    title = tr( "LENGTH_EDGES" ); break;
  case SMESH::FT_Length2D:
  case SMESH::FT_Length3D:
    title = tr( "MIN_ELEM_EDGE" ); break;
  case SMESH::FT_BallDiameter:
    title = tr( "BALL_DIAMETER" ); break;
  default:
    break;
  }
  return title;
}

/*!
  \brief Write information on given mesh nodes / elements.
  \param writer Information writer.
  \param ids Nodes / elements IDs.
*/
void SMESHGUI_ElemInfo::writeInfo( InfoWriter* writer, const QList<uint>& ids )
{
  if ( !proxy() )
    return;

  bool grp_details = SMESHGUI::resourceMgr()->booleanValue( "SMESH", "elem_info_grp_details", false );
  int cprecision = SMESHGUI::resourceMgr()->booleanValue( "SMESH", "use_precision", false ) ? 
    SMESHGUI::resourceMgr()->integerValue( "SMESH", "controls_precision", -1 ) : -1;

  SMESH::XYZ xyz;
  SMESH::Connectivity connectivity;
  SMESH::Position position;
  bool ok;

  foreach ( uint id, ids )
  {
    writer->separator();

    if ( what() == ShowNodes )
    {
      // show node info
      // - check that node exists
      if ( !proxy().hasNode( id ) )
        continue;
      // - id
      writer->write( tr( "NODE" ), (int)id, true );
      writer->indent();
      // - coordinates
      ok = proxy().nodeCoordinates( id, xyz );
      if ( ok )
      {
        writer->write( tr( "COORDINATES" ), xyz );
      }
      // - connectivity
      ok = proxy().nodeConnectivity( id, connectivity );
      if ( ok )
      {
        if ( !connectivity.isEmpty() )
        {
          writer->write( tr( "CONNECTIVITY" ) );
          writer->indent();
          for ( int i = SMDSAbs_Edge; i <= SMDSAbs_Ball; i++ )
          {
            QString formatted = formatConnectivity( connectivity, i );
            if ( !formatted.isEmpty() )
              writer->write( type2str( i, true ), formatted );
          }
          writer->unindent();
        }
        else
        {
          writer->write( tr( "CONNECTIVITY" ), tr( "FREE_NODE" ) );
        }
      }
      // - position
      ok = proxy().nodePosition( id, position );
      if ( ok && position.isValid() )
      {
        writer->write( tr( "POSITION" ), (stype2str( position.shapeType() ) + " #%1").arg( position.shapeId() ) );
        writer->indent();
        if ( position.hasU() )
          writer->write( tr("U_POSITION"), position.u() );
        if ( position.hasV() )
          writer->write( tr("V_POSITION"), position.v() );
        writer->unindent();
      }
      // - groups node belongs to
      QList<SMESH::SelectionProxy> groups = proxy().nodeGroups( id );
      bool topCreated = false;
      foreach( SMESH::SelectionProxy group, groups )
      {
        if ( group && !group.name().isEmpty() )
        {
          if ( !topCreated )
          {
            writer->write( SMESHGUI_AddInfo::tr( "GROUPS" ) );
            writer->indent();
            topCreated = true;
          }
          writer->write( group.name().trimmed() ); // trim name
          if ( grp_details )
          {
            writer->indent();
            int type = group.type();
            if ( type == SMESH::SelectionProxy::GroupStd )
            {
              writer->write( SMESHGUI_AddInfo::tr( "TYPE" ), SMESHGUI_AddInfo::tr( "STANDALONE_GROUP" ) );
            }
            else if ( type == SMESH::SelectionProxy::GroupGeom )
            {
              writer->write( SMESHGUI_AddInfo::tr( "TYPE" ), SMESHGUI_AddInfo::tr( "GROUP_ON_GEOMETRY" ) );
              writer->write( SMESHGUI_AddInfo::tr( "GEOM_OBJECT" ), group.shapeName() );
            }
            else if ( type == SMESH::SelectionProxy::GroupFilter )
            {
              writer->write( SMESHGUI_AddInfo::tr( "TYPE" ), SMESHGUI_AddInfo::tr( "GROUP_ON_FILTER" ) );
            }
            int size = group.size();
            if ( size != -1 )
              writer->write( SMESHGUI_AddInfo::tr( "SIZE" ), size );
            QColor color = group.color();
            if ( color.isValid() )
              writer->write( SMESHGUI_AddInfo::tr( "COLOR" ), color.name() );
            writer->unindent();
          }
        }
      }
      if ( topCreated )
        writer->unindent();
      writer->unindent();
    }
    else if ( what() == ShowElements )
    {
      // show element info
      // - check that element exists
      if ( !proxy().hasElement( id ) )
        continue;
      // - id & type
      int type = proxy().elementType( id );
      if  ( type == SMESH::ALL )
        continue;
      writer->write( type2str( type ), (int)id, true );
      writer->indent();
      // - geometry type
      type = proxy().elementEntityType( id );
      writer->write( tr( "TYPE" ), etype2str( type ) );
      // - connectivity
      if ( type == SMESH::Entity_Polyhedra ||
           type == SMESH::Entity_Quad_Polyhedra )
      {
        int nbNodes;
        ok = proxy().perFaceConnectivity( id, connectivity, nbNodes );
        if ( ok && !connectivity.isEmpty() )
        {
          writer->write( tr( "NB_NODES" ), nbNodes );
          writer->write( tr( "CONNECTIVITY" ) );
          writer->indent();
          int nbFaces = connectivity.size();
          for ( int iF = 1; iF <= nbFaces; ++iF )
          {
            QString formatted = formatConnectivity( connectivity, -iF );
            writer->write(( type2str( SMDSAbs_Face, 0 ) + " %1 / %2" ).arg( iF ).arg( nbFaces ),
                          formatted );
          }
          writer->unindent();
        }
      }
      else
      {
        ok = proxy().elementConnectivity( id, connectivity );
        if ( ok && !connectivity.isEmpty() )
        {
          QString formatted = formatConnectivity( connectivity, SMDSAbs_Node );
          if ( !formatted.isEmpty() )
          {
            writer->write( tr( "NB_NODES" ), connectivity[ SMDSAbs_Node ].size() );
            writer->write( tr( "CONNECTIVITY" ), formatted ); //todo: TypeRole: ElemConnectivity
          }
        }
      }
      // - position
      ok = proxy().elementPosition( id, position );
      if ( ok && position.isValid() )
      {
        writer->write( tr( "POSITION" ), (stype2str( position.shapeType() ) + " #%1").arg( position.shapeId() ) );
      }
      // - gravity center
      ok = proxy().elementGravityCenter( id, xyz );
      if ( ok )
      {
        writer->write( tr( "GRAVITY_CENTER" ), xyz );
      }
      // - normal vector
      ok = proxy().elementNormal( id, xyz );
      if ( ok )
      {
        writer->write( tr( "NORMAL_VECTOR" ), xyz );
      }
      // - controls
      bool topCreated = false;
      for ( int i = SMESH::FT_AspectRatio; i < SMESH::FT_Undefined; i++ )
      {
        QString ctrlTitle = ctrl2str( i );
        if ( ctrlTitle.isEmpty() )
          continue;
        if ( !topCreated )
        {
          writer->write( tr( "CONTROLS" ) );
          writer->indent();
          topCreated = true;
        }
        double value;
        if ( proxy().elementControl( id, i, cprecision, value ) )
          writer->write( ctrlTitle, value );
      }
      if ( topCreated )
        writer->unindent();
      // - groups element belongs to
      QList<SMESH::SelectionProxy> groups = proxy().elementGroups( id );
      topCreated = false;
      foreach( SMESH::SelectionProxy group, groups )
      {
        if ( group && !group.name().isEmpty() )
        {
          if ( !topCreated )
          {
            writer->write( SMESHGUI_AddInfo::tr( "GROUPS" ) );
            writer->indent();
            topCreated = true;
          }
          writer->write( group.name().trimmed() ); // trim name
          if ( grp_details )
          {
            writer->indent();
            int type = group.type();
            if ( type == SMESH::SelectionProxy::GroupStd )
            {
              writer->write( SMESHGUI_AddInfo::tr( "TYPE" ), SMESHGUI_AddInfo::tr( "STANDALONE_GROUP" ) );
            }
            else if ( type == SMESH::SelectionProxy::GroupGeom )
            {
              writer->write( SMESHGUI_AddInfo::tr( "TYPE" ), SMESHGUI_AddInfo::tr( "GROUP_ON_GEOMETRY" ) );
              writer->write( SMESHGUI_AddInfo::tr( "GEOM_OBJECT" ), group.shapeName() );
            }
            else if ( type == SMESH::SelectionProxy::GroupFilter )
            {
              writer->write( SMESHGUI_AddInfo::tr( "TYPE" ), SMESHGUI_AddInfo::tr( "GROUP_ON_FILTER" ) );
            }
            int size = group.size();
            if ( size != -1 )
              writer->write( SMESHGUI_AddInfo::tr( "SIZE" ), size );
            QColor color = group.color();
            if ( color.isValid() )
              writer->write( SMESHGUI_AddInfo::tr( "COLOR" ), color.name() );
            writer->unindent();
          }
        }
      }
      if ( topCreated )
        writer->unindent();
      writer->unindent();
    }
  }  
}

/*!
  \fn void SMESHGUI_ElemInfo::information( const QList<uint>& ids )
  \brief Show information on given mesh nodes / elements.

  This function has to be redefined in sub-classes.

  \param ids Nodes / elements IDs.
*/

/*!
  \brief Internal clean-up (reset panel).

  Default implementation does nothing; the method has to be redefined
  in sub-classes to perform internal clean-up.
*/
void SMESHGUI_ElemInfo::clearInternal()
{
}

/*!
  \brief Show previous chunk of information.
*/
void SMESHGUI_ElemInfo::showPrevious()
{
  myIndex = qMax( 0, myIndex-1 );
  updateControls();
  information( myIDs.mid( myIndex*blockSize(), blockSize() ) );
}

/*!
  \brief Show next chunk of information.
*/
void SMESHGUI_ElemInfo::showNext()
{
  myIndex = qMin( myIndex+1, myIDs.count() / blockSize() );
  updateControls();
  information( myIDs.mid( myIndex*blockSize(), blockSize() ) );
}

/*!
  \brief Update control widget state.
*/
void SMESHGUI_ElemInfo::updateControls()
{
  myExtra->updateControls( myIDs.count(), myIndex );
}

/*!
  \brief Write information from panel to output stream.
  \param out Text stream output.
*/
void SMESHGUI_ElemInfo::saveInfo( QTextStream &out )
{
  // title
  QString title = tr( "ELEM_INFO" );
  out << ruler( title.size() ) << endl;
  out << title << endl;
  out << ruler( title.size() ) << endl;
  //  out << endl;

  // info
  StreamWriter writer( out );
  writeInfo( &writer, myIDs );
  out << endl;
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_SimpleElemInfo
/// \brief Show mesh element information in the simple text area.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
  \param parent Parent widget. Defaults to 0.
*/
SMESHGUI_SimpleElemInfo::SMESHGUI_SimpleElemInfo( QWidget* parent )
  : SMESHGUI_ElemInfo( parent )
{
  myInfo = new QTextBrowser( centralWidget() );
  QVBoxLayout* l = new QVBoxLayout( centralWidget() );
  l->setMargin( 0 );
  l->addWidget( myInfo );

  connect( myInfo, SIGNAL( anchorClicked(QUrl)), this, SLOT( connectivityClicked( QUrl )));
}

/*!
  \brief Show mesh element information.
  \param ids Nodes / elements IDs.
*/
void SMESHGUI_SimpleElemInfo::information( const QList<uint>& ids )
{
  clearInternal();
  TextWriter writer( myInfo );
  writeInfo( &writer, ids );
}

/*!
  \brief Internal clean-up (reset widget)
*/
void SMESHGUI_SimpleElemInfo::clearInternal()
{
  myInfo->clear();
}

void SMESHGUI_SimpleElemInfo::connectivityClicked(const QUrl & url)
{
  int type = ( url.scheme()[0] == 'n' ) ? NodeConnectivity : ElemConnectivity;
  QString ids = url.path(); // excess chars will be filtered off by SMESHGUI_IdValidator
  emit( itemInfo( type, ids ));
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_TreeElemInfo::ItemDelegate
/// \brief Item delegate for tree mesh info widget.
/// \internal
////////////////////////////////////////////////////////////////////////////////

class SMESHGUI_TreeElemInfo::ItemDelegate : public QItemDelegate
{
public:
  ItemDelegate( QObject* );
  QWidget* createEditor( QWidget*, const QStyleOptionViewItem&, const QModelIndex& ) const;
};

/*!
  \brief Constructor.
  \internal
*/
SMESHGUI_TreeElemInfo::ItemDelegate::ItemDelegate( QObject* parent ): QItemDelegate( parent )
{
}

/*!
  \brief Redefined from QItemDelegate.
  \internal
*/
QWidget* SMESHGUI_TreeElemInfo::ItemDelegate::createEditor( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  QWidget* w = index.column() == 0 ? 0: QItemDelegate::createEditor( parent, option, index );
  if ( qobject_cast<QLineEdit*>( w ) )
    qobject_cast<QLineEdit*>( w )->setReadOnly( true );
  return w;
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_TreeElemInfo::ItemCreator
/// \brief Item creator for tree mesh info widget.
/// \internal
////////////////////////////////////////////////////////////////////////////////

class SMESHGUI_TreeElemInfo::ItemCreator : public TreeItemCreator
{
  SMESHGUI_TreeElemInfo* myView;
public:
  ItemCreator( SMESHGUI_TreeElemInfo* );
  QTreeWidgetItem* createItem( QTreeWidgetItem*, int );
};

/*!
  \brief Constructor.
  \param view Parent view.
  \internal
*/
SMESHGUI_TreeElemInfo::ItemCreator::ItemCreator( SMESHGUI_TreeElemInfo* view ): TreeItemCreator(), myView( view )
{
}

/*!
  \brief Create new tree item.
  \param parent Parent tree item.
  \param options Item options.
  \return New tree widget item.
  \internal
*/
QTreeWidgetItem* SMESHGUI_TreeElemInfo::ItemCreator::createItem( QTreeWidgetItem* parent, int options )
{
  return myView->createItem( parent, options );
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_TreeElemInfo
/// \brief Show mesh element information as the tree.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
  \param parent Parent widget. Defaults to 0.
*/
SMESHGUI_TreeElemInfo::SMESHGUI_TreeElemInfo( QWidget* parent )
  : SMESHGUI_ElemInfo( parent )
{
  myInfo = new QTreeWidget( centralWidget() );
  myInfo->setColumnCount( 2 );
  myInfo->setHeaderLabels( QStringList() << tr( "PROPERTY" ) << tr( "VALUE" ) );
  myInfo->header()->setStretchLastSection( true );
  myInfo->header()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
  myInfo->setItemDelegate( new ItemDelegate( myInfo ) );
  QVBoxLayout* l = new QVBoxLayout( centralWidget() );
  l->setMargin( 0 );
  l->addWidget( myInfo );
  connect( myInfo, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ), this, SLOT( itemDoubleClicked( QTreeWidgetItem*, int ) ) );
  connect( myInfo, SIGNAL( itemCollapsed( QTreeWidgetItem* )), this, SLOT( saveExpanded( QTreeWidgetItem* )) );
  connect( myInfo, SIGNAL( itemExpanded( QTreeWidgetItem* )), this, SLOT( saveExpanded( QTreeWidgetItem* )) );
}

/*!
  \brief Show mesh element information.
  \param ids Nodes / elements IDs.
*/
void SMESHGUI_TreeElemInfo::information( const QList<uint>& ids )
{
  clearInternal();
  TreeWriter writer( myInfo, new ItemCreator( this ) );
  writeInfo( &writer, ids );
}

/*!
  \brief Show node information
  \param node mesh node for showing
  \param index index of current node
  \param nbNodes number of unique nodes in element
  \param parentItem parent item of tree
*/
void SMESHGUI_TreeElemInfo::nodeInfo( const SMDS_MeshNode* /*node*/, int /*index*/,
                                      int /*nbNodes*/, QTreeWidgetItem* /*parentItem*/ )
{
  // int precision   = SMESHGUI::resourceMgr()->integerValue( "SMESH", "length_precision", 6 );
  // // node number and ID
  // QTreeWidgetItem* nodeItem = createItem( parentItem, Bold );
  // nodeItem->setText( 0, QString( "%1 %2 / %3" ).arg( SMESHGUI_ElemInfo::tr( "NODE" )).arg( index ).arg( nbNodes ));
  // nodeItem->setText( 1, QString( "#%1" ).arg( node->GetID() ));
  // nodeItem->setData( 1, TypeRole, ElemConnectivity );
  // nodeItem->setData( 1, IdRole, node->GetID() );
  // nodeItem->setExpanded( false );
  // // node coordinates
  // QTreeWidgetItem* coordItem = createItem( nodeItem );
  // coordItem->setText( 0, SMESHGUI_ElemInfo::tr( "COORDINATES" ));
  // QTreeWidgetItem* xItem = createItem( coordItem );
  // xItem->setText( 0, "X" );
  // xItem->setText( 1, QString::number( node->X(), precision > 0 ? 'f' : 'g', qAbs( precision )) );
  // QTreeWidgetItem* yItem = createItem( coordItem );
  // yItem->setText( 0, "Y" );
  // yItem->setText( 1, QString::number( node->Y(), precision > 0 ? 'f' : 'g', qAbs( precision )) );
  // QTreeWidgetItem* zItem = createItem( coordItem );
  // zItem->setText( 0, "Z" );
  // zItem->setText( 1, QString::number( node->Z(), precision > 0 ? 'f' : 'g', qAbs( precision )) );
  // // node connectivity
  // QTreeWidgetItem* nconItem = createItem( nodeItem );
  // nconItem->setText( 0, SMESHGUI_ElemInfo::tr( "CONNECTIVITY" ));
  // Connectivity connectivity = nodeConnectivity( node );
  // if ( !connectivity.isEmpty() ) {
  //   QString con = formatConnectivity( connectivity, SMDSAbs_0DElement );
  //   if ( !con.isEmpty() ) {
  //     QTreeWidgetItem* i = createItem( nconItem );
  //     i->setText( 0, SMESHGUI_ElemInfo::tr( "0D_ELEMENTS" ));
  //     i->setText( 1, con );
  //   }
  //   con = formatConnectivity( connectivity, SMDSAbs_Edge );
  //   if ( !con.isEmpty() ) {
  //     QTreeWidgetItem* i = createItem( nconItem );
  //     i->setText( 0, SMESHGUI_ElemInfo::tr( "EDGES" ));
  //     i->setText( 1, con );
  //     i->setData( 1, TypeRole, NodeConnectivity );
  //   }
  //   con = formatConnectivity( connectivity, SMDSAbs_Ball );
  //   if ( !con.isEmpty() ) {
  //     QTreeWidgetItem* i = createItem( nconItem );
  //     i->setText( 0, SMESHGUI_ElemInfo::tr( "BALL_ELEMENTS" ));
  //     i->setText( 1, con );
  //     i->setData( 1, TypeRole, NodeConnectivity );
  //   }
  //   con = formatConnectivity( connectivity, SMDSAbs_Face );
  //   if ( !con.isEmpty() ) {
  //     QTreeWidgetItem* i = createItem( nconItem );
  //     i->setText( 0, SMESHGUI_ElemInfo::tr( "FACES" ));
  //     i->setText( 1, con );
  //     i->setData( 1, TypeRole, NodeConnectivity );
  //   }
  //   con = formatConnectivity( connectivity, SMDSAbs_Volume );
  //   if ( !con.isEmpty() ) {
  //     QTreeWidgetItem* i = createItem( nconItem );
  //     i->setText( 0, SMESHGUI_ElemInfo::tr( "VOLUMES" ));
  //     i->setText( 1, con );
  //     i->setData( 1, TypeRole, NodeConnectivity );
  //   }
  // }
}
/*!
  \brief Internal clean-up (reset widget)
*/
void SMESHGUI_TreeElemInfo::clearInternal()
{
  myInfo->clear();
  myInfo->repaint();
}

/*!
  \brief Create new item and add it to the tree.
  \param parent Parent tree widget item. Defaults to 0 (top-level item).
  \param options Item flags. Defaults to 0 (none).
  \return New tree widget item.
*/
QTreeWidgetItem* SMESHGUI_TreeElemInfo::createItem( QTreeWidgetItem* parent, int options )
{
  QTreeWidgetItem* item = new QTreeWidgetItem( parent ? parent : myInfo->invisibleRootItem() );
  setTreeItemAttributes( item, options | Expanded | Editable );

  if ( parent && parent->childCount() == 1 && itemDepth( parent ) == 1 )
  {
    QString resName = expandedResource( parent );
    parent->setExpanded( SMESHGUI::resourceMgr()->booleanValue("SMESH", resName, true ));
  }
  
  return item;
}

void SMESHGUI_TreeElemInfo::contextMenuEvent( QContextMenuEvent* e )
{
  QList< QTreeWidgetItem* > widgets = myInfo->selectedItems();
  if ( widgets.isEmpty() ) return;
  QTreeWidgetItem* aTreeItem = widgets.first();
  int type = aTreeItem->data( 1, TypeRole ).toInt();
  if (( type == ElemConnectivity || type == NodeConnectivity ) &&
      ( !aTreeItem->text( 1 ).isEmpty() ))
  {
    QMenu menu;
    QAction* a = menu.addAction( tr( "SHOW_ITEM_INFO" ));
    if ( menu.exec( e->globalPos() ) == a )
      emit( itemInfo( type, aTreeItem->text( 1 )) );
  }
}

void SMESHGUI_TreeElemInfo::itemDoubleClicked( QTreeWidgetItem* theItem, int /*theColumn*/ )
{
  if ( theItem ) {
    int type = theItem->data( 1, TypeRole ).toInt();
    emit( itemInfo( type, theItem->text( 1 )) );
  }
}

void SMESHGUI_TreeElemInfo::saveExpanded( QTreeWidgetItem* theItem )
{
  if ( theItem )
    SMESHGUI::resourceMgr()->setValue("SMESH", expandedResource( theItem ), theItem->isExpanded() );
}

QString SMESHGUI_TreeElemInfo::expandedResource( QTreeWidgetItem* theItem )
{
  return QString("Expanded_") + ( what()==ShowElements ? "E_" : "N_" ) + theItem->text(0);
}

////////////////////////////////////////////////////////////////////////////////
/// \class InfoComputor
/// \brief Mesh information computor.
/// \internal
///
/// The class is created for different computation operations. Currently it is
/// used to compute size and number of underlying nodes for given group.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
  \param parent Parent object.
  \param proxy Object to compute information on (group).
  \param item Tree widget item, referenced by this computer.
  \param operation Value to compute.
  \internal
*/
InfoComputor::InfoComputor( QObject* parent, const SMESH::SelectionProxy& proxy, int operation )
  : QObject( parent ), myProxy( proxy ), myOperation( operation )
{
}

/*!
  \brief Compute requested information.
  \internal
*/
void InfoComputor::compute()
{
  if ( myProxy )
  {
    SUIT_OverrideCursor wc;
    myProxy.load();
    switch ( myOperation )
    {
    case GrpSize:
      myProxy.size( true ); // force size computation
      emit computed();
      break;
    case GrpNbNodes:
      myProxy.nbNodes( true ); // force size computation
      emit computed();
      break;
    default:
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_AddInfo
/// \brief Show additional information on selected object.
///
/// Displays an additional information about selected object: mesh, sub-mesh
/// or group.
///
/// \todo Rewrite saveInfo() method to print all data, not currently shown only.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
  \param parent Parent widget. Defaults to 0.
*/
SMESHGUI_AddInfo::SMESHGUI_AddInfo( QWidget* parent ): SMESHGUI_Info( parent )
{
  QVBoxLayout* l = new QVBoxLayout( this );
  l->setMargin( 0 );
  l->setSpacing( SPACING );

  myTree = new QTreeWidget( this );

  myTree->setColumnCount( 2 );
  myTree->header()->setStretchLastSection( true );
  myTree->header()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
  myTree->header()->hide();

  l->addWidget( myTree );
}

/*!
  \brief Destructor.
*/
SMESHGUI_AddInfo::~SMESHGUI_AddInfo()
{
}

/*!
  \brief Show information on given object.
  \param proxy Object to show information on (mesh, sub-mesh, group).
*/
void SMESHGUI_AddInfo::showInfo( const SMESH::SelectionProxy& proxy )
{
  // reset panel
  setProperty( "group_index", 0 );
  setProperty( "submesh_index",  0 );
  myComputors.clear();
  myTree->clear();

  // then fill panel with data if object is not null
  if ( proxy )
  {
    myProxy = proxy;

    // name
    QTreeWidgetItem* nameItem = createItem( 0, Bold | AllColumns );
    nameItem->setText( 0, tr( "NAME" ) );
    nameItem->setText( 1, proxy.name() );

    // object info
    if ( proxy.type() == SMESH::SelectionProxy::Mesh )
      meshInfo( proxy, nameItem );
    else if ( proxy.type() == SMESH::SelectionProxy::Submesh )
      subMeshInfo( proxy, nameItem );
    else if ( proxy.type() >= SMESH::SelectionProxy::Group )
      groupInfo( proxy, nameItem );
  }
}

/*!
  \brief Update information in panel.
*/
void SMESHGUI_AddInfo::updateInfo()
{
  showInfo( myProxy );
}

/*!
  \brief Reset panel (clear all data).
*/
void SMESHGUI_AddInfo::clear()
{
  myTree->clear();
}

/*!
  \brief Create new item and add it to the tree.
  \param parent Parent tree widget item. Defaults to 0 (top-level item).
  \param options Item flags. Defaults to 0 (none).
  \return New tree widget item.
*/
QTreeWidgetItem* SMESHGUI_AddInfo::createItem( QTreeWidgetItem* parent, int options )
{
  QTreeWidgetItem* item = parent ? new QTreeWidgetItem( parent ) : 
    new QTreeWidgetItem( myTree->invisibleRootItem() );
  setTreeItemAttributes( item, options | Expanded );
  return item;
}

/*!
  \brief Show information on mesh.
  \param proxy Proxy object (mesh).
  \param parent Parent tree item.
*/
void SMESHGUI_AddInfo::meshInfo( const SMESH::SelectionProxy& proxy, QTreeWidgetItem* parent )
{
  if ( !proxy )
    return;

  QString shapeName = proxy.shapeName();
  SMESH::MedInfo inf = proxy.medFileInfo();

  // type
  QTreeWidgetItem* typeItem = createItem( parent, Bold );
  typeItem->setText( 0, tr( "TYPE" ) );
  if ( !shapeName.isEmpty() )
  {
    typeItem->setText( 1, tr( "MESH_ON_GEOMETRY" ) );
    // shape
    QTreeWidgetItem* gobjItem = createItem( parent, Bold );
    gobjItem->setText( 0, tr( "GEOM_OBJECT" ) );
    gobjItem->setText( 1, shapeName );
  }
  else if ( inf.isValid() )
  {
    typeItem->setText( 1, tr( "MESH_FROM_FILE" ) );
    // med file information
    QTreeWidgetItem* fileItem = createItem( parent, Bold );
    fileItem->setText( 0, tr( "FILE_NAME" ) );
    fileItem->setText( 1, inf.fileName() );
    QTreeWidgetItem* sizeItem = createItem( parent, Bold );
    sizeItem->setText( 0, tr( "FILE_SIZE" ) );
    sizeItem->setText( 1, QString::number( inf.size() ) );
    QTreeWidgetItem* versionItem = createItem( parent, Bold );
    versionItem->setText( 0, tr( "FILE_VERSION" ) );
    versionItem->setText( 1, inf.version() != "0" ? inf.version() : tr( "VERSION_UNKNOWN" ) );
  }
  else
  {
    typeItem->setText( 1, tr( "STANDALONE_MESH" ) );
  }
  
  // groups
  myGroups = proxy.groups();
  showGroups();

  // sub-meshes
  mySubMeshes = proxy.submeshes();
  showSubMeshes();
}

/*!
  \brief Show information on sub-mesh.
  \param proxy Proxy object (sub-mesh).
  \param parent Parent tree item.
*/
void SMESHGUI_AddInfo::subMeshInfo( const SMESH::SelectionProxy& proxy, QTreeWidgetItem* parent )
{
  if ( !proxy )
    return;

  bool isShort = parent->parent() != 0;

  if ( !isShort )
  {
    // parent mesh
    SMESH::SelectionProxy meshProxy = proxy.mesh();
    if ( meshProxy )
    {
      QTreeWidgetItem* nameItem = createItem( parent, Bold );
      nameItem->setText( 0, tr( "PARENT_MESH" ) );
      nameItem->setText( 1, meshProxy.name() );
    }
  }
  
  // shape
  QString shapeName = proxy.shapeName();
  if ( !shapeName.isEmpty() )
  {
    QTreeWidgetItem* gobjItem = createItem( parent, Bold );
    gobjItem->setText( 1, shapeName );
  }
}

/*!
  \brief Show information on group.
  \param proxy Proxy object (group).
  \param parent Parent tree item.
*/
void SMESHGUI_AddInfo::groupInfo( const SMESH::SelectionProxy& proxy, QTreeWidgetItem* parent )
{
  if ( !proxy )
    return;

  bool isShort = parent->parent() != 0;

  if ( !isShort )
  {
    // parent mesh
    SMESH::SelectionProxy meshProxy = proxy.mesh();
    if ( meshProxy )
    {
      QTreeWidgetItem* nameItem = createItem( parent, Bold );
      nameItem->setText( 0, tr( "PARENT_MESH" ) );
      nameItem->setText( 1, meshProxy.name() );
    }
  }

  // type
  SMESH::SelectionProxy::Type type = proxy.type();
  QTreeWidgetItem* typeItem = createItem( parent, Bold );
  typeItem->setText( 0, tr( "TYPE" ) );
  if ( type == SMESH::SelectionProxy::GroupStd )
  {
    typeItem->setText( 1, tr( "STANDALONE_GROUP" ) );
  }
  else if ( type == SMESH::SelectionProxy::GroupGeom )
  {
    typeItem->setText( 1, tr( "GROUP_ON_GEOMETRY" ) );
    // shape
    QTreeWidgetItem* gobjItem = createItem( parent, Bold );
    gobjItem->setText( 0, tr( "GEOM_OBJECT" ) );
    gobjItem->setText( 1, proxy.shapeName() );
  }
  else if ( type == SMESH::SelectionProxy::GroupFilter )
  {
    typeItem->setText( 1, tr( "GROUP_ON_FILTER" ) );
  }

  // element type
  int etype = proxy.groupElementType();
  if ( !isShort )
  {
    QString typeName = tr( "UNKNOWN" );
    switch( etype )
    {
    case SMESH::NODE:
      typeName = tr( "NODE" );
      break;
    case SMESH::EDGE:
      typeName = tr( "EDGE" );
      break;
    case SMESH::FACE:
      typeName = tr( "FACE" );
      break;
    case SMESH::VOLUME:
      typeName = tr( "VOLUME" );
      break;
    case SMESH::ELEM0D:
      typeName = tr( "0DELEM" );
      break;
    case SMESH::BALL:
      typeName = tr( "BALL" );
      break;
    default:
      break;
    }
    QTreeWidgetItem* etypeItem = createItem( parent, Bold );
    etypeItem->setText( 0, tr( "ENTITY_TYPE" ) );
    etypeItem->setText( 1, typeName );
  }

  // size
  // note: size is not computed for group on filter for performance reasons, see IPAL52831
  bool meshLoaded = proxy.isMeshLoaded();
  int size = proxy.size();

  QTreeWidgetItem* sizeItem = createItem( parent, Bold );
  sizeItem->setText( 0, tr( "SIZE" ) );
  if ( size >= 0 )
  {
    sizeItem->setText( 1, QString::number( size ) );
  }
  else
  {
    QPushButton* btn = new QPushButton( meshLoaded ? tr( "COMPUTE" ) : tr( "LOAD" ), this );
    myTree->setItemWidget( sizeItem, 1, btn );
    InfoComputor* comp = new InfoComputor( this, proxy, InfoComputor::GrpSize );
    connect( btn, SIGNAL( clicked() ), comp, SLOT( compute() ) );
    connect( comp, SIGNAL( computed() ), this, SLOT( updateInfo() ) );
    myComputors.append( comp );
  }

  // color
  QTreeWidgetItem* colorItem = createItem( parent, Bold );
  colorItem->setText( 0, tr( "COLOR" ) );
  colorItem->setBackground( 1, proxy.color() );

  // nb of underlying nodes
  if ( etype != SMESH::NODE )
  {
    QTreeWidgetItem* nodesItem = createItem( parent, Bold );
    nodesItem->setText( 0, tr( "NB_NODES" ) );

    int nbNodes = proxy.nbNodes();
    if ( nbNodes >= 0 )
    {
      nodesItem->setText( 1, QString::number( nbNodes ) );
    }
    else
    {
      QPushButton* btn = new QPushButton( meshLoaded ? tr( "COMPUTE" ) : tr( "LOAD" ), this );
      myTree->setItemWidget( nodesItem, 1, btn );
      InfoComputor* comp = new InfoComputor( this, proxy, InfoComputor::GrpNbNodes ); 
      connect( btn, SIGNAL( clicked() ), comp, SLOT( compute() ) );
      connect( comp, SIGNAL( computed() ), this, SLOT( updateInfo() ) );
      myComputors.append( comp );
    }
  }
}

/*!
  \brief Update information on child groups.
*/
void SMESHGUI_AddInfo::showGroups()
{
  // remove all computors
  myComputors.clear();

  // tree root should be the first top level item
  QTreeWidgetItem* parent = myTree->topLevelItemCount() > 0 ? myTree->topLevelItem( 0 ) : 0;
  if ( !parent )
    return;

  int idx = property( "group_index" ).toInt();

  // find sub-meshes top-level container item
  QTreeWidgetItem* itemGroups = 0;
  for ( int i = 0; i < parent->childCount() && !itemGroups; i++ )
  {
    if ( parent->child( i )->data( 0, Qt::UserRole ).toInt() == GroupsId )
    {
      itemGroups = parent->child( i );
      // update controls
      ExtraWidget* extra = dynamic_cast<ExtraWidget*>( myTree->itemWidget( itemGroups, 1 ) );
      if ( extra )
        extra->updateControls( myGroups.count(), idx );
      // clear: remove all group items
      while ( itemGroups->childCount() )
        delete itemGroups->child( 0 );
    }
  }

  QMap<int, QTreeWidgetItem*> grpItems;
  for ( int i = idx*blockSize() ; i < qMin( (idx+1)*blockSize(), (int)myGroups.count() ); i++ )
  {
    SMESH::SelectionProxy grp = myGroups[i];
    if ( !grp )
      continue;

    int grpType = grp.groupElementType();

    // create top-level groups container item if it does not exist
    if ( !itemGroups )
    {
      itemGroups = createItem( parent, Bold | AllColumns );
      itemGroups->setText( 0, tr( "GROUPS" ) );
      itemGroups->setData( 0, Qt::UserRole, GroupsId );

      // if necessary, create extra widget to show information by chunks
      if ( myGroups.count() > blockSize() )
      {
        ExtraWidget* extra = new ExtraWidget( this, true );
        connect( extra->prev, SIGNAL( clicked() ), this, SLOT( showPreviousGroups() ) );
        connect( extra->next, SIGNAL( clicked() ), this, SLOT( showNextGroups() ) );
        myTree->setItemWidget( itemGroups, 1, extra );
        extra->updateControls( myGroups.count(), idx );
      }
    }

    // create container item corresponding to particular element type
    if ( !grpItems.contains( grpType ) )
    {
      grpItems[ grpType ] = createItem( itemGroups, Bold | AllColumns );
      grpItems[ grpType ]->setText( 0, tr( QString( "GROUPS_%1" ).arg( grpType ).toLatin1().constData() ) );
      itemGroups->insertChild( grpType-1, grpItems[ grpType ] ); // -1 needed since 0 corresponds to SMESH::ALL
    }
  
    // name
    QTreeWidgetItem* nameItem = createItem( grpItems[ grpType ] );
    nameItem->setText( 0, grp.name().trimmed() ); // trim name

    // group info
    groupInfo( grp, nameItem );
  }
}

/*!
  \brief Update information on child sub-meshes.
*/
void SMESHGUI_AddInfo::showSubMeshes()
{
  // tree root should be the first top level item
  QTreeWidgetItem* parent = myTree->topLevelItemCount() > 0 ? myTree->topLevelItem( 0 ) : 0;
  if ( !parent )
    return;

  int idx = property( "submesh_index" ).toInt();

  // find sub-meshes top-level container item
  QTreeWidgetItem* itemSubMeshes = 0;
  for ( int i = 0; i < parent->childCount() && !itemSubMeshes; i++ )
  {
    if ( parent->child( i )->data( 0, Qt::UserRole ).toInt() == SubMeshesId )
    {
      itemSubMeshes = parent->child( i );
      // update controls
      ExtraWidget* extra = dynamic_cast<ExtraWidget*>( myTree->itemWidget( itemSubMeshes, 1 ) );
      if ( extra )
        extra->updateControls( mySubMeshes.count(), idx );
      // clear: remove all sub-mesh items
      while ( itemSubMeshes->childCount() )
        delete itemSubMeshes->child( 0 );
    }
  }

  QMap<int, QTreeWidgetItem*> smItems;
  for ( int i = idx*blockSize() ; i < qMin( (idx+1)*blockSize(), mySubMeshes.count() ); i++ )
  {
    SMESH::SelectionProxy sm = mySubMeshes[i];
    if ( !sm )
      continue;
    
    int smType = sm.shapeType();
    if ( smType < 0 )
      continue;
    else if ( smType == GEOM::COMPSOLID )
      smType = GEOM::COMPOUND;

    // create top-level sub-meshes container item if it does not exist
    if ( !itemSubMeshes )
    {
      itemSubMeshes = createItem( parent, Bold | AllColumns );
      itemSubMeshes->setText( 0, tr( "SUBMESHES" ) );
      itemSubMeshes->setData( 0, Qt::UserRole, SubMeshesId );

      // if necessary, create extra widget to show information by chunks
      if ( mySubMeshes.count() > blockSize() )
      {
        ExtraWidget* extra = new ExtraWidget( this, true );
        connect( extra->prev, SIGNAL( clicked() ), this, SLOT( showPreviousSubMeshes() ) );
        connect( extra->next, SIGNAL( clicked() ), this, SLOT( showNextSubMeshes() ) );
        myTree->setItemWidget( itemSubMeshes, 1, extra );
        extra->updateControls( mySubMeshes.count(), idx );
      }
    }

    // create container item corresponding to particular shape type
    if ( !smItems.contains( smType ) )
    {
      smItems[ smType ] = createItem( itemSubMeshes, Bold | AllColumns );
      smItems[ smType ]->setText( 0, tr( QString( "SUBMESHES_%1" ).arg( smType ).toLatin1().constData() ) );
      itemSubMeshes->insertChild( smType, smItems[ smType ] );
    }
    
    // name
    QTreeWidgetItem* nameItem = createItem( smItems[ smType ] );
    nameItem->setText( 0, sm.name().trimmed() ); // trim name
    
    // submesh info
    subMeshInfo( sm, nameItem );
  }
}

/*!
  \brief Show previous chunk of information on child groups.
*/
void SMESHGUI_AddInfo::showPreviousGroups()
{
  int idx = property( "group_index" ).toInt();
  setProperty( "group_index", idx-1 );
  showGroups();
}

/*!
  \brief Show next chunk of information on child groups.
*/
void SMESHGUI_AddInfo::showNextGroups()
{
  int idx = property( "group_index" ).toInt();
  setProperty( "group_index", idx+1 );
  showGroups();
}

/*!
  \brief Show previous chunk of information on child sub-meshes.
*/
void SMESHGUI_AddInfo::showPreviousSubMeshes()
{
  int idx = property( "submesh_index" ).toInt();
  setProperty( "submesh_index", idx-1 );
  showSubMeshes();
}

/*!
  \brief Show next chunk of information on child sub-meshes.
*/
void SMESHGUI_AddInfo::showNextSubMeshes()
{
  int idx = property( "submesh_index" ).toInt();
  setProperty( "submesh_index", idx+1 );
  showSubMeshes();
}

/*!
  \brief Write information from panel to output stream.
  \param out Text stream output.
*/
void SMESHGUI_AddInfo::saveInfo( QTextStream &out )
{
  // title
  QString title = tr( "ADDITIONAL_INFO" );
  out << ruler( title.size() ) << endl;
  out << title << endl;
  out << ruler( title.size() ) << endl;
  out << endl;

  // info
  QTreeWidgetItemIterator it( myTree );
  while ( *it )
  {
    if ( !( ( *it )->text(0) ).isEmpty() )
    {
      out << indent( itemDepth( *it ) ) << ( *it )->text(0);
      if ( ( *it )->text(0)  == tr( "COLOR" ) )
        out << ":" << spacing() << ( ( ( *it )->background(1) ).color() ).name();
      else if ( !( ( *it )->text(1) ).isEmpty() )
        out << ":" << spacing() << ( *it )->text(1);
      out << endl;
    }
    ++it;
  }
  out << endl;
}

////////////////////////////////////////////////////////////////////////////////
/// \class GroupCombo
/// \brief Customized combo box to manage list of mesh groups.
/// \internal
////////////////////////////////////////////////////////////////////////////////

class GroupCombo: public QComboBox
{
  class Item: public QStandardItem
  {
  public:
    SMESH::SelectionProxy myGroup;
    Item( const SMESH::SelectionProxy& group )
    {
      myGroup = group;
      setText( myGroup.name() );
    }
    SMESH::SelectionProxy group()
    {
      return myGroup;
    }
  };

  SMESH::SelectionProxy myProxy;

public:
  GroupCombo( QWidget* );
  void setSource( const SMESH::SelectionProxy& );
  SMESH::SelectionProxy currentGroup() const;
};

/*!
  \brief Constructor.
  \param parent Parent widget.
  \internal
*/
GroupCombo::GroupCombo( QWidget* parent ): QComboBox( parent )
{
  setModel( new QStandardItemModel( this ) );
}

/*!
  \brief Set mesh source.
  \param obj Mesh source.
  \internal
*/
void GroupCombo::setSource( const SMESH::SelectionProxy& proxy )
{
  if ( myProxy == proxy )
    return;

  myProxy = proxy;

  bool blocked = blockSignals( true );
  QStandardItemModel* m = dynamic_cast<QStandardItemModel*>( model() );
  m->clear();

  if ( myProxy )
  {
    if ( myProxy.type() == SMESH::SelectionProxy::Mesh )
    {
      QList<SMESH::SelectionProxy> groups = myProxy.groups();
      for ( int i = 0; i < groups.count(); ++i )
      {
        if ( groups[i] )
        {
          QString name = groups[i].name();
          if ( !name.isEmpty() )
            m->appendRow( new Item( groups[i] ) );
        }
      }
      setCurrentIndex( -1 ); // for performance reasons
    }
    else if ( myProxy.type() >= SMESH::SelectionProxy::Group )
    {
      m->appendRow( new Item( myProxy ) );
      setCurrentIndex( 0 );
    }
  }

  blockSignals( blocked );
}

/*!
  \brief Get currently selected group.
  \return Selected group.
  \internal
*/
SMESH::SelectionProxy GroupCombo::currentGroup() const
{
  SMESH::SelectionProxy group;
  QStandardItemModel* m = dynamic_cast<QStandardItemModel*>( model() );
  if ( currentIndex() >= 0 )
    group = dynamic_cast<Item*>( m->item( currentIndex() ) )->group();
  return group;
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_MeshInfoDlg
/// \brief Mesh information dialog box
///
/// \todo Move all business logic for element info to SMESHGUI_ElemInfo class.
/// \todo Add selection button to reactivate selection on move from other dlg.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor
  \param parent Parent widget.
  \param page Dialog page to show at start-up. Defaults to \c BaseInfo.
*/
SMESHGUI_MeshInfoDlg::SMESHGUI_MeshInfoDlg( QWidget* parent, int page )
  : QDialog( parent )
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle( tr( "MESH_INFO" ) );
  setSizeGripEnabled( true );

  myTabWidget = new QTabWidget( this );

  // base info

  myBaseInfo = new SMESHGUI_BaseInfo( myTabWidget );
  myTabWidget->addTab( myBaseInfo, tr( "BASE_INFO" ) );

  // elem info 

  QWidget* w = new QWidget( myTabWidget );

  myMode = new QButtonGroup( this );
  myMode->addButton( new QRadioButton( tr( "NODE_MODE" ), w ), NodeMode );
  myMode->addButton( new QRadioButton( tr( "ELEM_MODE" ), w ), ElemMode );
  myMode->addButton( new QRadioButton( tr( "GROUP_MODE" ), w ), GroupMode );
  myMode->button( NodeMode )->setChecked( true );
  myID = new QLineEdit( w );
  myID->setValidator( new SMESHGUI_IdValidator( this ) );
  myGroups = new GroupCombo( w );
  QStackedWidget* stack = new QStackedWidget( w );
  stack->addWidget( myID );
  stack->addWidget( myGroups );
  myIDPreviewCheck = new QCheckBox( tr( "SHOW_IDS" ), w );
  myIDPreview = new SMESHGUI_IdPreview( SMESH::GetViewWindow( SMESHGUI::GetSMESHGUI() ) );

  int mode = SMESHGUI::resourceMgr()->integerValue( "SMESH", "mesh_elem_info", 1 );
  mode = qMin( 1, qMax( 0, mode ) );

  if ( mode == 0 )
    myElemInfo = new SMESHGUI_SimpleElemInfo( w );
  else
    myElemInfo = new SMESHGUI_TreeElemInfo( w );
  stack->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );

  QGridLayout* elemLayout = new QGridLayout( w );
  elemLayout->setMargin( MARGIN );
  elemLayout->setSpacing( SPACING );
  elemLayout->addWidget( myMode->button( NodeMode ), 0, 0 );
  elemLayout->addWidget( myMode->button( ElemMode ), 0, 1 );
  elemLayout->addWidget( myMode->button( GroupMode ), 0, 2 );
  elemLayout->addWidget( stack, 0, 3 );
  elemLayout->addWidget( myIDPreviewCheck, 1, 0, 1, 4 );
  elemLayout->addWidget( myElemInfo, 2, 0, 1, 4 );

  myTabWidget->addTab( w, tr( "ELEM_INFO" ) );

  // additional info

  myAddInfo = new SMESHGUI_AddInfo( myTabWidget );
  myTabWidget->addTab( myAddInfo, tr( "ADDITIONAL_INFO" ) );

  // controls info

  myCtrlInfo = new SMESHGUI_CtrlInfo( myTabWidget );
  myTabWidget->addTab( myCtrlInfo, tr( "CTRL_INFO" ) );

  // buttons

  QPushButton* okBtn = new QPushButton( tr( "SMESH_BUT_OK" ), this );
  okBtn->setAutoDefault( true );
  okBtn->setDefault( true );
  okBtn->setFocus();
  QPushButton* dumpBtn = new QPushButton( tr( "BUT_DUMP_MESH" ), this );
  dumpBtn->setAutoDefault( true );
  QPushButton* helpBtn = new QPushButton( tr( "SMESH_BUT_HELP" ), this );
  helpBtn->setAutoDefault( true );

  QHBoxLayout* btnLayout = new QHBoxLayout;
  btnLayout->setSpacing( SPACING );
  btnLayout->setMargin( 0 );

  btnLayout->addWidget( okBtn );
  btnLayout->addWidget( dumpBtn );
  btnLayout->addStretch( 10 );
  btnLayout->addWidget( helpBtn );

  // arrange widgets

  QVBoxLayout* l = new QVBoxLayout ( this );
  l->setMargin( MARGIN );
  l->setSpacing( SPACING );
  l->addWidget( myTabWidget );
  l->addLayout( btnLayout );

  // set initial page

  myTabWidget->setCurrentIndex( qMax( (int)BaseInfo, qMin( (int)ElemInfo, page ) ) );

  // set-up connections

  connect( okBtn, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( dumpBtn, SIGNAL( clicked() ), this, SLOT( dump() ) );
  connect( helpBtn, SIGNAL( clicked() ), this, SLOT( help() ) );
  connect( myTabWidget, SIGNAL( currentChanged( int ) ), this, SLOT( updateSelection() ) );
  connect( myMode, SIGNAL( buttonClicked( int ) ), this, SLOT( modeChanged() ) );
  connect( myGroups, SIGNAL( currentIndexChanged( int ) ), this, SLOT( modeChanged() ) );
  connect( myID, SIGNAL( textChanged( QString ) ), this, SLOT( idChanged() ) );
  connect( myIDPreviewCheck, SIGNAL( toggled( bool ) ), this, SLOT( idPreviewChange( bool ) ) );
  connect( SMESHGUI::GetSMESHGUI(), SIGNAL( SignalDeactivateActiveDialog() ), this, SLOT( deactivate() ) );
  connect( SMESHGUI::GetSMESHGUI(), SIGNAL( SignalCloseAllDialogs() ), this, SLOT( reject() ) );
  connect( myElemInfo, SIGNAL( itemInfo( int, QString ) ), this, SLOT( showItemInfo( int, QString ) ) );
  connect( this, SIGNAL( switchMode( int ) ), stack, SLOT( setCurrentIndex( int ) ) );

  // initialize

  myIDPreviewCheck->setChecked( SMESHGUI::resourceMgr()->booleanValue( "SMESH", "id_preview_resource", false ) );
  updateSelection();
}

/*!
  \brief Destructor.
*/
SMESHGUI_MeshInfoDlg::~SMESHGUI_MeshInfoDlg()
{
  delete myIDPreview;
}

/*!
  \brief Show mesh information on given object.
  \param io Interactive object.
*/
void SMESHGUI_MeshInfoDlg::showInfo( const Handle(SALOME_InteractiveObject)& io )
{
  if ( !io.IsNull() )
    showInfo( SMESH::SelectionProxy( io ) );
}

/*!
  \brief Show mesh information on given object.
  \param proxy Selection proxy.
*/
void SMESHGUI_MeshInfoDlg::showInfo( const SMESH::SelectionProxy& proxy )
{
  SUIT_OverrideCursor wc;

  if ( !proxy )
    return;

  myProxy = proxy;

  SMESH::SMESH_IDSource_var obj = myProxy.object();

  // "Base info" tab
  myBaseInfo->showInfo( proxy );

  // "Additional info" tab
  myAddInfo->showInfo( proxy );

  // "Quality info" tab
  // Note: for performance reasons we update it only if it is currently active
  if ( myTabWidget->currentIndex() == CtrlInfo )
    myCtrlInfo->showInfo( proxy );

  // "Element info" tab
  myGroups->setSource( proxy );
  if ( myMode->checkedId() == GroupMode ) {
    SMESH::SelectionProxy group = myGroups->currentGroup();
    if ( group )
      myElemInfo->showInfo( group );
    else
      myElemInfo->clear();
  }
  else {
    SVTK_Selector* selector = SMESH::GetSelector();
    QString ID;
    int nb = 0;
    if ( myProxy.actor() && selector ) { //todo: actor()?
      nb = myMode->checkedId() == NodeMode ?
        SMESH::GetNameOfSelectedElements( selector, myProxy.io(), ID ) :
        SMESH::GetNameOfSelectedNodes( selector, myProxy.io(), ID );
    }
    if ( nb > 0 ) {
      myID->setText( ID.trimmed() );
      QSet<uint> ids;
      QStringList idTxt = ID.split( " ", QString::SkipEmptyParts );
      foreach ( ID, idTxt )
        ids << ID.trimmed().toUInt();
      myElemInfo->showInfo( proxy, ids, myMode->checkedId() == ElemMode );
    }
    else {
      myID->clear();
      myElemInfo->clear();
    }
  }
}

/*!
  \brief Update information.
*/
void SMESHGUI_MeshInfoDlg::updateInfo()
{
  SALOME_ListIO selected;
  SMESHGUI::selectionMgr()->selectedObjects( selected );

  if ( selected.Extent() == 1 )
    showInfo( selected.First() );
  else
    showInfo( myProxy );
}

/*!
  \brief Clean-up on dialog closing.
*/
void SMESHGUI_MeshInfoDlg::reject()
{
  LightApp_SelectionMgr* selMgr = SMESHGUI::selectionMgr();
  selMgr->clearFilters();
  SMESH::SetPointRepresentation( false );
  if ( SVTK_ViewWindow* viewWindow = SMESH::GetViewWindow() )
    viewWindow->SetSelectionMode( ActorSelection );
  QDialog::reject();
  myIDPreview->SetPointsLabeled( false );
}

/*!
  \brief Process keyboard event.
  \param e Key press event.
*/
void SMESHGUI_MeshInfoDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( !e->isAccepted() && e->key() == Qt::Key_F1 ) {
    e->accept();
    help();
  }
}

/*!
  \brief Set-up selection mode for currently selected page.
*/
void SMESHGUI_MeshInfoDlg::updateSelection()
{
  LightApp_SelectionMgr* selMgr = SMESHGUI::selectionMgr();

  disconnect( selMgr, 0, this, 0 );
  selMgr->clearFilters();

  int selMode = ActorSelection;
  if ( myTabWidget->currentIndex() == ElemInfo && myMode->checkedId() == NodeMode )
    selMode = NodeSelection;
  else if ( myTabWidget->currentIndex() == ElemInfo && myMode->checkedId() == ElemMode )
    selMode = CellSelection;
  SMESH::SetPointRepresentation( selMode == NodeSelection );
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
    aViewWindow->SetSelectionMode( selMode );

  SMESH::SelectionProxy previous = myProxy;
  QString ids = myID->text().trimmed();
  myID->clear();
  
  connect( selMgr, SIGNAL( currentSelectionChanged() ), this, SLOT( updateInfo() ) );
  updateInfo();
  
  if ( myProxy && myProxy == previous && !ids.isEmpty() ) {
    myID->setText( ids );
    idChanged();
  }
}

/*!
  \brief Show documentation on selected dialog page.
*/
void SMESHGUI_MeshInfoDlg::help()
{
  QString helpPage = "mesh_infos.html";
  switch ( myTabWidget->currentIndex() )
  {
  case BaseInfo:
    helpPage += "#advanced-mesh-infos-anchor";
    break;
  case ElemInfo:
    helpPage += "#mesh-element-info-anchor";
    break;
  case AddInfo:
    helpPage += "#mesh-addition-info-anchor";
    break;
  case CtrlInfo:
    helpPage += "#mesh-quality-info-anchor";
    break;
  default:
    break;
  }
  SMESH::ShowHelpFile( helpPage );
}

/*!
  \brief Deactivate dialog box.
*/
void SMESHGUI_MeshInfoDlg::deactivate()
{
  disconnect( SMESHGUI::selectionMgr(), SIGNAL( currentSelectionChanged() ), this, SLOT( updateInfo() ) );
}

/*!
  \brief Called when users switches between node / element modes.
*/
void SMESHGUI_MeshInfoDlg::modeChanged()
{
  emit( switchMode( myMode->checkedId() == GroupMode ? 1 : 0 ) );
  myID->clear();
  updateSelection();
}

/*!
  \brief Called when users prints mesh element ID in the corresponding field.
*/
void SMESHGUI_MeshInfoDlg::idChanged()
{
  myIDPreview->SetPointsLabeled( false );

  if ( myProxy ) {
    SVTK_TVtkIDsMap      ID;
    QSet<uint>           ids;
    std::vector<int>     idVec;
    std::list< gp_XYZ >  aGrCentersXYZ;
    SMESH::XYZ           xyz;
    const bool           isElem = ( myMode->checkedId() == ElemMode );
    QStringList idTxt = myID->text().split( " ", QString::SkipEmptyParts );
    foreach ( QString tid, idTxt ) {
      long id = tid.toUInt();
      if ( isElem ? myProxy.hasElement( id ) : myProxy.hasNode( id ))
      {
        ID.Add( id );
        ids << id;
        if ( isElem && myProxy.actor() && myProxy.elementGravityCenter( id, xyz ))
        {
          idVec.push_back( id );
          aGrCentersXYZ.push_back( xyz );
        }
      }
    }
    SVTK_Selector* selector = SMESH::GetSelector();
    if ( myProxy.actor() && selector ) {
      Handle(SALOME_InteractiveObject) IO = myProxy.actor()->getIO();
      selector->AddOrRemoveIndex( IO, ID, false );
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
      {
        if ( myMode->checkedId() == NodeMode )
          myIDPreview->SetPointsData( myProxy.actor()->GetObject()->GetMesh(), ID );
        else
          myIDPreview->SetElemsData( idVec, aGrCentersXYZ );

        bool showIDs = ( !ID.IsEmpty() &&
                         myIDPreviewCheck->isChecked() &&
                         myTabWidget->currentIndex() == ElemInfo );
        myIDPreview->SetPointsLabeled( showIDs, myProxy.actor()->GetVisibility() );

        aViewWindow->highlight( IO, true, true );
        aViewWindow->Repaint();
      }
    }
    myElemInfo->showInfo( myProxy, ids, isElem );
  }
}

/*!
 * \brief Show IDs clicked
 */
void SMESHGUI_MeshInfoDlg::idPreviewChange( bool isOn )
{
  myIDPreview->SetPointsLabeled( isOn && !myID->text().simplified().isEmpty() );
  SMESHGUI::resourceMgr()->setValue("SMESH", "id_preview_resource", isOn );
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
    aViewWindow->Repaint();
}

void SMESHGUI_MeshInfoDlg::showItemInfo( int type, const QString& ids )
{
  if ( !ids.isEmpty() && ( type == NodeConnectivity || type == ElemConnectivity )) {
    myMode->button( type - NodeConnectivity )->click();
    myID->setText( ids );
  }
}

/*!
  \brief Dump information to file.
*/
void SMESHGUI_MeshInfoDlg::dump()
{
  DumpFileDlg fd( this );
  fd.setWindowTitle( tr( "SAVE_INFO" ) );
  fd.setNameFilters( QStringList() << tr( "TEXT_FILES" ) );
  fd.setChecked( BaseInfo, SMESHGUI::resourceMgr()->booleanValue( "SMESH", "info_dump_base", true ) );
  fd.setChecked( ElemInfo, SMESHGUI::resourceMgr()->booleanValue( "SMESH", "info_dump_elem", true ) );
  fd.setChecked( AddInfo, SMESHGUI::resourceMgr()->booleanValue( "SMESH", "info_dump_add", true ) );
  fd.setChecked( CtrlInfo, SMESHGUI::resourceMgr()->booleanValue( "SMESH", "info_dump_ctrl", true ) );
  if ( fd.exec() == QDialog::Accepted )
  {
    QString fileName = fd.selectedFile();
    if ( !fileName.isEmpty() ) {
      QFile file( fileName );
      if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
        return;

      QTextStream out( &file );
      if ( fd.isChecked( BaseInfo ) ) myBaseInfo->saveInfo( out );
      if ( fd.isChecked( ElemInfo ) ) myElemInfo->saveInfo( out );
      if ( fd.isChecked( AddInfo ) )  myAddInfo->saveInfo( out );
      if ( fd.isChecked( CtrlInfo ) )
      {
        myCtrlInfo->showInfo( myProxy ); // it saves what is shown only
        myCtrlInfo->saveInfo( out );
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_CtrlInfo
/// \brief Show quality statistics information on selected object.
///
/// Displays quality controls statistics about selected object: mesh, sub-mesh,
/// group or arbitrary ID source.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor.
  \param parent Parent widget. Defaults to 0.
*/
SMESHGUI_CtrlInfo::SMESHGUI_CtrlInfo( QWidget* parent ): SMESHGUI_Info( parent )
{
  QGridLayout* l = new QGridLayout( this );
  l->setMargin( MARGIN );
  l->setSpacing( SPACING );

  QIcon aComputeIcon( SUIT_Session::session()->resourceMgr()->loadPixmap( "SMESH", tr( "ICON_COMPUTE" ) ) );
  SMESH::FilterManager_var aFilterMgr = SMESH::GetFilterManager();

  // name
  QLabel* aNameLab = createLabel( tr( "NAME_LAB" ), this, Bold );
  QLabel* aName = createField( this, "ctrlName" );
  aName->setMinimumWidth( 150 );
  myWidgets << aName;

  // nodes info
  QLabel* aNodesLab = createLabel( tr( "NODES_INFO" ), this, Bold );
  QLabel* aNodesFreeLab = new QLabel( tr( "NUMBER_OF_THE_FREE_NODES" ), this );
  QLabel* aNodesFree = createField( this, "ctrlNodesFree" );
  myWidgets << aNodesFree;
  myPredicates << aFilterMgr->CreateFreeNodes();
  //
  QLabel* aNodesNbConnLab = new QLabel( tr( "MAX_NODE_CONNECTIVITY" ), this );
  QLabel* aNodesNbConn = createField( this, "ctrlNodesCnty" );
  myWidgets << aNodesNbConn;
  myNodeConnFunctor = aFilterMgr->CreateNodeConnectivityNumber();
  //
  QLabel* aNodesDoubleLab = new QLabel( tr( "NUMBER_OF_THE_DOUBLE_NODES" ), this );
  QLabel* aNodesDouble = createField( this, "ctrlNodesDouble" );
  myWidgets << aNodesDouble;
  myPredicates << aFilterMgr->CreateEqualNodes();
  QLabel* aToleranceLab = new QLabel( tr( "DOUBLE_NODES_TOLERANCE" ), this );
  myToleranceWidget = new SMESHGUI_SpinBox( this );
  myToleranceWidget->RangeStepAndValidator(0.0000000001, 1000000.0, 0.0000001, "length_precision" );
  myToleranceWidget->setAcceptNames( false );
  myToleranceWidget->SetValue( SMESHGUI::resourceMgr()->doubleValue( "SMESH", "equal_nodes_tolerance", 1e-7 ) );

  // edges info
  QLabel* anEdgesLab = createLabel( tr( "EDGES_INFO" ), this, Bold );
  QLabel* anEdgesDoubleLab = new QLabel( tr( "NUMBER_OF_THE_DOUBLE_EDGES" ),     this );
  QLabel* anEdgesDouble = createField( this, "ctrlEdgesDouble" );
  myWidgets << anEdgesDouble;
  myPredicates << aFilterMgr->CreateEqualEdges();

  // faces info
  QLabel* aFacesLab = createLabel( tr( "FACES_INFO" ), this, Bold );
  QLabel* aFacesDoubleLab = new QLabel( tr( "NUMBER_OF_THE_DOUBLE_FACES" ), this );
  QLabel* aFacesDouble = createField( this, "ctrlFacesDouble" );
  myWidgets << aFacesDouble;
  myPredicates << aFilterMgr->CreateEqualFaces();
  QLabel* aFacesOverLab = new QLabel( tr( "NUMBER_OF_THE_OVER_CONSTRAINED" ), this );
  QLabel* aFacesOver = createField( this, "ctrlFacesOver" );
  myWidgets << aFacesOver;
  myPredicates << aFilterMgr->CreateOverConstrainedFace();
  QLabel* anAspectRatioLab = new QLabel( tr( "ASPECT_RATIO_HISTOGRAM" ), this );
  myPlot = createPlot( this );
  myAspectRatio = aFilterMgr->CreateAspectRatio();
 
  // volumes info
  QLabel* aVolumesLab = createLabel( tr( "VOLUMES_INFO" ), this, Bold );
  QLabel* aVolumesDoubleLab = new QLabel( tr( "NUMBER_OF_THE_DOUBLE_VOLUMES" ), this );
  QLabel* aVolumesDouble = createField( this, "ctrlVolumesDouble" );
  myWidgets << aVolumesDouble;
  myPredicates << aFilterMgr->CreateEqualVolumes();
  QLabel* aVolumesOverLab = new QLabel( tr( "NUMBER_OF_THE_OVER_CONSTRAINED" ), this );
  QLabel* aVolumesOver = createField( this, "ctrlVolumesOver" );
  myWidgets << aVolumesOver;
  myPredicates << aFilterMgr->CreateOverConstrainedVolume();
  QLabel* anAspectRatio3DLab = new QLabel( tr( "ASPECT_RATIO_3D_HISTOGRAM" ), this );
  myPlot3D = createPlot( this );
  myAspectRatio3D = aFilterMgr->CreateAspectRatio3D();

  QToolButton* aFreeNodesBtn = new QToolButton( this );
  aFreeNodesBtn->setIcon(aComputeIcon);
  myButtons << aFreeNodesBtn;       //0

  QToolButton* aNodesNbConnBtn = new QToolButton( this );
  aNodesNbConnBtn->setIcon(aComputeIcon);
  myButtons << aNodesNbConnBtn;     //1

  QToolButton* aDoubleNodesBtn = new QToolButton( this );
  aDoubleNodesBtn->setIcon(aComputeIcon);
  myButtons << aDoubleNodesBtn;     //2

  QToolButton* aDoubleEdgesBtn = new QToolButton( this );
  aDoubleEdgesBtn->setIcon(aComputeIcon);
  myButtons << aDoubleEdgesBtn;     //3

  QToolButton* aDoubleFacesBtn = new QToolButton( this );
  aDoubleFacesBtn->setIcon(aComputeIcon);
  myButtons << aDoubleFacesBtn;     //4

  QToolButton* aOverContFacesBtn = new QToolButton( this );
  aOverContFacesBtn->setIcon(aComputeIcon);
  myButtons << aOverContFacesBtn;   //5

  QToolButton* aComputeFaceBtn = new QToolButton( this );
  aComputeFaceBtn->setIcon(aComputeIcon);
  myButtons << aComputeFaceBtn;     //6

  QToolButton* aDoubleVolumesBtn = new QToolButton( this );
  aDoubleVolumesBtn->setIcon(aComputeIcon);
  myButtons << aDoubleVolumesBtn;   //7

  QToolButton* aOverContVolumesBtn = new QToolButton( this );
  aOverContVolumesBtn->setIcon(aComputeIcon);
  myButtons << aOverContVolumesBtn; //8

  QToolButton* aComputeVolumeBtn = new QToolButton( this );
  aComputeVolumeBtn->setIcon(aComputeIcon);
  myButtons << aComputeVolumeBtn;   //9

  connect( aComputeFaceBtn,   SIGNAL( clicked() ), this, SLOT( computeAspectRatio() ) );
  connect( aComputeVolumeBtn, SIGNAL( clicked() ), this, SLOT( computeAspectRatio3D() ) );
  connect( aFreeNodesBtn,     SIGNAL( clicked() ), this, SLOT( computeFreeNodesInfo() ) );
  connect( aNodesNbConnBtn,   SIGNAL( clicked() ), this, SLOT( computeNodesNbConnInfo() ) );
  connect( aDoubleNodesBtn,   SIGNAL( clicked() ), this, SLOT( computeDoubleNodesInfo() ) );
  connect( aDoubleEdgesBtn,   SIGNAL( clicked() ), this, SLOT( computeDoubleEdgesInfo() ) );
  connect( aDoubleFacesBtn,   SIGNAL( clicked() ), this, SLOT( computeDoubleFacesInfo() ) );
  connect( aOverContFacesBtn, SIGNAL( clicked() ), this, SLOT( computeOverConstrainedFacesInfo() ) );
  connect( aDoubleVolumesBtn, SIGNAL( clicked() ), this, SLOT( computeDoubleVolumesInfo() ) );
  connect( aOverContVolumesBtn,SIGNAL( clicked() ), this, SLOT( computeOverConstrainedVolumesInfo() ) );
  connect( myToleranceWidget, SIGNAL( valueChanged( double ) ), this, SLOT( setTolerance( double ) ) );

  l->addWidget( aNameLab,           0, 0 );       //0
  l->addWidget( aName,              0, 1, 1, 2 ); //1
  l->addWidget( aNodesLab,          1, 0, 1, 3 ); //2
  l->addWidget( aNodesFreeLab,      2, 0 );       //3
  l->addWidget( aNodesFree,         2, 1 );       //4
  l->addWidget( aFreeNodesBtn,      2, 2 );       //5
  l->addWidget( aNodesNbConnLab,    3, 0 );       //6
  l->addWidget( aNodesNbConn,       3, 1 );       //7
  l->addWidget( aNodesNbConnBtn,    3, 2 );       //8
  l->addWidget( aNodesDoubleLab,    4, 0 );       //9
  l->addWidget( aNodesDouble,       4, 1 );       //10
  l->addWidget( aDoubleNodesBtn,    4, 2 );       //11
  l->addWidget( aToleranceLab,      5, 0 );       //12
  l->addWidget( myToleranceWidget,  5, 1 );       //13
  l->addWidget( anEdgesLab,         6, 0, 1, 3 ); //14
  l->addWidget( anEdgesDoubleLab,   7, 0 );       //15
  l->addWidget( anEdgesDouble,      7, 1 );       //16
  l->addWidget( aDoubleEdgesBtn,    7, 2 );       //17
  l->addWidget( aFacesLab,          8, 0, 1, 3 ); //18
  l->addWidget( aFacesDoubleLab,    9, 0 );       //19
  l->addWidget( aFacesDouble,       9, 1 );       //20
  l->addWidget( aDoubleFacesBtn,    9, 2 );       //21
  l->addWidget( aFacesOverLab,      10, 0 );      //22
  l->addWidget( aFacesOver,         10, 1 );      //23
  l->addWidget( aOverContFacesBtn,  10, 2 );      //24
  l->addWidget( anAspectRatioLab,   11, 0 );      //25
  l->addWidget( aComputeFaceBtn,    11, 2 );      //26
  l->addWidget( myPlot,             12, 0, 1, 3 );//27
  l->addWidget( aVolumesLab,        13, 0, 1, 3 );//28
  l->addWidget( aVolumesDoubleLab,  14, 0 );      //29
  l->addWidget( aVolumesDouble,     14, 1 );      //30
  l->addWidget( aDoubleVolumesBtn,  14, 2 );      //31
  l->addWidget( aVolumesOverLab,    15, 0 );      //32
  l->addWidget( aVolumesOver,       15, 1 );      //33
  l->addWidget( aOverContVolumesBtn,15, 2 );      //34
  l->addWidget( anAspectRatio3DLab, 16, 0 );      //35
  l->addWidget( aComputeVolumeBtn,  16, 2 );      //36
  l->addWidget( myPlot3D,           17, 0, 1, 3 );//37
 
  l->setColumnStretch(  0,  0 );
  l->setColumnStretch(  1,  5 );
  l->setRowStretch   ( 12,  5 );
  l->setRowStretch   ( 17,  5 );
  l->setRowStretch   ( 18,  1 );

  clearInternal();
}

/*!
  \brief Destructor.
*/
SMESHGUI_CtrlInfo::~SMESHGUI_CtrlInfo()
{
}

/*!
  \brief Create plot widget.
  \param parent Parent widget.
  \return New plot widget.
*/
QwtPlot* SMESHGUI_CtrlInfo::createPlot( QWidget* parent )
{
  QwtPlot* aPlot = new QwtPlot( parent );
  aPlot->setMinimumSize( 100, 100 );
  QFont xFont = aPlot->axisFont( QwtPlot::xBottom );
  xFont.setPointSize( 5 );
  QFont yFont = aPlot->axisFont( QwtPlot::yLeft );
  yFont.setPointSize( 5 );
  aPlot->setAxisFont( QwtPlot::xBottom, xFont );
  aPlot->setAxisFont( QwtPlot::yLeft, yFont );
  aPlot->replot();
  return aPlot;
}

/*!
  \brief Show information on given object.
  \param proxy Object to show information on (mesh, sub-mesh, group, ID source).
*/
void SMESHGUI_CtrlInfo::showInfo( const SMESH::SelectionProxy& proxy )
{
  clearInternal();

  if ( !proxy )
    return;

  myProxy = proxy;
  SMESH::SMESH_IDSource_var obj = proxy.object();

  myWidgets[0]->setText( proxy.name() );

  SMESH::SMESH_Mesh_var mesh = obj->GetMesh();
  if ( mesh->_is_nil() ) return;

  const bool meshLoaded = mesh->IsLoaded();
  if ( !meshLoaded ) // mesh not yet loaded from the hdf file
    // enable Compute buttons, just in case obj->GetNbElementsByType() fails
    for ( int i = 0; i < myButtons.count(); ++i )
      myButtons[i]->setEnabled( true );

  SMESH::smIdType_array_var nbElemsByType = obj->GetNbElementsByType();
  if ( ! &nbElemsByType.in() ) return;

  const CORBA::Long ctrlLimit =
    meshLoaded ? SMESHGUI::resourceMgr()->integerValue( "SMESH", "info_controls_limit", 3000 ) : -1;

  // nodes info
  const SMESH::smIdType nbNodes =  nbElemsByType[ SMESH::NODE ];
  // const CORBA::Long nbElems = ( nbElemsByType[ SMESH::EDGE ] +
  //                               nbElemsByType[ SMESH::FACE ] +
  //                               nbElemsByType[ SMESH::VOLUME ] );
  if ( nbNodes > 0 ) {
    if ( nbNodes <= ctrlLimit ) {
      // free nodes
      computeFreeNodesInfo();
      // node connectivity number
      computeNodesNbConnInfo();
      // double nodes
      computeDoubleNodesInfo();
    }
    else {
      myButtons[0]->setEnabled( true );
      myButtons[1]->setEnabled( true );
      myButtons[2]->setEnabled( true );
    }
  }
  else {
    for( int i=2; i<=13; i++)
      dynamic_cast<QGridLayout*>(layout())->itemAt(i)->widget()->setVisible( false );
  }

  // edges info
  if ( nbElemsByType[ SMESH::EDGE ] > 0 ) {
    // double edges
    if( nbElemsByType[ SMESH::EDGE ] <= ctrlLimit )
      computeDoubleEdgesInfo();
    else
      myButtons[3]->setEnabled( true );
  }
  else {
    for( int i=14; i<=17; i++)
      dynamic_cast<QGridLayout*>(layout())->itemAt(i)->widget()->setVisible( false );
  }

  // faces info
  if ( nbElemsByType[ SMESH::FACE ] > 0 ) {
    if ( nbElemsByType[ SMESH::FACE ] <= ctrlLimit ) {
      // double faces
      computeDoubleFacesInfo();
      // over constrained faces
      computeOverConstrainedFacesInfo();
      // aspect Ratio histogram
      computeAspectRatio();
    }
    else {
      myButtons[4]->setEnabled( true );
      myButtons[5]->setEnabled( true );
      myButtons[6]->setEnabled( true );
    }
#ifdef DISABLE_PLOT2DVIEWER
    for( int i=25; i<=27; i++)
      dynamic_cast<QGridLayout*>(layout())->itemAt(i)->widget()->setVisible( false );
#endif
  }
  else {
    for( int i=18; i<=27; i++)
      dynamic_cast<QGridLayout*>(layout())->itemAt(i)->widget()->setVisible( false );
  }

  // volumes info
  if ( nbElemsByType[ SMESH::VOLUME ] > 0 ) {
    if ( nbElemsByType[ SMESH::VOLUME ] <= ctrlLimit ) {
      // double volumes
      computeDoubleVolumesInfo();
      // over constrained volumes
      computeOverConstrainedVolumesInfo();
      // aspect Ratio 3D histogram
      computeAspectRatio3D();
    }
    else {
      myButtons[7]->setEnabled( true );
      myButtons[8]->setEnabled( true );
      myButtons[9]->setEnabled( true );
    }
#ifdef DISABLE_PLOT2DVIEWER
    for( int i=35; i<=37; i++)
      dynamic_cast<QGridLayout*>(layout())->itemAt(i)->widget()->setVisible( false );
#endif
  }
  else {
    for( int i=28; i<=37; i++)
      dynamic_cast<QGridLayout*>(layout())->itemAt(i)->widget()->setVisible( false );
  }
}

//================================================================================
/*!
 * \brief Computes and shows nb of elements satisfying a given predicate
 *  \param [in] ft - a predicate type (SMESH::FunctorType)
 *  \param [in] iBut - index of one of myButtons to disable
 *  \param [in] iWdg - index of one of myWidgets to show the computed number
 */
//================================================================================

void SMESHGUI_CtrlInfo::computeNb( int ft, int iBut, int iWdg )
{
  myButtons[ iBut ]->setEnabled( false );
  myWidgets[ iWdg ]->setText( "" );

  if ( !myProxy )
    return;

  SUIT_OverrideCursor wc;

  SMESH::SMESH_IDSource_var obj = myProxy.object();
  SMESH::SMESH_Mesh_var mesh = obj->GetMesh();

  if ( !mesh->_is_nil() && !mesh->IsLoaded() )
  {
    mesh->Load();
    showInfo( myProxy ); // try to show all values
    if ( !myWidgets[ iWdg ]->text().isEmpty() )
      return; // <ft> predicate already computed
  }
  // look for a predicate of type <ft>
  for ( int i = 0; i < myPredicates.count(); ++i )
    if ( myPredicates[i]->GetFunctorType() == ft )
    {
      CORBA::Long nb = myPredicates[i]->NbSatisfying( obj );
      myWidgets[ iWdg ]->setText( QString::number( nb ) );
    }
}

void SMESHGUI_CtrlInfo::computeFreeNodesInfo()
{
  computeNb( SMESH::FT_FreeNodes, 0, 1 );
}

void SMESHGUI_CtrlInfo::computeDoubleNodesInfo()
{
  computeNb( SMESH::FT_EqualNodes, 2, 3 );
}

void SMESHGUI_CtrlInfo::computeDoubleEdgesInfo()
{
  computeNb( SMESH::FT_EqualEdges, 3, 4 );
}

void SMESHGUI_CtrlInfo::computeDoubleFacesInfo()
{
  computeNb( SMESH::FT_EqualFaces, 4, 5 );
}

void SMESHGUI_CtrlInfo::computeOverConstrainedFacesInfo()
{
  computeNb( SMESH::FT_OverConstrainedFace, 5, 6 );
}

void SMESHGUI_CtrlInfo::computeDoubleVolumesInfo()
{
  computeNb( SMESH::FT_EqualVolumes, 7, 7 );
}

void SMESHGUI_CtrlInfo::computeOverConstrainedVolumesInfo()
{
  computeNb( SMESH::FT_OverConstrainedVolume, 8, 8 );
}

void SMESHGUI_CtrlInfo::computeNodesNbConnInfo()
{
  myButtons[ 1 ]->setEnabled( false );
  myWidgets[ 2 ]->setText( "" );

  if ( !myProxy )
    return;

  SUIT_OverrideCursor wc;

  SMESH::SMESH_IDSource_var obj = myProxy.object();
  SMESH::SMESH_Mesh_var mesh = obj->GetMesh();

  if ( !mesh->IsLoaded() )
  {
    mesh->Load();
    showInfo( myProxy ); // try to show all values
    if ( !myWidgets[ 2 ]->text().isEmpty() )
      return; // already computed
  }
  myNodeConnFunctor->SetMesh( mesh );
  SMESH::Histogram_var histogram =
    myNodeConnFunctor->GetLocalHistogram( 1, /*isLogarithmic=*/false, obj );

  myWidgets[ 2 ]->setText( QString::number( histogram[0].max ) );
}

void SMESHGUI_CtrlInfo::computeAspectRatio()
{
#ifndef DISABLE_PLOT2DVIEWER
  myButtons[6]->setEnabled( false );

  if ( !myProxy )
    return;

  SUIT_OverrideCursor wc;

  SMESH::SMESH_IDSource_var obj = myProxy.object();
  SMESH::SMESH_Mesh_var mesh = obj->GetMesh();

  Plot2d_Histogram* aHistogram = getHistogram( myAspectRatio );
  if ( aHistogram && !aHistogram->isEmpty() ) {
    QwtPlotItem* anItem = aHistogram->createPlotItem();
    anItem->attach( myPlot );
    myPlot->replot();
  }
  delete aHistogram;
#endif
}

void SMESHGUI_CtrlInfo::computeAspectRatio3D()
{
#ifndef DISABLE_PLOT2DVIEWER
  myButtons[9]->setEnabled( false );

  if ( !myProxy )
    return;

  SUIT_OverrideCursor wc;

  SMESH::SMESH_IDSource_var obj = myProxy.object();
  SMESH::SMESH_Mesh_var mesh = obj->GetMesh();

  Plot2d_Histogram* aHistogram = getHistogram( myAspectRatio3D );
  if ( aHistogram && !aHistogram->isEmpty() ) {
    QwtPlotItem* anItem = aHistogram->createPlotItem();
    anItem->attach( myPlot3D );
    myPlot3D->replot();
  }
  delete aHistogram;
#endif
}

/*!
  \brief Internal clean-up (reset widget)
*/
void SMESHGUI_CtrlInfo::clearInternal()
{
  for( int i=0; i<=37; i++)
    dynamic_cast<QGridLayout*>(layout())->itemAt(i)->widget()->setVisible( true );
  for( int i=0; i<=9; i++)
    myButtons[i]->setEnabled( false );
  myPlot->detachItems();
  myPlot3D->detachItems();
  myPlot->replot();
  myPlot3D->replot();
  myWidgets[0]->setText( QString() );
  for ( int i = 1; i < myWidgets.count(); i++ )
    myWidgets[i]->setText( "" );
}

void SMESHGUI_CtrlInfo::setTolerance( double theTolerance )
{
  myButtons[2]->setEnabled( true );
  myWidgets[3]->setText("");
  for ( int i = 0; i < myPredicates.count(); ++i )
    if ( myPredicates[i]->GetFunctorType() == SMESH::FT_EqualNodes )
    {
      SMESH::EqualNodes_var functor = SMESH::EqualNodes::_narrow( myPredicates[i] );
      if ( !functor->_is_nil() )
        functor->SetTolerance( theTolerance );
    }
}

#ifndef DISABLE_PLOT2DVIEWER
Plot2d_Histogram* SMESHGUI_CtrlInfo::getHistogram( SMESH::NumericalFunctor_ptr aNumFun )
{
  SUIT_OverrideCursor wc;

  SMESH::SMESH_IDSource_var obj = myProxy.object();
  SMESH::SMESH_Mesh_var mesh = obj->GetMesh();

  if ( !mesh->IsLoaded() )
    mesh->Load();
  aNumFun->SetMesh( mesh );

  CORBA::Long cprecision = 6;
  if ( SMESHGUI::resourceMgr()->booleanValue( "SMESH", "use_precision", false ) )
    cprecision = SMESHGUI::resourceMgr()->integerValue( "SMESH", "controls_precision", -1 );
  aNumFun->SetPrecision( cprecision );

  int nbIntervals = SMESHGUI::resourceMgr()->integerValue( "SMESH", "scalar_bar_num_colors", false );

  SMESH::Histogram_var histogramVar = aNumFun->GetLocalHistogram( nbIntervals,
                                                                  /*isLogarithmic=*/false,
                                                                  obj );
  Plot2d_Histogram* aHistogram = new Plot2d_Histogram();
  aHistogram->setColor( palette().color( QPalette::Highlight ) );
  if ( &histogramVar.in() )
  {
    for ( size_t i = 0, nb = histogramVar->length(); i < nb; i++ )
      aHistogram->addPoint( 0.5 * ( histogramVar[i].min + histogramVar[i].max ), histogramVar[i].nbEvents );
    if ( histogramVar->length() >= 2 )
      aHistogram->setWidth( ( histogramVar[0].max - histogramVar[0].min ) * 0.8 );
  }
  return aHistogram;
}
#endif

void SMESHGUI_CtrlInfo::saveInfo( QTextStream &out )
{
  // title
  QString title = tr( "CTRL_INFO" );
  out << ruler( title.size() ) << endl;
  out << title << endl;
  out << ruler( title.size() ) << endl;
  out << endl;

  // info
  out << tr( "NAME_LAB" ) << "  " << myWidgets[0]->text() << endl;
  out << tr( "NODES_INFO" ) << endl;
  out << indent() << tr( "NUMBER_OF_THE_FREE_NODES" ) << ": " << myWidgets[1]->text() << endl;
  out << indent() << tr( "NUMBER_OF_THE_DOUBLE_NODES" ) << ": " << myWidgets[3]->text() << endl;
  out << tr( "EDGES_INFO" ) << endl;
  out << indent() << tr( "NUMBER_OF_THE_DOUBLE_EDGES" ) << ": " << myWidgets[4]->text() << endl;
  out << tr( "FACES_INFO" ) << endl;
  out << indent() << tr( "NUMBER_OF_THE_DOUBLE_FACES" ) << ": " << myWidgets[5]->text() << endl;
  out << indent() << tr( "NUMBER_OF_THE_OVER_CONSTRAINED" ) << ": " << myWidgets[6]->text() << endl;
  out << tr( "VOLUMES_INFO" ) << endl;
  out << indent() << tr( "NUMBER_OF_THE_DOUBLE_VOLUMES" ) << ": " << myWidgets[7]->text() << endl;
  out << indent() << tr( "NUMBER_OF_THE_OVER_CONSTRAINED" ) << ": " << myWidgets[8]->text() << endl;
}

////////////////////////////////////////////////////////////////////////////////
/// \class SMESHGUI_CtrlInfoDlg
/// \brief Overall Mesh Quality dialog.
/// \todo Add selection button to reactivate selection on move from other dlg.
////////////////////////////////////////////////////////////////////////////////

/*!
  \brief Constructor
  \param parent parent widget
*/
SMESHGUI_CtrlInfoDlg::SMESHGUI_CtrlInfoDlg( QWidget* parent )
  : QDialog( parent )
{
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle( tr( "CTRL_INFO" ) );
  setMinimumSize( 400, 600 );

  myCtrlInfo = new SMESHGUI_CtrlInfo( this );
  
  // buttons
  QPushButton* okBtn = new QPushButton( tr( "SMESH_BUT_OK" ), this );
  okBtn->setAutoDefault( true );
  okBtn->setDefault( true );
  okBtn->setFocus();
  QPushButton* dumpBtn = new QPushButton( tr( "BUT_DUMP_MESH" ), this );
  dumpBtn->setAutoDefault( true );
  QPushButton* helpBtn = new QPushButton( tr( "SMESH_BUT_HELP" ), this );
  helpBtn->setAutoDefault( true );

  QHBoxLayout* btnLayout = new QHBoxLayout;
  btnLayout->setSpacing( SPACING );
  btnLayout->setMargin( 0 );

  btnLayout->addWidget( okBtn );
  btnLayout->addWidget( dumpBtn );
  btnLayout->addStretch( 10 );
  btnLayout->addWidget( helpBtn );

  QVBoxLayout* l = new QVBoxLayout ( this );
  l->setMargin( 0 );
  l->setSpacing( SPACING );
  l->addWidget( myCtrlInfo );
  l->addLayout( btnLayout );

  connect( okBtn,   SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( dumpBtn, SIGNAL( clicked() ), this, SLOT( dump() ) );
  connect( helpBtn, SIGNAL( clicked() ), this, SLOT( help() ) );
  connect( SMESHGUI::GetSMESHGUI(), SIGNAL( SignalDeactivateActiveDialog() ), this, SLOT( deactivate() ) );
  connect( SMESHGUI::GetSMESHGUI(), SIGNAL( SignalCloseAllDialogs() ),        this, SLOT( reject() ) );

  updateSelection();
}

/*!
  \brief Destructor
*/
SMESHGUI_CtrlInfoDlg::~SMESHGUI_CtrlInfoDlg()
{
}

/*!
  \brief Show mesh quality information on given object.
  \param io Interactive object.
*/
void SMESHGUI_CtrlInfoDlg::showInfo( const Handle(SALOME_InteractiveObject)& io )
{  
  if ( !io.IsNull() )
    showInfo( SMESH::SelectionProxy( io ) );
}

/*!
  \brief Show mesh quality information on given object.
  \param proxy Selection proxy.
*/
void SMESHGUI_CtrlInfoDlg::showInfo( const SMESH::SelectionProxy& proxy )
{
  SUIT_OverrideCursor wc;

  if ( !proxy )
    return;

  myProxy = proxy;
  myCtrlInfo->showInfo( proxy );
}

/*!
  \brief Show mesh information
*/
void SMESHGUI_CtrlInfoDlg::updateInfo()
{
  SALOME_ListIO selected;
  SMESHGUI::selectionMgr()->selectedObjects( selected );

  if ( selected.Extent() == 1 )
    showInfo( selected.First() );
  else
    showInfo( myProxy );
}

/*!
  \brief Perform clean-up actions on the dialog box closing.
*/
void SMESHGUI_CtrlInfoDlg::reject()
{
  SMESH::SetPointRepresentation( false );
  QDialog::reject();
}

/*!
  \brief Setup selection mode depending on the current dialog box state.
*/
void SMESHGUI_CtrlInfoDlg::updateSelection()
{
  LightApp_SelectionMgr* selMgr = SMESHGUI::selectionMgr();
  disconnect( selMgr, 0, this, 0 );
  SMESH::SetPointRepresentation( false );  
  connect( selMgr, SIGNAL( currentSelectionChanged() ), this, SLOT( updateInfo() ) );
  updateInfo();  
}

/*!
  \brief Deactivate dialog box.
*/
void SMESHGUI_CtrlInfoDlg::deactivate()
{
  disconnect( SMESHGUI::selectionMgr(), SIGNAL( currentSelectionChanged() ), this, SLOT( updateInfo() ) );
}

/*!
  \brief Dump information to file.
*/
void SMESHGUI_CtrlInfoDlg::dump()
{
  DumpFileDlg fd( this, false );
  fd.setWindowTitle( tr( "SAVE_INFO" ) );
  fd.setNameFilters( QStringList() << tr( "TEXT_FILES" ) );
  if ( fd.exec() == QDialog::Accepted )
  {
    QString fileName = fd.selectedFile();
    if ( !fileName.isEmpty() ) {
      QFile file( fileName );
      if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
        return;

      QTextStream out( &file );
      myCtrlInfo->saveInfo( out );
    }
  }
}

/*!
  \brief Show documentation on dialog.
*/
void SMESHGUI_CtrlInfoDlg::help()
{
  SMESH::ShowHelpFile( "mesh_infos.html#mesh-quality-info-anchor" );
}
