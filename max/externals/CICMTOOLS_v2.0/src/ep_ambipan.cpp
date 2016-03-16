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


#include "ep_MSP.h"
#include "ep_ambipan.h"

int const ep_ambipan::Nmax = 64;        //Nombre maximum de haut-parleurs,
int const ep_ambipan::Ndefaut = 4;      //Nombre de haut-parleurs par défaut,
int const ep_ambipan::Xdefaut = 0;      //Valeur par defaut de l'absisse,
int const ep_ambipan::Ydefaut = 1;      //Valeur par défaut de l'ordonnée,
int const ep_ambipan::Rdefaut = 1;      //Distance par défaut de la source,
int const ep_ambipan::T_COS = 4096;     //Taille du tableau cosinus (puissance de 2),
int const ep_ambipan::MASQUE = 4095;    //Masque pour le modulo = T_COS - 1,
int const ep_ambipan::DTIME = 10;       //Temps d'interpolation par dÈfaut, //en Èchantillons (10ms),
double const ep_ambipan::Phidefaut = 1.5708F; //Valeur par défaut de l'angle (PI/2),
double const ep_ambipan::Pi = 3.1415926535897932384F;   // Pi
double const ep_ambipan::I360 = 0.0027777777777777778F; // 1/360
double const ep_ambipan::OFFSET = 0.3;     //Offset par defaut de l'ambisonie,
double const ep_ambipan::EPSILON = 0.000001; //CritËre pour tester si les flottants sont nuls

//------ CONSTRUCTORS ------//

ep_ambipan::ep_ambipan(int nbLoudSpeaker, int baseCartOrPol, double initOffset, double initDtime){
	
	int i, hp;
	
	this->Nout = this->N = ep_clip<int>(nbLoudSpeaker, 2, Nmax);
	
	if (baseCartOrPol >= 1) this->base = 1;
	else this->base = 0;
	
	this->offset = fabs(initOffset);
	if (this->offset <= EPSILON) this->offset = OFFSET;
	
	this->dtime = (int) (initDtime * ep_MSP::get_sr()/1000.);
	if (this->dtime <= 0) this->dtime = 1;
	
	this->P = new float[Nmax];
	this->teta = new float[Nmax];
	this->dist = new float[Nmax];
	this->dP = new float[Nmax];
	this->Pstop = new float[Nmax];
	
	/*********************************************************/
	/*Initialisation des données de la classe. ***************/
	for( hp = this->N-1; hp >= 0; hp--) //Initialisation des P
		this->P[hp] = 0;
	
	this->mute= 0;             //entrées signal non mutées
	this->y   = Ydefaut;       //initialisation de y
	this->phi = Phidefaut;     //initialisation de phi
	
	//Initialisation des angles des haut-parleurs et des rayons,
	for( hp=0; hp<this->N; hp++)
	 {
		this->teta[hp] = (float)( Pi*( .5 + (1 - 2*hp )/(float)this->N) );
		this->dist[hp] = 1;
	 }
	
	/*********************************************************/
	/* Création des tableaux cosinus, cos_teta et sin_teta.  */
	/* pour l'audio.                                         */
	//this->cos_teta = (float*)getbytes( (short)((T_COS+2*this->N)*sizeof(float)) );
	this->cos_teta = new float[(T_COS+2*this->N)];
	this->sin_teta = this->cos_teta+this->N;
	
	//Précalculs des cos et sin des haut-parleurs.
	for( hp=0; hp<this->N; hp++){
		this->cos_teta[hp] = (float)cos( this->teta[hp] );
		this->sin_teta[hp] = (float)sin( this->teta[hp] );
	}
	
	//Remplissage du tableau cosinus,
	this->cosin = this->sin_teta+this->N;
	/*Pour avoir besoin de cos( phi ), on cherche:
	 cos(phi) = 
	 cosin[ (int)( phi*T_COS/360 ))&(((int)T_COS-1) ] */
	for( i=0; i<T_COS; i++) this->cosin[i] = (float)cos( i*2*Pi/T_COS );
	
	/*********************************************************/
	//initialisation de x, X, Y, W et Pn par la méthode recoit x.
	if(this->base) setX(Xdefaut);
	else           setY(Rdefaut);
	
	this->m_output = new double[this->N];
}

