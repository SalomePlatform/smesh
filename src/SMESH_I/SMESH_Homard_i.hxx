// Copyright (C) 2011-2023  CEA, EDF
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

#ifndef _SMESH_HOMARD_I_HXX_
#define _SMESH_HOMARD_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Homard)
#include CORBA_CLIENT_HEADER(SALOMEDS)
#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(SALOME_ModuleCatalog)
#include CORBA_CLIENT_HEADER(SMESH_Gen)
#include CORBA_CLIENT_HEADER(SMESH_Mesh)

#include "SMESH.hxx"
#include "SMESH_Homard.hxx"
//#include "FrontTrack.hxx"
#include "SALOME_Component_i.hxx"
#include "SALOME_NamingService.hxx"
#include "Utils_CorbaException.hxx"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace SMESHHOMARD_I
{

class SMESH_I_EXPORT HOMARD_Boundary_i : public virtual SALOME::GenericObj_i,
                                         public virtual POA_SMESHHOMARD::HOMARD_Boundary
{
public:
  HOMARD_Boundary_i(SMESHHOMARD::HOMARD_Gen_var gen_i);
  HOMARD_Boundary_i();

  virtual ~HOMARD_Boundary_i();

  // Generalites
  void                   SetName(const char* Name);
  char*                  GetName();

  char*                  GetDumpPython();

  // Caracteristiques
  void                   SetType(CORBA::Long Type);
  CORBA::Long            GetType();

  void                   SetMeshName(const char* MeshName);
  char*                  GetMeshName();

  void                   SetDataFile(const char* DataFile);
  char*                  GetDataFile();

  void                   SetCylinder(double Xcentre, double Ycentre, double ZCentre,
                                      double Xaxe, double Yaxe, double Zaxe,
                                      double rayon);
  void                   SetSphere(double Xcentre, double Ycentre, double ZCentre,
                                    double rayon);
  void                   SetConeR(double Xcentre1, double Ycentre1, double Zcentre1, double Rayon1,
                                   double Xcentre2, double Ycentre2, double Zcentre2, double Rayon2);
  void                   SetConeA(double Xaxe, double Yaxe, double Zaxe, double Angle,
                                   double Xcentre, double Ycentre, double ZCentre);
  void                   SetTorus(double Xcentre, double Ycentre, double ZCentre,
                                      double Xaxe, double Yaxe, double Zaxe,
                                      double rayonRev, double rayonPri);

  SMESHHOMARD::double_array*  GetCoords();

  void                   SetLimit(double Xincr, double Yincr, double Zincr);
  SMESHHOMARD::double_array*  GetLimit();

  void                   AddGroup(const char* Group);
  void                   SetGroups(const SMESHHOMARD::ListGroupType& ListGroup);
  SMESHHOMARD::ListGroupType* GetGroups();

private:
  SMESHHOMARDImpl::HOMARD_Boundary* myHomardBoundary;
  SMESHHOMARD::HOMARD_Gen_var _gen_i;
};

class SMESH_I_EXPORT HOMARD_Cas_i : public virtual SALOME::GenericObj_i,
                                    public virtual POA_SMESHHOMARD::HOMARD_Cas
{
public:
  HOMARD_Cas_i(SMESHHOMARD::HOMARD_Gen_var gen_i);
  HOMARD_Cas_i();

  virtual ~HOMARD_Cas_i();

  char* GetDumpPython();

  // Caracteristiques
  void  SetDirName(const char* NomDir);
  char* GetDirName();

  void                  SetBoundingBox(const SMESHHOMARD::extrema& LesExtremes);
  SMESHHOMARD::extrema* GetBoundingBox();

  void AddGroup(const char* Group);
  void SetGroups(const SMESHHOMARD::ListGroupType& ListGroup);
  SMESHHOMARD::ListGroupType* GetGroups();

  void AddBoundary(const char* Boundary);
  void AddBoundaryGroup(const char* Boundary, const char* Group);
  SMESHHOMARD::ListBoundaryGroupType* GetBoundaryGroup();
  void SupprBoundaryGroup();

  void AddIteration(const char* NomIteration);

private:
  SMESHHOMARDImpl::HOMARD_Cas* myHomardCas;
  SMESHHOMARD::HOMARD_Gen_var _gen_i;
};

class SMESH_I_EXPORT HOMARD_Iteration_i
{
public:
  HOMARD_Iteration_i(SMESHHOMARD::HOMARD_Gen_var gen_i);
  HOMARD_Iteration_i();

  virtual ~HOMARD_Iteration_i();

  // Generalites
  void                   SetName(const char* Name);
  char*                  GetName();

  // Caracteristiques
  void                   SetDirNameLoc(const char* NomDir);
  char*                  GetDirNameLoc();

  char*                  GetDirName();

  void                   SetNumber(CORBA::Long NumIter);
  CORBA::Long            GetNumber();

  void                   SetState(CORBA::Long etat);
  CORBA::Long            GetState();

  void                   SetMeshName(const char* NomMesh);
  char*                  GetMeshName();

  void                   SetMeshFile(const char* MeshFile);
  char*                  GetMeshFile();

  void                   SetLogFile(const char* LogFile);
  char*                  GetLogFile();

  void                   SetFileInfo(const char* FileInfo);
  char*                  GetFileInfo();

  // Liens avec les autres structures
  void                   SetCaseName(const char* NomCas);
  char*                  GetCaseName();

  // Drivers
  void                   SetInfoCompute(CORBA::Long MessInfo);
  CORBA::Long            GetInfoCompute();

private:
  SMESHHOMARDImpl::HOMARD_Iteration* myHomardIteration;
  SMESHHOMARD::HOMARD_Gen_var _gen_i;
};

class SMESH_I_EXPORT HOMARD_Gen_i : public virtual SALOME::GenericObj_i,
                                    public virtual POA_SMESHHOMARD::HOMARD_Gen
{
public:
  HOMARD_Gen_i();
  virtual ~HOMARD_Gen_i();

  // Les creations
  SMESHHOMARD::HOMARD_Boundary_ptr CreateBoundary (const char* nomBoundary,
                                                   CORBA::Long typeBoundary);
  SMESHHOMARD::HOMARD_Boundary_ptr CreateBoundaryCAO (const char* nomBoundary,
                                                      const char* DataFile);
  SMESHHOMARD::HOMARD_Boundary_ptr CreateBoundaryDi (const char* nomBoundary,
                                                     const char* MeshName, const char* DataFile);
  SMESHHOMARD::HOMARD_Boundary_ptr CreateBoundaryCylinder
    (const char* nomBoundary,
     CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
     CORBA::Double Xaxis, CORBA::Double Yaxis, CORBA::Double Zaxis,
     CORBA::Double Rayon);
  SMESHHOMARD::HOMARD_Boundary_ptr CreateBoundarySphere
    (const char* nomBoundary,
     CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
     CORBA::Double Rayon);
  SMESHHOMARD::HOMARD_Boundary_ptr CreateBoundaryConeR
    (const char* nomBoundary,
     CORBA::Double Xcentre1, CORBA::Double Ycentre1, CORBA::Double Zcentre1,
     CORBA::Double Rayon1,
     CORBA::Double Xcentre2, CORBA::Double Ycentre2, CORBA::Double Zcentre2,
     CORBA::Double Rayon2);
  SMESHHOMARD::HOMARD_Boundary_ptr CreateBoundaryConeA
    (const char* nomBoundary,
     CORBA::Double Xaxis, CORBA::Double Yaxis, CORBA::Double Zaxis,
     CORBA::Double Angle,
     CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre);
  SMESHHOMARD::HOMARD_Boundary_ptr CreateBoundaryTorus
    (const char* nomBoundary,
     CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
     CORBA::Double Xaxis, CORBA::Double Yaxis, CORBA::Double Zaxis,
     CORBA::Double RayonRev, CORBA::Double RayonPri);

  SMESHHOMARD::HOMARD_Cas_ptr CreateCaseOnMesh (const char* MeshName,
                                                SMESH::SMESH_Mesh_ptr smeshMesh,
                                                const char* theWorkingDir);
  SMESHHOMARD::HOMARD_Cas_ptr CreateCase (const char* MeshName,
                                          const char* FileName,
                                          const char* theWorkingDir);

  HOMARD_Iteration_i* CreateIteration();

  // Les informations
  SMESHHOMARD::HOMARD_Boundary_ptr  GetBoundary  (const char* nomBoundary);
  SMESHHOMARD::HOMARD_Cas_ptr       GetCase      ();
  HOMARD_Iteration_i* GetIteration (CORBA::Long numIter);

  SMESHHOMARD::listeBoundarys* GetAllBoundarysName();

  void InvalideBoundary(const char* nomBoundary);

  CORBA::Long DeleteBoundary(const char* nomBoundary);
  void        DeleteCase();
  CORBA::Long DeleteIteration(int numIter);

  void AddBoundary(const char* Boundary);
  void AddBoundaryGroup(const char* Boundary, const char* Group);

  void AssociateCaseIter(int numIter, const char* labelIter);
  char* CreateDirNameIter(const char* nomrep, CORBA::Long num);

  CORBA::Long Compute();
  CORBA::Long ComputeAdap(SMESHHOMARDImpl::HomardDriver* myDriver);
  CORBA::Long ComputeCAO();
  CORBA::Long ComputeCAObis();
  char* ComputeDirManagement();
  char* ComputeDirPaManagement();
  int   DriverTexteBoundary(SMESHHOMARDImpl::HomardDriver* myDriver);

  void PublishResultInSmesh(const char* NomFich);
  void DeleteResultInSmesh(std::string NomFich, std::string MeshName);

  void PythonDump();

  // Preferences
  void SetConfType (CORBA::Long theConfType);
  void SetKeepMedOUT (bool theKeepMedOUT);
  void SetPublishMeshOUT (bool thePublishMeshOUT);
  void SetMeshNameOUT (const char* theMeshNameOUT);
  void SetMeshFileOUT (const char* theMeshFileOUT);

  void SetVerboseLevel (CORBA::Long theVerboseLevel);
  void SetKeepWorkingFiles(bool theKeepWorkingFiles);
  void SetLogInFile(bool theLogInFile);
  void SetLogFile (const char* theLogFile);
  void SetRemoveLogOnSuccess(bool theRemoveLogOnSuccess);

private:
  SMESHHOMARD::HOMARD_Boundary_ptr newBoundary();
  SMESHHOMARD::HOMARD_Cas_ptr      newCase();
  HOMARD_Iteration_i*              newIteration();

private:
  SMESHHOMARDImpl::HOMARD_Gen* myHomard;
  HOMARD_Iteration_i*          myIteration0;
  HOMARD_Iteration_i*          myIteration1;
  SMESHHOMARD::HOMARD_Cas_var  myCase;
  std::map<std::string, SMESHHOMARD::HOMARD_Boundary_var> _mesBoundarys;

  // Preferences
  int  _ConfType; // Le type de conformite ou non conformite
  bool _KeepMedOUT;
  bool _PublishMeshOUT;
  bool _KeepWorkingFiles;
  bool _LogInFile;
  bool _RemoveLogOnSuccess;
  int         _VerboseLevel;
  std::string _MeshNameOUT;
  std::string _MeshFileOUT;
  std::string _LogFile;

  // Input data type
  bool _CaseOnMedFile;
  SMESH::SMESH_Mesh_var _SmeshMesh;
  std::string _TmpMeshFile;
};

SMESH_I_EXPORT std::vector<double>   GetBoundingBoxInMedFile(const char * aFile);
SMESH_I_EXPORT std::set<std::string> GetListeGroupesInMedFile(const char * aFile);

}; // namespace SMESHHOMARD_I

#endif
