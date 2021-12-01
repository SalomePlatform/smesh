// Copyright (C) 2011-2021  CEA/DEN, EDF R&D
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

#include "SMESHGUI_HomardAdaptDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_HomardBoundaryDlg.h"
#include "SMESHGUI_HomardUtils.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESH_TryCatch.hxx"

#include <SalomeApp_Tools.h>
#include <SalomeApp_Module.h>
#include <SalomeApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_FileDlg.h>
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SVTK_ViewWindow.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QButtonGroup>

#include <utilities.h>

#ifdef WIN32
#include <direct.h>
#endif

using namespace std;

// La gestion des repertoires
#ifndef CHDIR
  #ifdef WIN32
    #define CHDIR _chdir
  #else
    #define CHDIR chdir
  #endif
#endif

const int SPACING = 6;            // layout spacing
const int MARGIN  = 9;            // layout margin

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================
SMESHGUI_HomardAdaptDlg::SMESHGUI_HomardAdaptDlg(SMESHHOMARD::HOMARD_Gen_ptr myHomardGen0)
  : QDialog(SMESHGUI::desktop())
{
  MESSAGE("Debut du constructeur de SMESHGUI_HomardAdaptDlg");
  myHomardGen = SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen0);
  //myHomardGen = SMESHGUI::GetSMESHGen()->CreateHOMARD_ADAPT();
  myHomardGen->Register();

  setModal(false);
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle( tr( "ADAPT_WITH_HOMARD" ) );
  setSizeGripEnabled( true );

  QTabWidget* myTabWidget = new QTabWidget( this );

  // Arguments
  myArgs = new SMESHGUI_HomardAdaptArguments(myTabWidget);

  // Advanced options
  myAdvOpt = new SMESHGUI_HomardAdaptAdvanced(myTabWidget);

  myTabWidget->addTab( myArgs, tr( "Args" ) );
  myTabWidget->addTab( myAdvOpt, tr( "ADVOP" ) );

  myAdvOpt->logGroupBox               ->setTitle(tr( "LOG_GROUP_TITLE" ));
  myAdvOpt->workingDirectoryLabel     ->setText (tr( "WORKING_DIR" ));
  myAdvOpt->workingDirectoryPushButton->setText (tr( "SELECT_DIR" ));
  myAdvOpt->verboseLevelLabel         ->setText (tr( "VERBOSE_LEVEL" ));
  myAdvOpt->logInFileCheck            ->setText (tr( "LOG_IN_FILE" ));
  myAdvOpt->removeLogOnSuccessCheck   ->setText (tr( "REMOVE_LOG_ON_SUCCESS" ));
  myAdvOpt->keepWorkingFilesCheck     ->setText (tr( "KEEP_WORKING_FILES" ));

  //myAdvOpt->logInFileCheck->setChecked(true);
  //myAdvOpt->removeLogOnSuccessCheck->setChecked(false);

  // Working directory
  QString aWorkingDir = QDir::tempPath();
  char *aTmp_dir = getenv("SALOME_TMP_DIR");
  if (aTmp_dir != NULL) {
    QDir aTmpDir (aTmp_dir);
    if (aTmpDir.exists()) {
      aWorkingDir = aTmpDir.absolutePath();
    }
  }
  myAdvOpt->workingDirectoryLineEdit->setText(aWorkingDir);

  // Out med file and/or mesh publication
  myArgs->myOutMedFileChk->setChecked(true);
  myArgs->myOutPublishChk->setChecked(true);

  // buttons
  QHBoxLayout* btnLayout = new QHBoxLayout;
  btnLayout->setSpacing( 6 );
  btnLayout->setMargin( 0 );

  buttonOk = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), this);
  buttonOk->setAutoDefault(false);
  btnLayout->addWidget(buttonOk);
  btnLayout->addStretch( 10 );

  buttonApply = new QPushButton(tr("SMESH_BUT_APPLY"), this);
  buttonApply->setAutoDefault(false);
  btnLayout->addWidget(buttonApply);
  btnLayout->addStretch( 10 );

  buttonCancel = new QPushButton(tr( "SMESH_BUT_CANCEL" ), this);
  buttonCancel->setAutoDefault(false);
  btnLayout->addWidget(buttonCancel);
  btnLayout->addStretch( 10 );

  buttonHelp = new QPushButton(tr( "SMESH_BUT_HELP" ), this);
  buttonHelp->setAutoDefault(false);
  btnLayout->addWidget(buttonHelp);

  // dialog layout
  QVBoxLayout* l = new QVBoxLayout ( this );
  l->setMargin( 9 );
  l->setSpacing( 6 );
  l->addWidget( myTabWidget );
  l->addStretch();
  l->addLayout( btnLayout );

  // dialog name and size
  resize(600, 1000);
  QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
  setSizePolicy(sizePolicy);
  setMinimumSize(QSize(500, 320));
  setSizeIncrement(QSize(1, 1));
  setBaseSize(QSize(600, 600));
  setAutoFillBackground(true);

  SetBoundaryNo();
  InitConnect();

  myArgs->GBBoundaryC->setVisible(0);
  myArgs->GBBoundaryN->setVisible(0);
  myArgs->GBBoundaryA->setVisible(0);
  myArgs->GBBoundaryD->setVisible(0);

  adjustSize();

  //MESSAGE("Fin du constructeur de SMESHGUI_HomardAdaptDlg");
}

//=================================================================================
// function : ~SMESHGUI_HomardAdaptDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_HomardAdaptDlg::~SMESHGUI_HomardAdaptDlg()
{
  // no need to delete child widgets, Qt does it all for us
  myHomardGen->DeleteCase();
}

