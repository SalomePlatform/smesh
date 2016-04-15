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

// File   : StdMeshersGUI_LayerDistributionParamWdg.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "StdMeshersGUI_LayerDistributionParamWdg.h"

#include <SMESHGUI.h>
#include <SMESHGUI_HypothesesUtils.h>
#include <SMESHGUI_Hypotheses.h>

// SALOME GUI includes
#include <SalomeApp_Tools.h>

// Qt includes
#include <QPushButton>
#include <QCursor>
#include <QMenu>
#include <QDialog>
#include <QHBoxLayout>

#define SPACING 6

//================================================================================
/*!
 * \brief Constructor initialized by filter
 * \param f - object filter
 */
//================================================================================

StdMeshersGUI_LayerDistributionParamWdg
::StdMeshersGUI_LayerDistributionParamWdg(SMESH::SMESH_Hypothesis_ptr holderHyp,
                                          SMESH::SMESH_Hypothesis_ptr distribHyp,
                                          const QString&              name,
                                          QDialog*                    dlg):
  QWidget(), myDlg( dlg ), myName(name)
{
  myHolderHyp = SMESH::SMESH_Hypothesis::_duplicate( holderHyp );
  init();
  set( distribHyp );
}

//================================================================================
/*!
 * \brief initialize fields with hypothesis
  * \param hyp - hypothesis
 */
//================================================================================

void StdMeshersGUI_LayerDistributionParamWdg::set(SMESH::SMESH_Hypothesis_ptr hyp)
{
  myHyp = SMESH::SMESH_Hypothesis::_nil();
  if ( !CORBA::is_nil( hyp )) {
    myHyp = SMESH::SMESH_Hypothesis::_duplicate( hyp );
    myHyp->SetHolderHypothesis( myHolderHyp );
    myEditButton->setEnabled( true );
    myCreateButton->setText( tr("CHANGE_TYPE"));
    myParamValue = hyp->GetName();
  }
  else {
    myEditButton->setEnabled( false );
    myCreateButton->setText( tr("CREATE"));
    myParamValue = "";
  }
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

StdMeshersGUI_LayerDistributionParamWdg::~StdMeshersGUI_LayerDistributionParamWdg()
{
}

//================================================================================
/*!
 * \brief Create a layout, initialize fields
 */
//================================================================================

void StdMeshersGUI_LayerDistributionParamWdg::init()
{
  QHBoxLayout* aHBox = new QHBoxLayout( this );
  aHBox->setMargin( 0 );
  aHBox->setSpacing( SPACING );

  mySMESHGUI = SMESHGUI::GetSMESHGUI();

  myCreateButton = new QPushButton( this );
  myCreateButton->setObjectName( "createBut" );
  myCreateButton->setMinimumWidth(100);

  myEditButton   = new QPushButton( tr("EDIT"), this );
  myEditButton->setObjectName( "editBut" );
  myEditButton->setMinimumWidth(100);

  myHypTypePopup = new QMenu( this );

  // Add to pop-up hypotheses of "Regular_1D" algo
  HypothesisData* algoData = SMESH::GetHypothesisData( "Regular_1D" );
  myHypTypes = SMESH::GetAvailableHypotheses( false, 1 );
  QStringList::const_iterator anIter = myHypTypes.begin();
  for ( ; anIter != myHypTypes.end(); ++anIter )
  {
    HypothesisData* hypData = SMESH::GetHypothesisData( *anIter );
    bool bidon;
    if ( SMESH::IsAvailableHypothesis( algoData, hypData->TypeName, bidon ))
      myHypTypePopup->addAction( hypData->Label );
  }

  aHBox->addWidget( myCreateButton );
  aHBox->addStretch(5);
  aHBox->addWidget( myEditButton );

  connect( myCreateButton, SIGNAL(clicked()), SLOT(onCreate()));
  connect( myEditButton,   SIGNAL(clicked()), SLOT(onEdit()));
  connect( myHypTypePopup, SIGNAL(triggered( QAction* ) ), SLOT( onHypTypePopup( QAction* ) ) );
}

//================================================================================
/*!
 * \brief Create a new hyp of selected type
  * \param int - selected type index
 */
//================================================================================

void StdMeshersGUI_LayerDistributionParamWdg::onHypTypePopup( QAction* a )
{
  SMESH::SMESH_Gen_var gen = mySMESHGUI->GetSMESHGen();

  // avoid publishing a new 1D hyp
  gen->SetCurrentStudy( SALOMEDS::Study::_nil() );

  // create a hyp
  HypothesisData* aHypData = 0;
  QStringList::const_iterator anIter = myHypTypes.begin();

  for ( ; !aHypData && anIter != myHypTypes.end(); ++anIter )
  {
    HypothesisData* hypData = SMESH::GetHypothesisData( *anIter );
    if ( a->text() == hypData->Label )
      aHypData = hypData;
  }
  QString aServLib = aHypData->ServerLibName;
  QString aHypType = aHypData->TypeName;
  try {
    set( gen->CreateHypothesis(aHypType.toLatin1().data(), aServLib.toLatin1().data()));
  }
  catch (const SALOME::SALOME_Exception & S_ex) {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  }

  // restore current study
  mySMESHGUI->GetSMESHGen();

  onEdit();
}

//================================================================================
/*!
 * \brief Show popup with available types
 */
//================================================================================

void StdMeshersGUI_LayerDistributionParamWdg::onCreate()
{
  myHypTypePopup->exec( QCursor::pos() );
}

//================================================================================
/*!
 * \brief Edit hypothesis
 */
//================================================================================

void StdMeshersGUI_LayerDistributionParamWdg::onEdit()
{
  if ( myHyp->_is_nil() )
    return;

  CORBA::String_var hypType = myHyp->GetName();
  // BUG 0020378
  SMESHGUI_GenericHypothesisCreator* editor = SMESH::GetHypothesisCreator(hypType.in());
  if ( !editor ) return;

  if ( myDlg )
    myDlg->hide();

  try {
    QWidget* parent = this;
    if ( myDlg )
      parent = myDlg->parentWidget();
    editor->edit( myHyp, myName, parent, this, SLOT( onEdited( int ) ) );
  }
  catch(...)
  {
  }
}

void StdMeshersGUI_LayerDistributionParamWdg::onEdited( int result )
{
  if ( myDlg )
    myDlg->show();
}
