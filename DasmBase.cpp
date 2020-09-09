/*		fichier DasmBase.cpp : fichier implementation
 *
 *	descr : classe service communs automate desassembleur code donnes
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

#include <assert.h>

#include "DasmBase.h"
#include "DasmData.h"
#include "DasmCode.h"

////////////////////////////////////////////////////////////////////////
// class CDasm : desassemblage d'une portion de code

CDasm::CDasm()
{
	memset( this , 0 , sizeof(CDasm) ) ;
}

CDasm& CDasm::SetWindow( CSection* pSection )
{
	m_pSection = pSection ;
	return *this ;
}

CDasm& CDasm::SetFile( FILE* pFile ) 
{
	m_pFileOut = pFile ;
	return *this ;
}

void CDasm::Run( int iPass  ) 
{
	m_iPass = iPass ;

// choix du mode par defaut
	if( m_pSection->IsCode() )
		m_iMode = cteCodeMode ;
	else
		m_iMode = cteDataMode ;
// condition initiale
	m_iIBeginIP = m_pSection->va_begin() ;	
	m_iIP = m_iIBeginIP ;
	m_iLastIP = m_iIP ;
	
	m_pSymbCur = 0 ;
	m_pSymbNext =  GetExe()->FindSymbol( m_iIP ) ;
	if( ( m_pSymbNext != 0 ) && ( m_pSymbNext->m_VirtAdress == m_iIP ) )
	{
		m_pSymbCur = m_pSymbNext ;
		m_pSymbNext = GetExe()->FindSymbol( m_iIP + 1  ) ;
	}
	if( ( m_pSymbNext != 0 ) && ( m_pSymbNext->m_VirtAdress < va_end() ) )
		m_iNextIP = m_pSymbNext->m_VirtAdress ; 
	else
		m_iNextIP = va_end() ;

	m_pvBegin = GetExe()->va2ptr( m_iIBeginIP ) ;
	m_pvNext = GetExe()->va2ptr( m_iNextIP ) ;	
	m_pvCur = m_pvBegin ;


	while( m_iIP < va_end() )
	{			
		Reset();
		if( ( m_iPass == ctePassEcho ) && ( m_pSymbCur != 0 ) && ( m_pSymbCur->m_VirtAdress < va_end() ) )
			fprintf( m_pFileOut ,"\n%s", m_pSymbCur->GetDeclaration() ) ;	
	// choix du mode en fct de la nature du symbol 
		if( m_pSymbCur != 0 )
		{
			if( ( m_pSymbCur->m_Attrib & fSymCode ) != 0 )
				m_iMode = cteCodeMode ;
			else
				m_iMode = cteDataMode ;
		}

		while( m_iIP < m_iNextIP )
		{
			m_iLastIP = m_iIP ;
			if( ScanNext() && ( m_iIP <= m_iNextIP ) )
			{
				if( m_iPass == ctePassEcho )
				{
					PrintInstr();
					if( GetExe()->m_bLineNum || GetExe()->m_bRawData )
						fprintf( m_pFileOut ,";" ) ;
					if( GetExe()->m_bLineNum )
						fprintf( m_pFileOut , "0x%08x :\t" , m_iLastIP ) ;
					if( GetExe()->m_bRawData )
					{
						BYTE* pCur = GetExe()->va2ptr( m_iLastIP );
						BYTE* pEnd = GetExe()->va2ptr( m_iIP );
						if( ( pEnd - pCur ) < 32 )
						{
							for( ; pCur < pEnd ; ++ pCur )
							{
								fprintf( m_pFileOut ,"%02x" , *pCur );
							}
						}
					}
					fprintf( m_pFileOut ,"\n" ) ;
				}
			}
			else
			{	// on change de mode
				m_iMode = 1 - m_iMode ;
				m_iIP = m_iLastIP ;
				m_pvCur = GetExe()->va2ptr( m_iLastIP );			
				Reset();
				continue ;
			}
		}
	// iteration bloc inter symbol
		m_iIBeginIP = m_iNextIP ;
		m_iIP = m_iIBeginIP ;
		m_pSymbCur = m_pSymbNext ;
		m_pSymbNext = GetExe()->FindSymbol( m_iIP + 1 ) ;
		if( ( m_pSymbNext != 0 ) && ( m_pSymbNext->m_VirtAdress < va_end() ) )
			m_iNextIP = m_pSymbNext->m_VirtAdress ; 
		else
			m_iNextIP = va_end() ;	
		m_pvBegin = GetExe()->va2ptr( m_iIBeginIP ) ;
		m_pvCur = m_pvBegin  ;
		m_pvNext = GetExe()->va2ptr( m_iNextIP ) ;
	}
}

unsigned int CDasm::va_end()
{
	unsigned int uRet ;
	if( m_pSection->IsCode() )
	{
		uRet = m_pSection->va_init_end() ;
		if( uRet == m_pSection->va_begin() ) 
			uRet = m_pSection->va_end() ;
	}
	else
		uRet = m_pSection->va_end() ;
	return uRet  ;
}

void CDasm::Reset()
{
	if( m_iMode == cteCodeMode )
		static_cast< CDasmCode* >(this)->Reset() ;
	else
		static_cast< CDasmData* >(this)->Reset() ;
}

bool CDasm::ScanNext( ) 
{
	if( m_iMode == cteCodeMode )
		return static_cast< CDasmCode* >(this)->ScanNext(  ) ;
	else
		return static_cast< CDasmData* >(this)->ScanNext(  ) ;
}

void CDasm::PrintInstr( ) 
{
	if( m_iMode == cteCodeMode )
		static_cast< CDasmCode* >(this)->PrintInstr( ) ;
	else
		static_cast< CDasmData* >(this)->PrintInstr( ) ;
}

//////////////////////////////////////////////////////////////////
// fonctions globales
CDasm* CreateCDasm()
{
	BYTE* pBuff = new BYTE[ max( sizeof(CDasmData) , sizeof(CDasmCode) ) ] ;
	CDasm* pDasm = new( pBuff ) CDasm ;
	return pDasm ;
}

