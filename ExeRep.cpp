/*		fichier exeRep.cpp : fichier implementation
 *
 *	descr : classe qui represente un binaire
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

#include <algorithm>
#include <cassert>

#include "ExeRep.h"
#include "DasmBase.h"



//////////////////////////////////////////////////////////////////
// implementation class CSection 

CSection::CSection( int idSection )
	: m_idSection( idSection ) 
{
	m_pHeader = GetExe()->m_arSectHeader[ idSection ] ; 
	m_pRawData = GetExe()->rva2ptr( m_pHeader->VirtualAddress );
	m_pImageDir = 0 ;
}

// information
unsigned int CSection::size()				
{ return RoundUp( max ( m_pHeader->SizeOfRawData , m_pHeader->Misc.VirtualSize ), 4096 ) ;}
	
unsigned int CSection::init_size()		
{	return m_pHeader->Misc.VirtualSize ; } ;

BYTE* CSection::begin()		
{	return m_pRawData ; }; 

BYTE* CSection::end()			
{	return m_pRawData + size() ; };

BYTE* CSection::init_end()	
{	return m_pRawData + init_size() ; };

unsigned int CSection::va_begin()	
{	return GetExe()->rva2va( m_pHeader->VirtualAddress ) ; };

unsigned int CSection::va_end()	
{	return GetExe()->rva2va( m_pHeader->VirtualAddress ) + size() ; };

unsigned int CSection::va_init_end()
{	return GetExe()->rva2va( m_pHeader->VirtualAddress ) + init_size() ; };

DWORD CSection::GetFlags()
{
	return m_pHeader->Characteristics ;
}

bool CSection::IsCode()
{
	return (( GetFlags() & IMAGE_SCN_CNT_CODE ) != 0 ) 
		|| (( GetFlags() & IMAGE_SCN_MEM_EXECUTE ) != 0 ) ;
}

bool CSection::IsData()
{
	bool bRet = ( GetFlags() & (IMAGE_SCN_CNT_UNINITIALIZED_DATA | IMAGE_SCN_CNT_INITIALIZED_DATA ) ) != 0 ;	
	bRet &= ( ( GetFlags() & IMAGE_SCN_LNK_NRELOC_OVFL ) == 0 ) ;
	bRet &= ( ( GetFlags() & IMAGE_SCN_MEM_DISCARDABLE ) == 0 ) ;
	bRet &=	( strcmp( (const char*)&(m_pHeader->Name[0]) ,".rsrc") != 0 );
	bRet &=	( strcmp( (const char*)&(m_pHeader->Name[0]) ,".reloc") != 0 );
	return bRet ;
}

//////////////////////////////////////////////////////////////////
// implementation class CExeRep 
/** singleton exe */
CExeRep	m_theExe ;

	//////////////////////////////////////////////////////////////
	// initialisation

CExeRep::CExeRep()
{}

CExeRep::~CExeRep()
{
	for( int idSection = 0 ; idSection < m_arRawSections.size() ; idSection ++)
	{
		delete m_arRawSections[ idSection ] ;	
	}
	CSymbolColl::iterator iter ;
	for( iter = m_arSymbols.begin() ;iter != m_arSymbols.end(); iter ++)
		delete *iter ;	
}

void CExeRep::SetFileName( const char* pszFileIn , const char* pszFileOut )
{
	int iTmpIdx ;

	if( strchr( pszFileIn , PATH_SEP ) != 0 )
	{
		const char* pTmpStr = strrchr( pszFileIn , PATH_SEP ) ;
		iTmpIdx = pTmpStr - pszFileIn ;
		m_strPath = string( pszFileIn , iTmpIdx ) ;
		++ pTmpStr ;
		iTmpIdx = strrchr( pszFileIn , '.' ) -  pTmpStr ;
		m_strBaseName = string( pTmpStr , strrchr( pszFileIn , '.' ) ) ;
	}
	else
	{
		char pszTmp[128] ;
		GetCurrentDirectory( 127 , pszTmp );
		m_strPath = pszTmp ;
		if( strchr( pszFileIn , '.' ) != 0 )
		{
			iTmpIdx = strrchr( pszFileIn , '.' ) - pszFileIn ;
			m_strBaseName = string( pszFileIn , iTmpIdx ) ;
		}
		else
			m_strBaseName = pszFileIn ;
	}
	if( pszFileOut != 0 )
		m_strOutput = pszFileOut ;
	else 
		m_strOutput = string( m_strBaseName + string(".asm") ).c_str() ;

}

