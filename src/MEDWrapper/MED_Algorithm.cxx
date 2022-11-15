// Copyright (C) 2007-2022  CEA/DEN, EDF R&D, OPEN CASCADE
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

#include "MED_Algorithm.hxx"
#include "MED_Wrapper.hxx"
#include "MED_Utilities.hxx"

namespace MED
{
  //---------------------------------------------------------------
  TEntity2TGeom2ElemInfo
  GetEntity2TGeom2ElemInfo(const PWrapper& theWrapper,
                           const PMeshInfo& theMeshInfo,
                           const MED::TEntityInfo& theEntityInfo)
  {
    MSG("GetElemsByEntity(...)");
    TEntity2TGeom2ElemInfo anEntity2TGeom2ElemInfo;
    MED::TEntityInfo::const_iterator anIter = theEntityInfo.begin();
    PElemInfo anElemInfo;
    TErr anErr;
    for(; anIter != theEntityInfo.end(); anIter++){
      const EEntiteMaillage& anEntity = anIter->first;
      const TGeom2Size& aGeom2Size = anIter->second;
      TGeom2ElemInfo& aGeom2ElemInfo = anEntity2TGeom2ElemInfo[anEntity];

      if(anEntity == eNOEUD){
        aGeom2ElemInfo[ePOINT1] = theWrapper->GetPElemInfo(theMeshInfo);
        continue;
      }

      TGeom2Size::const_iterator anIter2 = aGeom2Size.begin();
      for(; anIter2 != aGeom2Size.end(); anIter2++){
        const EGeometrieElement& aGeom = anIter2->first;
        aGeom2ElemInfo[aGeom] = theWrapper->GetPElemInfo(theMeshInfo,anEntity,aGeom,MED::eNOD,&anErr);
      }
    }
    ADDMSG("\n");
    return anEntity2TGeom2ElemInfo;
  }

  //---------------------------------------------------------------
  TFamilyInfoSet
  GetFamilyInfoSet(const PWrapper& theWrapper,
                   const PMeshInfo& theMeshInfo)
  {
    MSG("GetFamilies(...)");
    TErr anErr;
    TFamilyInfoSet aFamilyInfoSet;
    TInt aNbFam = theWrapper->GetNbFamilies(*theMeshInfo);
    INITMSG("GetNbFamilies() = "<<aNbFam<<"\n");
    for(TInt iFam = 1; iFam <= aNbFam; iFam++){
      PFamilyInfo aFamilyInfo = theWrapper->GetPFamilyInfo(theMeshInfo,iFam,&anErr);
      if(anErr >= 0)
        aFamilyInfoSet.insert(aFamilyInfo);
    }
    ADDMSG("\n");
    return aFamilyInfoSet;
  }

  //---------------------------------------------------------------
  TGroupInfo
  GetGroupInfo(const TFamilyInfoSet& theFamilyInfoSet)
  {
    MSG("GetFamiliesByGroup(...)");
    TGroupInfo aGroup;
    TFamilyInfoSet::const_iterator anIter = theFamilyInfoSet.begin();
    for(; anIter != theFamilyInfoSet.end(); anIter++){
      const PFamilyInfo& aFamilyInfo = *anIter;
      TInt aNbGroup = aFamilyInfo->GetNbGroup();
      for(TInt iGroup = 0; iGroup < aNbGroup; iGroup++){
        aGroup[aFamilyInfo->GetGroupName(iGroup)].insert(aFamilyInfo);
      }
    }

    if(SALOME::VerbosityActivated()){
      TGroupInfo::const_iterator anIter = aGroup.begin();
      for(; anIter != aGroup.end(); anIter++){
        const std::string& aName = anIter->first;
        INITMSG("aGroupName = '"<<aName<<"'\n");
        const TFamilyInfoSet& aFamilyInfoSet = anIter->second;
        TFamilyInfoSet::const_iterator anFamIter = aFamilyInfoSet.begin();
        for(; anFamIter != aFamilyInfoSet.end(); anFamIter++){
          const PFamilyInfo& aFamilyInfo = *anFamIter;
          INITMSG("aFamilyName = '"<<aFamilyInfo->GetName()<<"'\n");
        }
      }
      ADDMSG("\n");
    }

    return aGroup;
  }