//=================================================================================
// function : InitConnect
// purpose  : 
//=================================================================================
void SMESHGUI_HomardAdaptDlg::InitConnect()
{
  connect( myArgs->mySelectInMedFileButton, SIGNAL(pressed()), this, SLOT(SetFileName()));

  connect( myArgs->RBBoundaryNo,      SIGNAL(clicked()), this, SLOT(SetBoundaryNo()));
  connect( myArgs->RBBoundaryCAO,     SIGNAL(clicked()), this, SLOT(SetBoundaryCAO()));
  connect( myArgs->RBBoundaryNonCAO,  SIGNAL(clicked()), this, SLOT(SetBoundaryNonCAO()));

  connect( myArgs->PBBoundaryCAONew,  SIGNAL(pressed()), this, SLOT(PushBoundaryCAONew()));
  connect( myArgs->PBBoundaryCAOEdit, SIGNAL(pressed()), this, SLOT(PushBoundaryCAOEdit()) );
  connect( myArgs->PBBoundaryCAOHelp, SIGNAL(pressed()), this, SLOT(PushBoundaryCAOHelp()) );
  connect( myArgs->CBBoundaryD,       SIGNAL(stateChanged(int)), this, SLOT(SetBoundaryD()));
  connect( myArgs->PBBoundaryDiNew,   SIGNAL(pressed()), this, SLOT(PushBoundaryDiNew()));
  connect( myArgs->PBBoundaryDiEdit,  SIGNAL(pressed()), this, SLOT(PushBoundaryDiEdit()) );
  connect( myArgs->PBBoundaryDiHelp,  SIGNAL(pressed()), this, SLOT(PushBoundaryDiHelp()) );
  connect( myArgs->CBBoundaryA,       SIGNAL(stateChanged(int)), this, SLOT(SetBoundaryA()));
  connect( myArgs->PBBoundaryAnNew,   SIGNAL(pressed()), this, SLOT(PushBoundaryAnNew()));
  connect( myArgs->PBBoundaryAnEdit,  SIGNAL(pressed()), this, SLOT(PushBoundaryAnEdit()) );
  connect( myArgs->PBBoundaryAnHelp,  SIGNAL(pressed()), this, SLOT(PushBoundaryAnHelp()) );

  connect( buttonOk,       SIGNAL(pressed()), this, SLOT(PushOnOK()));
  connect( buttonApply,    SIGNAL(pressed()), this, SLOT(PushOnApply()));
  connect( buttonCancel,   SIGNAL(pressed()), this, SLOT(close()));
  connect( buttonHelp,     SIGNAL(pressed()), this, SLOT(PushOnHelp()));

  connect(myArgs, SIGNAL(updateSelection()), this, SLOT(updateSelection()));
}

//=================================================================================
// function : InitBoundarys
// purpose  : Initialisation des menus avec les frontieres deja enregistrees
//=================================================================================
void SMESHGUI_HomardAdaptDlg::InitBoundarys()
{
  MESSAGE("InitBoundarys");
  //myArgs->TWBoundary->clearContents();
  //myArgs->TWBoundary->clear();
  // Pour les frontieres analytiques : la colonne des groupes
  SMESHHOMARD::ListGroupType_var _listeGroupesCas = myCase->GetGroups();
  QTableWidgetItem *__colItem = new QTableWidgetItem();
  __colItem->setText(tr(""));
  myArgs->TWBoundary->setHorizontalHeaderItem(0, __colItem);
  for ( int i = 0; i < (int)_listeGroupesCas->length(); i++ ) {
    myArgs->TWBoundary->insertRow(i);
    myArgs->TWBoundary->setItem( i, 0, new QTableWidgetItem(QString((_listeGroupesCas)[i]).trimmed()));
    myArgs->TWBoundary->item( i, 0 )->setFlags(Qt::ItemIsEnabled |Qt::ItemIsSelectable );
  }
  // Pour les frontieres CAO : la liste a saisir
  // Pour les frontieres discretes : la liste a saisir
  // Pour les frontieres analytiques : les colonnes de chaque frontiere
  SMESHHOMARD::HOMARD_Boundary_var myBoundary ;
  SMESHHOMARD::listeBoundarys_var  mesBoundarys = myHomardGen->GetAllBoundarysName();
  //MESSAGE("Nombre de frontieres enregistrees : "<<mesBoundarys->length());
  for (int i=0; i < (int)mesBoundarys->length(); i++) {
    myBoundary = myHomardGen->GetBoundary(mesBoundarys[i]);
    int type_obj = myBoundary->GetType() ;
    if ( type_obj==-1 )     { myArgs->CBBoundaryCAO->addItem(QString(mesBoundarys[i])); }
    else if ( type_obj==0 ) { myArgs->CBBoundaryDi->addItem(QString(mesBoundarys[i])); }
    else                    { AddBoundaryAn(QString(mesBoundarys[i])); }
  }
  // Ajustement
  myArgs->TWBoundary->resizeColumnsToContents();
  myArgs->TWBoundary->resizeRowsToContents();
  myArgs->TWBoundary->clearSelection();
}

//=================================================================================
// function : CheckCase
// purpose  : 
//=================================================================================
bool SMESHGUI_HomardAdaptDlg::CheckCase(bool fixCase)
{
  MESSAGE("CheckCase");

  QString aWorkingDir = myAdvOpt->workingDirectoryLineEdit->text().trimmed();
  if (aWorkingDir == QString("")) {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_CASE_DIRECTORY_1") );
    return false;
  }

  if (CHDIR(aWorkingDir.toStdString().c_str()) != 0) {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_CASE_DIRECTORY_3") );
    return false;
  }

  QString aMeshName, aFileName;
  if (myArgs->myInMedFileRadio->isChecked()) {
    aFileName = myArgs->mySelectInMedFileLineEdit->text().trimmed();
    if (aFileName == QString("")) {
      QMessageBox::critical(0, QObject::tr("HOM_ERROR"), QObject::tr("HOM_CASE_MESH"));
      return false;
    }

    // In mesh name
    aMeshName = SMESH_HOMARD_QT_COMMUN::LireNomMaillage(aFileName);
    if (aMeshName == "") {
      QMessageBox::critical(0, QObject::tr("HOM_ERROR"), QObject::tr("HOM_MED_FILE_2"));
      return false;
    }
  }
  else {
    aMeshName = myArgs->myInBrowserObject->text();
    if (aMeshName == "" || myMesh->_is_nil()) {
      QMessageBox::critical(0, QObject::tr("HOM_ERROR"),
                            QObject::tr("Mesh object is not selected"));
      return false;
    }
  }

  // On verifie qu'un groupe n'est pas associe a deux frontieres differentes
  if (myArgs->CBBoundaryA->isChecked()) {
    QStringList ListeGroup;
    QString NomGroup;
    int nbcol = myArgs->TWBoundary->columnCount();
    int nbrow = myArgs->TWBoundary->rowCount();
    for ( int col=1; col< nbcol; col++) {
      for ( int row=0; row< nbrow; row++) {
        if ( myArgs->TWBoundary->item( row, col )->checkState() ==  Qt::Checked ) {
          // Group name
          NomGroup = QString(myArgs->TWBoundary->item(row, 0)->text()) ;
          for ( int nugr = 0 ; nugr < ListeGroup.size(); nugr++) {
            if ( NomGroup == ListeGroup[nugr] ) {
              QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                        QObject::tr("HOM_CASE_GROUP").arg(NomGroup) );
              return false;
            }
          }
          ListeGroup.insert(0, NomGroup );
        }
      }
    }
  }

  if (!fixCase) return true;

  // Creation du cas
  if (myCase->_is_nil()) {
    try {
      if (myArgs->myInMedFileRadio->isChecked()) {
        // create case from MED file
        myCase = myHomardGen->CreateCase
          (CORBA::string_dup(aMeshName.toStdString().c_str()),
           CORBA::string_dup(aFileName.toStdString().c_str()),
           aWorkingDir.toStdString().c_str());
      }
      else {
        // create case from SMESH_Mesh
        myCase = myHomardGen->CreateCaseOnMesh
          (CORBA::string_dup(aMeshName.toStdString().c_str()),
           myMesh,
           aWorkingDir.toStdString().c_str());
      }
    }
    catch( SALOME::SALOME_Exception& S_ex ) {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                             QObject::tr(CORBA::string_dup(S_ex.details.text)) );
      return false;
    }
    // Prevent changing case data
    myArgs->myInMedFileRadio->setEnabled(false);
    myArgs->myInBrowserRadio->setEnabled(false);
    myArgs->mySelectInMedFileLineEdit->setReadOnly(true);
    myArgs->mySelectInMedFileButton->setEnabled(false);
    myArgs->myInBrowserObject->setReadOnly(true);
    myAdvOpt->workingDirectoryLineEdit->setReadOnly(true);
    myAdvOpt->workingDirectoryPushButton->setEnabled(false);
    InitBoundarys();
  }

  // Menage des eventuelles frontieres deja enregistrees
  myCase->SupprBoundaryGroup();

  return true;
}

