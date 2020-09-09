/*		fichier DasmCode.cpp : fichier implementation
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

#include "Config.h"

#include "DasmCode.h"
#include <algorithm>
#include <cassert>



////////////////////////////////////////////////////////////////////////
// definition des constantes

#include "DasmCte.h"

#define pAllBanksEnd ( pAllBanks + ( sizeof( pAllBanks ) / sizeof(SBank) ) )

	
/** indique si un ensemble d'instructions est disponible*/
bool IsBank( const char* pszBankName ) 
{
	SBank* pCur ; 
	for( pCur = pAllBanks ; pCur != pAllBanksEnd  ; ++ pCur )
	{
		if( strcmp( pCur->m_pszName , pszBankName ) == 0 )
			break ;
	}
	return 	pCur != pAllBanksEnd ;
}
	
/** si bLoad == true : charge le group , le decharge sinon */
void LoadBank( const char* pszBankName , bool bLoad ) 
{
	SBank* pCur ; 
	for( pCur = pAllBanks ; pCur != pAllBanksEnd  ; ++ pCur )
	{
		if( strcmp( pCur->m_pszName , pszBankName ) == 0 )
			break ;
	}
	if( pCur != pAllBanksEnd )
		pCur->m_bIsDefault = bLoad ;
}

void InitBanks() 
{
	vector< SOPContext >* pBuff = new vector< SOPContext >() ;
	for( SBank* pCur = &(pAllBanks[0]) ; pCur != pAllBanksEnd  ; ++ pCur )
	{
		if( pCur->m_bIsDefault )
		{
			for( SOPContext* pCtx = pCur->m_pData ; pCtx->m_pszMask != 0  ; ++ pCtx )
				pBuff->push_back( *pCtx ) ;
		}
	}
	SOPContext nullCtx = { 0 , 0 , 0 } ; 
	pBuff->push_back( nullCtx ) ;
	pfnOPCodeMask = pBuff->begin();
}

////////////////////////////////////////////////////////////////////////
// class CDasmCode : desassemblage d'une portion de code

CDasmCode::CDasmCode( )
{
	m_mode32 = 1 ; // 32bit par defaut
	m_OperandSize = 0 ;
}

void CDasmCode::Reset() 
{
	m_mode32 = 1 ;
}

bool CDasmCode::ScanNext( ) 
{
	bool bSuccess ;
	if( ! ScanPrefix() )
		return false ;
	m_iIP = GetExe()->ptr2va( m_pvPrefix ) ;
	m_OperandSize = ( ( m_mode32 ^ m_oprdSzOverd ) == 0 ) ? 2 : 3 ;
	m_AdresseSize = ( ( m_mode32 ^ m_adrSzOverd ) == 0 )  ? 2 : 3 ;
	if( GetExe()->m_arRelocSymbols.find( &CSymbol( m_iIP ) ) 
		!= GetExe()->m_arRelocSymbols.end() )
		return false ; // reloc en debut d'instr impossible
	bSuccess = ScanOPCode( ) ;
	if( bSuccess )
		m_iIP = GetExe()->ptr2va( m_pvCur ) ;
	return bSuccess  ;
}

bool CDasmCode::ScanPrefix() 
{
	int iNumPrefix = 0 ; 
	m_segPrefix = 0xff ;
	m_repPrefix = 0 ;
	m_oprdSzOverd = 0 ;
	m_adrSzOverd = 0 ;
	while( true )
	{
		if( iNumPrefix > 4 )	// - de 4 prefix
			return false; 
		switch( *m_pvCur )
		{
			case cteLockPrefix :
			case cteRepNENZPrefix :
			case cteRepPrefix :	
				m_repPrefix = *m_pvCur ;
				m_pvCur ++ ;
				iNumPrefix ++ ; 
				break ;
			case cteCSSRegPrefix : 
				m_segPrefix = 0x01 ;
				m_pvCur ++ ;
				iNumPrefix ++ ; 
				break ;
			case cteSSSRegPrefix : 
				m_segPrefix = 0x02 ;
				m_pvCur ++ ;
				iNumPrefix ++ ; 
				break ;
			case cteDSSRegPrefix : 
				m_segPrefix = 0x03 ;
				m_pvCur ++ ;
				iNumPrefix ++ ; 
				break ;
			case cteESSRegPrefix : 
				m_segPrefix = 0x00 ;
				m_pvCur ++ ;
				iNumPrefix ++ ; 
				break ;
			case cteFSSRegPrefix : 
				m_segPrefix = 0x04 ;
				m_pvCur ++ ;
				iNumPrefix ++ ; 
				break ;
			case cteGSSRegPrefix : 
				m_segPrefix = 0x05 ;
				m_pvCur ++ ;
				iNumPrefix ++ ; 
				break ;
			case cteOpSzOvrPrefix :
				m_oprdSzOverd = 1 ;
				m_pvCur ++ ;
				iNumPrefix ++ ;
				break ;
			case cteAdrSZOvrPrefix :
				m_adrSzOverd = 1 ;
				m_pvCur ++ ;
				iNumPrefix ++ ;
				break ; 
			default : //plus de prefixes
				m_pvPrefix = m_pvCur ;
				return true;
		}
	}
}

