/*		fichier splitAsm.cpp : point d'entree du programme SplitAsm
 *	
 *	descr : decompose un fichier file.asm
 *	en plusieurs fichiers file1.asm , file2.asm ....
 *
 *	projet : SplitAsm 
 *	
 *	rq1: program utile pour contourner la limitation de 32767 ligne 
 *	lorque que l'on utilise l'option Zd de masm
 *
 *	rq2:
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


#include <string>
#include <cstdlib>
#include <cstdio>
using namespace std;


#include "Globals.h"

///////////////////////////////////////////////////////
/** message d'aide	
 */

static char pszPres[] =
	"SplitAsm : freeware helper program for pedasm\n"
	"\tSplitAsm is a free software, you can redistribute it and/or modify\n"
    "\tit under the terms of the GNU General Public License ver 2 as published\n"
    "\tby the Free Software Foundation. It comes with ABSOLUTELY NO WARRANTY\n"
	"\tCopyright (C) 1997 - 1998 Nicolas Witczak <witczak@geocities.com>\n";

static char pszHelp[] = 
	"Usage :\n"
	"\tSplitAsm inputFile [switches]\n"
	"\t-l specify (approximatly) number of line per output files: default 24000\n"  ;

///////////////////////////////////////////////////////
/**		effectue le decoupage du fichier 
 *	PARAM : pInFile : fichier en entree
 *			pszFileName : nom sans extension en entree
 *			iLine : nbr de ligne par fichier de sortie
 */
void ProcessFile( FILE* pInFile , const char* pszFileName , int iLine )
{
	char pszTmpBuff[4096] ;
	char_buff strFileOut ;
	FILE* pszOutFile = 0 ;
	int iFileID = 1 ;
	for( int iLineCur = 0 ; fgets( pszTmpBuff , sizeof(pszTmpBuff) - 1 , pInFile ) != 0 ; ++ iLineCur )
	{
		if( iLineCur > iLine )	// essaye de spliter
		{
			if( ( strstr( pszTmpBuff , "Fun" ) == pszTmpBuff ) 
				|| ( strstr( pszTmpBuff , "Data" ) == pszTmpBuff ) 
				|| ( strstr( pszTmpBuff , ";---------" ) == pszTmpBuff ) 
				|| ( strstr( pszTmpBuff , ";*********" ) == pszTmpBuff ) )
			iLineCur = 0;	
	
		}
		if( iLineCur == 0 )
		{
			if( pszOutFile != 0 )
				fclose( pszOutFile );
			sprintf( strFileOut , "%s%02i.asm" , pszFileName , iFileID ++ ) ;
			pszOutFile = fopen( strFileOut , "w" ) ;
		}
		fputs( pszTmpBuff , pszOutFile );
	}
}

///////////////////////////////////////////////////////
/** point d'entree du programme
 *	
 */

int main( int argc , char* argv[] )
{
	const char* pszInput = 0 ;
	const char* pszTmp = 0 ;
	int iLinePerFile = 24000 ;
	FILE* pszInFile = 0 ;
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
			switch( argv[iArg][1] )
			{
				case 'h':
					printf( "%s" , pszHelp );
					return 0 ;
				case 'l':
				if( ++ iArg < argc )
				{
					pszTmp = argv[ iArg ] ;
					iLinePerFile = atoi( pszTmp ) ;
					if( iLinePerFile > 0 )
						break ;
				}
				printf( "Option -l : missing or bad parameter\n" ) ;
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
	pszInFile = fopen( pszInput , "r") ;
	if( pszInFile == 0  )
	{	
		printf( "Cannot open input file : %s\n" , pszInput ) ;
		return -1 ; 
	}
	char* pszFileExt = strchr( pszInput , '.' ) ;
	if( pszFileExt != 0 )
		*pszFileExt = '\0' ;
	ProcessFile( pszInFile , pszInput , iLinePerFile ) ;
	return 0 ;
}