//=================================================================================
// function : PushOnApply
// purpose  : 
//=================================================================================
bool SMESHGUI_HomardAdaptDlg::PushOnApply()
{
  MESSAGE("PushOnApply");

  // Check data, create Case if not yet
  if (!CheckCase(true))
    return false;

  // Create boundaries
  if (myArgs->RBBoundaryCAO->isChecked()) {
    QString monBoundaryCAOName = myArgs->CBBoundaryCAO->currentText();
    if (monBoundaryCAOName != "" ) {
      myCase->AddBoundary(monBoundaryCAOName.toStdString().c_str());
    }
  }
  if (myArgs->CBBoundaryD->isChecked()) {
    QString monBoundaryDiName = myArgs->CBBoundaryDi->currentText();
    if (monBoundaryDiName != "" ) {
      myCase->AddBoundary(monBoundaryDiName.toStdString().c_str());
    }
  }
  if (myArgs->CBBoundaryA->isChecked()) {
    QString NomGroup;
    int nbcol = myArgs->TWBoundary->columnCount();
    int nbrow = myArgs->TWBoundary->rowCount();
    for ( int col = 1; col < nbcol; col++) {
      for ( int row = 0; row < nbrow; row++) {
        if ( myArgs->TWBoundary->item( row, col )->checkState() == Qt::Checked ) {
          // Nom du groupe
          NomGroup = QString(myArgs->TWBoundary->item(row, 0)->text()) ;
          // Nom de la frontiere
          QTableWidgetItem *__colItem = myArgs->TWBoundary->horizontalHeaderItem(col);
          myCase->AddBoundaryGroup(QString(__colItem->text()).toStdString().c_str(),
                                   NomGroup.toStdString().c_str());
        }
      }
    }
  }

  // Output MED and MESH parameters
  myHomardGen->SetKeepMedOUT(myArgs->myOutMedFileChk->isChecked());
  myHomardGen->SetPublishMeshOUT(myArgs->myOutPublishChk->isChecked());
  QString anOutMeshName = myArgs->myOutMeshNameLineEdit->text();
  if (anOutMeshName.isEmpty()) anOutMeshName = "DEFAULT_MESH_NAME";
  myHomardGen->SetMeshNameOUT(anOutMeshName.toStdString().c_str());

  std::string aMeshFileOUT;
  if (myArgs->myOutMedFileChk->isChecked()) {
    QString anOutMed = myArgs->mySelectOutMedFileLineEdit->text();
    if (anOutMed.isEmpty()) {
      // store in working directory and with default name
      QString aWorkingDir = myAdvOpt->workingDirectoryLineEdit->text().trimmed();
      QFileInfo aFileInfo (QDir(aWorkingDir), "Uniform_R.med");
      anOutMed = aFileInfo.absoluteFilePath();
      // show it
      myArgs->mySelectOutMedFileLineEdit->setText(anOutMed);
    }
    else {
      QFileInfo aFileInfo (anOutMed);
      anOutMed = aFileInfo.absoluteFilePath();
    }
    aMeshFileOUT = anOutMed.toStdString();
  }
  else {
    // Set file name without path for it to be created in current directory
    // (it will be iteration's dir, and it will be destroyed after)
    aMeshFileOUT = "Uniform_R.med";
  }
  myHomardGen->SetMeshFileOUT(aMeshFileOUT.c_str());

  // Conformity type
  myHomardGen->SetConfType(myArgs->RBConforme->isChecked() ? 0 : 1);

  // Advanced options
  myHomardGen->SetVerboseLevel(myAdvOpt->verboseLevelSpin->value());
  myHomardGen->SetKeepWorkingFiles(myAdvOpt->keepWorkingFilesCheck->isChecked());
  myHomardGen->SetLogInFile(myAdvOpt->logInFileCheck->isChecked());
  myHomardGen->SetRemoveLogOnSuccess(myAdvOpt->removeLogOnSuccessCheck->isChecked());

  // Log file
  if (myAdvOpt->logInFileCheck->isChecked()) {
    // Write log file in the working dir
    QString aLogBaseName;
    if (myArgs->myInMedFileRadio->isChecked()) {
      // Name of log file will be "<name_of_input_med_file>_Uniform_R.log"
      QString aMedFileIn = myArgs->mySelectInMedFileLineEdit->text().trimmed();
      QFileInfo aFileInfoIn (aMedFileIn);
      aLogBaseName = aFileInfoIn.fileName();
    }
    else {
      // Name of log file will be "SMESH_Mesh_<name_of_input_mesh>_Uniform_R.log"
      aLogBaseName = "SMESH_Mesh_";
      aLogBaseName += myArgs->myInBrowserObject->text();
    }
    QString aWorkingDir = myAdvOpt->workingDirectoryLineEdit->text().trimmed();
    QFileInfo aFileInfo (QDir(aWorkingDir), aLogBaseName + "_Uniform_R.log");
    QString anOutLog = aFileInfo.absoluteFilePath();
    MESSAGE("myHomardGen->SetLogFile(" << anOutLog.toStdString().c_str() << ")");
    myHomardGen->SetLogFile(anOutLog.toStdString().c_str());
  }

  // Compute and publish
  bool isSuccess = true;
  try {
    SUIT_OverrideCursor aWaitCursor;
    isSuccess = myHomardGen->Compute() == 0;
  }
  catch( SALOME::SALOME_Exception& S_ex ) {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                           QObject::tr(CORBA::string_dup(S_ex.details.text)) );
    isSuccess = false;
  }

  // Update Object Browser
  if (isSuccess) {
    SMESHGUI::GetSMESHGUI()->updateObjBrowser();

    // Clean case, as it is deleted after successful Compute
    myCase = SMESHHOMARD::HOMARD_Cas::_nil();
  }

  // Enable new case data selection
  myArgs->myInMedFileRadio->setEnabled(true);
  myArgs->myInBrowserRadio->setEnabled(true);
  myArgs->mySelectInMedFileLineEdit->setReadOnly(false);
  myArgs->mySelectInMedFileButton->setEnabled(true);
  myArgs->myInBrowserObject->setReadOnly(false);
  myAdvOpt->workingDirectoryLineEdit->setReadOnly(false);
  myAdvOpt->workingDirectoryPushButton->setEnabled(true);

  return isSuccess;
}

