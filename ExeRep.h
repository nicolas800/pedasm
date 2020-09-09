/*		fichier exeRep.h : fichier header
 *
 *	descr : classe qui represente un executable binaire et gere la memoire associee
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

#ifndef EXE_REP_H
	#define EXE_REP_H

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <set>
#include <map>
#include <string>
#undef va_end
#include "Symbol.h"



class CExeRep ;


///////////////////////////////////////////////////////////////
// struct dll_fun represente 1 fct d'une dll 
//: assocition nom , ordinal , existence du nom
struct dll_fun 
{
	const char*	m_pszName ;
	const char*	m_pszDllName ;
	int 	m_iOrdinal ;
	bool	m_bHasName ;

// fct GetName :retourne le vrai nom ou un nom decore
	const char* GetName()
	{
		static char_buff	m_psztmp ;
		if( m_bHasName )
			return m_pszName ;
		else
			sprintf( m_psztmp , "%s_ord%i"
				, GetBaseName(m_pszDllName) , m_iOrdinal ); 
		return m_psztmp ;
	};
};


///////////////////////////////////////////////////////////////
/** class CSection : represente une section
 */
class CSection
{
public:
// init
	/** ctor : attache une section a sa zone memoire
	 * param : idSection : index de section 
	 */
	CSection( int idSection ) ;

// attributs
	/** index de la section */
	int						m_idSection ;

	/** ptr sur notre header , appartient a la classe CExeRep */
	IMAGE_SECTION_HEADER*	m_pHeader ;

	/** donnes de la partie initialisee de cette section , appartient a la classe CExeRep */
	BYTE*					m_pRawData ;

	/** image directory si il y a lieu */
	IMAGE_DATA_DIRECTORY*	m_pImageDir ;

// information
	/** size , init_size : retourne taille  */
	unsigned int size()	;	
	unsigned int init_size();
	
	/** begin end : retourne ptr memoire relle de la section ,fin ,fin initialisee */
	BYTE* begin()	;	
	BYTE* end();		
	BYTE* init_end();	

	/** va_begin va_end va_initend : retourne debut fin, fin initialisee en adr virt de la section */
	unsigned int va_begin()	;
	unsigned int va_end();	
	unsigned int va_init_end();

	/** retourne les flags de section*/
	DWORD GetFlags();

	/** IsCode : indique si c'est une section code */
	bool IsCode(); 
	
	/** IsCode : indique si c'est une section de donnee ordinaire */	
	bool IsData();
};

//////////////////////////////////////////////////////////////////
/** class CExeRep
 *	descr : representation d'un fichier binaire 
 *	en cours de desassemblage : classe singleton
 */
class CExeRep
{
public:
// donnees fichiers
	/** path de l'exe */
	string							m_strPath ;

	/** nom sans extension */
	string							m_strBaseName ;

	/** fichier de sortie du .asm */
	string							m_strOutput ;

	/** header dos*/
	_IMAGE_DOS_HEADER*				m_pDosHeader ;

// gestion du bloc de memoire brut
	/** bloc de memoire contenant tt les sections du prg */
	BYTE*							m_pRawData ;

	/** offset V.A -> ptr memoire */
	unsigned int					m_vaOffset ;

// donnees image PE	decortiquees
	/** signature */
	DWORD							m_dwNTSign ;
	
	/** header NT */
	_IMAGE_FILE_HEADER*				m_pNTHeader ;
	IMAGE_OPTIONAL_HEADER*			m_pNTOptHeader ;
	
	/** headers des sections */
	vector< IMAGE_SECTION_HEADER* >	m_arSectHeader ;
	
	/** liste des sections  */
	vector< CSection* >				m_arRawSections ;
	
	/** type liste de symbols */
	typedef	set< CSymbol* , symb_less >	CSymbolColl ;
	typedef CSymbolColl::iterator		symb_iterator ;

	/**liste des symbols  */
	CSymbolColl						m_arSymbols ;

	/**liste des symbols inutilises */
	CSymbolColl						m_arUnusedSymbols ;

	/** table de relocation 
	 *	si l'adresse est ds du code -> rep symbol donnee
	 *	si l'adresse est ds des donnees -> rep symbol type pointeur
	 */
	CSymbolColl						 m_arRelocSymbols;

