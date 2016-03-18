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

#include "SMESHGUI_FieldSelectorWdg.h"

#include "SMESHGUI_Utils.h"
#include "SMESHGUI_GEOMGenUtils.h"

#include <SALOMEDSClient_Study.hxx>
#include <GEOM_wrap.hxx>

#include <QGroupBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>
#include <QLabel>
#include "SMESHGUI.h"

namespace
{
  QTreeWidgetItem* createItem( QTreeWidget*     tree,
                               const QString&   text,
                               const int        index,
                               QTreeWidgetItem* parentItem=0)
  {
    QTreeWidgetItem* item;
    if ( parentItem )
      item = new QTreeWidgetItem( parentItem );
    else
      item = new QTreeWidgetItem( tree );
    item->setText( 0, text );
    item->setData( 0, Qt::UserRole, index );
    item->setFlags( item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEditable );
    item->setCheckState( 0, Qt::Unchecked );
    item->setExpanded( true );
    if ( index < 0 )
    {
      QFont f = item->font( 0 );
      f.setItalic( true );
      item->setFont( 0, f );
    }
    return item;
  }
}

//--------------------------------------------------------------------------------
/*!
 * \brief Constructor of SMESHGUI_FieldSelectorWdg
 */
SMESHGUI_FieldSelectorWdg::SMESHGUI_FieldSelectorWdg( QWidget* p )
  :QGroupBox( tr("FIELDS_TO_EXPORT"), p )
{
  setCheckable( true );
  myTree = new QTreeWidget( this );
  myTree->setHeaderHidden( true );
  
  QVBoxLayout* lay = new QVBoxLayout( this );
  lay->addWidget( myTree );

  connect( myTree, SIGNAL( itemChanged(QTreeWidgetItem*, int)),
           this,   SLOT  ( onItemCheck(QTreeWidgetItem*, int)));
}

//--------------------------------------------------------------------------------
/*!
 * \brief Retrieves all fields defined on geometry of given meshes
 */
bool SMESHGUI_FieldSelectorWdg::
GetAllFeilds(const QList< QPair< SMESH::SMESH_IDSource_var, QString > >& meshes,
             QList< QPair< GEOM::ListOfFields_var, QString > >&          fields)
{
  myFields = & fields;
  myTree->clear();
  
  _PTR(Study) study = SMESH::GetActiveStudyDocument();
  GEOM::GEOM_Gen_var geomGen = SMESH::GetGEOMGen();
  GEOM::GEOM_IFieldOperations_wrap fieldOp = geomGen->GetIFieldOperations( study->StudyId() );

  for ( int iM = 0; iM < meshes.count(); ++iM )
  {
    GEOM::ListOfFields_var fields;
    QString                geoAss;

    SMESH::SMESH_Mesh_var mesh = meshes[iM].first->GetMesh();
    if ( !mesh->_is_nil() && mesh->HasShapeToMesh() )
    {
      SMESH::array_of_ElementType_var elemTypes = meshes[iM].first->GetTypes();
      if ( &elemTypes.in() && elemTypes->length() > 0 )
      {
        QTreeWidgetItem* meshItem = createItem( myTree, meshes[iM].second, iM );

        GEOM::GEOM_Object_var shape = mesh->GetShapeToMesh();
        fields = fieldOp->GetFields( shape );
        for ( size_t iF = 0; iF < fields->length(); ++iF )
        {
          GEOM::field_data_type dataType = fields[ iF ]->GetDataType();
          if ( dataType == GEOM::FDT_String )
            continue;
          GEOM::ListOfLong_var stepIDs = fields[ iF ]->GetSteps();
          if ( stepIDs->length() < 1 )
            continue;
          GEOM::string_array_var comps = fields[ iF ]->GetComponents();
          if ( comps->length() < 1 )
            continue;
          CORBA::Short dim = fields[iF]->GetDimension();
          if ( dim < 0 )
            continue; // "whole shape" field ignored

          CORBA::String_var name = fields[iF]->GetName();
          createItem( myTree, name.in(), iF, meshItem );
        }
        QString geoAss;
        for ( size_t i = 0; i < elemTypes->length(); ++i )
        {
          QString name;
          switch ( elemTypes[i] ) {
          case SMESH::NODE:   name = "_vertices_"; break;
          case SMESH::EDGE:   name = "_edges_"   ; break;
          case SMESH::FACE:   name = "_faces_"   ; break;
          case SMESH::VOLUME: name = "_solids_"  ; break;
          default: continue;
          }
          geoAss += name[1];
          createItem( myTree, name, -1, meshItem );
        }
        if ( !geoAss.isEmpty() && !geoAss.count('v') )
        {
          createItem( myTree, "_vertices_", -1, meshItem );
        }
      }
    } // if ( mesh->HasShapeToMesh() )

    if ( ! &fields.in() )
      fields = new GEOM::ListOfFields();

    myFields->push_back( qMakePair( fields, QString() ));

  } // loop on meshes

  setChecked( false );

  return myTree->topLevelItemCount();
}

//--------------------------------------------------------------------------------
/*!
 * \brief Filter off not selected fields from myFields
 */
bool SMESHGUI_FieldSelectorWdg::GetSelectedFeilds()
{
  int nbSelected = 0;
  if ( myTree->isEnabled() )
    for ( int i = 0; i < myTree->topLevelItemCount(); ++i )
    {
      QTreeWidgetItem* meshItem = myTree->topLevelItem( i );
      int iM = meshItem->data( 0, Qt::UserRole ).toInt();

      GEOM::ListOfFields& fields = (*myFields)[ iM ].first.inout();
      QString&            geoAss = (*myFields)[ iM ].second;

      int nbF = 0;
      QTreeWidgetItemIterator it ( meshItem, QTreeWidgetItemIterator::Checked );
      if ( *it == meshItem ) ++it;
      for ( ; *it ; ++it, ++nbSelected )
      {
        if ( !(*it)->parent() )
          break; // next mesh item

        int iF = (*it)->data( 0, Qt::UserRole ).toInt();
        if ( iF < 0 )
        {
          geoAss += (*it)->text(0)[1];
        }
        else
        {
          if ( nbF != iF )
            fields[ nbF ] = fields[ iF ];
          ++nbF;
        }
      }
      fields.length( nbF );
    }
  else
  {
    for ( int iF = 0; iF < myFields->count(); ++iF )
    {
      GEOM::ListOfFields& fields = (*myFields)[ iF ].first.inout();
      fields.length( 0 );
    }
  }
  return nbSelected;
}

//--------------------------------------------------------------------------------
/*!
 * \brief SLOT called when a tree item is checked
 */
void SMESHGUI_FieldSelectorWdg::onItemCheck(QTreeWidgetItem * item, int column)
{
  myTree->blockSignals( true );
  if ( !item->parent() ) // mesh item
  {
    Qt::CheckState st = item->checkState(0);
    QTreeWidgetItemIterator it( item );
    for ( ++it; *it ; ++it )
      if ( !(*it)->parent() )
        break; // next mesh item
      else
        (*it)->setCheckState( 0, st );
  }
  else // field item
  {
    // update CheckState of a parent mesh item
    QTreeWidgetItem* meshItem = item->parent();
    Qt::CheckState st = item->checkState(0);
    QTreeWidgetItemIterator it( meshItem );
    for ( ++it; *it ; ++it )
      if ( !(*it)->parent() )
      {
        break; // next mesh item
      }
      else if ( (*it)->checkState(0) != st )
      {
        st = Qt::PartiallyChecked;
        break;
      }
    meshItem->setCheckState( 0, st );
  }
  myTree->blockSignals( false );
}