//=================================================================================
// function : PushOnOK
// purpose  : 
//=================================================================================
void SMESHGUI_HomardAdaptDlg::PushOnOK()
{
  bool bOK = PushOnApply();
  if ( bOK ) this->close();
}

void SMESHGUI_HomardAdaptDlg::PushOnHelp()
{
  SMESH::ShowHelpFile("adaptation.html#_homard_adapt_anchor");
}

void SMESHGUI_HomardAdaptDlg::updateSelection()
{
  LightApp_SelectionMgr *selMgr = SMESHGUI::selectionMgr();
  disconnect(selMgr, 0, this, 0);
  selMgr->clearFilters();

  if (!myArgs->myInBrowserRadio->isChecked())
    return;

  SMESH::SetPointRepresentation(false);
  if (SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow())
    aViewWindow->SetSelectionMode(ActorSelection);
  if (myArgs->myInBrowserRadio->isChecked()) {
    connect(selMgr, SIGNAL(currentSelectionChanged()), this, SLOT(selectionChanged()));
    selectionChanged();
  }
}

void SMESHGUI_HomardAdaptDlg::selectionChanged()
{
  if (!myArgs->myInBrowserRadio->isChecked())
    return;

  //SUIT_OverrideCursor aWaitCursor;
  LightApp_SelectionMgr *selMgr = SMESHGUI::selectionMgr();

  // get selected mesh
  SALOME_ListIO aList;
  selMgr->selectedObjects(aList);
  QString aMeshName = "";

  if (aList.Extent() == 1) {
    Handle(SALOME_InteractiveObject) IO = aList.First();
    myMesh = SMESH::GetMeshByIO(IO);
    SMESH::GetNameOfSelectedIObjects(selMgr, aMeshName);
    if (aMeshName.isEmpty()) aMeshName = " ";
    else                     aMeshName = aMeshName.trimmed();
  }
  else {
    myMesh = SMESH::SMESH_Mesh::_nil();
  }

  myArgs->myInBrowserObject->setText(aMeshName);

  // Out mesh name default value
  myArgs->myOutMeshNameLineEdit->setText(aMeshName + "_Uniform_R");

  // Output med file default value
  // Construct it from Input mesh name and working directory
  //if (myArgs->myOutMedFileChk->isChecked()) {
  if (aMeshName.isEmpty()) {
    myArgs->mySelectOutMedFileLineEdit->setText("");
  }
  else {
    QString aWorkingDir = myAdvOpt->workingDirectoryLineEdit->text().trimmed();
    QFileInfo aFileInfo (QDir(aWorkingDir), aMeshName + QString("_Uniform_R.med"));
    for (int ii = 1; aFileInfo.exists(); ii++) {
      QString anUniqueName = QString("%1_Uniform_R_%2.med").arg(aMeshName).arg(ii);
      aFileInfo.setFile(QDir(aWorkingDir), anUniqueName);
    }
    myArgs->mySelectOutMedFileLineEdit->setText(aFileInfo.absoluteFilePath());
  }
  //}

  // Check data
  if (!aMeshName.isEmpty())
    CheckCase(false);
}

void SMESHGUI_HomardAdaptDlg::SetFileName()
{
  // Input med file
  QString fileName0 = myArgs->mySelectInMedFileLineEdit->text().trimmed();
  QString fileName = SMESH_HOMARD_QT_COMMUN::PushNomFichier(false, QString("med"));
  //SUIT_OverrideCursor aWaitCursor;
  if (fileName.isEmpty()) {
    fileName = fileName0;
    if (fileName.isEmpty()) return;
  }
  QFileInfo aFileInInfo (fileName);
  fileName = aFileInInfo.absoluteFilePath();
  myArgs->mySelectInMedFileLineEdit->setText(fileName);

  // Out mesh name default value
  QString aMeshName = SMESH_HOMARD_QT_COMMUN::LireNomMaillage(fileName);
  myArgs->myOutMeshNameLineEdit->setText(aMeshName + "_Uniform_R");

  // Output med file default value
  // Construct it from Input med file name and path
  //if (myArgs->myOutMedFileChk->isChecked()) {
  std::string fname = fileName.toStdString();
  size_t lastdot = fname.find_last_of(".");
  if (lastdot != std::string::npos)
    fname = fname.substr(0, lastdot);
  QString fileNameOut = fname.c_str();
  QFileInfo aFileInfo (fileNameOut + QString("_Uniform_R.med"));
  for (int ii = 1; aFileInfo.exists(); ii++) {
    QString anUniqueName = QString("%1_Uniform_R_%2.med").arg(fileNameOut).arg(ii);
    aFileInfo.setFile(anUniqueName);
  }
  myArgs->mySelectOutMedFileLineEdit->setText(aFileInfo.absoluteFilePath());
  //}

  // Check data
  CheckCase(false);
}

// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::SetBoundaryNo()
{
  myArgs->GBBoundaryC->setVisible(0);
  myArgs->GBBoundaryN->setVisible(0);
  adjustSize();
}

// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::SetBoundaryCAO()
{
  if (CheckCase(true)) {
    myArgs->GBBoundaryC->setVisible(1);
    myArgs->GBBoundaryN->setVisible(0);
    resize(600, 550);
    //adjustSize();
  }
  else {
    myArgs->RBBoundaryNo->click();
  }
}
// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::SetBoundaryNonCAO()
{
  myArgs->GBBoundaryC->setVisible(0);
  myArgs->GBBoundaryN->setVisible(1);
  int aH = 550;
  if (myArgs->GBBoundaryD->isVisible()) aH += 50;
  if (myArgs->GBBoundaryA->isVisible()) aH += 150;
  resize(600, aH);
  //adjustSize();
}
// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::AddBoundaryCAO(QString newBoundary)
// ------------------------------------------------------------------------
{
  myArgs->CBBoundaryCAO->insertItem(0,newBoundary);
  myArgs->CBBoundaryCAO->setCurrentIndex(0);
}
// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::PushBoundaryCAONew()
// ------------------------------------------------------------------------
{
   SMESH_CreateBoundaryCAO *BoundaryDlg = new SMESH_CreateBoundaryCAO
     (this, true, SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen), "Case_1", "");
   BoundaryDlg->show();
}
// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::PushBoundaryCAOEdit()
// ------------------------------------------------------------------------
{
  if (myArgs->CBBoundaryCAO->currentText() == QString(""))  return;
  SMESH_EditBoundaryCAO *BoundaryDlg = new SMESH_EditBoundaryCAO
    (this, true, SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen),
     "Case_1", myArgs->CBBoundaryCAO->currentText());
  BoundaryDlg->show();
}

// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::PushBoundaryCAOHelp()
{
  SMESH::ShowHelpFile(QString("homard_create_boundary.html#cao-boundary"));
}

// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::SetBoundaryD()
{
  MESSAGE("Debut de SetBoundaryD ");
  if (myArgs->CBBoundaryD->isChecked()) {
    bool bOK = CheckCase(true);
    if (bOK) {
      myArgs->GBBoundaryD->setVisible(1);
    }
    else {
      myArgs->GBBoundaryD->setVisible(0);
      myArgs->CBBoundaryD->setChecked(0);
      myArgs->CBBoundaryD->setCheckState(Qt::Unchecked);
    }
  }
  else {
    myArgs->GBBoundaryD->setVisible(0);
  }

  //myArgs->mySelectInMedFileLineEdit->setReadOnly(true);
  //myArgs->mySelectInMedFileButton->hide();

  int aH = 550;
  if (myArgs->GBBoundaryD->isVisible()) aH += 50;
  if (myArgs->GBBoundaryA->isVisible()) aH += 150;
  resize(600, aH);
  //adjustSize();
}
// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::AddBoundaryDi(QString newBoundary)
// ------------------------------------------------------------------------
{
  myArgs->CBBoundaryDi->insertItem(0,newBoundary);
  myArgs->CBBoundaryDi->setCurrentIndex(0);
}
// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::PushBoundaryDiNew()
// ------------------------------------------------------------------------
{
   SMESH_CreateBoundaryDi *BoundaryDlg = new SMESH_CreateBoundaryDi(this, true,
                SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen), "Case_1", "");
   BoundaryDlg->show();
}
// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::PushBoundaryDiEdit()
// ------------------------------------------------------------------------
{
  if (myArgs->CBBoundaryDi->currentText() == QString(""))  return;
  SMESH_EditBoundaryDi *BoundaryDlg = new SMESH_EditBoundaryDi
    (this, true, SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen),
     "Case_1", myArgs->CBBoundaryDi->currentText());
  BoundaryDlg->show();
}

// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::PushBoundaryDiHelp()
{
  SMESH::ShowHelpFile(QString("homard_create_boundary.html#discrete-boundary"));
}

// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::SetBoundaryA()
{
  MESSAGE("Debut de SetBoundaryA ");
  if (myArgs->CBBoundaryA->isChecked()) {
    bool bOK = CheckCase(true);
    if (bOK) {
      myArgs->GBBoundaryA->setVisible(1);
    }
    else {
      myArgs->GBBoundaryA->setVisible(0);
      myArgs->CBBoundaryA->setChecked(0);
      myArgs->CBBoundaryA->setCheckState(Qt::Unchecked);
    }
  }
  else {
    myArgs->GBBoundaryA->setVisible(0);
  }

  //myArgs->mySelectInMedFileLineEdit->setReadOnly(true);
  //myArgs->mySelectInMedFileButton->hide();

  int aH = 550;
  if (myArgs->GBBoundaryD->isVisible()) aH += 50;
  if (myArgs->GBBoundaryA->isVisible()) aH += 150;
  resize(600, aH);
  //adjustSize();
}
// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::AddBoundaryAn(QString newBoundary)
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de AddBoundaryAn ");
// Ajout d'une nouvelle colonne
  int nbcol = myArgs->TWBoundary->columnCount();
//   MESSAGE("nbcol " <<  nbcol);
  nbcol += 1 ;
  myArgs->TWBoundary->setColumnCount ( nbcol ) ;
  QTableWidgetItem *__colItem = new QTableWidgetItem();
  __colItem->setText(tr(newBoundary.toStdString().c_str()));
  myArgs->TWBoundary->setHorizontalHeaderItem(nbcol-1, __colItem);
/*  TWBoundary->horizontalHeaderItem(nbcol-1)->setFlags( Qt::ItemIsSelectable|Qt::ItemIsEnabled );*/
// Chaque case est a cocher
  int nbrow = myArgs->TWBoundary->rowCount();
//   MESSAGE("nbrow " <<  nbrow);
  for ( int i = 0; i < nbrow; i++ )
  {
    myArgs->TWBoundary->setItem( i, nbcol-1, new QTableWidgetItem( QString ("") ) );
    myArgs->TWBoundary->item( i, nbcol-1 )->setFlags( 0 );
    myArgs->TWBoundary->item( i, nbcol-1 )->setFlags( Qt::ItemIsUserCheckable|Qt::ItemIsEnabled  );
    myArgs->TWBoundary->item( i, nbcol-1 )->setCheckState( Qt::Unchecked );
  }
  myArgs->TWBoundary->resizeColumnToContents(nbcol-1);
