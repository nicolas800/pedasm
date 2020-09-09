/*		fichier Globals.cpp : fichier implementation
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

#include "Config.h"

#include "Globals.h"

/////////////////////////////////////////////////////////////////////////
// separateurs utilises ds les fichiers de sortie
const char* cteHeaderSep = 
	";*******************************************************************\n";

const char* cteHeaderStLine = 
	";*" ;

const char* ctePartSep = 
	";------------------------------------------------------------\n" ;

const char* ctePartStLine = 
	";+" ;

//////////////////////////////////////////////////////////////////
// fonctions globales

/** retourne le nom sans extension
 *	param pszStr : nom en entree
 *	param pszBuff : buffer tmp
 */
const char* GetBaseName( const char* pszStr )
{
	static char_buff pszRet ;
	char* pszCur = pszRet ;
	while( ( *pszStr != '.' ) && ( *pszStr != '\0' )  )
		*pszCur++ = *pszStr++;
	*pszCur = '\0' ;
	return pszRet ;
}

void FillChar( char* pszBuff , char cFill  , int iMaxSize  ) 
{
	int iSize = strlen( pszBuff ) ;
	if( iSize < iMaxSize )
	{
		memset( pszBuff + iSize , cFill , iMaxSize - iSize ) ;
		pszBuff[iMaxSize] = '\0' ;
	}
}

char* StrDup( const char* pszSrc )
{
	char* pszRet = new char[ strlen(pszSrc) + 1 ] ;
	strcpy( pszRet , pszSrc ) ;
	return pszRet ;
}

/** filtrage des noms de dll */
#define DLL_FUNNAME_FILTER "@?"

const char* DllFilterNane( const char* pszSrc ) 
{
	char* pszOut = new char[ strlen(pszSrc) + 2 ] ;
	char* pszDest = pszOut ;
	char c ;
	for( const char* pszCur = pszSrc ; *pszCur != '\0'; ++ pszCur )
	{
		if( strchr( DLL_FUNNAME_FILTER , *pszCur ) == 0 )
		{
			if( ( pszDest == pszOut ) && isdigit( *pszCur ) )	// 1er charactere : ne doit pas etre un entier
				*pszDest++ = 'F' ;
			*pszDest++ = *pszCur ;	
		}
	}
	*pszDest = '\0' ;
	if( strcmp( pszOut , pszSrc ) == 0 )
	{
		delete pszOut ;
		return pszSrc ;
	}
	else
		return pszOut ;
}

unsigned int RoundUp( unsigned int iOldVal , unsigned  int iGranularity ) 
{
	unsigned int nbChunk = iOldVal / iGranularity ;
	if( iOldVal % iGranularity != 0 )
		nbChunk ++ ;
	return nbChunk * iGranularity ;
}

unsigned int GetFileSize( const char* pszFileName )
{
	HANDLE hFile = CreateFile( pszFileName 
		, GENERIC_READ ,FILE_SHARE_READ | FILE_SHARE_WRITE 
		, 0 , OPEN_EXISTING , 0 , 0 );
	if( hFile != 0 )
		return GetFileSize( hFile , 0 ) ;
	else return 0 ;
}