void CDasmCode::SortInstr() 
{
	static int iInstrCount = 0 ; // sert de ref pour la reorganisation du tableau pfnOPCodeMask
	static SOPContext* pEnd = 0 ; // pointe juste apres la fin du tableau pfnOPCodeMask
	
	++ iInstrCount ;
	if( iInstrCount >= 1024 )
	{
		iInstrCount = 0 ;
		if( pEnd == 0 ) 
		{	// initialise cette variable 1 fois
			for( pEnd = pfnOPCodeMask ; pEnd->m_pszMask != 0  ; ++ pEnd )	/*vide*/ ;
		}
		sort( pfnOPCodeMask , pEnd ) ;
	}
}

bool CDasmCode::ScanOPCode(  )
{
	for( SOPContext* pfnHandlerCur = pfnOPCodeMask ; pfnHandlerCur->m_pszMask != 0 ; pfnHandlerCur ++ )
	{
		if( ProcessMask( pfnHandlerCur->m_pszMask ) )
		{
			m_pszFormat = pfnHandlerCur->m_pszCodeOP ;
			++ ( pfnHandlerCur->m_iStat ) ; // mise a jour des stat d'utilisation
			SortInstr() ;
			return true ;
		}
		else
		{
			m_pvCur = m_pvPrefix ;
			continue ; // non reconnu	: on passe au masque suivant
		}
	}
	return false;	// aucunes instructions reconnues
}

