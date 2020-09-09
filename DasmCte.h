/*	
 *		fichier DasmCte.h 	: fichier header
 *
 *	descr : classe automate desassembleur
 *	fichier regroupant les constantes lie au language machine du x86 
 *
 *	projet : PEDasm
 *	rq : inclus dans dasmCode.cpp uniquement 
 *
 *	rq2:
 *  
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


////////////////////////////////////////////////////////////////////
// prefixes
	const unsigned int cteLockPrefix	= 0xf0 ;
	const unsigned int cteRepNENZPrefix = 0xf2 ;	
	const unsigned int cteRepPrefix		= 0xf3 ;

	const unsigned int cteCSSRegPrefix = 0x2e ;
	const unsigned int cteSSSRegPrefix = 0x36 ;
	const unsigned int cteDSSRegPrefix = 0x3e ;
	const unsigned int cteESSRegPrefix = 0x26 ;
	const unsigned int cteFSSRegPrefix = 0x64 ;
	const unsigned int cteGSSRegPrefix = 0x65 ;

	const unsigned int cteOpSzOvrPrefix = 0x66 ;
	const unsigned int cteAdrSZOvrPrefix = 0x67 ;

////////////////////////////////////////////////////////////////////
// taille d'operande
const char*		cteOperandQualif[5] = 
{	"" , "byte ptr" , "word ptr" , "dword ptr" ,"qword ptr" } ;

////////////////////////////////////////////////////////////////////
// conditions
const char*		cteConditions[16] = 
{	"o" , "no" , "b" , "nb" , "e" , "ne" , "be" , "a" , 
	"s" , "ns" , "p" , "np" , "l" , "nl" , "le" , "g" } ;

////////////////////////////////////////////////////////////////////
// prefix de repetition
const char*		cteRep[4] =
{ "lock " , "" , "repnz ", "rep "	} ;

const char*		cteRepZ[4] =
{ "lock " , "" , "repnz ", "repz "	} ;

////////////////////////////////////////////////////////////////////
// empacketage mmx
const char*		cteMmxPack[4] = 
{ "b" , "w" , "d" , "q"	} ;

////////////////////////////////////////////////////////////////////
// registres	
// 0 -> instruction non reconnu

/** registres d'usage generaux reg = [ taille ][code reg] */
const char*		cteGenReg[4][8] = 
{
	{ 0, 0 , 0 , 0 , 0 , 0 , 0 , 0 },
	{ "al" , "cl" , "dl" , "bl" , "ah" , "ch" , "dh" , "bh" },
	{ "ax" , "cx" , "dx" , "bx" , "sp" , "bp" , "si" , "di"  },
	{ "eax" , "ecx" , "edx" , "ebx" , "esp" , "ebp" , "esi" , "edi" }
};

/** registres de segment sreg2 et sreg3 */
const char*		cteSegReg[8] = 
{	"es" , "cs" , "ss" , "ds" , "fs" , "gs" , 0 , 0 } ;

/** registres de controle */
const char*		cteCtrlReg[8] = 
{	"cr0" , 0  , "cr2" , "cr3" , "cr4" , 0 , 0 , 0 } ;

/** registres de debug */
const char*		cteDebReg[8] = 
{	"dr0" , "dr1"  , "dr2" , "dr3" , 0 , 0 , "dr6" , "dr7" } ;

/** registres de mmx */
const char*		cteMmxReg[8] = 
{	"mm0" , "mm1"  , "mm2" , "mm3" , "mm4" , "mm5" , "mm6" , "mm7" } ;

/** registres fpu */
const char*		cteFpuReg[8] = 
{	"st(0)" , "st(1)"  , "st(2)" , "st(3)" , "st(4)" , "st(5)" , "st(6)" , "st(7)" } ;


////////////////////////////////////////////////////////////////////
// definition des actions ss forme de masque
// 
// format du mask :
//		modxxxrm		-> mod rm
//		registre gen	-> grg , g08 , g16 , g32
//		empacketage mmx -> pp 
//		registre mmx	-> xmm
//		registre seg	-> 2sg 3sg
//		registre control / debug	-> _cr / _dr
//		registre fpu	-> fpu
//		registre eax,ax,al	-> eax
//		flags : s , w, d
//		conditions tttn
//		valeurs :	immediat	-> i08 , is8 , i16 , i32 , i00(taille par defaut) 
//					call   c00 (tjs 32b) 
//					jump   j08 , j16(inutilise) , j32
//					adressage direct mem a32 
//		sep octet		-> : absent apres rm ou donees
//		oos / oas		-> : override operand size  ,override adresse size doit etre present
//		nos / nas		-> : override operand size  ,override adresse size ne doit pas etre present
//		fin reconnaissance -> \0


	////////////////////////////////////////////////////////////////////
	// instructions effectivement utilise
	// initialise ds InitBanks() , jamais detruit
static SOPContext*	pfnOPCodeMask ;

	////////////////////////////////////////////////////////////////////
	// instructions d'usage general