void CExeRep::ProcessExe( const char* pszFileIn , const char* pszFileOut )
{
	SetFileName( pszFileIn , pszFileOut );
	if( LoadBin( pszFileIn ) )
	{
		CDasm* pDasm = CreateCDasm() ;		
		if( !HasReloc() ) // test si manque relocations
			printf( "warning : missing relocation info , disassembly source may be inaccurate\n") ;
		printf( "Collecting directory entry symbols\n" ) ;
		AddDirEntrySymb() ;
		AddDllSymb() ;		
		AddRelocSymb() ;
		AddExportSymb() ;
		AddEntryPoint() ;

// boucle sections	desassemblage passe d'analyse
		vector< CSection* >::iterator iterSect  ;		
		for( int iDasmStep = 1 ; iDasmStep < m_iPassNum ; ++ iDasmStep )
		{
			printf( "Desassembling pass %i\n" , iDasmStep ) ;		

			for( iterSect = m_arRawSections.begin() + 1 ; iterSect != m_arRawSections.end() ; ++ iterSect )
			{
				if( (*iterSect)->IsCode() || (*iterSect)->IsData() )
				{
					pDasm->SetWindow( *iterSect ) ;
					pDasm->Run( ctePassScan ) ;
				}
			}
		}

		FILE* pOutFile = fopen( m_strOutput.c_str() ,"wb") ;
		pDasm->SetFile( pOutFile ) ;
		OutputMainHeader( pOutFile ) ;

// boucle sections	desassemblage pass de sortie asm
		printf( "Desassembling pass %i \n" ,m_iPassNum ) ;
		for( iterSect = m_arRawSections.begin() + 1  ; iterSect != m_arRawSections.end() ; ++ iterSect )
		{
			if( (*iterSect)->IsCode() || (*iterSect)->IsData() )
			{	
				pDasm->SetWindow( *iterSect ) ;
				OutputSectionHeader( *iterSect , pOutFile );
				pDasm->Run( ctePassEcho ) ;
			}
		}
		fprintf( pOutFile , "\nend\n" );
		delete pDasm ;
		fclose( pOutFile ) ;
	}
}

bool CExeRep::LoadBin( const char* pszName ) 
{
	BYTE* pFilePtr ;
	DWORD iFileSize = GetImageSize( pszName ) ;
	if( iFileSize == 0 )
		return false ;
// ouverture fichier et allocation
	FILE* 	pFile = fopen( pszName , "rb" ) ;
	if( pFile == 0 )
	{
		printf( "error: unable to open file : %s \n" , pszName ) ;
		return false ;
	}
	m_pRawData = (BYTE*)VirtualAlloc( 0 , iFileSize , MEM_COMMIT , PAGE_READWRITE ) ;
	pFilePtr = m_pRawData ;
	if( m_pRawData == 0 )
	{
		printf( "error: unable to allocate memory\n") ;		
		return false ;
	}

// lecture headers
	m_pDosHeader = (_IMAGE_DOS_HEADER*) pFilePtr ;
	fread( m_pDosHeader , sizeof( _IMAGE_DOS_HEADER ) , 1 , pFile );
	fseek( pFile , m_pDosHeader->e_lfanew , SEEK_SET ) ;	
	pFilePtr += m_pDosHeader->e_lfanew ;
	fread( pFilePtr , 4 , 1 , pFile ) ;
	m_dwNTSign =  *((DWORD*)pFilePtr) ;
	pFilePtr += 4 ;
	m_pNTHeader = (_IMAGE_FILE_HEADER*)( pFilePtr ) ;
	fread( m_pNTHeader , sizeof(_IMAGE_FILE_HEADER) , 1 , pFile ) ;	
	pFilePtr += sizeof( _IMAGE_FILE_HEADER ) ;
	m_pNTOptHeader = (IMAGE_OPTIONAL_HEADER*)( pFilePtr ) ;
	fread( m_pNTOptHeader , sizeof( IMAGE_OPTIONAL_HEADER ) , 1 , pFile ) ;
	pFilePtr += sizeof( IMAGE_OPTIONAL_HEADER ) ;
	if( m_pNTHeader->SizeOfOptionalHeader > sizeof( IMAGE_OPTIONAL_HEADER ) )// avance vers la table des sections
	{
		fseek( pFile , m_pNTHeader->SizeOfOptionalHeader - sizeof( IMAGE_OPTIONAL_HEADER ) , SEEK_CUR ) ;	
		pFilePtr += m_pNTHeader->SizeOfOptionalHeader - sizeof( IMAGE_OPTIONAL_HEADER ) ;	
	}

	m_vaOffset = reinterpret_cast<unsigned int>( m_pRawData ) - m_pNTOptHeader->ImageBase ;

// verifications
	if( m_dwNTSign != IMAGE_NT_SIGNATURE )
		printf( "warning : bad NT image signature\n" ) ;
	if( m_pNTHeader->Machine != IMAGE_FILE_MACHINE_I386 )
	{
		printf( "error: unsupported processor\n");
		return false ;
	}
	if( ( ( m_pNTHeader->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE ) == 0 ) 
		|| ( m_pNTHeader->SizeOfOptionalHeader == 0 ) )
	{
		printf( "error: obj or lib files unsupported\n" );
		return false ;	
	}
	
	m_arSectHeader.resize( m_pNTHeader->NumberOfSections + 1 ) ;
	
	// on considere le header comme une section ( des RVA utiles pointent dessus )	
	IMAGE_SECTION_HEADER* pHeaderSect = new IMAGE_SECTION_HEADER ;
	memset( pHeaderSect , 0 , sizeof(IMAGE_SECTION_HEADER) ) ;
	strcpy( (char*)pHeaderSect->Name , "header" ) ;
	pHeaderSect->SizeOfRawData = m_pNTOptHeader->SizeOfHeaders ;
	pHeaderSect->Misc.VirtualSize = m_pNTOptHeader->SizeOfHeaders ;
	pHeaderSect->VirtualAddress = m_pNTOptHeader->ImageBase ;
	m_arSectHeader[0] = pHeaderSect ;
	for( int idSect = 1 ; idSect < m_arSectHeader.size() ; idSect ++ )
	{
		m_arSectHeader[ idSect ] = (IMAGE_SECTION_HEADER*)pFilePtr ;		
		fread( m_arSectHeader[ idSect ] , sizeof( IMAGE_SECTION_HEADER ) , 1 , pFile ) ;
		pFilePtr += sizeof( IMAGE_SECTION_HEADER ) ;
	}
// lecture sections
	m_arRawSections.resize( m_arSectHeader.size() );
	for( idSect = 0 ; idSect < m_arSectHeader.size() ; idSect ++ )
	{
		m_arRawSections[idSect] = new CSection( idSect ) ;
		fseek( pFile , m_arSectHeader[ idSect ]->PointerToRawData , SEEK_SET ) ;
		fread( m_arRawSections[idSect]->begin() , m_arSectHeader[ idSect ]->SizeOfRawData , 1 , pFile ) ;		
	}
	DWORD dwTmp ; VirtualProtect( m_pRawData ,m_pNTOptHeader->SizeOfImage , PAGE_READONLY , &dwTmp ) ;
	fclose( pFile );
	return true;
}