bool CDasmCode::ProcessMask( const char* pszMask )
{
	BYTE  bBytePtr = 8 ; // position ds l'octet
	char_buff* ppszCurOprd = m_pszOperand ;
	DWORD dwData ;
	DWORD vaRef ;
	BYTE  cCur ;
	m_sFlag = false ;
	m_dFlag = false ;
	m_mmxPack = 0xff ;
while(true)	
	switch( cCur = *pszMask++ )
	{
	case '\0' : 
		return true ;
	case ' ' : continue ;
	case ':' :
		bBytePtr = 8 ;
		++ m_pvCur ;
		continue ;
	case '0' :
	case '1' :		
		if( ( ( cCur - '0' ) ^ ( ( (*m_pvCur) >> --bBytePtr ) & 1 ) ) != 0 )
			return false ;
		continue ;
	case 's' :
		m_sFlag = ( ( ( (*m_pvCur) >> --bBytePtr ) & 1 ) == 1 ) ? true : false ;
		continue ;	
	case 'd' :
		m_dFlag = ( ( ( (*m_pvCur) >> --bBytePtr ) & 1 ) == 1 ) ? true : false ;
		continue ;	
	case 'w' :
		if( ( ( (*m_pvCur) >> --bBytePtr ) & 1 ) == 0 )
			m_OperandSize = 1 ; // 8 bits
		continue ;	
	case 't' :	// condition flags
		bBytePtr -= 4 ;
		m_tttnFlag = ((*m_pvCur) >> bBytePtr ) & 15 ;
		pszMask += 3 ;
		continue ;
	case 'm' :	// mod
		ByteSplit3( *m_pvCur , m_Mod , m_regOpCode , m_RM ) ;
		bBytePtr = 8 - 2 ;
		pszMask += 2 ;
		continue ;
	case 'r' :	// rm (inclue le terminateur d'octet : )
		++m_pvCur ;
		if( ! ModMProcess( *ppszCurOprd++ ) )
			return false;
		bBytePtr = 8 ;
		++ pszMask ;
		continue;
	case '2' :	// 2sg
		bBytePtr -= 2 ;
		strcpy( *ppszCurOprd++ , cteSegReg[ ((*m_pvCur) >> bBytePtr ) & 3 ] ) ;		
		pszMask += 2 ;
		continue ;
	case '3' :  // 3sg
		bBytePtr -= 3 ;
		if( cteSegReg[ ((*m_pvCur) >> bBytePtr ) & 7 ] == 0 )
			return false ;
		else
			strcpy( *ppszCurOprd++ , cteSegReg[ ((*m_pvCur) >> bBytePtr ) & 7 ] ) ;
		pszMask += 2 ;
		continue ;
	case 'x':	// registre mmx
		bBytePtr -= 3 ;
		strcpy( *ppszCurOprd++ , cteMmxReg[ ((*m_pvCur) >> bBytePtr ) & 7 ] ) ;		
		pszMask += 2 ;		
		continue ;
	case 'f':	// registre fpu
		bBytePtr -= 3 ;
		strcpy( *ppszCurOprd++ , cteFpuReg[ ((*m_pvCur) >> bBytePtr ) & 7 ] ) ;		
		pszMask += 2 ;		
		continue ;
	case 'p':	// empacketage mmx
		bBytePtr -= 2 ;
		m_mmxPack = ((*m_pvCur) >> bBytePtr ) & 3 ;
		pszMask += 1 ;		
		continue ;
	case 'g' :	// grg
		bBytePtr -= 3 ;
		if( pszMask[0] == 'r' )
		{
			if( cteGenReg[m_OperandSize][ ((*m_pvCur) >> bBytePtr ) & 7 ] == 0 )
				return false ;
			else
				strcpy( *ppszCurOprd , cteGenReg[m_OperandSize][ ((*m_pvCur) >> bBytePtr ) & 7 ] ) ;
		}
		else if( pszMask[0] == '0')
			strcpy( *ppszCurOprd , cteGenReg[1][ ((*m_pvCur) >> bBytePtr ) & 7 ] ) ;
		else if( pszMask[0] == '1' )
			strcpy( *ppszCurOprd , cteGenReg[2][ ((*m_pvCur) >> bBytePtr ) & 7 ] ) ;
		else
		{
			strcpy( *ppszCurOprd , cteGenReg[3][ ((*m_pvCur) >> bBytePtr ) & 7 ] ) ;
			assert( pszMask[0] == '3' );
		}
		ppszCurOprd++ ;
		pszMask += 2 ;	
		continue ;
	case '_' :	// control debug reg
		bBytePtr -= 3 ;
		if( *pszMask == 'c' )
		{
			if( cteCtrlReg[ ((*m_pvCur) >> bBytePtr ) & 7 ] == 0 )
				return false ;
			else
				strcpy( *ppszCurOprd++ , cteCtrlReg[ ((*m_pvCur) >> bBytePtr ) & 7 ] ) ;
		}
		else if( *pszMask == 'd' )
		{
			if( cteDebReg[ ((*m_pvCur) >> bBytePtr ) & 7 ] == 0 )
				return false ;
			else
				strcpy( *ppszCurOprd++ , cteDebReg[ ((*m_pvCur) >> bBytePtr ) & 7 ] ) ;
		}
		else 
			assert(0) ;
		pszMask += 2 ;	
		continue ;
	case 'e' :	// eax
		if( cteGenReg[m_OperandSize][0] == 0 )
			return false ;
		else
			strcpy( *ppszCurOprd++ , cteGenReg[m_OperandSize][0] ) ;
		pszMask += 2 ;
		continue ;
	case 'o' :		// operand /adress size override
	case 'n' :
		pszMask += 2 ;
		if( pszMask[-2] == 'o')
		{
			if( ( m_oprdSzOverd ^ ( pszMask[-3] == 'n') )== 0 )
				return false ;
		}
		else if( pszMask[-2] == 'a' )
		{
			if( ( m_adrSzOverd ^ ( pszMask[-3] == 'n') ) == 0 )
				return false ;
		}
		else
			assert(0) ;
		continue ;
	case 'j' :	// jmp
	case 'c' :
		if( pszMask[1] == '8' )
		{
			dwData = (signed int)*(signed char*)m_pvCur ;
			dwData += GetExe()->ptr2va( m_pvCur ) + 1; // rel a l'instr suivante
			if( m_iPass == ctePassScan )
				GetExe()->AddSymbol( dwData ,0,true , cSymJmp ) ;
			m_pvCur ++ ;
		}
		else	// == 32
		{
			dwData = *(DWORD*)m_pvCur ;
			dwData += GetExe()->ptr2va( m_pvCur ) + 4 ; // rel a l'instr suivante
			if( m_iPass == ctePassScan )
			{
				if( cCur == 'j' )
					GetExe()->AddSymbol( dwData ,0,true , cSymJmp ) ;
				else
					GetExe()->AddSymbol( dwData ,0,true , cSymFunc ) ;
			}
			m_pvCur += 4 ;
		}
		if( m_iPass == ctePassEcho )
			strcpy( *ppszCurOprd++ , GetExe()->GetSymbName( dwData , 0 , true , 0 ) ) ;
		pszMask += 2 ;
		bBytePtr = 8 ;
		continue ;
	case 'a' :
	case 'i' : // donnees ou adresse 
		BYTE bSize ;
		bool bAdress ;
		bool sFlag ;
		vaRef = GetExe()->ptr2va( m_pvCur ) ;
		bAdress = ( cCur == 'a' ) ;
		if( pszMask[1] == '0' )	// determination par context
		{
			sFlag = m_sFlag ;
			bSize = m_OperandSize ;
		}
		else
		{
			sFlag = ( pszMask[0] == 's' ) ;
			if( pszMask[1] == '8'  )
				bSize = 1 ; 
			else if( pszMask[1] == '6' )
				bSize = 2 ; 
			else
				bSize = 3 ; 
		};
		if(sFlag)
		{
			dwData = (signed int)*(signed char*)m_pvCur ;
			m_pvCur ++ ;
			if( m_iPass == ctePassEcho )
				strcpy( *ppszCurOprd , GetExe()->GetValue(dwData ,0 ) ) ;
		}
		else if( bSize == 1 )
		{

			dwData = *m_pvCur ;	
			m_pvCur ++ ;
			if( m_iPass == ctePassEcho )
				strcpy( *ppszCurOprd , GetExe()->GetValue(dwData , 0 ) ) ;
		}
		else if( bSize == 2 )	
		{
			dwData = *(WORD*)m_pvCur ;
			m_pvCur += 2 ;
			if( m_iPass == ctePassEcho )
				strcpy( *ppszCurOprd , GetExe()->GetValue(dwData, 0 ) ) ;
		}
		else
		{
			dwData = *(DWORD*)m_pvCur ;
			if( ( m_iPass == ctePassScan ) && ( m_segPrefix == 0xff ) )
				GetExe()->AddSymbol( dwData ,vaRef,bAdress, cSymData );  // symbol ou constante ?
			if( m_iPass == ctePassEcho )	
			{
				if( bAdress )
				{
					if( m_segPrefix != 0xff )
						sprintf( *ppszCurOprd, "%s:[%s]" ,cteSegReg[m_segPrefix]
							,GetExe()->GetValue( dwData , cteDecorate ) ) ;
					else
						strcpy( *ppszCurOprd 
						, GetExe()->GetSymbName( dwData , vaRef , true , cteDecorate ) ) ;				
				}
				else
					strcpy( *ppszCurOprd 
					, GetExe()->GetSymbName( dwData , vaRef , false , cteDecorate | cteDecOffset ) ) ;
			}
			ppszCurOprd ++ ;
			m_pvCur += 4;
		}
		++ ppszCurOprd ;
		pszMask += 2 ;
		bBytePtr = 8 ;
		continue ;
	default :
		return false ;
	}
}

