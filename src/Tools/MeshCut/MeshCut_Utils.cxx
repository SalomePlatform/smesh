// Copyright (C) 2006-2016  EDF R&D
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

#include "MeshCut_Utils.hxx"

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <ctime>

using namespace std;
using namespace MESHCUT;

//string pathUsers = (string) "/var/www/XMeshLab/users/";

bool MESHCUT::estUnTypeMaille(std::string S)
{
  if (S == (string) "POI1" || S == (string) "SEG2" || S == (string) "SEG3" || S == (string) "TRIA3" || S
      == (string) "TRIA6" || S == (string) "QUAD4" || S == (string) "QUAD8" || S == (string) "QUAD9" || S
      == (string) "TETRA4" || S == (string) "TETRA10" || S == (string) "PYRAM5" || S == (string) "PYRAM13" || S
      == (string) "PENTA6" || S == (string) "PENTA15" || S == (string) "HEXA8" || S == (string) "HEXA20" || S
      == (string) "HEXA27")
    return true;
  else
    return false;
}

void MESHCUT::ERREUR(const char* msg)
{
  cout << endl << "====== ERROR ====== " << msg << endl << endl;
  exit(-1);
}

char* MESHCUT::string2char(std::string str)
{
  // créer le buffer pour copier la chaîne
  size_t size = str.size() + 1;
  char* buffer = new char[size];
  // copier la chaîne
  strncpy(buffer, str.c_str(), size);

  // libérer la mémoire
  //delete [] buffer;

  return buffer;
}

std::string MESHCUT::int2string(int k)
{
  std::stringstream oss;
  oss << k;
  return oss.str(); //  oss.seekp (ios_base::beg);
}

float MESHCUT::char2float(const char* ch)
{
  return atof(ch);
}

std::string MESHCUT::float2string(float f)
{
  stringstream buf;
  buf << fixed << f;
  string s = buf.str();
  return s;
}

bool MESHCUT::appartient(std::string e, std::string tableau[], int taille)
{
  for (int i = 0; i < taille; i++)
    if (tableau[i] == e)
      return true;
  return false;
}

float MESHCUT::arrondi(float x)
{
  if (x > 0 && x < 1.0e-5)
    return 0;
  else if (x < 0 && x > -1.0e-5)
    return 0;
  else
    return x;
}

int MESHCUT::numNoeudPointe(std::string b1, std::string b2, std::string b3)
{
  if (b1 == "1" && b2 == "1" && b3 == "2")
    return 2;
  else if (b1 == "1" && b2 == "2" && b3 == "1")
    return 1;
  else if (b1 == "1" && b2 == "2" && b3 == "2")
    return 2;
  else if (b1 == "2" && b2 == "1" && b3 == "1")
    return 3;
  else if (b1 == "2" && b2 == "1" && b3 == "2")
    return 3;
  else if (b1 == "2" && b2 == "2" && b3 == "1")
    return 1;
  else
    return -1;
}

std::string MESHCUT::strip(std::string S)
{
  if (S.empty())
    return S;
  int startIndex = S.find_first_not_of(" ");
  int endIndex = S.find_last_not_of(" ");
  return S.substr(startIndex, (endIndex - startIndex + 1));
}

std::string MESHCUT::entierSur10_g(int i)
{
  if (i > 999999999)
    ERREUR("trying to write a number superior to 999999999 on more than 10 chars");
  if (i < 10)
    return int2string(i) + (string) "         ";
  else if (i < 100)
    return int2string(i) + (string) "        ";
  else if (i < 1000)
    return int2string(i) + (string) "       ";
  else if (i < 10000)
    return int2string(i) + (string) "      ";
  else if (i < 100000)
    return int2string(i) + (string) "     ";
  else if (i < 1000000)
    return int2string(i) + (string) "    ";
  else if (i < 10000000)
    return int2string(i) + (string) "   ";
  else if (i < 100000000)
    return int2string(i) + (string) "  ";
  else if (i < 1000000000)
    return int2string(i) + (string) " ";
  else
    return int2string(i);
}

std::string MESHCUT::entierSur10_d(int i)
{
  if (i > 999999999)
    ERREUR("trying to write a number superior to 999999999 on more than 10 chars");
  if (i < 10)
    return (string) "         " + int2string(i);
  else if (i < 100)
    return (string) "        " + int2string(i);
  else if (i < 1000)
    return (string) "       " + int2string(i);
  else if (i < 10000)
    return (string) "      " + int2string(i);
  else if (i < 100000)
    return (string) "     " + int2string(i);
  else if (i < 1000000)
    return (string) "    " + int2string(i);
  else if (i < 10000000)
    return (string) "   " + int2string(i);
  else if (i < 100000000)
    return (string) "  " + int2string(i);
  else if (i < 1000000000)
    return (string) " " + int2string(i);
  else
    return int2string(i);
}

