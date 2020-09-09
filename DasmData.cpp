/*		fichier DasmData.cpp : fichier implementation
 *
 *	descr : classe automate desassemblage des constantes
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

#include "DasmData.h"
#include <cassert>


////////////////////////////////////////////////////////////////////////
// class CDasmDataData : desassemblage d'une portion de code

CDasmData::CDasmData( )
{}


bool CDasmData::PassScan()
{
	CExeRep::CSymbolColl::iterator iter 
		= GetExe()->m_arRelocSymbols.lower_bound( &CSymbol( m_iIP ) );
	for( ; ( iter != GetExe()->m_arRelocSymbols.end() )
		&& ( (*iter)->m_VirtAdress < m_iNextIP ) ; iter ++ )
	{
		CSymbol* pRelocSymb = *iter ;
		CExeRep::CSymbolColl::const_iterator prevSym = GetExe()->m_arSymbols.find( & CSymbol( pRelocSymb->m_VirtAdress ) ) ;
		DWORD dwSymbVal = *(DWORD*)(GetExe()->va2ptr( pRelocSymb->m_VirtAdress )) ;
		CSection* pCurSection = GetExe()->GetSectFromVA( dwSymbVal ) ;
		if( pCurSection != 0 )
		{
			if( pCurSection->IsCode() )
				GetExe()->AddSymbol( dwSymbVal , pRelocSymb->m_VirtAdress , true , cSymCallBack ) ;	
			else
				GetExe()->AddSymbol( dwSymbVal , pRelocSymb->m_VirtAdress , true , cSymData ) ;	
			if( prevSym == GetExe()->m_arSymbols.end() )
			{
				if( m_pSection->IsData() )
					GetExe()->AddSymbol( pRelocSymb->m_VirtAdress ,0,true, cSymDataPtr ) ;
			}
			else
			{
				if( ( (*prevSym)->m_Attrib & fSymPointer ) == 0 )
					(*prevSym)->m_Attrib = cSymDataPtr ; 
			}
		}
		else	// improbable : signifie que la relocation pointe a l'exterieur de l'espace 
				// d'adressage de l'exe cible
			GetExe()->AddSymbol( dwSymbVal , pRelocSymb->m_VirtAdress , true , cSymUnknown ) ;
	}
	m_iIP = m_iNextIP ; 
	return true ;
}

bool CDasmData::PassEcho()
{
	static char_buff pszTemp ;
	unsigned int iSymbSize ;
	int iLineWrap = 8 ;
	iSymbSize = m_iNextIP - m_iIP ;

// sortie
	if( m_pvCur >= m_pSection->init_end() ) // partie section non initialisee
		fprintf( m_pFileOut ,"\tbyte %i dup(?)\n" , iSymbSize );
	else
	{
		iLineWrap = 8 ;
	// adresse d'un autre symbol	
		if( ( m_pSymbCur != 0 ) && ( m_pSymbCur->m_Attrib & fSymPointer ) != 0 ) 
		{			
			fprintf( m_pFileOut , "\tdword %s\n" , GetExe()->GetSymbName( *(DWORD*)m_pvCur , m_pSymbCur->m_VirtAdress  ,false ,cteDecorate | cteDecOffset ) );
			m_pvCur += 4 ;
		}
	// cas particuliers
		else if( iSymbSize == 2 )	// un seul word
		{
			fprintf( m_pFileOut , "\tword %s" , GetExe()->GetValue( *(WORD*)m_pvCur , 0 ) );
			m_pvCur += 2 ;
		}
		else if( iSymbSize == 4 )	// un seul dword
		{
			fprintf( m_pFileOut , "\tdword %s" , GetExe()->GetValue( *(DWORD*)m_pvCur , 0 ) );
			m_pvCur += 4 ;
		}
	// iteration octet suivant
		while( m_pvCur < m_pvNext )
		{
			int iNumreq ;
			CExeRep::symb_iterator iterReloc = GetExe()->m_arRelocSymbols.find( &CSymbol( GetExe()->ptr2va( m_pvCur ) ) ) ;
			
			if( iterReloc != GetExe()->m_arRelocSymbols.end() )
			{
				if( iLineWrap != 8 )
					fprintf( m_pFileOut , "\n" ) ;	
				fprintf( m_pFileOut , "\tdword %s\n" , GetExe()->GetSymbName( *(DWORD*)m_pvCur , m_pSymbCur->m_VirtAdress  ,true ,cteDecorate | cteDecOffset ) );		
				m_pvCur += 4 ;
				iLineWrap = 8 ;
			}
			else if( ( iNumreq = ReqString( m_pvCur , m_pvNext , pszTemp ) ) > 0 )
			{					
				m_pvCur += iNumreq ;
				if( iLineWrap != 8 )
					fprintf( m_pFileOut , "\n" ) ;
				fprintf( m_pFileOut , "\tbyte %s\n" , pszTemp );					
				iLineWrap = 8 ;
			}
			else
			{
				iNumreq = ReqDup( m_pvCur , m_pvNext , pszTemp ) ;
				if( iNumreq > 0  )
					m_pvCur += iNumreq ;
				else
					strcpy( pszTemp , GetExe()->GetValue( *m_pvCur++ , 0 ) ) ; 
				if( iLineWrap == 0 )
				{
					iLineWrap = 8 ;
					fprintf( m_pFileOut ,"\n");
				}
				if( iLineWrap == 8 )
					fprintf( m_pFileOut ,"\tbyte %s " , pszTemp );
				else
					fprintf( m_pFileOut ," , %s ", pszTemp ) ;
				iLineWrap -- ;
			}
		}
	};
// iteration symbol suivante		
	fprintf( m_pFileOut , "\n" ) ;
	m_iIP = m_iNextIP ; 
	return true ;
}

bool CDasmData::ScanNext()
{
	if( m_iPass == ctePassScan )
		return PassScan();
	else
		return PassEcho();
}

void CDasmData::PrintInstr()
{
}

void CDasmData::Reset() 
{
}

int CDasmData::ReqString( BYTE* pszCur,BYTE* pszEnd , char* pszBuff )
{
	int iRep = 0 ;
	BYTE* pszIn ;
	for( pszIn = pszCur ; pszIn < pszEnd ; ++ pszIn )
	{	
		if( iRep > 126 )
			break ;
		if( ( *pszIn >= 32 ) && ( *pszIn < 127 ) )
			++ iRep ;
		else 
			break ;
	}
	if( iRep >= 4 )
	{
		// double quotation des '
		char* pszOut = pszBuff ;
		*pszOut++ = '\'' ;
		for( pszIn = pszCur ; pszIn < ( pszCur + iRep ) ; ++pszIn )
		{
			if( *pszIn == '\'' )
				*pszOut++ = '\'' ;
			*pszOut++ = *pszIn ;
		}	
		*pszOut++ = '\'' ;
		*pszOut = '\0' ;
		return iRep ;
	}
	else
		return 0 ;
}

int CDasmData::ReqDup( BYTE* pszCur,BYTE* pszEnd , char* pszBuff )
{
	int iRep = 0 ;
	BYTE cRep ;
	for( cRep = *pszCur ; pszCur < pszEnd ; ++ pszCur)
	{
		if( cRep == *pszCur )
			++ iRep ;
		else 
			break ;
	}
	if( iRep >= 4 )
	{
		sprintf( pszBuff , "%i dup(%s)" , iRep , GetExe()->GetValue( cRep , 0 ) ) ;
		return iRep ;
	}
	else
		return 0 ;
}

//////////////////////////////////////////////////////////////////
// fonctions globales