ep_ambipan::~ep_ambipan(){
	delete [] this->P;
	delete [] this->teta;
	delete [] this->dist;
	delete [] this->dP;
	delete [] this->Pstop;
	
	delete [] this->cos_teta;
	
	delete [] this->m_output;
}

//------ METHODS ------//


/**********************************************************************/
/*                         OUTPUT                                     */
/**********************************************************************/

double *ep_ambipan::output(double sampleInput, double coord_x, double coord_y){
	
	/*Déclarations des variables locales*/
	int   hp;             //indices relatif au haut-parleur,
	
	t_double K = (t_double)(sqrt(1/(t_double)this->N)/1.66);   //Facteur correctif.
	
	//Paramètres ambisoniques:
	t_double xtemp, xl, yl, ds, dist, X, Y, W, P;
	int   phii;
	
	/******************************************************************/
	/*  Traitements  **************************************************/
	
	this->c1 = coord_x;
	this->c2 = coord_y;
	
	xl = coord_x;
	yl = coord_y;
	
	//Conversion polaires -> cartésiennes,
	if(!this->base)
	 {
		//ici xl = rayon et yl = angle, 
		phii = (int)( yl*T_COS*I360 )&(int)MASQUE; 
		xtemp    = xl*this->cosin[ phii ];
		phii = (int)(.25*T_COS - phii)&(int)MASQUE;
		yl       = xl*this->cosin[ phii ];
		xl = xtemp;
		//maintenant xl = abscisse et yl = ordonnée.  
	 }
	
	//Calcul des distances,
	ds   = xl*xl + yl*yl;
	dist = (t_double)sqrt(ds);
	
	//Calcul des paramètres ambisoniques,
	X = (t_double)( 2*xl / (ds + this->offset) ); 
	Y = (t_double)( 2*yl / (ds + this->offset) ); 
	W = (t_double)( .707 / (dist + this->offset) );
	
	for( hp=this->N-1; hp >= 0 ; hp--)
	 {
		P = K  * ( W + X*this->cos_teta[hp]  
				  + Y*this->sin_teta[hp]  )
		* this->dist[hp];
		
		//Si Pn<0 on les force à 0
		if(P < 0) P = 0;
		
		/***********************/
		this->m_output[hp] = sampleInput * P;/**/
		/***********************/
	 }
	
	/*Initialisation à zéro des sorties inutilisées*/
	if( this->Nout > this->N){
		for( hp = this->Nout-1 ; hp >= this->N ; hp--){
			this->m_output[hp] = 0;
		}
	}

	return this->m_output;
}

/**********************************************************************/
/*                         OUTPUT_CTRL                                */
/**********************************************************************/

double *ep_ambipan::outputCTRL(double sampleInput){
	
	/*Déclarations des variables locales*/
	int   hp;             //indices relatif au haut-parleur,
	
	/******************************************************************/
	/*on utilise le tableau P de contrôle.***/
	
	//Modulation des sorties avec les coefficients ambisoniques Pn.
	for( hp = this->N-1; hp >= 0; hp--)
	 {
		//Incrémentation des P pour l'interpolation,
		if( this->P[hp] == this->Pstop[hp] ) /*rien*/  ;
		else if (fabs(this->Pstop[hp] - this->P[hp]) > fabs(this->dP[hp]) )
			this->P[hp] += this->dP[hp];
		else
			this->P[hp] = this->Pstop[hp];
		
		/******************************/
		this->m_output[hp] = sampleInput * (double)this->P[hp];/**/
		/******************************/
	 }
	
	/*Initialisation à zéro des sorties inutilisées*/
	
	if(this->Nout > this->N){
		for( hp = this->Nout-1 ; hp >= this->N ; hp--){
			this->m_output[hp] = 0;
		}
	}
	
	return this->m_output;
}