std::string MESHCUT::typeEnsight(std::string type)
{
  if (type == (string) "POI1")
    return (string) "point";
  else if (type == (string) "SEG2")
    return (string) "bar2";
  else if (type == (string) "SEG3")
    return (string) "bar2";// ATTENTION, triche!
  else if (type == (string) "TRIA3")
    return (string) "tria3";
  else if (type == (string) "TRIA6")
    return (string) "tria3";// ATTENTION, triche!
  else if (type == (string) "QUAD4")
    return (string) "quad4";
  else if (type == (string) "QUAD8")
    return (string) "quad4"; // ATTENTION, triche!
  else if (type == (string) "QUAD9")
    ERREUR("Type QUAD9 not supported by Ensight");
  else if (type == (string) "TETRA4")
    return (string) "tetra4";
  else if (type == (string) "TETRA10")
    return (string) "tetra4"; // ATTENTION, triche!
  else if (type == (string) "PYRAM5")
    return (string) "pyramid5";
  else if (type == (string) "PYRAM13")
    return (string) "pyramid5"; // ATTENTION, triche!
  else if (type == (string) "PENTA6")
    return (string) "penta6";
  else if (type == (string) "PENTA15")
    return (string) "penta6"; // ATTENTION, triche!
  else if (type == (string) "HEXA8")
    return (string) "hexa8";
  else if (type == (string) "HEXA20")
    return (string) "hexa8"; // ATTENTION, triche!
  else if (type == (string) "HEXA27")
    ERREUR("Type HEXA27 not supported by Ensight");
  else
    ERREUR("Type of element not accepted (method \"typeEnsight\"");
  return (string) "";
}

int MESHCUT::Nnoeuds(TYPE_MAILLE type)
{
  switch (type)
  {
    case POI1:
      {
        return 1;
        break;
      }
    case SEG2:
      {
        return 2;
        break;
      }
    case SEG3:
      {
        return 3;
        break;
      }
    case TRIA3:
      {
        return 3;
        break;
      }
    case TRIA6:
      {
        return 6;
        break;
      }
    case QUAD4:
      {
        return 4;
        break;
      }
    case QUAD8:
      {
        return 8;
        break;
      }
      //case QUAD9:                   { return 9; break; }
    case TETRA4:
      {
        return 4;
        break;
      }
    case TETRA10:
      {
        return 10;
        break;
      }
    case PYRAM5:
      {
        return 5;
        break;
      }
    case PYRAM13:
      {
        return 13;
        break;
      }
    case PENTA6:
      {
        return 6;
        break;
      }
    case PENTA15:
      {
        return 15;
        break;
      }
    case HEXA8:
      {
        return 8;
        break;
      }
    case HEXA20:
      {
        return 20;
        break;
      }
      //case HEXA27:                      { return 27;    break; }
    default:
      ERREUR("Type of elem not accepted (method Nnoeuds)");
  }
  return 0;
}

int MESHCUT::NnoeudsGeom(TYPE_MAILLE type)
{
  switch (type)
  {
    case POI1:
      {
        return 1;
        break;
      }
    case SEG2:
      {
        return 2;
        break;
      }
    case SEG3:
      {
        return 2;
        break;
      }
    case TRIA3:
      {
        return 3;
        break;
      }
    case TRIA6:
      {
        return 3;
        break;
      }
    case QUAD4:
      {
        return 4;
        break;
      }
    case QUAD8:
      {
        return 4;
        break;
      }
      //case QUAD9:                   { return 9; break; }
    case TETRA4:
      {
        return 4;
        break;
      }
    case TETRA10:
      {
        return 4;
        break;
      }
    case PYRAM5:
      {
        return 5;
        break;
      }
    case PYRAM13:
      {
        return 5;
        break;
      }
    case PENTA6:
      {
        return 6;
        break;
      }
    case PENTA15:
      {
        return 6;
        break;
      }
    case HEXA8:
      {
        return 8;
        break;
      }
    case HEXA20:
      {
        return 8;
        break;
      }
      //case HEXA27:                      { return 27;    break; }
    default:
      ERREUR("Type of elem not accepted (method NnoeudsGeom)");
  }
  return 0;
}

