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

#ifndef MED_Wrapper_HeaderFile
#define MED_Wrapper_HeaderFile

#include "MED_WrapperDef.hxx"
#include "MED_Structures.hxx"
#include "MED_Algorithm.hxx"

#include <boost/thread/mutex.hpp>

namespace MED
{
  //----------------------------------------------------------------------------
  class TFile;
  typedef boost::shared_ptr<TFile> PFile;

  typedef enum {eLECTURE, eLECTURE_ECRITURE, eLECTURE_AJOUT, eCREATION} EModeAcces;

  //----------------------------------------------------------------------------
  //! Class that wraps the MED API
  class MEDWRAPPER_EXPORT TWrapper
  {
    friend class TLockProxy;

    typedef boost::mutex TMutex;
    //! Synchronization primitive that allows supporting thread safety for the MED access
    TMutex myMutex;

    TWrapper();
    TWrapper(const TWrapper&);
    TWrapper& operator=(const TWrapper&);

  public:
    TWrapper(const std::string& theFileName, bool write, TInt theMajor=-1, TInt theVersion=-1);

    virtual
    ~TWrapper();

    //! Read number of MED Mesh entities in the MED file
    virtual
    TInt
    GetNbMeshes(TErr* theErr = NULL);

    //! Read a MEDWrapper MED Mesh representation by its number
    virtual
    void
    GetMeshInfo(TInt theMeshId,
                TMeshInfo& theInfo,
                TErr* theErr = NULL);

    //! Write the MEDWrapper MED Mesh representation into the MED file
    virtual
    void
    SetMeshInfo(const TMeshInfo& theInfo,
                TErr* theErr = NULL);

    void
    SetMeshInfo(const MED::TMeshInfo& theInfo,
                EModeAcces theMode,
                TErr* theErr = NULL);

    //! Create a MEDWrapper MED Mesh representation
    virtual
    PMeshInfo
    CrMeshInfo(TInt theDim = 0,
               TInt theSpaceDim = 0,
               const std::string& theValue = "",
               EMaillage theType = eNON_STRUCTURE,
               const std::string& theDesc = "");

    //! Copy constructor for the MEDWrapper MED Mesh representation
    virtual
    PMeshInfo
    CrMeshInfo(const PMeshInfo& theInfo);

    //! Get a MEDWrapper MED Mesh representation by its number
    virtual
    PMeshInfo
    GetPMeshInfo(TInt theId,
                 TErr* theErr = NULL);

    //! Read number of MED Family entities in the MED file
    virtual
    TInt
    GetNbFamilies(const TMeshInfo& theMeshInfo,
                  TErr* theErr = NULL);

    //! Read number of attributes for MED Family
    virtual
    TInt
    GetNbFamAttr(TInt theFamId,
                 const TMeshInfo& theInfo,
                 TErr* theErr = NULL);

    //! Read number of MED Groups to which the MED Family belongs to
    virtual
    TInt
     GetNbFamGroup(TInt theFamId,
                  const TMeshInfo& theInfo,
                  TErr* theErr = NULL);

    //! Read a MEDWrapper MED Family representation by its number
    virtual
    void
    GetFamilyInfo(TInt theFamId,
                  TFamilyInfo& theInfo,
                  TErr* theErr = NULL);

    //! Write a MEDWrapper MED Family representation into the MED file
    virtual
    void
    SetFamilyInfo(const TFamilyInfo& theInfo,
                  TErr* theErr = NULL);

    void
    SetFamilyInfo(const MED::TFamilyInfo& theInfo,
                  EModeAcces theMode,
                  TErr* theErr = NULL);

    //! Create a MEDWrapper MED Family representation
    virtual
    PFamilyInfo
    CrFamilyInfo(const PMeshInfo& theMeshInfo,
                 TInt theNbGroup = 0,
                 TInt theNbAttr = 0,
                 TInt theId = 0,
                 const std::string& theValue = "");

    //! Create a MEDWrapper MED Family representation
    virtual
    PFamilyInfo
    CrFamilyInfo(const PMeshInfo& theMeshInfo,
                 const std::string& theValue,
                 TInt theId,
                 const TStringSet& theGroupNames,
                 const TStringVector& theAttrDescs = TStringVector(),
                 const TIntVector& theAttrIds = TIntVector(),
                 const TIntVector& theAttrVals = TIntVector());

