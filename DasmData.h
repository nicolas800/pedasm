/*		fichier DasmData.h : fichier header
 *
 *	descr : classe automate sortie des constantes des sections
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


#ifndef DASM_DATA_H
	#define DASM_DATA_H

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <set>
#include <string>



#include "ExeRep.h"

#include "DasmBase.h"

///////////////////////////////////////////////////////////////
/** class CDasmData : desassemblage des variables
 */
class CDasmData : public CDasm
{
public:
// construction
	/** ctor , param VirtAdrStart */
	CDasmData(); 

// interface classe de base
	bool PassScan() ;
	bool PassEcho() ;
	bool ScanNext() ;
	void PrintInstr() ;
	void Reset() ;
// traitement
	/** complete la table des symbols avec les relocations necessaires*/
	void ScanReloc();

	/** imprime ds le fichier les declarations de variables */
	void PrintVar();

	/** ReqString : tente de reconnaitre une chaine de caractere 
	 *		entree pszCur , pszEnd : borne inf sup du buffer a reconnaitre
	 *		sortie pszBuff : buffer a remplir : 'chaine' 
	 *		retourne : nombre d'octets reconnu ou 0 si aucuns
	 */
	int ReqString( BYTE* pszCur,BYTE* pszEnd , char* pszBuff );

	/** ReqDup : tente de reconnaitre un octet duplique
	 *		entree pszCur , pszEnd : borne inf sup du buffer a reconnaitre
	 *		sortie pszBuff : buffer a remplir : rep dup(val)
	 *		retourne : nombre d'octets reconnu ou 0 si aucuns
	 */
	int ReqDup( BYTE* pszCur,BYTE* pszEnd , char* pszBuff );
};


#endif //DASM_DATA_H