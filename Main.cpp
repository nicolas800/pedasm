/*		fichier main.cpp : point d'entree du programme PEDasm
 *	
 *	descr : desassemble un fichier binaire
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

#include "ExeRep.h"


///////////////////////////////////////////////////////
/** message d'aide	
 */

static char pszPres[] =
	"PEDasm	: win32 executable file symbolic disassembler freeware ver 0.33\n"
	"\tPEDasm is a free software, you can redistribute it and/or modify\n"
    "\tit under the terms of the GNU General Public License ver 2 as published\n"
    "\tby the Free Software Foundation. It comes with ABSOLUTELY NO WARRANTY\n"
	"\tCopyright (C) 1997 - 2006 Nicolas Witczak <n_witczak@yahoo.com>\n";

static char pszHelp[] = 
	"Usage :\n"
	"\tPEDasm inputFile [switches]\n"
	"\t-o outputFile : default ouputFile.asm\n"
	"\t-l add line number : default no\n" 
	"\t-r add raw value : default no\n"
	"\t-p value : specify number of passes must be greater than 1 : default 3\n"
	"\t-s textfile : specify a file containing additionnal synchro point\n" 
	"\t\t line format : 0xAddress data | funct | ptr  [opt_name]\n" 
	"\t-b [+|-]bank_name : force or prevent use of bank_name instruction set\n"
	"\t\tdefault : -b +userX86 -b +fpuX86\n"
	"\t\talvailable banks are : user , system , fpu , mmx\n";

///////////////////////////////////////////////////////
/** point d'entree du programme
 *	
 */

int main( int argc , char* argv[] )
{
	const char* pszInput = 0 ;
	const char* pszOutput = 0 ;
	const char* pszTmp = 0 ;
	bool bLineNum = false ;
	bool bRawData = false ;
	int iPassNum = 3 ;
	printf( "%s" , pszPres ) ;
	if( argc <= 1 )
	{
		printf( "Missing file name argument \n" ) ;
		return -1 ;
	}
	for( int iArg = 1 ; iArg < argc ; ++ iArg )
	{
		if( argv[iArg][0] == '-')
		{
			bool bLoad ;
			switch( argv[iArg][1] )
			{
				case 'h':
					printf( "%s" , pszHelp );
					return 0 ;
				case 'l':
					bLineNum = true ;
					break;
				case 'r':
					bRawData = true ;
					break;
				case 'o':
					if( ++ iArg < argc )
					{
						pszOutput = argv[ iArg ] ;
						printf( "Output file : %s\n" , pszOutput ) ;
					}
					else
					{
						printf( "Option -o : Missing output name\n" ) ;
						return -1 ;
					}
					break;
				case 'p':
					if( ++ iArg < argc )
					{
						pszTmp = argv[ iArg ] ;
						iPassNum = atoi( pszTmp ) ;
						if( iPassNum >= 2 )
							break ;
					}
					printf( "Option -p : missing or bad parameter\n" ) ;
					return -1 ;
				case 's' :
					if( ++ iArg < argc )
					{
						pszTmp = argv[ iArg ] ;
						FILE* pAddConf = fopen( pszTmp  , "r" ) ;
						if( pAddConf != 0 )
						{
							bool bSuccess = GetExe()->ScanConfFile( pAddConf ) ;
							fclose( pAddConf );
							if( bSuccess )
							{
								printf( "Additionnel config file : %s\n" , pszTmp ) ;
								break;
							}
						}
					}
					printf( "Option -s : missing or bad file name\n" );
					return -1 ;
				case 'b' :					 
					pszTmp = argv[ ++iArg ]  ;					
					bLoad = true ;
					if( pszTmp[0] == '+' )
						++ pszTmp ;	
					else if( pszTmp[0] == '-' )
					{
						bLoad = false ;
						++ pszTmp ; 
					}
					if( !IsBank( pszTmp ) )
					{
						printf( "Option -b : unknown bank name : %s\n" , pszTmp );
						return -1 ;
					}
					LoadBank( pszTmp, bLoad );
					break ;
				default :
					printf( "Unknown switch -%c\n" , argv[iArg][1] ) ;
					printf( "%s" , pszHelp ) ;
					return -1 ;
			}
		}
		else // inputfile
		{
			if( pszInput == 0 )
			{
				pszInput = argv[ iArg ] ;
				printf( "Input file : %s\n" , pszInput ) ;
			}
			else
			{
				printf( "Unknown arg : %s\n" , argv[iArg] ) ;
				return -1 ;			
			}
		}			
	}
	InitBanks() ;
	GetExe()->m_iPassNum = iPassNum  ;	
	GetExe()->m_bLineNum = bLineNum  ;
	GetExe()->m_bRawData = bRawData ;
	GetExe()->ProcessExe( pszInput , pszOutput ) ;
	return 0 ;
}