    //! Copy constructor for the MEDWrapper MED Family representation
    virtual
    PFamilyInfo
    CrFamilyInfo(const PMeshInfo& theMeshInfo,
                 const PFamilyInfo& theInfo);

    //! Get a MEDWrapper MED Family representation by its number
    PFamilyInfo
    GetPFamilyInfo(const PMeshInfo& theMeshInfo,
                   TInt theId,
                   TErr* theErr = NULL);

    //! Read sequence of names for any descendant of TElemInfo
    virtual
    void
    GetNames(TElemInfo& theInfo,
             TInt theNb,
             EEntiteMaillage theEntity,
             EGeometrieElement theGeom,
             TErr* theErr = NULL);

    //! Write sequence of names for any descendant of TElemInfo
    virtual
    void
    SetNames(const TElemInfo& theInfo,
             EEntiteMaillage theEntity,
             EGeometrieElement theGeom,
             TErr* theErr = NULL);

    void
    SetNames(const TElemInfo& theInfo,
             EModeAcces theMode,
             EEntiteMaillage theEntity,
             EGeometrieElement theGeom,
             TErr* theErr = NULL);

    //! Read sequence of numerated data for any descendant of TElemInfo
    virtual
    void
    GetNumeration(TElemInfo& theInfo,
                  TInt theNb,
                  EEntiteMaillage theEntity,
                  EGeometrieElement theGeom,
                  TErr* theErr = NULL);

    //! Write sequence of numerated data for any descendant of TElemInfo
    virtual
    void
    SetNumeration(const TElemInfo& theInfo,
                  EEntiteMaillage theEntity,
                  EGeometrieElement theGeom,
                  TErr* theErr = NULL);

    void
    SetNumeration(const TElemInfo& theInfo,
                  EModeAcces theMode,
                  EEntiteMaillage theEntity,
                  EGeometrieElement theGeom,
                  TErr* theErr = NULL);

    //! Read sequence of MED Families' indices for any descendant of TElemInfo
    virtual
    void
    GetFamilies(TElemInfo& theInfo,
                TInt theNb,
                EEntiteMaillage theEntity,
                EGeometrieElement theGeom,
                TErr* theErr = NULL);

    //! Write sequence of MED Families' indices for any descendant of TElemInfo
    virtual
    void
    SetFamilies(const TElemInfo& theInfo,
                EEntiteMaillage theEntity,
                EGeometrieElement theGeom,
                TErr* theErr = NULL);

    void
    SetFamilies(const TElemInfo& theInfo,
                EModeAcces theMode,
                EEntiteMaillage theEntity,
                EGeometrieElement theGeom,
                TErr* theErr = NULL);

    //! Read number of nodes in MED Mesh
    virtual
    TInt
    GetNbNodes(const TMeshInfo& theMeshInfo,
               TErr* theErr = NULL);

    virtual
    TInt
    GetNbNodes(const TMeshInfo& theMeshInfo,
               ETable theTable,
               TErr* theErr = NULL);

    //! Read a MEDWrapper MED Nodes representation from the MED file
    virtual
    void
    GetNodeInfo(TNodeInfo& theInfo,
                TErr* theErr = NULL);

    //! Write the MEDWrapper MED Nodes representation into the MED file
    virtual
    void
    SetNodeInfo(const TNodeInfo& theInfo,
                TErr* theErr = NULL);

    void
    SetNodeInfo(const MED::TNodeInfo& theInfo,
                EModeAcces theMode,
                TErr* theErr = NULL);

    //! Create a MEDWrapper MED Nodes representation
    virtual
    PNodeInfo
    CrNodeInfo(const PMeshInfo& theMeshInfo,
               TInt theNbElem,
               EModeSwitch theMode = eFULL_INTERLACE,
               ERepere theSystem = eCART,
               EBooleen theIsElemNum = eVRAI,
               EBooleen theIsElemNames = eFAUX);

    //! Create a MEDWrapper MED Nodes representation
    virtual
    PNodeInfo
    CrNodeInfo(const PMeshInfo& theMeshInfo,
               const TFloatVector& theNodeCoords,
               EModeSwitch theMode = eFULL_INTERLACE,
               ERepere theSystem = eCART,
               const TStringVector& theCoordNames = TStringVector(),
               const TStringVector& theCoordUnits = TStringVector(),
               const TIntVector& theFamilyNums = TIntVector(),
               const TIntVector& theElemNums = TIntVector(),
               const TStringVector& theElemNames = TStringVector());