static SOPContext	pfnOPCGeneral[] = 
{ 
	{ "0011 0111:"						, "aaa" , 0 } , 

	{ "1101 0101 : 0000 1010:"			, "aad" , 0 } , 

	{ "1101 0100 : 0000 1010:"			, "aam" , 0 } , 	

	{ "0011 1111:"						, "aas" , 0 } , 

	{ "0001 00dw : 11 grg grg:"			, "adc %1 , %0" , 0 } , 
	{ "0001 00dw : mod grg rm"			, "adc %p %1 , %0" , 0 } , 
	{ "1000 00sw : 11 010 grg : i00"	, "adc %0 , %1" , 0 } , 
	{ "0001 010w eax : i00"				, "adc %0 , %1" , 0 } , 
	{ "1000 00sw : mod 010 rm i00"		, "adc %p %0 , %1" , 0 } , 

	{ "0000 00dw : 11 grg grg:"			, "add %1 , %0" , 0 } , 
	{ "0000 00dw : mod grg rm"			, "add %p %1 , %0" , 0 } , 
	{ "1000 00sw : 11 000 grg : i00"	, "add %0 , %1" , 0 } , 
	{ "0000 010w eax: i00"				, "add %0 , %1" , 0 } , 
	{ "1000 00sw : mod 000 rm i00"		, "add %p  %0 , %1" , 0 } , 

	{ "0010 00dw : 11 grg grg:"			, "and %1 , %0" , 0 } , 
	{ "0010 00dw : mod grg rm"			, "and %p %1 , %0" , 0 } , 
	{ "1000 00sw : 11 100 grg: i00"		, "and %0 , %1" , 0 } , 
	{ "0010 010w eax: i00"				, "and %0 , %1" , 0 } , 
	{ "1000 00sw : mod 100 rm i00"		, "and %p %0 , %1" , 0 } , 
	
	{ "0110 0010 : mod grg rm"			, "bound %0 , %1" , 0 } , 

	{ "0000 1111 : 1011 1100 : 11 grg grg:" , "bsf %1 , %0" , 0 } , 
	{ "0000 1111 : 1011 1100 : mod grg rm" , "bsf %p %1 , %0" , 0 } , 

	{ "0000 1111 : 1011 1101 : 11 grg grg:" , "bsr %1 , %0" , 0 } , 
	{ "0000 1111 : 1011 1101 : mod grg rm" , "bsr %p %1 , %0" , 0 } , 

	{ "0000 1111 : 1100 1 grg:"			, "bswap %0" , 0 } , 

	{ "0000 1111 : 1011 1010 : 11 100 grg : i08"	, "bt %0 , %1" , 0 } , 
	{ "0000 1111 : 1011 1010 : mod 100 rm i08"		, "bt %p %0 , %1" , 0 } , 
	{ "0000 1111 : 1010 0011 : 11 grg grg:"			, "bt %1 , %0" , 0 } , 
	{ "0000 1111 : 1010 0011 : mod grg rm"			, "bt %p %1 , %0" , 0 } , 

	{ "0000 1111 : 1011 1010 : 11 111 grg : i08"	, "btc %0 , %1" , 0 } , 
	{ "0000 1111 : 1011 1010 : mod 111 rm   i08"	, "btc %p %0 , %1" , 0 } , 
	{ "0000 1111 : 1011 1011 : 11 grg grg:"			, "btc %1 , %0" , 0 } , 
	{ "0000 1111 : 1011 1011 : mod grg rm"			, "btc %p %1 , %0" , 0 } , 

	{ "0000 1111 : 1011 1010 : 11 110 grg : i08"	, "btr %0 , %1" , 0 } , 
	{ "0000 1111 : 1011 1010 : mod 110 rm i08"		, "btr %p %0 , %1" , 0 } , 
	{ "0000 1111 : 1011 0011 : 11 grg grg:"			, "btr %1 , %0" , 0 } , 
	{ "0000 1111 : 1011 0011 : mod grg rm"			, "btr %p %1 , %0" , 0 } , 
	
	{ "0000 1111 : 1011 1010 : 11 101 grg : i08"	, "bts %0 , %1" , 0 } , 
	{ "0000 1111 : 1011 1010 : mod 101 rm i08"		, "bts %p %0 , %1" , 0 } , 
	{ "0000 1111 : 1010 1011 : 11 grg grg:"			, "bts %1 , %0" , 0 } , 
	{ "0000 1111 : 1010 1011 : mod grg rm"			, "bts %p %1 , %0" , 0 } , 

	{ "1110 1000:c32"			, "call %0", 0 } ,		// call direct
	{ "1111 1111:11 010 grg:"	, "call %0", 0 } ,		// call indirect
	{ "1111 1111:mod 010 rm"	, "call %p %0", 0 } , 

	{ "1001 1000:" , "cbw" , 0 } , 
	{ "1001 1001:" , "cdq" , 0 } , 
	{ "1111 1000:" , "clc" , 0 } , 
	{ "1111 1100:" , "cld" , 0 } , 
	{ "1111 1010:" , "cli" , 0 } , 
	{ "1111 0101:" , "cmc" , 0 } , 
	
	{ "0000 1111: 0100 tttn : 11 grg grg:"		, "cmov%c %0 , %1" , 0 } , 
	{ "0000 1111: 0100 tttn : mod grg rm"		, "cmov%c %0 , %p %1" , 0 } , 
	
	{ "0011 10dw : 11 grg grg:"					, "cmp %1 , %0" , 0 } , 
	{ "0011 10dw : mod grg rm"					, "cmp %p %1 , %0" , 0 } , 
	{ "1000 00sw : 11 111 grg : i00 "			, "cmp %0 , %1" , 0 } , 
	{ "0011 110w eax : i00"						, "cmp %0 , %1" , 0 } , 
	{ "1000 00sw : mod 111 rm i00"				, "cmp %p %0 , %1" , 0 } , 
	
	{ "1010 0110:"								, "%rzcmpsb" , 0 } ,			
	{ "oas 1010 0111:"							, "%rzcmpsw" , 0 } , 
	{ "nas 1010 0111:"							, "%rzcmpsd" , 0 } , 
	
	{ "0000 1111 : 1011 000w : 11 grg grg:"		, "cmpxchg %1 , %0" , 0 } , 
	{ "0000 1111 : 1011 000w : mod grg rm"		, "cmpxchg %p %1 , %0" , 0 } , 

	{ "0000 1111 : 1100 0111 : a32"				, "cmpxchg8b qword ptr %0" , 0 } , 

	{ "0000 1111 : 1010 0010:"					, "cpuid" , 0 } , 
	{ "1001 1001:"								, "cwd" , 0 } , 
	{ "1001 1000:"								, "cwde" , 0 } , 
	{ "0010 0111:"								, "daa" , 0 } , 
	{ "0010 1111:"								, "das" , 0 } , 

	{ "1111 111w : 11 001 grg:"					, "dec %0" , 0 } , 
	{ "0100 1 grg:"								, "dec %0" , 0 } , 
	{ "1111 111w : mod 001 rm"					, "dec %p %0" , 0 } , 

	{ "1111 011w : 11 110 grg:"					, "div %0" , 0 } , 
	{ "1111 011w : mod 110 rm"					, "div %p %0" , 0 } , 

	{ "1100 1000 : i16 i08"						, "enter %0 , %1" , 0 } , 
	
	{ "1111 011w : 11 111 grg:"					, "idiv %0" , 0 } , 
	{ "1111 011w : mod 111 rm"					, "idiv %p %0" , 0 } , 
	
	{ "1111 011w : 11 101 grg:"					, "imul %0" , 0 } , 
	{ "1111 011w : mod 101 grg:"				, "imul %p %0" , 0 } , 
	{ "0000 1111 : 1010 1111 : 11 grg grg:"		, "imul %0 , %1" , 0 } , 
	{ "0000 1111 : 1010 1111 : mod grg rm"		, "imul %0 , %p %1" , 0 } , 
	{ "0110 10s1 : 11 grg grg: i00 "			, "imul %0 , %1 , %2" , 0 } , 
	{ "0110 10s1 : mod grg rm  i00 "			, "imul %0 , %p %1 , %2" , 0 } , 

	{ "1111 111w : 11 000 grg:"	, "inc %0" , 0 } , 
	{ "0100 0 grg:"				, "inc %0" , 0 } , 
	{ "1111 111w : mod 000 rm"	, "inc %p %0" , 0 } , 
	
	{ "1100 1101 : i08" , "int %0" , 0 } , 
	{ "1100 1100:"		, "int 3" , 0 } , 
	
	{ "1100 1110:"							, "into" , 0 } , 
	{ "1100 1111:"							, "iret" , 0 } , 

	{ "0111 tttn : j08"						, "j%c %0" , 0 } , 
	{ "0000 1111 : 1000 tttn : j32 "		, "j%c %0" , 0 } , 

	{ "oas 1110 0011 : j08"					, "jcxz %0" , 0 } , 
	{ "nas1110 0011 : j08"					, "jecxz %0" , 0 } , 

	{ "1110 1011 : j08"			, "jmp %0" , 0 } , 
	{ "1110 1001 : j32"			, "jmp %0" , 0 } , 
	{ "1111 1111 : 11 100 grg:"	, "jmp %0" , 0 } , 
	{ "1111 1111 : mod 100 rm"	, "jmp %p %0" , 0 } , 

	{ "1110 1010 : i32 i16 " , "jmp far %1 : %0" , 0 } , 
	{ "1111 1111 : mod 101 rm" , "jmp far %p %0 " , 0 } , 
	
	{ "1001 1111:" , "lahf" , 0 } , 
	
	{ "1100 0101 : mod grg rm"				, "lds %0" , 0 } , 
	{ "1000 1101 : mod grg rm"				, "lea %0 , %p %1" , 0 } , 

	{ "1100 1001:"							, "leave" , 0 } , 
	{ "1100 0100 : mod grg rm"				, "les %0 , %1" , 0 } , 
	{ "0000 1111 : 1011 0100 : mod grg rm"	, "lfs %0 , %1" , 0 } , 
 
	{ "0000 1111 : 1011 0101 : mod grg rm"	, "lgs %0 , %p %1" , 0 } , 

	{ "1010 1100:"		, "%r_lodsb" , 0 } , 
	{ "oas1010 1101:"	, "%r_lodsw" , 0 } , 
	{ "nas1010 1101:"	, "%r_lodsd" , 0 } , 

	{ "1110 0010 : j08" , "loop %0" , 0 } , 
	{ "1110 0001 : j08" , "loope %0" , 0 } , 

	{ "0000 1111 : 1011 0010 : mod grg rm" , "lss %0 , %p %1" , 0 } , 

	{ "1000 10dw:11 grg grg: "		, "mov %1 , %0" , 0 } ,	// mov reg reg
	{ "1000 10dw:mod grg rm"		, "mov %p %1 , %0" , 0 } ,	// mov reg modm
	{ "1100 011w:11 000 grg:i00"	, "mov %0 , %1" , 0 } ,	// mov reg , imm
	{ "1100 011w:mod 000 rm i00"	, "mov %p %0 , %1" , 0 } ,	// mov rm , imm
	{ "1011 w grg:i00"				, "mov %0 , %1" , 0 } ,	// mov reg , imm alternate
	
	{ "1010 00dw eax:a32"			, "mov %0 , %p %1" , 0 } ,	// mov eax , disp

	{ "1000 11d0 : 11 3sg grg :"			, "mov %1 , %0" , 0 } , 
	{ "1000 11d0 : mod 3sg rm"				, "mov %1 , %0" , 0 } , 
	
	{ "1010 0100:"		, "%r_movsb" , 0 } , 
	{ "oas 1010 0101:"	, "%r_movsw" , 0 } , 
	{ "nas 1010 0101:"	, "%r_movsd" , 0 } , 

	{ "0000 1111 : 1011 1110 : 11 g32 g08:"	, "movsx %0 , %1" , 0 } , 
	{ "0000 1111 : 1011 1111 : 11 g32 g16:"	, "movsx %0 , %1" , 0 } , 
	{ "0000 1111 : 1011 1110 : mod g32 rm"	, "movsx %0 ,byte ptr %1" , 0 } , 
	{ "0000 1111 : 1011 1111 : mod g32 rm"	, "movsx %0 ,word ptr %1" , 0 } , 

	{ "0000 1111 : 1011 0110 : 11 g32 g08:"	, "movzx %0 , %1" , 0 } , 
	{ "0000 1111 : 1011 0111 : 11 g32 g16:"	, "movzx %0 , %1" , 0 } , 
	{ "0000 1111 : 1011 0110 : mod g32 rm"	, "movzx %0 ,byte ptr %1" , 0 } , 
	{ "0000 1111 : 1011 0111 : mod g32 rm"	, "movzx %0 ,word ptr %1" , 0 } , 

	{ "1111 011w : 11 100 grg:"		, "mul %0" , 0 } , 
	{ "1111 011w : mod 100 rm"		, "mul %p %0" , 0 } , 

	{ "1111 011w : mod 011 grg:"	, "neg %0" , 0 } , 
	{ "1111 011w : mod 011 rm"		, "neg %p %0" , 0 } , 

	{ "1001 0000:"					, "nop" , 0 } , 

	{ "1111 011w : 11 010 grg:" , "not %0" , 0 } , 
	{ "1111 011w : mod 010 rm"	, "not %p %0" , 0 } , 

	{ "0000 10dw : 11 grg grg:"			, "or %1 , %0" , 0 } , 
	{ "0000 10dw : mod grg rm"			, "or %p %1 , %0" , 0 } , 
	{ "1000 00sw : 11 001 grg : i00"	, "or %0 , %1" , 0 } , 
	{ "0000 110w eax : i00 "			, "or %0 , %1" , 0 } , 
	{ "1000 00sw : mod 001 rm i00"		, "or %p %0 , %1" , 0 } , 

	{ "1000 1111 : 11 000 grg:"		, "pop %0" , 0 } , 
	{ "0101 1 grg:"					, "pop %0" , 0 } , 
	{ "1000 1111 : mod 000 rm"		, "pop %p %0" , 0 } , 

	{ "0001 1111:"					, "pop ds" , 0 } , 
	{ "0000 0111:"					, "pop es" , 0 } , 
	{ "0001 0111:"					, "pop ss" , 0 } , 
	{ "0000 1111 : 1010 0001 :"		, "pop fs" , 0 } , 
	{ "0000 1111 : 1010 1001 :"		, "pop gs" , 0 } , 
	
	{ "oos 0110 0001:"				, "popa" , 0 } , 
	{ "nos 0110 0001:"				, "popad" , 0 } , 
	{ "oos 1001 1101:"				, "popf" , 0 } , 
	{ "nos 1001 1101:"				, "popfd" , 0 } , 

	{ "1111 1111:11 110 grg:"		, "push %0" },	// push reg 
	{ "0101 0 grg:"					, "push %0" },	// push reg 	
	{ "1111 1111 : mod 110 rm"		, "push %p %0" },	// push mem
	
	{ "oos 0110 10s0 : i00"			, "pushw %0" },
	{ "nos 0110 10s0 : i00"			, "pushd %0" },

	{ "0001 1110:"					, "push cs" , 0 } , 
	{ "0001 0110:"					, "push ss" , 0 } , 
	{ "0001 1110:"					, "push ds" , 0 } , 
	{ "0000 0110:"					, "push es" , 0 } , 
	{ "0000 1111 : 1010 0000 :"		, "push fs" , 0 } , 
	{ "0000 1111 : 1010 1000 :"		, "push gs" , 0 } , 

	{ "oos 0110 0000:"				, "pusha" , 0 } , 
	{ "nos 0110 0000:"				, "pushad" , 0 } , 
	{ "oos 1001 1100:"				, "pushf" , 0 } , 
	{ "nos 1001 1100:"				, "pushfd" , 0 } , 

	{ "1101 000w : 11 010 grg:"		, "rcl %0 , 1" , 0 } , 
	{ "1101 000w : mod 010 rm"		, "rcl %p %0 , 1" , 0 } , 
	{ "1101 001w : 11 010 grg:"		, "rcl %0 , cl" , 0 } , 
	{ "1101 001w : mod 010 rm"		, "rcl %p %0 , cl" , 0 } , 
	{ "1100 000w : 11 010 grg :i08" , "rcl %0 , %1" , 0 } , 
	{ "1100 000w : mod 010 rm  i08"	, "rcl %p %0 , %1" , 0 } , 

	{ "1101 000w : 11 011 grg:"		, "rcr %0 , 1" , 0 } , 
	{ "1101 000w : mod 011 rm"		, "rcr %p %0 , 1" , 0 } , 
	{ "1101 001w : 11 011 grg:"		, "rcr %0 , cl" , 0 } , 
	{ "1101 001w : mod 011 rm"		, "rcr %p %0 , cl" , 0 } , 
	{ "1100 000w : 11 011 grg :i08" , "rcr %0 , %1" , 0 } , 
	{ "1100 000w : mod 011 rm  i08"	, "rcr %p %0 , %1" , 0 } , 

	{ "1100 0011 :"					, "ret" , 0 } , 
	{ "1100 0010 : i16"				, "ret %0" , 0 } , 

	{ "1100 1011 :"					, "ret far" , 0 } , 
	{ "1100 1010 : i16"				, "ret far %0" , 0 } , 

	{ "1101 000w : 11 000 grg:"		, "rol %0 , 1" , 0 } , 
	{ "1101 000w : mod 000 rm"		, "rol %p %0 , 1" , 0 } , 
	{ "1101 001w : 11 000 grg:"		, "rol %0 , cl" , 0 } , 
	{ "1101 001w : mod 000 rm"		, "rol %p %0 , cl" , 0 } , 
	{ "1100 000w : 11 000 grg :i08" , "rol %0 , %1" , 0 } , 
	{ "1100 000w : mod 000 rm  i08"	, "rol %p %0 , %1" , 0 } , 

	{ "1101 000w : 11 001 grg:"		, "ror %0 , 1" , 0 } , 
	{ "1101 000w : mod 001 rm"		, "ror %p %0 , 1" , 0 } , 
	{ "1101 001w : 11 001 grg:"		, "ror %0 , cl" , 0 } , 
	{ "1101 001w : mod 001 rm"		, "ror %p %0 , cl" , 0 } , 
	{ "1100 000w : 11 001 grg :i08" , "ror %0 , %1" , 0 } , 
	{ "1100 000w : mod 001 rm  i08"	, "ror %p %0 , %1" , 0 } ,

	{ "1001 1110 :"					, "sahf" , 0 } , 

	{ "1101 000w : 11 111 grg:"		, "sar %0 , 1" , 0 } , 
	{ "1101 000w : mod 111 rm"		, "sar %p %0 , 1" , 0 } , 
	{ "1101 001w : 11 111 grg:"		, "sar %0 , cl" , 0 } , 
	{ "1101 001w : mod 111 rm"		, "sar %p %0 , cl" , 0 } , 
	{ "1100 000w : 11 111 grg :i08" , "sar %0 , %1" , 0 } , 
	{ "1100 000w : mod 111 rm  i08"	, "sar %p %0 , %1" , 0 } ,
	
	{ "0001 10dw : 11 grg grg:"		, "sbb %1 , %0" , 0 } , 
	{ "0001 10dw : mod grg rm"		, "sbb %1 , %0" , 0 } , 
	{ "1000 00sw : 11 011 grg: i00"	, "sbb %0 , %1" , 0 } , 
	{ "0001 110w eax : i00"			, "sbb %0 , %1" , 0 } , 
	{ "1000 00sw : mod 011 rm i00"	, "sbb %p %0 , %1" , 0 } , 
	
	{ "1010 1110:"		, "%rzscasb" , 0 } , 
	{ "oas 1010 1111:"	, "%rzscasw" , 0 } , 
	{ "nas 1010 1111:"	, "%rzscasd" , 0 } , 

	{ "0000 1111 : 1001 tttn : 11 000 g08:"	, "set%c %0" , 0 } , 
	{ "0000 1111 : 1001 tttn : mod 000 rm"	, "set%c byte ptr %0" , 0 } ,  

	{ "1101 000w : 11 100 grg:"		, "shl %0 , 1" , 0 } , 
	{ "1101 000w : mod 100 rm"		, "shl %p %0 , 1" , 0 } , 
	{ "1101 001w : 11 100 grg:"		, "shl %0 , cl" , 0 } , 
	{ "1101 001w : mod 100 rm"		, "shl %p %0 , cl" , 0 } , 
	{ "1100 000w : 11 100 grg :i08" , "shl %0 , %1" , 0 } , 
	{ "1100 000w : mod 100 rm  i08"	, "shl %p %0 , %1" , 0 } ,

	{ "0000 1111 : 1010 0100 : 11 grg grg : i08"	, "shld %1 , %0 , %2" , 0 } , 
	{ "0000 1111 : 1010 0100 : mod grg rm i08"		, "shld %p %1 , %0 , %2" , 0 } , 
	{ "0000 1111 : 1010 0101 : 11 grg grg:"			, "shld %1 , %0 , cl" , 0 } , 
	{ "0000 1111 : 1010 0101 : mod grg rm"			, "shld %p %1 , %0 , cl" , 0 } , 

	{ "1101 000w : 11 101 grg:"		, "shr %0 , 1" , 0 } , 
	{ "1101 000w : mod 101 rm"		, "shr %p %0 , 1" , 0 } , 
	{ "1101 001w : 11 101 grg:"		, "shr %0 , cl" , 0 } , 
	{ "1101 001w : mod 101 rm"		, "shr %p %0 , cl" , 0 } , 
	{ "1100 000w : 11 101 grg :i08" , "shr %0 , %1" , 0 } , 
	{ "1100 000w : mod 101 rm  i08"	, "shr %p %0 , %1" , 0 } ,

	{ "0000 1111 : 1010 1100 : 11 grg grg : i08"	, "shrd %1 , %0 , %2" , 0 } , 
	{ "0000 1111 : 1010 1100 : mod grg rm i08"		, "shrd %p %1 , %0 , %2" , 0 } , 
	{ "0000 1111 : 1010 1101 : 11 grg grg:"			, "shrd %1 , %0 , cl" , 0 } , 
	{ "0000 1111 : 1010 1101 : mod grg rm"			, "shrd %p %1 , %0 , cl" , 0 } , 

	{ "1111 1001 :" , "stc" , 0 } , 
	{ "1111 1101 :" , "std" , 0 } , 
	{ "1111 1011 :" , "sti" , 0 } , 

	{ "1010 1010 :"		, "%r_stosb" , 0 } , 
	{ "oas1010 1011 :"	, "%r_stosw" , 0 } , 
	{ "nas1010 1011 :"	, "%r_stosd" , 0 } , 

	{ "0010 10dw : 11 grg grg:"		, "sub %1 , %0" , 0 } , 
	{ "0010 10dw : mod grg rm"		, "sub %p %1 , %0" , 0 } , 
	{ "1000 00sw : 11 101 grg: i00"	, "sub %0 , %1" , 0 } , 
	{ "0010 110w eax : i00"			, "sub %0 , %1" , 0 } , 
	{ "1000 00sw : mod 101 rm i00"	, "sub %p %0 , %1" , 0 } , 

	{ "1000 010w : 11 grg grg:"		, "test %1 , %0" , 0 } , 
	{ "1000 010w : mod grg rm"		, "test %p %1 , %0" , 0 } , 
	{ "1111 011w : 11 000 grg: i00"	, "test %p %0 , %1" , 0 } , 
	{ "1010 100w eax: i00"			, "test %0 , %1" }, 
	{ "1111 011w : mod 000 rm i00"	, "test %p %0 , %1" , 0 } , 

	{ "0000 1111 : 0000 1011 :"		, "ud2" , 0 } , 

	{ "1001 1011 :" , "wait" , 0 } , 

	{ "0000 1111 : 1100 000w : 11 grg grg:" , "xadd %1 , %0" , 0 } , 
	{ "0000 1111 : 1100 000w : mod grg rm" , "xadd %p %1 , %0" , 0 } , 

	{ "1000 011w : 11 grg grg:"		, "xchg %1 , %0" , 0 } , 
	{ "1000 011w : mod grg rm"		, "xchg %p %1 , %0" , 0 } , 
	{ "1001 0 eax grg:"				, "xchg %0 , %1 " , 0 } , 
	{ "1101 0111:"					, "xlat" },
	{ "0011 00dw : 11 grg grg:"		, "xor %1 , %0" , 0 } , 
	{ "0011 00dw : mod grg rm"		, "xor %p %1 , %0" , 0 } , 
	{ "1000 00sw : 11 110 grg: i00 ", "xor %0 , %1" , 0 } , 
	{ "0011 010w eax: i00"			, "xor %0 , %1" , 0 } , 
	{ "1000 00sw : mod 110 rm i00"	, "xor %p %0 , %1" , 0 } , 
	{ 0 , 0 , 0 } 
};

	////////////////////////////////////////////////////////////////////
	// instructions fpu
