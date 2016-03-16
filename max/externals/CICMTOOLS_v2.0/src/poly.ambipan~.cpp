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

extern "C" {
#include "ext.h"							// standard Max include, always required (except in Jitter)
#include "ext_obex.h"						// required for new style objects
#include "z_dsp.h"							// required for MSP objects
}

#include "../cpp_src/ep_MSP.h"
#include "../cpp_src/ep_ambipan.h"

#define INLET_MAX 64

// object struct
typedef struct _polyAmbipan 
{
	t_pxobject ob;
	ep_ambipan **mypolyAmbipan;
	int Ninlets;
	int Nout;
	int connected[INLET_MAX+1];
	double gainDivisor;
} t_polyAmbipan;

// global class pointer variable
void *polyAmbipan_class;

///////////////////////// function prototypes
//// standard set

void polyAmbipan_free(t_polyAmbipan *x);
void polyAmbipan_assist(t_polyAmbipan *x, void *b, long m, long a, char *s);
void polyAmbipan_float(t_polyAmbipan *x, double f);
void polyAmbipan_int(t_polyAmbipan *x, long n);
void polyAmbipan_dsp64(t_polyAmbipan *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void polyAmbipan_perform64(t_polyAmbipan *x, t_object *dsp64, 
						 double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void  polyAmbipan_dsp(t_polyAmbipan *x, t_signal **sp, short *count);
t_int *polyAmbipan_perform32(t_int *w);

void  polyAmbipan_initialiser_nb_hp( t_polyAmbipan *x, long N);
void  polyAmbipan_changer_offset( t_polyAmbipan *x, double val);
void  polyAmbipan_changer_type_repere(t_polyAmbipan *x, t_symbol *sym);
void  polyAmbipan_muter_entrees_signal(t_polyAmbipan *x, int mute);
void  polyAmbipan_informations( t_polyAmbipan *x );
void  *polyAmbipan_new(t_symbol *s, int argc, Atom *argv );
void  polyAmbipan_recoit_coords(t_polyAmbipan *x, t_symbol *s, long ac, t_atom *av);

void polyAmbipan_dblclick(t_polyAmbipan *x);

////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**********************************************************************/
/*        METHODE RECOIT_float                                        */
/**********************************************************************/

void polyAmbipan_recoit_coords(t_polyAmbipan *x, t_symbol *s, long ac, t_atom *av)
{	
	if (proxy_getinlet((t_object *)x) != 0) return;// reception dans le premier inlet uniquement.
	int instance = -1;
	double cx, cy;
	bool instanceGood, cxGood, cyGood;
	instanceGood = cxGood = cyGood = false;
	
	if (ac>=2) {
		for (int i(0); i < ac; i++) {
			switch (i) {
				case 0:
					switch (av[i].a_type) {
						case A_LONG: instance = (av[i].a_w.w_long)-1; if(instance >=0 && instance < x->Ninlets) instanceGood = true; break;
					} break;
				case 1:
					switch (av[i].a_type) {
						case A_FLOAT: cx = av[i].a_w.w_float; cxGood = true; break;
						case A_LONG: cx = (double)av[i].a_w.w_long; cxGood = true; break;
					} break;
				case 2:
					switch (av[i].a_type) {
						case A_FLOAT: cy = av[i].a_w.w_float; cyGood = true; break;
						case A_LONG: cy = (double)av[i].a_w.w_long; cyGood = true; break;
					} break;
				default: break;
			} if(i>2) break;
		}
		
		if (instanceGood && cxGood) x->mypolyAmbipan[instance]->setX(cx);
		if (instanceGood && cyGood) x->mypolyAmbipan[instance]->setY(cy);
		//post("instance = %ld, cx = %f, cy = %f", instance, cx, cy);
	}
}

/**********************************************************************/
/*              METHODE MODIFIER LE NOMBRE DE HP                      */
/**********************************************************************/

void polyAmbipan_initialiser_nb_hp( t_polyAmbipan *x, long N)
{
	for (int i(0); i < x->Ninlets; i++) x->mypolyAmbipan[i]->setNbLoudSpeaker(N);
}

/**********************************************************************/
/*              METHODE POUR MUTER LES ENTREES SIGNAL                 */
/**********************************************************************/

void  polyAmbipan_muter_entrees_signal(t_polyAmbipan *x, int mute)
{
	;//for (int i(0); i < x->Ninlets; i++) x->mypolyAmbipan[i]->set_muteInSig(mute);
}

/**********************************************************************/
/*                   MODIFICATION DE L'OFFSET                         */
/**********************************************************************/

void polyAmbipan_changer_offset( t_polyAmbipan *x, double val)
{    
	for (int i(0); i < x->Ninlets; i++) x->mypolyAmbipan[i]->setOffset(val);
}

/**********************************************************************/
/*                   MODIFICATION DU TYPE DE REPERE                   */
/**********************************************************************/

void polyAmbipan_changer_type_repere( t_polyAmbipan *x, t_symbol *sym)
{
	int repere;
	//Changement de x->base
	if( sym->s_name[0] == 'c') repere = 1;
	else if( sym->s_name[0] == 'p') repere = 0;
	else { object_error((t_object *)x, "type de repere inconnu."); return; }
	for (int i(0); i < x->Ninlets; i++) x->mypolyAmbipan[i]->setRepereType(repere);
}

/**********************************************************************/
/*         AFFICHAGE DES INFORMATIONS DANS LA FENETRE DE MAX          */
/**********************************************************************/

void polyAmbipan_informations( t_polyAmbipan *x)
{
	x->mypolyAmbipan[0]->getInfo();
}

//--------------- Assistance Methode ---------------------//

void polyAmbipan_assist(t_polyAmbipan *x, void *b, long m, long a, char *s)
{	
	if (m == ASSIST_INLET) {
		if (a == 0) sprintf(s,"(list) Coordinates list, various msgs");
		else sprintf(s,"(Signal) Source input %ld", (a));
	} else sprintf(s,"(Signal) Out %ld", (a+1));
}

/**********************************************************************/
/*                       FONCTION CREATION                            */
/**********************************************************************/

void *polyAmbipan_new(t_symbol *s, int argc, t_atom *argv )
{
	int    i;
	char   car;
	
	int nbInlets, initNbLoudSpeaker, initBase;
	double initOffset, initDtime;
	
	nbInlets = initNbLoudSpeaker = initBase = initOffset = initDtime = 0;
	
	//--------- Allocation de la dataspace ---------------------------//
	
	
	t_polyAmbipan *x = NULL;
	
	if (x = (t_polyAmbipan *)object_alloc((t_class*)polyAmbipan_class)) {
		
		/*********************************************************/
		/*Récupération d'inlets */
		if( argc >= 1 ){
			switch (argv[0].a_type) {
				case A_FLOAT: nbInlets = (int)argv[0].a_w.w_float; break;
				case A_LONG: nbInlets = argv[0].a_w.w_long; break;
				default: nbInlets = 1; break;
			}
		} else nbInlets = 1;
		
		if (nbInlets < 1) x->Ninlets = 1;
		else if (nbInlets > INLET_MAX) x->Ninlets = INLET_MAX;
		else x->Ninlets = nbInlets;
		
		/*********************************************************/
		/*Récupération du nombre de sorties et de haut-parleurs. */
		if( argc >= 2 ){
			switch (argv[1].a_type) {
				case A_FLOAT: initNbLoudSpeaker = (int)argv[1].a_w.w_float; break;
				case A_LONG: initNbLoudSpeaker = argv[1].a_w.w_long; break;
				default: initNbLoudSpeaker = 4; break;
			}
		} else initNbLoudSpeaker = 4;
		
		/*Récupération du type repère ************************/
		if( argc >=3 ){
			if( argv[2].a_type == A_SYM )
				car = (char)(argv[2].a_w.w_sym->s_name[0]);
			if( car == 'c' )
				initBase = 1;
			else if( car == 'p' )
				initBase = 0;
			else {
				initBase = 1;
				object_error((t_object *)x, "erreur dans le type des coordonnees, elles sont cartesiennes par defaut.");
			}
		}
		else initBase = 1; //CartÈsienne par dÈfaut.

		
		/*Récupération de l'offset ***************************/
		if(argc >= 4){
			switch (argv[3].a_type) {
				case A_FLOAT: initOffset = argv[3].a_w.w_float; break;
				case A_LONG: initOffset = argv[3].a_w.w_long; break;
				default: initOffset = 0; break;
			}
		}
		
		/*Récupération du temps d'interpolation **************/
		if(argc >= 5){
			switch (argv[4].a_type) {
				case A_LONG: initDtime = argv[4].a_w.w_long; break;
				case A_FLOAT: initDtime = argv[4].a_w.w_float; break;
				default: break;
			}
		}
		
		x->mypolyAmbipan = new ep_ambipan *[x->Ninlets];
		for (i = 0; i < x->Ninlets; i++) {			
			switch (argc) {
				case 0: x->mypolyAmbipan[i] = new ep_ambipan(); break;
				case 1: x->mypolyAmbipan[i] = new ep_ambipan(); break;
				case 2: x->mypolyAmbipan[i] = new ep_ambipan(initNbLoudSpeaker); break;
				case 3: x->mypolyAmbipan[i] = new ep_ambipan(initNbLoudSpeaker, initBase); break;
				case 4: x->mypolyAmbipan[i] = new ep_ambipan(initNbLoudSpeaker, initBase, initOffset); break;
				case 5: x->mypolyAmbipan[i] = new ep_ambipan(initNbLoudSpeaker, initBase, initOffset, initDtime); break;
				default: break;
			}
		}
		
		/*********************************************************/
		/*Création des nouvelles entrées. ************************/
		dsp_setup((t_pxobject *)x, x->Ninlets+1);
		//inlet_new(x, NULL); // pour rentrer les coordinées de chacune des sources
		
		/*Création des sorties************************************/
		x->Nout = x->mypolyAmbipan[0]->getNbOut();
		for(i=0; i < x->Nout; i++) outlet_new((t_object *)x, "signal");
		
		x->ob.z_misc |= Z_NO_INPLACE;
		
	}
	return (x);
}

/**********************************************************************/
/*                    DEFINITION DE LA CLASSE	                      */
/**********************************************************************/

int main(void) 
{
	
	t_class *c = class_new("poly.ambipan~", 
						   (method)polyAmbipan_new, 
						   (method)polyAmbipan_free, 
						   (short)sizeof(t_polyAmbipan), NULL, A_GIMME, 0);
	
	//-------------Définition des méthodes-------------------//
	class_addmethod(c, (method)polyAmbipan_dsp, "dsp", A_CANT, 0);
	class_addmethod(c, (method)polyAmbipan_dsp64, "dsp64", A_CANT, 0);
	class_addmethod(c, (method)polyAmbipan_recoit_coords, "list", A_GIMME, 0);
	class_addmethod(c, (method)polyAmbipan_initialiser_nb_hp, "set_nb_hp", A_LONG, 0);
	class_addmethod(c, (method)polyAmbipan_changer_offset, "set_offset", A_FLOAT, 0);
	class_addmethod(c, (method)polyAmbipan_muter_entrees_signal, "mute_sig", A_LONG, 0);
	class_addmethod(c, (method)polyAmbipan_changer_type_repere, "change_type", A_SYM, 0);
	class_addmethod(c, (method)polyAmbipan_informations, "get_info", 0);
	class_addmethod(c, (method)polyAmbipan_assist,"assist",A_CANT,0);
	class_addmethod(c, (method)polyAmbipan_dblclick,"dblclick",A_CANT,0);
	
	class_dspinit(c);
	class_register(CLASS_BOX, c);
	polyAmbipan_class = c;	
	
	post("poly.ambipan~ object by Eliott PARIS. v.1 : "__DATE__"");
	
	return 0;
}

void polyAmbipan_dblclick(t_polyAmbipan *x)
{
	object_post((t_object *)x,"poly.ambipan~ object by Eliott PARIS. v.1 : "__DATE__"");
}

/**********************************************************************/
/*                       METHODE DSP                                  */
/**********************************************************************/

// à revoir :
void polyAmbipan_dsp(t_polyAmbipan *x, t_signal **sp, short *count)
{
	long i;
	t_int **sigvec;
	int pointer_count;
	int nbConnected = 0;
	
	ep_MSP::setup();
	//for(int i(1); i <= x->Ninlets; i++) if(x->connected[i-1] = count[i]) nbConnected++; // 1er inlet ignoré
	//x->gainDivisor = pow((double)nbConnected, -0.5) * 1.414;
	
	for(int i(0); i < x->Ninlets+1; i++) if(x->connected[i] = count[i]) nbConnected++;
	if (x->connected[0]) nbConnected--;
	if (nbConnected > 0) x->gainDivisor = 1.f/(double)nbConnected;//pow((double)nbConnected, -0.5) * 1.414;
	else x->gainDivisor = 0.f;
	
	pointer_count = x->Ninlets + x->Nout + 2; // object pointer, all inlets + 1 inlet coordonnées, all outlets and vec-samps
	
	sigvec  = (t_int **) calloc(pointer_count, sizeof(t_int *));
	
	for(i = 0; i < pointer_count; i++) sigvec[i] = (t_int *) calloc(sizeof(t_int),1);
	
	sigvec[0] = (t_int *)x; // first pointer is to the object
	
	sigvec[pointer_count - 1] = (t_int *)sp[0]->s_n; // last pointer is to vector size (N)
	
	for(i = 1; i < pointer_count-1; i++) sigvec[i] = (t_int *)sp[i]->s_vec;  // now attach all inlets and all outlets
	
	dsp_addv(polyAmbipan_perform32, pointer_count, (void **) sigvec); 
	free(sigvec);
}

//---------------------------------------------------------------------//

void  polyAmbipan_dsp64(t_polyAmbipan *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	int nbConnected = 0;
	ep_MSP::setup(samplerate);
	//for(int i(1); i <= x->Ninlets; i++) if(x->connected[i-1] = count[i]) nbConnected++;
	for(int i(0); i < x->Ninlets+1; i++) if(x->connected[i] = count[i]) nbConnected++;
	if (x->connected[0]) nbConnected--;
	if (nbConnected > 0) x->gainDivisor = 1.f/(double)nbConnected;//pow((double)nbConnected, -0.5) * 1.414;
	else x->gainDivisor = 0.f;
	object_method(dsp64, gensym("dsp_add64"), x, polyAmbipan_perform64, 0, NULL);
}

//---------------------------------------------------------------------//


t_int *polyAmbipan_perform32(t_int *w)
{
	t_polyAmbipan *x = (t_polyAmbipan *)(w[1]);
	int            n = (int)(w[x->Ninlets + x->Nout + 2]);
	
	int i, acum;
	
	double *tempOut = NULL; //sortie de chaque ambipan
	float outputs[x->Nout]; //somme des sorties
	for (acum=0; acum < x->Nout; acum++) outputs[acum] = 0; //init à 0
	
	float *entree[x->Ninlets];
	for( i = 0; i < x->Ninlets; i++) entree[i] = (float *)(w[i+2]); // 1er inlet ignoré
	
	float *sorties[x->Nout];
	for(i = 0; i < x->Nout; i++) sorties[i] = (float *)(w[x->Ninlets+i+2]); 
	
	if (x->ob.z_disabled) goto noProcess;
	
	while (n--){
		for (i=0; i < x->Ninlets; i++) {
			if(x->connected[i+1]) {
				tempOut = x->mypolyAmbipan[i]->outputCTRL(*entree[i]++);
				for (acum=0; acum < x->Nout; acum++) outputs[acum] += (float)(tempOut[acum]);
			}
		}
		
		for (i=0; i < x->Nout; i++) {
			*sorties[i]++ = (outputs[i] * x->gainDivisor); // somme de ts les ambipan + reduction du volume.
			outputs[i] = 0;
		}
	}
	
	
noProcess :	
	return (w+(x->Ninlets + x->Nout + 3));
}


void polyAmbipan_perform64(t_polyAmbipan *x, t_object *dsp64, 
					   double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	int n, i, acum;
	
	double *tempOut = NULL;  //sortie de chaque ambipan~
	double outputs[x->Nout]; //somme des sorties
	for (acum=0; acum < x->Nout; acum++) outputs[acum] = 0.f; //init à 0

	if (x->ob.z_disabled) goto out;
	
	for(n = 0; n < sampleframes; n++){
		
		for (i=1; i <= x->Ninlets; i++) {
			if(x->connected[i]) {
				tempOut = x->mypolyAmbipan[i-1]->outputCTRL(ins[i][n]);
				for (acum=0; acum < x->Nout; acum++) outputs[acum] += (tempOut[acum]);
			}
		}
		
		for (i=0; i < x->Nout; i++) {
			outs[i][n] = (outputs[i] * x->gainDivisor); // somme de ts les ambipan + reduction du volume.
			outputs[i] = 0.f;
		}
	}

out:
	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void polyAmbipan_free(t_polyAmbipan *x) 
{
	dsp_free((t_pxobject *)x);
	for (int i = 0; i < x->Ninlets; i++) {
		delete x->mypolyAmbipan[i];
	}
	delete x->mypolyAmbipan;
}