    //! Copy constructor for the MEDWrapper MED Nodes representation
    virtual
    PNodeInfo
    CrNodeInfo(const PMeshInfo& theMeshInfo,
               const PNodeInfo& theInfo);

    //! Get a MEDWrapper MED Nodes representation from the MED file
    PNodeInfo
    GetPNodeInfo(const PMeshInfo& theMeshInfo,
                 TErr* theErr = NULL);

    //! Create a MEDWrapper MED Elements representation
    virtual
    PElemInfo
    CrElemInfo(const PMeshInfo& theMeshInfo,
               TInt theNbElem,
               EBooleen theIsElemNum = eVRAI,
               EBooleen theIsElemNames = eVRAI);

    //! Create a MEDWrapper MED Elements representation
    virtual
    PElemInfo
    CrElemInfo(const PMeshInfo& theMeshInfo,
               TInt theNbElem,
               const TIntVector& theFamNum,
               const TIntVector& aElemNum,
               const TStringVector& aElemNames);

    //! Get a MEDWrapper MED Element representation from the MED file
    PElemInfo
    GetPElemInfo(const PMeshInfo& theMeshInfo,
                 EEntiteMaillage theEntity = eNOEUD,
                 EGeometrieElement theGeom = ePOINT1,
                 EConnectivite theConnMode = eNOD,
                 TErr* theErr = NULL);

    //! Read number of MED Polygones in MED Mesh
    virtual
    TInt
    GetNbPolygones(const TMeshInfo& theMeshInfo,
                   EEntiteMaillage theEntity,
                   EGeometrieElement theGeom,
                   EConnectivite theConnMode = eNOD,
                   TErr* theErr = NULL);

    //! Read connectivity information for the MED Polygones in defined MED Mesh
    virtual
    TInt
    GetPolygoneConnSize(const TMeshInfo& theMeshInfo,
                        EEntiteMaillage theEntity,
                        EGeometrieElement theGeom,
                        EConnectivite theConnMode = eNOD,
                        TErr* theErr = NULL);

    //! Read a MEDWrapper MED Polygones representation from the MED file
    virtual
    void
    GetPolygoneInfo(TPolygoneInfo& theInfo,
                    TErr* theErr = NULL);

    //! Write a MEDWrapper MED Polygones representation into the MED file
    virtual
    void
    SetPolygoneInfo(const TPolygoneInfo& theInfo,
                    TErr* theErr = NULL);

    void
    SetPolygoneInfo(const MED::TPolygoneInfo& theInfo,
                    EModeAcces theMode,
                    TErr* theErr = NULL);

    //! Create a MEDWrapper MED Polygones representation
    virtual
    PPolygoneInfo
    CrPolygoneInfo(const PMeshInfo& theMeshInfo,
                   EEntiteMaillage theEntity,
                   EGeometrieElement theGeom,
                   TInt theNbElem,
                   TInt theConnSize,
                   EConnectivite theConnMode = eNOD,
                   EBooleen theIsElemNum = eVRAI,
                   EBooleen theIsElemNames = eVRAI);

    //! Create a MEDWrapper MED Polygones representation
    virtual
    PPolygoneInfo
    CrPolygoneInfo(const PMeshInfo& theMeshInfo,
                   EEntiteMaillage theEntity,
                   EGeometrieElement theGeom,
                   const TIntVector& theIndexes,
                   const TIntVector& theConnectivities,
                   EConnectivite theConnMode = eNOD,
                   const TIntVector& theFamilyNums = TIntVector(),
                   const TIntVector& theElemNums = TIntVector(),
                   const TStringVector& theElemNames = TStringVector());

    //! Copy constructor for the MEDWrapper MED Polygones representation
    virtual
    PPolygoneInfo
    CrPolygoneInfo(const PMeshInfo& theMeshInfo,
                   const PPolygoneInfo& theInfo);

    //! Get a MEDWrapper MED Polygones representation from the MED file
    PPolygoneInfo
    GetPPolygoneInfo(const PMeshInfo& theMeshInfo,
                     EEntiteMaillage theEntity,
                     EGeometrieElement theGeom,
                     EConnectivite theConnMode = eNOD);