bool CExeRep::ScanConfFile( FILE* pFile )
{
	char pszTmpBuff[4096] ;
	char* pszCur ;
	unsigned int iAdr = 0 ;
	unsigned int  iType ;
	
	for( int iLine = 0 ; fgets( pszTmpBuff , sizeof(pszTmpBuff) - 1 , pFile ) != 0 ; ++ iLine )
	{
		if( pszTmpBuff[0] == '#' || pszTmpBuff[0] == '\n' )				// ligne commentaire
			continue ;
		
		pszCur = strtok( pszTmpBuff , " \t,") ;
		if( pszCur == 0 ) 
		{
			printf("config file error line %i \n" , iLine  ) ;
			return false ;
		}
		else if( ( pszTmpBuff[0] == '0') && ( pszTmpBuff[1] == 'x' ) )	// ligne symbol
		{	
			char* pszSymbName = 0 ; 
			char* pszComment = 0 ;
			if( ( sscanf( pszCur , "%x" , &iAdr ) == EOF ) || ( iAdr  == 0 ) )
			{
				printf("config file error line %i : bad adresse format %s \n" , iLine , pszCur ) ;
				return false ;
			}

			if( ( pszCur = strtok( 0 , " \t\r\n,") ) == 0 )
			{
				printf("config file error line %i : missing symbol type specification \n" , iLine  ) ;
				return false ;
			}			
				
			if( stricmp( pszCur , "data") == 0 )
				iType = cSymData | fSymUser ;
			else if( stricmp( pszCur , "funct") == 0 )
				iType = cSymFunc | fSymUser  ;
			else if( stricmp( pszCur , "ptr") == 0 )
				iType = cSymDataPtr | fSymUser ;
			else if( stricmp( pszCur , "label") == 0 )
				iType = cSymJmp | fSymUser ;
			else
			{
				printf("config file error line %i : %s bad symbol type \n" , iLine , pszCur ) ;
				return false ;
			}

			pszCur = strtok( 0 , " \r\n\t,") ;			// nom de symbol optionnel 
			if( pszCur != 0 )
			{
				pszSymbName = StrDup( pszCur ) ;
				pszCur = strtok( 0 , "\n") ;		// commentaire optionnel -> fin de ligne
				if( ( pszCur != 0 ) && ( strlen( pszCur ) > 0 ) ) 
					pszComment = StrDup( pszCur ) ;
			}
			CSymbol* pSymbol = new CSymbol( iAdr , iType , pszSymbName , pszComment ) ;
			m_arSymbols.insert( pSymbol ) ;
		}
		else												// ligne relocation
		{
			if( stricmp( pszCur , "xref") == 0 )
			{
				iAdr = 0 ;
				do		// recupere une liste d'adresses
				{
					pszCur = strtok( 0 , " \r\n\t,") ;
					if( ( pszCur != 0 ) && ( strlen( pszCur ) > 0 ) )
					{
						if( ( sscanf( pszCur , "%x" , &iAdr ) == EOF ) || ( iAdr  == 0 ) )
						{
							printf("config file error line %i : bad adresse format %s \n" , iLine , pszCur ) ;
							return false ;
						}
						else
						{
							CSymbol*	pReloc = new CSymbol( iAdr ) ; 
							m_arRelocSymbols.insert( pReloc ) ;			
						}
					}
					else
						iAdr = 0 ;
				}
				while( iAdr != 0 ) ;
			}
			else if( stricmp( pszCur , "xref_array") == 0 )
			{
				unsigned int iAdrStart = 0 , iAdrEnd = 0 ;
				// recupere un couple d'adresses
				pszCur = strtok( 0 , " \r\n\t,") ;
				if( pszCur == 0 )
				{
					printf("config file error line %i : missing reloc start adress \n" , iLine ) ;
					return false ;
				}
				if( ( sscanf( pszCur , "%x" , &iAdrStart ) == EOF ) || ( iAdrStart  == 0 ) )
				{
					printf("config file error line %i : bad reloc start adresse format %s \n" , iLine , pszCur ) ;
					return false ;
				}

				pszCur = strtok( 0 , " \r\n\t,") ;
				if( pszCur == 0 )
				{
					printf("config file error line %i : missing reloc end adress \n" , iLine ) ;
					return false ;
				}
				if( ( sscanf( pszCur , "%x" , &iAdrEnd ) == EOF ) || ( iAdrEnd  == 0 ) )
				{
					printf("config file error line %i : bad reloc end adresse format %s \n" , iLine , pszCur ) ;
					return false ;
				}
				for( iAdr = iAdrStart ; iAdr < iAdrEnd ; iAdr += 4 )
				{
					CSymbol*	pReloc = new CSymbol( iAdr ) ; 
					m_arRelocSymbols.insert( pReloc ) ;			
				}			
			}
			else
			{
				printf("config file error line %i : unknown %s \n" , iLine , pszCur ) ;
				return false ;
			}

		}
	}
	return true ;
}