//   TWBoundary->resizeRowsToContents();
//   MESSAGE("Fin de AddBoundaryAn ");
}
// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::PushBoundaryAnNew()
// ------------------------------------------------------------------------
{
   SMESH_CreateBoundaryAn *BoundaryDlg = new SMESH_CreateBoundaryAn
     (this, true, SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen), "Case_1");
   BoundaryDlg->show();
}
// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::PushBoundaryAnEdit()
// ------------------------------------------------------------------------
{
  QString nom="";
  int nbcol = myArgs->TWBoundary->columnCount();
  for ( int i = 1; i < nbcol; i++ ) {
    QTableWidgetItem *__colItem = new QTableWidgetItem();
    __colItem = myArgs->TWBoundary->horizontalHeaderItem(i);
    nom = QString(__colItem->text()) ;
    MESSAGE("nom "<<nom.toStdString().c_str());
    if (nom != QString("")) {
      SMESH_EditBoundaryAn *BoundaryDlg = new SMESH_EditBoundaryAn
        (this, true, SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen), "Case_1", nom);
      BoundaryDlg->show();
    }
  }
}
// ------------------------------------------------------------------------
void SMESHGUI_HomardAdaptDlg::PushBoundaryAnHelp()
// ------------------------------------------------------------------------
{
  SMESH::ShowHelpFile(QString("homard_create_boundary.html#analytical-boundary"));
}

//=================================================================================
// function : SMESHGUI_HomardAdaptArguments()
// purpose  :
//=================================================================================
SMESHGUI_HomardAdaptArguments::SMESHGUI_HomardAdaptArguments(QWidget* parent)
  : QWidget(parent)
{
  setupUi();
}

SMESHGUI_HomardAdaptArguments::~SMESHGUI_HomardAdaptArguments()
{
}