static SOPContext	pfnOPCFPU[] = 
{ 
	{ "11011 001 : 1111 0000:"				, "f2xm1" , 0 } , 
	{ "11011 001 : 1110 0001:"				, "fabs" , 0 } , 
	
	{ "11011 000 : mod 000 rm"				, "fadd dword ptr %0" , 0 } , 
	{ "11011 100 : mod 000 rm"				, "fadd qword ptr %0" , 0 } , 
	{ "11011 000 : 11 000 fpu:"				, "fadd st(0) , %0" , 0 } , 
	{ "11011 100 : 11 000 fpu:"				, "fadd %0 , st(0)" , 0 } , 
	
	{ "11011 110 : 11 000 fpu:"				, "faddp %0 , st(0)" , 0 } , 
	{ "11011 111 : mod 100 rm"				, "fbld tbyte ptr %0" , 0 } , 
	{ "11011 111 : mod 110 rm"				, "fbstp tbyte ptr %0" , 0 } , 
	{ "11011 001 : 1110 0000:"				, "fchs" , 0 } , 
	{ "11011 011 : 1110 0010:"				, "fclex" , 0 } , 

	{ "11011 010 : 11 000 fpu:"				, "fcmovb st(0) , %0" , 0 } , 
	{ "11011 010 : 11 001 fpu:"				, "fcmove st(0) , %0" , 0 } , 
	{ "11011 010 : 11 010 fpu:"				, "fcmovbe st(0) , %0" , 0 } , 
	{ "11011 010 : 11 011 fpu:"				, "fcmovu st(0) , %0" , 0 } , 
	{ "11011 011 : 11 000 fpu:"				, "fcmovnb st(0) , %0" , 0 } , 
	{ "11011 011 : 11 001 fpu:"				, "fcmovne st(0) , %0" , 0 } , 
	{ "11011 011 : 11 010 fpu:"				, "fcmovnbe st(0) , %0" , 0 } , 
	{ "11011 011 : 11 011 fpu:"				, "fcmovnu st(0) , %0" , 0 } , 

	{ "11011 000 : mod 010 rm"				, "fcom dword ptr %0" , 0 } , 
	{ "11011 100 : mod 010 rm"				, "fcom qword ptr %0" , 0 } , 
	{ "11011 000 : 11 010 fpu:"				, "fcom %0" , 0 } ,
	
	{ "11011 000 : mod 011 rm"				, "fcomp dword ptr %0" , 0 } , 
	{ "11011 100 : mod 011 rm"				, "fcomp qword ptr %0" , 0 } , 
	{ "11011 000 : 11 011 fpu:"				, "fcomp %0" , 0 } , 

	{ "11011 110 : 11 011 001:"				, "fcompp" , 0 } , 
	{ "11011 011 : 11 110 fpu:"				, "fcomi %0" , 0 } , 
	{ "11011 111 : 11 110 fpu:"				, "fcomip" , 0 } , 
	{ "11011 001 : 1111 1111:"				, "fcos" , 0 } , 
	{ "11011 001 : 1111 0110:"				, "fdecstp" , 0 } , 

	{ "11011 000 : mod 110 rm"				, "fdiv dword ptr %0" , 0 } , 
	{ "11011 100 : mod 110 rm"				, "fdiv qword ptr %0" , 0 } , 	
	{ "11011 000 : 11110 fpu:"				, "fdiv st(0) , %0" , 0 } , 
	{ "11011 100 : 11111 fpu:"				, "fdiv %0 , st(0)" , 0 } , 
	{ "11011 110 : 1111 1 fpu:"				, "fdivp %0 , st(0)" , 0 } , 

	{ "11011 000 : mod 111 rm"				, "fdivr dword ptr %0" , 0 } , 
	{ "11011 100 : mod 111 rm"				, "fdivr qword ptr %0" , 0 } , 
	{ "11011 000 : 11111 fpu:"				, "fdivr st(0) , %0" , 0 } , 
	{ "11011 100 : 11110 fpu:"				, "fdivr %0 , st(0)" , 0 } , 
	{ "11011 110 : 1111 0 fpu:"				, "fdivrp %0  , st(0)" , 0 } , 
	
	{ "11011 101 : 1100 0 fpu:"				, "ffree %0" , 0 } , 
	
	{ "11011 110 : mod 000 rm"				, "fiadd word ptr %0" , 0 } , 
	{ "11011 010 : mod 000 rm"				, "fiadd dword ptr %0" , 0 } , 

	{ "11011 110 : mod 010 rm"				, "ficom word ptr %0" , 0 } , 
	{ "11011 010 : mod 010 rm"				, "ficom dword ptr %0" , 0 } , 

	{ "11011 110 : mod 011 rm"				, "ficomp word ptr %0" , 0 } , 
	{ "11011 010 : mod 011 rm"				, "ficomp dword ptr %0" , 0 } , 

	{ "11011 110 : mod 110 rm"				, "fidiv word ptr %0" , 0 } , 
	{ "11011 010 : mod 110 rm"				, "fidiv dword ptr %0" , 0 } , 
	
	{ "11011 110 : mod 111 rm"				, "fidivr word ptr %0" , 0 } , 
	{ "11011 010 : mod 111 rm"				, "fidivr dword ptr %0" , 0 } , 

	{ "11011 111 : mod 000 rm"				, "fild word ptr %0 " , 0 } , 
	{ "11011 011 : mod 000 rm"				, "fild dword ptr %0 " , 0 } , 
	{ "11011 111 : mod 101 rm"				, "fild qword ptr %0 " , 0 } , 
	
	{ "11011 110 : mod 001 rm"				, "fimul word ptr %0" , 0 } , 
	{ "11011 010 : mod 001 rm"				, "fimul dword ptr %0" , 0 } , 

	{ "11011 001 : 1111 0111:"				, "fincstp" , 0 } , 
	{ "10011011 : 11011011 : 11100011:"		, "finit" , 0 } , 
	{ "11011011 : 11100011:"				, "fninit" , 0 } , 
	
	{ "11011 111 : mod 010 rm"				, "fist word ptr %0" , 0 } , 
	{ "11011 011 : mod 010 rm"				, "fist dword ptr %0" , 0 } , 
	
	{ "11011 111 : mod 011 rm"				, "fistp word ptr %0" , 0 } , 
	{ "11011 011 : mod 011 rm"				, "fistp dword ptr %0" , 0 } , 
	{ "11011 111 : mod 111 rm"				, "fistp qword ptr %0" , 0 } , 
	
	{ "11011 110 : mod 100 rm"				, "fisub word ptr %0" , 0 } , 
	{ "11011 010 : mod 100 rm"				, "fisub dword ptr %0" , 0 } , 

	{ "11011 110 : mod 101 rm"				, "fisubr word ptr %0" , 0 } , 
	{ "11011 010 : mod 101 rm"				, "fisubr dword ptr %0" , 0 } , 

	{ "11011 001 : mod 000 rm"				, "fld dword ptr %0" , 0 } , 
	{ "11011 101 : mod 000 rm"				, "fld qword ptr %0" , 0 } , 
	{ "11011 011 : mod 101 rm"				, "fld tbyte ptr %0" , 0 } , // a voir
	{ "11011 001 : 11 000 fpu:"				, "fld %0" },
			 
	{ "11011 001 : 1110 1000:"				, "fld1" , 0 } , 
	{ "11011 001 : mod 101 rm"				, "fldcw %0" , 0 } , 
	{ "11011 001 : mod 100 rm"				, "fldenv %0" , 0 } , 
	{ "11011 001 : 1110 1010:"				, "fldl2e" , 0 } , 
	{ "11011 001 : 1110 1001:"				, "fldl2t" , 0 } , 
	{ "11011 001 : 1110 1100:"				, "fldlg2" , 0 } , 
	{ "11011 001 : 1110 1101:"				, "fldln2" , 0 } , 
	{ "11011 001 : 1110 1011:"				, "fldpi" , 0 } , 
	{ "11011 001 : 1110 1110:"				, "fldz" , 0 } , 

	{ "11011 000 : mod 001 rm"				, "fmul dword ptr %0" , 0 } , 
	{ "11011 100 : mod 001 rm"				, "fmul qword ptr %0" , 0 } , 
	{ "11011 000 : 11 001 fpu:"				, "fmul st(0) , %0" , 0 } , 
	{ "11011 100 : 11 001 fpu:"				, "fmul %0 , st(0)" , 0 } , 
	{ "11011 110 : 11 001 fpu:"				, "fmulp %0 , st(0)" , 0 } , 

	{ "11011 001 : 1101 0000:"				, "fnop" , 0 } , 
	{ "11011 001 : 1111 0011:"				, "fpatan" , 0 } , 
	{ "11011 001 : 1111 1000:"				, "fprem" , 0 } , 
	{ "11011 001 : 1111 0101:"				, "fprem1" , 0 } , 
	{ "11011 001 : 1111 0010:"				, "fptan" , 0 } , 
	{ "11011 001 : 1111 1100:"				, "frndint" , 0 } , 
	{ "11011 101 : mod 100 rm"				, "frstor %0" , 0 } , 
	{ "11011 101 : mod 110 rm"				, "fsave %0" , 0 } , 
	{ "11011 001 : 1111 1101:"				, "fscale" , 0 } , 
	{ "11011 001 : 1111 1110:"				, "fsin" , 0 } , 
	{ "11011 001 : 1111 1011:"				, "fsincos" , 0 } , 
	{ "11011 001 : 1111 1010:"				, "fsqrt" , 0 } , 
	
	{ "11011 001 : mod 010 rm"				, "fst dword ptr %0" , 0 } , 
	{ "11011 101 : mod 010 rm"				, "fst qword ptr %0" , 0 } , 
	{ "11011 101 : 11 010 fpu:"				, "fst %0" , 0 } , 

	{ "11011 001 : mod 111 rm"				, "fstcw %0" , 0 } , 
	{ "11011 001 : mod 110 rm"				, "fstenv %0" , 0 } , 
	
	{ "11011 001 : mod 011 rm"				, "fstp dword ptr %0" , 0 } , 
	{ "11011 101 : mod 011 rm"				, "fstp qword ptr %0" , 0 } , 
	{ "11011 011 : mod 111 rm"				, "fstp tbyte ptr %0" , 0 } , // a voir
	{ "11011 101 : 11 011 fpu:"				, "fstp %0" , 0 } , 

	{ "11011 111 : 1110 0000:"				, "fstsw ax" , 0 } , 
	{ "11011 101 : mod 111 rm"				, "fstsw word ptr %0" , 0 } , 

	{ "11011 000 : mod 100 rm"				, "fsub dword ptr %0" , 0 } , 
	{ "11011 100 : mod 100 rm"				, "fsub qword ptr %0" , 0 } , 	
	{ "11011 000 : 11100 fpu:"				, "fsub st(0) , %0" , 0 } , 
	{ "11011 100 : 11101 fpu:"				, "fsub %0 , st(0)" , 0 } , 
	{ "11011 110 : 11101 fpu:"				, "fsubp %0 , st(0)" , 0 } , 

	{ "11011 000 : mod 101 rm"				, "fsubr dword ptr %0" , 0 } , 
	{ "11011 100 : mod 101 rm"				, "fsubr qword ptr %0" , 0 } , 
	{ "11011 000 : 11101 fpu:"				, "fsubr st(0) , %0" , 0 } , 
	{ "11011 100 : 11100 fpu:"				, "fsubr %0 , st(0)" , 0 } , 
	{ "11011 110 : 11100 fpu:"				, "fsubrp %0  , st(0)" , 0 } , 	
		
	{ "11011 001 : 1110 0100:"				, "ftst" , 0 } , 
	{ "11011 101 : 1110 0 fpu:"				, "fucom %0" , 0 } , 
	{ "11011 101 : 1110 1 fpu:"				, "fucomp %0" , 0 } , 
	{ "11011 010 : 1110 1001:"				, "fucompp" , 0 } , 
	{ "11011 011 : 11 101 fpu:"				, "fucomi" , 0 } , 
	{ "11011 111 : 11 101 fpu:"				, "fucomip" , 0 } , 
	{ "11011 001 : 1110 0101:"				, "fxam" , 0 } , 
	{ "11011 001 : 1100 1fpu:"				, "fxch %0" , 0 } , 
	{ "11011 001 : 1111 0100:"				, "fxtract" , 0 } , 
	{ "11011 001 : 1111 0001:"				, "fyl2x" , 0 } , 
	{ "11011 001 : 1111 1001:"				, "fyl2xp1" , 0 } , 
	{ "1001 1011:"							, "fwait" , 0 } , 
	{ 0 , 0 , 0 } 
};

	////////////////////////////////////////////////////////////////////
	// instructions privilegiees 