void CExeRep::AddDirEntrySymb() 
{
	for( int idDirTable = 0 ; idDirTable < IMAGE_NUMBEROF_DIRECTORY_ENTRIES ; idDirTable ++ )
	{
		if( m_pNTOptHeader->DataDirectory[ idDirTable ].Size > 0 )
		{
			CSection* pOwner = GetSectFromVA( rva2va( m_pNTOptHeader->DataDirectory[ idDirTable ].VirtualAddress)  ) ;
			if( pOwner != 0 )
				pOwner->m_pImageDir = &(m_pNTOptHeader->DataDirectory[ idDirTable ]) ;
		}
	}
}

void CExeRep::AddDllSymb()
{
	static char_buff pszTmp ; 
// creation fichier include pour les import de dll
	FILE* pOutFile = fopen( "dllImport.inc" ,"wb") ;	
	fprintf(pOutFile,"%s%s\n" , cteHeaderSep , cteHeaderStLine ) ;
	fprintf(pOutFile,
		"%s	 PEDasm generated dll import file for file: %s.asm \n" 
		,cteHeaderStLine, m_strBaseName.c_str() );	
	fprintf( pOutFile, "%s\n%s\n%s\n\n" , cteHeaderStLine,cteHeaderStLine,cteHeaderStLine);

	IMAGE_IMPORT_DESCRIPTOR* pImpDllTable = (IMAGE_IMPORT_DESCRIPTOR*)rva2ptr(
		m_pNTOptHeader->DataDirectory[ IMAGE_DIRECTORY_ENTRY_IMPORT ].VirtualAddress ) ;
	for( ; ( pImpDllTable != 0 ) && ( pImpDllTable->Name != 0 ) ; pImpDllTable ++ )
	{	// iteration sur les dll
		DWORD	curFunVA = (DWORD)pImpDllTable->FirstThunk + m_pNTOptHeader->ImageBase ;
		DWORD*	pRVANameArray ;
		const char* pszDllName = (const char*)rva2ptr( pImpDllTable->Name ) ;
		const char* pFunName ;
	// chargement de la dll
		if( !LoadImpDll( pszDllName ) )
		{
			printf( "Unable to Load dll : %s , use listing option \n" , pszDllName ) ;
			exit(1) ;
		}
	// creation de fichier .def pour creer des import lib	
		sprintf( pszTmp ,"%s.def" , GetBaseName(pszDllName) ) ;
		FILE* pDefFile = fopen( pszTmp , "wb" ) ;
		fprintf(pDefFile,
	"LIBRARY %s\n\nDESCRIPTION 'import lib def file for use with LIB.EXE'\n\nEXPORTS\n", GetBaseName(pszDllName) ) ;

	// ajout d'une section ds le fichier include
		fprintf(pOutFile,"\n\n%s%s\n",ctePartSep , ctePartStLine ) ;
		fprintf(pOutFile,
			"%s	from dll : %s \n%s\n\n" , ctePartStLine , pszDllName , ctePartStLine );	
		
		if( pImpDllTable->Characteristics != 0 )
			pRVANameArray = (DWORD*)rva2ptr( pImpDllTable->Characteristics ) ;
		else
			pRVANameArray = (DWORD*)rva2ptr( (DWORD)pImpDllTable->FirstThunk ) ;
		do
		{	
			bool bHasName ;
			CSymbol* pDllImpSym ;
			int iOrdinal = - 1 ;
		// iteration sur les fonctions de la dll

			if( ( (*pRVANameArray ) & IMAGE_ORDINAL_FLAG ) != 0 )			
			{	// l'appli importe avec les ordinaux -> on cherche le nom dans la dll
				pFunName = GetNameFromOrd( pszDllName , (*pRVANameArray) & 0x0000ffff , &bHasName ) ;
				iOrdinal =  (*pRVANameArray) & 0x0000ffff ;
			}
			else
			{	// l'appli importe avec le nom de la fct -> on recupere ce nom
				bHasName = true ;
				pFunName = (const char*)&(((IMAGE_IMPORT_BY_NAME*)rva2ptr( *pRVANameArray ))->Name)[0] ;			
				iOrdinal = GetOrdFromName( pszDllName , pFunName ) ;
			}
			 
			if( bHasName )
			{		
				const char* pLocalFunName ;
				if( ( pLocalFunName = DllFilterNane( pFunName ) ) != pFunName )	// on doit creer un nom local
				{	
					fprintf( pDefFile , "\t%s = %s\t@%i\n", pLocalFunName , pFunName , iOrdinal );	
					pFunName = pLocalFunName ;
				}
				else
					fprintf( pDefFile , "\t%s\t@%i\n", pFunName , iOrdinal );
			}
			else // on a pas decidement pas le nom on en fabrique un arbitrairement
			{
				fprintf( pDefFile , "\t%s\t@%i\tNONAME\n" , pFunName , iOrdinal );	
			}

			pDllImpSym = new CSymbol( curFunVA , cSymDllImp , pFunName ) ;
			m_arSymbols.insert( pDllImpSym );
			curFunVA += 4 ;
			pRVANameArray ++ ;
			fprintf(pOutFile , "%s" , pDllImpSym->GetDeclaration() ) ;		
		}
		while( *pRVANameArray != 0 ) ;
		fclose( pDefFile ) ;
	}
	fclose( pOutFile ) ;
}