void SMESHGUI_HomardAdaptArguments::setupUi()
{
  // Mesh in
  QGroupBox* aMeshIn    = new QGroupBox( tr( "MeshIn" ), this );
  myInMedFileRadio      = new QRadioButton( tr( "MEDFile" ), aMeshIn );
  myInBrowserRadio      = new QRadioButton( tr( "Browser" ), aMeshIn );
  myInBrowserObject     = new QLineEdit( aMeshIn );
  mySelectInMedFileButton   = new QPushButton("...", aMeshIn);
  mySelectInMedFileLineEdit = new QLineEdit( aMeshIn );

  QGridLayout* meshIn = new QGridLayout( aMeshIn );

  meshIn->setMargin( 9 );
  meshIn->setSpacing( 6 );
  meshIn->addWidget( myInMedFileRadio,          0, 0, 1, 1 );
  meshIn->addWidget( myInBrowserRadio,          0, 1, 1, 1 );
  meshIn->addWidget( mySelectInMedFileButton,   1, 0, 1, 1 );
  meshIn->addWidget( mySelectInMedFileLineEdit, 1, 1, 1, 2 );
  meshIn->addWidget( myInBrowserObject,         0, 2, 1, 1 );

  myInMeshGroup = new QButtonGroup( this );
  myInMeshGroup->addButton( myInMedFileRadio, 0 );
  myInMeshGroup->addButton( myInBrowserRadio, 1 );

  // Mesh out
  QGroupBox* aMeshOut = new QGroupBox( tr( "MeshOut" ), this );
  QLabel* meshName = new QLabel(tr("MeshName"), aMeshOut);
  QSpacerItem* secondHspacer = new QSpacerItem(100, 30);
  myOutMeshNameLineEdit = new QLineEdit(aMeshOut);
  myOutMedFileChk = new QCheckBox(tr("MEDFile"), aMeshOut);
  mySelectOutMedFileButton = new QPushButton("...", aMeshOut);
  mySelectOutMedFileLineEdit = new QLineEdit(aMeshOut);
  myOutPublishChk = new QCheckBox(tr("Publish_MG_ADAPT"), aMeshOut);

  QGridLayout* meshOut = new QGridLayout( aMeshOut );

  meshOut->setMargin( 9 );
  meshOut->setSpacing( 6 );
  meshOut->addWidget( meshName,  0, 0, 1,1 );
  meshOut->addItem( secondHspacer,  0, 1, 1, 1 );
  meshOut->addWidget( myOutMeshNameLineEdit, 0, 2,1,1);
  meshOut->addWidget( myOutMedFileChk,  1, 0,1,1 );
  meshOut->addWidget( mySelectOutMedFileButton,  1, 1,1,1 );
  meshOut->addWidget( mySelectOutMedFileLineEdit,  1, 2,1,1);
  meshOut->addWidget( myOutPublishChk,  2, 0,1,1 );

  // Conformity type
  QGroupBox *GBTypeConf = new QGroupBox(tr("Conformity type"), this);
  RBConforme = new QRadioButton(tr("Conformal"), GBTypeConf);
  RBNonConforme = new QRadioButton(tr("Non conformal"), GBTypeConf);
  RBConforme->setChecked(true);

  QHBoxLayout *hboxLayout2 = new QHBoxLayout(GBTypeConf);
  hboxLayout2->setSpacing(6);
  hboxLayout2->setContentsMargins(9, 9, 9, 9);
  hboxLayout2->addWidget(RBConforme);
  hboxLayout2->addWidget(RBNonConforme);

  // Boundary type
  GBTypeBoun = new QGroupBox(tr("Boundary type"), this);

  RBBoundaryNo     = new QRadioButton(tr("No boundary"), GBTypeBoun);
  RBBoundaryCAO    = new QRadioButton(tr("CAO"), GBTypeBoun);
  RBBoundaryNonCAO = new QRadioButton(tr("Non CAO"), GBTypeBoun);
  RBBoundaryNo->setChecked(true);

  //     CAO
  GBBoundaryC = new QGroupBox(tr("CAO"), GBTypeBoun);
  /*
  QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
  sizePolicy1.setHorizontalStretch(0);
  sizePolicy1.setVerticalStretch(0);
  sizePolicy1.setHeightForWidth(GBBoundaryC->sizePolicy().hasHeightForWidth());
  GBBoundaryC->setSizePolicy(sizePolicy1);
  */
  GBBoundaryC->setMinimumSize(QSize(450, 50));

  CBBoundaryCAO = new QComboBox(GBBoundaryC);
  CBBoundaryCAO->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  PBBoundaryCAONew = new QPushButton(tr("New"), GBBoundaryC);
  PBBoundaryCAOEdit = new QPushButton(tr("Edit"), GBBoundaryC);
  PBBoundaryCAOHelp = new QPushButton(tr("Help"), GBBoundaryC);

  PBBoundaryCAONew->setAutoDefault(false);
  PBBoundaryCAOEdit->setAutoDefault(false);
  PBBoundaryCAOHelp->setAutoDefault(false);

  QGridLayout* gridLayout2 = new QGridLayout(GBBoundaryC);
  gridLayout2->setSpacing(6);
  gridLayout2->setContentsMargins(9, 9, 9, 9);
  gridLayout2->addWidget(CBBoundaryCAO, 0, 0, 1, 1);
  QSpacerItem* spacerItem3 = new QSpacerItem(40, 13, QSizePolicy::Fixed, QSizePolicy::Minimum);
  gridLayout2->addItem(spacerItem3, 0, 1, 1, 1);
  gridLayout2->addWidget(PBBoundaryCAONew, 0, 2, 1, 1);
  gridLayout2->addWidget(PBBoundaryCAOEdit, 0, 3, 1, 1);
  gridLayout2->addWidget(PBBoundaryCAOHelp, 0, 4, 1, 1);
  gridLayout2->setRowMinimumHeight(0, 80);

  //     Non CAO (discrete / analytical)
  GBBoundaryN = new QGroupBox(tr("Non CAO"), GBTypeBoun);
  GBBoundaryN->setMinimumSize(QSize(450, 80));

  CBBoundaryD = new QCheckBox(tr("Discrete boundary"), GBBoundaryN);
  CBBoundaryA = new QCheckBox(tr("Analytical boundary"), GBBoundaryN);

  //          discrete
  GBBoundaryD = new QGroupBox(tr("Discrete boundary"), GBBoundaryN);
  //sizePolicy1.setHeightForWidth(GBBoundaryD->sizePolicy().hasHeightForWidth());
  //GBBoundaryD->setSizePolicy(sizePolicy1);
  QGridLayout* gridLayoutD = new QGridLayout(GBBoundaryD);
  gridLayoutD->setSpacing(6);
  gridLayoutD->setContentsMargins(9, 9, 9, 9);
  CBBoundaryDi = new QComboBox(GBBoundaryD);
  CBBoundaryDi->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  gridLayoutD->addWidget(CBBoundaryDi, 0, 0, 1, 1);

  QSpacerItem* spacerItem5 = new QSpacerItem(40, 13, QSizePolicy::Fixed, QSizePolicy::Minimum);
  gridLayoutD->addItem(spacerItem5, 0, 1, 1, 1);

  PBBoundaryDiNew  = new QPushButton(tr("New"), GBBoundaryD);
  PBBoundaryDiEdit = new QPushButton(tr("Edit"), GBBoundaryD);
  PBBoundaryDiHelp = new QPushButton(tr("Help"), GBBoundaryD);

  PBBoundaryDiNew->setAutoDefault(false);
  PBBoundaryDiEdit->setAutoDefault(false);
  PBBoundaryDiHelp->setAutoDefault(false);

  gridLayoutD->addWidget(PBBoundaryDiNew,  0, 2, 1, 1);
  gridLayoutD->addWidget(PBBoundaryDiEdit, 0, 3, 1, 1);
  gridLayoutD->addWidget(PBBoundaryDiHelp, 0, 4, 1, 1);

  //          analytical
  GBBoundaryA = new QGroupBox(tr("Analytical boundary"), GBBoundaryN);
  GBBoundaryA->setMinimumSize(QSize(548, 200));
  formLayout = new QFormLayout(GBBoundaryA);
  TWBoundary = new QTableWidget(GBBoundaryA);
  if (TWBoundary->columnCount() < 1)
    TWBoundary->setColumnCount(1);
  QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
  TWBoundary->setHorizontalHeaderItem(0, __qtablewidgetitem);
  TWBoundary->setEditTriggers(QAbstractItemView::AnyKeyPressed|QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed|QAbstractItemView::SelectedClicked);
  TWBoundary->setShowGrid(true);
  TWBoundary->setRowCount(0);
  TWBoundary->setColumnCount(1);

  formLayout->setWidget(0, QFormLayout::LabelRole, TWBoundary);

  QGridLayout* gridLayout1 = new QGridLayout();
  gridLayout1->setSpacing(6);
  gridLayout1->setContentsMargins(0, 0, 0, 0);

  PBBoundaryAnNew  = new QPushButton(tr("New"), GBBoundaryA);
  PBBoundaryAnEdit = new QPushButton(tr("Edit"), GBBoundaryA);
  PBBoundaryAnHelp = new QPushButton(tr("Help"), GBBoundaryA);

  PBBoundaryAnNew->setAutoDefault(false);
  PBBoundaryAnEdit->setAutoDefault(false);
  PBBoundaryAnHelp->setAutoDefault(false);

  gridLayout1->addWidget(PBBoundaryAnNew,  0, 0, 1, 1);
  gridLayout1->addWidget(PBBoundaryAnEdit, 1, 0, 1, 1);
  gridLayout1->addWidget(PBBoundaryAnHelp, 2, 0, 1, 1);

  formLayout->setLayout(0, QFormLayout::FieldRole, gridLayout1);

  // Boundary No Layout
  QGridLayout* aBoundaryNoLayout = new QGridLayout(GBBoundaryN);
  //aBoundaryNoLayout->addLayout(hboxLayout3, 0, 0);
  aBoundaryNoLayout->addWidget(CBBoundaryD, 0, 0);
  aBoundaryNoLayout->addWidget(CBBoundaryA, 0, 1);
  aBoundaryNoLayout->addWidget(GBBoundaryD, 1, 0, 1, 2);
  aBoundaryNoLayout->addWidget(GBBoundaryA, 2, 0, 1, 2);

  // Boundary type Layout
  QGridLayout* aBoundTypeLayout = new QGridLayout(GBTypeBoun);
  aBoundTypeLayout->addWidget(RBBoundaryNo,     0, 0);
  aBoundTypeLayout->addWidget(RBBoundaryCAO,    0, 1);
  aBoundTypeLayout->addWidget(RBBoundaryNonCAO, 0, 2);

  aBoundTypeLayout->addWidget(GBBoundaryC, 1, 0, 1, 3);
  aBoundTypeLayout->addWidget(GBBoundaryN, 2, 0, 1, 3);

  // Arguments layout
  QGridLayout *argumentsLayout = new QGridLayout(this);
  argumentsLayout->addWidget(aMeshIn,     0, 0, 1, 3);
  argumentsLayout->addWidget(aMeshOut,    1, 0, 1, 3);
  argumentsLayout->addWidget(GBTypeConf,  2, 0, 1, 3);
  argumentsLayout->addWidget(GBTypeBoun,  3, 0, 1, 3);
  argumentsLayout->setColumnStretch( 1, 5 );
  argumentsLayout->setRowStretch( 4, 5 );

  QTableWidgetItem *___qtablewidgetitem = TWBoundary->horizontalHeaderItem(0);
  ___qtablewidgetitem->setText(tr("a_virer"));

  // Initial state
  myInMedFileRadio->setChecked( true );
  modeInChanged( MedFile );
  RBBoundaryNo->setChecked( true );
  //SetBoundaryNo();

  myOutMedFileChk->setChecked(true);
  CBBoundaryCAO->setCurrentIndex(-1);
  CBBoundaryDi->setCurrentIndex(-1);

  // Connections
  connect(myInMeshGroup,            SIGNAL(buttonClicked(int)), this, SLOT(modeInChanged(int)));
  connect(myOutMedFileChk,          SIGNAL(stateChanged(int)),  this, SLOT(onOutMedFileChk(int)));
  connect(myOutPublishChk,          SIGNAL(stateChanged(int)),  this, SLOT(onOutPublishChk(int)));
  connect(mySelectOutMedFileButton, SIGNAL(pressed()), this, SLOT(onSelectOutMedFileButton()));
  emit updateSelection();
}