static SOPContext	pfnOPCPrivileged[] = 
{
	{ "0110 0011 : 11 g16 g16:"			, "arpl %1 , %0" , 0 } , 
	{ "0110 0011 : mod g16 rm"			, "arpl %1 , %0" , 0 } , 

	{ "0000 1111 : 0000 0110 :" , "clts" , 0 } , 

	{ "1111 0100:"								, "hlt" , 0 } , 

	{ "1110 010w eax : i08" , "in %0 , %1" , 0 } , 
	{ "1110 110w eax : "	, "in %0 , dx" , 0 } , 

	{ "0110 1100:"			, "%r_insb" , 0 } , 
	{ "oas 0110 1101:"		, "%r_insw" , 0 } , 
	{ "nas 0110 1101:"		, "%r_insd" , 0 } , 

	{ "0000 1111 : 0000 1000:"				, "invd" , 0 } , 
	{ "0000 1111 : 0000 0001 : mod 111 rm"	, "invlpg %p %0" , 0 } , 

	{ "0000 1111 : 0000 0010 : 11 grg grg:" , "lar %0 , %1" , 0 } , 
	{ "0000 1111 : 0000 0010 : mod grg rm" , "lar %0 ,%p  %1 " , 0 } , 

	{ "0000 1111 : 0000 0001 : mod 010 rm"	, "lgdt %0" , 0 } ,
	{ "0000 1111 : 0000 0001 : mod 011 rm"	, "lidt %0" , 0 } ,

	{ "0000 1111 : 0000 0000 : 11 010 grg:"	, "lldt %0" , 0 } , 
	{ "0000 1111 : 0000 0000 : mod 010 rm"  , "lldt %p %0" , 0 } , 

	{ "0000 1111 : 0000 0001 : 11 110 grg:"	, "lmsw %0" , 0 } , 
	{ "0000 1111 : 0000 0001 : mod 110 rm"	, "lmsw %p %0" , 0 } , 
	
	{ "1111 0000:"							, "lock" , 0 } , 

	{ "0000 1111 : 0000 0011 : 11 grg grg:" , "lsl %0 , %1" , 0 } , 
	{ "0000 1111 : 0000 0011 : mod grg rm" , "lsl %0 , %p %1" , 0 } , 

	{ "0000 1111 : 0000 0000 : 11 011 grg:" , "ltr %0" , 0 } , 
	{ "0000 1111 : 0000 0000 : mod 011 rm" , "ltr %p %0" , 0 } , 

	{ "0000 1111 : 0010 00d0 : 11 _cr grg:" , "mov %1 , %0" , 0 } ,  
	{ "0000 1111 : 0010 00d1 : 11 _dr grg:"	, "mov %1 , %0" , 0 } , 

	{ "1110 011w eax : i08" , "out %1 , %0" , 0 } , 
	{ "1110 111w eax : "	, "out dx , %0" , 0 } , 
	
	{ "0110 1110:"		, "%r_outsb" , 0 } , 
	{ "oas 0110 1111:"	, "%r_outsw" , 0 } , 
	{ "nas 0110 1111:"	, "%r_outsd" , 0 } , 

	{ "0000 1111 : 0011 0010 :"		, "rdmsr" , 0 } , 
	{ "0000 1111 : 0011 0011 :"		, "rdpmc" , 0 } , 
	{ "0000 1111 : 0011 0001 :"		, "rdtsc" , 0 } , 

	{ "0000 1111 : 0000 0001 : mod 000 rm"	, "sgdt %p %0" , 0 } ,

	{ "0000 1111 : 0000 0000 : 11 000 grg:" , "sldt %0" , 0 } , 
	{ "0000 1111 : 0000 0000 : mod 000 rm" , "sldt %0" , 0 } , 

	{ "0000 1111 : 1010 1010: "		, "rsm" , 0 } , 

	{ "0000 1111 : 0000 0000 : 11 001 grg:" , "str %0" , 0 } , 
	{ "0000 1111 : 0000 0000 : mod 001 rm" , "str %0" , 0 } , 

	{ "0000 1111 : 0000 0001 : mod 001 rm" , "sidt %0" , 0 } , 

	{ "0000 1111 : 0000 0001 : 11 100 grg:" , "smsw %0" , 0 } , 
	{ "0000 1111 : 0000 0001 : mod 100 rm" , "smsw %0" , 0 } , 
	
	{ "0000 1111 : 0000 1001 :" , "wbinvd" , 0 } , 

	{ "0000 1111 : 0000 0000 : 11 100 grg:"		, "verr %0" , 0 } , 
	{ "0000 1111 : 0000 0000 : mod 100 rm"		, "verr %p %0" , 0 } , 
	{ "0000 1111 : 0000 0000 : 11 101 grg:"		, "verw %0" , 0 } , 
	{ "0000 1111 : 0000 0000 : mod 101 rm"		, "verw %p %0" , 0 } , 

	{ "0000 1111 : 0011 0000 :" , "wrmsr" , 0 } , 

	{ 0 , 0 , 0 } 
};

	////////////////////////////////////////////////////////////////////
	// instructions mmx