void CExeRep::AddRelocSymb()
{
	BYTE* pRelocTable , *pRelocCur , *pRelocEnd ;
	pRelocTable = (BYTE*)rva2ptr( 
		m_pNTOptHeader->DataDirectory[ IMAGE_DIRECTORY_ENTRY_BASERELOC ].VirtualAddress );
	pRelocCur = pRelocTable ;
	pRelocEnd = pRelocTable + m_pNTOptHeader->DataDirectory[ IMAGE_DIRECTORY_ENTRY_BASERELOC ].Size ;
	while( ( pRelocCur < pRelocEnd ) 
		&& ( *((DWORD*)pRelocCur) != 0 ) && (((DWORD*)pRelocCur)[1] != 0 ) )
	{	// iteration sur les pages de 4 Ko 
		unsigned int iVirtAdr = *((DWORD*)pRelocCur) + m_pNTOptHeader->ImageBase ;	
		int iNumReloc = ( ((DWORD*)pRelocCur)[1] - sizeof( IMAGE_BASE_RELOCATION ) ) / sizeof(WORD) ;
		pRelocCur += sizeof( IMAGE_BASE_RELOCATION );
		for( int idReloc = 0 ; idReloc < iNumReloc ; idReloc++ , pRelocCur += sizeof(WORD) )
		{	// iteration sur les reloc de la page
			if( ( (*(WORD*)pRelocCur ) >> 12 ) & IMAGE_REL_BASED_HIGHLOW != 0 )
			{
				unsigned int iRelocVA = ( (*(WORD*)pRelocCur) & 0x0fff ) + iVirtAdr ;
				CSymbol*	pReloc = new CSymbol( iRelocVA ) ; 
				m_arRelocSymbols.insert( pReloc );
			}
		}
	}
}

void CExeRep::AddExportSymb( )
{
	static char_buff pszTmp ; 
	int iMaxNamedFunOrd = 0 ;
	
	if( m_pNTOptHeader->DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ].VirtualAddress == 0 )
		return ; // rien a exporter

// creation de fichier .def pour creer des import lib	
	sprintf( pszTmp ,"%s.def" , m_strBaseName.c_str() ) ;
	FILE* pDefFile = fopen( pszTmp , "wb" ) ;
	fprintf(pDefFile,"LIBRARY %s\n\nDESCRIPTION 'exported symbols def file '\n\nEXPORTS\n"
		,  m_strBaseName.c_str() ) ;

	IMAGE_EXPORT_DIRECTORY* pExtDir = (IMAGE_EXPORT_DIRECTORY*)rva2ptr( 
		m_pNTOptHeader->DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ].VirtualAddress );

	void** parFunNames = (void**)rva2ptr(  (long)pExtDir->AddressOfNames );
	WORD*  pOrdinals = (WORD*)rva2ptr( (long)pExtDir->AddressOfNameOrdinals );
	DWORD* pAdresses = (DWORD*)rva2ptr( (long)pExtDir->AddressOfFunctions ); 
	const char* pFunName ;
	int iOrdinal ;
	DWORD dwFunAddress ;
	CSymbol* pCurSymbol ;
	for( int idFun = 0 ; idFun < pExtDir->NumberOfNames ; idFun ++ )
	{
		pFunName = (const char*)rva2ptr( (long)( parFunNames[ idFun ] ) ) ;
		iOrdinal = pExtDir->Base + pOrdinals[ idFun ] ;
		dwFunAddress = rva2va( pAdresses[ iOrdinal ] ) ;
		pCurSymbol = new CSymbol( dwFunAddress, cSymFunc | fSymPublic , pFunName );
		m_arSymbols.insert( pCurSymbol ) ;
		iMaxNamedFunOrd = max( iMaxNamedFunOrd , iOrdinal ) ;
		fprintf( pDefFile , "\t%s\t@%i\n", pFunName , iOrdinal );
	}
	for( ; idFun < pExtDir->NumberOfFunctions ; idFun ++ )
	{	// recuperation des fct exorte par ordinal -> ds l'ordre des ordinaux
		++iMaxNamedFunOrd ;
		iOrdinal = pExtDir->Base + pOrdinals[ idFun ] ;
		dwFunAddress = rva2va( pAdresses[ iOrdinal ] ) ;
		pCurSymbol = new CSymbol( dwFunAddress, cSymFunc | fSymPublic , 0 );
		m_arSymbols.insert( pCurSymbol ) ;
		iMaxNamedFunOrd = max( iMaxNamedFunOrd , iOrdinal ) ;
		fprintf( pDefFile , "\t%s\t@%i\tNONAME\n", pCurSymbol->GetName() , iOrdinal ) ;
	}		
	fclose( pDefFile ) ;
}

