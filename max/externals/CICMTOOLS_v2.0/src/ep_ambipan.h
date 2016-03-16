
#ifndef DEF_EP_AMBIPAN
#define DEF_EP_AMBIPAN

/**********************************************************************/
//                                                                    //
// /****************************************************************/ //
// /*                                                              */ //
// /*                         POLY.AMBIPAN~                        */ //
// /*                                                              */ //
// /* Auteur: Eliott PARIS                                         */ //
// /*         Université Paris8.								   */ //
// /*                                                              */ //
// /* Date de creation:   07/04/12                                 */ //
// /*                                                              */ //
// /* Réalisé à partir du code l'objet ambipan~ développé par	   */ //
// /*		  R.Mignot à la MSH Paris Nord (Maison des Sciences	   */ //
// /*		  de l'Homme)                                          */ //
// /*         en collaboration avec A.Sedes, B.Courribet           */ //
// /*         et J.B.Thiebaut,                                     */ //
// /*         CICM Université Paris8, MSH Paris Nord,              */ //
// /*         ACI Jeunes Chercheurs "Espaces Sonores".             */ //
// /*                                                              */ //
// /****************************************************************/ //
//                                                                    //
/**********************************************************************/


/**
 * Copyright (C) 2003-2004 RÈmi Mignot, MSH Paris Nord, Eliott PARIS 2012.
 * 
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU Library General Public License as published 
 * by the Free Software Foundation; either version 2 of the License.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public 
 * License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License 
 * along with this library; if not, write to the Free Software Foundation, 
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * www.mshparisnord.org
 * rmignot@mshparisnord.org
 * eliott.paris@yahoo.fr
 */


/*Description:  Cet objet permet de spatialiser plusieurs sources sonores
 à l'aide de N haut-parleurs situés en cercle autour
 de l'auditeur. La spatialisation se fait grâce à
 l'ambisonie de Michael Gerzon.                        */

class ep_ambipan {
	
private:
	
	static int const Nmax;    //Nombre maximum de haut-parleurs,
	static int const Ndefaut; //Nombre de haut-parleurs par défaut,
	static int const Xdefaut; //Valeur par defaut de l'absisse,
	static int const Ydefaut; //Valeur par défaut de l'ordonnée,
	static int const Rdefaut; //Distance par défaut de la source,
	static int const T_COS;   //Taille du tableau cosinus (puissance de 2),
	static int const MASQUE;  //Masque pour le modulo = T_COS - 1,
	static int const DTIME;   //Temps d'interpolation par dÈfaut, //en Èchantillons (10ms),
	static double const Phidefaut; //Valeur par défaut de l'angle (PI/2),
	static double const Pi;   // Pi
	static double const I360; // 1/360
	static double const OFFSET;  //Offset par defaut de l'ambisonie,
	static double const EPSILON; //CritËre pour tester si les flottants sont nuls
	
	
	int     base;         //Type de base, 1->cartésienne, 0->polaire,
	int     mute;         //1->entrées signal mutées, 0->non mutées,
	int     Nout;         //Nombre de sorties,
	int     N;            //Nombre de haut-parleurs.
	
	float   x, y;         //Coordonnées de la source en cartésien,
	float   phi, r;       //Coordonnées polaires de la source,
	float   c1, c2;       // coordonnées entrées dans l'inlet 1 et 2 (en contrôle ou signal);
	
	double  offset;       //Offset de l'ambisonie,
	int     dtime;        //Temps d'interpolation en échantillons,
	
	float   *P;      //Tableau contenant les N coefficients ambisoniques Pn,
	float   *teta;   //Angle de chaque haut-parleur,
	float   *dist;   //Distance des haut-parleurs,
	
	float   *dP;     //pas pour l'interpolation,
	float   *Pstop;  //cible pour l'interpolation,
	
	float   *cosin;       //Adresse des tableaux pour les cosinus,
	float   *cos_teta;    //les sinus et cosinus des angles des 
	float   *sin_teta;    //haut-parleurs.
	
	double  *m_output;
	
public:
	/**
	 * Create an Ampipan object.
	 * @param nbLoudSpeaker : (int) the number of loudSpeaker (4 by default).
	 * @param base :		  (int) coordinate base (1 for Cartesian, 0 for Polar, 1 by default).
	 * @param offset :		  (double) initialise the offset.
	 * @param Dtime :		  (double) initialise the interpolation time.
	 * @see setNbLoudSpeaker, setRepereType, setOffset
	 */
	ep_ambipan(int nbLoudSpeaker = Ndefaut, int baseCartOrPol = 1, double initOffset = OFFSET, double initDtime = DTIME);
	~ep_ambipan();
	
	//------ METHODES ------//
	/**
	 * Ambipan process function (signal in).
	 * @param sampleInput : sample input.
	 * @param coord_x :		coordinate X.
 	 * @param coord_y :		coordinate Y.
	 */
	double *output(double sampleInput, double coord_x, double coord_y);
	
	/**
	 * Ambipan process function (drive by control values).
	 * @param sampleInput : sample input.
	 * @see setX, setY
	 */
	double *outputCTRL(double sampleInput);
	
	void  setCoord_1(double const coord_1) {c1 = coord_1;};
	void  setCoord_2(double const coord_2) {c2 = coord_2;};
	
	void  setX(double xp); /**< Change the X coordinate. */
	void  setY(double yp); /**< Change the Y coordinate. */
	void  recoit_liste(t_symbol *s, int argc, t_atom *argv); /**< (not implemented). */
	
	/**
	 * Set the the number of loudSpeaker.
	 * @param nbLoudSpeaker : the number of loudSpeaker.
	 */
	void  setNbLoudSpeaker(long nbLoudSpeaker);
	
	/**
	 * Modify the disposition only with the distance. So the loudspeakers are on the unity circle with the specified angle.
	 */
	void  set_dist_teta_pos_hp(t_symbol *s, int argc, t_atom *argv );
	
	/**
	 * Modify the disposition with polar coordinate, "distance angle."
	 */
	void  set_teta_pos_hp(t_symbol *s, int argc, t_atom *argv );
	
	/**
	 * Modify the loudspeakers disposition with their cartesian coordinates. "abscissa ordinate" from the
	 * first loudspeaker to the last.
	 */
	void  set_xy_pos_hp(t_symbol *s, int argc, t_atom *argv );
	void  setOffset(double val); /**< Change the offset value (must be > 0). */
	void  setRepereType(int baseCartOrPol); /**< Change the the coordinate type (1 for Cartesian, 0 for Polar). */
	void  set_muteInSig(int mute);
	
	// get
	void  getInfo(); /**< Report some informations to the Max window. */
	int   getBase() const {return base;}; /**< Get the current coordinate type */
	int   getNbOut() const {return N;}; /**< Get the number of outputs */

private:
	
};

#endif