static SOPContext	pfnOPCMmx[] = 
{ 
	{ "0000 1111 : 01110111: "					, "emms" , 0  } ,

	{ "0000 1111 : 011d1110: 11 xmm grg:"		, "movd %0 , %1" , 0 } , 
	{ "0000 1111 : 011d1110: mod xmm rm"		, "movd %0 , %1" , 0 } , 

	{ "0000 1111 : 011d1111: 11 xmm grg:"		, "movq %0 , %1" , 0 } , 
	{ "0000 1111 : 011d1111: mod xmm rm"		, "movq %0 , %1" , 0 } , 
	
	{ "0000 1111 : 01101011: 11 xmm xmm:"		, "packssdw %0 , %1" , 0 } , 
	{ "0000 1111 : 01101011: mod xmm rm"		, "packssdw %0 , %1" , 0 } ,

	{ "0000 1111 : 01100011: 11 xmm xmm:"		, "packsswb %0 , %1" , 0 } , 
	{ "0000 1111 : 01100011: mod xmm rm"		, "packsswb %0 , %1" , 0 } ,

	{ "0000 1111 : 01100111: 11 xmm xmm:"		, "packuswb %0 , %1" , 0 } , 
	{ "0000 1111 : 01100111: mod xmm rm"		, "packuswb %0 , %1" , 0 } ,
	
	{ "0000 1111 : 111111pp: 11 xmm xmm:"		, "padd%g %0 , %1" , 0 } , 
	{ "0000 1111 : 111111pp: mod xmm rm"		, "padd%g %0 , %1" , 0 } ,

	{ "0000 1111 : 111011pp: 11 xmm xmm:"		, "padds%g %0 , %1" , 0 } , 
	{ "0000 1111 : 111011pp: mod xmm rm"		, "padds%g %0 , %1" , 0 } ,

	{ "0000 1111 : 110111pp: 11 xmm xmm:"		, "paddus%g %0 , %1" , 0 } , 
	{ "0000 1111 : 110111pp: mod xmm rm"		, "paddus%g %0 , %1" , 0 } ,

	{ "0000 1111 : 11011011: 11 xmm xmm:"		, "pand %0 , %1" , 0 } , 
	{ "0000 1111 : 11011011: mod xmm rm"		, "pand %0 , %1" , 0 } ,

	{ "0000 1111 : 11011111: 11 xmm xmm:"		, "pandn %0 , %1" , 0 } , 
	{ "0000 1111 : 11011111: mod xmm rm"		, "pandn %0 , %1" , 0 } ,

	{ "0000 1111 : 011101pp: 11 xmm xmm:"		, "pcmpeq%g %0 , %1" , 0 } , 
	{ "0000 1111 : 011101pp: mod xmm rm"		, "pcmpeq%g %0 , %1" , 0 } ,

	{ "0000 1111 : 011001pp: 11 xmm xmm:"		, "pcmpgt%g %0 , %1" , 0 } , 
	{ "0000 1111 : 011001pp: mod xmm rm"		, "pcmpgt%g %0 , %1" , 0 } ,

	{ "0000 1111 : 11110101: 11 xmm xmm:"		, "pmadd %0 , %1" , 0 } , 
	{ "0000 1111 : 11011111: mod xmm rm"		, "pmadd, %1" , 0 } ,

	{ "0000 1111 : 11100101: 11 xmm xmm:"		, "pmulh %0 , %1" , 0 } , 
	{ "0000 1111 : 11100101: mod xmm rm"		, "pmulh %0 , %1" , 0 } ,

	{ "0000 1111 : 11010101: 11 xmm xmm:"		, "pmull %0 , %1" , 0 } , 
	{ "0000 1111 : 11010101: mod xmm rm"		, "pmull %0 , %1" , 0 } ,

	{ "0000 1111 : 11101011: 11 xmm xmm:"		, "por %0 , %1" , 0 } , 
	{ "0000 1111 : 11101011: mod xmm rm"		, "por %0 , %1" , 0 } ,

	{ "0000 1111 : 111100pp: 11 xmm xmm:"		, "psll%g %0 , %1" , 0 } , 
	{ "0000 1111 : 111100pp: mod xmm rm"		, "psll%g %0 , %1" , 0 } ,
	{ "0000 1111 : 011100pp: 11 100 xmm : i08"	, "psll%g %0 , %1" , 0 } ,

	{ "0000 1111 : 111100pp: 11 xmm xmm:"		, "psra%g %0 , %1" , 0 } , 
	{ "0000 1111 : 111100pp: mod xmm rm"		, "psra%g %0 , %1" , 0 } ,
	{ "0000 1111 : 011100pp: 11 100 xmm : i08"	, "psra%g %0 , %1" , 0 } ,

	{ "0000 1111 : 111100pp: 11 xmm xmm:"		, "psrl%g %0 , %1" , 0 } , 
	{ "0000 1111 : 111100pp: mod xmm rm"		, "psrl%g %0 , %1" , 0 } ,
	{ "0000 1111 : 011100pp: 11 100 xmm : i08"	, "psrl%g %0 , %1" , 0 } ,

	{ "0000 1111 : 111110pp: 11 xmm xmm:"		, "psub%g %0 , %1" , 0 } , 
	{ "0000 1111 : 111110pp: mod xmm rm"		, "psub%g %0 , %1" , 0 } ,

	{ "0000 1111 : 111010pp: 11 xmm xmm:"		, "psubs%g %0 , %1" , 0 } , 
	{ "0000 1111 : 111010pp: mod xmm rm"		, "psubs%g %0 , %1" , 0 } ,

	{ "0000 1111 : 110110pp: 11 xmm xmm:"		, "psubus%g %0 , %1" , 0 } , 
	{ "0000 1111 : 110110pp: mod xmm rm"		, "psubus%g %0 , %1" , 0 } ,

	{ "0000 1111 : 011010pp: 11 xmm xmm:"		, "punpckh%g %0 , %1" , 0 } , 
	{ "0000 1111 : 011010pp: mod xmm rm"		, "punpckh%g %0 , %1" , 0 } ,

	{ "0000 1111 : 011000pp: 11 xmm xmm:"		, "punpckl%g %0 , %1" , 0 } , 
	{ "0000 1111 : 011000pp: mod xmm rm"		, "punpckl%g %0 , %1" , 0 } ,

	{ "0000 1111 : 11101111: 11 xmm xmm:"		, "pxor %0 , %1" , 0 } , 
	{ "0000 1111 : 11101111: mod xmm rm"		, "pxor %0 , %1" , 0 } ,

	{ 0 , 0 , 0 } 
};