    //! Read number of MED Polyedres in MED Mesh
    virtual
    TInt
    GetNbPolyedres(const TMeshInfo& theMeshInfo,
                   EEntiteMaillage theEntity,
                   EGeometrieElement theGeom,
                   EConnectivite theConnMode = eNOD,
                   TErr* theErr = NULL);

    //! Read connectivity information for the MED Polyedres in MED Mesh
    virtual
    void
    GetPolyedreConnSize(const TMeshInfo& theMeshInfo,
                        TInt& theNbFaces,
                        TInt& theConnSize,
                        EConnectivite theConnMode = eNOD,
                        TErr* theErr = NULL);

    //! Read a MEDWrapper MED Polyedres representation from the MED file
    virtual
    void
    GetPolyedreInfo(TPolyedreInfo& theInfo,
                    TErr* theErr = NULL);

    //! Write a MEDWrapper MED Polyedres representation into the MED file
    virtual
    void
    SetPolyedreInfo(const TPolyedreInfo& theInfo,
                    TErr* theErr = NULL);

    void
    SetPolyedreInfo(const MED::TPolyedreInfo& theInfo,
                    EModeAcces theMode,
                    TErr* theErr = NULL);

    //! Create a MEDWrapper MED Polyedres representation
    virtual
    PPolyedreInfo
    CrPolyedreInfo(const PMeshInfo& theMeshInfo,
                   EEntiteMaillage theEntity,
                   EGeometrieElement theGeom,
                   TInt theNbElem,
                   TInt theNbFaces,
                   TInt theConnSize,
                   EConnectivite theConnMode = eNOD,
                   EBooleen theIsElemNum = eVRAI,
                   EBooleen theIsElemNames = eVRAI);

    //! Create a MEDWrapper MED Polyedres representation
    virtual
    PPolyedreInfo
    CrPolyedreInfo(const PMeshInfo& theMeshInfo,
                   EEntiteMaillage theEntity,
                   EGeometrieElement theGeom,
                   const TIntVector& theIndexes,
                   const TIntVector& theFaces,
                   const TIntVector& theConnectivities,
                   EConnectivite theConnMode = eNOD,
                   const TIntVector& theFamilyNums = TIntVector(),
                   const TIntVector& theElemNums = TIntVector(),
                   const TStringVector& theElemNames = TStringVector());

    //! Copy constructor for the MEDWrapper MED Polyedres representation
    virtual
    PPolyedreInfo
    CrPolyedreInfo(const PMeshInfo& theMeshInfo,
                   const PPolyedreInfo& theInfo);

    //! Get a MEDWrapper MED Polyedres representation from the MED file
    PPolyedreInfo
    GetPPolyedreInfo(const PMeshInfo& theMeshInfo,
                     EEntiteMaillage theEntity,
                     EGeometrieElement theGeom,
                     EConnectivite theConnMode = eNOD);

    //! Get TEntityInfo which contains brief information about existing cells
    //  and their destribution among MED ENTITIES
    virtual
    TEntityInfo
    GetEntityInfo(const TMeshInfo& theMeshInfo,
                  EConnectivite theConnMode = eNOD,
                  TErr* theErr = NULL);

    //! Read number of cells for defined MED Mesh, ENTITY and geometrical type with given connectivity mode
    virtual
    TInt
    GetNbCells(const TMeshInfo& theMeshInfo,
               EEntiteMaillage theEntity,
               EGeometrieElement theGeom,
               EConnectivite theConnMode = eNOD,
               TErr* theErr = NULL);

    //! Read a MEDWrapper MED Cells representation from the MED file
    virtual
    void
    GetCellInfo(TCellInfo& theInfo,
                TErr* theErr = NULL);

    //! Write the MEDWrapper MED Cells representation into the MED file
    virtual
    void
    SetCellInfo(const TCellInfo& theInfo,
                TErr* theErr = NULL);

    void
    SetCellInfo(const MED::TCellInfo& theInfo,
                EModeAcces theMode,
                TErr* theErr = NULL);

    //! Create a MEDWrapper MED Cells representation
    virtual
    PCellInfo
    CrCellInfo(const PMeshInfo& theMeshInfo,
               EEntiteMaillage theEntity,
               EGeometrieElement theGeom,
               TInt theNbElem,
               EConnectivite theConnMode = eNOD,
               EBooleen theIsElemNum = eVRAI,
               EBooleen theIsElemNames = eFAUX,
               EModeSwitch theMode = eFULL_INTERLACE);