  //---------------------------------------------------------------
  TFieldInfo2TimeStampInfoSet
  GetFieldInfo2TimeStampInfoSet(const PWrapper& theWrapper,
                                const PMeshInfo& theMeshInfo,
                                const MED::TEntityInfo& theEntityInfo)
  {
    MSG("GetFieldsByEntity(...)");
    TFieldInfo2TimeStampInfoSet aFieldInfo2TimeStampInfoSet;
    TInt aNbFields = theWrapper->GetNbFields();
    INITMSG("GetNbFields() = "<<aNbFields<<"\n");
    for(TInt iField = 1; iField <= aNbFields; iField++){
      PFieldInfo aFieldInfo = theWrapper->GetPFieldInfo(theMeshInfo,iField);
      INITMSG("aFieldName = '"<<aFieldInfo->GetName()<<
              "'; aNbComp = "<<aFieldInfo->GetNbComp()<<"; ");
      TGeom2Size aGeom2Size;
      EEntiteMaillage anEntity = EEntiteMaillage(-1);
      TInt aNbTimeStamps = theWrapper->GetNbTimeStamps(aFieldInfo,theEntityInfo,anEntity,aGeom2Size);
      ADDMSG("anEntity = "<<anEntity<<"; GetNbTimeStamps = "<<aNbTimeStamps<<"\n");
      for(TInt iTimeStamp = 1; iTimeStamp <= aNbTimeStamps; iTimeStamp++){
        PTimeStampInfo aTimeStamp =
          theWrapper->GetPTimeStampInfo(aFieldInfo,anEntity,aGeom2Size,iTimeStamp);
        aFieldInfo2TimeStampInfoSet[aFieldInfo].insert(aTimeStamp);
        INITMSG("aDt = "<<aTimeStamp->GetDt()<<", Unit = \'"<<aTimeStamp->GetUnitDt()<<"\n");
      }
    }
    ADDMSG("\n");
    return aFieldInfo2TimeStampInfoSet;
  }

  //---------------------------------------------------------------
  TEntite2TFieldInfo2TimeStampInfoSet
  GetEntite2TFieldInfo2TimeStampInfoSet(const TFieldInfo2TimeStampInfoSet& theFieldInfo2TimeStampInfoSet)
  {
    TEntite2TFieldInfo2TimeStampInfoSet anEntite2TFieldInfo2TimeStampInfoSet;
    TFieldInfo2TimeStampInfoSet::const_iterator anIter = theFieldInfo2TimeStampInfoSet.begin();
    for(; anIter != theFieldInfo2TimeStampInfoSet.end(); anIter++){
      const TTimeStampInfoSet& aTimeStampInfoSet = anIter->second;
      //const PFieldInfo& aFieldInfo = anIter->first;
      if(aTimeStampInfoSet.empty())
        continue;
      const PTimeStampInfo& aTimeStampInfo = *aTimeStampInfoSet.begin();
      anEntite2TFieldInfo2TimeStampInfoSet[ConvertEntity(aTimeStampInfo->GetEntity())].insert(*anIter);
    }
    return anEntite2TFieldInfo2TimeStampInfoSet;
  }

  //---------------------------------------------------------------
  bool
  operator<(const TFamilyTSize& theLeft, const TFamilyTSize& theRight)
  {
    const MED::PFamilyInfo& aLeftInfo = boost::get<0>(theLeft);
    const MED::PFamilyInfo& aRightInfo = boost::get<0>(theRight);
    return aLeftInfo->GetId() < aRightInfo->GetId();
  }

