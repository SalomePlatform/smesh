..
   Copyright (C) 2015-2016 EDF

   This file is part of SALOME HYDRO module.

   SALOME HYDRO module is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   SALOME HYDRO module is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with SALOME HYDRO module.  If not, see <http://www.gnu.org/licenses/>.


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Documentation du plugin blocFissure
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


Le plugin blocFissure traite l'insertion de fissure dans un maillage sain.

###################
Outil métier coudes
################### 

Cet outil génère des maillages de tuyaux coudés, avec des fissures de différents types, au niveau du coude.
Cet outil traite un cas particulier de l'outil général d'insertion de fissure dans un maillage sain.

##########################################
Insertion de fissure dans un maillage sain
##########################################

Partant d'un maillage sain et d'une face de fissure décrite en CAO, l'outil modifie le maillage sain,
en introduisant un maillage de la fissure, sous la forme d'un tore rayonnant le long de la ligne de fond de fissure.
