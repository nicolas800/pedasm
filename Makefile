#		fichier makefile
#	
#	projet : PEDasm
#	
#	rq:
#	Ce programme est libre de droits. Il peut etre distribue et/ou modifie
#   selon les termes de la licence 'GNU General Public License version 2'.
# 	
# 	Ce programme est distribue sans aucunes garanties, y compris d'utilite 
# 	ni de risques encouru, quelle que soit son utilisation.
# 
# 	lire le fichier licence.txt fourni ou bien ecrire a :
# 	the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
# 	pour recevoir une copie de la licence.
#
#	Copyright (C) 1997 - 1998 Nicolas Witczak <witczak@geocities.com>
#



CC = cc.exes

LINK = link.exe

SRCS = DasmBase.cpp DasmCode.cpp DasmData.cpp ExeRep.cpp Globals.cpp Main.cpp Symbol.cpp

ALL : "PEDasm.exe"


CLEAN :
	-@erase *.obj
	-@erase PEDasm.exe

CPP=cc.exe

CPP_FLAGS= -DWIN32 -DNDEBUG -D_CONSOLE -D_MBCS -o3

.cpp .obj:
	$(CPP) $(CPP_FLAGS) -c $<


LIBS=kernel32.lib user32.lib advapi32.lib shell32.lib imagehlp.lib 
OBJS= DasmBase.obj DasmCode.obj DasmData.obj ExeRep.obj Globals.obj Main.obj Symbol.obj

LINK_FLAGS=

PEDasm.exe :  $(OBJS)
    $(LINK) $(LINK_FLAGS) $(OBJS) $(LIBS) $<