////////////////////////////////////////////////////////////////////
// definition de tout les banks
struct SBank 
{
	/** { 0 , 0 , 0 } tab de motif d'instructions */
	SOPContext*		m_pData ;
	
	/** nom du bank tq ds la ligne de command */
	const char* m_pszName ; 

	/** == true si utilise */
	bool m_bIsDefault ;

	/** op de comparaison */
	bool operator==( const SBank& other )
	{	return ( strcmp( m_pszName , other.m_pszName ) == 0 ); }
};

/** bank list ( par ordre de proba d'utilisation decroissante !)*/
static SBank  pAllBanks[] = 
{ 
	{ pfnOPCGeneral , "user" , true } , 
	{ pfnOPCFPU , "fpu" , true } ,
	{ pfnOPCMmx , "mmx" , false } ,
	{ pfnOPCPrivileged , "system" , false } 
};


////////////////////////////////////////////////////////////////////
// divers flags

////////////////////////////////////////////////////////////////////
// decoupage typique d'un octet en 3 parties (2,3,3)
inline void ByteSplit3( BYTE in , BYTE& outHigh , BYTE& outMed , BYTE& outLow )
{
	outLow = in & BIN_CTE(0,0,0,0,0,1,1,1) ;
	outMed = ( in & BIN_CTE(0,0,1,1,1,0,0,0) ) >> 3 ;
	outHigh = ( in & BIN_CTE(1,1,0,0,0,0,0,0) ) >> 6 ;
}