//-------------- setters --------------//


/**********************************************************************/
/*        METHODE setX                                                */
/**********************************************************************/
//Cette méthode reçoit x et change tous les paramètres ambisoniques: 
//  X, Y, W et les Pn 
void ep_ambipan::setX(double xp)
{
	int hp;
	float ds, dist;
	float X, Y, W;
	float yp = this->y;
	float K = (float)(sqrt(1/(float)this->N)/1.66 ); 
	
	
	//Conversion polaire -> cartésienne,
	if( !this->base )
	 {
		//ici xp = rayon,
		this->r = xp;
		xp   = (float)( this->r * cos( this->phi ) );
		yp   = (float)( this->r * sin( this->phi ) );
		//maintenant xp = abscisse,
	 }
	this->x = xp;
	this->y = yp;
	
	//Calcul des distances,
	ds = xp*xp + yp*yp;
	dist = (float)sqrt(ds);
	
	//Calcul des paramètres ambisoniques,
	X = (float)( 2*xp/(ds + this->offset) ); 
	Y = (float)( 2*yp/(ds + this->offset) ); 
	W = (float)( .707/(dist + this->offset) );
	
	//Calcul des coefficients ambisoniques cibles et des pas,
	for( hp = this->N-1; hp >= 0 ; hp--)
	 {
		this->Pstop[hp] = (float)( ( W + X*cos(this->teta[hp]) 
								 + Y*sin(this->teta[hp])
								 ) * K  * this->dist[hp]);
		//Si Pstop_n < 0 on les force à 0.
		if(this->Pstop[hp] < 0)
			this->Pstop[hp] = 0;
		
		this->dP[hp] = (this->Pstop[hp] - this->P[hp])/(float)this->dtime;
	 }
	
	return;
}



/**********************************************************************/
/*        METHODE setY                                                */
/**********************************************************************/
//Cette méthode reçoit y et change tous les paramètres ambisoniques: 
//  X, Y, W et les Pn 
void ep_ambipan::setY(double yp)
{
	int hp;
	float ds, dist;
	float X, Y, W;
	float xp = this->x;
	float K = (float)(sqrt(1/(float)this->N)/1.66 ); 
	
	
	//Conversion polaires -> cartésienne,
	if( !this->base )
	 {
		//ici yp = angle,
		this->phi = (float)( yp*Pi/180 );
		xp   = (float)( this->r * cos( this->phi ) );
		yp   = (float)( this->r * sin( this->phi ) );
		//maintenant xp = ordonnée,
	 }
	this->y = yp;
	this->x = xp;
	
	//Calcul des distances,
	ds = xp*xp + yp*yp;
	dist = (float)sqrt(ds);
	
	//Calcul des paramètres ambisoniques,
	X = (float)( 2*xp/(ds + this->offset) ); 
	Y = (float)( 2*yp/(ds + this->offset) ); 
	W = (float)( .707/(dist + this->offset) );
	
	//Calcul des coefficients ambisoniques cibles et des pas,
	for( hp=this->N-1; hp >= 0 ; hp--)
	 {
		this->Pstop[hp] = (float)( ( W + X*cos(this->teta[hp]) 
								 + Y*sin(this->teta[hp])
								 ) * K  * this->dist[hp]);
		//Si Pstop_n<0 on les force ‡ 0
		if(this->Pstop[hp] < 0)
			this->Pstop[hp] = 0;
		
		this->dP[hp] = (this->Pstop[hp] - this->P[hp])/(float)this->dtime;    
	 }
	
	return;
}

/**********************************************************************/
/*              METHODE MODIFIER LE NOMBRE DE HP                      */
/**********************************************************************/

