/*		fichier config.h : fichier header
 *
 *	descr : configuration des parametres relatifs au compilateur
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
 *
 *	rq2 : a inclure dans chaque unite de traduction avant tt le reste
 */


///////////////////////////////////////////////////////////////////////////
/*	les flags des compilateurs consideres sont:
 *	(ils sont defini automatiquement)
 *		__GNUC__ , _MSC_VER
 */

///////////////////////////////////////////////////////////////////////////
/* flag du systeme d'exploitation ou s'execute PEDasm
 *	pour le moment	: MSDOS , WIN32 , LINUX 
 */
//#define WIN32

///////////////////////////////////////////////////////////////////////////
/*	le flag suivant indique si l'on souhaite utiliser les headers MS
 *
 *		USE_MSHEADER
 */
#ifdef WIN32
	#define USE_MSHEADER
#endif //WIN32

///////////////////////////////////////////////////////////////////////////
/*	separateur de fichier (un grand classic)
 *
 */
#ifdef WIN32
	#define PATH_SEP '\\'
	#define PATH_STRING "\\"
#else
	#define PATH_SEP '/'
	#define PATH_STRING "/"
#endif

//////////////////////////////////////////////////////////////////////////
// suppression des warnings penibles

#ifdef _MSC_VER
	#pragma warning( disable : 4786 ) 
#endif //_MSC_VER

//////////////////////////////////////////////////////////////////////////
// support des namespace de la stl
#ifndef __GNUC__
	namespace std {} ; // definition 'en avant' du namespace lib c++ std
	using namespace std ;
#endif	//__GNUC__


//////////////////////////////////////////////////////////////////////////
// definition des types et structures Wnd
#ifdef USE_MSHEADER
	#include <windows.h>
	#include <winnt.h>
	#include <imagehlp.h>
#endif //USE_MSHEADER