void CDasmCode::PrintInstr(  )
{
	static char_buff pszTemp ;
	char* pszOut = m_pszInstr ;
	const char* pszCurFmt  ;
	const char* pszCur ;
	bool bPtrQualif = false;

// 1ere passe recuperation operandes
	for( pszCurFmt = m_pszFormat ; *pszCurFmt != '\0'; pszCurFmt ++ )
	{
		if( *pszCurFmt == '%' )
		{
			pszCurFmt++ ;
			switch( *pszCurFmt )
			{
				case '0':
				case '1':
				case '2':
					if( bPtrQualif )
					{
						strcpy( pszTemp ,  m_pszOperand[ *pszCurFmt - '0']  );
						sprintf( m_pszOperand[ *pszCurFmt - '0'] ,"%s %s" , cteOperandQualif[ m_OperandSize ] , (const char*)pszTemp) ;
					}
					bPtrQualif = false ;
					break ;
				case 'p':
					bPtrQualif = true ;			
					break ;
			}
		}
	}
// inversion des operandes %0 et %1 si besoin
	if( m_dFlag )						
	{	
		strcpy( pszTemp , m_pszOperand[ 0 ] ) ;
		strcpy( m_pszOperand[ 0 ] , m_pszOperand[ 1 ] ) ;
		strcpy( m_pszOperand[ 1 ] , pszTemp ) ;
	}

// 2eme passe emission du source
	for(  pszCurFmt = m_pszFormat ; *pszCurFmt != '\0'; pszCurFmt ++ )
	{
		if( *pszCurFmt == '%' )
		{
			pszCurFmt++ ;
			switch( *pszCurFmt )
			{
				case '0':
				case '1':
				case '2':
					pszCur = m_pszOperand[ *pszCurFmt - '0'] ;
					for(  ; *pszCur != '\0' ; ++pszCur ) 
						*pszOut++ =	*pszCur	;
					break ;
				case 'p':		
					break ;
				case 'c':
					for( pszCur = cteConditions[ m_tttnFlag ] ; *pszCur != '\0' ; ++pszCur ) 
						*pszOut++ =	*pszCur	;
					break ;
				case 'g':
					for( pszCur = cteMmxPack[ m_mmxPack ] ; *pszCur != '\0' ; ++pszCur ) 
						*pszOut++ =	*pszCur	;
					break ;
				case 'r':
					pszCurFmt++ ;
					if( m_repPrefix != 0 )
					{						
						const char* pszPrefix = 0 ;
						if( *pszCurFmt == '_' )
							pszPrefix = cteRep[ m_repPrefix - 0xf0 ] ;
						else if(*pszCurFmt == 'z' )
							pszPrefix = cteRepZ[ m_repPrefix - 0xf0 ] ;
						else 
							assert(0);
						for( pszCur = pszPrefix ; *pszCur != '\0' ; ++pszCur ) 
							*pszOut++ =	*pszCur	;

					}
					break ;
				default:
					assert(0) ;
			}
		}
		else
			*pszOut++ = *pszCurFmt ;
	}
	*pszOut = '\0' ;
	if( GetExe()->m_bLineNum || GetExe()->m_bRawData  )
		FillChar( m_pszInstr ) ;
	fprintf( m_pFileOut ,"\t%s", m_pszInstr ) ;
}

