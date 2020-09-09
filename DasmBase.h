/*		fichier DasmBase.h : fichier header
 *
 *	descr : classe de base analyse et desassemblage d'une section
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

#ifndef DASM_BASE_H
	#define DASM_BASE_H


#include <cstdlib>
#include <cstdio>
#include <vector>
#include <set>
#include <string>



#include "ExeRep.h"

//////////////////////////////////////////////////////////////////
/** class CDasm : service communs pour desassemblage 
 *	d'une portion de code ou de donnees . 
 *	rq : cette classe est en fait un union d'instances CDasmCode 
 *	et CDasmData . Ces deux dernieres classes ne doivent pas contenir
 *	de donnees membres necessitant un constructeur ou destructeur
 */
class CDasm
{
public:
// construction
	CDasm();

	/** fct SetWindow : met en place une fenetre de desassemblage 
	 *	sur une section
	 */
	CDasm& SetWindow( CSection* pSection );
	
	/** SetFile : attache un fichier en sortie */
	CDasm& SetFile( FILE* pFile ) ;

// attributs
	// parametres
	/** fichier de sortie du source*/
	FILE*		m_pFileOut ;
	
	/** indicateur 1ere ou 2eme passe 
	 *	: ctePassScan -> scan uniquement , ctePassEcho -> produit la sortie
	 */
	int 		m_iPass ;

	/** indicateur de mode code donnes 
	 *	vaut iCodeMode ou iDataMode 
	 */
	int 		m_iMode ;

// symbols
	
	/** symbols encadrant le bloc courant : eventuellement nuls*/
	CSymbol* m_pSymbCur , *m_pSymbNext ;

	/** section ds laquelle on desassemble */
	CSection*	m_pSection ;
	
// pointeurs sur le bloc en cours
// 1 bloc == espace entre 2 symbols
	/** ptr mem d'instruction courante 
	 * , ptr mem dans l'instruction , pointeur d'arret*/
	BYTE*		m_pvBegin , *m_pvNext , *m_pvCur ;
	
	/** valeur du registre EIP en cours et de la fois d'avant */
	unsigned int m_iIP , m_iLastIP;

	/** valeur du registre EIP de debut de bloc */
	unsigned int m_iIBeginIP ;

	/** valeur du registre EIP debut du block suivant*/
	unsigned int m_iNextIP ;
		
// interface avec le programme

	/** point d'entree desassemblage
	 *	param iPass  : iPassScan -> scan uniquement , iPassEcho -> produit la sortie 
	 */
	void Run( int iPass ) ;

// fonctions d'aide	
	/** retourne l'adresse virtuelle de fin*/
	unsigned int va_end();

// interface avec classes derivees 

	/** appele a chaque permutation code / donnees
	 */
	void Reset();

	/** fct ScanNext : interprete l'instruction suivante 
	 *	positionne les variables de cette classe
	 *	peut modifier m_iIP ou m_pvCur
	 *		retourne : false en cas d'erreur 
	 */
	bool ScanNext() ;
	
	/** PrintOutput : ecrit l'instruction ds le stream membre
	 */
	void PrintInstr() ;
};
 
/** constantes mode code , donnee */
const int cteCodeMode = 0 ;
const int cteDataMode = 1 ;

/** constantes mode analyse , echo */
const int ctePassScan  = 1 ;
const int ctePassEcho  = 2 ;

//////////////////////////////////////////////////////////////////
// fonctions globales
/** fct CreateCDasm : cree un objet CDasm 
 *		retourne un buffer assez grand pour contenir union( CDasmCode , CDasmData )
 */
CDasm* CreateCDasm();


#endif //DASM_BASE_H
	