	/** liste des dll importees
	 *	nom dll -> ( ordinal -> nom fonction )
	 */
	typedef vector< dll_fun >									dllname_coll ;
	typedef dllname_coll::iterator								dllfun_iterator ;
	typedef map< const char* , dllname_coll , psz_less_nocase >	dll_coll ;
	typedef dll_coll::iterator									dll_iterator ;

	dll_coll		m_arDll ;

public:
// parametrage
	/** m_bRawData : var de config : rajoute les donnees dans le source desassemble */
	bool			m_bRawData;

	/** m_bLineNum : ajoute les numero de ligne dans le source desassemble */
	bool			m_bLineNum  ;

	/** m_iPassNum : specifie le nombre de passes a effectuer sur le binaire */
	int				m_iPassNum ;

// fonctions d'aide

// initialisation , destruction
	CExeRep();	
	~CExeRep();
	
	/** genere les noms de fichiers */
	void SetFileName( const char* pszFileIn , const char* pszFileOut );

// etapes de traitement
	/** fct ProcessExe : point d'entree: traite l'executable et produit les sources 
	 *		si pszFileOut : nom de sortie par defaut
	 */
	void ProcessExe( const char* pszFileIn , const char* pszFileOut = 0 ) ;

	/**fct LoadBin : charge un binaire et remplit les donnees brutes 
	 *	retourne : etat de success
	 */
	bool LoadBin( const char* pszName ) ;
	
	/** fct ScanConfFile : analyse le fichier passe en entree et rajoute les
	 *	symbols correspondants dans la table des symbols
	 *	format du fichier de config
	 *		0xadresse  [ data | funct | ptr ] opt nom
	 */
	bool ScanConfFile( FILE* pFile ) ;

	/** fct AddDirEntrySymb :marque comme symbols non utiles les zones 
	 *		de directory entry inclu dans les differentes sections
	 */
	void AddDirEntrySymb() ;

	/** AddDllSymb : importe les symboles fct dll d'import
	 */
	void AddDllSymb();

	/** AddExportSymb : ajoute les symboles exportes ds le binaire 
	 *	sert principalement pour les dll
	 */
	void AddExportSymb();

	/** AddRelocSymb : importe les symboles trouves dans la table de relocation
	 *		genere un fichier include des references externes
	 */
	void AddRelocSymb();


	/** AddEntryPoint : ajoute le point d'entree du binaire	
	 */
	void AddEntryPoint();

// fonctions d'aides
	
	/** fct IsExeBin : retourne true si c'est un exe 0 si c'est une dll */
	bool IsExeBin();

	/** fct HasReloc : retourne true si le binaire a une table de relocation */
	bool HasReloc();

	/** GetSectFromVA : retourne la section qui contient l'adresse abs 
	 *		passee en parametre ou 0 si non trouve
	 */
	CSection* GetSectFromVA( unsigned int uAdresse );
	
	/** fct de conversion ptr <-> va <-> rva
	 */
	BYTE* CExeRep::rva2ptr( unsigned int uRVA )	
	{	return  m_pRawData + uRVA ; }

	BYTE* CExeRep::va2ptr( unsigned int uVA )
	{	return reinterpret_cast< BYTE* >( uVA + m_vaOffset ) ; }

	unsigned int CExeRep::ptr2va( BYTE* p ) 
	{	return reinterpret_cast< unsigned int >(p - m_vaOffset); }

	unsigned int CExeRep::ptr2rva( BYTE* p ) 
	{	return ( unsigned int )(p - m_pRawData ) ; }

	unsigned int CExeRep::rva2va( unsigned int uRVA ) 
	{	return uRVA + m_pNTOptHeader->ImageBase ; }

// gestion des dll d'import

	/** fct LoadimpDll : charge les symbol (nom ordinal) d'une dll
	 */
	bool LoadImpDll(const char* pszName );