int MESHCUT::codeGMSH(std::string type)
{
  if (type == (string) "POI1")
    ERREUR("POI1 not taken into account by GMSH");
  else if (type == (string) "SEG2")
    return 1;
  else if (type == (string) "SEG3")
    return 8;
  else if (type == (string) "TRIA3")
    return 2;
  else if (type == (string) "TRIA6")
    return 9;
  else if (type == (string) "QUAD4")
    return 3;
  else if (type == (string) "QUAD8")
    return 16;
  else if (type == (string) "QUAD9")
    return 10;
  else if (type == (string) "TETRA4")
    return 4;
  else if (type == (string) "TETRA10")
    return 11;
  else if (type == (string) "PYRAM5")
    return 7;
  else if (type == (string) "PENTA6")
    return 6;
  else if (type == (string) "PENTA15")
    return 18;
  else if (type == (string) "HEXA8")
    return 5;
  else if (type == (string) "HEXA20")
    return 17;
  else if (type == (string) "HEXA27")
    return 12;
  else
    ERREUR("Type of elem not accepted (method codeGMSH)");
  return 0;
}

std::string MESHCUT::floatEnsight(float x)
{
  char buf[12];
  string s;
  if (x < 0.0)
    sprintf(buf, "%1.5E", x);
  else
    sprintf(buf, " %1.5E", x);
  s = (string) buf;
  s.erase(10, 1);
  return s;
}

bool MESHCUT::typeComplexe(std::string type)
{
  if (type == (string) "SEG3")
    return true;
  else if (type == (string) "TRIA6")
    return true;
  else if (type == (string) "QUAD8")
    return true;
  else if (type == (string) "QUAD9")
    return true;
  else if (type == (string) "TETRA10")
    return true;
  else if (type == (string) "PYRAM13")
    return true;
  else if (type == (string) "PENTA15")
    return true;
  else if (type == (string) "HEXA20")
    return true;
  else if (type == (string) "HEXA27")
    return true;
  else
    return false;
}

std::string MESHCUT::ASTER8(std::string s)
{
  if (s.size() == 0)
    return (s + (string) "        ");
  else if (s.size() == 1)
    return (s + (string) "       ");
  else if (s.size() == 2)
    return (s + (string) "      ");
  else if (s.size() == 3)
    return (s + (string) "     ");
  else if (s.size() == 4)
    return (s + (string) "    ");
  else if (s.size() == 5)
    return (s + (string) "   ");
  else if (s.size() == 6)
    return (s + (string) "  ");
  else if (s.size() == 7)
    return (s + (string) " ");
  else if (s.size() == 8)
    return (s);
  else
    ERREUR("More than 8 char for an ASTER string");
  return (s);
}

/*!
 *  Distance à laquelle doit se tenir l'observateur sur un axe
 *  pour voir sous 90° un objet centré de dimensions a et b selon les deux autres axes.
 *  Si on ne tient pas compte de la dimension de l'objet selon l'axe choisi,
 *  la formule d_obs=max(a,b)/2 donne la cote
 *  qui permet de voir l'objet plat dans un angle de 90°.
 *  A cela il faut ajouter la dimension de l'objet selon l'axe d'observation = c.
 *
 *  @param a dimensions de l'objet selon un des axes normal à l'axe d'observation
 *  @param b dimensions de l'objet selon l'autre axe normal à l'axe d'observation
 *  @param c est la dimension de l'objet selon l'axe d'observation
 */
float MESHCUT::dObservateur(float a, float b, float c)
{
  return (max(a, b) / 2.0 + c);
}

int MESHCUT::copieFichier(std::string source, std::string cible)
{
  FILE *fsource, *fcible;
  char buffer[512];
  int NbLu;
  if ((fsource = fopen(string2char(source), "rb")) == NULL)
    return -1;
  if ((fcible = fopen(string2char(cible), "wb")) == NULL)
    {
      fclose(fsource);
      return -2;
    }
  while ((NbLu = fread(buffer, 1, 512, fsource)) != 0)
    fwrite(buffer, 1, NbLu, fcible);
  fclose(fcible);
  fclose(fsource);
  return 0;
}

