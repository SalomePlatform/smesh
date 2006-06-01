//  SMESH SMDS : implementaion of Salome mesh data structure
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMDS_FacePosition.hxx
//  Module : SMESH

#ifndef _SMDS_FacePosition_HeaderFile
#define _SMDS_FacePosition_HeaderFile

#include "SMDS_Position.hxx"

//#ifdef WNT
//#include <SALOME_WNT.hxx>
//#else
//#define SALOME_WNT_EXPORT
//#endif

#if defined WNT && defined WIN32 && defined SMDS_EXPORTS
#define SMDS_WNT_EXPORT __declspec( dllexport )
#else
#define SMDS_WNT_EXPORT
#endif

class SMDS_WNT_EXPORT SMDS_FacePosition:public SMDS_Position
{

  public:
	SMDS_FacePosition(int aFaceId=0, double aUParam=0,
		double aVParam=0);
	const virtual double * Coords() const;
	SMDS_TypeOfPosition GetTypeOfPosition() const;
	void SetUParameter(double aUparam);
	void SetVParameter(double aVparam);
	double GetUParameter() const;
	double GetVParameter() const;

  private:
	double myUParameter;
	double myVParameter;
};
#endif