void CExeRep::AddEntryPoint()
{
	if( m_arSymbols.find( &CSymbol( m_pNTOptHeader->AddressOfEntryPoint + m_pNTOptHeader->ImageBase ) )
		!= m_arSymbols.end() ) 
		return ;
	const char* pszEntryPoint ;
	if( IsExeBin() )
		pszEntryPoint = "main" ;
	else
		pszEntryPoint = "dllmain" ;
	m_arSymbols.insert( new CSymbol( m_pNTOptHeader->AddressOfEntryPoint 
		+ m_pNTOptHeader->ImageBase, cSymFunc | fSymPublic , pszEntryPoint ) ) ;
}


CSection* CExeRep::GetSectFromVA( unsigned int uAdresse )
{
	CSection* pRet = 0 ;
	CSection* pSection ;
	for( int idSect = 0 ; idSect < m_arSectHeader.size() ; idSect ++ )
	{
		pSection = m_arRawSections[idSect] ;
		if( ( pSection->va_begin() <= uAdresse ) && ( pSection->va_end() > uAdresse ) ) 
		{
			pRet = pSection ;
			break ;
		}
	}
	return pRet ;
}


	//////////////////////////////////////////////////////////////
	// fonctions d'aides

bool CExeRep::IsExeBin()
{
	return ( m_pNTHeader->Characteristics & IMAGE_FILE_DLL ) == 0 ;
}

bool CExeRep::HasReloc()
{
	return ( m_pNTHeader->Characteristics & IMAGE_FILE_RELOCS_STRIPPED ) == 0  ;
}

bool CExeRep::LoadImpDll( const char* pszDllName )
{
	int iMaxNamedFunOrd = 0 ;
	LOADED_IMAGE sImage ;
	unsigned long szDir ;
	if( ! MapAndLoad( (char*)pszDllName , 0 , &sImage , TRUE , TRUE ) ) 
		return false ;
	dll_iterator iterDll = (m_arDll.insert( pair<const char* const,dllname_coll>( pszDllName , dllname_coll() ) ) ).first ;
	IMAGE_EXPORT_DIRECTORY* pExtDir = (IMAGE_EXPORT_DIRECTORY* )ImageDirectoryEntryToData( sImage.MappedAddress , FALSE 
		 , IMAGE_DIRECTORY_ENTRY_EXPORT , &szDir ) ;
	void** parFunNames = (void**)ImageRvaToVa( sImage.FileHeader, sImage.MappedAddress , (long)pExtDir->AddressOfNames , 0 );
	WORD*  pOrdinals = (WORD*)ImageRvaToVa( sImage.FileHeader, sImage.MappedAddress , (long)pExtDir->AddressOfNameOrdinals , 0 );
	dll_fun		funName ;
	for( int idFun = 0 ; idFun < pExtDir->NumberOfNames ; idFun ++ )
	{
		const char* pTmp ;
		pTmp = (const char*)ImageRvaToVa( sImage.FileHeader, sImage.MappedAddress , (long)(parFunNames[idFun]) , 0 );
		funName.m_bHasName = true ;
		funName.m_pszDllName = pszDllName ;
		funName.m_iOrdinal = pExtDir->Base + pOrdinals[idFun] ;
		funName.m_pszName = StrDup(pTmp) ;
		((*iterDll).second).push_back( funName ) ;
		iMaxNamedFunOrd = max( iMaxNamedFunOrd , funName.m_iOrdinal ) ;
	}
	for( ; idFun < pExtDir->NumberOfFunctions ; idFun ++ )
	{	// recuperation des fct exorte par ordinal -> ds l'ordre des ordinaux
		++iMaxNamedFunOrd ;
		funName.m_bHasName = false ;
		funName.m_iOrdinal = iMaxNamedFunOrd ;
		funName.m_pszDllName = pszDllName ;
		funName.m_pszName = 0 ;
		((*iterDll).second).push_back( funName ) ;
	}		
	UnMapAndLoad( &sImage ) ;
	return true ;
}

const char* CExeRep::GetNameFromOrd( const char* pszDllName ,int iOrdinal,bool* pbHasName )
{
	dll_iterator iterDll = m_arDll.find( pszDllName ) ;
	if( iterDll == m_arDll.end() )
		return 0 ;
	for( dllfun_iterator iterFctName = (*iterDll).second.begin() 
		; iterFctName != (*iterDll).second.end() ; ++ iterFctName  )
	{
		if( iOrdinal == iterFctName->m_iOrdinal )
		{
			if( iterFctName->m_bHasName )
			{
				if( pbHasName != 0 ) *pbHasName = true ;
			}
			else
			{
				if( pbHasName != 0 ) *pbHasName = false ;
			}
			return iterFctName->GetName() ;
		}
	}
	return 0 ;
}

int CExeRep::GetOrdFromName( const char* pszDllName , const char* pszFunName )
{
	dll_iterator iterDll = m_arDll.find( pszDllName ) ;
	if( iterDll == m_arDll.end() )
		return 0 ;
	for( dllfun_iterator iterFctName = (*iterDll).second.begin() 
		; iterFctName != (*iterDll).second.end() ; ++ iterFctName  )
	{
		if( iterFctName->m_bHasName && ( stricmp( pszFunName , iterFctName->m_pszName ) == 0 ) )
			return iterFctName->m_iOrdinal ;
	}
	return -1 ;
}