med_geometry_type MESHCUT::InstanceMGE(TYPE_MAILLE TYPE)
{
  med_geometry_type typeBanaliseMED = MED_NONE;

  switch (TYPE)
  {
    case POI1:
      typeBanaliseMED = MED_POINT1;
      break; // Attention, piège !
    case SEG2:
      typeBanaliseMED = MED_SEG2;
      break;
    case SEG3:
      typeBanaliseMED = MED_SEG3;
      break;
    case TRIA3:
      typeBanaliseMED = MED_TRIA3;
      break;
    case TRIA6:
      typeBanaliseMED = MED_TRIA6;
      break;
    case QUAD4:
      typeBanaliseMED = MED_QUAD4;
      break;
    case QUAD8:
      typeBanaliseMED = MED_QUAD8;
      break;
    case TETRA4:
      typeBanaliseMED = MED_TETRA4;
      break;
    case TETRA10:
      typeBanaliseMED = MED_TETRA10;
      break;
    case PYRAM5:
      typeBanaliseMED = MED_PYRA5;
      break; // Attention, piège !
    case PYRAM13:
      typeBanaliseMED = MED_PYRA13;
      break; // Attention, piège !
    case PENTA6:
      typeBanaliseMED = MED_PENTA6;
      break;
    case PENTA15:
      typeBanaliseMED = MED_PENTA15;
      break;
    case HEXA8:
      typeBanaliseMED = MED_HEXA8;
      break;
    case HEXA20:
      typeBanaliseMED = MED_HEXA20;
      break;
    default:
      ERREUR("Method InstanceMGE, unknown type ");
  }
  return typeBanaliseMED;
}

int MESHCUT::chrono()
{
  return clock() / CLOCKS_PER_SEC;
}

TYPE_MAILLE MESHCUT::typeMaille(std::string type)
{
  if (type == (string) "POI1")
    return POI1;
  else if (type == (string) "SEG2")
    return SEG2;
  else if (type == (string) "SEG3")
    return SEG3;
  else if (type == (string) "TRIA3")
    return TRIA3;
  else if (type == (string) "TRIA6")
    return TRIA6;
  else if (type == (string) "QUAD4")
    return QUAD4;
  else if (type == (string) "QUAD8")
    return QUAD8;
  else if (type == (string) "TETRA4")
    return TETRA4;
  else if (type == (string) "TETRA10")
    return TETRA10;
  else if (type == (string) "PYRAM5")
    return PYRAM5;
  else if (type == (string) "PYRAM13")
    return PYRAM13;
  else if (type == (string) "PENTA6")
    return PENTA6;
  else if (type == (string) "PENTA15")
    return PENTA15;
  else if (type == (string) "HEXA8")
    return HEXA8;
  else if (type == (string) "HEXA20")
    return HEXA20;
  else
    ERREUR("ERROR method typeMaille, unknown type");
  return POI1;
}

std::string MESHCUT::MGE2string(med_geometry_type MGE)
{
  if (MGE == MED_NONE)
    return (string) "NOEUD";
  else if (MGE == MED_POINT1)
    return (string) "POI1";
  else if (MGE == MED_SEG2)
    return (string) "SEG2";
  else if (MGE == MED_SEG3)
    return (string) "SEG3";
  else if (MGE == MED_TRIA3)
    return (string) "TRIA3";
  else if (MGE == MED_TRIA6)
    return (string) "TRIA6";
  else if (MGE == MED_QUAD4)
    return (string) "QUAD4";
  else if (MGE == MED_QUAD8)
    return (string) "QUAD8";
  else if (MGE == MED_TETRA4)
    return (string) "TETRA4";
  else if (MGE == MED_TETRA10)
    return (string) "TETRA10";
  else if (MGE == MED_PYRA5)
    return (string) "PYRAM5";
  else if (MGE == MED_PYRA13)
    return (string) "PYRAM13";
  else if (MGE == MED_PENTA6)
    return (string) "PENTA6";
  else if (MGE == MED_PENTA15)
    return (string) "PENTA15";
  else if (MGE == MED_HEXA8)
    return (string) "HEXA8";
  else if (MGE == MED_HEXA20)
    return (string) "HEXA20";
  else
    ERREUR("ERROR method MGE2string, unknown type");
  return (string) "NOEUD";
}

std::string MESHCUT::TM2string(TYPE_MAILLE MGE)
{
  if (MGE == POI1)
    return (string) "POI1";
  else if (MGE == SEG2)
    return (string) "SEG2";
  else if (MGE == SEG3)
    return (string) "SEG3";
  else if (MGE == TRIA3)
    return (string) "TRIA3";
  else if (MGE == TRIA6)
    return (string) "TRIA6";
  else if (MGE == QUAD4)
    return (string) "QUAD4";
  else if (MGE == QUAD8)
    return (string) "QUAD8";
  else if (MGE == TETRA4)
    return (string) "TETRA4";
  else if (MGE == TETRA10)
    return (string) "TETRA10";
  else if (MGE == PYRAM5)
    return (string) "PYRAM5";
  else if (MGE == PYRAM13)
    return (string) "PYRAM13";
  else if (MGE == PENTA6)
    return (string) "PENTA6";
  else if (MGE == PENTA15)
    return (string) "PENTA15";
  else if (MGE == HEXA8)
    return (string) "HEXA8";
  else if (MGE == HEXA20)
    return (string) "HEXA20";
  else
    ERREUR("ERROR method TM2string, unknown type");
  return (string) "POI1";
}