  //---------------------------------------------------------------
  TEntity2FamilySet
  GetEntity2FamilySet(const PWrapper& /*theWrapper*/,
                      const TEntity2TGeom2ElemInfo& theEntity2TGeom2ElemInfo,
                      const TFamilyInfoSet& theFamilyInfoSet)
  {
    MSG("GetFamiliesByEntity(...)");
    TEntity2FamilySet anEntity2FamilySet;

    typedef std::map<TInt,PFamilyInfo> TId2Family;
    TId2Family anId2Family;
    TFamilyInfoSet::const_iterator anIter = theFamilyInfoSet.begin();
    for(; anIter != theFamilyInfoSet.end(); anIter++){
      const PFamilyInfo& aFamilyInfo = *anIter;
      anId2Family.insert(TId2Family::value_type(aFamilyInfo->GetId(),aFamilyInfo));
    }

    if(!anId2Family.empty()){
      typedef std::map<TInt,TInt> TFamilyID2Size;
      typedef std::map<EEntiteMaillage,TFamilyID2Size> TEntity2FamilyID;
      TEntity2FamilyID anEntity2FamilyID;

      if(!theEntity2TGeom2ElemInfo.empty()){
        TEntity2TGeom2ElemInfo::const_iterator anIter = theEntity2TGeom2ElemInfo.begin();
        for(; anIter != theEntity2TGeom2ElemInfo.end(); anIter++){
          const EEntiteMaillage& anEntity = anIter->first;
          TFamilyID2Size& aFamilyID2Size = anEntity2FamilyID[anEntity];
          const TGeom2ElemInfo& aGeom2ElemInfo = anIter->second;
          TGeom2ElemInfo::const_iterator aGeom2ElemInfoIter = aGeom2ElemInfo.begin();
          for(; aGeom2ElemInfoIter != aGeom2ElemInfo.end(); aGeom2ElemInfoIter++){
            const PElemInfo& aElemInfo = aGeom2ElemInfoIter->second;
            if(TInt aNbElem = aElemInfo->GetNbElem()){
              for(TInt i = 0; i < aNbElem; i++){
                aFamilyID2Size[aElemInfo->GetFamNum(i)] += 1;
              }
            }
          }
        }
      }

      if(!anEntity2FamilyID.empty()){
        TEntity2FamilyID::const_iterator anIter = anEntity2FamilyID.begin();
        for(; anIter != anEntity2FamilyID.end(); anIter++){
          const EEntiteMaillage& anEntity = anIter->first;
          INITMSG("anEntity = "<<anEntity<<":\n");
          const TFamilyID2Size& aFamilyID2Size = anIter->second;
          TFamilyID2Size::const_iterator anIter2 = aFamilyID2Size.begin();
          for(; anIter2 != aFamilyID2Size.end(); anIter2++){
            TInt anId = anIter2->first;
            TInt aSize = anIter2->second;
            TId2Family::const_iterator anIter3 = anId2Family.find(anId);
            if(anIter3 != anId2Family.end()){
              const PFamilyInfo& aFamilyInfo = anIter3->second;
              anEntity2FamilySet[anEntity].insert(TFamilyTSize(aFamilyInfo,aSize));
              INITMSG("aFamilyName = '"<<aFamilyInfo->GetName()<<
                      "' anId = "<<aFamilyInfo->GetId()<<"\n");
            }
          }
        }
      }
    }
    ADDMSG("\n");
    return anEntity2FamilySet;
  }

  //---------------------------------------------------------------
  TKey2Gauss
  GetKey2Gauss(const PWrapper& theWrapper,
               TErr* theErr,
               EModeSwitch theMode)
  {
    INITMSG("GetKey2Gauss - theMode = "<<theMode<<std::endl);
    TKey2Gauss aKey2Gauss;
    TInt aNbGauss = theWrapper->GetNbGauss(theErr);
    for(TInt anId = 1; anId <= aNbGauss; anId++){
      TGaussInfo::TInfo aPreInfo = theWrapper->GetGaussPreInfo(anId);
      PGaussInfo anInfo = theWrapper->CrGaussInfo(aPreInfo,theMode);
      theWrapper->GetGaussInfo(anId,anInfo,theErr);
      TGaussInfo::TKey aKey = boost::get<0>(aPreInfo);
      aKey2Gauss[aKey] = anInfo;

      if (SALOME::VerbosityActivated())
      {
        const EGeometrieElement& aGeom = boost::get<0>(aKey);
        const std::string& aName = boost::get<1>(aKey);
        INITMSG("- aGeom = "<<aGeom<<"; aName = '"<<aName<<"'"<<std::endl);
      }
    }
    
    return aKey2Gauss;
  }

