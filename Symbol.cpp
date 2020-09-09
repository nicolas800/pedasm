/*		fichier Symbol.cpp : fichier implementation
 *
 *	descr : herarchie de classe qui represente un differents
 *		type de symbols
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

#include <cassert>

#include "Symbol.h"
#include "ExeRep.h"


//////////////////////////////////////////////////////////////////
// implementation class CSymbol


CSymbol::CSymbol( unsigned int VirtAdress , unsigned int attrib , const char* pszName 
	, const char* pszComment ) 
	: m_VirtAdress( VirtAdress ) , m_Attrib(attrib) , m_pszName(pszName) , m_pszComment(pszComment)
{
}

CSymbol::~CSymbol()
{}

CSection* CSymbol::GetSection() const
{
	CSection* pRefSection = GetExe()->GetSectFromVA( m_VirtAdress ) ;
	assert( pRefSection != 0 ) ;
	return pRefSection ;
}

unsigned int CSymbol::GetSize() const
{
	if( m_Attrib & fSymPointer != 0 )
		return 4 ;
	else
		return -1 ;
}

void CSymbol::Dump( FILE* pFile ) const
{
	fprintf( pFile ,"symbol %s : adr 0x%08x , size %i , section %s \n" ,
		GetName() , m_VirtAdress , GetSize() , GetSection()->m_pHeader->Name ) ;
}

const char* CSymbol::GetName() const 
{
	static char_buff pszBuff ;
	if( m_pszName != 0)
		return m_pszName ;
	else
	{
		const char* pszTemp ;
		if( ( m_Attrib & fSymKnown ) == 0 )
			pszTemp =  "Unknown" ;
		else if( ( m_Attrib & fSymCode ) != 0 )
		{
			if( ( m_Attrib & fSymFunc ) != 0 )
			{
				if(  ( m_Attrib & fCallBack ) != 0  )
					pszTemp = "CallBack" ;
				else
					pszTemp = "Fun" ;
			}
			else // jump
				pszTemp = "Label" ;
		}
		else // data
		{
			if( ( m_Attrib & fSymPointer ) != 0 )
				pszTemp = "DataPtr" ;
			else // jump
				pszTemp = "Data" ;
		}
		sprintf( pszBuff , "%s%x" , pszTemp , m_VirtAdress ) ;
		return pszBuff ;
	}
}

const char* CSymbol::GetComment() const 
{
	return m_pszComment ;
}

const char* CSymbol::GetDeclaration() const
{
	static char_buff pszBuff ;
	char* pszCur = pszBuff ;
	if( ( m_Attrib & fSymKnown != 0 ) 
		&& ( ( m_Attrib & fSymHidden ) == 0 ) )
	{
		if( ( m_Attrib & fSymFunc ) != 0 )
		{
			sprintf(pszCur  , "\n\n%s" ,ctePartSep ) ;
			pszCur = pszBuff + strlen( pszBuff ) ;
		}
		if( GetComment() != 0 )
		{
			sprintf(pszCur  , "%s %s : %s \n" ,ctePartStLine , GetName() , GetComment() ) ;
			pszCur = pszBuff + strlen( pszBuff ) ;
		}
		if( ( m_Attrib & fSymFunc ) != 0 )
		{
			sprintf(pszCur  , "%s\n\n" , ctePartStLine ) ;
			pszCur = pszBuff + strlen( pszBuff ) ;
		}

		if( ( m_Attrib & fSymPublic ) != 0 )
		{
			sprintf( pszCur , "public %s\n" , GetName() ) ;
			pszCur = pszBuff + strlen( pszBuff ) ;
		}
		if( ( m_Attrib & fSymExtern ) != 0 )
		{
			const char* pszTmp = GetName() ;
			sprintf( pszCur , "extern\t_imp__%s : dword\n%s equ _imp__%s\n\n" , pszTmp , pszTmp , pszTmp  ) ;	
		}
		else if( ( m_Attrib & fSymCode ) != 0 )
		{		
			if( ( m_Attrib & fSymFunc ) != 0 )
				sprintf(pszCur  , "%s :: ; proc near\n" , GetName() ) ;
			else // jump
				sprintf( pszCur  , "%s ::\n", GetName() );			
		}
		else
		{
			sprintf( pszCur  , "%s", GetName() );
		}
	}
	return pszBuff ;
}


CSymbol* CSymbol::GetSource() const 
{	return 0 ;}

