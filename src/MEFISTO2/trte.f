c  MEFISTO : library to compute 2D triangulation from segmented boundaries
c
c  Copyright (C) 2003  Laboratoire J.-L. Lions UPMC Paris
c
c  This library is free software; you can redistribute it and/or
c  modify it under the terms of the GNU Lesser General Public
c  License as published by the Free Software Foundation; either
c  version 2.1 of the License.
c
c  This library is distributed in the hope that it will be useful,
c  but WITHOUT ANY WARRANTY; without even the implied warranty of
c  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
c  Lesser General Public License for more details.
c
c  You should have received a copy of the GNU Lesser General Public
c  License along with this library; if not, write to the Free Software
c  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
c
c  See http://www.ann.jussieu.fr/~perronne or email Perronnet@ann.jussieu.fr
c
c
c  File   : trte.f
c  Module : SMESH
c  Author: Alain PERRONNET

      subroutine qutr2d( p1, p2, p3, qualite )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :     calculer la qualite d'un triangle de r**2
c -----     2 coordonnees des 3 sommets en double precision
c
c entrees :
c ---------
c p1,p2,p3 : les 3 coordonnees des 3 sommets du triangle
c            sens direct pour une surface et qualite >0
c sorties :
c ---------
c qualite: valeur de la qualite du triangle entre 0 et 1 (equilateral)
c          1 etant la qualite optimale
c ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet analyse numerique upmc paris     janvier 1995
c2345x7..............................................................012
      parameter  ( d2uxr3 = 3.4641016151377544d0 )
c                  d2uxr3 = 2 * sqrt(3)
      double precision  p1(2), p2(2), p3(2), qualite, a, b, c, p
c
c     la longueur des 3 cotes
      a = sqrt( (p2(1)-p1(1))**2 + (p2(2)-p1(2))**2 )
      b = sqrt( (p3(1)-p2(1))**2 + (p3(2)-p2(2))**2 )
      c = sqrt( (p1(1)-p3(1))**2 + (p1(2)-p3(2))**2 )
c
c     demi perimetre
      p = (a+b+c) * 0.5d0
c
      if ( (a*b*c) .ne. 0d0 ) then
c        critere : 2 racine(3) * rayon_inscrit / plus longue arete
         qualite = d2uxr3 * sqrt( abs( (p-a) / p * (p-b) * (p-c) ) )
     %          / max(a,b,c)
      else
         qualite = 0d0
      endif
c
c
c     autres criteres possibles:
c     critere : 2 * rayon_inscrit / rayon_circonscrit
c     qualite = 8d0 * (p-a) * (p-b) * (p-c) / (a * b * c)
c
c     critere : 3*sqrt(3.) * ray_inscrit / demi perimetre
c     qualite = 3*sqrt(3.) * sqrt ((p-a)*(p-b)*(p-c) / p**3)
c
c     critere : 2*sqrt(3.) * ray_inscrit / max( des aretes )
c     qualite = 2*sqrt(3.) * sqrt( (p-a)*(p-b)*(p-c) / p ) / max(a,b,c)
      end


      double precision function surtd2( p1 , p2 , p3 )
c ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but : calcul de la surface d'un triangle defini par 3 points de R**2
c -----
c parametres d entree :
c ---------------------
c p1 p2 p3 : les 3 fois 2 coordonnees des sommets du triangle
c
c parametre resultat :
c --------------------
c surtd2 : surface du triangle
c ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet analyse numerique upmc paris     fevrier 1992
c2345x7..............................................................012
      double precision  p1(2), p2(2), p3(2)
c
c     la surface du triangle
      surtd2 = ( ( p2(1)-p1(1) ) * ( p3(2)-p1(2) )
     %         - ( p2(2)-p1(2) ) * ( p3(1)-p1(1) ) ) * 0.5d0
      end

      integer function nopre3( i )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :   numero precedent i dans le sens circulaire  1 2 3 1 ...
c -----
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc    fevrier 1992
c2345x7..............................................................012
      if( i .eq. 1 ) then
         nopre3 = 3
      else
         nopre3 = i - 1
      endif
      end

      integer function nosui3( i )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :   numero suivant i dans le sens circulaire  1 2 3 1 ...
c -----
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc    fevrier 1992
c2345x7..............................................................012
      if( i .eq. 3 ) then
         nosui3 = 1
      else
         nosui3 = i + 1
      endif
      end

      subroutine provec( v1 , v2 , v3 )
c ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    v3 vecteur = produit vectoriel de 2 vecteurs de r ** 3
c -----
c entrees:
c --------
c v1, v2 : les 2 vecteurs de 3 composantes
c
c sortie :
c --------
c v3     : vecteur = v1  produit vectoriel v2
cc++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : perronnet alain upmc analyse numerique paris        mars 1987
c2345x7..............................................................012
      double precision    v1(3), v2(3), v3(3)
c
      v3( 1 ) = v1( 2 ) * v2( 3 ) - v1( 3 ) * v2( 2 )
      v3( 2 ) = v1( 3 ) * v2( 1 ) - v1( 1 ) * v2( 3 )
      v3( 3 ) = v1( 1 ) * v2( 2 ) - v1( 2 ) * v2( 1 )
c
      return
      end

      subroutine norme1( n, v, ierr )
c ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :   normalisation euclidienne a 1 d un vecteur v de n composantes
c -----
c entrees :
c ---------
c n       : nombre de composantes du vecteur
c
c modifie :
c ---------
c v       : le vecteur a normaliser a 1
c
c sortie  :
c ---------
c ierr    : 1 si la norme de v est egale a 0
c           0 si pas d'erreur
c ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet analyse numerique paris             mars 1987
c ......................................................................
      double precision  v( n ), s, sqrt
c
      s = 0.0d0
      do 10 i=1,n
         s = s + v( i ) * v( i )
   10 continue
c
c     test de nullite de la norme du vecteur
c     --------------------------------------
      if( s .le. 0.0d0 ) then
c        norme nulle du vecteur non normalisable a 1
         ierr = 1
         return
      endif
c
      s = 1.0d0 / sqrt( s )
      do 20 i=1,n
         v( i ) = v ( i ) * s
   20 continue
c
      ierr = 0
      end


      subroutine insoar( mxsomm, mosoar, mxsoar, n1soar, nosoar )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    initialiser le tableau nosoar pour le hachage des aretes
c -----
c
c entrees:
c --------
c mxsomm : plus grand numero de sommet d'une arete au cours du calcul
c mosoar : nombre maximal d'entiers par arete du tableau nosoar
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c          avec mxsoar>=3*mxsomm
c
c sorties:
c --------
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c          une arete i de nosoar est vide  <=>  nosoar(1,i)=0
c          chainage des aretes vides amont et aval
c          l'arete vide qui precede=nosoar(4,i)
c          l'arete vide qui suit   =nosoar(5,i)
c nosoar : numero des 2 sommets, no ligne, 2 triangles de l'arete,
c          chainage momentan'e d'aretes, chainage du hachage des aretes
c          hachage des aretes = min( nosoar(1), nosoar(2) )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c2345x7..............................................................012
      integer   nosoar(mosoar,mxsoar)
c
c     initialisation des aretes 1 a mxsomm
      do 10 i=1,mxsomm
c
c        sommet 1 = 0 <=> temoin d'arete vide pour le hachage
         nosoar( 1, i ) = 0
c
c        arete sur aucune ligne
         nosoar( 3, i ) = 0
c
c        la position de l'arete interne ou frontaliere est inconnue
         nosoar( 6, i ) = -2
c
c        fin de chainage du hachage pas d'arete suivante
         nosoar( mosoar, i ) = 0
c
 10   continue
c
c     la premiere arete vide chainee est la mxsomm+1 du tableau
c     car ces aretes ne sont pas atteignables par le hachage direct
      n1soar = mxsomm + 1
c
c     initialisation des aretes vides et des chainages
      do 20 i = n1soar, mxsoar
c
c        sommet 1 = 0 <=> temoin d'arete vide pour le hachage
         nosoar( 1, i ) = 0
c
c        arete sur aucune ligne
         nosoar( 3, i ) = 0