	/** fct GetNameFromOrd : retourne le nom d'une dll a partir d'un ordinal 
	 *	PARAM : pszDllName : nom de la dll
	 *			int		   : ordinal  
	 *			sortie pbNoName : false -> indique qu'un nom n'a pas ete trouve
	 *	RETOUR : une chaine temporaire sur le nom de dll ou un nom genere si le nom
	 *	n'est pas exporte , retourne 0 en cas d'echec
	 */
	const char* GetNameFromOrd( const char* pszDllName ,int iOrdinal,bool* pbHasName );
	
	/**	fct GetOrdFromName : retourne l'ordinal a partir du nom de fct ds une dll
	 *	ou -1 en cas d'echec
	 */
	int GetOrdFromName( const char* pszDllName , const char* pszFunName );

// interface
	/** GetSymbol : retourn un pointeur sur 1 symbol ou 0 si aucuns symbols 
	 *	param uVA : adresse absolue 
	 */
	CSymbol* GetSymbol( unsigned int uVA );

	/** FindSymbol : cherche le prochain symbol satisfaisant au mask
	 *		donne par ordre d'adresse croissant a partir de l'adresse uVA incluse
	 * retourne 0 si non trouve
	 *	rq : DEPRECIE
	 */
	CSymbol* FindSymbol( unsigned int uVA , unsigned int uMask );

	/** FindSymbol : cherche le 1er symbol disponible
	 *		donne par ordre d'adresse croissant a partir de l'adresse uVA incluse
	 * retourne 0 si non trouve
	 */
	CSymbol* FindSymbol( unsigned int uVA );

	/** AddSymbol : appele par le desassembleur pour rajouter ou preciser 
	 *	un symbol 
	 *	param uVA : adresse de location du symbol
	 *		pRefPoint : adresse mem ou a ete reference le symbol	 
	 *		bTrust : niveau de confiance (ne requiert pas la table de reloc )
	 *		SymbolCat : type reconnu
	 */
	void AddSymbol( unsigned int uVA,unsigned int uRefVA, bool bTrust , unsigned int SymbolCat   );

	/** GetSymbName : retourne le nom d'un symbol a partir de 
	 * l'adresse virtuelle uVA ou la valeur si non trouve
	 *	PARAM : uVA	  : valeur a traduire
	 *			refVA : adresse ou a ete trouve ce symbol
	 *			bTrust : niveau de confiance (ne requiert pas la table de reloc )
	 *			uflags   : format d'affichage
	 */
	const char* GetSymbName( unsigned int uVA , unsigned int refVA , bool bTrust , unsigned int uflags ) ;

	/** GetValue : retourne la valeur chaine d'une cte
	 *		param uVal : valeur a traiter 		
	 *		iflags : spec de formatage
	 */
	const char* GetValue( unsigned int uVal , int uflags ) ;	

//	fonction de sorties fichier
	/** genere le commentaire header du fichier asm principale */
	void OutputMainHeader( FILE* pFile );

	/** genere le commentaire du header de section pSection
	 *	sortie sur le stream pFile
	 */
	void OutputSectionHeader( CSection* pSection , FILE* pFile );

};

/** format d'affichage de valeurs */
const unsigned int cteFmtInteger	= 1 ; // format decimal, hexa sinon
const unsigned int cteFmtSign		= 2 ; // format nbr signe avec sign
const unsigned int cteDecorate		= 4 ; // decore avec ( [] ou offset si besoin ) , pas de decoration sinon
const unsigned int cteDecOffset		= 8 ; // decore avec offset si ==1 , [] sinon

/** format de sortie de desassemblage */
const int cteMASM		= 0 ;
const int cteListing	= 1 ;
const int cteInlineC	= 2 ;

//////////////////////////////////////////////////////////////////
// fonctions globales

/** acces au singleton exe */
extern CExeRep	m_theExe ;
inline CExeRep* GetExe()
{	return &m_theExe ;	}

/** GetImageSize : retourne la taille d'un binaire en mem vive */
unsigned int GetImageSize( const char* pszStr );

/** indique si un ensemble d'instructions est disponible*/
bool IsBank( const char* pszBankName ) ;
	
/** si bLoad == true : marque comme candidat le groupe de modele d'instructions , 
 * l'enleve sinon 
 */
void LoadBank( const char* pszBankName , bool bLoad ) ;

/** Genere la liste des modeles d'instruction utilisees durant la decompilation */
void InitBanks() ;


#endif //EXE_REP_H