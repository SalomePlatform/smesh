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
//  File   : SMESHGUI_aParameterDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_aParameterDlg.h"
#include "SMESHGUI_aParameter.h"
#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_FunctionPreview.h"

#include "SUIT_Tools.h"
#include "SUIT_Desktop.h"

// QT Includes
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qvalidator.h>
#include <qlineedit.h>

using namespace std;

//======================================================================================
// function : SMESHGUI_aParameterDlg()
//
//  The dialog will by default be modal, unless you set 'modal' to
//  false when constructing dialog
//
//======================================================================================
SMESHGUI_aParameterDlg::SMESHGUI_aParameterDlg
                                       ( SMESHGUI* theModule,
					 std::list<SMESHGUI_aParameterPtr> params,
                                         QString                           title,
                                         bool                              modal)
: QDialog( SMESH::GetDesktop( theModule ), "MyParameterDialog", modal, WStyle_Customize |
           WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu),	   
  myParamList(params),
  mySMESHGUI(theModule)
{
  /* creating widgets */
  init();
  /* title */
  setCaption(title);

  /* Move widget on the botton right corner of main widget */
  SUIT_Tools::centerWidget(this, SMESH::GetDesktop( theModule ) );
}

//======================================================================================
// function : SMESHGUI_aParameterDlg::init()
// purpose  : creates dialog's layout
//======================================================================================
void SMESHGUI_aParameterDlg::init()
{
  setSizeGripEnabled(TRUE);

  QVBoxLayout* topLayout = new QVBoxLayout(this);
  topLayout->setMargin(11); topLayout->setSpacing(6);

  /***************************************************************/
  QGroupBox* GroupC1 = new QGroupBox(this, "GroupC1");
  GroupC1->setColumnLayout(0, Qt::Vertical);
  GroupC1->layout()->setSpacing(0);
  GroupC1->layout()->setMargin(0);
  QGridLayout* GroupC1Layout = new QGridLayout(GroupC1->layout());
  GroupC1Layout->setAlignment(Qt::AlignTop);
  GroupC1Layout->setSpacing(6);
  GroupC1Layout->setMargin(11);
  /* Spin boxes with labels */
  list<SMESHGUI_aParameterPtr>::iterator paramIt = myParamList.begin();
  int row;
  for( row = 0; paramIt != myParamList.end(); paramIt++ , row++)
  {
    SMESHGUI_aParameterPtr param = (*paramIt);
    QLabel * label = new QLabel(GroupC1, "TextLabel");
    GroupC1Layout->addWidget(label, row, 0);
    label->setText(param->Label());
    QWidget* aSpinWidget = param->CreateWidget( GroupC1 );
    if (aSpinWidget) {
      GroupC1Layout->addWidget(aSpinWidget, row, 1);
      aSpinWidget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
      aSpinWidget->setMinimumSize(150, 0);

      QString sig = param->sigValueChanged();
      if( !sig.isEmpty() /*&& param->GetType()!=SMESHGUI_aParameter::TABLE*/ )
        connect( aSpinWidget, sig.latin1(), this, SLOT( onValueChanged() ) );
      
      param->InitializeWidget(aSpinWidget);
      mySpinList.push_back(aSpinWidget);
      myLabelList.push_back(label);
    }
  }

  myPreview = new SMESHGUI_FunctionPreview( GroupC1 );
  GroupC1Layout->addWidget( myPreview, row, 1 );

  paramIt = myParamList.begin();
  std::list<QWidget*>::const_iterator anIt = mySpinList.begin();
  for( ; paramIt!=myParamList.end(); paramIt++, anIt++ )
  {
    (*paramIt)->TakeValue( *anIt );
    UpdateShown( *paramIt, *anIt );
    FunctionPreview( *paramIt, *anIt );
  }

  /***************************************************************/
  QGroupBox* GroupButtons = new QGroupBox(this, "GroupButtons");
  GroupButtons->setColumnLayout(0, Qt::Vertical);
  GroupButtons->layout()->setSpacing(0);
  GroupButtons->layout()->setMargin(0);
  QGridLayout* GroupButtonsLayout = new QGridLayout(GroupButtons->layout());
  GroupButtonsLayout->setAlignment(Qt::AlignTop);
  GroupButtonsLayout->setSpacing(6);
  GroupButtonsLayout->setMargin(11);
  /* Ok button */
  myButtonOk = new QPushButton(GroupButtons, "buttonOk");
  myButtonOk->setText(tr("SMESH_BUT_OK"));
  myButtonOk->setAutoDefault(TRUE);
  myButtonOk->setDefault(TRUE);
  GroupButtonsLayout->addWidget(myButtonOk, 0, 0);
  /* add spacer between buttons */
  GroupButtonsLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 1);
  /* Cancel button */
  myButtonCancel = new QPushButton(GroupButtons, "buttonCancel");
  myButtonCancel->setText(tr("SMESH_BUT_CANCEL"));
  myButtonCancel->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(myButtonCancel, 0, 2);

  /***************************************************************/
  topLayout->addWidget(GroupC1,      1 );
  topLayout->addWidget(GroupButtons, 0 );

  /* signals and slots connections */
  connect(myButtonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(myButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

//======================================================================================
// function : ~SMESHGUI_aParameterDlg()
// purpose  : Destructor
//======================================================================================
SMESHGUI_aParameterDlg::~SMESHGUI_aParameterDlg()
{
}

//=======================================================================
//function : ClickOnOk
//purpose  :
//=======================================================================
void SMESHGUI_aParameterDlg::ClickOnOk()
{
  if (!mySMESHGUI->isActiveStudyLocked()) {
    list<SMESHGUI_aParameterPtr>::iterator paramIt  = myParamList.begin();
    list<QWidget*>::iterator               widgetIt = mySpinList.begin();
    for (;
         paramIt != myParamList.end() && widgetIt != mySpinList.end();
         paramIt++ , widgetIt++)
      (*paramIt)->TakeValue(*widgetIt);

    accept();
  }
}

//=======================================================================
// function : Parameters()
// purpose  : return a list of parameters from a dialog box
//=======================================================================
bool SMESHGUI_aParameterDlg::Parameters( SMESHGUI* theModule, 
					 list<SMESHGUI_aParameterPtr> params,
                                         const char *aTitle)
{
  if (!params.empty()) {
    SMESHGUI_aParameterDlg *Dialog =
      new SMESHGUI_aParameterDlg( theModule, params, aTitle, TRUE);
    return (Dialog->exec() == QDialog::Accepted);
  }
  return false;
}

//=======================================================================
// function : FunctionPreview
// purpose  : 
//=======================================================================
void SMESHGUI_aParameterDlg::FunctionPreview( const SMESHGUI_aParameterPtr p, QWidget* w )
{
  if( !w || !w->isShown() )
    return;

  SMESHGUI_strParameter* str_param = dynamic_cast<SMESHGUI_strParameter*>( p.operator->() );
  SMESHGUI_tableParameter* tab_param = dynamic_cast<SMESHGUI_tableParameter*>( p.operator->() );
  SMESHGUI_boolParameter* bool_param = dynamic_cast<SMESHGUI_boolParameter*>( p.operator->() );
  if( str_param && str_param->needPreview() )
  {
    QString val; str_param->GetNewText( val );
    if( !val.isNull() )
      myPreview->setParams( val );
  }
  else if( tab_param && tab_param->needPreview() )
  {
    SMESH::double_array d;
    tab_param->data( d );
    myPreview->setParams( d );
  }
  else if( bool_param && bool_param->needPreview() )
  {
    int exp=0;
    bool_param->GetNewInt( exp );
    myPreview->setIsExp( exp );
  }
}

//=======================================================================
// function : onValueChanged
// purpose  : 
//=======================================================================
void SMESHGUI_aParameterDlg::onValueChanged()
{
  if( sender()->inherits( "QWidget" ) )
  {
    QWidget* w = ( QWidget* )sender();


    std::list<QWidget*>::const_iterator anIt = mySpinList.begin(),
                                        aLast = mySpinList.end();
    std::list<SMESHGUI_aParameterPtr>::const_iterator aPIt = myParamList.begin();
    for( ; anIt!=aLast; anIt++, aPIt++ )
      if( *anIt == w )
      {
        (*aPIt)->TakeValue( w );
        UpdateShown( *aPIt, w );
	FunctionPreview( *aPIt, w );
        break;
      }
  }
}

//=======================================================================
// function : onValueChanged
// purpose  :
//=======================================================================
void SMESHGUI_aParameterDlg::UpdateShown( const SMESHGUI_aParameterPtr param, QWidget* w )
{
  SMESHGUI_dependParameter* depPar = dynamic_cast<SMESHGUI_enumParameter*>( param.get() );
  if( !depPar )
    depPar = dynamic_cast<SMESHGUI_boolParameter*>( param.get() );

  if( !depPar )
    return;

  SMESHGUI_dependParameter::ShownMap& map = depPar->shownMap();
  if( map.isEmpty() )
    return;

  int val;
  depPar->TakeValue( w );
  depPar->GetNewInt( val );
  bool hasValue = map.contains( val );

  std::list<QWidget*>::const_iterator anIt = mySpinList.begin(),
                                      aLast = mySpinList.end(),
                                      aLIt = myLabelList.begin();
  std::list<SMESHGUI_aParameterPtr>::iterator aPIt = myParamList.begin();
  bool preview = false;
  for( int i=0; anIt!=aLast; anIt++, aLIt++, i++, aPIt++ )
  {
    bool shown = hasValue && map[ val ].contains( i );
    (*anIt)->setShown( shown );
    (*aLIt)->setShown( shown );
    if( shown )
    {
      SMESHGUI_strParameter* str_param = dynamic_cast<SMESHGUI_strParameter*>( (*aPIt).operator->() );
      SMESHGUI_tableParameter* tab_param = dynamic_cast<SMESHGUI_tableParameter*>( (*aPIt).operator->() );
      preview = preview || ( str_param && str_param->needPreview() ) || ( tab_param && tab_param->needPreview() );
    }
  }
  myPreview->setShown( preview );
}