c
c        chainage sur l'arete vide qui precede
c        (si arete occupee cela deviendra le no du triangle 1 de l'arete)
         nosoar( 4, i ) = i-1
c
c        chainage sur l'arete vide qui suit
c        (si arete occupee cela deviendra le no du triangle 2 de l'arete)
         nosoar( 5, i ) = i+1
c
c        chainages des aretes frontalieres ou internes ou ...
         nosoar( 6, i ) = -2
c
c        fin de chainage du hachage
         nosoar( mosoar, i ) = 0
c
 20   continue
c
c     la premiere arete vide n'a pas de precedent
      nosoar( 4, n1soar ) = 0
c
c     la derniere arete vide est mxsoar sans arete vide suivante
      nosoar( 5, mxsoar ) = 0
      end


      subroutine azeroi ( l , ntab )
c ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but : initialisation a zero d un tableau ntab de l variables entieres
c -----
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet analyse numerique upmc paris septembre 1988
c23456---------------------------------------------------------------012
      integer ntab(l)
      do 1 i = 1 , l
         ntab( i ) = 0
    1 continue
      end


      subroutine fasoar( ns1,    ns2,    nt1,    nt2,    nolign,
     %                   mosoar, mxsoar, n1soar, nosoar, noarst,
     %                   noar,   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    former l'arete de sommet ns1-ns2 dans le hachage du tableau
c -----    nosoar des aretes de la triangulation
c
c entrees:
c --------
c ns1 ns2: numero pxyd des 2 sommets de l'arete
c nt1    : numero du triangle auquel appartient l'arete
c          nt1=-1 si numero inconnu
c nt2    : numero de l'eventuel second triangle de l'arete si connu
c          nt2=-1 si numero inconnu
c nolign : numero de la ligne de l'arete dans ladefi(wulftr-1+nolign)
c          =0 si l'arete n'est une arete de ligne
c          ce numero est ajoute seulement si l'arete est creee
c mosoar : nombre maximal d'entiers par arete du tableau nosoar
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c
c modifies:
c ---------
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c          une arete i de nosoar est vide  <=>  nosoar(1,i)=0
c          chainage des aretes vides amont et aval
c          l'arete vide qui precede=nosoar(4,i)
c          l'arete vide qui suit   =nosoar(5,i)
c nosoar : numero des 2 sommets, no ligne, 2 triangles de l'arete,
c          chainage momentan'e d'aretes, chainage du hachage des aretes
c          hachage des aretes = min( nosoar(1), nosoar(2) )
c noarst : noarst(np) numero d'une arete du sommet np
c
c ierr   : si < 0  en entree pas d'affichage en cas d'erreur du type
c         "arete appartenant a plus de 2 triangles et a creer!"
c          si >=0  en entree       affichage de ce type d'erreur
c
c sorties:
c --------
c noar   : >0 numero de l'arete retrouvee ou ajoutee
c ierr   : =0 si pas d'erreur
c          =1 si le tableau nosoar est sature
c          =2 si arete a creer et appartenant a 2 triangles distincts
c             des triangles nt1 et nt2
c          =3 si arete appartenant a 2 triangles distincts
c             differents des triangles nt1 et nt2
c          =4 si arete appartenant a 2 triangles distincts
c             dont le second n'est pas le triangle nt2
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c2345x7..............................................................012
      common / unites / lecteu, imprim, nunite(30)
      integer           nosoar(mosoar,mxsoar), noarst(*)
      integer           nu2sar(2)
c
c     ajout eventuel de l'arete s1 s2 dans nosoar
      nu2sar(1) = ns1
      nu2sar(2) = ns2
c
c     hachage de l'arete de sommets nu2sar
      call hasoar( mosoar, mxsoar, n1soar, nosoar, nu2sar, noar )
c     en sortie: noar>0 => no arete retrouvee
c                    <0 => no arete ajoutee
c                    =0 => saturation du tableau nosoar
c
      if( noar .eq. 0 ) then
c
c        saturation du tableau nosoar
         write(imprim,*) 'fasoar: tableau nosoar sature'
         ierr = 1
         return
c
      else if( noar .lt. 0 ) then
c
c        l'arete a ete ajoutee. initialisation des autres informations
         noar = -noar
c        le numero de la ligne de l'arete
         nosoar(3,noar) = nolign
c        le triangle 1 de l'arete => le triangle nt1
         nosoar(4,noar) = nt1
c        le triangle 2 de l'arete => le triangle nt2
         nosoar(5,noar) = nt2
c
c        le sommet appartient a l'arete noar
         noarst( nu2sar(1) ) = noar
         noarst( nu2sar(2) ) = noar
c
      else
c
c        l'arete a ete retrouvee.
c        si elle appartient a 2 triangles differents de nt1 et nt2
c        alors il y a une erreur
         if( nosoar(4,noar) .gt. 0 .and.
     %       nosoar(5,noar) .gt. 0 ) then
             if( nosoar(4,noar) .ne. nt1 .and.
     %           nosoar(4,noar) .ne. nt2 .or.
     %           nosoar(5,noar) .ne. nt1 .and.
     %           nosoar(5,noar) .ne. nt2 ) then
c                arete appartenant a plus de 2 triangles => erreur
                 if( ierr .ge. 0 ) then
                    write(imprim,*) 'erreur fasoar: arete ',noar,
     %              ' dans 2 triangles et a creer!'
                 endif
                 ierr = 2
                 return
             endif
         endif
c
c        mise a jour du numero des triangles de l'arete noar
c        le triangle 2 de l'arete => le triangle nt1
         if( nosoar(4,noar) .lt. 0 ) then
c            pas de triangle connu pour cette arete
             n = 4
         else
c            deja un triangle connu. ce nouveau est le second
             if( nosoar(5,noar) .gt. 0  .and.  nt1 .gt. 0 .and.
     %          nosoar(5,noar) .ne. nt1 ) then
c               arete appartenant a plus de 2 triangles => erreur
                write(imprim,*) 'erreur fasoar: arete ',noar,
     %          ' dans plus de 2 triangles'
                ierr = 3
                return
             endif
             n = 5
         endif
         nosoar(n,noar) = nt1
c
c        cas de l'arete frontaliere retrouvee comme diagonale d'un quadrangle
         if( nt2 .gt. 0 ) then
c           l'arete appartient a 2 triangles
            if( nosoar(5,noar) .gt. 0  .and.
     %          nosoar(5,noar) .ne. nt2 ) then
c               arete appartenant a plus de 2 triangles => erreur
                write(imprim,*) 'erreur fasoar: arete ',noar,
     %         ' dans plus de 2 triangles'
                ierr = 4
                return
            endif
            nosoar(5,noar) = nt2
         endif
c
      endif
c
c     pas d'erreur
      ierr = 0
      end

      subroutine fq1inv( x, y, s, xc, yc, ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :   calcul des 2 coordonnees (xc,yc) dans le carre (0,1)
c -----   image par f:carre unite-->quadrangle appartenant a q1**2
c         par une resolution directe due a nicolas thenault
c
c entrees:
c --------
c x,y   : coordonnees du point image dans le quadrangle de sommets s
c s     : les 2 coordonnees des 4 sommets du quadrangle
c
c sorties:
c --------
c xc,yc : coordonnees dans le carre dont l'image par f vaut (x,y)
c ierr  : 0 si calcul sans erreur, 1 si quadrangle degenere
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteurs: thenault tulenew  analyse numerique paris        janvier 1998
c modifs : perronnet alain   analyse numerique paris        janvier 1998
c234567..............................................................012
      real             s(1:2,1:4), dist(2)
      double precision a,b,c,d,alpha,beta,gamma,delta,x0,y0,t(2),u,v,w
c
      a = s(1,1)
      b = s(1,2) - s(1,1)
      c = s(1,4) - s(1,1)
      d = s(1,1) - s(1,2) + s(1,3) - s(1,4)
c
      alpha = s(2,1)
      beta  = s(2,2) - s(2,1)
      gamma = s(2,4) - s(2,1)
      delta = s(2,1) - s(2,2) + s(2,3) - s(2,4)
c
      u = beta  * c - b * gamma
      if( u .eq. 0 ) then
c        quadrangle degenere
         ierr = 1
         return
      endif
      v = delta * c - d * gamma
      w = b * delta - beta * d
c
      x0 = c * (y-alpha) - gamma * (x-a)
      y0 = b * (y-alpha) - beta  * (x-a)
c
      a = v  * w
      b = u  * u - w * x0 - v * y0
      c = x0 * y0
c
      if( a .ne. 0 ) then
c
         delta = sqrt( b*b-4*a*c )
         if( b .ge. 0.0 ) then
            t(2) = -b - delta
         else
            t(2) = -b + delta
         endif
c        la racine de plus grande valeur absolue
c       (elle donne le plus souvent le point exterieur au carre unite
c        donc a tester en second pour reduire les calculs)
         t(2) = t(2) / ( 2 * a )
c        calcul de la seconde racine a partir de la somme => plus stable
         t(1) = - b/a - t(2)
c
         do 10 i=1,2
c
c           la solution i donne t elle un point interne au carre unite?
            xc = ( x0 - v * t(i) ) / u
            yc = ( w * t(i) - y0 ) / u
            if( 0.0 .le. xc .and. xc .le. 1.0 ) then
               if( 0.0 .le. yc .and. yc .le. 1.0 ) goto 9000
            endif
c
c           le point (xc,yc) n'est pas dans le carre unite
c           cela peut etre du aux erreurs d'arrondi
c           => choix par le minimum de la distance aux bords du carre
            dist(i) = max( 0.0, -xc, xc-1.0, -yc, yc-1.0 )
c
 10      continue
c
         if( dist(1) .gt. dist(2) ) then
c           f(xc,yc) pour la racine 2 est plus proche de x,y
c           xc yc sont deja calcules
            goto 9000
         endif
c
      else if ( b .ne. 0 ) then
         t(1) = - c / b
      else
         t(1) = 0
      endif
c
c     les 2 coordonnees du point dans le carre unite
      xc = ( x0 - v * t(1) ) / u
      yc = ( w * t(1) - y0 ) / u
c
 9000 ierr = 0
      return
      end


      subroutine ptdatr( point, pxyd, nosotr, nsigne )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    le point est il dans le triangle de sommets nosotr
c -----
c
c entrees:
c --------
c point  : les 2 coordonnees du point
c pxyd   : les 2 coordonnees et distance souhaitee des points du maillage
c nosotr : le numero des 3 sommets du triangle
c
c sorties:
c --------
c nsigne : >0 si le point est dans le triangle ou sur une des 3 aretes
c          =0 si le triangle est degenere ou indirect ou ne contient pas le poin
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
      integer           nosotr(3)
      double precision  point(2), pxyd(3,*)
      double precision  xp,yp, x1,x2,x3, y1,y2,y3, d,dd, cb1,cb2,cb3
c
      xp = point( 1 )
      yp = point( 2 )
c
      n1 = nosotr( 1 )
      x1 = pxyd( 1 , n1 )
      y1 = pxyd( 2 , n1 )
c
      n2 = nosotr( 2 )
      x2 = pxyd( 1 , n2 )
      y2 = pxyd( 2 , n2 )
c
      n3 = nosotr( 3 )
      x3 = pxyd( 1 , n3 )
      y3 = pxyd( 2 , n3 )
c
c     2 fois la surface du triangle = determinant de la matrice
c     de calcul des coordonnees barycentriques du point p
      d  = ( x2 - x1 ) * ( y3 - y1 ) - ( x3 - x1 ) * ( y2 - y1 )
c
      if( d .gt. 0 ) then
c
c        triangle non degenere
c        =====================
c        calcul des 3 coordonnees barycentriques du
c        point xp yp dans le triangle
         cb1 = ( ( x2-xp ) * ( y3-yp ) - ( x3-xp ) * ( y2-yp ) ) / d
         cb2 = ( ( x3-xp ) * ( y1-yp ) - ( x1-xp ) * ( y3-yp ) ) / d
         cb3 = 1d0 - cb1 -cb2
ccc         cb3 = ( ( x1-xp ) * ( y2-yp ) - ( x2-xp ) * ( y1-yp ) ) / d
c
ccc         if( cb1 .ge. -0.00005d0 .and. cb1 .le. 1.00005d0 .and.
         if( cb1 .ge. 0d0 .and. cb1 .le. 1d0 .and.
     %       cb2 .ge. 0d0 .and. cb2 .le. 1d0 .and.
     %       cb3 .ge. 0d0 .and. cb3 .le. 1d0 ) then
c
c           le triangle nosotr contient le point
            nsigne = 1
         else
            nsigne = 0
         endif
c
      else
c
c        triangle degenere
c        =================
c        le point est il du meme cote que le sommet oppose de chaque arete?
         nsigne = 0
         do 10 i=1,3
c           le sinus de l'angle p1 p2-p1 point
            x1  = pxyd(1,n1)
            y1  = pxyd(2,n1)
            d   = ( pxyd(1,n2) - x1 ) * ( point(2) - y1 )
     %          - ( pxyd(2,n2) - y1 ) * ( point(1) - x1 )
            dd  = ( pxyd(1,n2) - x1 ) * ( pxyd(2,n3) - y1 )
     %          - ( pxyd(2,n2) - y1 ) * ( pxyd(1,n3) - x1 )
            cb1 = ( pxyd(1,n2) - x1 ) ** 2
     %          + ( pxyd(2,n2) - y1 ) ** 2
            cb2 = ( point(1) - x1 ) ** 2
     %          + ( point(2) - y1 ) ** 2
            cb3 = ( pxyd(1,n3) - x1 ) ** 2
     %          + ( pxyd(2,n3) - y1 ) ** 2
            if( abs( dd ) .le. 1e-4 * sqrt( cb1 * cb3 ) ) then
c              le point 3 est sur l'arete 1-2
c              le point doit y etre aussi
               if( abs( d ) .le. 1e-4 * sqrt( cb1 * cb2 ) ) then
c                 point sur l'arete
                  nsigne = nsigne + 1
               endif
            else
c              le point 3 n'est pas sur l'arete . test des signes
               if( d * dd .ge. 0 ) then
                  nsigne = nsigne + 1
               endif
            endif
c           permutation circulaire des 3 sommets et aretes
            n  = n1
            n1 = n2
            n2 = n3
            n3 = n
 10      continue
         if( nsigne .ne. 3 ) nsigne = 0
      endif
      end

      integer function nosstr( p, pxyd, nt, letree )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    calculer le numero 0 a 3 du sous-triangle te contenant
c -----    le point p
c
c entrees:
c --------
c p      : point de r**2 contenu dans le te nt de letree
c pxyd   : x y distance des points
c nt     : numero letree du te de te voisin a calculer
c letree : arbre-4 des triangles equilateraux (te) fond de la triangulation
c      letree(0,0)  no du 1-er te vide dans letree
c      letree(0,1) : maximum du 1-er indice de letree (ici 8)
c      letree(0,2) : maximum declare du 2-eme indice de letree (ici mxtree)
c      letree(0:8,1) : racine de l'arbre  (triangle sans sur triangle)
c      si letree(0,.)>0 alors
c         letree(0:3,j) : no (>0) letree des 4 sous-triangles du triangle j
c      sinon
c         letree(0:3,j) :-no pxyd des 1 …a 4 points internes au triangle j
c                         0  si pas de point
c                       ( j est alors une feuille de l'arbre )
c      letree(4,j) : no letree du sur-triangle du triangle j
c      letree(5,j) : 0 1 2 3 no du sous-triangle j pour son sur-triangle
c      letree(6:8,j) : no pxyd des 3 sommets du triangle j
c
c sorties :
c ---------
c nosstr : 0 si le sous-triangle central contient p
c          i =1,2,3 numero du sous-triangle contenant p
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc    fevrier 1992
c2345x7..............................................................012
      integer           letree(0:8,0:*)
      double precision  pxyd(3,*), p(2),
     %                  x1, y1, x21, y21, x31, y31, d, xe, ye
c
c     le numero des 3 sommets du triangle
      ns1 = letree( 6, nt )
      ns2 = letree( 7, nt )
      ns3 = letree( 8, nt )
c
c     les coordonnees entre 0 et 1 du point p
      x1  = pxyd(1,ns1)
      y1  = pxyd(2,ns1)
c
      x21 = pxyd(1,ns2) - x1
      y21 = pxyd(2,ns2) - y1
c
      x31 = pxyd(1,ns3) - x1
      y31 = pxyd(2,ns3) - y1
c
      d   = 1.0 / ( x21 * y31 - x31 * y21 )
c
      xe  = ( ( p(1) - x1 ) * y31 - ( p(2) - y1 ) * x31 ) * d
      ye  = ( ( p(2) - y1 ) * x21 - ( p(1) - x1 ) * y21 ) * d
c
      if( xe .gt. 0.5d0 ) then
c        sous-triangle droit
         nosstr = 2
      else if( ye .gt. 0.5d0 ) then
c        sous-triangle haut
         nosstr = 3
      else if( xe+ye .lt. 0.5d0 ) then
c        sous-triangle gauche
         nosstr = 1
      else
c        sous-triangle central
         nosstr = 0
      endif
      end


      integer function notrpt( p, pxyd, notrde, letree )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    calculer le numero letree du sous-triangle feuille contenant
c -----    le point p a partir du te notrde de letree
c
c entrees:
c --------
c p      : point de r**2 contenu dans le te nt de letree
c pxyd   : x y distance des points
c notrde : numero letree du triangle depart de recherche (1=>racine)
c letree : arbre-4 des triangles equilateraux (te) fond de la triangulation
c      letree(0,0)  no du 1-er te vide dans letree
c      letree(0,1) : maximum du 1-er indice de letree (ici 8)
c      letree(0,2) : maximum declare du 2-eme indice de letree (ici mxtree)
c      letree(0:8,1) : racine de l'arbre  (triangle sans sur triangle)
c      si letree(0,.)>0 alors
c         letree(0:3,j) : no (>0) letree des 4 sous-triangles du triangle j
c      sinon
c         letree(0:3,j) :-no pxyd des 1 … 4 points internes au triangle j
c                         0  si pas de point
c                        ( j est alors une feuille de l'arbre )
c      letree(4,j) : no letree du sur-triangle du triangle j
c      letree(5,j) : 0 1 2 3 no du sous-triangle j pour son sur-triangle
c      letree(6:8,j) : no pxyd des 3 sommets du triangle j
c
c sorties :
c ---------
c notrpt : numero letree du triangle contenant le point p
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc    fevrier 1992
c2345x7..............................................................012
      integer           letree(0:8,0:*)
      double precision  pxyd(1:3,*), p(2)
c
c     la racine depart de la recherche
      notrpt = notrde
c
c     tant que la feuille n'est pas atteinte descendre l'arbre
 10   if( letree(0,notrpt) .gt. 0 ) then
c
c        recherche du sous-triangle contenant p
         nsot = nosstr( p, pxyd, notrpt, letree )
c
c        le numero letree du sous-triangle
         notrpt = letree( nsot, notrpt )
         goto 10
c
      endif
      end


      subroutine teajpt( ns,   nbsomm, mxsomm, pxyd, letree,
     &                   ntrp, ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    ajout du point ns de pxyd dans letree
c -----
c
c entrees:
c --------
c ns     : numero du point a ajouter dans letree
c mxsomm : nombre maximal de points declarables dans pxyd
c pxyd   : tableau des coordonnees des points
c          par point : x  y  distance_souhaitee
c
c modifies :
c ----------
c nbsomm : nombre actuel de points dans pxyd
c
c letree : arbre-4 des triangles equilateraux (te) fond de la triangulation
c      letree(0,0) : no du 1-er te vide dans letree
c      letree(0,1) : maximum du 1-er indice de letree (ici 8)
c      letree(0,2) : maximum declare du 2-eme indice de letree (ici mxtree)
c      letree(0:8,1) : racine de l'arbre  (triangle sans sur triangle)
c      si letree(0,.)>0 alors
c         letree(0:3,j) : no (>0) letree des 4 sous-triangles du triangle j
c      sinon
c         letree(0:3,j) :-no pxyd des 1 …a 4 points internes au triangle j
c                         0  si pas de point
c                        ( j est alors une feuille de l'arbre )
c      letree(4,j) : no letree du sur-triangle du triangle j
c      letree(5,j) : 0 1 2 3 no du sous-triangle j pour son sur-triangle
c      letree(6:8,j) : no pxyd des 3 sommets du triangle j
c
c sorties :
c ---------
c ntrp    : numero letree du triangle te ou a ete ajoute le point
c ierr    : 0 si pas d'erreur,  51 saturation letree, 52 saturation pxyd
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc    fevrier 1992
c2345x7..............................................................012
      integer           letree(0:8,0:*)
      double precision  pxyd(3,mxsomm)
c
c     depart de la racine
      ntrp = 1
c
c     recherche du triangle contenant le point pxyd(ns)
 1    ntrp = notrpt( pxyd(1,ns), pxyd, ntrp, letree )
c
c     existe t il un point libre
      do 10 i=0,3
         if( letree(i,ntrp) .eq. 0 ) then
c           la place i est libre
            letree(i,ntrp) = -ns
            return
         endif
 10   continue
c
c     pas de place libre => 4 sous-triangles sont crees
c                           a partir des 3 milieux des aretes
      call te4ste( nbsomm, mxsomm, pxyd, ntrp, letree, ierr )
      if( ierr .ne. 0 ) return
c
c     ajout du point ns
      goto 1
      end

      subroutine n1trva( nt, lar, letree, notrva, lhpile )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    calculer le numero letree du triangle voisin du te nt
c -----    par l'arete lar (1 a 3 ) de nt
c          attention : notrva n'est pas forcement minimal
c
c entrees:
c --------
c nt     : numero letree du te de te voisin a calculer
c lar    : numero 1 a 3 de l'arete du triangle nt
c letree : arbre-4 des triangles equilateraux (te) fond de la triangulation
c   letree(0,0)  no du 1-er te vide dans letree
c   letree(0,1) : maximum du 1-er indice de letree (ici 8)
c   letree(0,2) : maximum declare du 2-eme indice de letree (ici mxtree)
c   letree(0:8,1) : racine de l'arbre  (triangle sans sur-triangle)
c   si letree(0,.)>0 alors
c      letree(0:3,j) : no (>0) letree des 4 sous-triangles du triangle j
c   sinon
c      letree(0:3,j) :-no pxyd des 1 a 4 points internes au triangle j
c                      0  si pas de point
c                     ( j est alors une feuille de l'arbre )
c   letree(4,j) : no letree du sur-triangle du triangle j
c   letree(5,j) : 0 1 2 3 no du sous-triangle j pour son sur-triangle
c   letree(6:8,j) : no pxyd des 3 sommets du triangle j
c
c sorties :
c ---------
c notrva  : >0 numero letree du te voisin par l'arete lar
c           =0 si pas de te voisin (racine , ... )
c lhpile  : =0 si nt et notrva ont meme taille
c           >0 nt est 4**lhpile fois plus petit que notrva
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc    fevrier 1992
c2345x7..............................................................012
      integer   letree(0:8,0:*)
      integer   lapile(1:64)
c
c     initialisation de la pile
c     le triangle est empile
      lapile(1) = nt
      lhpile = 1
c
c     tant qu'il existe un sur-triangle
 10   ntr  = lapile( lhpile )
      if( ntr .eq. 1 ) then
c        racine atteinte => pas de triangle voisin
         notrva = 0
         lhpile = lhpile - 1
         return
      endif
c
c     le type du triangle ntr
      nty  = letree( 5, ntr )
c     l'eventuel sur-triangle
      nsut = letree( 4, ntr )
c
      if( nty .eq. 0 ) then
c
c        triangle de type 0 => triangle voisin de type precedent(lar)
c                              dans le sur-triangle de ntr
c                              ce triangle remplace ntr dans lapile
         lapile( lhpile ) = letree( nopre3(lar), nsut )
         goto 20
      endif
c
c     triangle ntr de type nty>0
      if( nosui3(nty) .eq. lar ) then
c
c        le triangle voisin par lar est le triangle 0
         lapile( lhpile ) = letree( 0, nsut )
         goto 20
      endif
c
c     triangle sans voisin direct => passage par le sur-triangle
      if( nsut .eq. 0 ) then
c
c        ntr est la racine => pas de triangle voisin par cette arete
         notrva = 0
         return
      else
c
c        le sur-triangle est empile
         lhpile = lhpile + 1
         lapile(lhpile) = nsut
         goto 10
      endif
c
c     descente aux sous-triangles selon la meme arete
 20   notrva = lapile( lhpile )
c
 30   lhpile = lhpile - 1
      if( letree(0,notrva) .le. 0 ) then
c        le triangle est une feuille de l'arbre 0 sous-triangle
c        lhpile = nombre de differences de niveaux dans l'arbre
         return
      else
c        le triangle a 4 sous-triangles
         if( lhpile .gt. 0 ) then
c
c           bas de pile non atteint
            nty  = letree( 5, lapile(lhpile) )
            if( nty .eq. lar ) then
c              l'oppose est suivant(nty) de notrva
               notrva = letree( nosui3(nty) , notrva )
            else
c              l'oppose est precedent(nty) de notrva
               notrva = letree( nopre3(nty) , notrva )
            endif
            goto 30
         endif
      endif
c
c     meme niveau dans l'arbre lhpile = 0
      end


      subroutine cenced( xy1, xy2, xy3, cetria, ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but : calcul des coordonnees du centre du cercle circonscrit
c ----- du triangle defini par ses 3 sommets de coordonnees
c       xy1 xy2 xy3 ainsi que le carre du rayon de ce cercle
c
c entrees :
c ---------
c xy1 xy2 xy3 : les 2 coordonnees des 3 sommets du triangle
c ierr   : <0  => pas d'affichage si triangle degenere
c          >=0 =>       affichage si triangle degenere
c
c sortie :
c --------
c cetria : cetria(1)=abcisse  du centre
c          cetria(2)=ordonnee du centre
c          cetria(3)=carre du rayon   1d28 si triangle degenere
c ierr   : 0 si triangle non degenere
c          1 si triangle degenere
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : perronnet alain upmc analyse numerique paris        juin 1995
c2345x7..............................................................012
      parameter        (epsurf=1d-7)
      common / unites / lecteu,imprim,nunite(30)
      double precision  x1,y1,x21,y21,x31,y31,
     %                  aire2,xc,yc,rot,
     %                  xy1(2),xy2(2),xy3(2),cetria(3)
c
c     le calcul de 2 fois l'aire du triangle
c     attention l'ordre des 3 sommets est direct ou non
      x1  = xy1(1)
      x21 = xy2(1) - x1
      x31 = xy3(1) - x1
c
      y1  = xy1(2)
      y21 = xy2(2) - y1
      y31 = xy3(2) - y1
c
      aire2  = x21 * y31 - x31 * y21
c
c     recherche d'un test relatif peu couteux
c     pour reperer la degenerescence du triangle
      if( abs(aire2) .le.
     %    epsurf*(abs(x21)+abs(x31))*(abs(y21)+abs(y31)) ) then
c        triangle de qualite trop faible
         if( ierr .ge. 0 ) then
c            nblgrc(nrerr) = 1
c            kerr(1) = 'erreur cenced: triangle degenere'
c            call lereur
            write(imprim,*) 'erreur cenced: triangle degenere'
            write(imprim,10000)  xy1,xy2,xy3,aire2
         endif
10000 format( 3(' x=',g24.16,' y=',g24.16/),' aire*2=',g24.16)
         cetria(1) = 0d0
         cetria(2) = 0d0
         cetria(3) = 1d28
         ierr = 1
         return
      endif
c
c     les 2 coordonnees du centre intersection des 2 mediatrices
c     x = (x1+x2)/2 + lambda * (y2-y1)
c     y = (y1+y2)/2 - lambda * (x2-x1)
c     x = (x1+x3)/2 + rot    * (y3-y1)
c     y = (y1+y3)/2 - rot    * (x3-x1)
c     ==========================================================
      rot = ((xy2(1)-xy3(1))*x21 + (xy2(2)-xy3(2))*y21) / (2 * aire2)
c
      xc = ( x1 + xy3(1) ) * 0.5d0 + rot * y31
      yc = ( y1 + xy3(2) ) * 0.5d0 - rot * x31
c
      cetria(1) = xc
      cetria(2) = yc
c
c     le carre du rayon
      cetria(3) = (x1-xc) ** 2 + (y1-yc) ** 2
c
c     pas d'erreur rencontree
      ierr = 0
      end


      double precision function angled( p1, p2, p3 )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :   calculer l'angle (p1p2,p1p3) en radians
c -----
c
c entrees :
c ---------
c p1,p2,p3 : les 2 coordonnees des 3 sommets de l'angle
c               sens direct pour une surface >0
c sorties :
c ---------
c angled :  angle (p1p2,p1p3) en radians entre [0 et 2pi]
c           0 si p1=p2 ou p1=p3
c ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet analyse numerique upmc paris     fevrier 1992
c2345x7..............................................................012
      double precision  p1(2),p2(2),p3(2),x21,y21,x31,y31,a1,a2,d,c
c
c     les cotes
      x21 = p2(1) - p1(1)
      y21 = p2(2) - p1(2)
      x31 = p3(1) - p1(1)
      y31 = p3(2) - p1(2)
c
c     longueur des cotes
      a1 = x21 * x21 + y21 * y21
      a2 = x31 * x31 + y31 * y31
      d  = sqrt( a1 * a2 )
      if( d .eq. 0 ) then
         angled = 0
         return
      endif
c
c     cosinus de l'angle
      c  = ( x21 * x31 + y21 * y31 ) / d
      if( c .le. -1.d0 ) then
c        tilt sur apollo si acos( -1 -eps )
         angled = atan( 1.d0 ) * 4.d0
         return
      else if( c .ge. 1.d0 ) then
c        tilt sur apollo si acos( 1 + eps )
         angled = 0
         return
      endif
c
      angled = acos( c )
      if( x21 * y31 - x31 * y21 .lt. 0 ) then
c        demi plan inferieur
         angled = 8.d0 * atan( 1.d0 ) - angled
      endif
      end


      subroutine teajte( mxsomm, nbsomm, pxyd,   comxmi,
     %                   aretmx, mxtree, letree,
     %                   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    initialisation des tableaux letree
c -----    ajout des sommets 1 a nbsomm (valeur en entree) dans letree
c
c entrees:
c --------
c mxsomm : nombre maximal de sommets permis pour la triangulation
c mxtree : nombre maximal de triangles equilateraux (te) declarables
c aretmx : longueur maximale des aretes des triangles equilateraux
c
c entrees et sorties :
c --------------------
c nbsomm : nombre de sommets apres identification
c pxyd   : tableau des coordonnees 2d des points
c          par point : x  y  distance_souhaitee
c          tableau reel(3,mxsomm)
c
c sorties:
c --------
c comxmi : coordonnees minimales et maximales des points frontaliers
c letree : arbre-4 des triangles equilateraux (te) fond de la triangulation
c          letree(0,0) : no du 1-er te vide dans letree
c          letree(0,1) : maximum du 1-er indice de letree (ici 8)
c          letree(0,2) : maximum declare du 2-eme indice de letree (ici mxtree)
c          letree(0:8,1) : racine de l'arbre  (triangle sans sur triangle)
c          si letree(0,.)>0 alors
c             letree(0:3,j) : no (>0) letree des 4 sous-triangles du triangle j
c          sinon
c             letree(0:3,j) :-no pxyd des 1 a 4 points internes au triangle j
c                             0  si pas de point
c                             ( j est alors une feuille de l'arbre )
c          letree(4,j) : no letree du sur-triangle du triangle j
c          letree(5,j) : 0 1 2 3 no du sous-triangle j pour son sur-triangle
c          letree(6:8,j) : no pxyd des 3 sommets du triangle j
c
c ierr   :  0 si pas d'erreur
c          51 saturation letree
c          52 saturation pxyd
c           7 tous les points sont alignes
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc    juillet 1994
c....................................................................012
      integer           letree(0:8,0:mxtree)
      double precision  pxyd(3,mxsomm)
      double precision  comxmi(3,2)
      double precision  a(2),s,aretmx,rac3
c
c     protection du nombre de sommets avant d'ajouter ceux de tetree
      nbsofr = nbsomm
      do 1 i = 1, nbsomm 
         comxmi(1,1) = min( comxmi(1,1), pxyd(1,i) )
         comxmi(1,2) = max( comxmi(1,2), pxyd(1,i) )
         comxmi(2,1) = min( comxmi(2,1), pxyd(2,i) )
         comxmi(2,2) = max( comxmi(2,2), pxyd(2,i) )
 1    continue
c
c     creation de l'arbre tee
c     =======================
c     la premiere colonne vide de letree
      letree(0,0) = 2
c     chainage des te vides
      do 4 i = 2 , mxtree
         letree(0,i) = i+1
 4    continue
      letree(0,mxtree) = 0
c     les maxima des 2 indices de letree
      letree(1,0) = 8
      letree(2,0) = mxtree
c
c     la racine
c     aucun point interne au triangle equilateral (te) 1
      letree(0,1) = 0
      letree(1,1) = 0
      letree(2,1) = 0
      letree(3,1) = 0
c     pas de sur-triangle
      letree(4,1) = 0
      letree(5,1) = 0
c     le numero pxyd des 3 sommets du te 1
      letree(6,1) = nbsomm + 1
      letree(7,1) = nbsomm + 2
      letree(8,1) = nbsomm + 3
c
c     calcul de la largeur et hauteur du rectangle englobant
c     ======================================================
      a(1) = comxmi(1,2) - comxmi(1,1)
      a(2) = comxmi(2,2) - comxmi(2,1)
c     la longueur de la diagonale
      s = sqrt( a(1)**2 + a(2)**2 )
      do 60 k=1,2
         if( a(k) .lt. 1e-4 * s ) then
c            nblgrc(nrerr) = 1
            write(imprim,*) 'tous les points sont alignes'
c            call lereur
            ierr = 7
            return
         endif
 60   continue
c
c     le maximum des ecarts
      s = s + s
c
c     le triangle equilateral englobant
c     =================================
c     ecart du rectangle au triangle equilateral
      rac3 = sqrt( 3.0d0 )
      arete = a(1) + 2 * aretmx + 2 * ( a(2) + aretmx ) / rac3
c
c     le point nbsomm + 1 en bas a gauche
      nbsomm = nbsomm + 1
      pxyd(1,nbsomm) = (comxmi(1,1)+comxmi(1,2))*0.5d0 - arete*0.5d0
      pxyd(2,nbsomm) =  comxmi(2,1) - aretmx
      pxyd(3,nbsomm) = s
c
c     le point nbsomm + 2 en bas a droite
      nbsomm = nbsomm + 1
      pxyd(1,nbsomm) = pxyd(1,nbsomm-1) + arete
      pxyd(2,nbsomm) = pxyd(2,nbsomm-1)
      pxyd(3,nbsomm) = s
c
c     le point nbsomm + 3 sommet au dessus
      nbsomm = nbsomm + 1
      pxyd(1,nbsomm) = pxyd(1,nbsomm-2) + arete * 0.5d0
      pxyd(2,nbsomm) = pxyd(2,nbsomm-2) + arete * 0.5d0 * rac3
      pxyd(3,nbsomm) = s
c
c     ajout des sommets des lignes pour former letree
c     ===============================================
      do 150 i=1,nbsofr
c        ajout du point i de pxyd a letree
         call teajpt(  i, nbsomm, mxsomm, pxyd, letree,
     &                nt, ierr )
         if( ierr .ne. 0 ) return
 150  continue
c
      return
      end


      subroutine tetaid( nutysu, dx, dy, longai, ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :     calculer la longueur de l'arete ideale en dx,dy
c -----
c
c entrees:
c --------
c nutysu : numero de traitement de areteideale() selon le type de surface
c          0 pas d'emploi de la fonction areteideale() => aretmx active
c          1 il existe une fonction areteideale(xyz,xyzdir)
c          ... autres options a definir ...
c dx, dy : abscisse et ordonnee dans le plan du point (reel2!)
c
c sorties:
c --------
c longai : longueur de l'areteideale(xyz,xyzdir) autour du point xyz
c ierr   : 0 si pas d'erreur, <>0 sinon
c          1 calcul incorrect de areteideale(xyz,xyzdir)
c          2 longueur calculee nulle
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c2345x7..............................................................012
      common / unites / lecteu, imprim, nunite(30)
c
      double precision  areteideale
      double precision  dx, dy, longai
      double precision  xyz(3), xyzd(3), d0
c
      ierr = 0
      if( nutysu .gt. 0 ) then
         d0 = longai
c        le point ou se calcule la longueur
         xyz(1) = dx
         xyz(2) = dy
c        z pour le calcul de la longueur (inactif ici!)
         xyz(3) = 0d0
c        la direction pour le calcul de la longueur (inactif ici!)
         xyzd(1) = 0d0
         xyzd(2) = 0d0
         xyzd(3) = 0d0

         longai = areteideale(xyz,xyzd)
         if( longai .lt. 0d0 ) then
            write(imprim,10000) xyz
10000       format('attention: longueur de areteideale(',
     %              g14.6,',',g14.6,',',g14.6,')<=0! => rendue >0' )
            longai = -longai
         endif
         if( longai .eq. 0d0 ) then
            write(imprim,10001) xyz
10001       format('erreur: longueur de areteideale(',
     %              g14.6,',',g14.6,',',g14.6,')=0!' )
            ierr = 2
            longai = d0
         endif
      endif
      end


      subroutine tehote( nutysu,
     %                   nbarpi, mxsomm, nbsomm, pxyd,
     %                   comxmi, aretmx,
     %                   letree, mxqueu, laqueu,
     %                   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :     homogeneisation de l'arbre des te a un saut de taille au plus
c -----     prise en compte des distances souhaitees autour des sommets initiaux
c
c entrees:
c --------
c nutysu : numero de traitement de areteideale() selon le type de surface
c          0 pas d'emploi de la fonction areteideale() => aretmx active
c          1 il existe une fonction areteideale()
c            dont seules les 2 premieres composantes de uv sont actives
c          autres options a definir...
c nbarpi : nombre de sommets de la frontiere + nombre de points internes
c          imposes par l'utilisateur
c mxsomm : nombre maximal de sommets permis pour la triangulation  et te
c mxqueu : nombre d'entiers utilisables dans laqueu
c comxmi : minimum et maximum des coordonnees de l'objet
c aretmx : longueur maximale des aretes des triangles equilateraux
c permtr : perimetre de la ligne enveloppe dans le plan
c          avant mise a l'echelle a 2**20
c
c modifies :
c ----------
c nbsomm : nombre de sommets apres identification
c pxyd   : tableau des coordonnees 2d des points
c          par point : x  y  distance_souhaitee
c letree : arbre-4 des triangles equilateraux (te) fond de la triangulation
c          letree(0,0) : no du 1-er te vide dans letree
c          letree(1,0) : maximum du 1-er indice de letree (ici 8)
c          letree(2,0) : maximum declare du 2-eme indice de letree (ici mxtree)
c          letree(0:8,1) : racine de l'arbre  (triangle sans sur triangle)
c          si letree(0,.)>0 alors
c             letree(0:3,j) : no (>0) letree des 4 sous-triangles du triangle j
c          sinon
c             letree(0:3,j) :-no pxyd des 1 a 4 points internes au triangle j
c                             0  si pas de point
c                             ( j est alors une feuille de l'arbre )
c          letree(4,j) : no letree du sur-triangle du triangle j
c          letree(5,j) : 0 1 2 3 no du sous-triangle j pour son sur-triangle
c          letree(6:8,j) : no pxyd des 3 sommets du triangle j
c
c auxiliaire :
c ------------
c laqueu : mxqueu entiers servant de queue pour le parcours de letree
c
c sorties:
c --------
c ierr   :  0 si pas d'erreur
c          51 si saturation letree dans te4ste
c          52 si saturation pxyd   dans te4ste
c          >0 si autre erreur
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc      avril 1997
c2345x7..............................................................012
      double precision  ampli
      parameter        (ampli=1.34d0)
      common / unites / lecteu, imprim, intera, nunite(29)
c
      double precision  pxyd(3,mxsomm), d2, aretm2
      double precision  comxmi(3,2),aretmx,a,s,xrmin,xrmax,yrmin,yrmax
      double precision  dmin, dmax
      integer           letree(0:8,0:*)
c
      integer           laqueu(1:mxqueu),lequeu
c                       lequeu : entree dans la queue
c                       lhqueu : longueur de la queue
c                       gestion circulaire
c
      integer           nuste(3)
      equivalence      (nuste(1),ns1),(nuste(2),ns2),(nuste(3),ns3)
c
c     existence ou non de la fonction 'taille_ideale' des aretes
c     autour du point.  ici la carte est supposee isotrope
c     ==========================================================
c     attention: si la fonction taille_ideale existe
c                alors pxyd(3,*) est la taille_ideale dans l'espace initial
c                sinon pxyd(3,*) est la distance calculee dans le plan par
c                propagation a partir des tailles des aretes de la frontiere
c
      if( nutysu .gt. 0 ) then
c
c        la fonction taille_ideale(x,y,z) existe
c        ---------------------------------------
c        initialisation de la distance souhaitee autour des points 1 a nbsomm
         do 1 i=1,nbsomm
c           calcul de pxyzd(3,i)
            call tetaid( nutysu, pxyd(1,i), pxyd(2,i),
     %                   pxyd(3,i), ierr )
            if( ierr .ne. 0 ) goto 9999
 1       continue
c
      else
c
c        la fonction taille_ideale(x,y,z) n'existe pas
c        ---------------------------------------------
c        prise en compte des distances souhaitees dans le plan
c        autour des points frontaliers et des points internes imposes
c        toutes les autres distances souhaitees ont ete mis a aretmx
c        lors de l'execution du sp teqini
         do 3 i=1,nbarpi
c           le sommet i n'est pas un sommet de letree => sommet frontalier
c           recherche du sous-triangle minimal feuille contenant le point i
            nte = 1
 2          nte = notrpt( pxyd(1,i), pxyd, nte, letree )
c           la distance au sommet le plus eloigne est elle inferieure
c           a la distance souhaitee?
            ns1 = letree(6,nte)
            ns2 = letree(7,nte)
            ns3 = letree(8,nte)
            d2  = max( ( pxyd(1,i)-pxyd(1,ns1) )**2 +
     %                 ( pxyd(2,i)-pxyd(2,ns1) )**2
     %               , ( pxyd(1,i)-pxyd(1,ns2) )**2 +
     %                 ( pxyd(2,i)-pxyd(2,ns2) )**2
     %               , ( pxyd(1,i)-pxyd(1,ns3) )**2 +
     %                 ( pxyd(2,i)-pxyd(2,ns3) )**2 )
            if( d2 .gt. pxyd(3,i)**2 ) then
c              le triangle nte trop grand doit etre subdivise en 4 sous-triangle
               call te4ste( nbsomm, mxsomm, pxyd, nte, letree,
     &                      ierr )
               if( ierr .ne. 0 ) return
               goto 2
            endif
 3       continue
      endif
c
c     le sous-triangle central de la racine est decoupe systematiquement
c     ==================================================================
      nte = 2
      if( letree(0,2) .le. 0 ) then
c        le sous-triangle central de la racine n'est pas subdivise
c        il est donc decoupe en 4 soustriangles
         nbsom0 = nbsomm
         call te4ste( nbsomm, mxsomm, pxyd, nte, letree,
     %                ierr )
         if( ierr .ne. 0 ) return
         do 4 i=nbsom0+1,nbsomm
c           mise a jour de taille_ideale des nouveaux sommets de te
            call tetaid( nutysu, pxyd(1,i), pxyd(2,i), pxyd(3,i), ierr )
            if( ierr .ne. 0 ) goto 9999
 4       continue
      endif
c
c     le carre de la longueur de l'arete de triangles equilateraux
c     souhaitee pour le fond de la triangulation
      aretm2 = (aretmx*ampli) ** 2
c
c     tout te contenu dans le rectangle englobant doit avoir un
c     cote < aretmx et etre de meme taille que les te voisins
c     s'il contient un point; sinon un seul saut de taille est permis
c     ===============================================================
c     le rectangle englobant pour selectionner les te "internes"
c     le numero des 3 sommets du te englobant racine de l'arbre des te
      ns1 = letree(6,1)
      ns2 = letree(7,1)
      ns3 = letree(8,1)
      a   = aretmx * 0.01d0
c     abscisse du milieu de l'arete gauche du te 1
      s      = ( pxyd(1,ns1) + pxyd(1,ns3) ) / 2
      xrmin  = min( s, comxmi(1,1) - aretmx ) - a
c     abscisse du milieu de l'arete droite du te 1
      s      = ( pxyd(1,ns2) + pxyd(1,ns3) ) / 2
      xrmax  = max( s, comxmi(1,2) + aretmx ) + a
      yrmin  = comxmi(2,1) - aretmx
c     ordonnee de la droite passant par les milieus des 2 aretes
c     droite gauche du te 1
      s      = ( pxyd(2,ns1) + pxyd(2,ns3) ) / 2
      yrmax  = max( s, comxmi(2,2) + aretmx ) + a
c
c     cas particulier de 3 ou 4 ou peu d'aretes frontalieres
      if( nbarpi .le. 8 ) then
c        tout le triangle englobant (racine) est a prendre en compte
         xrmin = pxyd(1,ns1) - a
         xrmax = pxyd(1,ns2) + a
         yrmin = pxyd(2,ns1) - a
         yrmax = pxyd(2,ns3) + a
      endif
c
      nbs0   = nbsomm
      nbiter = -1
c
c     initialisation de la queue
  5   nbiter = nbiter + 1
      lequeu = 1
      lhqueu = 0
c     la racine de letree initialise la queue
      laqueu(1) = 1
c
c     tant que la longueur de la queue est >=0 traiter le debut de queue
 10   if( lhqueu .ge. 0 ) then
c
c        le triangle te a traiter
         i   = lequeu - lhqueu
         if( i .le. 0 ) i = mxqueu + i
         nte = laqueu( i )
c        la longueur de la queue est reduite
         lhqueu = lhqueu - 1
c
c        nte est il un sous-triangle feuille minimal ?
 15      if( letree(0,nte) .gt. 0 ) then
c
c           non les 4 sous-triangles sont mis dans la queue
            if( lhqueu + 4 .ge. mxqueu ) then
               write(imprim,*) 'tehote: saturation de la queue'
               ierr = 7
               return
            endif
            do 20 i=3,0,-1
c              ajout du sous-triangle i
               lhqueu = lhqueu + 1
               lequeu = lequeu + 1
               if( lequeu .gt. mxqueu ) lequeu = lequeu - mxqueu
               laqueu( lequeu ) = letree( i, nte )
 20         continue
            goto 10
c
         endif
c
c        ici nte est un triangle minimal non subdivise
c        ---------------------------------------------
c        le te est il dans le cadre englobant de l'objet ?
         ns1 = letree(6,nte)
         ns2 = letree(7,nte)
         ns3 = letree(8,nte)
         if( pxyd(1,ns1) .gt. pxyd(1,ns2) ) then
            dmin = pxyd(1,ns2)
            dmax = pxyd(1,ns1)
         else
            dmin = pxyd(1,ns1)
            dmax = pxyd(1,ns2)
         endif
         if( (xrmin .le. dmin .and. dmin .le. xrmax) .or.
     %       (xrmin .le. dmax .and. dmax .le. xrmax) ) then
            if( pxyd(2,ns1) .gt. pxyd(2,ns3) ) then
               dmin = pxyd(2,ns3)
               dmax = pxyd(2,ns1)
            else
               dmin = pxyd(2,ns1)
               dmax = pxyd(2,ns3)
            endif
            if( (yrmin .le. dmin .and. dmin .le. yrmax) .or.
     %          (yrmin .le. dmax .and. dmax .le. yrmax) ) then
c
c              nte est un te feuille et interne au rectangle englobant
c              =======================================================
c              le carre de la longueur de l'arete du te de numero nte
               d2 = (pxyd(1,ns1)-pxyd(1,ns2)) ** 2 +
     %              (pxyd(2,ns1)-pxyd(2,ns2)) ** 2
c
               if( nutysu .eq. 0 ) then
c
c                 il n'existe pas de fonction 'taille_ideale'
c                 -------------------------------------------
c                 si la taille effective de l'arete du te est superieure a aretmx
c                 alors le te est decoupe
                  if( d2 .gt. aretm2 ) then
c                    le triangle nte trop grand doit etre subdivise
c                    en 4 sous-triangles
                     call te4ste( nbsomm,mxsomm, pxyd,
     %                            nte, letree, ierr )
                     if( ierr .ne. 0 ) return
                     goto 15
                  endif
c
               else
c
c                 il existe ici une fonction 'taille_ideale'
c                 ------------------------------------------
c                 si la taille effective de l'arete du te est superieure au mini
c                 des 3 tailles_ideales aux sommets  alors le te est decoupe
                  do 28 i=1,3
                     if( d2 .gt. (pxyd(3,nuste(i))*ampli)**2 ) then
c                       le triangle nte trop grand doit etre subdivise
c                       en 4 sous-triangles
                        nbsom0 = nbsomm
                        call te4ste( nbsomm, mxsomm, pxyd,
     &                               nte, letree, ierr )
                        if( ierr .ne. 0 ) return
                        do 27 j=nbsom0+1,nbsomm
c                          mise a jour de taille_ideale des nouveaux sommets de
                           call tetaid( nutysu, pxyd(1,j), pxyd(2,j),
     %                                  pxyd(3,j), ierr )
                           if( ierr .ne. 0 ) goto 9999
 27                     continue
                        goto 15
                     endif
 28               continue
               endif
c
c              recherche du nombre de niveaux entre nte et les te voisins par se
c              si la difference de subdivisions excede 1 alors le plus grand des
c              =================================================================
 29            do 30 i=1,3
c
c                 noteva triangle voisin de nte par l'arete i
                  call n1trva( nte, i, letree, noteva, niveau )
                  if( noteva .le. 0 ) goto 30
c                 il existe un te voisin
                  if( niveau .gt. 0 ) goto 30
c                 nte a un te voisin plus petit ou egal
                  if( letree(0,noteva) .le. 0 ) goto 30
c                 nte a un te voisin noteva subdivise au moins une fois
c
                  if( nbiter .gt. 0 ) then
c                    les 2 sous triangles voisins sont-ils subdivises?
                     ns2 = letree(i,noteva)
                     if( letree(0,ns2) .le. 0 ) then
c                       ns2 n'est pas subdivise
                        ns2 = letree(nosui3(i),noteva)
                        if( letree(0,ns2) .le. 0 ) then
c                          les 2 sous-triangles ne sont pas subdivises
                           goto 30
                        endif
                     endif
                  endif
c
c                 saut>1 => le triangle nte doit etre subdivise en 4 sous-triang
c                 --------------------------------------------------------------
                  nbsom0 = nbsomm
                  call te4ste( nbsomm,mxsomm, pxyd, nte, letree,
     &                         ierr )
                  if( ierr .ne. 0 ) return
                  if( nutysu .gt. 0 ) then
                     do 32 j=nbsom0+1,nbsomm
c                       mise a jour de taille_ideale des nouveaux sommets de te
                        call tetaid( nutysu, pxyd(1,j), pxyd(2,j),
     %                               pxyd(3,j), ierr )
                        if( ierr .ne. 0 ) goto 9999
 32                  continue
                  endif
                  goto 15
c
 30            continue
            endif
         endif
         goto 10
      endif
      if( nbs0 .lt. nbsomm ) then
         nbs0 = nbsomm
         goto 5
      endif
      return
c
c     pb dans le calcul de la fonction taille_ideale

 9999 write(imprim,*) 'pb dans le calcul de taille_ideale'
c      nblgrc(nrerr) = 1
c      kerr(1) = 'pb dans le calcul de taille_ideale'
c      call lereur
      return
      end


      subroutine tetrte( comxmi, aretmx, nbarpi, mxsomm, pxyd,
     %                   mxqueu, laqueu, letree,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, mxartr, n1artr, noartr, noarst,
     %                   ierr  )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    trianguler les triangles equilateraux feuilles et
c -----    les points de la frontiere et les points internes imposes
c
c attention: la triangulation finale n'est pas de type delaunay!
c
c entrees:
c --------
c comxmi : minimum et maximum des coordonnees de l'objet
c aretmx : longueur maximale des aretes des triangles equilateraux
c nbarpi : nombre de sommets de la frontiere + nombre de points internes
c          imposes par l'utilisateur
c mxsomm : nombre maximal de sommets declarables dans pxyd
c pxyd   : tableau des coordonnees 2d des points
c          par point : x  y  distance_souhaitee
c
c mxqueu : nombre d'entiers utilisables dans laqueu
c mosoar : nombre maximal d'entiers par arete du tableau nosoar
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxartr : nombre maximal de triangles stockables dans le tableau noartr
c letree : arbre-4 des triangles equilateraux (te) fond de la triangulation
c          letree(0,0) : no du 1-er te vide dans letree
c          letree(0,1) : maximum du 1-er indice de letree (ici 8)
c          letree(0,2) : maximum declare du 2-eme indice de letree (ici mxtree)
c          letree(0:8,1) : racine de l'arbre  (triangle sans sur triangle)
c          si letree(0,.)>0 alors
c             letree(0:3,j) : no (>0) letree des 4 sous-triangles du triangle j
c          sinon
c             letree(0:3,j) :-no pxyd des 1 a 4 points internes au triangle j
c                             0  si pas de point
c                             ( j est alors une feuille de l'arbre )
c          letree(4,j) : no letree du sur-triangle du triangle j
c          letree(5,j) : 0 1 2 3 no du sous-triangle j pour son sur-triangle
c          letree(6:8,j) : no pxyd des 3 sommets du triangle j
c
c modifies:
c ---------
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c          une arete i de nosoar est vide  <=>  nosoar(1,i)=0
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = nosoar(1)+nosoar(2)*2
c noarst : noarst(i) numero d'une arete de sommet i
c
c auxiliaire :
c ------------
c laqueu : mxqueu entiers servant de queue pour le parcours de letree
c
c sorties:
c --------
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c ierr   : =0 si pas d'erreur
c          =1 si le tableau nosoar est sature
c          =2 si le tableau noartr est sature
c          =3 si aucun des triangles ne contient l'un des points internes d'un t
c          =5 si saturation de la queue de parcours de l'arbre des te
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c2345x7..............................................................012
      common / unites / lecteu, imprim, intera, nunite(29)
c
      double precision  pxyd(3,mxsomm)
      double precision  comxmi(3,2),aretmx,a,s,xrmin,xrmax,yrmin,yrmax
      double precision  dmin, dmax
c
      integer           nosoar(mosoar,mxsoar),
     %                  noartr(moartr,mxartr),
     %                  noarst(mxsomm)
c
      integer           letree(0:8,0:*)
      integer           laqueu(1:mxqueu)
c                       lequeu:entree dans la queue en gestion circulaire
c                       lhqueu:longueur de la queue en gestion circulaire
c
      integer           milieu(3), nutr(1:13)
c
c     le rectangle englobant pour selectionner les te "internes"
c     le numero des 3 sommets du te englobant racine de l'arbre des te
      ns1 = letree(6,1)
      ns2 = letree(7,1)
      ns3 = letree(8,1)
      a   = aretmx * 0.01d0
c     abscisse du milieu de l'arete gauche du te 1
      s      = ( pxyd(1,ns1) + pxyd(1,ns3) ) / 2
      xrmin  = min( s, comxmi(1,1) - aretmx ) - a
c     abscisse du milieu de l'arete droite du te 1
      s      = ( pxyd(1,ns2) + pxyd(1,ns3) ) / 2
      xrmax  = max( s, comxmi(1,2) + aretmx ) + a
      yrmin  = comxmi(2,1) - aretmx
c     ordonnee de la droite passant par les milieus des 2 aretes
c     droite gauche du te 1
      s      = ( pxyd(2,ns1) + pxyd(2,ns3) ) / 2
      yrmax  = max( s, comxmi(2,2) + aretmx ) + a
c
c     cas particulier de 3 ou 4 ou peu d'aretes frontalieres
      if( nbarpi .le. 8 ) then
c        tout le triangle englobant (racine) est a prendre en compte
         xrmin = pxyd(1,ns1) - a
         xrmax = pxyd(1,ns2) + a
         yrmin = pxyd(2,ns1) - a
         yrmax = pxyd(2,ns3) + a
      endif
c
c     initialisation du tableau noartr
      do 5 i=1,mxartr
c        le numero de l'arete est inconnu
         noartr(1,i) = 0
c        le chainage sur le triangle vide suivant
         noartr(2,i) = i+1
 5    continue
      noartr(2,mxartr) = 0
      n1artr = 1
c
c     parcours des te jusqu'a trianguler toutes les feuilles (triangles eq)
c     =====================================================================
c     initialisation de la queue sur les te
      ierr   = 0
      lequeu = 1
      lhqueu = 0
c     la racine de letree initialise la queue
      laqueu(1) = 1
c
c     tant que la longueur de la queue est >=0 traiter le debut de queue
 10   if( lhqueu .ge. 0 ) then
c
c        le triangle te a traiter
         i   = lequeu - lhqueu
         if( i .le. 0 ) i = mxqueu + i
         nte = laqueu( i )
c        la longueur est reduite
         lhqueu = lhqueu - 1
c
c        nte est il un sous-triangle feuille (minimal) ?
 15      if( letree(0,nte) .gt. 0 ) then
c           non les 4 sous-triangles sont mis dans la queue
            if( lhqueu + 4 .ge. mxqueu ) then
               write(imprim,*) 'tetrte: saturation de la queue'
               ierr = 5
               return
            endif
            do 20 i=3,0,-1
c              ajout du sous-triangle i
               lhqueu = lhqueu + 1
               lequeu = lequeu + 1
               if( lequeu .gt. mxqueu ) lequeu = lequeu - mxqueu
               laqueu( lequeu ) = letree( i, nte )
 20         continue
            goto 10
         endif
c
c        ici nte est un triangle minimal non subdivise
c        ---------------------------------------------
c        le te est il dans le cadre englobant de l'objet ?
         ns1 = letree(6,nte)
         ns2 = letree(7,nte)
         ns3 = letree(8,nte)
         if( pxyd(1,ns1) .gt. pxyd(1,ns2) ) then
            dmin = pxyd(1,ns2)
            dmax = pxyd(1,ns1)
         else
            dmin = pxyd(1,ns1)
            dmax = pxyd(1,ns2)
         endif
         if( (xrmin .le. dmin .and. dmin .le. xrmax) .or.
     %       (xrmin .le. dmax .and. dmax .le. xrmax) ) then
            if( pxyd(2,ns1) .gt. pxyd(2,ns3) ) then
               dmin = pxyd(2,ns3)
               dmax = pxyd(2,ns1)
            else
               dmin = pxyd(2,ns1)
               dmax = pxyd(2,ns3)
            endif
            if( (yrmin .le. dmin .and. dmin .le. yrmax) .or.
     %          (yrmin .le. dmax .and. dmax .le. yrmax) ) then
c
c              te minimal et interne au rectangle englobant
c              --------------------------------------------
c              recherche du nombre de niveaux entre nte et les te voisins
c              par ses aretes
               nbmili = 0
               do 30 i=1,3
c
c                 a priori pas de milieu de l'arete i du te nte
                  milieu(i) = 0
c
c                 recherche de noteva te voisin de nte par l'arete i
                  call n1trva( nte, i, letree, noteva, niveau )
c                 noteva  : >0 numero letree du te voisin par l'arete i
c                           =0 si pas de te voisin (racine , ... )
c                 niveau  : =0 si nte et noteva ont meme taille
c                           >0 nte est 4**niveau fois plus petit que noteva
                  if( noteva .gt. 0 ) then
c                    il existe un te voisin
                     if( letree(0,noteva) .gt. 0 ) then
c                       noteva est plus petit que nte
c                       => recherche du numero du milieu du cote=sommet du te no
c                       le sous-te 0 du te noteva
                        nsot = letree(0,noteva)
c                       le numero dans pxyd du milieu de l'arete i de nte
                        milieu( i ) = letree( 5+nopre3(i), nsot )
                        nbmili = nbmili + 1
                     endif
                  endif
c
 30            continue
c
c              triangulation du te nte en fonction du nombre de ses milieux
               goto( 50, 100, 200, 300 ) , nbmili + 1
c
c              0 milieu => 1 triangle = le te nte
c              ----------------------------------
 50            call f0trte( letree(0,nte),  pxyd,
     %                      mosoar, mxsoar, n1soar, nosoar,
     %                      moartr, mxartr, n1artr, noartr,
     %                      noarst,
     %                      nbtr,   nutr,   ierr )
               if( ierr .ne. 0 ) return
               goto 10
c
c              1 milieu => 2 triangles = 2 demi te
c              -----------------------------------
 100           call f1trte( letree(0,nte),  pxyd,   milieu,
     %                      mosoar, mxsoar, n1soar, nosoar,
     %                      moartr, mxartr, n1artr, noartr,
     %                      noarst,
     %                      nbtr,   nutr,   ierr )
               if( ierr .ne. 0 ) return
               goto 10
c
c              2 milieux => 3 triangles
c              -----------------------------------
 200           call f2trte( letree(0,nte),  pxyd,   milieu,
     %                      mosoar, mxsoar, n1soar, nosoar,
     %                      moartr, mxartr, n1artr, noartr,
     %                      noarst,
     %                      nbtr,   nutr,   ierr )
               if( ierr .ne. 0 ) return
               goto 10
c
c              3 milieux => 4 triangles = 4 quart te
c              -------------------------------------
 300           call f3trte( letree(0,nte),  pxyd,   milieu,
     %                      mosoar, mxsoar, n1soar, nosoar,
     %                      moartr, mxartr, n1artr, noartr,
     %                      noarst,
     %                      nbtr,   nutr,   ierr )
               if( ierr .ne. 0 ) return
               goto 10
            endif
         endif
         goto 10
      endif
      end


      subroutine aisoar( mosoar, mxsoar, nosoar, na1 )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    chainer en colonne lchain les aretes non vides et
c -----    non frontalieres du tableau nosoar
c
c entrees:
c --------
c mosoar : nombre maximal d'entiers par arete dans le tableau nosoar
c mxsoar : nombre maximal d'aretes frontalieres declarables
c
c modifies :
c ----------
c nosoar : numero des 2 sommets , no ligne, 2 triangles, chainages en +
c          nosoar(lchain,i)=arete interne suivante
c
c sortie :
c --------
c na1    : numero dans nosoar de la premiere arete interne
c          les suivantes sont nosoar(lchain,na1), ...
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
      parameter (lchain=6)
      integer    nosoar(mosoar,mxsoar)
c
c     formation du chainage des aretes internes a echanger eventuellement
c     recherche de la premiere arete non vide et non frontaliere
      do 10 na1=1,mxsoar
         if( nosoar(1,na1) .gt. 0 .and. nosoar(3,na1) .le. 0 ) goto 15
 10   continue
c
c     protection de la premiere arete non vide et non frontaliere
 15   na0 = na1
      do 20 na=na1+1,mxsoar
         if( nosoar(1,na) .gt. 0 .and. nosoar(3,na) .le. 0 ) then
c           arete interne => elle est chainee a partir de la precedente
            nosoar(lchain,na0) = na
            na0 = na
         endif
 20   continue
c
c     la derniere arete interne n'a pas de suivante
      nosoar(lchain,na0) = 0
      end


      subroutine tedela( pxyd,   noarst,
     %                   mosoar, mxsoar, n1soar, nosoar, n1ardv,
     %                   moartr, mxartr, n1artr, noartr, modifs )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    pour toutes les aretes chainees dans nosoar(lchain,*)
c -----    du tableau nosoar
c          echanger la diagonale des 2 triangles si le sommet oppose
c          a un triangle ayant en commun une arete appartient au cercle
c          circonscrit de l'autre (violation boule vide delaunay)
c
c entrees:
c --------
c pxyd   : tableau des x  y  distance_souhaitee de chaque sommet
c
c modifies :
c ----------
c noarst : noarst(i) numero d'une arete de sommet i
c mosoar : nombre maximal d'entiers par arete dans le tableau nosoar
c mxsoar : nombre maximal d'aretes frontalieres declarables
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c nosoar : numero des 2 sommets , no ligne, 2 triangles, chainages en +
c n1ardv : numero dans nosoar de la premiere arete du chainage
c          des aretes a rendre delaunay
c
c moartr : nombre d'entiers par triangle dans le tableau noartr
c mxartr : nombre maximal de triangles declarables dans noartr
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c modifs : nombre d'echanges de diagonales pour maximiser la qualite
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
      parameter        (lchain=6)
      common / unites / lecteu, imprim, nunite(30)
      double precision  pxyd(3,*), surtd2, s123, s142, s143, s234,
     %                  s12, s34, a12, cetria(3), r0
      integer           nosoar(mosoar,mxsoar),
     %                  noartr(moartr,mxartr),
     %                  noarst(*)
c
c     le nombre d'echanges de diagonales pour minimiser l'aire
      modifs = 0
      r0     = 0
c
c     la premiere arete du chainage des aretes a rendre delaunay
      na0 = n1ardv
c
c     tant que la pile des aretes a echanger eventuellement est non vide
c     ==================================================================
 20   if( na0 .gt. 0 ) then
c
c        l'arete a traiter
         na  = na0
c        la prochaine arete a traiter
         na0 = nosoar(lchain,na0)
c
c        l'arete est marquee traitee avec le numero -1
         nosoar(lchain,na) = -1
c
c        l'arete est elle active?
         if( nosoar(1,na) .eq. 0 ) goto 20
c
c        si arete frontaliere pas d'echange possible
         if( nosoar(3,na) .gt. 0 ) goto 20
c
c        existe-t-il 2 triangles ayant cette arete commune?
         if( nosoar(4,na) .le. 0 .or. nosoar(5,na) .le. 0 ) goto 20
c
c        aucun des 2 triangles est-il desactive?
         if( noartr(1,nosoar(4,na)) .eq. 0 .or.
     %       noartr(1,nosoar(5,na)) .eq. 0 ) goto 20
c
c        l'arete appartient a deux triangles actifs
c        le numero des 4 sommets du quadrangle des 2 triangles
         call mt4sqa( na, moartr, noartr, mosoar, nosoar,
     %                ns1, ns2, ns3, ns4 )
         if( ns4 .eq. 0 ) goto 20
c
c        carre de la longueur de l'arete ns1 ns2
         a12 = (pxyd(1,ns2)-pxyd(1,ns1))**2+(pxyd(2,ns2)-pxyd(2,ns1))**2
c
c        comparaison de la somme des aires des 2 triangles
c        -------------------------------------------------
c        calcul des surfaces des triangles 123 et 142 de cette arete
         s123=surtd2( pxyd(1,ns1), pxyd(1,ns2), pxyd(1,ns3) )
         s142=surtd2( pxyd(1,ns1), pxyd(1,ns4), pxyd(1,ns2) )
         s12 = abs( s123 ) + abs( s142 )
         if( s12 .le. 0.001*a12 ) goto 20
c
c        calcul des surfaces des triangles 143 et 234 de cette arete
         s143=surtd2( pxyd(1,ns1), pxyd(1,ns4), pxyd(1,ns3) )
         s234=surtd2( pxyd(1,ns2), pxyd(1,ns3), pxyd(1,ns4) )
         s34 = abs( s234 ) + abs( s143 )
c
         if( abs(s34-s12) .gt. 1d-15*s34 ) goto 20
c
c        quadrangle convexe : le critere de delaunay intervient
c        ------------------   ---------------------------------
c        calcul du centre et rayon de la boule circonscrite a 123
c        pas d'affichage si le triangle est degenere
         ierr = -1
         call cenced( pxyd(1,ns1), pxyd(1,ns2), pxyd(1,ns3), cetria,
     %                ierr )
         if( ierr .gt. 0 ) then
c           ierr=1 si triangle degenere  => abandon
            goto 20
         endif
c
         if( (cetria(1)-pxyd(1,ns4))**2+(cetria(2)-pxyd(2,ns4))**2
     %       .lt. cetria(3) ) then
c
c           protection contre une boucle infinie sur le meme cercle
            if( r0 .eq. cetria(3) ) goto 20
c
c           oui: ns4 est dans le cercle circonscrit a ns1 ns2 ns3
c           => ns3 est aussi dans le cercle circonscrit de ns1 ns2 ns4
c
cccc           les 2 triangles d'arete na sont effaces
ccc            do 25 j=4,5
ccc               nt = nosoar(j,na)
cccc              trace du triangle nt
ccc               call mttrtr( pxyd, nt, moartr, noartr, mosoar, nosoar,
ccc     %                      ncnoir, ncjaun )
ccc 25         continue
c
c           echange de la diagonale 12 par 34 des 2 triangles
            call te2t2t( na,     mosoar, n1soar, nosoar, noarst,
     %                   moartr, noartr, na34 )
            if( na34 .eq. 0 ) goto 20
            r0 = cetria(3)
c
c           l'arete na34 est marquee traitee
            nosoar(lchain,na34) = -1
            modifs = modifs + 1
c
c           les aretes internes peripheriques des 2 triangles sont enchainees
            do 60 j=4,5
               nt = nosoar(j,na34)
cccc              trace du triangle nt
ccc               call mttrtr( pxyd, nt, moartr, noartr, mosoar, nosoar,
ccc     %                      ncoran, ncgric )
               do 50 i=1,3
                  n = abs( noartr(i,nt) )
                  if( n .ne. na34 ) then
                     if( nosoar(3,n)      .eq.  0  .and.
     %                   nosoar(lchain,n) .eq. -1 ) then
c                        cette arete marquee est chainee pour etre traitee
                         nosoar(lchain,n) = na0
                         na0 = n
                     endif
                  endif
 50            continue
 60         continue
            goto 20
         endif
c
c        retour en haut de la pile des aretes a traiter
         goto 20
      endif
      end


      subroutine terefr( nbarpi, pxyd,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, n1artr, noartr, noarst,
     %                   mxarcf, n1arcf, noarcf, larmin, notrcf,
     %                   nbarpe, ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :   recherche des aretes de la frontiere non dans la triangulation
c -----   triangulation frontale pour les reobtenir
c
c         attention: le chainage lchain de nosoar devient celui des cf
c
c entrees:
c --------
c          le tableau nosoar
c nbarpi : numero du dernier point interne impose par l'utilisateur
c pxyd   : tableau des coordonnees 2d des points
c          par point : x  y  distance_souhaitee
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c          attention: mxsoar>3*mxsomm obligatoire!
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxarcf : nombre de variables des tableaux n1arcf, noarcf, larmin, notrcf
c
c modifies:
c ---------
c n1soar : no de l'eventuelle premiere arete libre dans le tableau nosoar
c          chainage des vides suivant en 3 et precedant en 2 de nosoar
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = nosoar(1)+nosoar(2)*2
c          avec mxsoar>=3*mxsomm
c          une arete i de nosoar est vide <=> nosoar(1,i)=0 et
c          nosoar(2,arete vide)=l'arete vide qui precede
c          nosoar(3,arete vide)=l'arete vide qui suit
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c noarst : noarst(i) numero d'une arete de sommet i
c
c
c auxiliaires :
c -------------
c n1arcf : tableau (0:mxarcf) auxiliaire d'entiers
c noarcf : tableau (3,mxarcf) auxiliaire d'entiers
c larmin : tableau (mxarcf)   auxiliaire d'entiers
c notrcf : tableau (mxarcf)   auxiliaire d'entiers
c
c sortie :
c --------
c nbarpe : nombre d'aretes perdues puis retrouvees
c ierr   : =0 si pas d'erreur
c          >0 si une erreur est survenue
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
      parameter        (lchain=6)
      common / unites / lecteu,imprim,intera,nunite(29)
      double precision  pxyd(3,*)
      integer           nosoar(mosoar,mxsoar),
     %                  noartr(moartr,*),
     %                  noarst(*),
     %                  n1arcf(0:mxarcf),
     %                  noarcf(3,mxarcf),
     %                  larmin(mxarcf),
     %                  notrcf(mxarcf)
c
c     le nombre d'aretes de la frontiere non arete de la triangulation
      nbarpe = 0
c
c     initialisation du chainage des aretes des cf => 0 arete de cf
      do 10 narete=1,mxsoar
         nosoar( lchain, narete) = -1
 10   continue
c
c     boucle sur l'ensemble des aretes actuelles
c     ==========================================
      do 30 narete=1,mxsoar
c
         if( nosoar(3,narete) .gt. 0 ) then
c           arete appartenant a une ligne => frontaliere
c
            if(nosoar(4,narete) .le. 0 .or. nosoar(5,narete) .le. 0)then
c              l'arete narete frontaliere n'appartient pas a 2 triangles
c              => elle est perdue
               nbarpe = nbarpe + 1
c
c              le numero des 2 sommets de l'arete frontaliere perdue
               ns1 = nosoar( 1, narete )
               ns2 = nosoar( 2, narete )
c               write(imprim,10000) ns1,(pxyd(j,ns1),j=1,2),
c     %                             ns2,(pxyd(j,ns2),j=1,2)
10000          format(' arete perdue a forcer',
     %               (t24,'sommet=',i6,' x=',g13.5,' y=',g13.5))
c
c              traitement de cette arete perdue ns1-ns2
               call tefoar( narete, nbarpi, pxyd,
     %                      mosoar, mxsoar, n1soar, nosoar,
     %                      moartr, n1artr, noartr, noarst,
     %                      mxarcf, n1arcf, noarcf, larmin, notrcf,
     %                      ierr )
               if( ierr .ne. 0 ) return
c
c              fin du traitement de cette arete perdue et retrouvee
            endif
         endif
c
 30   continue
      end


      subroutine tesuex( nblftr, nulftr,
     %                   ndtri0, nbsomm, pxyd, nslign,
     %                   mosoar, mxsoar, nosoar,
     %                   moartr, mxartr, n1artr, noartr, noarst,
     %                   nbtria, letrsu, ierr  )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    supprimer du tableau noartr les triangles externes au domaine
c -----    en annulant le numero de leur 1-ere arete dans noartr
c          et en les chainant comme triangles vides
c
c entrees:
c --------
c nblftr : nombre de  lignes fermees definissant la surface
c nulftr : numero des lignes fermees definissant la surface
c ndtri0 : plus grand numero dans noartr d'un triangle
c pxyd   : tableau des coordonnees 2d des points
c          par point : x  y  distance_souhaitee
c nslign : tableau du numero de sommet dans sa ligne pour chaque
c          sommet frontalier
c          numero du point dans le lexique point si interne impose
c          0 si le point est interne non impose par l'utilisateur
c         -1 si le sommet est externe au domaine
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c          attention: mxsoar>3*mxsomm obligatoire!
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = nosoar(1)+nosoar(2)*2
c          avec mxsoar>=3*mxsomm
c          une arete i de nosoar est vide <=> nosoar(1,i)=0 et
c          nosoar(2,arete vide)=l'arete vide qui precede
c          nosoar(3,arete vide)=l'arete vide qui suit
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxartr : nombre maximal de triangles declarables
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c noarst : noarst(i) numero nosoar d'une arete de sommet i
c
c sorties:
c --------
c nbtria : nombre de triangles internes au domaine
c letrsu : letrsu(nt)=numero du triangle interne, 0 sinon
c noarst : noarst(i) numero nosoar d'une arete du sommet i (modifi'e)
c ierr   : 0 si pas d'erreur, >0 sinon
cc++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc        mai 1999
c2345x7..............................................................012
      double precision  pxyd(3,*)
      integer           nulftr(nblftr),nslign(nbsomm),
     %                  nosoar(mosoar,mxsoar),
     %                  noartr(moartr,mxartr),
     %                  noarst(*)
      integer           letrsu(1:ndtri0)
      double precision  dmin
c
c     les triangles sont a priori non marques
      do 5 nt=1,ndtri0
         letrsu(nt) = 0
 5    continue
c
c     les aretes sont marquees non chainees
      do 10 noar1=1,mxsoar
         nosoar(6,noar1) = -2
 10   continue
c
c     recherche du sommet de la triangulation de plus petite abscisse
c     ===============================================================
      ntmin = 0
      dmin  = 1d38
      do 20 i=1,nbsomm
         if( pxyd(1,i) .lt. dmin ) then
c           le nouveau minimum
            noar1 = noarst(i)
            if( noar1 .gt. 0 ) then
c              le sommet appartient a une arete de triangle
               if( nosoar(4,noar1) .gt. 0 ) then
c                 le nouveau minimum
                  dmin  = pxyd(1,i)
                  ntmin = i
               endif
            endif
         endif
 20   continue
c
c     une arete de sommet ntmin
      noar1 = noarst( ntmin )
c     un triangle d'arete noar1
      ntmin = nosoar( 4, noar1 )
      if( ntmin .le. 0 ) then
c         nblgrc(nrerr) = 1
c         kerr(1) = 'pas de triangle d''abscisse minimale'
c         call lereur
         write(imprim,*) 'pas de triangle d''abscisse minimale'
         ierr = 2
         goto 9990
      endif
c
c     chainage des 3 aretes du triangle ntmin
c     =======================================
c     la premiere arete du chainage des aretes traitees
      noar1 = abs( noartr(1,ntmin) )
      na0   = abs( noartr(2,ntmin) )
c     elle est chainee sur la seconde arete du triangle ntmin
      nosoar(6,noar1) = na0
c     les 2 autres aretes du triangle ntmin sont chainees
      na1 = abs( noartr(3,ntmin) )
c     la seconde est chainee sur la troisieme arete
      nosoar(6,na0) = na1
c     la troisieme n'a pas de suivante
      nosoar(6,na1) = 0
c
c     le triangle ntmin est a l'exterieur du domaine
c     tous les triangles externes sont marques -123 456 789
c     les triangles de l'autre cote d'une arete sur une ligne
c     sont marques: no de la ligne de l'arete * signe oppose
c     =======================================================
      ligne0 = 0
      ligne  = -123 456 789
c
 40   if( noar1 .ne. 0 ) then
c
c        l'arete noar1 du tableau nosoar est a traiter
c        ---------------------------------------------
         noar = noar1
c        l'arete suivante devient la premiere a traiter ensuite
         noar1 = nosoar(6,noar1)
c        l'arete noar est traitee
         nosoar(6,noar) = -3
c
         do 60 i=4,5
c
c           l'un des 2 triangles de l'arete
            nt = nosoar(i,noar)
            if( nt .gt. 0 ) then
c
c              triangle deja traite pour une ligne anterieure?
               if(     letrsu(nt)  .ne. 0      .and.
     %             abs(letrsu(nt)) .ne. ligne ) goto 60
c
cccc              trace du triangle nt en couleur ligne0
ccc               call mttrtr( pxyd,   nt, moartr, noartr, mosoar, nosoar,
ccc     %                      ligne0, ncnoir )
c
c              le triangle est marque avec la valeur de ligne
               letrsu(nt) = ligne
c
c              chainage eventuel des autres aretes de ce triangle
c              si ce n'est pas encore fait
               do 50 j=1,3
c
c                 le numero na de l'arete j du triangle nt dans nosoar
                  na = abs( noartr(j,nt) )
                  if( nosoar(6,na) .ne. -2 ) goto 50
c
c                 le numero de 1 a nblftr dans nulftr de la ligne de l'arete
                  nl = nosoar(3,na)
c
c                 si l'arete est sur une ligne fermee differente de celle envelo
c                 et non marquee alors examen du triangle oppose
                  if( nl .gt. 0 ) then
c
                     if( nl .eq. ligne0 ) goto 50
c
c                    arete frontaliere de ligne non traitee
c                    => passage de l'autre cote de la ligne
c                    le triangle de l'autre cote de la ligne est recherche
                     if( nt .eq. abs( nosoar(4,na) ) ) then
                        nt2 = 5
                     else
                        nt2 = 4
                     endif
                     nt2 = abs( nosoar(nt2,na) )
                     if( nt2 .gt. 0 ) then
c
c                       le triangle nt2 de l'autre cote est marque avec le
c                       avec le signe oppose de celui de ligne
                        if( ligne .ge. 0 ) then
                           lsigne = -1
                        else
                           lsigne =  1
                        endif
                        letrsu(nt2) = lsigne * nl
c
c                       temoin de ligne a traiter ensuite dans nulftr
                        nulftr(nl) = -abs( nulftr(nl) )
c
cccc                       trace du triangle nt2 en jaune borde de magenta
ccc                        call mttrtr( pxyd,nt2,
ccc     %                               moartr,noartr,mosoar,nosoar,
ccc     %                               ncjaun, ncmage )
c
c                       l'arete est traitee
                        nosoar(6,na) = -3
c
                     endif
c
c                    l'arete est traitee
                     goto 50
c
                  endif
c
c                 arete non traitee => elle est chainee
                  nosoar(6,na) = noar1
                  noar1 = na
c
 50            continue
c
            endif
 60      continue
c
         goto 40
      endif
c     les triangles de la ligne fermee ont tous ete marques
c     plus d'arete chainee
c
c     recherche d'une nouvelle ligne fermee a traiter
c     ===============================================
 65   do 70 nl=1,nblftr
         if( nulftr(nl) .lt. 0 ) goto 80
 70   continue
c     plus de ligne fermee a traiter
      goto 110
c
c     tous les triangles de cette composante connexe
c     entre ligne et ligne0 vont etre marques
c     ==============================================
c     remise en etat du numero de ligne
c     nl est le numero de la ligne dans nulftr a traiter
 80   nulftr(nl) = -nulftr(nl)
      do 90 nt2=1,ndtri0
         if( abs(letrsu(nt2)) .eq. nl ) goto 92
 90   continue
c
c     recherche de l'arete j du triangle nt2 avec ce numero de ligne nl
 92   do 95 j=1,3
c
c        le numero de l'arete j du triangle dans nosoar
         noar1 = 0
         na0   = abs( noartr(j,nt2) )
         if( nl .eq. nosoar(3,na0) ) then
c
c           na0 est l'arete de ligne nl
c           l'arete suivante du triangle nt2
            i   = mod(j,3) + 1
c           le numero dans nosoar de l'arete i de nt2
            na1 = abs( noartr(i,nt2) )
            if( nosoar(6,na1) .eq. -2 ) then
c              arete non traitee => elle est la premiere du chainage
               noar1 = na1
c              pas de suivante dans ce chainage
               nosoar(6,na1) = 0
            else
               na1 = 0
            endif
c
c           l'eventuelle seconde arete suivante
            i  = mod(i,3) + 1
            na = abs( noartr(i,nt2) )
            if( nosoar(6,na) .eq. -2 ) then
               if( na1 .eq. 0 ) then
c                 1 arete non traitee et seule a chainer
                  noar1 = na
                  nosoar(6,na) = 0
               else
c                 2 aretes a chainer
                  noar1 = na
                  nosoar(6,na) = na1
               endif
            endif
c
            if( noar1 .gt. 0 ) then
c
c              il existe au moins une arete a visiter pour ligne
c              marquage des triangles internes a la ligne nl
               ligne  = letrsu(nt2)
               ligne0 = nl
               goto 40
c
            else
c
c              nt2 est le seul triangle de la ligne fermee
               goto 65
c
            endif
         endif
 95   continue
c
c     reperage des sommets internes ou externes dans nslign
c     nslign(sommet externe au domaine)=-1
c     nslign(sommet interne au domaine)= 0
c     =====================================================
 110  do 170 ns1=1,nbsomm
c        tout sommet non sur la frontiere ou interne impose
c        est suppose externe
         if( nslign(ns1) .eq. 0 ) nslign(ns1) = -1
 170  continue
c
c     les triangles externes sont marques vides dans le tableau noartr
c     ================================================================
      nbtria = 0
      do 200 nt=1,ndtri0
c
         if( letrsu(nt) .le. 0 ) then
c
c           triangle nt externe
            if( noartr(1,nt) .ne. 0 ) then
c              la premiere arete est annulee
               noartr(1,nt) = 0
c              le triangle nt est considere comme etant vide
               noartr(2,nt) = n1artr
               n1artr = nt
            endif
c
         else
c
c           triangle nt interne
            nbtria = nbtria + 1
            letrsu(nt) = nbtria
c
c           marquage des 3 sommets du triangle nt
            do 190 i=1,3
c              le numero nosoar de l'arete i du triangle nt
               noar = abs( noartr(i,nt) )
c              le numero des 2 sommets
               ns1 = nosoar(1,noar)
               ns2 = nosoar(2,noar)
c              mise a jour du numero d'une arete des 2 sommets de l'arete
               noarst( ns1 ) = noar
               noarst( ns2 ) = noar
c              ns1 et ns2 sont des sommets de la triangulation du domaine
               if( nslign(ns1) .lt. 0 ) nslign(ns1)=0
               if( nslign(ns2) .lt. 0 ) nslign(ns2)=0
 190        continue
c
         endif
c
 200  continue
c     ici tout sommet externe ns verifie nslign(ns)=-1
c
c     les triangles externes sont mis a zero dans nosoar
c     ==================================================
      do 300 noar=1,mxsoar
c
         if( nosoar(1,noar) .gt. 0 ) then
c
c           le second triangle de l'arete noar
            nt = nosoar(5,noar)
            if( nt .gt. 0 ) then
c              si le triangle nt est externe
c              alors il est supprime pour l'arete noar
               if( letrsu(nt) .le. 0 ) nosoar(5,noar)=0
            endif
c
c           le premier triangle de l'arete noar
            nt = nosoar(4,noar)
            if( nt .gt. 0 ) then
               if( letrsu(nt) .le. 0 ) then
c                 si le triangle nt est externe
c                 alors il est supprime pour l'arete noar
c                 et l'eventuel triangle oppose prend sa place
c                 en position 4 de nosoar
                  if( nosoar(5,noar) .gt. 0 ) then
                     nosoar(4,noar)=nosoar(5,noar)
                     nosoar(5,noar)=0
                  else
                     nosoar(4,noar)=0
                  endif
               endif
            endif
         endif
c
 300  continue
c
c     remise en etat pour eviter les modifications de ladefi
 9990 do 9991 nl=1,nblftr
         if( nulftr(nl) .lt. 0 ) nulftr(nl)=-nulftr(nl)
 9991 continue
      return
      end



      subroutine trp1st( ns,     noarst, mosoar, nosoar, moartr, noartr,
     %                   mxpile, lhpile, lapile )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :   recherche des triangles de noartr partageant le sommet ns
c -----
c         limite: un camembert de centre ns entame 2 fois
c                 ne donne que l'une des parties
c
c entrees:
c --------
c ns     : numero du sommet
c noarst : noarst(i) numero d'une arete de sommet i
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c moartr : nombre maximal d'entiers par arete du tableau noartr
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c mxpile : nombre maximal de triangles empilables
c
c sorties :
c --------
c lhpile : >0 nombre de triangles empiles
c          =0       si impossible de tourner autour du point
c          =-lhpile si apres butee sur la frontiere il y a a nouveau
c          butee sur la frontiere . a ce stade on ne peut dire si tous
c          les triangles ayant ce sommet ont ete recenses
c          ce cas arrive seulement si le sommet est sur la frontiere
c lapile : numero dans noartr des triangles de sommet ns
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
      common / unites / lecteu, imprim, nunite(30)
      integer           noartr(moartr,*),
     %                  nosoar(mosoar,*),
     %                  noarst(*)
      integer           lapile(1:mxpile)
      integer           nosotr(3)
c
c     la premiere arete de sommet ns
      nar = noarst( ns )
      if( nar .le. 0 ) then
         write(imprim,*) 'trp1st: sommet',ns,' sans arete'
         goto 9999
      endif
c
c     l'arete nar est elle active?
      if( nosoar(1,nar) .le. 0 ) then
ccc         write(imprim,*) 'trp1st: arete vide',nar,
ccc     %                  ' st1:', nosoar(1,nar),' st2:',nosoar(2,nar)
         goto 9999
      endif
c
c     le premier triangle de sommet ns
      nt0 = abs( nosoar(4,nar) )
      if( nt0 .le. 0 ) then
         write(imprim,*) 'trp1st: sommet',ns,' dans aucun triangle'
         goto 9999
      endif
c
c     le triangle est il interne?
      if( noartr(1,nt0) .eq. 0 ) goto 9999
c
c     le numero des 3 sommets du triangle nt0 dans le sens direct
      call nusotr( nt0, mosoar, nosoar, moartr, noartr, nosotr )
c
c     reperage du sommet ns dans le triangle nt0
      do 5 nar=1,3
         if( nosotr(nar) .eq. ns ) goto 10
 5    continue
      nta = nt0
      goto 9995
c
c     ns retrouve : le triangle nt0 est empile
 10   lhpile = 1
      lapile(1) = nt0
      nta = nt0
c
c     recherche dans le sens des aiguilles d'une montre
c     (sens indirect) du triangle nt1 de l'autre cote de l'arete
c     nar du triangle et en tournant autour du sommet ns
c     ==========================================================
      noar = abs( noartr(nar,nt0) )
c     le triangle nt1 oppose du triangle nt0 par l'arete noar
      if( nosoar(4,noar) .eq. nt0 ) then
         nt1 = nosoar(5,noar)
      else
         nt1 = nosoar(4,noar)
      endif
c
c     la boucle sur les triangles nt1 de sommet ns dans le sens indirect
c     ==================================================================
      if( nt1 .gt. 0 ) then
c
         if( noartr(1,nt1) .eq. 0 ) goto 30
c
c        le triangle nt1 n'a pas ete detruit. il est actif
c        le triangle oppose par l'arete noar existe
c        le numero des 3 sommets du triangle nt1 dans le sens direct
 15      call nusotr( nt1, mosoar, nosoar, moartr, noartr, nosotr )
c
c        reperage du sommet ns dans nt1
         do 20 nar=1,3
            if( nosotr(nar) .eq. ns ) goto 25
 20      continue
         nta = nt1
         goto 9995
c
c        nt1 est empile
 25      if( lhpile .ge. mxpile ) goto 9990
         lhpile = lhpile + 1
         lapile(lhpile) = nt1
c
c        le triangle nt1 de l'autre cote de l'arete de sommet ns
c        sauvegarde du precedent triangle dans nta
         nta  = nt1
         noar = abs( noartr(nar,nt1) )
         if( nosoar(4,noar) .eq. nt1 ) then
            nt1 = nosoar(5,noar)
         else
            nt1 = nosoar(4,noar)
         endif
         if( nt1 .le. 0   ) goto 30
c        le triangle suivant est a l'exterieur
         if( nt1 .ne. nt0 ) goto 15
c
c        recherche terminee par arrivee sur nt0
c        les triangles forment un "cercle" de "centre" ns
         return
c
      endif
c
c     pas de triangle voisin a nt1
c     ============================
c     le parcours passe par 1 des triangles exterieurs
c     le parcours est inverse par l'arete de gauche
c     le triangle nta est le premier triangle empile
 30   lhpile = 1
      lapile(lhpile) = nta
c
c     le numero des 3 sommets du triangle nta dans le sens direct
      call nusotr( nta, mosoar, nosoar, moartr, noartr, nosotr )
      do 32 nar=1,3
         if( nosotr(nar) .eq. ns ) goto 33
 32   continue
      goto 9995
c
c     l'arete qui precede (rotation / ns dans le sens direct)
 33   if( nar .eq. 1 ) then
         nar = 3
      else
         nar = nar - 1
      endif
c
c     le triangle voisin de nta dans le sens direct
      noar = abs( noartr(nar,nta) )
      if( nosoar(4,noar) .eq. nta ) then
         nt1 = nosoar(5,noar)
      else
         nt1 = nosoar(4,noar)
      endif
      if( nt1 .le. 0 ) then
c        un seul triangle contient ns
         goto 70
      endif
c
c     boucle sur les triangles de sommet ns dans le sens direct
c     ==========================================================
 40   if( noartr(1,nt1) .eq. 0 ) goto 70
c
c     le triangle nt1 n'a pas ete detruit. il est actif
c     le numero des 3 sommets du triangle nt1 dans le sens direct
      call nusotr( nt1, mosoar, nosoar, moartr, noartr, nosotr )
c
c     reperage du sommet ns dans nt1
      do 50 nar=1,3
         if( nosotr(nar) .eq. ns ) goto 60
 50   continue
      nta = nt1
      goto 9995
c
c     nt1 est empile
 60   if( lhpile .ge. mxpile ) goto 9990
      lhpile = lhpile + 1
      lapile(lhpile) = nt1
c
c     l'arete qui precede dans le sens direct
      if( nar .eq. 1 ) then
         nar = 3
      else
         nar = nar - 1
      endif
c
c     l'arete de sommet ns dans nosoar
      noar = abs( noartr(nar,nt1) )
c
c     le triangle voisin de nta dans le sens direct
      nta  = nt1
      if( nosoar(4,noar) .eq. nt1 ) then
         nt1 = nosoar(5,noar)
      else
         nt1 = nosoar(4,noar)
      endif
      nta = nt1
      if( nt1 .gt. 0 ) goto 40
c
c     butee sur le trou => fin des triangles de sommet ns
c     ----------------------------------------------------
 70   lhpile = -lhpile
c     impossible ici de trouver les autres triangles de sommet ns
c     les triangles de sommet ns ne forment pas une boule de centre ns
      return
c
c     saturation de la pile des triangles
c     -----------------------------------
 9990 write(imprim,*) 'trp1st:saturation pile des triangles autour ',
     %'sommet',ns
      goto 9999
c
c     erreur triangle ne contenant pas le sommet ns
c     ----------------------------------------------
 9995 write(imprim,*) 'trp1st:triangle ',nta,' st=',
     %   (nosotr(nar),nar=1,3),' sans le sommet' ,ns
c
 9999 lhpile = 0
      return
      end



      subroutine nusotr( nt, mosoar, nosoar, moartr, noartr, nosotr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    calcul du numero des 3 sommets du triangle nt de noartr
c -----    dans le sens direct (aire>0 si non degenere)
c
c entrees:
c --------
c nt     : numero du triangle dans le tableau noartr
c mosoar : nombre maximal d'entiers par arete
c nosoar : numero des 2 sommets , no ligne, 2 triangles, chainages en +
c          sommet 1 = 0 si arete vide => sommet 2 = arete vide suivante
c moartr : nombre maximal d'entiers par arete du tableau noartr
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1=0 si triangle vide => arete2=triangle vide suivant
c
c sorties:
c --------
c nosotr : numero (dans le tableau pxyd) des 3 sommets du triangle
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c2345x7..............................................................012
      integer     nosoar(mosoar,*), noartr(moartr,*), nosotr(3)
c
c     les 2 sommets de l'arete 1 du triangle nt dans le sens direct
      na = noartr( 1, nt )
      if( na .gt. 0 ) then
         nosotr(1) = 1
         nosotr(2) = 2
      else
         nosotr(1) = 2
         nosotr(2) = 1
         na = -na
      endif
      nosotr(1) = nosoar( nosotr(1), na )
      nosotr(2) = nosoar( nosotr(2), na )
c
c     l'arete suivante
      na = abs( noartr(2,nt) )
c
c     le sommet nosotr(3 du triangle 123
      nosotr(3) = nosoar( 1, na )
      if( nosotr(3) .eq. nosotr(1) .or. nosotr(3) .eq. nosotr(2) ) then
         nosotr(3) = nosoar(2,na)
      endif
      end


      subroutine tesusp( nbarpi, pxyd,   noarst,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, mxartr, n1artr, noartr,
     %                   mxarcf, n1arcf, noarcf, larmin, notrcf, liarcf,
     %                   nbstsu, ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :   supprimer de la triangulation les sommets de te trop proches
c -----   soit d'un sommet frontalier ou point interne impose
c         soit d'une arete frontaliere
c
c         attention: le chainage lchain de nosoar devient celui des cf
c
c entrees:
c --------
c nbarpi : numero du dernier point interne impose par l'utilisateur
c pxyd   : tableau des coordonnees 2d des points
c          par point : x  y  distance_souhaitee
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c          attention: mxsoar>3*mxsomm obligatoire!
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxarcf : nombre de variables des tableaux n1arcf, noarcf, larmin, notrcf
c
c modifies:
c ---------
c noarst : noarst(i) numero d'une arete de sommet i
c n1soar : no de l'eventuelle premiere arete libre dans le tableau nosoar
c          chainage des vides suivant en 3 et precedant en 2 de nosoar
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = nosoar(1)+nosoar(2)*2
c          avec mxsoar>=3*mxsomm
c          une arete i de nosoar est vide <=> nosoar(1,i)=0 et
c          nosoar(2,arete vide)=l'arete vide qui precede
c          nosoar(3,arete vide)=l'arete vide qui suit
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c
c
c auxiliaires :
c -------------
c n1arcf : tableau (0:mxarcf) auxiliaire d'entiers
c noarcf : tableau (3,mxarcf) auxiliaire d'entiers
c larmin : tableau ( mxarcf ) auxiliaire d'entiers
c notrcf : tableau ( mxarcf ) auxiliaire d'entiers
c liarcf : tableau ( mxarcf ) auxiliaire d'entiers
c
c sortie :
c --------
c nbstsu : nombre de sommets de te supprimes
c ierr   : =0 si pas d'erreur
c          >0 si une erreur est survenue
c          11 algorithme defaillant
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
c      parameter       ( quamal=0.3 ) => ok
c      parameter       ( quamal=0.4 ) => pb pour le test ocean
c      parameter       ( quamal=0.5 ) => pb pour le test ocean
c
      parameter       ( quamal=0.333, lchain=6 )
      common / unites / lecteu,imprim,intera,nunite(29)
      double precision  pxyd(3,*), qualit
      integer           nosoar(mosoar,mxsoar),
     %                  noartr(moartr,*),
     %                  noarst(*),
     %                  n1arcf(0:mxarcf),
     %                  noarcf(3,mxarcf),
     %                  larmin(mxarcf),
     %                  notrcf(mxarcf),
     %                  liarcf(mxarcf)
c
      integer           nosotr(3)
      equivalence      (nosotr(1),ns1), (nosotr(2),ns2),
     %                 (nosotr(3),ns3)
c
c     le nombre de sommets de te supprimes
      nbstsu = 0
c
c     initialisation du chainage des aretes des cf => 0 arete de cf
      do 10 narete=1,mxsoar
         nosoar( lchain, narete ) = -1
 10   continue
c
c     boucle sur l'ensemble des sommets frontaliers ou points internes
c     ================================================================
      do 100 ns = 1, nbarpi
c
cccc        le nombre de sommets supprimes pour ce sommet ns
ccc         nbsuns = 0
c
c        la qualite minimale au dessous de laquelle le point proche
c        interne est supprime
         quaopt = quamal
c
c        une arete de sommet ns
 15      narete = noarst( ns )
         if( narete .le. 0 ) then
c           erreur: le point appartient a aucune arete
            write(imprim,*) 'sommet ',ns,' dans aucune arete'
            ierr = 11
            return
         endif
c
c        recherche des triangles de sommet ns
c        ils doivent former un contour ferme de type etoile
         call trp1st( ns, noarst, mosoar, nosoar, moartr, noartr,
     %                mxarcf, nbtrcf, notrcf )
         if( nbtrcf .le. 0 ) then
c           erreur: impossible de trouver tous les triangles de sommet ns
c           seule une partie est a priori retrouvee
            nbtrcf = -nbtrcf
         endif
c
c        boucle sur les triangles de l'etoile du sommet ns
         quamin = 2.0
         do 20 i=1,nbtrcf
c
c           le numero des 3 sommets du triangle nt
            nt = notrcf(i)
            call nusotr( nt, mosoar, nosoar, moartr, noartr,
     %                   nosotr )
c           nosotr(1:3) est en equivalence avec ns1, ns2, ns3
c
c           la qualite du triangle ns1 ns2 ns3
            call qutr2d( pxyd(1,ns1), pxyd(1,ns2), pxyd(1,ns3), qualit )
            if( qualit .lt. quamin ) then
               quamin = qualit
               ntqmin = nt
            endif
 20      continue
c
c        bilan sur la qualite des triangles de sommet ns
         if( quamin .lt. quaopt ) then
c
c           recherche du sommet de ntqmin le plus proche et non frontalier
c           ==============================================================
c           le numero des 3 sommets du triangle nt
            call nusotr( ntqmin, mosoar, nosoar, moartr, noartr,
     %                   nosotr )
            nste   = 0
            quamin = 1e28
            do 30 j=1,3
               if( nosotr(j) .ne. ns .and. nosotr(j) .gt. nbarpi ) then
                  d = (pxyd(1,nosotr(j))-pxyd(1,ns))**2
     %              + (pxyd(2,nosotr(j))-pxyd(2,ns))**2
                  if( d .lt. quamin ) then
                     quamin = d
                     nste   = j
                  endif
               endif
 30         continue
c
            if( nste .gt. 0 ) then
c
c              nste est le sommet le plus proche de ns de ce
c              triangle de mauvaise qualite et sommet non encore traite
               nste = nosotr( nste )
c
c              nste est un sommet de triangle equilateral
c              => le sommet nste va etre supprime
c              ==========================================
               call te1stm( nste,   pxyd,   noarst,
     %                      mosoar, mxsoar, n1soar, nosoar,
     %                      moartr, mxartr, n1artr, noartr,
     %                      mxarcf, n1arcf, noarcf,
     %                      larmin, notrcf, liarcf, ierr )
               if( ierr .eq. 0 ) then
c                 un sommet de te supprime de plus
                  nbstsu = nbstsu + 1
               else if( ierr .lt. 0 ) then
c                 le sommet nste est externe donc non supprime
c                 ou bien le sommet nste est le centre d'un cf dont toutes
c                 les aretes simples sont frontalieres
c                 dans les 2 cas le sommet n'est pas supprime
                  ierr = 0
                  goto 100
               else
c                 erreur motivant un arret de la triangulation
                  return
               endif
c
c              boucle jusqu'a obtenir une qualite suffisante
c              si triangulation tres irreguliere =>
c              destruction de beaucoup de points internes
c              les 2 variables suivantes brident ces destructions massives
ccc               nbsuns = nbsuns + 1
               quaopt = quaopt * 0.8
ccc               if( nbsuns .le. 5 ) goto 15
               goto 15
            endif
         endif
c
 100  continue
      end


      subroutine teamqa( nutysu,
     %                   noarst, mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, mxartr, n1artr, noartr,
     %                   mxtrcf, notrcf, nostbo,
     %                   n1arcf, noarcf, larmin,
     %                   comxmi, nbarpi, nbsomm, mxsomm, pxyd, nslign,
     %                   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but:    si la taille de l'arete moyenne est >ampli*taille souhaitee
c ----    alors ajout d'un sommet barycentre du plus grand triangle
c               de sommet ns
c         si la taille de l'arete moyenne est <ampli/2*taille souhaitee
c         alors suppression du sommet ns
c         sinon le sommet ns devient le barycentre pondere de ses voisins
c
c         remarque: ampli est defini dans $mefisto/mail/tehote.f
c         et doit avoir la meme valeur pour eviter trop de modifications
c
c entrees:
c --------
c nutysu : numero de traitement de areteideale() selon le type de surface
c          0 pas d'emploi de la fonction areteideale() => aretmx active
c          1 il existe une fonction areteideale()
c            dont seules les 2 premieres composantes de uv sont actives
c          autres options a definir...
c noarst : noarst(i) numero d'une arete de sommet i
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes frontalieres declarables
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxartr : nombre maximal de triangles declarables dans noartr
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c mxtrcf : nombre maximal de triangles empilables
c nbarpi : numero du dernier sommet frontalier ou interne impose
c nslign : tableau du numero de sommet dans sa ligne pour chaque
c          sommet frontalier
c          numero du point dans le lexique point si interne impose
c          0 si le point est interne non impose par l'utilisateur
c         -1 si le sommet est externe au domaine
c comxmi : min et max des coordonneees des sommets du maillage
c
c modifies :
c ----------
c nbsomm : nombre actuel de sommets de la triangulation
c          (certains sommets internes ont ete desactives ou ajoutes)
c pxyd   : tableau des coordonnees 2d des points
c
c auxiliaires:
c ------------
c notrcf : tableau ( mxtrcf ) auxiliaire d'entiers
c          numero dans noartr des triangles de sommet ns
c nostbo : tableau ( mxtrcf ) auxiliaire d'entiers
c          numero dans pxyd des sommets des aretes simples de la boule
c n1arcf : tableau (0:mxtrcf) auxiliaire d'entiers
c noarcf : tableau (3,mxtrcf) auxiliaire d'entiers
c larmin : tableau ( mxtrcf ) auxiliaire d'entiers
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       juin 1997
c....................................................................012
      double precision  ampli,ampli2
      parameter        (ampli=1.34d0,ampli2=ampli/2d0)
      parameter        (lchain=6)
      common / unites / lecteu, imprim, nunite(30)
      double precision  pxyd(3,*)
      double precision  ponder, ponde1, xbar, ybar, x, y, surtd2
      double precision  d, dmoy
      double precision  d2d3(3,3)
      real              origin(3), xyz(3)
      integer           noartr(moartr,*),
     %                  nosoar(mosoar,*),
     %                  noarst(*),
     %                  notrcf(mxtrcf),
     %                  nslign(*),
     %                  nostbo(*),
     %                  n1arcf(0:mxtrcf),
     %                  noarcf(3,mxtrcf),
     %                  larmin(mxtrcf)
      double precision  comxmi(3,2)
      integer           nosotr(3)
c
c     le nombre d'iterations pour ameliorer la qualite
      nbitaq = 4
      ier    = 0
c
c     initialisation du parcours
      nbs1 = nbsomm
      nbs2 = nbarpi + 1
      nbs3 = -1
c
      do 5000 iter=1,nbitaq
c
c        le nombre de sommets supprimes
         nbstsu = 0
         nbbaaj = 0
c
c        coefficient de ponderation croissant avec les iterations
         ponder = min( 1d0, ( 50 + (50*iter)/nbitaq ) * 0.01d0 )
         ponde1 = 1d0 - ponder
c
c        l'ordre du parcours dans le sens croissant ou decroissant
         nt   = nbs1
         nbs1 = nbs2
         nbs2 = nt
c        alternance du parcours
         nbs3 = -nbs3
c
         do 1000 ns = nbs1, nbs2, nbs3
c
c           le sommet est il interne au domaine?
            if( nslign(ns) .ne. 0 ) goto 1000
c
c           existe-t-il une arete de sommet ns ?
 10         noar = noarst( ns )
            if( noar .le. 0 ) goto 1000
c
c           le 1-er triangle de l'arete noar
            nt = nosoar( 4, noar )
            if( nt .le. 0 ) goto 1000
c
c           recherche des triangles de sommet ns
c           ils doivent former un contour ferme de type etoile
            call trp1st( ns, noarst, mosoar, nosoar, moartr, noartr,
     %                   mxtrcf, nbtrcf, notrcf )
            if( nbtrcf .le. 0 ) goto 1000
c
c           mise a jour de la distance souhaitee
            if( nutysu .gt. 0 ) then
c              la fonction taille_ideale(x,y,z) existe
c              calcul de pxyzd(3,ns) dans le repere initial => xyz(1:3)
               call tetaid( nutysu, pxyd(1,ns), pxyd(2,ns),
     %                      pxyd(3,ns), ier )
            endif
c
c           boucle sur les triangles qui forment une boule autour du sommet ns
            nbstbo = 0
c           chainage des aretes simples de la boule a rendre delaunay
            noar0  = 0
            do 40 i=1,nbtrcf
c
c              le numero de l'arete du triangle nt ne contenant pas le sommet ns
               nt = notrcf(i)
               do 20 na=1,3
c                 le numero de l'arete na dans le tableau nosoar
                  noar = abs( noartr(na,nt) )
                  if( nosoar(1,noar) .ne. ns   .and.
     %                nosoar(2,noar) .ne. ns ) goto 25
 20            continue
c
c              construction de la liste des sommets des aretes simples
c              de la boule des triangles de sommet ns
c              -------------------------------------------------------
 25            do 35 na=1,2
                  ns1 = nosoar(na,noar)
                  do 30 j=nbstbo,1,-1
                     if( ns1 .eq. nostbo(j) ) goto 35
 30               continue
c                 ns1 est un nouveau sommet a ajouter
                  nbstbo = nbstbo + 1
                  nostbo(nbstbo) = ns1
 35            continue
c
c              noar est une arete potentielle a rendre delaunay
               if( nosoar(3,noar) .eq. 0 ) then
c                 arete non frontaliere
                  nosoar(lchain,noar) = noar0
                  noar0 = noar
               endif
c
 40         continue
c
c           calcul des 2 coordonnees du barycentre de la boule du sommet ns
c           calcul de la longueur moyenne des aretes issues du sommet ns
c           ---------------------------------------------------------------
            xbar = 0d0
            ybar = 0d0
            dmoy = 0d0
            do 50 i=1,nbstbo
               x    = pxyd(1,nostbo(i))
               y    = pxyd(2,nostbo(i))
               xbar = xbar + x
               ybar = ybar + y
               dmoy = dmoy + sqrt( (x-pxyd(1,ns))**2+(y-pxyd(2,ns))**2 )
 50         continue
            dmoy = dmoy / nbstbo
c
c           pas de modification de la topologie lors de la derniere iteration
c           =================================================================
            if( iter .eq. nbitaq ) goto 200
c
c           si la taille de l'arete moyenne est >ampli*taille souhaitee
c           alors ajout d'un sommet barycentre du plus grand triangle
c                 de sommet ns
c           ===========================================================
            if( dmoy .gt. ampli*pxyd(3,ns) ) then
c
               dmoy = 0d0
               do 150 i=1,nbtrcf
c                 recherche du plus grand triangle en surface
                  call nusotr( notrcf(i), mosoar, nosoar,
     %                         moartr, noartr, nosotr )
                  d  = surtd2( pxyd(1,nosotr(1)),
     %                         pxyd(1,nosotr(2)),
     %                         pxyd(1,nosotr(3)) )
                  if( d .gt. dmoy ) then
                     dmoy = d
                     imax = i
                  endif
 150           continue
c
c              ajout du barycentre du triangle notrcf(imax)
               nt = notrcf( imax )
               call nusotr( nt, mosoar, nosoar,
     %                      moartr, noartr, nosotr )
               if( nbsomm .ge. mxsomm ) then
                  write(imprim,*) 'saturation du tableau pxyd'
c                 abandon de l'amelioration du sommet ns
                  goto 9999
               endif
               nbsomm = nbsomm + 1
               do 160 i=1,3
                  pxyd(i,nbsomm) = ( pxyd(i,nosotr(1))
     %                             + pxyd(i,nosotr(2))
     %                             + pxyd(i,nosotr(3)) ) / 3d0
 160           continue
c
               if( nutysu .gt. 0 ) then
c                 la fonction taille_ideale(x,y,z) existe
c                 calcul de pxyzd(3,nbsomm) dans le repere initial => xyz(1:3)
                  call tetaid( nutysu, pxyd(1,nbsomm), pxyd(2,nbsomm),
     %                         pxyd(3,nbsomm), ier )
               endif
c
c              sommet interne a la triangulation
               nslign(nbsomm) = 0
c
c              les 3 aretes du triangle nt sont a rendre delaunay
               do 170 i=1,3
                  noar = abs( noartr(i,nt) )
                  if( nosoar(3,noar) .eq. 0 ) then
c                    arete non frontaliere
                     if( nosoar(lchain,noar) .lt. 0 ) then
c                       arete non encore chainee
                        nosoar(lchain,noar) = noar0
                        noar0 = noar
                     endif
                  endif
 170           continue
c
c              triangulation du triangle de barycentre nbsomm
c              protection a ne pas modifier sinon erreur!
               call tr3str( nbsomm, nt,
     %                      mosoar, mxsoar, n1soar, nosoar,
     %                      moartr, mxartr, n1artr, noartr,
     %                      noarst,
     %                      nosotr, ierr )
               if( ierr .ne. 0 ) goto 9999
c
c              un barycentre ajoute de plus
               nbbaaj = nbbaaj + 1
c
c              les aretes chainees de la boule sont rendues delaunay
               goto 900
c
            endif
c
c           si la taille de l'arete moyenne est <ampli/2*taille souhaitee
c           alors suppression du sommet ns
c           =============================================================
            if( dmoy .lt. ampli2*pxyd(3,ns) ) then
c              remise a -1 du chainage des aretes peripheriques de la boule ns
               noar = noar0
 90            if( noar .gt. 0 ) then
c                 protection du no de l'arete suivante
                  na = nosoar(lchain,noar)
c                 l'arete interne est remise a -1
                  nosoar(lchain,noar) = -1
c                 l'arete suivante
                  noar = na
                  goto 90
               endif
               call te1stm( ns,     pxyd,   noarst,
     %                      mosoar, mxsoar, n1soar, nosoar,
     %                      moartr, mxartr, n1artr, noartr,
     %                      mxtrcf, n1arcf, noarcf,
     %                      larmin, notrcf, nostbo,
     %                      ierr )
               if( ierr .lt. 0 ) then
c                 le sommet ns est externe donc non supprime
c                 ou bien le sommet ns est le centre d'un cf dont toutes
c                 les aretes simples sont frontalieres
c                 dans les 2 cas le sommet ns n'est pas supprime
                  ierr = 0
                  goto 200
               else if( ierr .gt. 0 ) then
c                 erreur irrecuperable
                  goto 9999
               endif
               nbstsu = nbstsu + 1
               goto 1000
c
            endif
c
c           les 2 coordonnees du barycentre des sommets des aretes
c           simples de la boule du sommet ns
c           ======================================================
 200        xbar = xbar / nbstbo
            ybar = ybar / nbstbo
c
c           ponderation pour eviter les degenerescenses
            pxyd(1,ns) = ponde1 * pxyd(1,ns) + ponder * xbar
            pxyd(2,ns) = ponde1 * pxyd(2,ns) + ponder * ybar
c
            if( nutysu .gt. 0 ) then
c              la fonction taille_ideale(x,y,z) existe
c              calcul de pxyzd(3,ns) dans le repere initial => xyz(1:3)
               call tetaid( nutysu, pxyd(1,ns), pxyd(2,ns),
     %                      pxyd(3,ns), ier )
            endif
c
c           les aretes chainees de la boule sont rendues delaunay
 900        call tedela( pxyd,   noarst,
     %                   mosoar, mxsoar, n1soar, nosoar, noar0,
     %                   moartr, mxartr, n1artr, noartr, modifs )
c
 1000    continue
c
ccc         write(imprim,11000) nbstsu, nbbaaj
ccc11000 format( i6,' sommets supprimes ' ,
ccc     %        i6,' barycentres ajoutes' )
c
c        mise a jour pour ne pas oublier les nouveaux sommets
         if( nbs1 .gt. nbs2 ) then
            nbs1 = nbsomm
         else
            nbs2 = nbsomm
         endif
c
 5000 continue
c
 9999 return
      end


      subroutine teamsf( nutysu,
     %                   noarst, mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, mxartr, n1artr, noartr,
     %                   mxtrcf, notrcf, nostbo,
     %                   n1arcf, noarcf, larmin,
     %                   comxmi, nbarpi, nbsomm, mxsomm, pxyd, nslign,
     %                   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    modification de la topologie des triangles autour des
c -----    sommets frontaliers et mise en triangulation delaunay locale
c
c entrees:
c --------
c nutysu : numero de traitement de areteideale() selon le type de surface
c          0 pas d'emploi de la fonction areteideale() => aretmx active
c          1 il existe une fonction areteideale()
c            dont seules les 2 premieres composantes de uv sont actives
c          autres options a definir...
c noarst : noarst(i) numero d'une arete de sommet i
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes frontalieres declarables
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxartr : nombre maximal de triangles declarables dans noartr
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c mxtrcf : nombre maximal de triangles empilables
c nbarpi : numero du dernier sommet frontalier ou interne impose
c nslign : >0 => ns numero du point dans le lexique point si interne impose
c          ou => 1 000 000 * n + ns1
c              ou n   est le numero (1 a nblftr) de la ligne de ce point
c                 ns1 est le numero du point dans sa ligne
c          = 0 si le point est interne non impose par l'utilisateur
c          =-1 si le sommet est externe au domaine
c comxmi : min et max des coordonneees des sommets du maillage
c
c modifies :
c ----------
c nbsomm : nombre actuel de sommets de la triangulation
c          (certains sommets internes ont ete desactives ou ajoutes)
c pxyd   : tableau des coordonnees 2d des points
c
c auxiliaires:
c ------------
c notrcf : tableau ( mxtrcf ) auxiliaire d'entiers
c          numero dans noartr des triangles de sommet ns
c nostbo : tableau ( mxtrcf ) auxiliaire d'entiers
c          numero dans pxyd des sommets des aretes simples de la boule
c n1arcf : tableau (0:mxtrcf) auxiliaire d'entiers
c noarcf : tableau (3,mxtrcf) auxiliaire d'entiers
c larmin : tableau ( mxtrcf ) auxiliaire d'entiers
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc    janvier 1998
c....................................................................012
      parameter        (lchain=6)
      common / unites / lecteu, imprim, nunite(30)
      double precision  pxyd(3,*)
      double precision  a, angle, angled, pi, deuxpi, pis3
      double precision  d2d3(3,3)
      real              origin(3), xyz(3)
      integer           noartr(moartr,*),
     %                  nosoar(mosoar,*),
     %                  noarst(*),
     %                  notrcf(mxtrcf),
     %                  nslign(*),
     %                  nostbo(*),
     %                  n1arcf(0:mxtrcf),
     %                  noarcf(3,mxtrcf),
     %                  larmin(mxtrcf),
     %                  nosotr(3)
      double precision  comxmi(3,2)
c
c     le nombre d'iterations pour ameliorer la qualite
      nbitaq = 2
      ier    = 0
c
c     pi / 3
      pi     = atan(1d0) * 4d0
      pis3   = pi / 3d0
      deuxpi = 2d0 * pi
c
c     initialisation du parcours
      modifs = 0
      nbs1   = nbarpi
      nbs2   =  1
c     => pas de traitement sur les points des lignes de la frontiere
      nbs3   = -1
c
      do 5000 iter=1,nbitaq
c
c        le nombre de sommets supprimes
         nbstsu = 0
c
c        l'ordre du parcours dans le sens croissant ou decroissant
         nt   = nbs1
         nbs1 = nbs2
         nbs2 = nt
c        alternance du parcours
         nbs3 = -nbs3
c
         do 1000 ns = nbs1, nbs2, nbs3
c
c           le sommet est il sur une ligne de la frontiere?
c           if( nslign(ns) .lt. 1 000 000 ) goto 1000
c
c           traitement d'un sommet d'une ligne de la frontiere
c           ==================================================
c           existe-t-il une arete de sommet ns ?
            noar = noarst( ns )
            if( noar .le. 0 ) goto 1000
c
c           le 1-er triangle de l'arete noar
            nt = nosoar( 4, noar )
            if( nt .le. 0 ) goto 1000
c
c           recherche des triangles de sommet ns
c           ils doivent former un contour ferme de type camembert
            call trp1st( ns, noarst, mosoar, nosoar, moartr, noartr,
     %                   mxtrcf, nbtrcf, notrcf )
            if( nbtrcf .ge. -1 ) goto 1000
c
c           boucle sur les triangles qui forment un camembert autour du sommet n
            nbtrcf = -nbtrcf
c
c           angle interne au camembert autour du sommet ns
            angle = 0d0
            do 540 i=1,nbtrcf
c
c              le numero de l'arete du triangle nt ne contenant pas le sommet ns
               nt = notrcf(i)
               do 520 na=1,3
c                 le numero de l'arete na dans le tableau nosoar
                  noar = abs( noartr(na,nt) )
                  if( nosoar(1,noar) .ne. ns   .and.
     %                nosoar(2,noar) .ne. ns ) goto 525
 520           continue
c
c              calcul de l'angle (ns-st1 arete, ns-st2 arete)
 525           ns1 = nosoar(1,noar)
               ns2 = nosoar(2,noar)
               a   = angled( pxyd(1,ns), pxyd(1,ns1), pxyd(1,ns2) )
               if( a .gt. pi ) a = deuxpi - a
               angle = angle + a
c
 540        continue
c
c           nombre ideal de triangles autour du sommet ns
            n = nint( angle / pis3 )
            if( n .le. 1 ) goto 1000
            i = 1
            if( nbtrcf .gt. n ) then
c
c              ajout du barycentre du triangle "milieu"
               nt = notrcf( (n+1)/2 )
               call nusotr( nt, mosoar, nosoar,
     %                      moartr, noartr, nosotr )
               if( nbsomm .ge. mxsomm ) then
                  write(imprim,*) 'saturation du tableau pxyd'
c                 abandon de l'amelioration du sommet ns
                  goto 1000
               endif
               nbsomm = nbsomm + 1
               do 560 i=1,3
                  pxyd(i,nbsomm) = ( pxyd(i,nosotr(1))
     %                             + pxyd(i,nosotr(2))
     %                             + pxyd(i,nosotr(3)) ) / 3d0
 560           continue
c
               if( nutysu .gt. 0 ) then
c                 la fonction taille_ideale(x,y,z) existe
c                 calcul de pxyzd(3,nbsomm) dans le repere initial => xyz(1:3)
                  call tetaid( nutysu, pxyd(1,nbsomm), pxyd(2,nbsomm),
     %                         pxyd(3,nbsomm), ier )
               endif
c
c              sommet interne a la triangulation
               nslign(nbsomm) = 0
c
c              les 3 aretes du triangle nt sont a rendre delaunay
               noar0 = 0
               do 570 i=1,3
                  noar = abs( noartr(i,nt) )
                  if( nosoar(3,noar) .eq. 0 ) then
c                    arete non frontaliere
                     if( nosoar(lchain,noar) .lt. 0 ) then
c                       arete non encore chainee
                        nosoar(lchain,noar) = noar0
                        noar0 = noar
                     endif
                  endif
 570           continue
c
c              triangulation du triangle de barycentre nbsomm
c              protection a ne pas modifier sinon erreur!
               call tr3str( nbsomm, nt,
     %                      mosoar, mxsoar, n1soar, nosoar,
     %                      moartr, mxartr, n1artr, noartr,
     %                      noarst,
     %                      nosotr, ierr )
               if( ierr .ne. 0 ) goto 9999
c
c              les aretes chainees de la boule sont rendues delaunay
               call tedela( pxyd,   noarst,
     %                      mosoar, mxsoar, n1soar, nosoar, noar0,
     %                      moartr, mxartr, n1artr, noartr, modifs )
            endif
c
 1000    continue
c
 5000 continue
c
 9999 return
      end


      subroutine teamqs( nutysu,
     %                   noarst, mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, mxartr, n1artr, noartr,
     %                   mxtrcf, notrcf, nostbo,
     %                   n1arcf, noarcf, larmin,
     %                   comxmi, nbarpi, nbsomm, mxsomm, pxyd, nslign,
     %                   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    une iteration de barycentrage des points internes
c -----    modification de la topologie pour avoir 4 ou 5 ou 6 triangles
c          pour chaque sommet de la triangulation
c          mise en triangulation delaunay
c
c entrees:
c --------
c nutysu : numero de traitement de areteideale() selon le type de surface
c          0 pas d'emploi de la fonction areteideale() => aretmx active
c          1 il existe une fonction areteideale()
c            dont seules les 2 premieres composantes de uv sont actives
c          autres options a definir...
c noarst : noarst(i) numero d'une arete de sommet i
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes frontalieres declarables
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxartr : nombre maximal de triangles declarables dans noartr
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c mxtrcf : nombre maximal de triangles empilables
c nbarpi : numero du dernier sommet frontalier ou interne impose
c nslign : >0 => ns numero du point dans le lexique point si interne impose
c          ou => 1 000 000 * n + ns1
c              ou n   est le numero (1 a nblftr) de la ligne de ce point
c                 ns1 est le numero du point dans sa ligne
c          = 0 si le point est interne non impose par l'utilisateur
c          =-1 si le sommet est externe au domaine
c comxmi : min et max des coordonneees des sommets du maillage
c
c modifies :
c ----------
c nbsomm : nombre actuel de sommets de la triangulation
c          (certains sommets internes ont ete desactives ou ajoutes)
c pxyd   : tableau des coordonnees 2d des points
c
c auxiliaires:
c ------------
c notrcf : tableau ( mxtrcf ) auxiliaire d'entiers
c          numero dans noartr des triangles de sommet ns
c nostbo : tableau ( mxtrcf ) auxiliaire d'entiers
c          numero dans pxyd des sommets des aretes simples de la boule
c n1arcf : tableau (0:mxtrcf) auxiliaire d'entiers
c noarcf : tableau (3,mxtrcf) auxiliaire d'entiers
c larmin : tableau ( mxtrcf ) auxiliaire d'entiers
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc        mai 1997
c....................................................................012
      parameter        (lchain=6)
      common / unites / lecteu, imprim, nunite(30)
      double precision  pxyd(3,*)
      double precision  ponder, ponde1, xbar, ybar, x, y, d, dmin, dmax
      double precision  d2d3(3,3)
      real              origin(3), xyz(3)
      integer           noartr(moartr,*),
     %                  nosoar(mosoar,*),
     %                  noarst(*),
     %                  notrcf(mxtrcf),
     %                  nslign(*),
     %                  nostbo(*),
     %                  n1arcf(0:mxtrcf),
     %                  noarcf(3,mxtrcf),
     %                  larmin(mxtrcf)
      integer           nosotr(3,2)
      double precision  comxmi(3,2)
c
c     le nombre d'iterations pour ameliorer la qualite
      nbitaq = 6
      ier    = 0
c
c     initialisation du parcours
      nbs1 = nbsomm
      nbs2 = nbarpi + 1
c     => pas de traitement sur les points des lignes de la frontiere
      nbs3 = -1
c
      do 5000 iter=1,nbitaq
c
c        le nombre de sommets supprimes
         nbstsu = 0
c
c        les compteurs de passage sur les differents cas
         nbst4 = 0
         nbst5 = 0
         nbst8 = 0
c
c        coefficient de ponderation croissant avec les iterations
         ponder = min( 1d0, ( 50 + (50*iter)/nbitaq ) * 0.01d0 )
         ponde1 = 1d0 - ponder
c
c        l'ordre du parcours dans le sens croissant ou decroissant
         nt   = nbs1
         nbs1 = nbs2
         nbs2 = nt
c        alternance du parcours
         nbs3 = -nbs3
c
         do 1000 ns = nbs1, nbs2, nbs3
c
c           le sommet est il interne au domaine?
            if( nslign(ns) .ne. 0 ) goto 1000
c
c           traitement d'un sommet interne non impose par l'utilisateur
c           ===========================================================
c           existe-t-il une arete de sommet ns ?
 10         noar = noarst( ns )
            if( noar .le. 0 ) goto 1000
c
c           le 1-er triangle de l'arete noar
            nt = nosoar( 4, noar )
            if( nt .le. 0 ) goto 1000
c
c           recherche des triangles de sommet ns
c           ils doivent former un contour ferme de type etoile
            call trp1st( ns, noarst, mosoar, nosoar, moartr, noartr,
     %                   mxtrcf, nbtrcf, notrcf )
            if( nbtrcf .le. 0 ) goto 1000
c
c           boucle sur les triangles qui forment une boule autour du sommet ns
            nbstbo = 0
c           chainage des aretes simples de la boule a rendre delaunay
            noar0  = 0
            do 40 i=1,nbtrcf
c
c              le numero de l'arete du triangle nt ne contenant pas le sommet ns
               nt = notrcf(i)
               do 20 na=1,3
c                 le numero de l'arete na dans le tableau nosoar
                  noar = abs( noartr(na,nt) )
                  if( nosoar(1,noar) .ne. ns   .and.
     %                nosoar(2,noar) .ne. ns ) goto 25
 20            continue
c
c              construction de la liste des sommets des aretes simples
c              de la boule des triangles de sommet ns
c              -------------------------------------------------------
 25            do 35 na=1,2
                  ns1 = nosoar(na,noar)
                  do 30 j=nbstbo,1,-1
                     if( ns1 .eq. nostbo(j) ) goto 35
 30               continue
c                 ns1 est un nouveau sommet a ajouter
                  nbstbo = nbstbo + 1
                  nostbo(nbstbo) = ns1
 35            continue
c
c              noar est une arete potentielle a rendre delaunay
               if( nosoar(3,noar) .eq. 0 ) then
c                 arete non frontaliere
                  nosoar(lchain,noar) = noar0
                  noar0 = noar
               endif
c
 40         continue
c
c           calcul des 2 coordonnees du barycentre de la boule du sommet ns
c           calcul de l'arete de taille maximale et minimale issue de ns
c           ---------------------------------------------------------------
            xbar = 0d0
            ybar = 0d0
            dmin = 1d28
            dmax = 0d0
            do 50 i=1,nbstbo
               x    = pxyd(1,nostbo(i))
               y    = pxyd(2,nostbo(i))
               xbar = xbar + x
               ybar = ybar + y
               d    = (x-pxyd(1,ns)) ** 2 + (y-pxyd(2,ns)) ** 2
               if( d .gt. dmax ) then
                  dmax = d
                  imax = i
               endif
               if( d .lt. dmin ) then
                  dmin = d
                  imin = i
               endif
 50         continue
c
c           pas de modification de la topologie lors de la derniere iteration
c           =================================================================
            if( iter .ge. nbitaq ) goto 200
c
c           si la boule de ns contient 3 ou 4 triangles le sommet ns est detruit
c           ====================================================================
            if( nbtrcf .le. 4 ) then
c
c              remise a -1 du chainage des aretes peripheriques de la boule ns
               noar = noar0
 60            if( noar .gt. 0 ) then
c                 protection du no de l'arete suivante
                  na = nosoar(lchain,noar)
c                 l'arete interne est remise a -1
                  nosoar(lchain,noar) = -1
c                 l'arete suivante
                  noar = na
                  goto 60
               endif
               call te1stm( ns,     pxyd,   noarst,
     %                      mosoar, mxsoar, n1soar, nosoar,
     %                      moartr, mxartr, n1artr, noartr,
     %                      mxtrcf, n1arcf, noarcf,
     %                      larmin, notrcf, nostbo,
     %                      ierr )
               if( ierr .lt. 0 ) then
c                 le sommet ns est externe donc non supprime
c                 ou bien le sommet ns est le centre d'un cf dont toutes
c                 les aretes simples sont frontalieres
c                 dans les 2 cas le sommet ns n'est pas supprime
                  ierr = 0
                  goto 200
               else if( ierr .eq. 0 ) then
                  nbst4  = nbst4 + 1
                  nbstsu = nbstsu + 1
               else
c                 erreur irrecuperable
                  goto 9999
               endif
               goto 1000
c
            endif
c
c           si la boule de ns contient 5 triangles et a un sommet voisin
c           sommet de 5 triangles alors l'arete joignant ces 2 sommets
c           est transformee en un seul sommet de 6 triangles
c           ============================================================
            if( nbtrcf .eq. 5 ) then
c
               do 80 i=1,5
c                 le numero du sommet de l'arete i et different de ns
                  ns1 = nostbo(i)
c                 la liste des triangles de sommet ns1
                  call trp1st( ns1, noarst,
     %                         mosoar, nosoar, moartr, noartr,
     %                         mxtrcf-5, nbtrc1, notrcf(6) )
                  if( nbtrc1 .eq. 5 ) then
c
c                    l'arete de sommets ns-ns1 devient un point
c                    par suppression du sommet ns
c
c                    remise a -1 du chainage des aretes peripheriques de la boul
                     noar = noar0
 70                  if( noar .gt. 0 ) then
c                       protection du no de l'arete suivante
                        na = nosoar(lchain,noar)
c                       l'arete interne est remise a -1
                        nosoar(lchain,noar) = -1
c                       l'arete suivante
                        noar = na
                        goto 70
                     endif
c
c                    le point ns1 devient le milieu de l'arete ns-ns1
                     do 75 j=1,3
                        pxyd(j,ns1) = (pxyd(j,ns) + pxyd(j,ns1)) * 0.5d0
 75                  continue
c
                     if( nutysu .gt. 0 ) then
c                       la fonction taille_ideale(x,y,z) existe
c                       calcul de pxyzd(3,ns1) dans le repere initial => xyz(1:3
                        call tetaid( nutysu,pxyd(1,ns1),pxyd(2,ns1),
     %                               pxyd(3,ns1), ier )
                     endif
c
c                    suppression du point ns et mise en delaunay
                     call te1stm( ns,     pxyd,   noarst,
     %                            mosoar, mxsoar, n1soar, nosoar,
     %                            moartr, mxartr, n1artr, noartr,
     %                            mxtrcf, n1arcf, noarcf,
     %                            larmin, notrcf, nostbo,
     %                            ierr )
                     if( ierr .lt. 0 ) then
c                       le sommet ns est externe donc non supprime
c                       ou bien le sommet ns est le centre d'un cf dont toutes
c                       les aretes simples sont frontalieres
c                       dans les 2 cas le sommet ns n'est pas supprime
                        ierr = 0
                        goto 200
                     else if( ierr .eq. 0 ) then
                        nbstsu = nbstsu + 1
                        nbst5  = nbst5 + 1
                        goto 1000
                     else
c                       erreur irrecuperable
                        goto 9999
                     endif
                  endif
 80            continue
            endif
c
c           si la boule de ns contient au moins 8 triangles
c           alors un triangle interne est ajoute + 3 triangles (1 par arete)
c           ================================================================
            if( nbtrcf .ge. 8 ) then
c
c              modification des coordonnees du sommet ns
c              il devient le barycentre du triangle notrcf(1)
               call nusotr( notrcf(1), mosoar, nosoar,
     %                      moartr, noartr, nosotr )
               do 110 i=1,3
                  pxyd(i,ns) = ( pxyd(i,nosotr(1,1))
     %                         + pxyd(i,nosotr(2,1))
     %                         + pxyd(i,nosotr(3,1)) ) / 3d0
 110           continue
c
               if( nutysu .gt. 0 ) then
c                 la fonction taille_ideale(x,y,z) existe
c                 calcul de pxyzd(3,nbsomm) dans le repere initial => xyz(1:3)
                  call tetaid( nutysu, pxyd(1,ns), pxyd(2,ns),
     %                         pxyd(3,ns), ier )
               endif
c
c              ajout des 2 autres sommets comme barycentres des triangles
c              notrcf(1+nbtrcf/3) et notrcf(1+2*nbtrcf/3)
               nbt1 = ( nbtrcf + 1 ) / 3
               do 140 n=1,2
c
c                 le triangle traite
                  nt = notrcf(1 + n * nbt1 )
c
c                 le numero pxyd de ses 3 sommets
                  call nusotr( nt, mosoar, nosoar,
     %                         moartr, noartr, nosotr )
c
c                 ajout du nouveau barycentre
                  if( nbsomm .ge. mxsomm ) then
                     write(imprim,*) 'saturation du tableau pxyd'
c                    abandon de l'amelioration
                     goto 1100
                  endif
                  nbsomm = nbsomm + 1
                  do 120 i=1,3
                     pxyd(i,nbsomm) = ( pxyd(i,nosotr(1,1))
     %                                + pxyd(i,nosotr(2,1))
     %                                + pxyd(i,nosotr(3,1)) ) / 3d0
 120              continue
c
                  if( nutysu .gt. 0 ) then
c                    la fonction taille_ideale(x,y,z) existe
c                    calcul de pxyzd(3,nbsomm) dans le repere initial => xyz(1:3
                     call tetaid( nutysu, pxyd(1,nbsomm),pxyd(2,nbsomm),
     %                            pxyd(3,nbsomm), ier )
                  endif
c
c                 sommet interne a la triangulation
                  nslign(nbsomm) = 0
c
c                 les 3 aretes du triangle nt sont a rendre delaunay
                  do 130 i=1,3
                     noar = abs( noartr(i,nt) )
                     if( nosoar(3,noar) .eq. 0 ) then
c                       arete non frontaliere
                        if( nosoar(lchain,noar) .lt. 0 ) then
c                          arete non encore chainee
                           nosoar(lchain,noar) = noar0
                           noar0 = noar
                        endif
                     endif
 130              continue
c
c                 triangulation du triangle de barycentre nbsomm
c                 protection a ne pas modifier sinon erreur!
                  call tr3str( nbsomm, nt,
     %                         mosoar, mxsoar, n1soar, nosoar,
     %                         moartr, mxartr, n1artr, noartr,
     %                         noarst,
     %                         nosotr, ierr )
                  if( ierr .ne. 0 ) goto 9999
 140           continue
c
               nbst8  = nbst8 + 1
c
c              les aretes chainees de la boule sont rendues delaunay
               goto 300
c
            endif
c
c           nbtrcf est compris entre 5 et 7 => barycentrage simple
c           ======================================================
c           les 2 coordonnees du barycentre des sommets des aretes
c           simples de la boule du sommet ns
 200        xbar = xbar / nbstbo
            ybar = ybar / nbstbo
c
c           ponderation pour eviter les degenerescenses
            pxyd(1,ns) = ponde1 * pxyd(1,ns) + ponder * xbar
            pxyd(2,ns) = ponde1 * pxyd(2,ns) + ponder * ybar
c
c           les aretes chainees de la boule sont rendues delaunay
 300        call tedela( pxyd,   noarst,
     %                   mosoar, mxsoar, n1soar, nosoar, noar0,
     %                   moartr, mxartr, n1artr, noartr, modifs )
c
 1000    continue
c
c        trace de la triangulation actuelle et calcul de la qualite
 1100    continue
c
ccc         write(imprim,11000) nbst4, nbst5, nbst8
ccc11000 format( i7,' sommets de 4t',
ccc     %        i7,' sommets 5t+5t',
ccc     %        i7,' sommets >7t' )
c
c        mise a jour pour ne pas oublier les nouveaux sommets
         if( nbs1 .gt. nbs2 ) then
            nbs1 = nbsomm
            nbs2 = nbarpi + 1
         else
            nbs1 = nbarpi + 1
            nbs2 = nbsomm
         endif
c
 5000 continue
c
 9999 return
      end


      subroutine teamqt( nutysu,
     %                   noarst, mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, mxartr, n1artr, noartr,
     %                   mxarcf, notrcf, nostbo,
     %                   n1arcf, noarcf, larmin,
     %                   comxmi, nbarpi, nbsomm, mxsomm, pxyd, nslign,
     %                   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    amelioration de la qualite de la triangulation issue de teabr4
c -----
c
c entrees:
c --------
c nutysu : numero de traitement de areteideale() selon le type de surface
c          0 pas d'emploi de la fonction areteideale() => aretmx active
c          1 il existe une fonction areteideale()
c            dont seules les 2 premieres composantes de uv sont actives
c          autres options a definir...
c noarst : noarst(i) numero d'une arete de sommet i
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes frontalieres declarables
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxartr : nombre maximal de triangles declarables dans noartr
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c mxarcf : nombre maximal de triangles empilables
c nbarpi : numero du dernier sommet frontalier ou interne impose
c nslign : tableau du numero de sommet dans sa ligne pour chaque
c          sommet frontalier
c          numero du point dans le lexique point si interne impose
c          0 si le point est interne non impose par l'utilisateur
c         -1 si le sommet est externe au domaine
c comxmi : min et max des coordonneees des sommets du maillage
c
c modifies :
c ----------
c nbsomm : nombre actuel de sommets de la triangulation
c          (certains sommets internes ont ete desactives ou ajoutes)
c pxyd   : tableau des coordonnees 2d des points
c
c auxiliaires:
c ------------
c notrcf : tableau ( mxarcf ) auxiliaire d'entiers
c          numero dans noartr des triangles de sommet ns
c nostbo : tableau ( mxarcf ) auxiliaire d'entiers
c          numero dans pxyd des sommets des aretes simples de la boule
c n1arcf : tableau (0:mxarcf) auxiliaire d'entiers
c noarcf : tableau (3,mxarcf) auxiliaire d'entiers
c larmin : tableau ( mxarcf ) auxiliaire d'entiers
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       juin 1997
c....................................................................012
      common / unites / lecteu, imprim, nunite(30)
      double precision  pxyd(3,*), d2d3(3,3)
      integer           noartr(moartr,*),
     %                  nosoar(mosoar,*),
     %                  noarst(*),
     %                  notrcf(mxarcf),
     %                  nslign(*),
     %                  nostbo(mxarcf),
     %                  n1arcf(0:mxarcf),
     %                  noarcf(3,mxarcf),
     %                  larmin(mxarcf)
      double precision  comxmi(3,2)
c
c     suppression des sommets de triangles equilateraux trop proches
c     d'un sommet frontalier ou d'un point interne impose par
c     triangulation frontale de l'etoile et mise en delaunay
c     ==============================================================
      call tesusp( nbarpi, pxyd,   noarst,
     %             mosoar, mxsoar, n1soar, nosoar,
     %             moartr, mxartr, n1artr, noartr,
     %             mxarcf, n1arcf, noarcf, larmin, notrcf, nostbo,
     %             nbstsu, ierr )
      if( ierr .ne. 0 ) goto 9999
c      write(imprim,*) 'retrait de',nbstsu,
c     %                ' sommets de te trop proches de la frontiere'
c
c     ajustage des tailles moyennes des aretes avec ampli=1.34d0 entre
c     ampli/2 x taille_souhaitee et ampli x taille_souhaitee 
c     + barycentrage des sommets et mise en triangulation delaunay
c     ================================================================
      call teamqa( nutysu,
     %             noarst, mosoar, mxsoar, n1soar, nosoar,
     %             moartr, mxartr, n1artr, noartr,
     %             mxarcf, notrcf, nostbo,
     %             n1arcf, noarcf, larmin,
     %             comxmi, nbarpi, nbsomm, mxsomm, pxyd, nslign,
     %             ierr )
      if( ierr .ne. 0 ) goto 9999
c
c     modification de la topologie autour des sommets frontaliers
c     pour avoir un nombre de triangles egal a l'angle/60 degres
c     et mise en triangulation delaunay locale
c     ===========================================================
      call teamsf( nutysu,
     %             noarst, mosoar, mxsoar, n1soar, nosoar,
     %             moartr, mxartr, n1artr, noartr,
     %             mxarcf, notrcf, nostbo,
     %             n1arcf, noarcf, larmin,
     %             comxmi, nbarpi, nbsomm, mxsomm, pxyd, nslign,
     %             ierr )
      if( ierr .ne. 0 ) goto 9999
c
c     quelques iterations de barycentrage des points internes
c     modification de la topologie pour avoir 4 ou 5 ou 6 triangles
c     pour chaque sommet de la triangulation
c     et mise en triangulation delaunay
c     =============================================================
      call teamqs( nutysu,
     %             noarst, mosoar, mxsoar, n1soar, nosoar,
     %             moartr, mxartr, n1artr, noartr,
     %             mxarcf, notrcf, nostbo,
     %             n1arcf, noarcf, larmin,
     %             comxmi, nbarpi, nbsomm, mxsomm, pxyd, nslign,
     %             ierr )
c
 9999 return
      end

      subroutine trfrcf( nscent, mosoar, nosoar, moartr, noartr,
     %                   nbtrcf, notrcf, nbarfr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    calculer le nombre d'aretes simples du contour ferme des
c -----    nbtrcf triangles de numeros stockes dans le tableau notrcf
c          ayant tous le sommet nscent
c
c entrees:
c --------
c nscent : numero du sommet appartenant a tous les triangles notrcf
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c moartr : nombre maximal d'entiers par arete du tableau noartr
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c nbtrcf : >0 nombre de triangles empiles
c          =0       si impossible de tourner autour du point
c          =-nbtrcf si apres butee sur la frontiere il y a a nouveau
c          butee sur la frontiere . a ce stade on ne peut dire si tous
c          les triangles ayant ce sommet ont ete recenses
c          ce cas arrive seulement si le sommet est sur la frontiere
c notrcf : numero dans noartr des triangles de sommet ns
c
c sortie :
c --------
c nbarfr : nombre d'aretes simples frontalieres
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       juin 1997
c....................................................................012
      integer           noartr(moartr,*),
     %                  nosoar(mosoar,*),
     %                  notrcf(1:nbtrcf)
c
      nbarfr = 0
      do 50 n=1,nbtrcf
c        le numero du triangle n dans le tableau noartr
         nt = notrcf( n )
c        parcours des 3 aretes du triangle nt
         do 40 i=1,3
c           le numero de l'arete i dans le tableau nosoar
            noar = abs( noartr( i, nt ) )
            do 30 j=1,2
c              le numero du sommet j de l'arete noar
               ns = nosoar( j, noar )
               if( ns .eq. nscent ) goto 40
 30         continue
c           l'arete noar (sans sommet nscent) est elle frontaliere?
            if( nosoar( 5, noar ) .le. 0 ) then
c              l'arete appartient au plus a un triangle
c              une arete simple frontaliere de plus
               nbarfr = nbarfr + 1
            endif
c           le triangle a au plus une arete sans sommet nscent
            goto 50
 40      continue
 50   continue
      end

      subroutine int2ar( p1, p2, p3, p4, oui )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    les 2 aretes de r**2 p1-p2  p3-p4 s'intersectent elles
c -----    entre leurs sommets?
c
c entrees:
c --------
c p1,p2,p3,p4 : les 2 coordonnees reelles des sommets des 2 aretes
c
c sortie :
c --------
c oui    : .true. si intersection, .false. sinon
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc    octobre 1991
c2345x7..............................................................012
      double precision  p1(2),p2(2),p3(2),p4(2)
      double precision  x21,y21,d21,x43,y43,d43,d,x,y,xx
      logical  oui
c
c     longueur des aretes
      x21 = p2(1)-p1(1)
      y21 = p2(2)-p1(2)
      d21 = x21**2 + y21**2
c
      x43 = p4(1)-p3(1)
      y43 = p4(2)-p3(2)
      d43 = x43**2 + y43**2
c
c     les 2 aretes sont-elles jugees paralleles ?
      d = x43 * y21 - y43 * x21
      if( abs(d) .le. 0.001 * sqrt(d21 * d43) ) then
c        aretes paralleles . pas d'intersection
         oui = .false.
         return
      endif
c
c     les 2 coordonnees du point d'intersection
      x = ( p1(1)*x43*y21 - p3(1)*x21*y43 - (p1(2)-p3(2))*x21*x43 ) / d
      y =-( p1(2)*y43*x21 - p3(2)*y21*x43 - (p1(1)-p3(1))*y21*y43 ) / d
c
c     coordonnees de x,y dans le repere ns1-ns2
      xx  = ( x - p1(1) ) * x21 + ( y - p1(2) ) * y21
c     le point est il entre p1 et p2 ?
      oui = -0.00001d0*d21 .le. xx .and. xx .le. 1.00001d0*d21
c
c     coordonnees de x,y dans le repere ns3-ns4
      xx  = ( x - p3(1) ) * x43 + ( y - p3(2) ) * y43
c     le point est il entre p3 et p4 ?
      oui = oui .and. -0.00001d0*d43 .le. xx .and. xx .le. 1.00001d0*d43
      end


      subroutine trchtd( pxyd,   nar00, nar0,  noarcf,
     %                   namin0, namin, larmin )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    recherche dans le contour ferme du sommet qui joint a la plus
c -----    courte arete nar00 donne le triangle sans intersection
c          avec le contour ferme de meilleure qualite
c
c entrees:
c --------
c pxyd   : tableau des coordonnees des sommets et distance_souhaitee
c
c entrees et sorties:
c -------------------
c nar00  : numero dans noarcf de l'arete avant nar0
c nar0   : numero dans noarcf de la plus petite arete du contour ferme
c          a joindre a noarcf(1,namin) pour former le triangle ideal
c noarcf : numero du sommet , numero de l'arete suivante
c          numero du triangle exterieur a l'etoile
c
c sortie :
c --------
c namin0 : numero dans noarcf de l'arete avant namin
c namin  : numero dans noarcf du sommet choisi
c          0 si contour ferme reduit a moins de 3 aretes
c larmin : tableau auxiliaire pour stocker la liste des numeros des
c          aretes de meilleure qualite pour faire le choix final
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc    fevrier 1992
c2345x7..............................................................012
      double precision dmaxim, precision
      parameter        (dmaxim=1.7d+308, precision=1d-16)
c     ATTENTION:variables a ajuster selon la machine!
c     ATTENTION:dmaxim : le plus grand reel machine
c     ATTENTION:sur dec-alpha la precision est de 10**-14 seulement

      common / unites / lecteu,imprim,nunite(30)
      double precision  pxyd(1:3,1:*)
      integer           noarcf(1:3,1:*),
     %                  larmin(1:*)
      double precision  q, dd, dmima,
     %                  unpeps, rayon, surtd2
      logical           oui
      double precision  centre(3)
c
c     initialisations
c     dmaxim : le plus grand reel machine
      unpeps = 1d0 + 100d0 * precision
c
c     recherche de la plus courte arete du contour ferme
      nbmin = 0
      na00  = nar00
      dmima = dmaxim
      nbar  = 0
c
 2    na0  = noarcf( 2, na00 )
      na1  = noarcf( 2, na0  )
      nbar = nbar + 1
c     les 2 sommets de l'arete na0 du cf
      ns1  = noarcf( 1, na0 )
      ns2  = noarcf( 1, na1 )
      dd   = (pxyd(1,ns2)-pxyd(1,ns1))**2 + (pxyd(2,ns2)-pxyd(2,ns1))**2
      if( dd .lt. dmima ) then
         dmima = dd
         larmin(1) = na00
      endif
      na00 = na0
      if( na00 .ne. nar00 ) then
c        derniere arete non atteinte
         goto 2
      endif
c
      if( nbar .eq. 3 ) then
c
c        contour ferme reduit a un triangle
c        ----------------------------------
         namin  = nar00
         nar0   = noarcf( 2, nar00 )
         namin0 = noarcf( 2, nar0  )
         return
c
      else if( nbar .le. 2 ) then
         write(imprim,*) 'erreur trchtd: cf<3 aretes'
         namin  = 0
         namin0 = 0
         return
      endif
c
c     cf non reduit a un triangle
c     la plus petite arete est nar0 dans noarcf
      nar00 = larmin( 1 )
      nar0  = noarcf( 2, nar00 )
      nar   = noarcf( 2, nar0  )
c
      ns1   = noarcf( 1, nar0 )
      ns2   = noarcf( 1, nar  )
c
c     recherche dans cette etoile du sommet offrant la meilleure qualite
c     du triangle ns1-ns2 ns3 sans intersection avec le contour ferme
c     ==================================================================
      nar3  = nar
      qmima = -1
c
c     parcours des sommets possibles ns3
 10   nar3  = noarcf( 2, nar3 )
      if( nar3 .ne. nar0 ) then
c
c        il existe un sommet ns3 different de ns1 et ns2
         ns3 = noarcf( 1, nar3 )
c
c        les aretes ns1-ns3 et ns2-ns3 intersectent-elles une arete
c        du contour ferme ?
c        ----------------------------------------------------------
c        intersection de l'arete ns2-ns3 et des aretes du cf
c        jusqu'au sommet ns3
         nar1 = noarcf( 2, nar )
c
 15      if( nar1 .ne. nar3 .and. noarcf( 2, nar1 ) .ne. nar3 ) then
c           l'arete suivante
            nar2 = noarcf( 2, nar1 )
c           le numero des 2 sommets de l'arete
            np1  = noarcf( 1, nar1 )
            np2  = noarcf( 1, nar2 )
            call int2ar( pxyd(1,ns2), pxyd(1,ns3),
     %                   pxyd(1,np1), pxyd(1,np2), oui )
            if( oui ) goto 10
c           les 2 aretes ne s'intersectent pas entre leurs sommets
            nar1 = nar2
            goto 15
         endif
c
c        intersection de l'arete ns3-ns1 et des aretes du cf
c        jusqu'au sommet de l'arete nar0
         nar1 = noarcf( 2, nar3 )
c
 18      if( nar1 .ne. nar0 .and. noarcf( 2, nar1 ) .ne. nar0 ) then
c           l'arete suivante
            nar2 = noarcf( 2, nar1 )
c           le numero des 2 sommets de l'arete
            np1  = noarcf( 1, nar1 )
            np2  = noarcf( 1, nar2 )
            call int2ar( pxyd(1,ns1), pxyd(1,ns3),
     %                   pxyd(1,np1), pxyd(1,np2), oui )
            if( oui ) goto 10
c           les 2 aretes ne s'intersectent pas entre leurs sommets
            nar1 = nar2
            goto 18
         endif
c
c        le triangle ns1-ns2-ns3 n'intersecte pas une arete du contour ferme
c        le calcul de la surface du triangle
         dd = surtd2( pxyd(1,ns1), pxyd(1,ns2), pxyd(1,ns3) )
         if( dd .le. 0d0 ) then
c           surface negative => triangle a rejeter
            q = 0
         else
c           calcul de la qualite du  triangle  ns1-ns2-ns3
            call qutr2d( pxyd(1,ns1), pxyd(1,ns2), pxyd(1,ns3), q )
         endif
c
         if( q .ge. qmima*1.00001 ) then
c           q est un vrai maximum de la qualite
            qmima = q
            nbmin = 1
            larmin(1) = nar3
         else if( q .ge. qmima*0.999998 ) then
c           q est voisin de qmima
c           il est empile
            nbmin = nbmin + 1
            larmin( nbmin ) = nar3
         endif
         goto 10
      endif
c
c     bilan : existe t il plusieurs sommets de meme qualite?
c     ======================================================
      if( nbmin .gt. 1 ) then
c
c        oui:recherche de ceux de cercle ne contenant pas d'autres sommets
         do 80 i=1,nbmin
c           le sommet
            nar = larmin( i )
            if( nar .le. 0 ) goto 80
            ns3 = noarcf(1,nar)
c           les coordonnees du centre du cercle circonscrit
c           et son rayon
            ier = -1
            call cenced( pxyd(1,ns1), pxyd(1,ns2), pxyd(1,ns3),
     %                   centre, ier )
            if( ier .ne. 0 ) then
c              le sommet ns3 ne convient pas
               larmin( i ) = 0
               goto 80
            endif
            rayon = centre(3) * unpeps
            do 70 j=1,nbmin
               if( j .ne. i ) then
c                 l'autre sommet
                  nar1 = larmin(j)
                  if( nar1 .le. 0 ) goto 70
                  ns4 = noarcf(1,nar1)
c                 appartient t il au cercle ns1 ns2 ns3 ?
                  dd = (centre(1)-pxyd(1,ns4))**2 +
     %                 (centre(2)-pxyd(2,ns4))**2
                  if( dd .le. rayon ) then
c                    ns4 est dans le cercle circonscrit  ns1 ns2 ns3
c                    le sommet ns3 ne convient pas
                     larmin( i ) = 0
                     goto 80
                  endif
               endif
 70         continue
 80      continue
c
c        existe t il plusieurs sommets ?
         j = 0
         do 90 i=1,nbmin
            if( larmin( i ) .gt. 0 ) then
c              compactage des min
               j = j + 1
               larmin(j) = larmin(i)
            endif
 90      continue
c
         if( j .gt. 1 ) then
c           oui : choix du plus petit rayon de cercle circonscrit
            dmima = dmaxim
            do 120 i=1,nbmin
               ns3 = noarcf(1,larmin(i))
c
c              les coordonnees du centre de cercle circonscrit
c              au triangle nt et son rayon
               ier = -1
               call cenced( pxyd(1,ns1), pxyd(1,ns2), pxyd(1,ns3),
     %                      centre, ier )
               if( ier .ne. 0 ) then
c                 le sommet ns3 ne convient pas
                  goto 120
               endif
               rayon = sqrt( centre(3) )
               if( rayon .lt. dmima ) then
                  dmima = rayon
                  larmin(1) = larmin(i)
               endif
 120        continue
         endif
      endif
c
c     le choix final
c     ==============
      namin = larmin(1)
c
c     recherche de l'arete avant namin ( nar0 <> namin )
c     ==================================================
      nar1 = nar0
 200  if( nar1 .ne. namin ) then
         namin0 = nar1
         nar1   = noarcf( 2, nar1 )
         goto 200
      endif
      end

      subroutine trcf0a( nbcf,   na01,   na1, na2, na3,
     %                   noar1,  noar2,  noar3,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, n1artr, noartr, noarst,
     %                   mxarcf, n1arcf, noarcf, nt )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    modification de la triangulation du contour ferme nbcf
c -----    par ajout d'un triangle ayant 0 arete sur le contour
c          creation des 3 aretes dans le tableau nosoar
c          modification du contour par ajout de la 3-eme arete
c          creation d'un contour ferme a partir de la seconde arete
c
c entrees:
c --------
c nbcf    : numero dans n1arcf du cf traite ici
c na01    : numero noarcf de l'arete precedent l'arete na1 de noarcf
c na1     : numero noarcf du 1-er sommet du triangle
c           implicitement l'arete na1 n'est pas une arete du triangle
c na2     : numero noarcf du 2-eme sommet du triangle
c           implicitement l'arete na1 n'est pas une arete du triangle
c na3     : numero noarcf du 3-eme sommet du triangle
c           implicitement l'arete na1 n'est pas une arete du triangle
c
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c          attention: mxsoar>3*mxsomm obligatoire!
c moartr : nombre maximal d'entiers par arete du tableau noartr
c
c entrees et sorties :
c --------------------
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c          une arete i de nosoar est vide  <=>  nosoar(1,i)=0
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = nosoar(1)+nosoar(2)*2
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c
c noarst : noarst(i) numero d'une arete de sommet i
c n1arcf : numero d'une arete de chaque contour
c noarcf : numero des aretes de la ligne du contour ferme
c          attention : chainage circulaire des aretes
c
c sortie :
c --------
c noar1  : numero dans le tableau nosoar de l'arete 1 du triangle
c noar2  : numero dans le tableau nosoar de l'arete 2 du triangle
c noar3  : numero dans le tableau nosoar de l'arete 3 du triangle
c nt     : numero du triangle ajoute dans noartr
c          0 si saturation du tableau noartr ou noarcf ou n1arcf
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c2345x7..............................................................012
      common / unites / lecteu, imprim, nunite(30)
      integer           nosoar(mosoar,*),
     %                  noartr(moartr,*),
     %                  noarst(*),
     %                  n1arcf(0:*),
     %                  noarcf(3,*)
c
      ierr = 0
c
c     2 contours fermes peuvent ils etre ajoutes ?
      if( nbcf+2 .gt. mxarcf ) goto 9100
c
c     creation des 3 aretes du triangle dans le tableau nosoar
c     ========================================================
c     la formation de l'arete sommet1-sommet2 dans le tableau nosoar
      call fasoar( noarcf(1,na1), noarcf(1,na2), -1, -1,  0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             noar1,  ierr )
      if( ierr .ne. 0 ) goto 9900
c
c     la formation de l'arete sommet2-sommet3 dans le tableau nosoar
      call fasoar( noarcf(1,na2), noarcf(1,na3), -1, -1,  0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             noar2,  ierr )
      if( ierr .ne. 0 ) goto 9900
c
c     la formation de l'arete sommet3-sommet1 dans le tableau nosoar
      call fasoar( noarcf(1,na3), noarcf(1,na1), -1, -1,  0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             noar3,  ierr )
      if( ierr .ne. 0 ) goto 9900
c
c     ajout dans noartr de ce triangle nt
c     ===================================
      call trcf3a( noarcf(1,na1),  noarcf(1,na2), noarcf(1,na3),
     %             noar1,  noar2,  noar3,
     %             mosoar, nosoar,
     %             moartr, n1artr, noartr,
     %             nt )
      if( nt .le. 0 ) return
c
c     modification du contour nbcf existant
c     chainage de l'arete na2 vers l'arete na1
c     ========================================
c     modification du cf en pointant na2 sur na1
      na2s = noarcf( 2, na2 )
      noarcf( 2, na2 ) = na1
c     le numero de l'arete dans le tableau nosoar
      noar2s = noarcf( 3, na2 )
c     le numero de l'arete dans le tableau nosoar
      noarcf( 3, na2 ) = noar1
c     debut du cf
      n1arcf( nbcf ) = na2
c
c     creation d'un nouveau contour ferme na2 - na3
c     =============================================
      nbcf = nbcf + 1
c     recherche d'une arete de cf vide
      nav = n1arcf(0)
      if( nav .le. 0 ) goto 9100
c     la 1-ere arete vide est mise a jour
      n1arcf(0) = noarcf( 2, nav )
c
c     ajout de l'arete nav pointant sur na2s
c     le numero du sommet
      noarcf( 1, nav ) = noarcf( 1, na2 )
c     l'arete suivante
      noarcf( 2, nav ) = na2s
c     le numero nosoar de cette arete
      noarcf( 3, nav ) = noar2s
c
c     l'arete na3 se referme sur nav
      na3s = noarcf( 2, na3 )
      noarcf( 2, na3 ) = nav
c     le numero de l'arete dans le tableau nosoar
      noar3s = noarcf( 3, na3 )
      noarcf( 3, na3 ) = noar2
c     debut du cf+1
      n1arcf( nbcf ) = na3
c
c     creation d'un nouveau contour ferme na3 - na1
c     =============================================
      nbcf = nbcf + 1
c     recherche d'une arete de cf vide
      nav = n1arcf(0)
      if( nav .le. 0 ) goto 9100
c     la 1-ere arete vide est mise a jour
      n1arcf(0) = noarcf( 2, nav )
c
c     ajout de l'arete nav pointant sur na3s
c     le numero du sommet
      noarcf( 1, nav ) = noarcf( 1, na3 )
c     l'arete suivante
      noarcf( 2, nav ) = na3s
c     le numero de l'arete dans le tableau nosoar
      noarcf( 3, nav ) = noar3s
c
c     recherche d'une arete de cf vide
      nav1 = n1arcf(0)
      if( nav1 .le. 0 ) goto 9100
c     la 1-ere arete vide est mise a jour
      n1arcf(0) = noarcf( 2, nav1 )
c
c     l'arete precedente na01 de na1 pointe sur la nouvelle nav1
      noarcf( 2, na01 ) = nav1
c
c     ajout de l'arete nav1 pointant sur nav
c     le numero du sommet
      noarcf( 1, nav1 ) = noarcf( 1, na1 )
c     l'arete suivante
      noarcf( 2, nav1 ) = nav
c     le numero de l'arete dans le tableau nosoar
      noarcf( 3, nav1 ) = noar3
c
c     debut du cf+2
      n1arcf( nbcf ) = nav1
      return
c
c     erreur
 9100 write(imprim,*) 'saturation du tableau mxarcf'
      nt = 0
      return
c
c     erreur tableau nosoar sature
 9900 write(imprim,*) 'saturation du tableau nosoar'
      nt = 0
      return
      end


      subroutine trcf1a( nbcf,   na01,   na1,    na2, noar1, noar3,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, n1artr, noartr, noarst,
     %                   mxarcf, n1arcf, noarcf, nt )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    modification de la triangulation du contour ferme nbcf
c -----    par ajout d'un triangle ayant 1 arete sur le contour
c          modification du contour par ajout de la 3-eme arete
c          creation d'un contour ferme a partir de la seconde arete
c
c entrees:
c --------
c nbcf    : numero dans n1arcf du cf traite ici
c na01    : numero noarcf de l'arete precedant l'arete na1 de noarcf
c na1     : numero noarcf du 1-er sommet du triangle
c           implicitement l'arete na1 n'est pas une arete du triangle
c na2     : numero noarcf du 2-eme sommet du triangle
c           cette arete est l'arete 2 du triangle a ajouter
c           son arete suivante dans noarcf n'est pas sur le contour
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c          attention: mxsoar>3*mxsomm obligatoire!
c moartr : nombre maximal d'entiers par arete du tableau noartr
c
c entrees et sorties :
c --------------------
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c          une arete i de nosoar est vide  <=>  nosoar(1,i)=0
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = nosoar(1)+nosoar(2)*2
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c
c noarst : noarst(i) numero d'une arete de sommet i
c n1arcf : numero d'une arete de chaque contour
c noarcf : numero des aretes de la ligne du contour ferme
c          attention : chainage circulaire des aretes
c
c sortie :
c --------
c noar1  : numero nosoar de l'arete 1 du triangle cree
c noar3  : numero nosoar de l'arete 3 du triangle cree
c nt     : numero du triangle ajoute dans notria
c          0 si saturation du tableau notria ou noarcf ou n1arcf
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c2345x7..............................................................012
      common / unites / lecteu, imprim, nunite(30)
      integer           nosoar(mosoar,mxsoar),
     %                  noartr(moartr,*),
     %                  noarst(*),
     %                  n1arcf(0:*),
     %                  noarcf(3,*)
c
c     un cf supplementaire peut il etre ajoute ?
      if( nbcf .ge. mxarcf ) then
         write(imprim,*) 'saturation du tableau noarcf'
         nt = 0
         return
      endif
c
      ierr = 0
c
c     l' arete suivante du triangle non sur le cf
      na3 = noarcf( 2, na2 )
c
c     creation des 2 nouvelles aretes du triangle dans le tableau nosoar
c     ==================================================================
c     la formation de l'arete sommet1-sommet2 dans le tableau nosoar
      call fasoar( noarcf(1,na1), noarcf(1,na2), -1, -1,  0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             noar1,  ierr )
      if( ierr .ne. 0 ) goto 9900
c
c     la formation de l'arete sommet1-sommet3 dans le tableau nosoar
      call fasoar( noarcf(1,na3), noarcf(1,na1), -1, -1,  0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             noar3,  ierr )
      if( ierr .ne. 0 ) goto 9900
c
c     le triangle nt de noartr a l'arete 2 comme arete du contour na2
c     ===============================================================
      call trcf3a( noarcf(1,na1), noarcf(1,na2), noarcf(1,na3),
     %             noar1, noarcf(3,na2), noar3,
     %             mosoar, nosoar,
     %             moartr, n1artr, noartr,
     %             nt )
      if( nt .le. 0 ) return
c
c     modification du contour ferme existant
c     suppression de l'arete na2 du cf
c     ======================================
c     modification du cf en pointant na2 sur na1
      noarcf( 2, na2 ) = na1
      noarcf( 3, na2 ) = noar1
c     debut du cf
      n1arcf( nbcf ) = na2
c
c     creation d'un nouveau contour ferme na3 - na1
c     =============================================
      nbcf = nbcf + 1
c
c     recherche d'une arete de cf vide
      nav = n1arcf(0)
      if( nav .le. 0 ) then
         write(imprim,*) 'saturation du tableau noarcf'
         nt = 0
         return
      endif
c
c     la 1-ere arete vide est mise a jour
      n1arcf(0) = noarcf( 2, nav )
c
c     ajout de l'arete nav pointant sur na3
c     le numero du sommet
      noarcf( 1, nav ) = noarcf( 1, na1 )
c     l'arete suivante
      noarcf( 2, nav ) = na3
c     le numero de l'arete dans le tableau nosoar
      noarcf( 3, nav ) = noar3
c
c     l'arete precedente na01 de na1 pointe sur la nouvelle nav
      noarcf( 2, na01 ) = nav
c
c     debut du cf
      n1arcf( nbcf ) = nav
      return
c
c     erreur tableau nosoar sature
 9900 write(imprim,*) 'saturation du tableau nosoar'
      nt = 0
      return
      end


      subroutine trcf2a( nbcf,   na1,    noar3,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, n1artr, noartr, noarst,
     %                   n1arcf, noarcf, nt )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    modification de la triangulation du contour ferme nbcf
c -----    par ajout d'un triangle ayant 2 aretes sur le contour
c          creation d'une arete dans nosoar (sommet3-sommet1)
c          et modification du contour par ajout de la 3-eme arete
c
c entrees:
c --------
c nbcf   : numero dans n1arcf du cf traite ici
c na1    : numero noarcf de la premiere arete sur le contour
c          implicitement sa suivante est sur le contour
c          la suivante de la suivante n'est pas sur le contour
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c          attention: mxsoar>3*mxsomm obligatoire!
c moartr : nombre maximal d'entiers par arete du tableau noartr
c
c entrees et sorties :
c --------------------
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c          une arete i de nosoar est vide  <=>  nosoar(1,i)=0
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = nosoar(1)+nosoar(2)*2
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c
c noarst : noarst(i) numero d'une arete de sommet i
c n1arcf : numero d'une arete de chaque contour
c noarcf : numero des aretes de la ligne du contour ferme
c          attention : chainage circulaire des aretes
c
c sortie :
c --------
c noar3  : numero de l'arete 3 dans le tableau nosoar
c nt     : numero du triangle ajoute dans noartr
c          0 si saturation du tableau noartr ou nosoar
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c2345x7..............................................................012
      common / unites / lecteu, imprim, nunite(30)
      integer           nosoar(mosoar,*),
     %                  noartr(moartr,*),
     %                  noarst(*)
      integer           n1arcf(0:*),
     %                  noarcf(3,*)
c
      ierr = 0
c
c     l'arete suivante de l'arete na1 dans noarcf
      na2 = noarcf( 2, na1 )
c     l'arete suivante de l'arete na2 dans noarcf
      na3 = noarcf( 2, na2 )
c
c     la formation de l'arete sommet3-sommet1 dans le tableau nosoar
      call fasoar( noarcf(1,na3), noarcf(1,na1), -1, -1,  0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             noar3,  ierr )
      if( ierr .ne. 0 ) then
         if( ierr .eq. 1 ) then
            write(imprim,*) 'saturation des aretes (tableau nosoar)'
         endif
         nt = 0
         return
      endif
c
c     le triangle a ses 2 aretes na1 na2 sur le contour ferme
c     ajout dans noartr de ce triangle nt
      call trcf3a( noarcf(1,na1), noarcf(1,na2), noarcf(1,na3),
     %             noarcf(3,na1), noarcf(3,na2), noar3,
     %             mosoar, nosoar,
     %             moartr, n1artr, noartr,
     %             nt )
      if( nt .le. 0 ) return
c
c     suppression des 2 aretes (na1 na2) du cf
c     ces 2 aretes se suivent dans le chainage du cf
c     ajout de la 3-eme arete  (noar3) dans le cf
c     l'arete suivante de na1 devient la suivante de na2
      noarcf(2,na1) = na3
      noarcf(3,na1) = noar3
c
c     l'arete na2 devient vide dans noarcf
      noarcf(2,na2) = n1arcf( 0 )
      n1arcf( 0 )   = na2
c
c     la premiere pointee dans noarcf est na1
c     chainage circulaire => ce peut etre n'importe laquelle
      n1arcf(nbcf) = na1
      end


      subroutine trcf3a( ns1,    ns2,    ns3,
     %                   noar1,  noar2,  noar3,
     %                   mosoar, nosoar,
     %                   moartr, n1artr, noartr,
     %                   nt )
c++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    ajouter dans le tableau noartr le triangle
c -----    de sommets ns1   ns2   ns3
c          d'aretes   noar1 noar2 noar3 deja existantes
c                     dans le tableau nosoar des aretes
c
c entrees:
c --------
c ns1,  ns2,  ns3   : le numero dans pxyd   des 3 sommets du triangle
c noar1,noar2,noar3 : le numero dans nosoar des 3 aretes  du triangle
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c          attention: mxsoar>3*mxsomm obligatoire!
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxartr : nombre maximal de triangles stockables dans le tableau noartr
c
c modifies :
c ----------
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = nosoar(1)+nosoar(2)*2
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c
c sorties:
c --------
c nt     : numero dans noartr du triangle ajoute
c          =0 si le tableau noartr est sature
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
      common / unites / lecteu,imprim,nunite(30)
      integer           nosoar(mosoar,*),
     %                  noartr(moartr,*)
c
c     recherche d'un triangle libre dans le tableau noartr
      if( n1artr .le. 0 ) then
         write(imprim,*) 'saturation du tableau noartr des aretes'
         nt = 0
         return
      endif
c
c     le numero dans noartr du nouveau triangle
      nt = n1artr
c
c     le nouveau premier triangle vide dans le tableau noartr
      n1artr = noartr(2,n1artr)
c
c     arete 1 du triangle nt
c     ======================
c     orientation des 3 aretes du triangle pour qu'il soit direct
      if( ns1 .eq. nosoar(1,noar1) ) then
         n =  1
      else
         n = -1
      endif
c     le numero de l'arete 1 du triangle nt
      noartr(1,nt) = n * noar1
c
c     le numero du triangle nt pour l'arete
      if( nosoar(4,noar1) .le. 0 ) then
         n = 4
      else
         n = 5
      endif
      nosoar(n,noar1) = nt
c
c     arete 2 du triangle nt
c     ======================
c     orientation des 3 aretes du triangle pour qu'il soit direct
      if( ns2 .eq. nosoar(1,noar2) ) then
         n =  1
      else
         n = -1
      endif
c     le numero de l'arete 2 du triangle nt
      noartr(2,nt) = n * noar2
c
c     le numero du triangle nt pour l'arete
      if( nosoar(4,noar2) .le. 0 ) then
         n = 4
      else
         n = 5
      endif
      nosoar(n,noar2) = nt
c
c     arete 3 du triangle nt
c     ======================
c     orientation des 3 aretes du triangle pour qu'il soit direct
      if( ns3 .eq. nosoar(1,noar3) ) then
         n =  1
      else
         n = -1
      endif
c     le numero de l'arete 3 du triangle nt
      noartr(3,nt) = n * noar3
c
c     le numero du triangle nt pour l'arete
      if( nosoar(4,noar3) .le. 0 ) then
         n = 4
      else
         n = 5
      endif
      nosoar(n,noar3) = nt
      end



      subroutine trcf3s( nbcf,   na01,   na1,    na02,  na2, na03, na3,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, n1artr, noartr, noarst,
     %                   mxarcf, n1arcf, noarcf, nt )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :     ajout d'un triangle d'aretes na1 2 3 du tableau noarcf
c -----     a la triangulation d'un contour ferme (cf)
c
c entrees:
c --------
c nbcf    : numero dans n1arcf du cf traite ici
c           mais aussi nombre actuel de cf avant ajout du triangle
c na01    : numero noarcf de l'arete precedent l'arete na1 de noarcf
c na1     : numero noarcf du 1-er sommet du triangle
c na02    : numero noarcf de l'arete precedent l'arete na2 de noarcf
c na2     : numero noarcf du 2-eme sommet du triangle
c na03    : numero noarcf de l'arete precedent l'arete na3 de noarcf
c na3     : numero noarcf du 3-eme sommet du triangle
c
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c          attention: mxsoar>3*mxsomm obligatoire!
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxarcf : nombre maximal d'aretes declarables dans noarcf, n1arcf
c
c modifies:
c ---------
c n1soar : no de l'eventuelle premiere arete libre dans le tableau nosoar
c          chainage des vides suivant en 3 et precedant en 2 de nosoar
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = nosoar(1)+nosoar(2)*2
c          avec mxsoar>=3*mxsomm
c          une arete i de nosoar est vide <=> nosoar(1,i)=0 et
c          nosoar(2,arete vide)=l'arete vide qui precede
c          nosoar(3,arete vide)=l'arete vide qui suit
c
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c noarst : noarst(i) numero d'une arete de sommet i
c
c n1arcf : numero d'une arete de chaque contour ferme
c noarcf : numero du sommet , numero de l'arete suivante
c          numero de l'arete dans le tableau nosoar
c          attention : chainage circulaire des aretes
c
c sortie :
c --------
c nbcf   : nombre actuel de cf apres ajout du triangle
c nt     : numero du triangle ajoute dans noartr
c          0 si saturation du tableau nosoar ou noartr ou noarcf ou n1arcf
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c2345x7..............................................................012
      integer        nosoar(mosoar,*),
     %               noartr(moartr,*),
     %               noarst(*),
     %               n1arcf(0:mxarcf),
     %               noarcf(3,mxarcf)
c
c     combien y a t il d'aretes nbascf sur le cf ?
c     ============================================
c     la premiere arete est elle sur le cf?
      if( noarcf(2,na1) .eq. na2 ) then
c        la 1-ere arete est sur le cf
         na1cf  = 1
      else
c        la 1-ere arete n'est pas sur le cf
         na1cf  = 0
      endif
c
c     la seconde arete est elle sur le cf?
      if( noarcf(2,na2) .eq. na3 ) then
c        la 2-eme arete est sur le cf
         na2cf = 1
      else
         na2cf = 0
      endif
c
c     la troisieme arete est elle sur le cf?
      if( noarcf(2,na3) .eq. na1 ) then
c        la 3-eme arete est sur le cf
         na3cf = 1
      else
         na3cf = 0
      endif
c
c     le nombre d'aretes sur le cf
      nbascf = na1cf + na2cf + na3cf
c
c     traitement selon le nombre d'aretes sur le cf
c     =============================================
      if( nbascf .eq. 3 ) then
c
c        le contour ferme se reduit a un triangle avec 3 aretes sur le cf
c        ----------------------------------------------------------------
c        ajout dans noartr de ce nouveau triangle
         call trcf3a( noarcf(1,na1), noarcf(1,na2), noarcf(1,na3),
     %                noarcf(3,na1), noarcf(3,na2), noarcf(3,na3),
     %                mosoar, nosoar,
     %                moartr, n1artr, noartr,
     %                nt )
         if( nt .le. 0 ) return
c
c        le cf est supprime et chaine vide
         noarcf(2,na3) = n1arcf(0)
         n1arcf( 0 )   = na1
c
c        ce cf a ete traite => un cf de moins a traiter
         nbcf = nbcf - 1
c
      else if( nbascf .eq. 2 ) then
c
c        le triangle a 2 aretes sur le contour
c        -------------------------------------
c        les 2 aretes sont la 1-ere et 2-eme du triangle
         if( na1cf .eq. 0 ) then
c           l'arete 1 n'est pas sur le cf
            naa1 = na2
         else if( na2cf .eq. 0 ) then
c           l'arete 2 n'est pas sur le cf
            naa1 = na3
         else
c           l'arete 3 n'est pas sur le cf
            naa1 = na1
         endif
c        le triangle oppose a l'arete 3 est inconnu
c        modification du contour apres integration du
c        triangle ayant ses 2-eres aretes sur le cf
         call trcf2a( nbcf,   naa1,   naor3,
     %                mosoar, mxsoar, n1soar, nosoar,
     %                moartr, n1artr, noartr, noarst,
     %                n1arcf, noarcf, nt )
c
      else if( nbascf .eq. 1 ) then
c
c        le triangle a 1 arete sur le contour
c        ------------------------------------
c        cette arete est la seconde du triangle
         if( na3cf .ne. 0 ) then
c           l'arete 3 est sur le cf
            naa01 = na02
            naa1  = na2
            naa2  = na3
         else if( na1cf .ne. 0 ) then
c           l'arete 1 est sur le cf
            naa01 = na03
            naa1  = na3
            naa2  = na1
         else
c           l'arete 2 est sur le cf
            naa01 = na01
            naa1  = na1
            naa2  = na2
         endif
c        le triangle oppose a l'arete 1 et 3 est inconnu
c        modification du contour apres integration du
c        triangle ayant 1 arete sur le cf avec creation
c        d'un nouveau contour ferme
         call trcf1a( nbcf, naa01, naa1, naa2, naor1, naor3,
     %                mosoar, mxsoar, n1soar, nosoar,
     %                moartr, n1artr, noartr, noarst,
     %                mxarcf, n1arcf, noarcf, nt )
c
      else
c
c        le triangle a 0 arete sur le contour
c        ------------------------------------
c        modification du contour apres integration du
c        triangle ayant 0 arete sur le cf avec creation
c        de 2 nouveaux contours fermes
         call trcf0a( nbcf, na01,  na1, na2, na3,
     %                naa1, naa2, naa01,
     %                mosoar, mxsoar, n1soar, nosoar,
     %                moartr, n1artr, noartr, noarst,
     %                mxarcf, n1arcf, noarcf, nt )
      endif
      end


      subroutine tridcf( nbcf0,  pxyd,   noarst,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, n1artr, noartr,
     %                   mxarcf, n1arcf, noarcf, larmin,
     %                   nbtrcf, notrcf, ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    triangulation directe de nbcf0 contours fermes (cf)
c -----    definis par la liste circulaire de leurs aretes peripheriques
c
c entrees:
c --------
c nbcf0  : nombre initial de cf a trianguler
c pxyd   : tableau des coordonnees 2d des points
c          par point : x  y  distance_souhaitee
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c          attention: mxsoar>3*mxsomm obligatoire!
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxarcf  : nombre maximal d'aretes declarables dans noarcf, n1arcf, larmin, not
c
c modifies:
c ---------
c noarst : noarst(i) numero d'une arete de sommet i
c n1soar : no de l'eventuelle premiere arete libre dans le tableau nosoar
c          chainage des vides suivant en 3 et precedant en 2 de nosoar
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = nosoar(1)+nosoar(2)*2
c          avec mxsoar>=3*mxsomm
c          une arete i de nosoar est vide <=> nosoar(1,i)=0 et
c          nosoar(2,arete vide)=l'arete vide qui precede
c          nosoar(3,arete vide)=l'arete vide qui suit
c
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c
c n1arcf : numero de la premiere arete de chacun des nbcf0 cf
c          n1arcf(0)   no de la premiere arete vide du tableau noarcf
c          noarcf(2,i) no de l'arete suivante
c noarcf : numero du sommet , numero de l'arete suivante du cf
c          numero de l'arete dans le tableau nosoar
c
c auxiliaires :
c -------------
c larmin : tableau (mxarcf)   auxiliaire
c          stocker la liste des numeros des meilleures aretes
c          lors de la selection du meilleur sommet du cf a trianguler
c          cf le sp trchtd
c
c sortie :
c --------
c nbtrcf : nombre de  triangles des nbcf0 cf
c notrcf : numero des triangles des nbcf0 cf dans le tableau noartr
c ierr   : 0 si pas d'erreur
c          2 saturation de l'un des des tableaux nosoar, noartr, ...
c          3 si contour ferme reduit a moins de 3 aretes
c          4 saturation du tableau notrcf
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
      common / unites / lecteu, imprim, nunite(30)
      double precision  pxyd(3,*)
      integer           noartr(moartr,*),
     %                  nosoar(mosoar,mxsoar),
     %                  noarst(*),
     %                  n1arcf(0:mxarcf),
     %                  noarcf(3,mxarcf),
     %                  larmin(mxarcf),
     %                  notrcf(mxarcf)
c
ccc      integer           nosotr(3)
ccc      double precision  d, surtd2
c
c     depart avec nbcf0 cf a trianguler
      nbcf   = nbcf0
c
c     le nombre de triangles formes dans l'ensemble des cf
      nbtrcf = 0
c
c     tant qu'il existe un cf a trianguler faire
c     la triangulation directe du cf
c     ==========================================
 10   if( nbcf .gt. 0 ) then
c
c        le cf en haut de pile a pour premiere arete
         na01 = n1arcf( nbcf )
         na1  = noarcf( 2, na01 )
c
c        choix du sommet du cf a relier a l'arete na1
c        --------------------------------------------
         call trchtd( pxyd, na01, na1, noarcf,
     %                na03, na3,  larmin )
         if( na3 .eq. 0 ) then
            ierr = 3
            return
         endif
c
c        l'arete suivante de na1
         na02 = na1
         na2  = noarcf( 2, na1 )
c
c        formation du triangle arete na1 - sommet noarcf(1,na3)
c        ------------------------------------------------------
         call trcf3s( nbcf,   na01, na1, na02, na2, na03, na3,
     %                mosoar, mxsoar, n1soar, nosoar,
     %                moartr, n1artr, noartr, noarst,
     %                mxarcf, n1arcf, noarcf, nt )
         if( nt .le. 0 ) then
c           saturation du tableau noartr ou noarcf ou n1arcf
            ierr = 2
            return
         endif
c
c        ajout du triangle cree a sa pile
         if( nbtrcf .ge. mxarcf ) then
            write(imprim,*) 'saturation du tableau notrcf'
            ierr = 4
            return
         endif
         nbtrcf = nbtrcf + 1
         notrcf( nbtrcf ) = nt
         goto 10
      endif
c
c     mise a jour du chainage des triangles des aretes
c     ================================================
      do 30 ntp0 = 1, nbtrcf
c
c        le numero du triangle ajoute dans le tableau noartr
         nt0 = notrcf( ntp0 )
c
cccc        aire signee du triangle nt0
cccc        le numero des 3 sommets du triangle nt
ccc         call nusotr( nt0, mosoar, nosoar, moartr, noartr,
ccc     %                nosotr )
ccc         d = surtd2( pxyd(1,nosotr(1)), pxyd(1,nosotr(2)),
ccc     %               pxyd(1,nosotr(3)) )
ccc         if( d .le. 0 ) then
cccc
cccc           un triangle d'aire negative de plus
ccc            write(imprim,*) 'triangle ',nt0,' st:',nosotr,
ccc     %                      ' d aire ',d,'<=0'
ccc            pause
ccc         endif
c
cccc        trace du triangle nt0
ccc         call mttrtr( pxyd, nt0, moartr, noartr, mosoar, nosoar,
ccc     %                ncturq, ncblan )
c
c        boucle sur les 3 aretes du triangle
         do 20 i=1,3
c
c           le numero de l'arete i du triangle dans le tableau nosoar
            noar = abs( noartr(i,nt0) )
c
c           ce triangle est il deja chaine dans cette arete?
            nt1 = nosoar(4,noar)
            nt2 = nosoar(5,noar)
            if( nt1 .eq. nt0 .or. nt2 .eq. nt0 ) goto 20
c
c           ajout de ce triangle nt0 a l'arete noar
            if( nt1 .le. 0 ) then
c               le triangle est ajoute a l'arete
                nosoar( 4, noar ) = nt0
            else if( nt2 .le. 0 ) then
c               le triangle est ajoute a l'arete
                nosoar( 5, noar ) = nt0
            else
c              l'arete appartient a 2 triangles differents de nt0
c              anomalie. chainage des triangles des aretes defectueux
c              a corriger
               write(imprim,*) 'pause dans tridcf'
               ierr = 5
               return
            endif
c
 20      continue
c
 30   continue
      end


      subroutine te1stm( nsasup, pxyd,   noarst,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, mxartr, n1artr, noartr,
     %                   mxarcf, n1arcf, noarcf, larmin, notrcf, liarcf,
     %                   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    supprimer de la triangulation le sommet nsasup qui doit
c -----    etre un sommet interne ("centre" d'une boule de triangles)
c
c          attention: le chainage lchain de nosoar devient celui des cf
c
c entrees:
c --------
c nsasup : numero dans le tableau pxyd du sommet a supprimer
c pxyd   : tableau des coordonnees 2d des points
c          par point : x  y  distance_souhaitee
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c          attention: mxsoar>3*mxsomm obligatoire!
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxarcf : nombre de variables des tableaux n1arcf, noarcf, larmin, notrcf
c
c modifies:
c ---------
c noarst : noarst(i) numero d'une arete de sommet i
c n1soar : no de l'eventuelle premiere arete libre dans le tableau nosoar
c          chainage des vides suivant en 3 et precedant en 2 de nosoar
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = nosoar(1)+nosoar(2)*2
c          avec mxsoar>=3*mxsomm
c          une arete i de nosoar est vide <=> nosoar(1,i)=0 et
c          nosoar(2,arete vide)=l'arete vide qui precede
c          nosoar(3,arete vide)=l'arete vide qui suit
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c
c
c auxiliaires :
c -------------
c n1arcf : tableau (0:mxarcf) auxiliaire d'entiers
c noarcf : tableau (3,mxarcf) auxiliaire d'entiers
c larmin : tableau ( mxarcf ) auxiliaire d'entiers
c notrcf : tableau ( mxarcf ) auxiliaire d'entiers
c liarcf : tableau ( mxarcf ) auxiliaire d'entiers
c
c sortie :
c --------
c ierr   : =0 si pas d'erreur
c          -1 le sommet a supprimer n'est pas le centre d'une boule
c             de triangles. il est suppose externe
c             ou bien le sommet est centre d'un cf dont toutes les
c             aretes sont frontalieres
c             dans les 2 cas => retour sans modifs
c          >0 si une erreur est survenue
c          =11 algorithme defaillant
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
      parameter       ( lchain=6, quamal=0.3)
      common / unites / lecteu,imprim,intera,nunite(29)
      double precision  pxyd(3,*)
      integer           nosoar(mosoar,mxsoar),
     %                  noartr(moartr,*),
     %                  noarst(*),
     %                  n1arcf(0:mxarcf),
     %                  noarcf(3,mxarcf),
     %                  larmin(mxarcf),
     %                  notrcf(mxarcf),
     %                  liarcf(mxarcf)
c
c     nsasup est il un sommet interne, "centre" d'une boule de triangles?
c     => le sommet nsasup peut etre supprime
c     ===================================================================
c     formation du cf de ''centre'' le sommet nsasup
      call trp1st( nsasup, noarst, mosoar, nosoar,
     %             moartr, noartr,
     %             mxarcf, nbtrcf, notrcf )
      if( nbtrcf .le. 0 ) then
c        erreur: impossible de trouver tous les triangles de sommet nsasup
c        le sommet nsasup n'est pas supprime de la triangulation
         ierr = -1
         return
      else if( nbtrcf .le. 2 ) then
c        le sommet nsasup n'est pas supprime
         ierr = -1
         return
      endif
      if( nbtrcf*3 .gt. mxarcf ) then
         write(imprim,*) 'saturation du tableau noarcf'
         ierr = 10
         return
      endif
c
ccc      trace des triangles de l'etoile du sommet nsasup
ccc      call trpltr( nbtrcf, notrcf, pxyd,
ccc     %             moartr, noartr, mosoar, nosoar,
ccc     %             ncroug, ncblan )
c
c     si toutes les aretes du cf sont frontalieres, alors il est
c     interdit de detruire le sommet "centre" du cf
c     calcul du nombre nbarfr des aretes simples des nbtrcf triangles
      call trfrcf( nsasup, mosoar, nosoar, moartr, noartr,
     %             nbtrcf, notrcf, nbarfr  )
      if( nbarfr .ge. nbtrcf ) then
c        toutes les aretes simples sont frontalieres
c        le sommet nsasup ("centre" de la cavite) n'est pas supprime
         ierr = -1
         return
      endif
c
c     formation du contour ferme (liste chainee des aretes simples)
c     forme a partir des aretes des triangles de l'etoile du sommet nsasup
      call focftr( nbtrcf, notrcf, pxyd,   noarst,
     %             mosoar, mxsoar, n1soar, nosoar,
     %             moartr, n1artr, noartr,
     %             nbarcf, n1arcf, noarcf,
     %             ierr )
      if( ierr .ne. 0 ) return
c
c     ici le sommet nsasup appartient a aucune arete
      noarst( nsasup ) = 0
c
c     chainage des aretes vides dans le tableau noarcf
      n1arcf(0) = nbarcf+1
      mmarcf = min(8*nbarcf,mxarcf)
      do 40 i=nbarcf+1,mmarcf
         noarcf(2,i) = i+1
 40   continue
      noarcf(2,mmarcf) = 0
c
c     sauvegarde du chainage des aretes peripheriques
c     pour la mise en delaunay du maillage
      nbcf = n1arcf(1)
      do 50 i=1,nbarcf
c        le numero de l'arete dans le tableau nosoar
         liarcf( i ) = noarcf( 3, nbcf )
c        l'arete suivante dans le cf
         nbcf = noarcf( 2, nbcf )
 50   continue
c
c     triangulation directe du contour ferme sans le sommet nsasup
c     ============================================================
      nbcf = 1
      call tridcf( nbcf,   pxyd,   noarst,
     %             mosoar, mxsoar, n1soar, nosoar,
     %             moartr, n1artr, noartr,
     %             mxarcf, n1arcf, noarcf, larmin,
     %             nbtrcf, notrcf, ierr )
      if( ierr .ne. 0 ) return
c
c     transformation des triangles du cf en triangles delaunay
c     ========================================================
c     construction du chainage lchain dans nosoar
c     des aretes peripheriques du cf a partir de la sauvegarde liarcf
      noar0 = liarcf(1)
      do 60 i=2,nbarcf
c        le numero de l'arete peripherique du cf dans nosoar
         noar = liarcf( i )
         if( nosoar(3,noar) .le. 0 ) then
c           arete interne => elle est chainee a partir de la precedente
            nosoar( lchain, noar0 ) = noar
            noar0 = noar
         endif
 60   continue
c     la derniere arete peripherique n'a pas de suivante
      nosoar(lchain,noar0) = 0
c
c     mise en delaunay des aretes chainees
      call tedela( pxyd,   noarst,
     %             mosoar, mxsoar, n1soar, nosoar, liarcf(1),
     %             moartr, mxartr, n1artr, noartr, modifs )
ccc   write(imprim,*) 'nombre echanges diagonales =',modifs
      return
      end


      subroutine tr3str( np,     nt,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, mxartr, n1artr, noartr,
     %                   noarst,
     %                   nutr,   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    former les 3 sous-triangles du triangle nt a partir
c -----    du point interne np
c
c entrees:
c --------
c np     : numero dans le tableau pxyd du point
c nt     : numero dans le tableau noartr du triangle a trianguler
c mosoar : nombre maximal d'entiers par arete du tableau nosoar
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxartr : nombre maximal de triangles stockables dans le tableau noartr
c
c modifies:
c ---------
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c          une arete i de nosoar est vide  <=>  nosoar(1,i)=0
c nosoar : numero des 2 sommets , no ligne, 2 triangles, chainages
c          sommet 1 = 0 si arete vide => sommet 2 = arete vide suivante
c          hachage des aretes = (nosoar(1)+nosoar(2)) modulo mxsoar
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c noarst : noarst(i) numero d'une arete de sommet i
c
c sorties:
c --------
c nutr   : le numero des 3 sous-triangles du triangle nt
c nt     : en sortie le triangle initial n'est plus actif dans noartr
c          c'est en fait le premier triangle vide de noartr
c ierr   : =0 si pas d'erreur
c          =1 si le tableau nosoar est sature
c          =2 si le tableau noartr est sature
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
      integer    nosoar(mosoar,mxsoar),
     %           noartr(moartr,mxartr),
     %           noarst(*),
     %           nutr(3)
c
      integer    nosotr(3), nu2sar(2), nuarco(3)
c
c     reservation des 3 nouveaux triangles dans le tableau noartr
c     ===========================================================
      do 10 i=1,3
c        le numero du sous-triangle i dans le tableau noartr
         if( n1artr .le. 0 ) then
c           tableau noartr sature
            ierr = 2
            return
         endif
         nutr(i) = n1artr
c        le nouveau premier triangle libre dans noartr
         n1artr = noartr(2,n1artr)
 10   continue
c
c     les numeros des 3 sommets du triangle nt
      call nusotr( nt, mosoar, nosoar, moartr, noartr, nosotr )
c
c     formation des 3 aretes nosotr(i)-np dans le tableau nosoar
c     ==========================================================
      nt0 = nutr(3)
      do 20 i=1,3
c
c        le triangle a creer
         nti = nutr(i)
c
c        les 2 sommets du cote i du triangle nosotr
         nu2sar(1) = nosotr(i)
         nu2sar(2) = np
         call hasoar( mosoar, mxsoar, n1soar, nosoar, nu2sar, noar )
c        en sortie: noar>0 => no arete retrouvee
c                       <0 => no arete ajoutee
c                       =0 => saturation du tableau nosoar
c
         if( noar .eq. 0 ) then
c           saturation du tableau nosoar
            ierr = 1
            return
         else if( noar .lt. 0 ) then
c           l'arete a ete ajoutee. initialisation des autres informations
            noar = -noar
c           le numero des 2 sommets a ete initialise par hasoar
c           et (nosoar(1,noar)<nosoar(2,noar))
c           le numero de la ligne de l'arete: ici arete interne
            nosoar(3,noar) = 0
c        else
c           l'arete a ete retrouvee
c           le numero des 2 sommets a ete retrouve par hasoar
c           et (nosoar(1,noar)<nosoar(2,noar))
c           le numero de ligne reste inchange
         endif
c
c        le triangle 1 de l'arete noar => le triangle nt0
         nosoar(4,noar) = nt0
c        le triangle 2 de l'arete noar => le triangle nti
         nosoar(5,noar) = nti
c
c        le sommet nosotr(i) appartient a l'arete noar
         noarst( nosotr(i) ) = noar
c
c        le numero d'arete nosotr(i)-np
         nuarco(i) = noar
c
c        le triangle qui precede le suivant
         nt0 = nti
 20   continue
c
c     le numero d'une arete du point np
      noarst( np ) = noar
c
c     les 3 sous-triangles du triangle nt sont formes dans le tableau noartr
c     ======================================================================
      do 30 i=1,3
c
c        le numero suivant i => i mod 3 + 1
         if( i .ne. 3 ) then
            i1 = i + 1
         else
            i1 = 1
         endif
c
c        le numero dans noartr du sous-triangle a ajouter
         nti = nutr( i )
c
c        le numero de l'arete i du triangle initial nt
c        est l'arete 1 du sous-triangle i
         noar = noartr(i,nt)
         noartr( 1, nti ) = noar
c
c        mise a jour du numero de triangle de cette arete
         noar = abs( noar )
         if( nosoar(4,noar) .eq. nt ) then
c           le sous-triangle nti remplace le triangle nt
            nosoar(4,noar) = nti
         else
c           le sous-triangle nti remplace le triangle nt
            nosoar(5,noar) = nti
         endif
c
c        l'arete 2 du sous-triangle i est l'arete i1 ajoutee
         if( nosotr(i1) .eq. nosoar(1,nuarco(i1)) ) then
c           l'arete ns i1-np dans nosoar est dans le sens direct
            noartr( 2, nti ) = nuarco(i1)
         else
c           l'arete ns i1-np dans nosoar est dans le sens indirect
            noartr( 2, nti ) = -nuarco(i1)
         endif
c
c        l'arete 3 du sous-triangle i est l'arete i ajoutee
         if( nosotr(i) .eq. nosoar(1,nuarco(i)) ) then
c           l'arete ns i1-np dans nosoar est dans le sens indirect
            noartr( 3, nti ) = -nuarco(i)
         else
c           l'arete ns i1-np dans nosoar est dans le sens direct
            noartr( 3, nti ) = nuarco(i)
         endif
 30   continue
c
c     le triangle nt est rendu libre
c     ==============================
c     il devient n1artr le premier triangle libre
      noartr( 1, nt ) = 0
      noartr( 2, nt ) = n1artr
      n1artr = nt
      end


      subroutine mt4sqa( na,  moartr, noartr, mosoar, nosoar,
     %                   ns1, ns2, ns3, ns4)
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    calcul du numero des 4 sommets de l'arete na de nosoar
c -----    formant un quadrangle
c
c entrees:
c --------
c na     : numero de l'arete dans nosoar a traiter
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1=0 si triangle vide => arete2=triangle vide suivant
c mosoar : nombre maximal d'entiers par arete
c nosoar : numero des 2 sommets , no ligne, 2 triangles, chainages en +
c          sommet 1 = 0 si arete vide => sommet 2 = arete vide suivante
c
c sorties:
c --------
c ns1,ns2,ns3 : les 3 numeros des sommets du triangle t1 en sens direct
c ns1,ns4,ns2 : les 3 numeros des sommets du triangle t2 en sens direct
c
c si erreur rencontree => ns4 = 0
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c2345x7..............................................................012
      common / unites / lecteu, imprim, nunite(30)
      integer           noartr(moartr,*), nosoar(mosoar,*)
c
c     le numero de triangle est il correct  ?
c     a supprimer apres mise au point
      if( na .le. 0 ) then
c         nblgrc(nrerr) = 1
c         write(kerr(mxlger)(1:6),'(i6)') na
c         kerr(1) = kerr(mxlger)(1:6) //
c     %           ' no incorrect arete dans nosoar'
c         call lereur
          write(imprim,*) na, ' no incorrect arete dans nosoar'
         ns4 = 0
         return
      endif
c
      if( nosoar(1,na) .le. 0 ) then
c         nblgrc(nrerr) = 1
c         write(kerr(mxlger)(1:6),'(i6)') na
c         kerr(1) = kerr(mxlger)(1:6) //
c     %           ' arete non active dans nosoar'
c         call lereur
         write(imprim,*) na, ' arete non active dans nosoar'
         ns4 = 0
         return
      endif
c
c     recherche de l'arete na dans le premier triangle
      nt = nosoar(4,na)
      if( nt .le. 0 ) then
c         nblgrc(nrerr) = 1
c         write(kerr(mxlger)(1:6),'(i6)') na
c         kerr(1) =  'triangle 1 incorrect pour l''arete ' //
c     %               kerr(mxlger)(1:6)
c         call lereur
         write(imprim,*) 'triangle 1 incorrect pour l''arete ', na
         ns4 = 0
         return
      endif
c
      do 5 i=1,3
         if( abs( noartr(i,nt) ) .eq. na ) goto 8
 5    continue
c     si arrivee ici => bogue avant
      write(imprim,*) 'mt4sqa: arete',na,' non dans le triangle',nt
      ns4 = 0
      return
c
c     les 2 sommets de l'arete na
 8    if( noartr(i,nt) .gt. 0 ) then
         ns1 = 1
         ns2 = 2
      else
         ns1 = 2
         ns2 = 1
      endif
      ns1 = nosoar(ns1,na)
      ns2 = nosoar(ns2,na)
c
c     l'arete suivante
      if( i .lt. 3 ) then
         i = i + 1
      else
         i = 1
      endif
      naa = abs( noartr(i,nt) )
c
c     le sommet ns3 du triangle 123
      ns3 = nosoar(1,naa)
      if( ns3 .eq. ns1 .or. ns3 .eq. ns2 ) then
         ns3 = nosoar(2,naa)
      endif
c
c     le triangle de l'autre cote de l'arete na
c     =========================================
      nt = nosoar(5,na)
      if( nt .le. 0 ) then
c         nblgrc(nrerr) = 1
c         write(kerr(mxlger)(1:6),'(i6)') na
c         kerr(1) =  'triangle 2 incorrect pour l''arete ' //
c     %               kerr(mxlger)(1:6)
c         call lereur
          write(imprim,*) 'triangle 2 incorrect pour l''arete ',na
         ns4 = 0
         return
      endif
c
c     le numero de l'arete naa du triangle nt
      naa = abs( noartr(1,nt) )
      if( naa .eq. na ) naa = abs( noartr(2,nt) )
      ns4 = nosoar(1,naa)
      if( ns4 .eq. ns1 .or. ns4 .eq. ns2 ) then
         ns4 = nosoar(2,naa)
      endif
      end


      subroutine te2t2t( noaret, mosoar, n1soar, nosoar, noarst,
     %                   moartr, noartr, noar34 )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    echanger la diagonale des 2 triangles ayant en commun
c -----    l'arete noaret du tableau nosoar si c'est possible
c
c entrees:
c --------
c noaret : numero de l'arete a echanger entre les 2 triangles
c mosoar : nombre maximal d'entiers par arete
c moartr : nombre maximal d'entiers par triangle
c
c modifies :
c ----------
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c nosoar : numero des 2 sommets , no ligne, 2 triangles, chainages en +
c          sommet 1 = 0 si arete vide => sommet 2 = arete vide suivante
c noarst : noarst(i) numero d'une arete de sommet i
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c
c sortie :
c --------
c noar34 : numero nosoar de la nouvelle arete diagonale
c          0 si pas d'echange des aretes diagonales
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc      avril 1997
c....................................................................012
      integer     nosoar(mosoar,*),
     %            noartr(moartr,*),
     %            noarst(*)
c
c     une arete frontaliere ne peut etre echangee
      noar34 = 0
      if( nosoar(3,noaret) .gt. 0 ) return
c
c     les 4 sommets des 2 triangles ayant l'arete noaret en commun
      call mt4sqa( noaret, moartr, noartr, mosoar, nosoar,
     %             ns1, ns2, ns3, ns4)
c     ns1,ns2,ns3 : les 3 numeros des sommets du triangle nt1 en sens direct
c     ns1,ns4,ns2 : les 3 numeros des sommets du triangle nt2 en sens direct
c
c     recherche du numero de l'arete noaret dans le triangle nt1
      nt1 = nosoar(4,noaret)
      do 10 n1 = 1, 3
         if( abs(noartr(n1,nt1)) .eq. noaret ) goto 15
 10   continue
c     impossible d'arriver ici sans bogue!
      write(imprim,*) 'pause dans te2t2t 1'
c
c     l'arete de sommets 2 et 3
 15   if( n1 .lt. 3 ) then
         n2 = n1 + 1
      else
         n2 = 1
      endif
      na23 = noartr(n2,nt1)
c
c     l'arete de sommets 3 et 1
      if( n2 .lt. 3 ) then
         n3 = n2 + 1
      else
         n3 = 1
      endif
      na31 = noartr(n3,nt1)
c
c     recherche du numero de l'arete noaret dans le triangle nt2
      nt2 = nosoar(5,noaret)
      do 20 n1 = 1, 3
         if( abs(noartr(n1,nt2)) .eq. noaret ) goto 25
 20   continue
c     impossible d'arriver ici sans bogue!
      write(imprim,*) 'pause dans te2t2t 2'
c
c     l'arete de sommets 1 et 4
 25   if( n1 .lt. 3 ) then
         n2 = n1 + 1
      else
         n2 = 1
      endif
      na14 = noartr(n2,nt2)
c
c     l'arete de sommets 4 et 2
      if( n2 .lt. 3 ) then
         n3 = n2 + 1
      else
         n3 = 1
      endif
      na42 = noartr(n3,nt2)
c
c     les triangles 123 142 deviennent 143 234
c     ========================================
c     ajout de l'arete ns3-ns4
c     on evite l'affichage de l'erreur
      ierr = -1
      call fasoar( ns3,    ns4,    nt1,    nt2,    0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             noar34, ierr )
      if( ierr .gt. 0 ) then
c        ierr=1 si le tableau nosoar est sature
c            =2 si arete a creer et appartenant a 2 triangles distincts
c               des triangles nt1 et nt2
c            =3 si arete appartenant a 2 triangles distincts
c               differents des triangles nt1 et nt2
c            =4 si arete appartenant a 2 triangles distincts
c               dont le second n'est pas le triangle nt2
c        => pas d'echange
         noar34 = 0
         return
      endif
c
c     suppression de l'arete noaret
      call sasoar( noaret, mosoar, mxsoar, n1soar, nosoar )
c
c     nt1 = triangle 143
      noartr(1,nt1) =  na14
c     sens de stockage de l'arete ns3-ns4 dans nosoar?
      if( nosoar(1,noar34) .eq. ns3 ) then
         n1 = -1
      else
         n1 =  1
      endif
      noartr(2,nt1) = noar34 * n1
      noartr(3,nt1) = na31
c
c     nt2 = triangle 234
      noartr(1,nt2) =  na23
      noartr(2,nt2) = -noar34 * n1
      noartr(3,nt2) =  na42
c
c     echange nt1 -> nt2 pour l'arete na23
      na23 = abs( na23 )
      if( nosoar(4,na23) .eq. nt1 ) then
         n1 = 4
      else
         n1 = 5
      endif
      nosoar(n1,na23) = nt2
c
c     echange nt2 -> nt1 pour l'arete na14
      na14 = abs( na14 )
      if( nosoar(4,na14) .eq. nt2 ) then
         n1 = 4
      else
         n1 = 5
      endif
      nosoar(n1,na14) = nt1
c
c     numero d'une arete de chacun des 4 sommets
      noarst(ns1) = na14
      noarst(ns2) = na23
      noarst(ns3) = noar34
      noarst(ns4) = noar34
      end



      subroutine f0trte( letree, pxyd,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, mxartr, n1artr, noartr,
     %                   noarst,
     %                   nbtr,   nutr,   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    former le ou les triangles du triangle equilateral letree
c -----    les points internes au te deviennent des sommets des
c          sous-triangles du te
c
c entrees:
c --------
c letree : arbre-4 des triangles equilateraux (te) fond de la triangulation
c          si letree(0)>0 alors
c             letree(0:3) : no (>0) letree des 4 sous-triangles du triangle j
c          sinon
c             letree(0:3) :-no pxyd des 1 a 4 points internes au triangle j
c                           0  si pas de point
c                         ( le te est une feuille de l'arbre )
c          letree(4) : no letree du sur-triangle du triangle j
c          letree(5) : 0 1 2 3 no du sous-triangle j pour son sur-triangle
c          letree(6:8) : no pxyd des 3 sommets du triangle j
c pxyd   : tableau des x  y  distance_souhaitee de chaque sommet
c mosoar : nombre maximal d'entiers par arete du tableau nosoar
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxartr : nombre maximal de triangles stockables dans le tableau noartr
c
c modifies:
c ---------
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c          une arete i de nosoar est vide  <=>  nosoar(1,i)=0
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = (nosoar(1)+nosoar(2)) modulo mxsoar
c          sommet 1 = 0 si arete vide => sommet 2 = arete vide suivante
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c noarst : noarst(i) numero d'une arete de sommet i
c
c sorties:
c --------
c nbtr   : nombre de sous-triangles du te, triangulation du te
c nutr   : numero des nbtr sous-triangles du te dans le tableau noartr
c ierr   : =0 si pas d'erreur
c          =1 si le tableau nosoar est sature
c          =2 si le tableau noartr est sature
c          =3 si aucun des triangles ne contient l'un des points internes au te
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
      common / unites / lecteu, imprim, nunite(30)
      double precision  pxyd(3,*)
      integer           letree(0:8),
     %                  nosoar(mosoar,mxsoar),
     %                  noartr(moartr,mxartr),
     %                  noarst(*),
     %                  nutr(1:nbtr)
      integer           nuarco(3)
c
c     le numero nt du triangle dans le tableau noartr
      if( n1artr .le. 0 ) then
c        tableau noartr sature
         write(imprim,*) 'f0trte: tableau noartr sature'
         ierr = 2
         return
      endif
      nt = n1artr
c     le numero du nouveau premier triangle libre dans noartr
      n1artr = noartr( 2, n1artr )
c
c     formation du triangle = le triangle equilateral letree
      do 10 i=1,3
         if( i .ne. 3 ) then
            i1 = i + 1
         else
            i1 = 1
         endif
c        ajout eventuel de l'arete si si+1 dans le tableau nosoar
         call fasoar( letree(5+i), letree(5+i1), nt, -1, 0,
     %                mosoar, mxsoar, n1soar, nosoar, noarst,
     %                nuarco(i), ierr )
         if( ierr .ne. 0 ) return
 10   continue
c
c     le triangle nt est forme dans le tableau noartr
      do 20 i=1,3
c        letree(5+i) est le numero du sommet 1 de l'arete i du te
         if( letree(5+i) .eq. nosoar(1,nuarco(i)) ) then
            lesign =  1
         else
            lesign = -1
         endif
c        l'arete ns1-ns2 dans nosoar est celle du cote du te
         noartr( i, nt ) = lesign * nuarco(i)
 20   continue
c
c     triangulation du te=triangle nt par ajout des points internes du te
      nbtr    = 1
      nutr(1) = nt
      call trpite( letree, pxyd,
     %             mosoar, mxsoar, n1soar, nosoar,
     %             moartr, mxartr, n1artr, noartr, noarst,
     %             nbtr,   nutr,   ierr )
      end


      subroutine f1trte( letree, pxyd,   milieu,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, mxartr, n1artr, noartr,
     %                   noarst,
     %                   nbtr,   nutr,   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    former les triangles du triangle equilateral letree
c -----    a partir de l'un des 3 milieux des cotes du te
c          et des points internes au te
c          ils deviennent tous des sommets des sous-triangles du te
c
c entrees:
c --------
c letree : arbre-4 des triangles equilateraux (te) fond de la triangulation
c          si letree(0)>0 alors
c             letree(0:3) : no (>0) letree des 4 sous-triangles du triangle j
c          sinon
c             letree(0:3) :-no pxyd des 1 a 4 points internes au triangle j
c                           0  si pas de point
c                         ( le te est une feuille de l'arbre )
c          letree(4) : no letree du sur-triangle du triangle j
c          letree(5) : 0 1 2 3 no du sous-triangle j pour son sur-triangle
c          letree(6:8) : no pxyd des 3 sommets du triangle j
c pxyd   : tableau des x  y  distance_souhaitee de chaque sommet
c milieu : milieu(i) numero dans pxyd du milieu de l'arete i du te
c                    0 si pas de milieu du cote i a ajouter
c mosoar : nombre maximal d'entiers par arete du tableau nosoar
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxartr : nombre maximal de triangles stockables dans le tableau noartr
c
c modifies:
c ---------
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c          une arete i de nosoar est vide  <=>  nosoar(1,i)=0
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = (nosoar(1)+nosoar(2)) modulo mxsoar
c          sommet 1 = 0 si arete vide => sommet 2 = arete vide suivante
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c noarst : noarst(np) numero d'une arete du sommet np
c
c sorties:
c --------
c nbtr   : nombre de sous-triangles du te, triangulation du te
c nutr   : numero des nbtr sous-triangles du te dans le tableau noartr
c ierr   : =0 si pas d'erreur
c          =1 si le tableau nosoar est sature
c          =2 si le tableau noartr est sature
c          =3 si aucun des triangles ne contient l'un des points internes au te
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
      double precision  pxyd(3,*)
      integer           letree(0:8),
     %                  milieu(3),
     %                  nosoar(mosoar,mxsoar),
     %                  noartr(moartr,mxartr),
     %                  noarst(*),
     %                  nutr(1:nbtr)
c
      integer           nosotr(3), nuarco(5)
c
c     le numero des 2 triangles (=2 demi te) a creer dans le tableau noartr
      do 5 nbtr=1,2
         if( n1artr .le. 0 ) then
c           tableau noartr sature
            ierr = 2
            return
         endif
         nutr(nbtr) = n1artr
c        le nouveau premier triangle libre dans noartr
         n1artr = noartr(2,n1artr)
 5    continue
      nbtr = 2
c
c     recherche du milieu a creer
      do 7 i=1,3
         if( milieu(i) .ne. 0 ) goto 9
 7    continue
c     le numero pxyd du point milieu du cote i
 9    nm = milieu( i )
c
c     on se ramene au seul cas i=3 c-a-d le milieu est sur le cote 3
      if( i .eq. 1 ) then
c        milieu sur le cote 1
         nosotr(1) = letree(7)
         nosotr(2) = letree(8)
         nosotr(3) = letree(6)
      else if( i .eq. 2 ) then
c        milieu sur le cote 2
         nosotr(1) = letree(8)
         nosotr(2) = letree(6)
         nosotr(3) = letree(7)
      else
c        milieu sur le cote 3
         nosotr(1) = letree(6)
         nosotr(2) = letree(7)
         nosotr(3) = letree(8)
      endif
c
c     formation des 2 aretes s1 s2 et s2 s3
      do 10 i=1,2
         if( i .ne. 3 ) then
            i1 = i + 1
         else
            i1 = 1
         endif
c        ajout eventuel de l'arete dans nosoar
         call fasoar( nosotr(i), nosotr(i1), nutr(i), -1, 0,
     %                mosoar, mxsoar, n1soar, nosoar, noarst,
     %                nuarco(i), ierr )
         if( ierr .ne. 0 ) return
 10   continue
c
c     ajout eventuel de l'arete s3 milieu dans nosoar
      call fasoar( nosotr(3), nm, nutr(2), -1, 0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             nuarco(3), ierr )
      if( ierr .ne. 0 ) return
c
c     ajout eventuel de l'arete milieu s1 dans nosoar
      call fasoar( nosotr(1), nm, nutr(1), -1, 0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             nuarco(4), ierr )
      if( ierr .ne. 0 ) return
c
c     ajout eventuel de l'arete milieu s2 dans nosoar
      call fasoar( nosotr(2), nm, nutr(1), nutr(2), 0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             nuarco(5), ierr )
      if( ierr .ne. 0 ) return
c
c     les aretes s1 s2 et s2 s3 dans le tableau noartr
      do 20 i=1,2
c        nosotr(i) est le numero du sommet 1 de l'arete i du te
         if( nosotr(i) .eq. nosoar(1,nuarco(i)) ) then
            lesign = 1
         else
            lesign = -1
         endif
c        l'arete ns1-ns2 dans nosoar est celle du cote du te
         noartr( 1, nutr(i) ) = lesign * nuarco(i)
 20   continue
c
c     l'arete mediane s2 milieu
      if( nm .eq. nosoar(1,nuarco(5)) ) then
         lesign = -1
      else
         lesign =  1
      endif
      noartr( 2, nutr(1) ) =  lesign * nuarco(5)
      noartr( 3, nutr(2) ) = -lesign * nuarco(5)
c
c     l'arete s1 milieu
      if( nm .eq. nosoar(1,nuarco(4)) ) then
         lesign =  1
      else
         lesign = -1
      endif
      noartr( 3, nutr(1) ) = lesign * nuarco(4)
c
c     l'arete s3 milieu
      if( nm .eq. nosoar(1,nuarco(3)) ) then
         lesign = -1
      else
         lesign =  1
      endif
      noartr( 2, nutr(2) ) = lesign * nuarco(3)
c
c     triangulation des 2 demi te par ajout des points internes du te
      call trpite( letree, pxyd,
     %             mosoar, mxsoar, n1soar, nosoar,
     %             moartr, mxartr, n1artr, noartr, noarst,
     %             nbtr,   nutr,   ierr )
      end


      subroutine f2trte( letree, pxyd,   milieu,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, mxartr, n1artr, noartr,
     %                   noarst,
     %                   nbtr,   nutr,   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    former les triangles du triangle equilateral letree
c -----    a partir de 2 milieux des cotes du te
c          et des points internes au te
c          ils deviennent tous des sommets des sous-triangles du te
c
c entrees:
c --------
c letree : arbre-4 des triangles equilateraux (te) fond de la triangulation
c          si letree(0)>0 alors
c             letree(0:3) : no (>0) letree des 4 sous-triangles du triangle j
c          sinon
c             letree(0:3) :-no pxyd des 1 a 4 points internes au triangle j
c                           0  si pas de point
c                         ( le te est une feuille de l'arbre )
c          letree(4) : no letree du sur-triangle du triangle j
c          letree(5) : 0 1 2 3 no du sous-triangle j pour son sur-triangle
c          letree(6:8) : no pxyd des 3 sommets du triangle j
c pxyd   : tableau des x  y  distance_souhaitee de chaque sommet
c milieu : milieu(i) numero dans pxyd du milieu de l'arete i du te
c                    0 si pas de milieu du cote i a ajouter
c mosoar : nombre maximal d'entiers par arete du tableau nosoar
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxartr : nombre maximal de triangles stockables dans le tableau noartr
c
c modifies:
c ---------
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c          une arete i de nosoar est vide  <=>  nosoar(1,i)=0
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = (nosoar(1)+nosoar(2)) modulo mxsoar
c          sommet 1 = 0 si arete vide => sommet 2 = arete vide suivante
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c noarst : noarst(np) numero d'une arete du sommet np
c
c sorties:
c --------
c nbtr   : nombre de sous-triangles du te, triangulation du te
c nutr   : numero des nbtr sous-triangles du te dans le tableau noartr
c ierr   : =0 si pas d'erreur
c          =1 si le tableau nosoar est sature
c          =2 si le tableau noartr est sature
c          =3 si aucun des triangles ne contient l'un des points internes au te
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
      common / unites / lecteu, imprim, nunite(30)
      double precision  pxyd(3,*)
      integer           letree(0:8),
     %                  milieu(3),
     %                  nosoar(mosoar,mxsoar),
     %                  noartr(moartr,mxartr),
     %                  noarst(*),
     %                  nutr(1:nbtr)
c
      integer           nosotr(3), nuarco(7)
c
c     le numero des 3 triangles a creer dans le tableau noartr
      do 5 nbtr=1,3
         if( n1artr .le. 0 ) then
c           tableau noartr sature
            ierr = 2
            return
         endif
         nutr(nbtr) = n1artr
c        le nouveau premier triangle libre dans noartr
         n1artr = noartr(2,n1artr)
 5    continue
      nbtr = 3
c
c     recherche du premier milieu a creer
      do 7 i=1,3
         if( milieu(i) .ne. 0 ) goto 9
 7    continue
c
c     on se ramene au seul cas i=2 c-a-d le cote 1 n'a pas de milieu
 9    if( i .eq. 2 ) then
c        pas de milieu sur le cote 1
         nosotr(1) = letree(6)
         nosotr(2) = letree(7)
         nosotr(3) = letree(8)
c        le numero pxyd du milieu du cote 2
         nm2 = milieu( 2 )
c        le numero pxyd du milieu du cote 3
         nm3 = milieu( 3 )
      else if( milieu(2) .ne. 0 ) then
c        pas de milieu sur le cote 3
         nosotr(1) = letree(8)
         nosotr(2) = letree(6)
         nosotr(3) = letree(7)
c        le numero pxyd du milieu du cote 2
         nm2 = milieu( 1 )
c        le numero pxyd du milieu du cote 3
         nm3 = milieu( 2 )
      else
c        pas de milieu sur le cote 2
         nosotr(1) = letree(7)
         nosotr(2) = letree(8)
         nosotr(3) = letree(6)
c        le numero pxyd du milieu du cote 2
         nm2 = milieu( 3 )
c        le numero pxyd du milieu du cote 3
         nm3 = milieu( 1 )
      endif
c
c     ici seul le cote 1 n'a pas de milieu
c     nm2 est le milieu du cote 2
c     nm3 est le milieu du cote 3
c
c     ajout eventuel de l'arete s1 s2 dans nosoar
      call fasoar( nosotr(1), nosotr(2), nutr(1), -1, 0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             nuarco(1), ierr )
      if( ierr .ne. 0 ) return
c
c     ajout eventuel de l'arete s1 s2 dans nosoar
      call fasoar( nosotr(2), nm2, nutr(1), -1, 0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             nuarco(2), ierr )
      if( ierr .ne. 0 ) return
c
c     ajout eventuel de l'arete s1 nm2 dans nosoar
      call fasoar( nosotr(1), nm2, nutr(1), nutr(2), 0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             nuarco(3), ierr )
      if( ierr .ne. 0 ) return
c
c     ajout eventuel de l'arete nm2 nm3 dans nosoar
      call fasoar( nm3, nm2, nutr(2), nutr(3), 0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             nuarco(4), ierr )
      if( ierr .ne. 0 ) return
c
c     ajout eventuel de l'arete s1 nm3 dans nosoar
      call fasoar( nosotr(1), nm3, nutr(2), -1, 0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             nuarco(5), ierr )
      if( ierr .ne. 0 ) return
c
c     ajout eventuel de l'arete nm2 s3 dans nosoar
      call fasoar( nm2, nosotr(3), nutr(3), -1, 0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             nuarco(6), ierr )
c
c     ajout eventuel de l'arete nm3 s3 dans nosoar
      call fasoar( nosotr(3), nm3, nutr(3), -1, 0,
     %             mosoar, mxsoar, n1soar, nosoar, noarst,
     %             nuarco(7), ierr )
      if( ierr .ne. 0 ) return
c
c     le triangle s1 s2 nm2  ou arete1 arete2 arete3
      do 20 i=1,2
c        nosotr(i) est le numero du sommet 1 de l'arete i du te
         if( nosotr(i) .eq. nosoar(1,nuarco(i)) ) then
            lesign = 1
         else
            lesign = -1
         endif
c        l'arete ns1-ns2 dans nosoar est celle du cote du te
         noartr( i, nutr(1) ) = lesign * nuarco(i)
 20   continue
      if( nm2 .eq. nosoar(1,nuarco(3)) ) then
         lesign =  1
      else
         lesign = -1
      endif
      noartr( 3, nutr(1) ) = lesign * nuarco(3)
c
c     le triangle s1 nm2 nm3
      noartr( 1, nutr(2) ) = -lesign * nuarco(3)
      if( nm2 .eq. nosoar(1,nuarco(4)) ) then
         lesign =  1
      else
         lesign = -1
      endif
      noartr( 2, nutr(2) ) =  lesign * nuarco(4)
      noartr( 1, nutr(3) ) = -lesign * nuarco(4)
      if( nm3 .eq. nosoar(1,nuarco(5)) ) then
         lesign =  1
      else
         lesign = -1
      endif
      noartr( 3, nutr(2) ) = lesign * nuarco(5)
c
c     le triangle nm2 nm3 s3
      if( nm2 .eq. nosoar(1,nuarco(6)) ) then
         lesign =  1
      else
         lesign = -1
      endif
      noartr( 2, nutr(3) ) =  lesign * nuarco(6)
      if( nm3 .eq. nosoar(1,nuarco(7)) ) then
         lesign = -1
      else
         lesign =  1
      endif
      noartr( 3, nutr(3) ) = lesign * nuarco(7)
c
c     triangulation des 3 sous-te par ajout des points internes du te
      call trpite( letree, pxyd,
     %             mosoar, mxsoar, n1soar, nosoar,
     %             moartr, mxartr, n1artr, noartr, noarst,
     %             nbtr,   nutr,   ierr )
      end


      subroutine f3trte( letree, pxyd,   milieu,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, mxartr, n1artr, noartr,
     %                   noarst,
     %                   nbtr,   nutr,   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    former les triangles du triangle equilateral letree
c -----    a partir de 3 milieux des cotes du te
c          et des points internes au te
c          ils deviennent tous des sommets des sous-triangles du te
c
c entrees:
c --------
c letree : arbre-4 des triangles equilateraux (te) fond de la triangulation
c          si letree(0)>0 alors
c             letree(0:3) : no (>0) letree des 4 sous-triangles du triangle j
c          sinon
c             letree(0:3) :-no pxyd des 1 a 4 points internes au triangle j
c                           0  si pas de point
c                         ( le te est une feuille de l'arbre )
c          letree(4) : no letree du sur-triangle du triangle j
c          letree(5) : 0 1 2 3 no du sous-triangle j pour son sur-triangle
c          letree(6:8) : no pxyd des 3 sommets du triangle j
c pxyd   : tableau des x  y  distance_souhaitee de chaque sommet
c milieu : milieu(i) numero dans pxyd du milieu de l'arete i du te
c                    0 si pas de milieu du cote i a ajouter
c mosoar : nombre maximal d'entiers par arete du tableau nosoar
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxartr : nombre maximal de triangles stockables dans le tableau noartr
c
c modifies:
c ---------
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c          une arete i de nosoar est vide  <=>  nosoar(1,i)=0
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = (nosoar(1)+nosoar(2)) modulo mxsoar
c          sommet 1 = 0 si arete vide => sommet 2 = arete vide suivante
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c noarst : noarst(np) numero d'une arete du sommet np
c
c sorties:
c --------
c nbtr   : nombre de sous-triangles du te, triangulation du te
c nutr   : numero des nbtr sous-triangles du te dans le tableau noartr
c ierr   : =0 si pas d'erreur
c          =1 si le tableau nosoar est sature
c          =2 si le tableau noartr est sature
c          =3 si aucun des triangles ne contient l'un des points internes au te
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
      common / unites / lecteu, imprim, nunite(30)
      double precision  pxyd(3,*)
      integer           letree(0:8),
     %                  milieu(3),
     %                  nosoar(mosoar,mxsoar),
     %                  noartr(moartr,mxartr),
     %                  noarst(*),
     %                  nutr(1:nbtr)
c
      integer           nuarco(9)
c
c     le numero des 4 triangles a creer dans le tableau noartr
      do 5 nbtr=1,4
         if( n1artr .le. 0 ) then
c           tableau noartr sature
            ierr = 2
            return
         endif
         nutr(nbtr) = n1artr
c        le nouveau premier triangle libre dans noartr
         n1artr = noartr(2,n1artr)
 5    continue
      nbtr = 4
c
      do 10 i=1,3
c        le sommet suivant
         if( i .ne. 3 ) then
            i1 = i + 1
         else
            i1 = 1
         endif
c        le sommet precedant
         if( i .ne. 1 ) then
            i0 = i - 1
         else
            i0 = 3
         endif
         i3 = 3 * i
c
c        ajout eventuel de l'arete si mi dans nosoar
         call fasoar( letree(5+i), milieu(i), nutr(i), -1, 0,
     %                mosoar, mxsoar, n1soar, nosoar, noarst,
     %                nuarco(i3-2), ierr )
         if( ierr .ne. 0 ) return
c
c        ajout eventuel de l'arete mi mi-1 dans nosoar
         call fasoar( milieu(i), milieu(i0), nutr(i), nutr(4), 0,
     %                mosoar, mxsoar, n1soar, nosoar, noarst,
     %                nuarco(i3-1), ierr )
         if( ierr .ne. 0 ) return
c
c        ajout eventuel de l'arete m i-1  si dans nosoar
         call fasoar( milieu(i0), letree(5+i), nutr(i), -1, 0,
     %                mosoar, mxsoar, n1soar, nosoar, noarst,
     %                nuarco(i3), ierr )
         if( ierr .ne. 0 ) return
c
 10   continue
c
c     les 3 sous-triangles pres des sommets
      do 20 i=1,3
c        le sommet suivant
         if( i .ne. 3 ) then
            i1 = i + 1
         else
            i1 = 1
         endif
c        le sommet precedant
         if( i .ne. 1 ) then
            i0 = i - 1
         else
            i0 = 3
         endif
         i3 = 3 * i
c
c        ajout du triangle  arete3i-2 arete3i-1 arete3i
         if( letree(5+i) .eq. nosoar(1,nuarco(i3-2)) ) then
            lesign =  1
         else
            lesign = -1
         endif
         noartr( 1, nutr(i) ) = lesign * nuarco(i3-2)
c
         if( milieu(i) .eq. nosoar(1,nuarco(i3-1)) ) then
            lesign =  1
         else
            lesign = -1
         endif
         noartr( 2, nutr(i) ) = lesign * nuarco(i3-1)
c
         if( milieu(i0) .eq. nosoar(1,nuarco(i3)) ) then
            lesign =  1
         else
            lesign = -1
         endif
         noartr( 3, nutr(i) ) = lesign * nuarco(i3)
c
 20   continue
c
c     le sous triangle central
      i3 = -1
      do 30 i=1,3
         i3 = i3 + 3
         if( milieu(i) .eq. nosoar(1,nuarco(i3)) ) then
            lesign = -1
         else
            lesign =  1
         endif
         noartr( i, nutr(4) ) = lesign * nuarco(i3)
 30   continue
c
c     triangulation des 3 sous-te par ajout des points internes du te
      call trpite( letree, pxyd,
     %             mosoar, mxsoar, n1soar, nosoar,
     %             moartr, mxartr, n1artr, noartr, noarst,
     %             nbtr,   nutr,   ierr )
      end



      subroutine hasoar( mosoar, mxsoar, n1soar, nosoar, nu2sar,
     %                   noar )
c ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    rechercher le numero des 2 sommets d'une arete parmi
c -----    les numeros des 2 sommets des aretes du tableau nosoar
c          s ils n y sont pas stockes les y ajouter
c          dans tous les cas retourner le numero de l'arete dans nosoar
c
c          la methode employee ici est celle du hachage
c          avec pour fonction d'adressage h(ns1,ns2)=min(ns1,ns2)
c
c          remarque: h(ns1,ns2)=ns1 + 2*ns2
c                    ne marche pas si des aretes sont detruites
c                    et ajoutees aux aretes vides
c                    le chainage est commun a plusieurs hachages!
c                    d'ou ce choix du minimum pour le hachage
c
c entrees:
c --------
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c          attention: mxsoar>3*mxsomm obligatoire!
c
c modifies:
c ---------
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c          une arete i de nosoar est vide  <=>  nosoar(1,i)=0
c          chainage des aretes vides amont et aval
c          l'arete vide qui precede=nosoar(4,i)
c          l'arete vide qui suit   =nosoar(5,i)
c nosoar : numero des 2 sommets, no ligne, 2 triangles de l'arete,
c          chainage momentan'e d'aretes, chainage du hachage des aretes
c          hachage des aretes = min( nosoar(1), nosoar(2) )
c nu2sar : en entree les 2 numeros des sommets de l'arete
c          en sortie nu2sar(1)<nu2sar(2) numeros des 2 sommets de l'arete
c
c sorties:
c --------
c noar   : numero dans nosoar de l'arete apres hachage
c          =0 si saturation du tableau nosoar
c          >0 si le tableau nu2sar est l'arete noar retrouvee
c             dans le tableau nosoar
c          <0 si le tableau nu2sar a ete ajoute et forme l'arete
c             -noar du tableau nosoar avec nosoar(1,noar)<nosoar(2,noar)
c ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique upmc paris       mars 1997
c ...................................................................012
      integer  nu2sar(2), nosoar(mosoar,mxsoar)
c
      if( nu2sar(1) .gt. nu2sar(2) ) then
c
c        permutation des numeros des 2 sommets pour
c        amener le plus petit dans nu2sar(1)
         i         = nu2sar(1)
         nu2sar(1) = nu2sar(2)
         nu2sar(2) = i
      endif
c
c     la fonction d'adressage du hachage des aretes : h(ns1,ns2)=min(ns1,ns2)
c     ===============================================
      noar = nu2sar(1)
c
c     la recherche de l'arete dans le chainage du hachage
c     ---------------------------------------------------
 10   if( nu2sar(1) .eq. nosoar(1,noar) ) then
         if( nu2sar(2) .eq. nosoar(2,noar) ) then
c
c           l'arete est retrouvee
c           .....................
            return
         endif
      endif
c
c     l'arete suivante parmi celles ayant meme fonction d'adressage
      i = nosoar( mosoar, noar )
      if( i .gt. 0 ) then
         noar = i
         goto 10
      endif
c
c     noar est ici la derniere arete (sans suivante) du chainage
c     a partir de l'adressage du hachage
c
c     l'arete non retrouvee doit etre ajoutee
c     .......................................
      if( nosoar( 1, nu2sar(1) ) .eq. 0 ) then
c
c        l'adresse de hachage est libre => elle devient la nouvelle arete
c        retouche des chainages de cette arete noar qui ne sera plus vide
         noar = nu2sar(1)
c        l'eventuel chainage du hachage n'est pas modifie
c
      else
c
c        la premiere arete dans l'adressage du hachage n'est pas libre
c        => choix quelconque d'une arete vide pour ajouter cette arete
         if( n1soar .le. 0 ) then
c
c           le tableau nosoar est sature avec pour temoin d'erreur
            noar = 0
            return
c
         else
c
c           l'arete n1soar est vide => c'est la nouvelle arete
c           mise a jour du chainage de la derniere arete noar du chainage
c           sa suivante est la nouvelle arete n1soar
            nosoar( mosoar, noar ) = n1soar
c
c           l'arete ajoutee est n1soar
            noar = n1soar
c
c           la nouvelle premiere arete vide
            n1soar = nosoar( 5, n1soar )
c
c           la premiere arete vide n1soar n'a pas d'arete vide precedente
            nosoar( 4, n1soar ) = 0
c
c           noar la nouvelle arete est la derniere du chainage du hachage
            nosoar( mosoar, noar ) = 0
c
         endif
c
      endif
c
c     les 2 sommets de la nouvelle arete noar
      nosoar( 1, noar ) = nu2sar(1)
      nosoar( 2, noar ) = nu2sar(2)
c
c     le tableau nu2sar a ete ajoute avec l'indice -noar
      noar = - noar
      end


      subroutine mt3str( nt, moartr, noartr, mosoar, nosoar,
     %                   ns1, ns2, ns3 )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but : calcul du numero des 3 sommets du triangle nt du tableau noartr
c -----
c
c entrees:
c --------
c nt     : numero du triangle de noartr a traiter
c moartr : nombre maximal d'entiers par triangle
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1=0 si triangle vide => arete2=triangle vide suivant
c mosoar : nombre maximal d'entiers par arete
c nosoar : numero des 2 sommets , no ligne, 2 triangles, chainages en +
c          sommet 1 = 0 si arete vide => sommet 2 = arete vide suivante
c
c sorties:
c --------
c ns1,ns2,ns3 : les 3 numeros des sommets du triangle en sens direct
c
c si erreur rencontree => ns1 = 0
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc    juillet 1995
c2345x7..............................................................012
      integer    noartr(moartr,*), nosoar(mosoar,*)
c
c     le numero de triangle est il correct  ?
c     a supprimer apres mise au point
      if( nt .le. 0 ) then
c         nblgrc(nrerr) = 1
c         write(kerr(mxlger)(1:6),'(i6)') nt
c         kerr(1) = kerr(mxlger)(1:6) //
c     %           ' no triangle dans noartr incorrect'
c         call lereur
         write(imprim,*) nt,' no triangle dans noartr incorrect'
         ns1 = 0
         return
      endif
c
      na = noartr(1,nt)
      if( na .gt. 0 ) then
c        arete dans le sens direct
         ns1 = nosoar(1,na)
         ns2 = nosoar(2,na)
      else
c        arete dans le sens indirect
         ns1 = nosoar(2,-na)
         ns2 = nosoar(1,-na)
      endif
c
      na = noartr(2,nt)
      if( na .gt. 0 ) then
c        arete dans le sens direct => ns3 est le second sommet de l'arete
         ns3 = nosoar(2,na)
      else
c        arete dans le sens indirect => ns3 est le premier sommet de l'arete
         ns3 = nosoar(1,-na)
      endif
      end
      subroutine trpite( letree, pxyd,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, mxartr, n1artr, noartr,
     %                   noarst,
     %                   nbtr,   nutr,   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    former le ou les sous-triangles des nbtr triangles nutr
c -----    qui forment le triangle equilateral letree par ajout
c          des points internes au te qui deviennent des sommets des
c          sous-triangles des nbtr triangles
c
c entrees:
c --------
c letree : arbre-4 des triangles equilateraux (te) fond de la triangulation
c          letree(0:3):-no pxyd des 1 a 4 points internes au triangle j
c                       0  si pas de point
c                     ( le te est ici une feuille de l'arbre )
c          letree(4) : no letree du sur-triangle du triangle j
c          letree(5) : 0 1 2 3 no du sous-triangle j pour son sur-triangle
c          letree(6:8) : no pxyd des 3 sommets du triangle j
c pxyd   : tableau des x  y  distance_souhaitee de chaque sommet
c mosoar : nombre maximal d'entiers par arete du tableau nosoar
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c moartr : nombre maximal d'entiers par arete du tableau noartr
c mxartr : nombre maximal de triangles stockables dans le tableau noartr
c
c modifies:
c ---------
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c          une arete i de nosoar est vide  <=>  nosoar(1,i)=0
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = (nosoar(1)+nosoar(2)) modulo mxsoar
c          sommet 1 = 0 si arete vide => sommet 2 = arete vide suivante
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c noarst : noarst(i) numero d'une arete de sommet i
c
c sorties:
c --------
c nbtr   : nombre de sous-triangles du te
c nutr   : numero des nbtr sous-triangles du te dans le tableau noartr
c ierr   : =0 si pas d'erreur
c          =1 si le tableau nosoar est sature
c          =2 si le tableau noartr est sature
c          =3 si aucun des triangles ne contient l'un des points internes au te
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
      logical           tratri
      common / dv2dco / tratri
c     trace ou non des triangles generes dans la triangulation
      common / unites / lecteu, imprim, nunite(30)
      double precision  pxyd(3,*)
      integer           letree(0:8),
     %                  nosoar(mosoar,mxsoar),
     %                  noartr(moartr,mxartr),
     %                  noarst(*),
     %                  nutr(1:nbtr)
c
      integer           nosotr(3)
c
c     si pas de point interne alors trace eventuel puis retour
      if( letree(0) .eq. 0 ) goto 150
c
c     il existe au moins un point interne a trianguler
c     dans les nbtr triangles
      do 100 k=0,3
c
c        le numero du point
         np = -letree(k)
         if( np .eq. 0 ) goto 150
c
c        le point np dans pxyd est a traiter
         do 10 n = 1, nbtr
c
c           les numeros des 3 sommets du triangle nt=nutr(n)
            nt = nutr(n)
            call nusotr( nt, mosoar, nosoar, moartr, noartr,  nosotr )
c
c           le triangle nt contient il le point np?
            call ptdatr( pxyd(1,np), pxyd, nosotr, nsigne )
c           nsigne>0 si le point est dans le triangle ou sur une des 3 aretes
c                 =0 si triangle degenere ou indirect ou ne contient pas le poin
c
            if( nsigne .gt. 0 ) then
c
c              le triangle nt est triangule en 3 sous-triangles
               call tr3str( np, nt,
     %                      mosoar, mxsoar, n1soar, nosoar,
     %                      moartr, mxartr, n1artr, noartr,
     %                      noarst,
     %                      nutr(nbtr+1),  ierr )
               if( ierr .ne. 0 ) return
c
c              reamenagement des 3 triangles crees dans nutr
c              en supprimant le triangle nt
               nutr( n ) = nutr( nbtr + 3 )
               nbtr = nbtr + 2
c              le point np est triangule
               goto 100
c
            endif
 10      continue
c
c        erreur: le point np n'est pas dans l'un des nbtr triangles
         write(imprim,10010) np
         ierr = 3
         return
c
 100  continue
10010 format(' erreur trpite: pas de triangle contenant le point',i7)
c
 150  continue

ccc 150  if( tratri ) then
cccc       les traces sont demandes
ccc        call efface
cccc       le cadre objet global en unites utilisateur
ccc        xx1 = min(pxyd(1,nosotr(1)),pxyd(1,nosotr(2)),pxyd(1,nosotr(3)))
ccc        xx2 = max(pxyd(1,nosotr(1)),pxyd(1,nosotr(2)),pxyd(1,nosotr(3)))
ccc        yy1 = min(pxyd(2,nosotr(1)),pxyd(2,nosotr(2)),pxyd(2,nosotr(3)))
ccc        yy2 = max(pxyd(2,nosotr(1)),pxyd(2,nosotr(2)),pxyd(2,nosotr(3)))
ccc        if( xx1 .ge. xx2 ) xx2 = xx1 + (yy2-yy1)
ccc        if( yy1 .ge. yy2 ) yy2 = yy1 + (xx2-xx1)*0.5
ccc        call isofenetre( xx1-(xx2-xx1), xx2+(xx2-xx1),
ccc     %                   yy1-(yy2-yy1), yy2+(yy2-yy1) )
ccc         do 200 i=1,nbtr
cccc           trace du triangle nutr(i)
ccc            call mttrtr( pxyd, nutr(i), moartr, noartr, mosoar, nosoar,
ccc     %                   i, ncblan )
ccc 200     continue
ccc      endif

      end


      subroutine sasoar( noar, mosoar, mxsoar, n1soar, nosoar )
c ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    supprimer l'arete noar du tableau nosoar
c -----    si celle ci n'est pas une arete des lignes de la frontiere
c
c          la methode employee ici est celle du hachage
c          avec pour fonction d'adressage h = min( nu2sar(1), nu2sar(2) )
c
c          attention: il faut mettre a jour le no d'arete des 2 sommets
c                     de l'arete supprimee dans le tableau noarst!
c
c entrees:
c --------
c noar   : numero de l'arete de nosoar a supprimer
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage h
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c          attention: mxsoar>3*mxsomm obligatoire!
c
c modifies:
c ---------
c n1soar : no de l'eventuelle premiere arete libre dans le tableau nosoar
c          chainage des vides suivant en 3 et precedant en 2 de nosoar
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          une arete i de nosoar est vide <=> nosoar(1,i)=0 et
c          nosoar(4,arete vide)=l'arete vide qui precede
c          nosoar(5,arete vide)=l'arete vide qui suit
c ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique upmc paris       mars 1997
c ...................................................................012
      common / unites / lecteu, imprim, nunite(30)
      integer           nosoar(mosoar,mxsoar)
c
      if( nosoar(3,noar) .le. 0 ) then
c
c        l'arete n'est pas frontaliere => elle devient une arete vide
c
c        recherche de l'arete qui precede dans le chainage du hachage
         noar1 = nosoar(1,noar)
c
c        parcours du chainage du hachage jusqu'a retrouver l'arete noar
 10      if( noar1 .ne. noar ) then
c
c           l'arete suivante parmi celles ayant meme fonction d'adressage
            noar0 = noar1
            noar1 = nosoar( mosoar, noar1 )
            if( noar1 .gt. 0 ) goto 10
c
c           l'arete noar n'a pas ete retrouvee dans le chainage => erreur
            write(imprim,*) 'erreur sasoar:arete non dans le chainage '
     %                      ,noar
            write(imprim,*) 'arete de st1=',nosoar(1,noar),
     %      ' st2=',nosoar(2,noar),' ligne=',nosoar(3,noar),
     %      ' tr1=',nosoar(4,noar),' tr2=',nosoar(5,noar)
            write(imprim,*) 'chainages=',(nosoar(i,noar),i=6,mosoar)
c           l'arete n'est pas detruite
            return
c
         endif
c
         if( noar .ne. nosoar(1,noar) ) then
c
c           saut de l'arete noar dans le chainage du hachage
c           noar0 initialisee est ici l'arete qui precede noar dans ce chainage
            nosoar( mosoar, noar0 ) = nosoar( mosoar, noar )
c
c           le chainage du hachage n'existe plus pour noar
c           pas utile car mise a zero faite dans le sp hasoar
ccc         nosoar( mosoar, noar ) = 0
c
c           noar devient la nouvelle premiere arete du chainage des vides
            nosoar( 4, noar ) = 0
            nosoar( 5, noar ) = n1soar
c           la nouvelle precede l'ancienne premiere
            nosoar( 4, n1soar ) = noar
            n1soar = noar
c
ccc      else
c
c           noar est la premiere arete du chainage du hachage h
c           cette arete ne peut etre consideree dans le chainage des vides
c           car le chainage du hachage doit etre conserve (sinon perte...)
c
         endif
c
c        le temoin d'arete vide
         nosoar( 1, noar ) = 0
      endif
      end


      subroutine caetoi( noar,   mosoar, mxsoar, n1soar, nosoar,
     %                   n1aeoc, nbtrar  )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    ajouter (ou retirer) l'arete noar de nosoar de l'etoile
c -----    des aretes simples chainees en position lchain de nosoar
c          detruire du tableau nosoar les aretes doubles
c
c          attention: le chainage lchain de nosoar devient celui des cf
c
c entree :
c --------
c noar   : numero dans le tableau nosoar de l'arete a traiter
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c          attention: mxsoar>3*mxsomm obligatoire!
c
c entrees et sorties:
c -------------------
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c          une arete i de nosoar est vide  <=>  nosoar(1,i)=0
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c n1aeoc : numero dans nosoar de la premiere arete simple de l'etoile
c
c sortie :
c --------
c nbtrar : 1 si arete ajoutee, 2 si arete double supprimee
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c2345x7..............................................................012
      parameter        (lchain=6)
      integer           nosoar(mosoar,mxsoar)
c
c     si    l'arete n'appartient pas aux aretes de l'etoile naetoi
c     alors elle est ajoutee a l'etoile dans naetoi
c     sinon elle est empilee dans npile pour etre detruite ensuite
c           elle est supprimee de l'etoile naetoi
c
      if( nosoar( lchain, noar ) .lt. 0 ) then
c
c        arete de l'etoile vue pour la premiere fois
c        elle est ajoutee au chainage
         nosoar( lchain, noar ) = n1aeoc
c        elle devient la premiere du chainage
         n1aeoc = noar
c        arete simple
         nbtrar = 1
c
      else
c
c        arete double de l'etoile. elle est supprimee du chainage
         na0 = 0
         na  = n1aeoc
c        parcours des aretes chainees jusqu'a trouver l'arete noar
 10      if( na .ne. noar ) then
c           passage a la suivante
            na0 = na
            na  = nosoar( lchain, na )
            goto 10
         endif
c
c        suppression de noar du chainage des aretes simples de l'etoile
         if( na0 .gt. 0 ) then
c           il existe une arete qui precede
            nosoar( lchain, na0 ) = nosoar( lchain, noar )
         else
c           noar est en fait n1aeoc la premiere du chainage
            n1aeoc = nosoar( lchain, noar )
         endif
c        noar n'est plus une arete simple de l'etoile
         nosoar( lchain, noar ) = -1
c
c        destruction du tableau nosoar de l'arete double noar
         call sasoar( noar, mosoar, mxsoar, n1soar, nosoar )
c
c        arete double
         nbtrar = 2
      endif
      end


      subroutine focftr( nbtrcf, notrcf, pxyd,   noarst,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, n1artr, noartr,
     %                   nbarcf, n1arcf, noarcf,
     %                   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    former un contour ferme (cf) avec les aretes simples des
c -----    nbtrcf triangles du tableau notrcf
c          destruction des nbtrcf triangles du tableau noartr
c          destruction des aretes doubles   du tableau nosoar
c
c          attention: le chainage lchain de nosoar devient celui des cf
c
c entrees:
c --------
c nbtrcf : nombre de  triangles du cf a former
c notrcf : numero des triangles dans le tableau noartr
c pxyd   : tableau des coordonnees 2d des points
c          par point : x  y  distance_souhaitee
c
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c          attention: mxsoar>3*mxsomm obligatoire!
c moartr : nombre maximal d'entiers par arete du tableau noartr
c
c entrees et sorties :
c --------------------
c noarst : noarst(i) numero d'une arete de sommet i
c n1soar : numero de la premiere arete vide dans le tableau nosoar
c          une arete i de nosoar est vide  <=>  nosoar(1,i)=0
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = nosoar(1)+nosoar(2)*2
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c
c sorties:
c --------
c nbarcf : nombre d'aretes du cf
c n1arcf : numero d'une arete de chaque contour
c noarcf : numero des aretes de la ligne du contour ferme
c attention: chainage circulaire des aretes
c            les aretes vides pointes par n1arcf(0) ne sont pas chainees
c ierr   :  0 si pas d'erreur
c          14 si les lignes fermees se coupent => donnees a revoir
c          15 si une seule arete simple frontaliere
c          16 si boucle infinie car toutes les aretes simples
c                de la boule sont frontalieres!
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
      parameter        (lchain=6)
      common / unites / lecteu, imprim, nunite(30)
      double precision  pxyd(3,*)
      integer           notrcf(1:nbtrcf)
      integer           nosoar(mosoar,mxsoar),
     %                  noartr(moartr,*),
     %                  n1arcf(0:*),
     %                  noarcf(3,*),
     %                  noarst(*)
c
c     formation des aretes simples du cf autour de l'arete ns1-ns2
c     attention: le chainage lchain du tableau nosoar devient actif
c     ============================================================
c     ici toutes les aretes du tableau nosoar verifient nosoar(lchain,i) = -1
c     ce qui equivaut a dire que l'etoile des aretes simples est vide
c     (initialisation dans le sp insoar puis remise a -1 dans la suite!)
      n1aeoc = 0
c
c     ajout a l'etoile des aretes simples des 3 aretes des triangles a supprimer
c     suppression des triangles de l'etoile pour les aretes simples de l'etoile
      do 10 i=1,nbtrcf
c        ajout ou retrait des 3 aretes du triangle notrcf(i) de l'etoile
         nt = notrcf( i )
         do 5 j=1,3
c           l'arete de nosoar a traiter
            noar = abs( noartr(j,nt) )
            call caetoi( noar,   mosoar, mxsoar, n1soar, nosoar,
     %                   n1aeoc, nbtrar  )
c           si arete simple alors suppression du numero de triangle pour cette a
            if( nbtrar .eq. 1 ) then
               if( nosoar(4,noar) .eq. nt ) then
                  nosoar(4,noar) = nosoar(5,noar)
               endif
               nosoar(5,noar) = -1
c           else
c              l'arete appartient a aucun triangle => elle est vide
c              les positions 4 et 5 servent maintenant aux chainages des vides
            endif
  5      continue
 10   continue
c
c     les aretes simples de l'etoile sont reordonnees pour former une
c     ligne fermee = un contour ferme peripherique de l'etoile encore dit 1 cf
c     ========================================================================
      n1ae00 = n1aeoc
 12   na1    = n1aeoc
c     la premiere arete du contour ferme
      ns0 = nosoar(1,na1)
      ns1 = nosoar(2,na1)
c
c     l'arete est-elle dans le sens direct?
c     recherche de l'arete du triangle exterieur nt d'arete na1
      nt = nosoar(4,na1)
      if( nt .le. 0 ) nt = nosoar(5,na1)
c
c     attention au cas de l'arete initiale frontaliere de no de triangles 0 et -
      if( nt .le. 0 ) then
c        permutation circulaire des aretes simples chainees
c        la premiere arete doit devenir la derniere du chainage,
c        la 2=>1, la 3=>2, ... , la derniere=>l'avant derniere, 1=>derniere
         n1aeoc = nosoar( lchain, n1aeoc )
         if( n1aeoc .eq. n1ae00 ) then
c           attention: boucle infinie si toutes les aretes simples
c           de la boule sont frontalieres!... arretee par ce test
            ierr = 16
            return
         endif
         noar = n1aeoc
         na0  = 0
 14      if( noar .gt. 0 ) then
c           la sauvegarde de l'arete et l'arete suivante
            na0  = noar
            noar = nosoar(lchain,noar)
            goto 14
         endif
         if( na0 .le. 0 ) then
c           une seule arete simple frontaliere
            ierr = 15
            return
         endif
c        le suivant de l'ancien dernier est l'ancien premier
         nosoar(lchain,na0) = na1
c        le nouveau dernier est l'ancien premier
         nosoar(lchain,na1) = 0
         goto 12
      endif
c
c     ici l'arete na1 est l'une des aretes du triangle nt
      do 15 i=1,3
         if( abs(noartr(i,nt)) .eq. na1 ) then
c           c'est l'arete
            if( noartr(i,nt) .gt. 0 ) then
c              elle est parcourue dans le sens indirect de l'etoile
c             (car c'est en fait le triangle exterieur a la boule)
               ns0 = nosoar(2,na1)
               ns1 = nosoar(1,na1)
            endif
            goto 17
         endif
 15   continue
c
c     le 1-er sommet ou arete du contour ferme
 17   n1arcf( 1 ) = 1
c     le nombre de sommets du contour ferme de l'etoile
      nbarcf = 1
c     le premier sommet de l'etoile
      noarcf( 1, nbarcf ) = ns0
c     l'arete suivante du cf
      noarcf( 2, nbarcf ) = nbarcf + 1
c     le numero de cette arete dans le tableau nosoar
      noarcf( 3, nbarcf ) = na1
c     mise a jour du numero d'arete du sommet ns0
      noarst(ns0) = na1
c
cccc     trace de l'arete
ccc      call dvtrar( pxyd, ns0, ns1, ncvert, ncblan )
c
c     l'arete suivante a chainer
      n1aeoc = nosoar( lchain, na1 )
c     l'arete na1 n'est plus dans l'etoile
      nosoar( lchain, na1 ) = -1
c
c     boucle sur les aretes simples de l'etoile
 20   if( n1aeoc .gt. 0 ) then
c
c        recherche de l'arete de 1-er sommet ns1
         na0 = -1
         na1 = n1aeoc
 25      if( na1 .gt. 0 ) then
c
c           le numero du dernier sommet de l'arete precedente
c           est il l'un des 2 sommets de l'arete na1?
            if ( ns1 .eq. nosoar(1,na1) ) then
c               l'autre sommet de l'arete na1
                ns2 = nosoar(2,na1)
            else if( ns1 .eq. nosoar(2,na1) ) then
c               l'autre sommet de l'arete na1
                ns2 = nosoar(1,na1)
            else
c              non: passage a l'arete suivante
               na0 = na1
               na1 = nosoar( lchain, na1 )
               goto 25
            endif
c
c           oui: na1 est l'arete peripherique suivante
c                na0 est sa precedente dans le chainage
c           une arete de plus dans le contour ferme (cf)
            nbarcf = nbarcf + 1
c           le premier sommet de l'arete nbarcf peripherique
            noarcf( 1, nbarcf ) = ns1
c           l'arete suivante du cf
            noarcf( 2, nbarcf ) = nbarcf + 1
c           le numero de cette arete dans le tableau nosoar
            noarcf( 3, nbarcf ) = na1
c           mise a jour du numero d'arete du sommet ns1
            noarst(ns1) = na1
c
cccc           trace de l'arete
ccc            call dvtrar( pxyd, ns1, ns2, ncvert, ncblan )
c
c           suppression de l'arete des aretes simples de l'etoile
            if( n1aeoc .eq. na1 ) then
                n1aeoc = nosoar( lchain, na1 )
            else
                nosoar( lchain, na0 ) = nosoar( lchain, na1 )
            endif
c           l'arete n'est plus une arete simple de l'etoile
            nosoar( lchain, na1 ) = -1
c
c           le sommet final de l'arete a rechercher ensuite
            ns1 = ns2
            goto 20
         endif
      endif
c
c     verification
      if( ns1 .ne. ns0 ) then
c        arete non retrouvee : l'etoile ne se referme pas
c         nblgrc(nrerr) = 3
c         kerr(1) = 'focftr: revoyez vos donnees'
c         kerr(2) = 'les lignes fermees doivent etre disjointes'
c         kerr(3) = 'verifiez si elles ne se coupent pas'
c         call lereur
          write(imprim,*) 'focftr: revoyez vos donnees'
          write(imprim,*)'les lignes fermees doivent etre disjointes'
          write(imprim,*)'verifiez si elles ne se coupent pas'
         ierr = 14
         return
      endif
c
c     l'arete suivant la derniere arete du cf est la premiere du cf
c     => realisation d'un chainage circulaire des aretes du cf
      noarcf( 2, nbarcf ) = 1
c
c     destruction des triangles de l'etoile du tableau noartr
c     -------------------------------------------------------
      do 50 i=1,nbtrcf
c        le numero du triangle dans noartr
         nt0 = notrcf( i )
c        l'arete 1 de nt0 devient nulle
         noartr( 1, nt0 ) = 0
c        chainage de nt0 en tete du chainage des triangles vides de noartr
         noartr( 2, nt0 ) = n1artr
         n1artr = nt0
 50   continue
      end


      subroutine int1sd( ns1, ns2, ns3, ns4, pxyd, linter, x0, y0 )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    existence ou non  d'une intersection a l'interieur
c -----    des 2 aretes ns1-ns2 et ns3-ns4
c          attention les intersections au sommet sont comptees
c
c entrees:
c --------
c ns1,...ns4 : numero pxyd des 4 sommets
c pxyd   : les coordonnees des sommets
c
c sortie :
c --------
c linter : -1 si ns3-ns4 parallele a ns1 ns2
c           0 si ns3-ns4 n'intersecte pas ns1-ns2 entre les aretes
c           1 si ns3-ns4   intersecte     ns1-ns2 entre les aretes
c           2 si le point d'intersection est ns1  entre ns3-ns4
c           3 si le point d'intersection est ns3  entre ns1-ns2
c           4 si le point d'intersection est ns4  entre ns1-ns2
c x0,y0  :  2 coordonnees du point d'intersection s'il existe(linter>=1)
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc    fevrier 1992
c2345x7..............................................................012
      parameter        ( epsmoi=-0.000001d0, eps=0.001d0,
     %                   unmeps= 0.999d0, unpeps=1.000001d0 )
      double precision  pxyd(3,*), x0, y0
      double precision  x1,y1,x21,y21,d21,x43,y43,d43,d,x,y,p21,p43
c
      x1  = pxyd(1,ns1)
      y1  = pxyd(2,ns1)
      x21 = pxyd(1,ns2) - x1
      y21 = pxyd(2,ns2) - y1
      d21 = x21**2 + y21**2
c
      x43 = pxyd(1,ns4) - pxyd(1,ns3)
      y43 = pxyd(2,ns4) - pxyd(2,ns3)
      d43 = x43**2 + y43**2
c
c     les 2 aretes sont-elles jugees paralleles ?
      d = x43 * y21 - y43 * x21
      if( d*d .le. 0.000001d0 * d21 * d43 ) then
c        cote i parallele a ns1-ns2
         linter = -1
         return
      endif
c
c     les 2 coordonnees du point d'intersection
      x =( x1*x43*y21-pxyd(1,ns3)*x21*y43-(y1-pxyd(2,ns3))*x21*x43)/d
      y =(-y1*y43*x21+pxyd(2,ns3)*y21*x43+(x1-pxyd(1,ns3))*y21*y43)/d
c
c     coordonnee barycentrique de x,y dans le repere ns1-ns2
      p21 = ( ( x - x1 )       * x21 + ( y - y1 )        * y21 ) / d21
c     coordonnee barycentrique de x,y dans le repere ns3-ns4
      p43 = ( (x - pxyd(1,ns3))* x43 + (y - pxyd(2,ns3)) * y43 ) / d43
c
c
      if( epsmoi .le. p21 .and. p21 .le. unpeps ) then
c        x,y est entre ns1-ns2
         if( (p21 .le. eps)  .and.
     %       (epsmoi .le. p43 .and. p43 .le. unpeps) ) then
c           le point x,y est proche de ns1 et interne a ns3-ns4
            linter = 2
            x0 = pxyd(1,ns1)
            y0 = pxyd(2,ns1)
            return
         else if( epsmoi .le. p43 .and. p43 .le. eps ) then
c           le point x,y est proche de ns3 et entre ns1-ns2
            linter = 3
            x0 = pxyd(1,ns3)
            y0 = pxyd(2,ns3)
            return
         else if( unmeps .le. p43 .and. p43 .le. unpeps ) then
c           le point x,y est proche de ns4 et entre ns1-ns2
            linter = 4
            x0 = pxyd(1,ns4)
            y0 = pxyd(2,ns4)
            return
         else if( eps .le. p43 .and. p43 .le. unmeps ) then
c           le point x,y est entre ns3-ns4
            linter = 1
            x0     = x
            y0     = y
            return
         endif
      endif
c
c     pas d'intersection a l'interieur des aretes
      linter = 0
      end


      subroutine tefoar( narete, nbarpi, pxyd,
     %                   mosoar, mxsoar, n1soar, nosoar,
     %                   moartr, n1artr, noartr, noarst,
     %                   mxarcf, n1arcf, noarcf, larmin, notrcf,
     %                   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :   forcer l'arete narete de nosoar dans la triangulation actuelle
c -----   triangulation frontale pour la reobtenir
c
c         attention: le chainage lchain(=6) de nosoar devient actif
c                    durant la formation des contours fermes (cf)
c
c entrees:
c --------
c narete : numero nosoar de l'arete frontaliere a forcer
c nbarpi : numero du dernier point interne impose par l'utilisateur
c pxyd   : tableau des coordonnees 2d des points
c          par point : x  y  distance_souhaitee
c
c mosoar : nombre maximal d'entiers par arete et
c          indice dans nosoar de l'arete suivante dans le hachage
c mxsoar : nombre maximal d'aretes stockables dans le tableau nosoar
c          attention: mxsoar>3*mxsomm obligatoire!
c moartr : nombre maximal d'entiers par arete du tableau noartr
c
c modifies:
c ---------
c n1soar : no de l'eventuelle premiere arete libre dans le tableau nosoar
c          chainage des vides suivant en 3 et precedant en 2 de nosoar
c nosoar : numero des 2 sommets , no ligne, 2 triangles de l'arete,
c          chainage des aretes frontalieres, chainage du hachage des aretes
c          hachage des aretes = nosoar(1)+nosoar(2)*2
c          avec mxsoar>=3*mxsomm
c          une arete i de nosoar est vide <=> nosoar(1,i)=0 et
c          nosoar(2,arete vide)=l'arete vide qui precede
c          nosoar(3,arete vide)=l'arete vide qui suit
c n1artr : numero du premier triangle vide dans le tableau noartr
c          le chainage des triangles vides se fait sur noartr(2,.)
c noartr : les 3 aretes des triangles +-arete1, +-arete2, +-arete3
c          arete1 = 0 si triangle vide => arete2 = triangle vide suivant
c noarst : noarst(i) numero d'une arete de sommet i
c
c mxarcf : nombre de variables des tableaux n1arcf, noarcf, larmin, notrcf
c
c tableaux auxiliaires :
c ----------------------
c n1arcf : tableau (0:mxarcf) auxiliaire
c noarcf : tableau (3,mxarcf) auxiliaire
c larmin : tableau (mxarcf)   auxiliaire
c notrcf : tableau (1:mxarcf) auxiliaire
c
c sortie :
c --------
c ierr   : 0 si pas d'erreur
c          1 saturation des sommets
c          2 ns1 dans aucun triangle
c          9 tableau nosoar de taille insuffisante car trop d'aretes
c            a probleme
c          10 un des tableaux n1arcf, noarcf notrcf est sature
c             augmenter a l'appel mxarcf
c          11 algorithme defaillant
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc       mars 1997
c....................................................................012
      parameter        (mxpitr=32)
      common / unites / lecteu,imprim,intera,nunite(29)
      logical           tratri
      common / dv2dco / tratri
      double precision  pxyd(3,*)
      integer           noartr(moartr,*),
     %                  nosoar(mosoar,mxsoar),
     %                  noarst(*),
     %                  n1arcf(0:mxarcf),
     %                  noarcf(3,mxarcf),
     %                  larmin(mxarcf),
     %                  notrcf(mxarcf)
c
      integer           lapitr(mxpitr)
      double precision  x1,y1,x2,y2,d12,d3,d4,x,y,d,dmin
      integer           nosotr(3), ns(2)
      integer           nacf(1:2), nacf1, nacf2
      equivalence      (nacf(1),nacf1), (nacf(2),nacf2)
c
c     traitement de cette arete perdue
      ns1 = nosoar( 1, narete )
      ns2 = nosoar( 2, narete )
c
      if( tratri ) then
c        les traces sont demandes
c         call efface
c        le cadre objet global en unites utilisateur
         xx1 = min( pxyd(1,ns1), pxyd(1,ns2) )
         xx2 = max( pxyd(1,ns1), pxyd(1,ns2) )
         yy1 = min( pxyd(2,ns1), pxyd(2,ns2) )
         yy2 = max( pxyd(2,ns1), pxyd(2,ns2) )
         if( xx1 .ge. xx2 ) xx2 = xx1 + (yy2-yy1)
         if( yy1 .ge. yy2 ) yy2 = yy1 + (xx2-xx1)*0.5
c         call isofenetre( xx1-(xx2-xx1), xx2+(xx2-xx1),
c     %                    yy1-(yy2-yy1), yy2+(yy2-yy1) )
      endif
c
cccc     trace de l'arete perdue
ccc      call dvtrar( pxyd, ns1, ns2, ncroug, ncblan )
c
c     le sommet ns2 est il correct?
      na = noarst( ns2 )
      if( na .le. 0 ) then
         write(imprim,*) 'tefoar: erreur sommet ',ns2,' sans arete'
         ierr = 8
         return
      endif
      if( nosoar(4,na) .le. 0 ) then
         write(imprim,*) 'tefoar: erreur sommet ',ns2,
     %                   ' dans aucun triangle'
         ierr = 8
         return
      endif
c
c     recherche du triangle voisin dans le sens indirect de rotation
      nsens = -1
c     le premier passage: recherche dans le sens ns1->ns2
      ipas = 0
c
c     recherche des triangles intersectes par le segment ns1-ns2
c     ==========================================================
 3    x1  = pxyd(1,ns1)
      y1  = pxyd(2,ns1)
      x2  = pxyd(1,ns2)
      y2  = pxyd(2,ns2)
      d12 = (x2-x1)**2 + (y2-y1)**2
c
c     recherche du no local du sommet ns1 dans l'un de ses triangles
      na01 = noarst( ns1 )
      if( na01 .le. 0 ) then
         write(imprim,*) 'tefoar: sommet ',ns1,' sans arete'
         ierr = 8
         return
      endif
      nt0 = nosoar(4,na01)
      if( nt0 .le. 0 ) then
         write(imprim,*) 'tefoar: sommet ',ns1,' dans aucun triangle'
         ierr = 8
         return
      endif
c
c     le numero des 3 sommets du triangle nt0 dans le sens direct
 20   call nusotr( nt0, mosoar, nosoar, moartr, noartr, nosotr )
      do 22 na00=1,3
         if( nosotr(na00) .eq. ns1 ) goto 26
 22   continue
c
 25   if( ipas .eq. 0 ) then
c        le second passage: recherche dans le sens ns2->ns1
c        tentative d'inversion des 2 sommets extremites de l'arete a forcer
         na00 = ns1
         ns1  = ns2
         ns2  = na00
         ipas = 1
         goto 3
      else
c        les sens ns1->ns2 et ns2->ns1 ne donne pas de solution!
         write(imprim,*)'tefoar:arete ',ns1,' - ',ns2,' a imposer'
         write(imprim,*)'tefoar:anomalie sommet ',ns1,
     %   'non dans le triangle de sommets ',(nosotr(i),i=1,3)
         ierr = 11
         return
      endif
c
c     le numero des aretes suivante et precedente
 26   na0 = nosui3( na00 )
      na1 = nopre3( na00 )
      ns3 = nosotr( na0 )
      ns4 = nosotr( na1 )
c
cccc     trace du triangle nt0 et de l'arete perdue
ccc      call mttrtr( pxyd, nt0, moartr, noartr, mosoar, nosoar,
ccc     %             ncblan, ncjaun )
ccc      call dvtrar( pxyd, ns1, ns2, ncroug, ncblan )
ccc      call dvtrar( pxyd, ns3, ns4, ncbleu, nccyan )
c
c     point d'intersection du segment ns1-ns2 avec l'arete ns3-ns4
c     ------------------------------------------------------------
      call int1sd( ns1, ns2, ns3, ns4, pxyd, linter, x1, y1 )
      if( linter .le. 0 ) then
c
c        pas d'intersection: rotation autour du point ns1
c        pour trouver le triangle de l'autre cote de l'arete na01
         if( nsens .lt. 0 ) then
c           sens indirect de rotation: l'arete de sommet ns1
            na01 = abs( noartr(na00,nt0) )
         else
c           sens direct de rotation: l'arete de sommet ns1 qui precede
            na01 = abs( noartr(na1,nt0) )
         endif
c        le triangle de l'autre cote de l'arete na01
         if( nosoar(4,na01) .eq. nt0 ) then
            nt0 = nosoar(5,na01)
         else
            nt0 = nosoar(4,na01)
         endif
         if( nt0 .gt. 0 ) goto 20
c
c        le parcours sort du domaine
c        il faut tourner dans l'autre sens autour de ns1
         if( nsens .lt. 0 ) then
            nsens = 1
            nt0   = noarst( ns1 )
            goto 20
         endif
c
c        dans les 2 sens, pas d'intersection => impossible
c        essai avec l'arete inversee ns1 <-> ns2
         if( ipas .eq. 0 ) goto 25
         write(imprim,*) 'tefoar: arete ',ns1,' ',ns2,
     %  ' sans intersection avec les triangles actuels'
         write(imprim,*) 'revoyez les lignes du contour'
         ierr = 11
         return
      endif
c
c     il existe une intersection avec l'arete opposee au sommet ns1
c     =============================================================
c     nbtrcf : nombre de triangles du cf
      nbtrcf = 1
      notrcf( 1 ) = nt0
c
c     le triangle oppose a l'arete na0 de nt0
 30   noar = abs( noartr(na0,nt0) )
      if( nosoar(4,noar) .eq. nt0 ) then
         nt1 = nosoar(5,noar)
      else
         nt1 = nosoar(4,noar)
      endif
c
cccc     trace du triangle nt1 et de l'arete perdue
ccc      call mttrtr( pxyd, nt1, moartr, noartr, mosoar, nosoar,
ccc     %             ncjaun, ncmage )
ccc      call dvtrar( pxyd, ns1, ns2, ncroug, ncblan )
c
c     le numero des 3 sommets du triangle nt1 dans le sens direct
      call nusotr( nt1, mosoar, nosoar, moartr, noartr, nosotr )
c
c     le triangle nt1 contient il ns2 ?
      do 32 j=1,3
         if( nosotr(j) .eq. ns2 ) goto 70
 32   continue
c
c     recherche de l'arete noar, na1 dans nt1 qui est l'arete na0 de nt0
      do 34 na1=1,3
         if( abs( noartr(na1,nt1) ) .eq. noar ) goto 35
 34   continue
c
c     trace du triangle nt1 et de l'arete perdue
 35   continue
ccc 35   call mttrtr( pxyd, nt1, moartr, noartr, mosoar, nosoar,
ccc     %             ncjaun, ncmage )
ccc      call dvtrar( pxyd, ns1, ns2, ncroug, ncblan )
c
c     recherche de l'intersection de ns1-ns2 avec les 2 autres aretes de nt1
c     ======================================================================
      na2 = na1
      do 50 i1 = 1,2
c        l'arete suivante
         na2 = nosui3(na2)
c
c        les 2 sommets de l'arete na2 de nt1
         noar = abs( noartr(na2,nt1) )
         ns3  = nosoar( 1, noar )
         ns4  = nosoar( 2, noar )
ccc         call dvtrar( pxyd, ns3, ns4, ncbleu, nccyan )
c
c        point d'intersection du segment ns1-ns2 avec l'arete ns3-ns4
c        ------------------------------------------------------------
         call int1sd( ns1, ns2, ns3, ns4, pxyd, linter, x , y )
         if( linter .gt. 0 ) then
c
c           les 2 aretes s'intersectent en (x,y)
c           distance de (x,y) a ns3 et ns4
            d3 = (pxyd(1,ns3)-x)**2 + (pxyd(2,ns3)-y)**2
            d4 = (pxyd(1,ns4)-x)**2 + (pxyd(2,ns4)-y)**2
c           nsp est le point le plus proche de (x,y)
            if( d3 .lt. d4 ) then
               nsp = ns3
               d   = d3
            else
               nsp = ns4
               d   = d4
            endif
            if( d .gt. 1d-5*d12 ) goto 60
c
c           ici le sommet nsp est trop proche de l'arete perdue ns1-ns2
            if( nsp .le. nbarpi ) then
c              point utilisateur ou frontalier non supprimable
               ierr = 11
               write(imprim,*) 'pause dans tefoar 1', d, d3, d4, d12
               return
            endif
c
c           le sommet interne nsp est supprime en mettant tous les triangles
c           l'ayant comme sommet dans la pile notrcf des triangles a supprimer
c           ------------------------------------------------------------------
ccc            write(imprim,*) 'tefoar: le sommet ',nsp,' est supprime'
c           construction de la liste des triangles de sommet nsp
            call trp1st( nsp, noarst, mosoar, nosoar, moartr, noartr,
     %                   mxpitr, nbt, lapitr )
            if( nbt .le. 0 ) then
c              les triangles de sommet nsp ne forme pas une "boule"
c              avec ce sommet nsp pour "centre"
               write(imprim,*)
     %        'tefoar: pas d''etoile de triangles autour du sommet',nsp
cccc              trace des triangles de l'etoile du sommet nsp
ccc               tratri = .true.
ccc               call trpltr( nbt,    lapitr, pxyd,
ccc     %                      moartr, noartr, mosoar, nosoar,
ccc     %                      ncroug, ncblan )
ccc               tratri = .false.
               ierr = 11
               write(imprim,*) 'pause dans tefoar 2'
               return
            endif
c
c           ajout des triangles de sommet ns1 a notrcf
            nbtrc0 = nbtrcf
            do 38 j=1,nbt
               nt = lapitr(j)
               do 37 k=nbtrcf,1,-1
                  if( nt .eq. notrcf(k) ) goto 38
 37            continue
c              triangle ajoute
               nbtrcf = nbtrcf + 1
               notrcf( nbtrcf ) = nt
ccc               call mttrtr( pxyd, nt, moartr, noartr, mosoar, nosoar,
ccc     %                      ncjaun, ncmage )
ccc               call dvtrar( pxyd, ns1, ns2, ncroug, ncblan )
 38         continue
c
c           ce sommet supprime n'appartient plus a aucun triangle
            noarst( nsp ) = 0
c
c           ns2 est-il un sommet des triangles empiles?
c           -------------------------------------------
            do 40 nt=nbtrc0+1,nbtrcf
c              le triangle a supprimer nt
               nt1 = notrcf( nt )
c              le numero des 3 sommets du triangle nt1 dans le sens direct
               call nusotr( nt1, mosoar, nosoar, moartr, noartr, nosotr)
               do 39 k=1,3
c                 le sommet k de nt1
                  if( nosotr( k ) .eq. ns2 ) then
c                    but atteint
                     goto 80
                  endif
 39            continue
 40         continue
c
c           recherche du plus proche point d'intersection de ns1-ns2
c           par rapport a ns2 avec les aretes des triangles ajoutes
            nt0  = 0
            dmin = d12 * 10000
            do 48 nt=nbtrc0+1,nbtrcf
               nt1 = notrcf( nt )
c              le numero des 3 sommets du triangle nt1 dans le sens direct
               call nusotr( nt1, mosoar, nosoar, moartr, noartr, nosotr)
               do 45 k=1,3
c                 les 2 sommets de l'arete k de nt
                  ns3 = nosotr( k )
                  ns4 = nosotr( nosui3(k) )
c
c                 point d'intersection du segment ns1-ns2 avec l'arete ns3-ns4
c                 ------------------------------------------------------------
                  call int1sd( ns1, ns2, ns3, ns4, pxyd,
     %                         linter, x , y )
                  if( linter .gt. 0 ) then
c                    les 2 aretes s'intersectent en (x,y)
                     d = (x-x2)**2+(y-y2)**2
                     if( d .lt. dmin ) then
                        nt0  = nt1
                        na0  = k
                        dmin = d
                     endif
                  endif
 45            continue
 48         continue
c
c           redemarrage avec le triangle nt0 et l'arete na0
            if( nt0 .gt. 0 ) goto 30
c
            write(imprim,*) 'tefoar: algorithme defaillant'
            ierr = 11
            return
         endif
 50   continue
c
c     pas d'intersection differente de l'initiale => sommet sur ns1-ns2
c     rotation autour du sommet par l'arete suivant na1
      write(imprim,*)
      write(imprim,*) 'tefoar 50: revoyez vos donnees'
      write(imprim,*) 'les lignes fermees doivent etre disjointes'
      write(imprim,*) 'verifiez si elles ne se coupent pas'
      ierr = 13
      return
c
c     cas sans probleme : intersection differente de celle initiale
c     =================   =========================================
 60   nbtrcf = nbtrcf + 1
      notrcf( nbtrcf ) = nt1
c     passage au triangle suivant
      na0 = na2
      nt0 = nt1
      goto 30
c
c     ----------------------------------------------------------
c     ici toutes les intersections de ns1-ns2 ont ete parcourues
c     tous les triangles intersectes ou etendus forment les
c     nbtrcf triangles du tableau notrcf
c     ----------------------------------------------------------
 70   nbtrcf = nbtrcf + 1
      notrcf( nbtrcf ) = nt1
c
c     formation du cf des aretes simples des triangles de notrcf
c     et destruction des nbtrcf triangles du tableau noartr
c     attention: le chainage lchain du tableau nosoar devient actif
c     =============================================================
 80   if( nbtrcf*3 .gt. mxarcf ) then
         write(imprim,*) 'saturation du tableau noarcf'
         ierr = 10
         return
      endif
c
      call focftr( nbtrcf, notrcf, pxyd,   noarst,
     %             mosoar, mxsoar, n1soar, nosoar,
     %             moartr, n1artr, noartr,
     %             nbarcf, n1arcf, noarcf,
     %             ierr )
      if( ierr .ne. 0 ) return
c
c     chainage des aretes vides dans le tableau noarcf
c     ------------------------------------------------
c     decalage de 2 aretes car 2 aretes sont necessaires ensuite pour
c     integrer 2 fois l'arete perdue et former ainsi 2 cf
c     comme nbtrcf*3 minore mxarcf il existe au moins 2 places vides
c     derriere => pas de test de debordement
      n1arcf(0) = nbarcf+3
      mmarcf = min(8*nbarcf,mxarcf)
      do 90 i=nbarcf+3,mmarcf
         noarcf(2,i) = i+1
 90   continue
      noarcf(2,mmarcf) = 0
c
c     reperage des sommets ns1 ns2 de l'arete perdue dans le cf
c     ---------------------------------------------------------
      ns1   = nosoar( 1, narete )
      ns2   = nosoar( 2, narete )
      ns(1) = ns1
      ns(2) = ns2
      do 120 i=1,2
c        la premiere arete dans noarcf du cf
         na0 = n1arcf(1)
 110     if( noarcf(1,na0) .ne. ns(i) ) then
c           passage a l'arete suivante
            na0 = noarcf( 2, na0 )
            goto 110
         endif
c        position dans noarcf du sommet i de l'arete perdue
         nacf(i) = na0
 120  continue
c
c     formation des 2 cf chacun contenant l'arete ns1-ns2
c     ---------------------------------------------------
c     sauvegarde de l'arete suivante de celle de sommet ns1
      na0 = noarcf( 2, nacf1 )
      nt1 = noarcf( 3, nacf1 )
c
c     le premier cf
      n1arcf( 1 ) = nacf1
c     l'arete suivante dans le premier cf
      noarcf( 2, nacf1 ) = nacf2
c     cette arete est celle perdue
      noarcf( 3, nacf1 ) = narete
c
c     le second cf
c     l'arete doublee
      n1 = nbarcf + 1
      n2 = nbarcf + 2
c     le premier sommet de la premiere arete du second cf
      noarcf( 1, n1 ) = ns2
c     l'arete suivante dans le second cf
      noarcf( 2, n1 ) = n2
c     cette arete est celle perdue
      noarcf( 3, n1 ) = narete
c     la seconde arete du second cf
      noarcf( 1, n2 ) = ns1
      noarcf( 2, n2 ) = na0
      noarcf( 3, n2 ) = nt1
      n1arcf( 2 ) = n1
c
c     recherche du precedent de nacf2
 130  na1 = noarcf( 2, na0 )
      if( na1 .ne. nacf2 ) then
c        passage a l'arete suivante
         na0 = na1
         goto 130
      endif
c     na0 precede nacf2 => il precede n1
      noarcf( 2, na0 ) = n1
c
c     depart avec 2 cf
      nbcf   = 2
c
c     triangulation directe des 2 contours fermes
c     l'arete ns1-ns2 devient une arete de la triangulation des 2 cf
c     ==============================================================
      call tridcf( nbcf,   pxyd,   noarst,
     %             mosoar, mxsoar, n1soar, nosoar,
     %             moartr, n1artr, noartr,
     %             mxarcf, n1arcf, noarcf, larmin,
     %             nbtrcf, notrcf, ierr )
      end


      subroutine te4ste( nbsomm, mxsomm, pxyd, ntrp, letree,
     &                   ierr )
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c but :    decouper un te ntrp de letree en 4 sous-triangles
c -----    eliminer les sommets de te trop proches des points
c
c entrees:
c --------
c mxsomm : nombre maximal de points declarables dans pxyd
c ntrp   : numero letree du triangle a decouper en 4 sous-triangles
c
c modifies :
c ----------
c nbsomm : nombre actuel de points dans pxyd
c pxyd   : tableau des coordonnees des points
c          par point : x  y  distance_souhaitee
c letree : arbre-4 des triangles equilateraux (te) fond de la triangulation
c      letree(0,0) :  no du 1-er te vide dans letree
c      letree(0,1) : maximum du 1-er indice de letree (ici 8)
c      letree(0,2) : maximum declare du 2-eme indice de letree (ici mxtree)
c      letree(0:8,1) : racine de l'arbre  (triangle sans sur triangle)
c      si letree(0,.)>0 alors
c         letree(0:3,j) : no (>0) letree des 4 sous-triangles du triangle j
c      sinon
c         letree(0:3,j) :-no pxyd des 1 a 4 points internes au triangle j
c                         0  si pas de point
c                        ( j est alors une feuille de l'arbre )
c      letree(4,j) : no letree du sur-triangle du triangle j
c      letree(5,j) : 0 1 2 3 no du sous-triangle j pour son sur-triangle
c      letree(6:8,j) : no pxyd des 3 sommets du triangle j
c
c sorties :
c ---------
c ierr    : 0 si pas d'erreur, 51 saturation letree, 52 saturation pxyd
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c auteur : alain perronnet  analyse numerique paris upmc    juillet 1994
c2345x7..............................................................012
      common / unites / lecteu,imprim,nunite(30)
      integer           letree(0:8,0:*)
      double precision  pxyd(3,mxsomm)
      integer           np(0:3),milieu(3)
c
c     debut par l'arete 2 du triangle ntrp
      i1 = 2
      i2 = 3
      do 30 i=1,3
c
c        le milieu de l'arete i1 existe t il deja ?
         call n1trva( ntrp, i1, letree, noteva, niveau )
         if( noteva .gt. 0 ) then
c           il existe un te voisin
c           s'il existe 4 sous-triangles le milieu existe deja
            if( letree(0,noteva) .gt. 0 ) then
c              le milieu existe
               nsot = letree(0,noteva)
               milieu(i) = letree( 5+nopre3(i1), nsot )
               goto 25
            endif
         endif
c
c        le milieu n'existe pas. il est cree
         nbsomm = nbsomm + 1
         if( nbsomm .gt. mxsomm ) then
c           plus assez de place dans pxyd
            write(imprim,*) 'te4ste: saturation pxyd'
            write(imprim,*)
            ierr = 52
            return
         endif
c        le milieu de l'arete i
         milieu(i) = nbsomm
c
c        ntrp est le triangle de milieux d'arete ces 3 sommets
         ns1    = letree( 5+i1, ntrp )
         ns2    = letree( 5+i2, ntrp )
         pxyd(1,nbsomm) = ( pxyd(1,ns1) + pxyd(1,ns2) ) * 0.5
         pxyd(2,nbsomm) = ( pxyd(2,ns1) + pxyd(2,ns2) ) * 0.5
c
c        l'arete et milieu suivant
 25      i1 = i2
         i2 = nosui3( i2 )
 30   continue
c
      do 50 i=0,3
c
c        le premier triangle vide
         nsot = letree(0,0)
         if( nsot .le. 0 ) then
c           manque de place. saturation letree
            ierr = 51
            write(imprim,*) 'te4ste: saturation letree'
            write(imprim,*)
            return
         endif
c
c        mise a jour du premier te libre
         letree(0,0) = letree(0,nsot)
c
c        nsot est le i-eme sous triangle
         letree(0,nsot) = 0
         letree(1,nsot) = 0
         letree(2,nsot) = 0
         letree(3,nsot) = 0
c
c        le numero des points et sous triangles dans ntrp
         np(i) = -letree(i,ntrp)
         letree(i,ntrp) = nsot
c
c        le sommet commun avec le triangle ntrp
         letree(5+i,nsot) = letree(5+i,ntrp)
c
c        le sur-triangle et numero de sous-triangle de nsot
c        a laisser ici car incorrect sinon pour i=0
         letree(4,nsot) = ntrp
         letree(5,nsot) = i
c
c        le sous-triangle du triangle
         letree(i,ntrp) = nsot
 50   continue
c
c     le numero des nouveaux sommets milieux
      nsot = letree(0,ntrp)
      letree(6,nsot) = milieu(1)
      letree(7,nsot) = milieu(2)
      letree(8,nsot) = milieu(3)
c
      nsot = letree(1,ntrp)
      letree(7,nsot) = milieu(3)
      letree(8,nsot) = milieu(2)
c
      nsot = letree(2,ntrp)
      letree(6,nsot) = milieu(3)
      letree(8,nsot) = milieu(1)
c
      nsot = letree(3,ntrp)
      letree(6,nsot) = milieu(2)
      letree(7,nsot) = milieu(1)
c
c     repartition des eventuels 4 points np dans ces 4 sous-triangles
c     il y a obligatoirement suffisamment de place
      do 110 i=0,3
         if( np(i) .gt. 0 ) then
            nsot = notrpt( pxyd(1,np(i)), pxyd, ntrp, letree )
c           ajout du point
            do 100 i1=0,3
               if( letree(i1,nsot) .eq. 0 ) then
c                 place libre a occuper
                  letree(i1,nsot) = -np(i)
                  goto 110
               endif
 100        continue
         endif
 110  continue
      end