    //! Create a MEDWrapper MED Cells representation
    virtual
    PCellInfo
    CrCellInfo(const PMeshInfo& theMeshInfo,
               EEntiteMaillage theEntity,
               EGeometrieElement theGeom,
               const TIntVector& theConnectivities,
               EConnectivite theConnMode = eNOD,
               const TIntVector& theFamilyNums = TIntVector(),
               const TIntVector& theElemNums = TIntVector(),
               const TStringVector& theElemNames = TStringVector(),
               EModeSwitch theMode = eFULL_INTERLACE);

    //! Copy constructor for the MEDWrapper MED Cells representation
    virtual
    PCellInfo
    CrCellInfo(const PMeshInfo& theMeshInfo,
               const PCellInfo& theInfo);

    //! Get a MEDWrapper MED Cells representation from the MED file
    PCellInfo
    GetPCellInfo(const PMeshInfo& theMeshInfo,
                 EEntiteMaillage theEntity,
                 EGeometrieElement theGeom,
                 EConnectivite theConnMode = eNOD,
                 TErr* theErr = NULL);

    //! Read geom type of MED_BALL structural element
    EGeometrieElement
    GetBallGeom(const TMeshInfo& theMeshInfo);

    //! Read number of balls in the Mesh
    /*! This feature is supported since version 3.0 */
    virtual
    TInt
    GetNbBalls(const TMeshInfo& theMeshInfo);

    //! Read a MEDWrapper representation of MED_BALL from the MED file
    /*! This feature is supported since version 3.0 */
    virtual
    void
    GetBallInfo(TBallInfo& theInfo,
                TErr* theErr = NULL);

    //! Write a MEDWrapper representation of MED_BALL into the MED file
    /*! This feature is supported since version 3.0 */
    virtual
    void
    SetBallInfo(const TBallInfo& theInfo,
                TErr* theErr = NULL);

    void
    SetBallInfo(const TBallInfo& theInfo,
                EModeAcces theMode,
                TErr* theErr);

    //! Create a MEDWrapper MED Balls representation
    /*! This feature is supported since version 3.0 */
    virtual
    PBallInfo
    CrBallInfo(const PMeshInfo& theMeshInfo,
               TInt theNbBalls,
               EBooleen theIsElemNum = eVRAI);

    //! Create a MEDWrapper MED Balls representation
    /*! This feature is supported since version 3.0 */
    virtual
    PBallInfo
    CrBallInfo(const PMeshInfo& theMeshInfo,
               const TIntVector& theNodes,
               TFloatVector& theDiameters,
               const TIntVector& theFamilyNums = TIntVector(),
               const TIntVector& theElemNums = TIntVector());

    //! Copy constructor for the MEDWrapper MED Balls representation
    /*! This feature is supported since version 3.0 */
    virtual
    PBallInfo
    CrBallInfo(const PMeshInfo& theMeshInfo,
               const PBallInfo& theInfo);

    //! Get a MEDWrapper MED Balls representation from the MED file
    /*! This feature is supported since version 3.0 */
    virtual
    PBallInfo
    GetPBallInfo(const PMeshInfo& theMeshInfo);

    //! Read number of MED FIELDS in the MED Mesh
    virtual
    TInt
    GetNbFields(TErr* theErr = NULL);

    //! Read number of components for the MED FIELD by its order number
    virtual
    TInt
    GetNbComp(TInt theFieldId,
              TErr* theErr = NULL);

    //! Read MEDWrapper MED FIELD representation from the MED file by its order number
    virtual
    void
    GetFieldInfo(TInt theFieldId,
                 TFieldInfo& theInfo,
                 TErr* theErr = NULL);

    //! Write MEDWrapper MED FIELD representation into the MED file
    virtual
    void
    SetFieldInfo(const TFieldInfo& theInfo,
                 TErr* theErr = NULL);

    void
    SetFieldInfo(const MED::TFieldInfo& theInfo,
                 EModeAcces theMode,
                 TErr* theErr = NULL);

    //! Create a MEDWrapper MED FIELD representation
    virtual
    PFieldInfo
    CrFieldInfo(const PMeshInfo& theMeshInfo,
                TInt theNbComp = 0,
                ETypeChamp theType = eFLOAT64,
                const std::string& theValue = "",
                EBooleen theIsLocal = eVRAI,
                TInt theNbRef = 1);

