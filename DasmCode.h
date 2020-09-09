/*		fichier DasmCode.h : fichier header
 *
 *	descr : classe automate desassembleur
 *	projet : PEDasm
 *	
 *	rq:
 *	Ce programme est libre de droits. Il peut etre distribue et/ou modifie
 *  selon les termes de la licence 'GNU General Public License version 2'.
 *	
 *	Ce programme est distribue sans aucunes garanties, y compris d'utilite 
 *	ni de risques encouru, quelle que soit son utilisation.
 *
 *	lire le fichier licence.txt fourni ou bien ecrire a :
 *	the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *	pour recevoir une copie de la licence.
 *  	
 *	Copyright (C) 1997 - 1998 Nicolas Witczak <witczak@geocities.com> 
 */

#ifndef DASM_CODE_H
	#define DASM_CODE_H

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <set>
#include <string>

#include "ExeRep.h"
#include "DasmBase.h"

struct SOPContext ;

///////////////////////////////////////////////////////////////
/** class CDasmCode : desassemblage d'une portion de code
 */
class CDasmCode : public CDasm
{
public:
// construction
	/** ctor  */
	CDasmCode(); 

// attributs

	/** mode de desassemblage par defaut 1 -> (32b), 0 ->(16b) */
	BYTE		m_mode32 ;

	// prefixes

	/** prefix de repetition ou 0 si pas de repetition */
	BYTE		m_repPrefix ;

	/** prefix de segment sreg3 ou ff si pas de surcharge */
	BYTE 		m_segPrefix ;

	/** operand size overide si == 1 */
	BYTE		m_oprdSzOverd ; 
	
	/** adress size overide si == 1 */
	BYTE		m_adrSzOverd ; 

	// mask
	bool		m_dFlag , m_sFlag ;
	BYTE		m_tttnFlag ;

	// pointeurs en cours

	/** ptr mem d'instruction  apres prefix */
	BYTE *m_pvPrefix ;

	// operandes

	/** taille de l'operande (0 -> aucun , 1 ->8b , 2 ->16b , 3 -> 32b )
	 *		calc avec le mode processeur par defaut + operand size override 	
	 *	traitement du bit w 
	 */
	BYTE	m_OperandSize ;

	/** empacketage mmx 0 -> 3 
	 */
	BYTE	m_mmxPack ;

	/** taille de l'adresse (0 -> aucun , 1 ->8b , 2 ->16b , 3 -> 32b )
	 *		calc avec le mode processeur par defaut + adresse size override
	 */
	BYTE	m_AdresseSize ;
	
	/** partie ModR/M (optionnelle) */
	BYTE	m_Mod , m_regOpCode , m_RM ;

	/** chaine de caractere rep ds l'ordre les 3 operande si m_bOutputDasm */
	char_buff m_pszOperand[3] ;

	// instruction
	/** chaine de caractere rep l'instruction */
	char_buff	m_pszInstr ;

	/** dernier format reconnu */
	const char*	m_pszFormat ;

// interface general

	/** fct ScanNext : interprete l'instruction suivante 
	 *	positionne les variables de cette classe
	 *		retourne false en cas d'erreur
	 */
	bool ScanNext() ;

	/** fct ScanPrefix : interprete les prefixes
	 *		retourne false en cas d'erreur
	 */
	bool ScanPrefix() ;

	/** Reset */
	void Reset() ;

// interface op code
	/** fct ScanOPCode : interprete le code op en fonction de la 
	 *		table globale pfnOPCodeMask
	 */
	bool ScanOPCode( ) ;


	/**  ProcessMask -> traite le ptr section courant en regard a la chaine de mask
	 *	retourne  false en cas de non reconnaissance
	 *			  true en cas de reconnaissance correcte
	 */
	bool ProcessMask( const char* pszMask );
	
	/** PrintInstr : ecrit l'instruction ds le buffer membre m_pszInstr
	 *		param pszFormat : format d'impression
	 */
	void PrintInstr() ;

// fonctions d'aide
	
	/** traitement par defaut d'une operande memoire 	
	 *		lit le SIB si besoin
	 *	-> resultat ds pszBuff
	 */
	bool ModMProcess( char* pszBuff );

	/** traitement du SIB : renseigne les champs oprd	
	 *		interprete le SIB -> appele par DefModRMProcess
	 *	-> resultat ds pszBuff
	 */
	bool SIBProcess( char* pszBuff );

	/** reordonne le tableau pfnOPCodeMask en fonction de la frequence d'utilisation
	 *	des instructions , accelere le desassemblage
	 *	->utilise pour cela le champs m_iStat de SOPContext
	 */
	void SortInstr() ;
};
 
///////////////////////////////////////////////////////////
/** type SOPContext : association masque code op -> handler
 */

struct SOPContext 
{
// donnees
	/** mask de cod op ou 0 pour fin de tab */
	const char* m_pszMask ;

	/** chaine de caractere de code op
	 *		format de la chaine "operd % ..."
	 *		%0 , %1 , %2 -> operandes
	 *		%p -> qualif ptr , %c -> condition
	 *		%r_ , %rz -> prefix de repetition : rep | repz
	 *		%g -> empacketage mmx
	 */
	const char* m_pszCodeOP ;

	/** m_iStat : nombre d'occurence de cette instruction rencontree dans le binaire 
	 *		sert a optimiser le desassemblage
	 */
	int			m_iStat ;
};


//////////////////////////////////////////////////////////////////
// fonctions globales
//		(aucunes)

/** operateurs servant pour le trie des SOPContext
 */
inline bool operator<(const SOPContext &op1, const SOPContext &op2)
{	return op1.m_iStat > op2.m_iStat; }

inline bool operator==(const SOPContext &op1, const SOPContext &op2)
{	return op1.m_iStat == op2.m_iStat;	}


#endif //DASM_CODE_H