TYPE_MAILLE MESHCUT::string2TM(std::string stm)
{
  if (stm == (string) "POI1")
    return POI1;
  else if (stm == (string) "SEG2")
    return SEG2;
  else if (stm == (string) "SEG3")
    return SEG3;
  else if (stm == (string) "TRIA3")
    return TRIA3;
  else if (stm == (string) "TRIA6")
    return TRIA6;
  else if (stm == (string) "QUAD4")
    return QUAD4;
  else if (stm == (string) "QUAD8")
    return QUAD8;
  else if (stm == (string) "TETRA4")
    return TETRA4;
  else if (stm == (string) "TETRA10")
    return TETRA10;
  else if (stm == (string) "PYRAM5")
    return PYRAM5;
  else if (stm == (string) "PYRAM13")
    return PYRAM13;
  else if (stm == (string) "PENTA6")
    return PENTA6;
  else if (stm == (string) "PENTA15")
    return PENTA15;
  else if (stm == (string) "HEXA8")
    return HEXA8;
  else if (stm == (string) "HEXA20")
    return HEXA20;
  else
    ERREUR("ERROR method string2TM, unknown type");
  return POI1;
}

std::string MESHCUT::coordIndex_ILS(TYPE_MAILLE tm)
{
  if (tm == SEG2)
    return (string) " 0,1 ";
  else if (tm == SEG3)
    return (string) " 0,1 "; // Idem SEG2
  else if (tm == TRIA3)
    return (string) " 0,1,2,0 ";
  else if (tm == TRIA6)
    return (string) " 0,1,2,0 ";
  else if (tm == QUAD4)
    return (string) " 0,1,2,3,0 ";
  else if (tm == QUAD8)
    return (string) " 0,1,2,3,0 ";
  else if (tm == TETRA4)
    return (string) " 0,1,2,0,-1, 0,3,-1, 1,3,-1, 2,3,-1 ";
  else if (tm == TETRA10)
    return (string) " 0,1,2,0,-1, 0,3,-1, 1,3,-1, 2,3,-1 ";
  else if (tm == PYRAM5)
    return (string) " 0,1,2,3,0,-1, 0,4,-1, 1,4,-1, 2,4,-1, 3,4,-1 ";
  else if (tm == PYRAM13)
    return (string) " 0,1,2,3,0,-1, 0,4,-1, 1,4,-1, 2,4,-1, 3,4,-1 ";
  else if (tm == PENTA6)
    return (string) " 0,1,2,0,-1, 3,4,5,3,-1, 0,3,-1, 1,4,-1, 2,5,-1 ";
  else if (tm == PENTA15)
    return (string) " 0,1,2,0,-1, 3,4,5,3,-1, 0,3,-1, 1,4,-1, 2,5,-1 ";
  else if (tm == HEXA8)
    return (string) " 0,1,2,3,0,-1, 4,5,6,7,4,-1, 0,4,-1, 1,5,-1, 2,6,-1, 3,7,-1 ";
  else if (tm == HEXA20)
    return (string) " 0,1,2,3,0,-1, 4,5,6,7,4,-1, 0,4,-1, 1,5,-1, 2,6,-1, 3,7,-1 ";
  else
    return (string) "";
}