//////////////////////////////////////////////////////////////////
// fonctions d'aides

bool CDasmCode::ModMProcess( char* pszBuff )
{
	bool bRet = true ;
	unsigned int uDisp , uVARef ;

// on exclue le mode registre : traite a part 
	if( m_Mod ==0x03 )
		return false ;
	
	char* pszIdx = pszBuff ;
	uVARef = GetExe()->ptr2va( m_pvCur ) ;

// registre segment override
	if( ( m_iPass == ctePassEcho ) && ( m_segPrefix != 0xff ) )
	{
		sprintf( pszIdx, "%s:" ,cteSegReg[m_segPrefix] ) ;
		pszIdx = pszBuff + strlen( pszBuff ) ;
	}

// utilisation du SIB	
	if( m_RM == 0x04 )
		return SIBProcess( pszIdx ) ;
	else if( m_Mod == 0x00 )
	{
		if( m_RM == 0x05 )	// cas particulier [ variable ]
		{
			uDisp = *(DWORD*)m_pvCur ;
			if( ( m_iPass == ctePassScan ) && ( m_segPrefix == 0xff ) )		
				GetExe()->AddSymbol( uDisp , 0 , true ,cSymData );
			if( m_iPass == ctePassEcho )
			{					
				if( m_segPrefix != 0xff )
					sprintf( pszIdx ,"[%s]",GetExe()->GetValue( uDisp , cteDecorate ) ) ;
				else					
					strcpy( pszIdx , GetExe()->GetSymbName(uDisp,0,true ,cteDecorate ) );
			}
			m_pvCur += 4 ;
		}
		else				// [reg]
			if( m_iPass == ctePassEcho )
			{
				if( cteGenReg[3][m_RM] == 0 )
					return false ;
				else
					sprintf( pszIdx , "[%s]" , cteGenReg[3][m_RM] ) ;
			}
	}
	else if( m_Mod == 0x01 )
	{		// disp8[reg]
		uDisp = (signed int)(*((signed char*)m_pvCur++));
		if( m_iPass == ctePassEcho )
		{
			if( cteGenReg[3][m_RM] == 0 )
				return false ;
			else
				sprintf( pszIdx , "[ %s %s ]" , cteGenReg[3][m_RM] 
					, GetExe()->GetValue( uDisp , cteFmtInteger | cteFmtSign ) ) ;
		}
	}
	else 
	{	// disp32[reg]
		assert( m_Mod == 0x02 ) ;
		uDisp = *(DWORD*)m_pvCur ;
		if( m_iPass == ctePassScan )		
			GetExe()->AddSymbol( uDisp , uVARef , false , cSymData ) ;
		if( m_iPass == ctePassEcho )
		{
			if( cteGenReg[3][m_RM] == 0 )
				return false ;
			else
				sprintf( pszIdx , "[ %s + %s ]" , cteGenReg[3][m_RM] 
				, GetExe()->GetSymbName(uDisp, uVARef , false , 0) ) ;
		}
		m_pvCur += 4 ;	
	}
	return bRet ;
}