  //---------------------------------------------------------------
  PProfileInfo
  GetProfileInfo(const PWrapper& theWrapper,
                 const std::string& theProfileName,
                 TErr* theErr,
                 EModeProfil theMode)
  {
    PProfileInfo anInfo;
    TInt aNbProfiles = theWrapper->GetNbProfiles(theErr);
    for(TInt anId = 1; anId <= aNbProfiles; anId++){
      TProfileInfo::TInfo aPreInfo = theWrapper->GetProfilePreInfo(anId);
      const std::string& aName = boost::get<0>(aPreInfo);
      if(aName == theProfileName)
        return theWrapper->GetPProfileInfo(anId,theMode,theErr);
    }
    return anInfo;
  }

  //---------------------------------------------------------------
  TMKey2Profile
  GetMKey2Profile(const PWrapper& theWrapper,
                  TErr* theErr,
                  EModeProfil theMode)
  {
    INITMSG("GetMKey2Profile - theMode = "<<theMode<<std::endl);
    TKey2Profile aKey2Profile;
    TInt aNbProfiles = theWrapper->GetNbProfiles(theErr);
    for(TInt anId = 1; anId <= aNbProfiles; anId++){
      TProfileInfo::TInfo aPreInfo = theWrapper->GetProfilePreInfo(anId);
      PProfileInfo anInfo = theWrapper->GetPProfileInfo(anId,theMode,theErr);
      const std::string& aName = boost::get<0>(aPreInfo);
      aKey2Profile[aName] = anInfo;

      if(SALOME::VerbosityActivated()){
        INITMSG("- aName = '"<<aName<<"'"<<" : "<<std::endl);
        TInt aNbElem = anInfo->GetSize();

        for(TInt iElem = 0; iElem < aNbElem; iElem++){
          ADDMSG(anInfo->GetElemNum(iElem)<<", ");
        }

        ADDMSG(std::endl);
      }
    }
    return TMKey2Profile(theMode,aKey2Profile);
  }

  //---------------------------------------------------------------
  EEntiteMaillage
  GetEntityByFamilyId(PGrilleInfo& theInfo,
                      TInt theId)
  {
    TElemNum::iterator aNodeFamIter = (theInfo->myFamNumNode).begin();
    for(;aNodeFamIter != (theInfo->myFamNumNode).end(); aNodeFamIter++){
      if(theId == *aNodeFamIter)
        return eNOEUD;
    }
    TElemNum::iterator aCellFamIter = (theInfo->myFamNum).begin();
    for(;aCellFamIter != (theInfo->myFamNum).end(); aCellFamIter++){
      if(theId == *aCellFamIter)
        return eMAILLE;
    }
    EXCEPTION(std::runtime_error, "GetEntityByFamilyId - fails");
    return EEntiteMaillage(-1);
  }

  //---------------------------------------------------------------
  TFamilyID2NbCells
  GetFamilyID2NbCells(PGrilleInfo& theInfo)
  {
    TFamilyID2NbCells aFamily2NbCells;
    TInt aNbNodes = theInfo->myFamNumNode.size();
    TInt aNbCells = theInfo->myFamNum.size();
    for(TInt i=0; i<aNbNodes; i++) aFamily2NbCells[theInfo->GetFamNumNode(i)] = 0;
    for(TInt i=0; i<aNbCells; i++) aFamily2NbCells[theInfo->GetFamNum(i)] = 0;
    for(TInt i=0; i<aNbNodes; i++) aFamily2NbCells[theInfo->GetFamNumNode(i)] += 1;
    for(TInt i=0; i<aNbCells; i++) aFamily2NbCells[theInfo->GetFamNum(i)] += 1;
    return aFamily2NbCells;
  }

  //---------------------------------------------------------------
  EEntiteMaillage
  ConvertEntity(const EEntiteMaillage& aEntity)
  {
    switch( aEntity ){
    case eNOEUD_ELEMENT:
    case eMAILLE:
      return eMAILLE; // eNOEUD_ELEMENT is eMAILLE
    case eFACE:
    case eARETE:
    case eNOEUD:
      return aEntity;
    default:
      break;
    }
    return EEntiteMaillage(-1);
  }
}