std::string MESHCUT::coordIndex_IFS(TYPE_MAILLE tm)
{
  if (tm == SEG2)
    return (string) "  ";
  else if (tm == SEG3)
    return (string) "  "; // Idem SEG2
  else if (tm == TRIA3)
    return (string) " 0,1,2,0,-1, 0,2,1,0,-1 ";
  else if (tm == TRIA6)
    return (string) " 0,1,2,0,-1, 0,2,1,0,-1 ";
  else if (tm == QUAD4)
    return (string) " 0,1,2,3,0,-1, 0,3,2,1,0,-1 ";
  else if (tm == QUAD8)
    return (string) " 0,1,2,3,0,-1, 0,3,2,1,0,-1 ";
  else if (tm == TETRA4)
    return (string) " 0,1,2,0,-1, 0,2,1,0,-1, 0,3,1,0,-1, 0,1,3,0,-1, 1,3,2,1,-1, 1,2,3,1,-1, 0,2,3,0,-1, 0,3,2,0,-1 ";
  else if (tm == TETRA10)
    return (string) " 0,1,2,0,-1, 0,2,1,0,-1, 0,3,1,0,-1, 0,1,3,0,-1, 1,3,2,1,-1, 1,2,3,1,-1, 0,2,3,0,-1, 0,3,2,0,-1 ";
  else if (tm == PYRAM5)
    return (string) " 0,1,2,3,0,-1, 0,3,2,1,0,-1, 0,1,4,0,-1, 0,4,1,0,-1, 1,2,4,1,-1, 1,4,2,1,-1, 2,4,3,2,-1, 2,3,4,2,-1, 3,4,0,3,-1, 3,0,4,3,-1 ";
  else if (tm == PYRAM13)
    return (string) " 0,1,2,3,0,-1, 0,3,2,1,0,-1, 0,1,4,0,-1, 0,4,1,0,-1, 1,2,4,1,-1, 1,4,2,1,-1, 2,4,3,2,-1, 2,3,4,2,-1, 3,4,0,3,-1, 3,0,4,3,-1 ";
  else if (tm == PENTA6)
    return (string) " 0,1,2,0,-1, 0,2,1,0,-1, 3,4,5,3,-1, 3,5,4,3,-1, 0,1,4,3,0,-1, 0,3,4,1,0,-1, 1,4,5,2,1,-1, 1,2,5,4,1,-1, 0,3,5,2,0,-1, 0,2,5,3,0,-1 ";
  else if (tm == PENTA15)
    return (string) " 0,1,2,0,-1, 0,2,1,0,-1, 3,4,5,3,-1, 3,5,4,3,-1, 0,1,4,3,0,-1, 0,3,4,1,0,-1, 1,4,5,2,1,-1, 1,2,5,4,1,-1, 0,3,5,2,0,-1, 0,2,5,3,0,-1 ";
  else if (tm == HEXA8)
    return (string) " 0,1,2,3,0,-1, 0,3,2,1,0,-1, 1,5,6,2,1,-1, 1,2,6,5,1,-1, 5,4,7,6,5,-1, 5,6,7,4,5,-1, 4,0,3,7,4,-1, 4,7,3,0,4,-1, 0,4,5,1,0,-1, 0,1,5,4,0,-1, 3,7,6,2,3,-1, 3,2,6,7,3,-1 ";
  else if (tm == HEXA20)
    return (string) " 0,1,2,3,0,-1, 0,3,2,1,0,-1, 1,5,6,2,1,-1, 1,2,6,5,1,-1, 5,4,7,6,5,-1, 5,6,7,4,5,-1, 4,0,3,7,4,-1, 4,7,3,0,4,-1, 0,4,5,1,0,-1, 0,1,5,4,0,-1, 3,7,6,2,3,-1, 3,2,6,7,3,-1 ";
  else
    return (string) "";
}

std::string MESHCUT::SIGNE(double x)
{
  if (x < 0)
    return "-";
  else if (x > 0)
    return "+";
  else
    return "0";
}

