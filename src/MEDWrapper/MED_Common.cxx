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

#include "MED_Common.hxx"
#include <med.h>

namespace MED
{
  TInt
  GetDESCLength()
  {
    return 200;
  }

  TInt
  GetIDENTLength()
  {
    return 8;
  }

  TInt
  GetNOMLength()
  {
    return 64;
  }

  TInt
  GetLNOMLength()
  {
    return 80;
  }

  TInt
  GetPNOMLength()
  {
    return 16;
  }

  void
  GetVersionRelease(TInt& major, TInt& minor, TInt& release)
  {
    major   = MED_MAJOR_NUM;
    minor   = MED_MINOR_NUM;
    release = MED_RELEASE_NUM;
  }

  TInt
  GetNbConn(EGeometrieElement typmai,
            EEntiteMaillage /*typent*/,
            TInt /*mdim*/)
  {
    return typmai % 100;
  }

  TInt
  GetNbNodes(EGeometrieElement typmai)
  {
    return typmai % 100;
  }

  const MED::TEntity2GeomSet& GetEntity2GeomSet()
  {
    static MED::TEntity2GeomSet Entity2GeomSet;

    if ( Entity2GeomSet.empty() ) {
      TGeomSet& aGeomARETESet = Entity2GeomSet[MED::eARETE];
      aGeomARETESet.insert(MED::eSEG2);
      aGeomARETESet.insert(MED::eSEG3);

      TGeomSet& aGeomFACESet = Entity2GeomSet[MED::eFACE];
      aGeomFACESet.insert(MED::eTRIA3);
      aGeomFACESet.insert(MED::eQUAD4);
      aGeomFACESet.insert(MED::eTRIA6);
      aGeomFACESet.insert(MED::eTRIA7);
      aGeomFACESet.insert(MED::eQUAD8);
      aGeomFACESet.insert(MED::eQUAD9);
      aGeomFACESet.insert(MED::ePOLYGONE);
      aGeomFACESet.insert(MED::ePOLYGON2);

      TGeomSet& aGeomMAILLESet = Entity2GeomSet[MED::eMAILLE];
      aGeomMAILLESet.insert(MED::ePOINT1);
      aGeomMAILLESet.insert(aGeomARETESet.begin(), aGeomARETESet.end());
      aGeomMAILLESet.insert(aGeomFACESet.begin(), aGeomFACESet.end());
      aGeomMAILLESet.insert(MED::eTETRA4);
      aGeomMAILLESet.insert(MED::ePYRA5);
      aGeomMAILLESet.insert(MED::ePENTA6);
      aGeomMAILLESet.insert(MED::eHEXA8);
      aGeomMAILLESet.insert(MED::eOCTA12);
      aGeomMAILLESet.insert(MED::eTETRA10);
      aGeomMAILLESet.insert(MED::ePYRA13);
      aGeomMAILLESet.insert(MED::ePENTA15);
      aGeomMAILLESet.insert(MED::ePENTA18);
      aGeomMAILLESet.insert(MED::eHEXA20);
      aGeomMAILLESet.insert(MED::eHEXA27);
      aGeomMAILLESet.insert(MED::ePOLYEDRE);

      /* This combination allows reading nb of models of structure elements */
      Entity2GeomSet[MED::eSTRUCT_ELEMENT].insert(MED::eAllGeoType);
    }

    return Entity2GeomSet;
  }
}
