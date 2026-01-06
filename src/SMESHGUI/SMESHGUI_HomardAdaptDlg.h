// Copyright (C) 2011-2026  CEA, EDF
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

#ifndef SMESHGUI_HOMARDADAPTDLG_HXX
#define SMESHGUI_HOMARDADAPTDLG_HXX

#include "SMESH_SMESHGUI.hxx"

#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SMESH_Homard)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include <SALOME_GenericObj_wrap.hxx>

#include <SalomeApp_Module.h>

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QSpinBox>

class SMESHGUI_HomardAdaptArguments;
class SMESHGUI_HomardAdaptAdvanced;

//=================================================================================
// class    : SMESHGUI_HomardAdaptDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_HomardAdaptDlg : public QDialog
{
  Q_OBJECT

 public:
  SMESHGUI_HomardAdaptDlg(SMESHHOMARD::HOMARD_Gen_ptr theHomardGen);
  ~SMESHGUI_HomardAdaptDlg();

  void AddBoundaryCAO(QString newBoundary);
  void AddBoundaryAn(QString newBoundary);
  void AddBoundaryDi(QString newBoundary);

 protected:
  QString myWorkingDir;

  SMESHHOMARD::HOMARD_Cas_var myCase;
  SALOME::GenericObj_wrap< SMESHHOMARD::HOMARD_Gen > myHomardGen;

  virtual void InitConnect();
  virtual void InitBoundarys();

 public slots:
  virtual void SetFileName();

  virtual void SetBoundaryNo();
  virtual void SetBoundaryCAO();
  virtual void SetBoundaryNonCAO();

  virtual void PushBoundaryCAONew();
  virtual void PushBoundaryCAOEdit();
  virtual void PushBoundaryCAOHelp();

  virtual void SetBoundaryD();
  virtual void PushBoundaryDiNew();
  virtual void PushBoundaryDiEdit();
  virtual void PushBoundaryDiHelp();

  virtual void SetBoundaryA();
  virtual void PushBoundaryAnNew();
  virtual void PushBoundaryAnEdit();
  virtual void PushBoundaryAnHelp();

  bool CheckCase(bool fixCase);

  virtual void PushOnOK();
  virtual bool PushOnApply();
  virtual void PushOnHelp();

  void selectionChanged();
  void updateSelection();

 private:
  SMESHGUI_HomardAdaptArguments* myArgs;
  SMESHGUI_HomardAdaptAdvanced* myAdvOpt;

  SMESH::SMESH_Mesh_var myMesh;

  QPushButton *buttonHelp;
  QPushButton *buttonApply;
  QPushButton *buttonOk;
  QPushButton *buttonCancel;
};

//=================================================================================
// class    : SMESHGUI_HomardAdaptArguments
// purpose  :
//=================================================================================
class SMESHGUI_HomardAdaptArguments : public QWidget
{
  Q_OBJECT

public:
  enum ModeIn { MedFile, Browser };

  SMESHGUI_HomardAdaptArguments (QWidget* parent);
  ~SMESHGUI_HomardAdaptArguments();

  void setupUi();

public:
  // Mesh In
  QButtonGroup* myInMeshGroup;
  QRadioButton* myInMedFileRadio;
  QRadioButton* myInBrowserRadio;

  QPushButton*  mySelectInMedFileButton;
  QLineEdit*    mySelectInMedFileLineEdit; // LEFileName

  QLineEdit*    myInBrowserObject;

  // Mesh Out
  QLineEdit*    myOutMeshNameLineEdit;
  QCheckBox*    myOutMedFileChk;

  QPushButton*  mySelectOutMedFileButton;
  QLineEdit*    mySelectOutMedFileLineEdit;

  QCheckBox*    myOutPublishChk;

  // Conformity type
  QRadioButton *RBConforme;
  QRadioButton *RBNonConforme;

  // Boundary type
  QGroupBox *GBTypeBoun;
  QRadioButton *RBBoundaryNo;
  QRadioButton *RBBoundaryCAO;
  QRadioButton *RBBoundaryNonCAO;
  QHBoxLayout *hboxLayout3;
  QCheckBox *CBBoundaryD;
  QCheckBox *CBBoundaryA;

  QGroupBox *GBBoundaryC; // CAO boundary
  QPushButton *PBBoundaryCAOEdit;
  QPushButton *PBBoundaryCAOHelp;
  QComboBox *CBBoundaryCAO;
  QPushButton *PBBoundaryCAONew;
  QGroupBox *GBBoundaryN; // Non CAO boundary
  QGroupBox *GBBoundaryD;
  QComboBox *CBBoundaryDi;
  QPushButton *PBBoundaryDiEdit;
  QPushButton *PBBoundaryDiHelp;
  QPushButton *PBBoundaryDiNew;
  QGroupBox *GBBoundaryA;
  QFormLayout *formLayout;
  QTableWidget *TWBoundary;
  QPushButton *PBBoundaryAnEdit;
  QPushButton *PBBoundaryAnNew;
  QPushButton *PBBoundaryAnHelp;

signals:
  void updateSelection();

private slots:
  void modeInChanged(int);
  void clear();
  void onOutMedFileChk(int);
  void onOutPublishChk(int);
  void onSelectOutMedFileButton();
};

//=================================================================================
// class    : SMESHGUI_HomardAdaptAdvanced
// purpose  :
//=================================================================================
class SMESHGUI_HomardAdaptAdvanced : public QWidget
{
  Q_OBJECT

public:
  SMESHGUI_HomardAdaptAdvanced(QWidget* = 0);
  ~SMESHGUI_HomardAdaptAdvanced();

  void setupWidget();

public:
  QGroupBox   *logGroupBox;

  QLabel      *workingDirectoryLabel;
  QLineEdit   *workingDirectoryLineEdit;
  QPushButton *workingDirectoryPushButton;

  QLabel      *verboseLevelLabel;
  QSpinBox    *verboseLevelSpin;

  QCheckBox   *logInFileCheck;
  QCheckBox   *removeLogOnSuccessCheck;

  QCheckBox   *keepWorkingFilesCheck;

private slots:
  void onWorkingDirectoryPushButton();
};

#endif // SMESHGUI_HOMARDADAPTDLG_HXX