    //! Copy constructor for the MEDWrapper MED FIELD representation
    virtual
    PFieldInfo
    CrFieldInfo(const PMeshInfo& theMeshInfo,
                const PFieldInfo& theInfo);

    //! Get a MEDWrapper MED FIELD representation from the MED file
    PFieldInfo
    GetPFieldInfo(const PMeshInfo& theMeshInfo,
                  TInt theId,
                  TErr* theErr = NULL);

    //! Read number of MED GAUSS in the MED Mesh
    virtual
    TInt
    GetNbGauss(TErr* theErr = NULL);

    //! Read brief MED GAUSS information by its order number from the MED file
    virtual
    TGaussInfo::TInfo
    GetGaussPreInfo(TInt theId,
                    TErr* theErr = NULL);

    //! Read a MEDWrapper MED GAUSS representation by its order number from the MED file
    virtual
    void
    GetGaussInfo(TInt theId,
                 TGaussInfo& theInfo,
                 TErr* theErr = NULL);

    //! Create a MEDWrapper MED GAUSS representation
    virtual
    PGaussInfo
    CrGaussInfo(const TGaussInfo::TInfo& theInfo,
                EModeSwitch theMode = eFULL_INTERLACE);

    //! Read number of MED TIMESTAMPS in the MED Mesh
    /*!
      In addition, the following information can be obtained:
      - what MED ENTITY the MED TIMESTAMP is connected to;
      - what geometrical types the MED TIMESTAMP is defined on.
    */
    virtual
    TInt
    GetNbTimeStamps(const TFieldInfo& theInfo,
                    const TEntityInfo& theEntityInfo,
                    EEntiteMaillage& theEntity,
                    TGeom2Size& theGeom2Size,
                    TErr* theErr = NULL);

    //! Read MEDWrapper MED TIMESTAMP representation by its order number from the MED file
    virtual
    void
    GetTimeStampInfo(TInt theTimeStampId,
                     TTimeStampInfo& theInfo,
                     TErr* theErr = NULL);

    //! Create a MEDWrapper MED TIMESTAMP representation
    virtual
    PTimeStampInfo
    CrTimeStampInfo(const PFieldInfo& theFieldInfo,
                    EEntiteMaillage theEntity,
                    const TGeom2Size& theGeom2Size,
                    const TGeom2NbGauss& theGeom2NbGauss = TGeom2NbGauss(),
                    TInt theNumDt = 0,
                    TInt theNumOrd = 0,
                    TFloat theDt = 0,
                    const std::string& theUnitDt = "",
                    const TGeom2Gauss& theGeom2Gauss = TGeom2Gauss());

    //! Copy constructor for the MEDWrapper MED TIMESTAMP representation
    virtual
    PTimeStampInfo
    CrTimeStampInfo(const PFieldInfo& theFieldInfo,
                    const PTimeStampInfo& theInfo);

    //! Get MEDWrapper MED TIMESTAMP representation from the MED file
    PTimeStampInfo
    GetPTimeStampInfo(const PFieldInfo& theFieldInfo,
                      EEntiteMaillage theEntity,
                      const TGeom2Size& theGeom2Size,
                      TInt theId,
                      TErr* theErr = NULL);

    //! Read number of MED PROFILES in the MED file
    virtual
    TInt
    GetNbProfiles(TErr* theErr = NULL);

    //! Read brief MED PROFILE information by its order number from the MED file
    virtual
    TProfileInfo::TInfo
    GetProfilePreInfo(TInt theId,
                      TErr* theErr = NULL);

    //! Read a MEDWrapper MED PROFILE representation by its order number from the MED file
    virtual
    void
    GetProfileInfo(TInt theId,
                   TProfileInfo& theInfo,
                   TErr* theErr = NULL);

    //! Write a MEDWrapper MED PROFILE representation into the MED file
    virtual
    void
    SetProfileInfo(const TProfileInfo& theInfo,
                   TErr* theErr = NULL);

    void
    SetProfileInfo(const TProfileInfo& theInfo,
                   EModeAcces theMode,
                   TErr* theErr = NULL);

    //! Create a MEDWrapper MED PROFILE representation
    virtual
    PProfileInfo
    CrProfileInfo(const TProfileInfo::TInfo& theInfo,
                  EModeProfil theMode = eCOMPACT);