void MESHCUT::champType(std::string type, med_entity_type MEM, med_geometry_type MGE, med_idt fid, med_idt fidout,
                        char *maa, char *nomChamp, char *nomChampMoy, med_field_type typeChamp, char *compChamp,
                        char *unitChamp, med_int nCompChamp, std::map<std::string, int> REFGAUSS, int ichamp)
{

  bool debug = true;
  int ipt, nmailles, ngauss, imaille, igauss, icomp;
  //  int ival, ngpdt;
  med_int nval, numdt, numo, nPasTemps;
  char dtunit[MED_SNAME_SIZE + 1] = "";
  char locname[MED_NAME_SIZE + 1] = "";
  char nomprofil[MED_NAME_SIZE + 1] = "";
  med_float dt = 0.0;
  med_float *valr = NULL;
  med_float *valr2 = NULL;
  med_bool local;
  //  med_int nbrefmaa;
  med_field_type fieldType;

  if (MEDfieldInfo(fid, ichamp, nomChamp, maa, &local, &fieldType, compChamp, unitChamp, dtunit, &nPasTemps) < 0)
    ERREUR("Error MEDfieldInfo");
  cout << type << " : " << (int) nPasTemps << " timestep  " << endl;

  for (ipt = 1; ipt <= nPasTemps; ipt++)
    {
      //for (ipt=1; ipt<=min(nPasTemps,1); ipt++) {
      if (debug)
        cout << endl;
      if (debug)
        cout << "************************************************************" << endl;
      if (debug)
        cout << "                    FIELD " << ichamp << endl;
      if (debug)
        cout << "          " << nomChamp << endl;
      if (debug)
        cout << "          " << type << "   ---   Timestep " << ipt << endl;
      if (debug)
        cout << "************************************************************" << endl;
      if (debug)
        cout << endl;

      if (MEDfieldComputingStepInfo(fid, nomChamp, ipt, &numdt, &numo, &dt) < 0)
        {
          cout << endl;
          cout << endl << "####################################################################" << endl;
          cout << "                   ERROR MEDpasdetempsInfo                         " << endl;
          cout << endl << "####################################################################" << endl;
          cout << "                  Field: " << (string) nomChamp << endl;
          cout << "                  Geometrie: " << MGE2string(MGE) << endl;
          cout << "                  Timestep " << ipt << " ignored" << endl;

          continue;
        }

      med_int profilesize, nintegrationpoint;
      nval = MEDfieldnValueWithProfile(fid, nomChamp, numdt, numo, MEM, MGE, ipt, MED_COMPACT_PFLMODE, nomprofil,
                                       &profilesize, locname, &nintegrationpoint);
      if (debug)
        cout << "     Number of values in this timestep: " << (int) nval << endl;

      if (typeChamp == MED_FLOAT64)
        valr = (med_float*) calloc(nCompChamp * nval, sizeof(med_float));
      else
        ERREUR("Type of field not taken into account");

      if (MEDfieldValueWithProfileRd(fid, maa, numdt, numo, MEM, MGE, MED_COMPACT_PFLMODE, nomprofil,
                                     MED_FULL_INTERLACE, MED_ALL_CONSTITUENT, (unsigned char*) valr) < 0)
        {
          cout << endl;
          cout << endl << "####################################################################" << endl;
          cout << "                         ERROR MEDchampLire                        " << endl;
          cout << endl << "####################################################################" << endl;
          cout << endl;
          cout << "   Field: " << (string) nomChamp << endl;
          cout << "   Geometry: " << MGE2string(MGE) << endl;
          cout << "   Timestep " << ipt << " ignored" << endl;
          cout << endl << endl;
          continue;
        }

      if (debug)
        cout << "       profile  = " << (string) nomprofil << endl;
      // Localisation du champ aux points de Gauss
      if (debug)
        cout << "       locname = " << (string) locname << endl;

      if (REFGAUSS[(string) locname])
        {
          ngauss = REFGAUSS[(string) locname];
          if (debug)
            cout << "       " << ngauss << " Gauss points by element)" << endl;
        }
      else
        ngauss = 1;

      nmailles = nval / ngauss;
      if (debug)
        cout << "      Nbre de mailles: " << nmailles << endl;

      if (debug)
        {
          cout << endl << "       Liste des valeurs du champ brut aux 3 premiers éléments:" << endl;
          for (imaille = 0; imaille < min(nmailles, 3); imaille++)
            {
              cout << "         Maille " << imaille << endl;
              for (igauss = 0; igauss < ngauss; igauss++)
                {
                  cout << "             PG " << igauss << " : ";
                  for (icomp = 0; icomp < nCompChamp; icomp++)
                    cout << " " << *(valr + imaille * ngauss * nCompChamp + igauss * nCompChamp + icomp);
                  cout << endl;
                }
              cout << endl;
            }
          cout << endl;
        }

      if (ngauss > 1)
        {

          valr2 = (med_float*) calloc(nCompChamp * nmailles, sizeof(med_float));

          if (debug)
            cout << endl << "       Moyenne sur les PG des mailles" << endl;
          for (imaille = 0; imaille < nmailles; imaille++)
            {
              for (icomp = 0; icomp < nCompChamp; icomp++)
                {
                  float valCompMaille = 0.0;
                  for (igauss = 0; igauss < ngauss; igauss++)
                    valCompMaille += *(valr + imaille * ngauss * nCompChamp + igauss * nCompChamp + icomp);
                  *(valr2 + imaille * nCompChamp + icomp) = valCompMaille / ngauss;

                }
            }

          //cout << endl << "Nom champ moy = " <<  (string)nomChampMoy << endl;
          //cout << endl << "Type champ = " <<  typeChamp << endl;
          //cout << endl << "Comp champ = " <<  (string)compChamp << endl;
          //cout << endl << "Unit champ = " <<  (string)unitChamp << endl;
          //cout << endl << "N comp champ = " <<  nCompChamp << endl;

          if (MEDfieldValueWithProfileWr(fidout, nomChampMoy, numdt, numo, dt, MEM, MGE, MED_COMPACT_PFLMODE,
                                         nomprofil, MED_NO_LOCALIZATION, MED_FULL_INTERLACE, MED_ALL_CONSTITUENT,
                                         (med_int) nmailles, (unsigned char*) valr2) < 0)
            {
              cout << endl;
              cout << endl << "********************************************************************" << endl;
              cout << "********************                         ***********************" << endl;
              cout << "********************   ERROR MEDchampEcr     ***********************" << endl;
              cout << "********************                         ***********************" << endl;
              cout << "********************************************************************" << endl;
              cout << endl;
              cout << "   Champ: " << (string) nomChampMoy << endl;
              cout << "   Géométrie: " << MGE2string(MGE) << endl;
              cout << "   Pas de temps " << ipt << " ignoré" << endl;
              cout << endl << endl;
              continue;
            }

          if (debug)
            cout << "    Writing mean values in new field: OK " << endl;

          // Restitution du champ moyenné
          if (debug)
            {
              cout << endl << "       Liste des valeurs du champ moyenné aux 3 premiers éléments:" << endl;
              for (imaille = 0; imaille < min(nmailles, 3); imaille++)
                {
                  cout << "         Maille " << imaille << endl;
                  for (icomp = 0; icomp < nCompChamp; icomp++)
                    cout << " " << *(valr2 + imaille * nCompChamp + icomp);
                  cout << endl;
                }
              cout << endl;
            }

        }

      free(valr);
      free(valr2);

    } // boucle sur les pas de temps

  cout << endl;
}

