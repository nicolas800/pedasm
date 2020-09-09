/*		fichier Symbol.h : fichier header
 *
 *	descr : herarchie de classe qui represente les differents types de symbols
 *
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

#ifndef SYMBOLS
	#define SYMBOLS

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <set>
#include <string>

#include "Globals.h"


//////////////////////////////////////////////////////////////////
// structure d'aide
class CExeRep ;
class CSection ;


#pragma pack( 4 )

///////////////////////////////////////////////////////////////
// drapeaux et categories de la class CSymbol 
	// categorie
const unsigned int fSymKnown	= 1	 ;	
const unsigned int fSymData		= 2	 ;	// code sinon 
const unsigned int fSymCode		= 4	 ;	// donnee sinon 
const unsigned int fSymPointer	= 8	 ;	// tjs a 0 ci c'est du code, ou inconnu
const unsigned int fSymFunc		= 16 ;	// jmp sinon , tjs a 0 ci c'est des donnees 

	// qualificatifs
const unsigned int fSymExtern	= 32 ;	// tjs a 0 ci c'est du code
const unsigned int fSymHidden	= 64 ; 	// ne doit pas apparaitre ds le code
const unsigned int fSymPublic	= 128 ; // le symbol est a exporter	
const unsigned int fComment		= 256 ; // commentaire de source
const unsigned int fCallBack	= 512 ; // fonction appelee en call back
const unsigned int fSymUser		= 1024; // symbol rajoute manuellement par l'utilisateur

const unsigned int cSymUnknown	= 0	;	
const unsigned int cSymData		= fSymKnown | fSymData	 ;	
const unsigned int cSymUser		= fSymKnown | fSymUser	 ;	
const unsigned int cSymDataPtr	= fSymKnown | fSymData | fSymPointer ;	
const unsigned int cSymFunc		= fSymKnown | fSymCode | fSymFunc ;	
const unsigned int cSymCallBack	= cSymFunc  | fCallBack ;	
const unsigned int cSymJmp		= fSymKnown | fSymCode ;	
const unsigned int cSymDllImp	= fSymKnown | fSymData | fSymPointer | fSymExtern  ;
const unsigned int cSymUnused	= fSymKnown | fSymData | fSymHidden	;	


///////////////////////////////////////////////////////////////
/** class CSymbol : represente un symbol : variable , fonction , jmp ...
 *		classe de base
 */
class CSymbol 
{
public:
// construction
	/**  pszName pszComment : pointeur externe (doivent valide durant tt l'execution du prog) */
	CSymbol( unsigned int VirtAdress = 0 , unsigned int attrib = 0 , const char* pszName = 0 
		, const char* pszComment= 0); 
	virtual ~CSymbol() ;

// atributs
	/** var m_VirtAdress: adresse abs de debut du symbole */
	unsigned int	m_VirtAdress ;

	/** attributs */
	unsigned int	m_Attrib ;
	
	/** nom opt ( ou 0 ) */
	const char*		m_pszName ;

	/** commentaire optionnel ou 0 */
	const char*		m_pszComment ;

// interface virtuelle
	/** accede a la section qui contient ce symbol*/
	CSection* GetSection() const;

	/** accede au nom : retourne une chaine temporaire */
	const char* GetName() const ;

	/** retourne un commentaire utilisateur ou 0 si aucuns */
	const char* GetComment() const ;

	/** fct GetDeclaration : retourne une chaine contenant la
	 *		declaration imprimable du symbol
	 */
	const char* GetDeclaration() const ;

	/** taille du symbol ou -1 si inconnu */
	unsigned int GetSize() const ;

	/** retourne le symbol pointe par ce symbol ou 0 si aucun */
	CSymbol* GetSource() const ;

	/** dump le contenu de ce symbole sur une ligne texte
	 *		retourne le stream passe en entree 
	 */
	void Dump( FILE* pFile ) const ;
};

#pragma pack()


//////////////////////////////////////////////////////////////////
// fonctions globales

	//////////////////////////////////////////////////////////////
	// obj fonctionnel de comparaison de symbol (par adresse)
struct symb_less : public binary_function< CSymbol* , CSymbol* , bool>
{
    bool operator()(const CSymbol* x, const CSymbol* y) const
	{	return x->m_VirtAdress < y->m_VirtAdress ; } ;
};


#endif //SYMBOLS