    //! Get a MEDWrapper MED PROFILE representation from the MED file
    PProfileInfo
    GetPProfileInfo(TInt theId,
                    EModeProfil theMode = eCOMPACT,
                    TErr* theErr = NULL);

    //! Read the values for MEDWrapper MED TIMESTAMP from the MED file
    virtual
    void
    GetTimeStampValue(const PTimeStampValueBase& theTimeStampValue,
                      const TMKey2Profile& theMKey2Profile,
                      const TKey2Gauss& theKey2Gauss,
                      TErr* theErr = NULL);

    //! Write the values for MEDWrapper MED TIMESTAMP into the MED file
    virtual
    void
    SetTimeStampValue(const PTimeStampValueBase& theTimeStampValue,
                      TErr* theErr = NULL);

    void
    SetTimeStampValue(const PTimeStampValueBase& theTimeStampValue,
                      EModeAcces theMode,
                      TErr* theErr = NULL);

    //! Create the values for MEDWrapper MED TIMESTAMP representation
    virtual
    PTimeStampValueBase
    CrTimeStampValue(const PTimeStampInfo& theTimeStampInfo,
                     ETypeChamp theTypeChamp,
                     const TGeom2Profile& theGeom2Profile = TGeom2Profile(),
                     EModeSwitch theMode = eFULL_INTERLACE);

    //! Create the values for MEDWrapper MED TIMESTAMP representation
    virtual
    PTimeStampValueBase
    CrTimeStampValue(const PTimeStampInfo& theTimeStampInfo,
                     const TGeom2Profile& theGeom2Profile = TGeom2Profile(),
                     EModeSwitch theMode = eFULL_INTERLACE);

    //! Copy constructor for the values for MEDWrapper MED TIMESTAMP representation
    virtual
    PTimeStampValueBase
    CrTimeStampValue(const PTimeStampInfo& theTimeStampInfo,
                     const PTimeStampValueBase& theInfo,
                     ETypeChamp theTypeChamp);

    //! Copy constructor for the values for MEDWrapper MED TIMESTAMP representation
    virtual
    PTimeStampValueBase
    CrTimeStampValue(const PTimeStampInfo& theTimeStampInfo,
                     const PTimeStampValueBase& theInfo);

    //! Get the values for MEDWrapper MED TIMESTAMP from the MED file
    PTimeStampValueBase
    GetPTimeStampValue(const PTimeStampInfo& theTimeStampInfo,
                       const TMKey2Profile& theMKey2Profile,
                       const TKey2Gauss& theKey2Gauss,
                       TErr* theErr = NULL);

    //
    // Backward compatibility declarations
    //

    //! Read the values for MEDWrapper MED TIMESTAMP from the MED file
    virtual
    void
    GetTimeStampVal(const PTimeStampVal& theVal,
                    const TMKey2Profile& theMKey2Profile,
                    const TKey2Gauss& theKey2Gauss,
                    TErr* theErr = NULL);

    //! Write the values for MEDWrapper MED TIMESTAMP into the MED file
    virtual
    void
    SetTimeStamp(const PTimeStampVal& theVal,
                 TErr* theErr = NULL);

    //! Create the values for MEDWrapper MED TIMESTAMP representation
    virtual
    PTimeStampVal
    CrTimeStampVal(const PTimeStampInfo& theTimeStampInfo,
                   const TGeom2Profile& theGeom2Profile = TGeom2Profile(),
                   EModeSwitch theMode = eFULL_INTERLACE);

    //! Copy constructor for the values for MEDWrapper MED TIMESTAMP representation
    virtual
    PTimeStampVal
    CrTimeStampVal(const PTimeStampInfo& theTimeStampInfo,
                   const PTimeStampVal& theInfo);

    //! Get the values for MEDWrapper MED TIMESTAMP from the MED file
    PTimeStampVal
    GetPTimeStampVal(const PTimeStampInfo& theTimeStampInfo,
                     const TMKey2Profile& theMKey2Profile,
                     const TKey2Gauss& theKey2Gauss,
                     TErr* theErr = NULL);

    //! Read a MEDWrapper MED Grille representation from the MED file
    PGrilleInfo
    GetPGrilleInfo(const PMeshInfo& theMeshInfo);

    //! Read a MEDWrapper MED Grille representation from the MED file
    PGrilleInfo
    GetPGrilleInfo(const PMeshInfo& theMeshInfo,
                   const PGrilleInfo& theInfo);