bool CDasmCode::SIBProcess( char* pszBuff )
{
	char* pszIdx = pszBuff ;
	BYTE	Scale , Index , Base ;
	unsigned int uDisp = 0 ;
	ByteSplit3( *m_pvCur++ , Scale , Index , Base ) ;
	unsigned int uVARef = GetExe()->ptr2va( m_pvCur ) ;

//base
	if( ( Base == 0x05 ) && ( m_Mod == 0x00 ) )
	{	// cas particulier : pas de registre de base
		if( Index == 0x04 )	// pas d'index non plus -> quit
			return false ;
		if( m_iPass == ctePassEcho )
			strcpy( pszIdx , "[ " );		
	}
	else if( m_iPass == ctePassEcho )
	{
		if(  Index == 4  )
			sprintf( pszIdx , "[ %s" , cteGenReg[3][Base] ) ;
		else
			sprintf( pszIdx , "[ %s + " , cteGenReg[3][Base] ) ;
	};
	if( m_iPass == ctePassEcho )
		pszIdx = pszBuff + strlen( pszBuff ) ;

// scale index		
	if( ( Index != 4 ) && ( m_iPass == ctePassEcho ) )		//sinon pas d'index
	{
		if( Scale == 0 ) // pas d'echelle
			sprintf( pszIdx , "%s" , cteGenReg[3][Index]);
		else
			sprintf( pszIdx , "%i * %s" , 1 << Scale , cteGenReg[3][Index] );
	}
	pszIdx = pszBuff + strlen( pszBuff ) ;

// offset 
	if( m_Mod == 0x01 )					// disp8
		uDisp = (signed int)(*((signed char*)m_pvCur++)) ;
	else if( ( m_Mod == 0x02 ) || ( ( Base == 0x05 ) && ( m_Mod == 0x00 )  )) //disp32
	{
		uDisp = *(DWORD*)m_pvCur ;
		if( m_iPass == ctePassScan )	
			GetExe()->AddSymbol( uDisp , uVARef , false , cSymData  );
		m_pvCur += 4 ;
	}
	
	if( m_iPass == ctePassEcho )
	{
		if( uDisp == 0 )
			strcpy( pszIdx ," ]" ) ;
		else if( ( m_Mod == 0x00 ) || ( m_Mod == 0x02 ) )	//disp32
			sprintf( pszIdx , " + %s ]" , GetExe()->GetSymbName( uDisp , uVARef , false , 0 ) ) ;
		else											// disp8
			sprintf( pszIdx , " %s ]" , GetExe()->GetValue( uDisp , cteFmtInteger | cteFmtSign ) ) ;				
	}		
	return true ;
}


//////////////////////////////////////////////////////////////////
// fonctions globales