void SMESHGUI_HomardAdaptArguments::modeInChanged( int theMode )
{
  clear();
  if (theMode == MedFile) {
    mySelectInMedFileLineEdit->show();
    mySelectInMedFileButton->show();
    myInBrowserObject->hide();
  }
  else {
    mySelectInMedFileLineEdit->hide();
    mySelectInMedFileButton->hide();
    myInBrowserObject->show();
    emit updateSelection();
  }
}

void SMESHGUI_HomardAdaptArguments::onSelectOutMedFileButton()
{
  // Current value
  QString fileName0 = mySelectOutMedFileLineEdit->text().trimmed();

  // Ask user for the new value
  QString filtre = QString("Med");
  filtre += QString(" files (*.") + QString("med") + QString(");;");
  QString fileName = QFileDialog::getSaveFileName(this, tr("SAVE_MED"), QString(""), filtre);

  // Check the new value
  if (fileName.isEmpty()) fileName = fileName0;

  QFileInfo aFileInfo (fileName);
  mySelectOutMedFileLineEdit->setText(aFileInfo.absoluteFilePath());
}

void SMESHGUI_HomardAdaptArguments::clear()
{
  mySelectInMedFileLineEdit->clear();
  myInBrowserObject->clear();

  myOutMeshNameLineEdit->clear();
  mySelectOutMedFileLineEdit->clear();
}

void SMESHGUI_HomardAdaptArguments::onOutMedFileChk(int state)
{
  if (state == Qt::Checked) {
    mySelectOutMedFileButton->show();
    mySelectOutMedFileLineEdit->show();
    mySelectOutMedFileButton->setEnabled(true);
    mySelectOutMedFileLineEdit->setEnabled(true);
  }
  else {
    mySelectOutMedFileButton->setEnabled(false);
    mySelectOutMedFileLineEdit->setEnabled(false);
    myOutPublishChk->setChecked(true);
  }
}

void SMESHGUI_HomardAdaptArguments::onOutPublishChk(int state)
{
  if (state == Qt::Unchecked) {
    myOutMedFileChk->setChecked(true);
  }
}

//////////////////////////////////////////
// SMESHGUI_HomardAdaptAdvanced
//////////////////////////////////////////

SMESHGUI_HomardAdaptAdvanced::SMESHGUI_HomardAdaptAdvanced(QWidget* parent)
  : QWidget(parent)
{
  setupWidget();
  connect(workingDirectoryPushButton, SIGNAL(pressed()),
          this, SLOT(onWorkingDirectoryPushButton()));
}

SMESHGUI_HomardAdaptAdvanced::~SMESHGUI_HomardAdaptAdvanced()
{
}

void SMESHGUI_HomardAdaptAdvanced::setupWidget()
{
  //this->resize(337, 369);

  // Logs and debug
  logGroupBox = new QGroupBox(this);
  QGridLayout* logGroupBoxLayout = new QGridLayout(this);
  logGroupBoxLayout->setMargin( 9 );
  logGroupBoxLayout->setSpacing( 6 );
  logGroupBoxLayout->addWidget(logGroupBox, 0, 0, 1, 1);

  QGridLayout* logsLayout = new QGridLayout(logGroupBox);
  logsLayout->setMargin( 9 );
  logsLayout->setSpacing( 6 );

  // Working directory + Verbose level layout
  QGridLayout* gridLayout = new QGridLayout();

  // Working directory
  workingDirectoryLabel = new QLabel(logGroupBox);
  workingDirectoryLineEdit = new QLineEdit(logGroupBox);
  workingDirectoryPushButton = new QPushButton(logGroupBox);

  gridLayout->addWidget(workingDirectoryLabel,      0, 0, 1, 1);
  gridLayout->addWidget(workingDirectoryLineEdit,   0, 1, 1, 1);
  gridLayout->addWidget(workingDirectoryPushButton, 0, 2, 1, 1);

  // Verbose level
  verboseLevelLabel = new QLabel(logGroupBox);
  verboseLevelSpin = new QSpinBox(logGroupBox);

  gridLayout->addWidget(verboseLevelLabel, 1, 0, 1, 1);
  gridLayout->addWidget(verboseLevelSpin,  1, 1, 1, 1);

  logsLayout->addLayout(gridLayout, 0, 0, 1, 1);

  // logInFileCheck + removeLogOnSuccessCheck
  QHBoxLayout* horizontalLayout = new QHBoxLayout();

  logInFileCheck = new QCheckBox(logGroupBox);
  removeLogOnSuccessCheck = new QCheckBox(logGroupBox);

  logInFileCheck->setChecked(true);
  removeLogOnSuccessCheck->setChecked(true);

  horizontalLayout->addWidget(logInFileCheck);
  horizontalLayout->addWidget(removeLogOnSuccessCheck);

  logsLayout->addLayout(horizontalLayout, 1, 0, 1, 1);

  // Keep Working Files Check
  keepWorkingFilesCheck = new QCheckBox(logGroupBox);
  keepWorkingFilesCheck->setAutoExclusive(false);
  keepWorkingFilesCheck->setChecked(false);

  logsLayout->addWidget(keepWorkingFilesCheck, 2, 0, 1, 1);

  //logsLayout->setColumnStretch( 1, 5 );
  logsLayout->setRowStretch( 3, 5 );
}

void SMESHGUI_HomardAdaptAdvanced::onWorkingDirectoryPushButton()
{
  QString aWorkingDir = QFileDialog::getExistingDirectory();
  if (!(aWorkingDir.isEmpty())) workingDirectoryLineEdit->setText(aWorkingDir);
}