void CExeRep::AddSymbol( unsigned int uVA,unsigned int uRefVA, bool bTrust , unsigned int SymbolCat   )
{
	CSymbolColl::const_iterator prevSym = m_arSymbols.find( & CSymbol( uVA ) ) ;
	bool bReallySymbol = bTrust || ( m_arRelocSymbols.find( & CSymbol( uRefVA ) ) != m_arRelocSymbols.end() ) ;
	if( ( prevSym == m_arSymbols.end() ) && bReallySymbol )
	{	// nouveau symbol a referencer
		if( SymbolCat == cSymData ) // test si eventuellement callback 
		{
			CSection* pSect = GetSectFromVA( uVA ) ;
			if( ( pSect != 0 ) && pSect->IsCode() )
				SymbolCat = cSymCallBack ;
		}
		m_arSymbols.insert( new CSymbol( uVA , SymbolCat ) ) ;
	}
	if( prevSym != m_arSymbols.end() && ( ( (*prevSym)->m_Attrib & fSymUser ) == 0 ) )
	{	// on complete eventuellement
		if( (*prevSym)->m_Attrib == cSymUnknown )
			(*prevSym)->m_Attrib = SymbolCat ;
		else if( ( (*prevSym)->m_Attrib == cSymData ) && ( SymbolCat == cSymDataPtr ) )
			(*prevSym)->m_Attrib = cSymDataPtr ;
		else if( ( (*prevSym)->m_Attrib == cSymCallBack )
			&& ( ( SymbolCat == cSymJmp ) || ( SymbolCat == cSymFunc ) /*|| ( SymbolCat == cSymData )*/) )
			(*prevSym)->m_Attrib = SymbolCat ;
	}
}

const char* CExeRep::GetSymbName( unsigned int uVA, unsigned int refVA , bool bTrust , unsigned int uflags  )
{
	static char_buff pszRet ;
	const char*		pszTmp ;
	CSymbolColl::const_iterator ppSymb = m_arSymbols.end() ;

// recherche si c'est un symbol et recupere un iterateur dessus ci c'est le cas
	if( bTrust || ( m_arRelocSymbols.find( & CSymbol( refVA ) ) != m_arRelocSymbols.end() ) )
	{
		ppSymb = m_arSymbols.find( & CSymbol( uVA ) ) ;
		if( ppSymb == m_arSymbols.end() ) 
		{
			//assert(0);
			CSection* pSection = GetSectFromVA( uVA );
		}
	}

// recuperation symbol	
	if( ppSymb != m_arSymbols.end() )
		pszTmp = (*ppSymb)->GetName() ;
	else
		pszTmp = GetValue( uVA , uflags ) ;

// decoration
	if( ( uflags & cteDecorate ) != 0 )
	{
		if( ( ppSymb == m_arSymbols.end() ) && ( ( uflags & cteDecOffset ) != 0 ) )
			return pszTmp ; // val num et offset

		if( ( uflags & cteDecOffset ) == 0 )
			sprintf( pszRet , "[ %s ]" , pszTmp ) ;
		else
			sprintf( pszRet , "offset %s" , pszTmp ) ;
		return pszRet ;
	}
	else 
		return pszTmp ;
}

const char* CExeRep::GetValue( unsigned int uVal , int uflags ) 
{
	static char pszTmpBuff[32] ;
	if( ( uflags & cteFmtInteger ) != 0 )
	{
		if( ( uflags & cteFmtSign ) != 0 )	
		{
			if( ((signed int)uVal) < 0 )
				sprintf( pszTmpBuff , "- %i" , -((signed int)uVal)  ) ;	
			else
				sprintf( pszTmpBuff , "+ %i" , uVal  ) ;
		}
		else
			sprintf( pszTmpBuff , "%i" , uVal  ) ;	
	}
	else
	{
		sprintf( pszTmpBuff , "0%Xh" , uVal  ) ;	
	}

	return pszTmpBuff ;
}

CSymbol* CExeRep::GetSymbol( unsigned int uVA )
{
	CSymbolColl::iterator iterCurSymb = m_arSymbols.find( &CSymbol( uVA ) ) ;
	if( iterCurSymb == m_arSymbols.end() ) 
		return 0 ;
	else
		return *iterCurSymb ;
}

CSymbol* CExeRep::FindSymbol( unsigned int uVA , unsigned int uMask )
{
	symb_iterator iter = m_arSymbols.lower_bound( &CSymbol(uVA) ) ;
	while( ( iter != m_arSymbols.end() ) && ( ( (*iter)->m_Attrib ) & uMask ) == 0 )	
		++ iter ;
	if( iter == m_arSymbols.end()  )
		return 0 ;
	else 
		return *iter ;
}

CSymbol* CExeRep::FindSymbol( unsigned int uVA )
{
	symb_iterator iter = m_arSymbols.lower_bound( &CSymbol(uVA) ) ;
	if( iter == m_arSymbols.end()  )
		return 0 ;
	else 
		return *iter ;
}