std::string MESHCUT::nomMaille(TYPE_MAILLE tm, int nl)
{
  return (TM2string(tm) + (string) "_" + int2string(nl));
}

bool MESHCUT::appartientVN(int n, std::vector<int> V)
{
  bool app = false;
  for (unsigned int i = 0; i < V.size(); i++)
    if (n == V[i])
      {
        app = true;
        break;
      }
  return app;
}

float MESHCUT::distance2(float x1, float y1, float z1, float x2, float y2, float z2)
{
  return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1);
}

/*!
 *  Conversion HL-MED d'une table de connectivités
 */
void MESHCUT::conversionCNX(int *CNXtm, TYPE_MAILLE tm, int N)
{

  int n = Nnoeuds(tm);

  if (tm == TETRA4)
    {
      for (int i = 0; i < N; i++)
        {
          int i1 = CNXtm[i * n + 1];
          int i2 = CNXtm[i * n + 2];
          CNXtm[i * n + 1] = i2;
          CNXtm[i * n + 2] = i1;
        }
    }
  else if (tm == PYRAM5)
    {
      for (int i = 0; i < N; i++)
        {
          int i1 = CNXtm[i * n + 1];
          int i3 = CNXtm[i * n + 3];
          CNXtm[i * n + 1] = i3;
          CNXtm[i * n + 3] = i1;
        }
    }
  else if (tm == PENTA6)
    {
      for (int i = 0; i < N; i++)
        {
          int i0 = CNXtm[i * n + 0];
          int i1 = CNXtm[i * n + 1];
          int i2 = CNXtm[i * n + 2];
          int i3 = CNXtm[i * n + 3];
          int i4 = CNXtm[i * n + 4];
          int i5 = CNXtm[i * n + 5];
          CNXtm[i * n + 0] = i3;
          CNXtm[i * n + 1] = i4;
          CNXtm[i * n + 2] = i5;
          CNXtm[i * n + 3] = i0;
          CNXtm[i * n + 4] = i1;
          CNXtm[i * n + 5] = i2;
        }
    }

  else if (tm == HEXA8)
    {
      for (int i = 0; i < N; i++)
        {
          int i0 = CNXtm[i * n + 0];
          int i1 = CNXtm[i * n + 1];
          int i2 = CNXtm[i * n + 2];
          int i3 = CNXtm[i * n + 3];
          int i4 = CNXtm[i * n + 4];
          int i5 = CNXtm[i * n + 5];
          int i6 = CNXtm[i * n + 6];
          int i7 = CNXtm[i * n + 7];
          CNXtm[i * n + 0] = i4;
          CNXtm[i * n + 1] = i5;
          CNXtm[i * n + 2] = i6;
          CNXtm[i * n + 3] = i7;
          CNXtm[i * n + 4] = i0;
          CNXtm[i * n + 5] = i1;
          CNXtm[i * n + 6] = i2;
          CNXtm[i * n + 7] = i3;
        }
    }
}

