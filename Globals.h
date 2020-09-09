/*		fichier Globals.h : fichier header
 *
 *	descr : constantes et fct de portee global au projet
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

#ifndef GLOBALS_H
	#define GLOBALS_H

#include <cstdlib>
#include <cctype> 
#include <cstdio>
#include <functional>
#include <string>


/////////////////////////////////////////////////////////////////////////
/** type buffer assez grand */
typedef char char_buff[256] ;

/////////////////////////////////////////////////////////////////////////
// separateurs utilises ds les fichiers de sortie

extern const char* cteHeaderSep;
extern const char* cteHeaderStLine ;
extern const char* ctePartSep ;
extern const char* ctePartStLine ;

////////////////////////////////////////////////////////////////////////
// macro d'aide
/** definie une cte 8 bit binaire */
#define BIN_CTE(a7,a6,a5,a4,a3,a2,a1,a0)	\
	( (a7 * 128 ) + (a6 * 64) + (a5 * 32) + (a4 * 16) + ( a3 * 8 )+ ( a2 * 4 )+ ( a1 * 2 ) + a0 )


//////////////////////////////////////////////////////////////////
// fonctions globales

/** RoundUp : retourne iOldVal arrondi tq iOldVal >= n * iGranularity 
 */
unsigned int RoundUp( unsigned int iOldVal , unsigned  int iGranularity ) ;

/** GetFileSize : retourne la taille d'un fichier en nbr d'octets
 */
unsigned int GetFileSize( const char* pszFileName ) ;

/** StrDup : duplique une chaine : retourne  un buffer a effacer avec delete
 */
char* StrDup( const char* pszSrc );

/** GetBaseName : retourne un nom sans extension */
const char* GetBaseName( const char* pszStr );

/** FillChar : remplie le buffer pszBuff avec le caractere cFill a concurrence de iMaxSize */
void FillChar( char* pszBuff , char cFill = ' ' , int iMaxSize = 64 ) ;

/** si le nom pszSrc n'est pas conforme au MASM 
 *	retourne un nom conforme alloue avec new
 *	retourne pszSrc sinon 
 */
const char* DllFilterNane( const char* pszSrc ) ;

/////////////////////////////////////////////////////////////////////////
// objets fonctionnels

	//////////////////////////////////////////////////////////////
	// objet fonctionnel de comparaison de chaine de caracteres (par valeur)

struct psz_less : public binary_function< const char* , const char* , bool>
{
	bool operator()( const char* x , const char* y) const
	{	return ( strcmp( x, y ) < 0 ) ; } ;
};

struct psz_less_nocase : public binary_function< const char* , const char* , bool>
{
	bool operator()( const char* x , const char* y) const
	{	return ( stricmp( x, y ) < 0 ) ; } ;
};


#endif //GLOBALS_H