void CExeRep::OutputMainHeader( FILE* pFile )
{
	fprintf(pFile,"%s%s\n" , cteHeaderSep , cteHeaderStLine ) ;
	fprintf(pFile, "%s	 PEDasm generated source file : %s.asm \n" 
		,cteHeaderStLine, m_strBaseName.c_str() );	
	fprintf(pFile, "%s\n%s\n" 
		,cteHeaderStLine, cteHeaderStLine );	
	fprintf(  pFile, "\n\n.586P\n.model flat , stdcall\n\n" );
	fprintf(  pFile, "\n\n.radix 10 \n\n" );
	fprintf(  pFile, "include dllImport.inc\n\n" );
}

void CExeRep::OutputSectionHeader( CSection* pSection ,FILE* pFile )
{
	fprintf(pFile,"\n\n\n%s%s section %8s\n" , cteHeaderSep 
		, cteHeaderStLine ,pSection->m_pHeader->Name) ;
	fprintf(pFile,"%s\tV.A = 0x%08x , init part size = %u , total size = %u\n" 
		, cteHeaderStLine , pSection->va_begin() , pSection->init_size() , pSection->size() ) ;	
	fprintf(pFile,"%s\tflags : ", cteHeaderStLine );
	
	if( ( pSection->GetFlags() & IMAGE_SCN_CNT_CODE ) != 0 )
		fprintf(pFile," code section , ");
	if( ( pSection->GetFlags() & IMAGE_SCN_CNT_INITIALIZED_DATA ) != 0 )
		fprintf(pFile," initialized data section , ");
	if( ( pSection->GetFlags() & IMAGE_SCN_CNT_UNINITIALIZED_DATA ) != 0 )
		fprintf(pFile," uninitialized data section , ");
	if( ( pSection->GetFlags() & IMAGE_SCN_LNK_INFO ) != 0 )
		fprintf(pFile," section contains comments ,");

	if( ( pSection->GetFlags() & IMAGE_SCN_LNK_NRELOC_OVFL ) != 0 )
		fprintf(pFile," section contains extended relocations ,");
	if( ( pSection->GetFlags() & IMAGE_SCN_MEM_DISCARDABLE ) != 0 )
		fprintf(pFile," section can be discarded , ");
	if( ( pSection->GetFlags() & IMAGE_SCN_MEM_NOT_CACHED ) != 0 )
		fprintf(pFile," section not cachable ,");
	if( ( pSection->GetFlags() & IMAGE_SCN_MEM_NOT_PAGED ) != 0 )
		fprintf(pFile," section not pageable ,");
	if( ( pSection->GetFlags() & IMAGE_SCN_MEM_SHARED ) != 0 )
		fprintf(pFile," section in shared memory ,");
	if( ( pSection->GetFlags() & IMAGE_SCN_MEM_EXECUTE ) != 0 )
		fprintf(pFile," section executable , ");
	if( ( pSection->GetFlags() & IMAGE_SCN_MEM_READ ) != 0 )
		fprintf(pFile," section readable ");
	if( ( pSection->GetFlags() & IMAGE_SCN_MEM_WRITE ) != 0 )
		fprintf(pFile," section writeable ");

	if( pSection->IsData() )
		fprintf(pFile,"\n\n.data\n\n\n" ) ;
	else if( pSection->IsCode() )
		fprintf(pFile,"\n\n.code\n\nassume fs:nothing ; this requiered for SEH\n\n" ) ;
}


//////////////////////////////////////////////////////////////////
// fonctions globales


unsigned int GetImageSize( const char* pszStr )
{
	_IMAGE_DOS_HEADER			dosHeader ;
	_IMAGE_FILE_HEADER			NTHeader ;
	IMAGE_OPTIONAL_HEADER		NTOptHeader ;
	DWORD						dwNTSign ;
	unsigned int				uiRetSize ;
	FILE* 	pFile = fopen( pszStr , "rb" ) ;
	if( pFile == 0 )
	{
		printf( "error: unable to open file : %s\n" , pszStr ) ;	
		return 0;
	}
	fread( &dosHeader , sizeof( _IMAGE_DOS_HEADER ) , 1 , pFile );
	fseek( pFile , dosHeader.e_lfanew , SEEK_SET ) ;
	fread( &dwNTSign , 4 , 1 , pFile ) ;
	
	if( dwNTSign != IMAGE_NT_SIGNATURE )
	{
		fclose(pFile) ;
		printf( "error: unrecognized binary format , file : %s\n" , pszStr ) ;
		return 0;
	}
	fread( &NTHeader , sizeof(_IMAGE_FILE_HEADER) , 1 , pFile ) ;
	if( NTHeader.Machine != IMAGE_FILE_MACHINE_I386 )
	{
		fclose(pFile) ;
		printf( "error: unknown processor , file : %s\n" , pszStr ) ;
		return 0;
	}
	if( ( ( NTHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE ) == 0 ) 
		|| ( NTHeader.SizeOfOptionalHeader == 0 ) )
	{
		fclose(pFile) ;
		printf( "error: file : %s not executable\n" , pszStr ) ;	
		return 0 ;	
	}	
	fread( &NTOptHeader , sizeof( IMAGE_OPTIONAL_HEADER ) , 1 , pFile ) ;
	uiRetSize = NTOptHeader.SizeOfImage ;
	fclose(pFile) ;
	return uiRetSize  ;
}