void ep_ambipan::setNbLoudSpeaker(long nbLoudSpeaker)
{
	int hp;
	
	if( this->Nout >= nbLoudSpeaker && 2 <= nbLoudSpeaker )
	 {
		this->N = (int)nbLoudSpeaker;
		
		//Modifications des angles des haut-parleurs et des rayons,
		for( hp=0; hp < this->N; hp++)
		 {
			this->teta[hp] = (float)( Pi*( .5 + (1 - 2*hp )/(float)this->N) );
			this->dist[hp] = 1;
		 }
		
		
		/*****************************************************/
		/*Modification des tableaux cos_teta et sin_teta     */
		/*pour l'audio.                                      */
		//Précalculs des cos et sin des haut-parleurs.
		for( hp=0; hp < this->N; hp++)
		 {
			this->cos_teta[hp] = (float)cos( this->teta[hp] );
			this->sin_teta[hp] = (float)sin( this->teta[hp] );
		 }
	 }  
	else
		object_error(ep_MSP::getObjectPointer(), "Le nombre de haut-parleurs doit etre "
					 "inferieur a %d, \n"
					 "  et superieur à 2, ou egal.", this->Nout);
    
	/*Affectation des gains de l'ambisonie*/
    if(this->base) setX(this->x);
    else           setY(this->r);
	
}


/**********************************************************************/
/*                   MODIFICATION DE L'OFFSET                         */
/**********************************************************************/

void ep_ambipan::setOffset(double val)
{    
	if( val <= EPSILON ){
		object_error(ep_MSP::getObjectPointer(), "Pas d'offset negatif ou nul s'il vous plait.");
		return;
	}
	
	this->offset = val; //Changement de l'offset
	
	/*Affectation des gains de l'ambisonie*/
    if(this->base) setX(this->x);
    else           setY(this->r);
}


/**********************************************************************/
/*                   MODIFICATION DU TYPE DE REPERE                   */
/**********************************************************************/

void ep_ambipan::setRepereType(int baseCartOrPol)
{
	//Changement de this->base
	if (baseCartOrPol == 0) this->base = 0;
	else if (baseCartOrPol == 1) this->base = 1;
	else return;
	
    
	//Si coordonnées polaires il faut initialiser this->r et this->phi:
	if( this->base == 0 )
	 {
		//Conversion de x et y en polaires:  
		this->r = (float)sqrt( pow( this->x, 2) + pow( this->y, 2) );
		if( this->r!=0 )
		 {
			this->phi = (float)acos( (float)this->x/this->r );
			if(this->y < 0)
				this->phi = - this->phi;
		 }
		else
			this->phi = 0;
	 }  
}

/**********************************************************************/
/*                             MUTE                                   */
/**********************************************************************/

void  ep_ambipan::set_muteInSig(int mute)
{
	if( mute == 0)   this->mute = 0;
	else             this->mute = 1;
}


/**********************************************************************/
/*         AFFICHAGE DES INFORMATIONS DANS LA FENETRE DE MAX          */
/**********************************************************************/

void ep_ambipan::getInfo()
{
	int hp;
	object_post(ep_MSP::getObjectPointer(), "Info Ambipan~ : ");
	
	if(this->base) post("   coordonnees cartesiennes,");
	else post("   coordonnees polaires,");
	
	post("   offset = %f,", this->offset);
	post("   temps d'interpolation (pour le controle)  = %d ms,", (int)(this->dtime*1000/sys_getsr()));
	post("   nombre de haut-parleurs = %d," , this->N);
	post("   position des haut-parleurs:");
	for( hp=0; hp<this->N-1; hp++)
		post("      hp n°%d: %f.x + %f.y,", hp+1, this->dist[hp]*cos(this->teta[hp]), 
			 this->dist[hp]*sin(this->teta[hp]));
    
	post("      hp n°%d: %f.x + %f.y.", hp+1, this->dist[hp]*cos(this->teta[hp]), 
		 this->dist[hp]*sin(this->teta[hp]));
	post("      ***");
}