    //! Read a MEDWrapper MED Grille representation from the MED file
    virtual
    void
    GetGrilleInfo(TGrilleInfo& theInfo,
                  TErr* theErr = NULL);

    //! Write the MEDWrapper MED Grille representation into the MED file
    virtual
    void
    SetGrilleInfo(const TGrilleInfo& theInfo,
                  TErr* theErr = NULL);

    void
    SetGrilleInfo(const MED::TGrilleInfo& theGrilleInfo,
                  EModeAcces theMode,
                  TErr* theErr = NULL);

    //! Create the MEDWrapper MED Grille representation
    virtual
    PGrilleInfo
    CrGrilleInfo(const PMeshInfo& theMeshInfo,
                 const PGrilleInfo& theGrilleInfo);

    //! Create the MEDWrapper MED Grille representation
    virtual
    PGrilleInfo
    CrGrilleInfo(const PMeshInfo& theMeshInfo);

    //! Create the MEDWrapper MED Grille representation
    virtual
    PGrilleInfo
    CrGrilleInfo(const PMeshInfo& theMeshInfo,
                 const EGrilleType& type);

    //! Create the MEDWrapper MED Grille representation
    virtual
    PGrilleInfo
    CrGrilleInfo(const PMeshInfo& theMeshInfo,
                 const EGrilleType& type,
                 const TInt& nbNodes);

    //! Create the MEDWrapper MED Grille representation
    virtual
    PGrilleInfo
    CrGrilleInfo(const PMeshInfo& theMeshInfo,
                 const EGrilleType& type,
                 const MED::TIntVector& nbNodeVec);

    //! Get a MEDWrapper MED Grille type
    virtual
    void
    GetGrilleType(const TMeshInfo& theMeshInfo,
                  EGrilleType& type,
                  TErr* theErr = NULL);

    //! Get a MEDWrapper MED Grille structure
    void
    GetGrilleStruct(const MED::TMeshInfo& theMeshInfo,
                    TIntVector& theStruct,
                    TErr* theErr = NULL);

  protected:
    PFile myFile;
    TInt myMajor;
    TInt myMinor;
  };

  //----------------------------------------------------------------------------
  typedef SharedPtr<TWrapper> PWrapper;

  //----------------------------------------------------------------------------
  //! This class provides thread-safety for MEDWrapper interaction
  class MEDWRAPPER_EXPORT TLockProxy
  {
    TLockProxy& operator=(const TLockProxy& );
    TWrapper* myWrapper;

  public:
    TLockProxy(TWrapper* theWrapper);
    ~TLockProxy();

    TWrapper* operator->() const;
  };

  //----------------------------------------------------------------------------
  //! Specialization of SharedPtr for TWrapper
  template<>
  class MEDWRAPPER_EXPORT SharedPtr<TWrapper>: public boost::shared_ptr<TWrapper>
  {
  public:
    SharedPtr() {}

    SharedPtr(TWrapper* p):
      boost::shared_ptr<TWrapper>(p)
    {}

    template<class Y>
    explicit SharedPtr(Y* p):
      boost::shared_ptr<TWrapper>(p)
    {}

    template<class Y>
    SharedPtr(const SharedPtr<Y>& r):
      boost::shared_ptr<TWrapper>(boost::dynamic_pointer_cast<TWrapper,Y>(r))
    {}

    template<class Y>
    SharedPtr&
    operator=(const SharedPtr<Y>& r)
    {
      SharedPtr<TWrapper>(r).swap(*this);
      return *this;
    }

    template<class Y>
    SharedPtr&
    operator()(Y* p) // Y must be complete
    {
      return operator=<Y>(SharedPtr<Y>(p));
    }

    template<class Y>
    SharedPtr&
    operator()(const SharedPtr<Y>& r) // Y must be complete
    {
      return operator=<Y>(SharedPtr<Y>(r));
    }

    TLockProxy
    operator->() const // never throws
    {
      return TLockProxy(this->get());
    }

  protected:
    operator const TWrapper&() const;

    operator TWrapper&();

    TWrapper&
    operator*() const;

    TWrapper*
    get() const // never throws
    {
      return boost::shared_ptr<TWrapper>::get();
    }
  };
}

#endif // MED_Wrapper_HeaderFile
