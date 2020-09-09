---------------------------------*-text-*---------------------------------

    PEDasm -- win32 executable file disassember ver 0.33 november 2006

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Copyright (C) 1997 - 1999 Nicolas Witczak <witczak@geocities.com>
	Home page : http://www.geocities.com/SiliconValley/Lab/6307

--------------------------------------------------------------------------

remarks concerning the source code
==================================

Don't mail me in order to ask the following kind of questions :

- how do I recompile the sources ?

- I can't compile, my xxxcc generate xxx errors  ?

- Where can I find the headers xxx ? Didn't you forget .h extensions ?

- I recompiled the sources and the generated bin hangs ?

...

overview
========


PEDasm is a Win32 command line tool wich disassemble binary files targeted 
for win32 X86 plateforms. It produces on output an asm source file that can 
in several case be successefully recompiled with an masm compatible assembler . 
This executable may be downloaded and used freeely. 


list of files from this archive
===============================


* CPP source files 

	+ DasmBase.cpp .h , DasmCode.cpp .h , DasmData.cpp .h 
		core disassembly code

	+ symbol.cpp .h :
		symbols type ( var function ... ) categorization

	+ DasmCte.h : x86 machine code and assembly related constants

	+ ExeRep.cpp .h :
		ms PE file exe and dll loader specific code .

	+ globals.cpp globals.h	:
	   generic helpers 

	+ config.h :
		plateform and compiler specific stuff

	+ main.cpp :
		entry point, cmd line analysis 	   

* build file

	+ Makefile : generic standard make file 

* binary file :

	+ PEDasm.exe : win32 executable

* text file 
	+  readme.txt 	
	+  licence.txt


Command line options and output format
======================================

Given an executable inputFile.exe it produces the following files
	- inputFile.asm : disassembled file 
	- (opt) inputFile.def : if inputFile.exe export section is not empty 
	this file gathers exported symbols
	- various xxx.def files : one for each imported dll used by inputFile.exe 
	 
Command line options are :
--------------------------

	-h : echos help message
	-o name : override default asm output with name
	-r : feed each assembly line with raw hexa content within a comment
	-l : add to each assembly line its virtual address
	-p integerVal : override number of pass ( default 3 )
		increasing this number may help distinguish code from data when both are deeply mixed
	-b [+|-]bank_name : use instruction set bank_name . Not using an instruction bank prevent from 
recognizing some kind of instructions. Accessible banks are : user , system , fpu , mmx 
Usage -b [+]bank_name : force use of bank_name instruction set
Usage -b -bank_name : prevent use of bank_name instruction set
	
	default -b +userX86 +fpuX86
rq : using minimal banks can help detect garbage code !

	-s : specify additional synchro point config file file must have the following form  :

config file definition
----------------------

The optionnal config file given as -s option can improve readability of the generated 
assembly source file. It can also resolve ambiguity found in code lacking relocation table
The format is the following :

config_file_line : comment_line | symbol_line | reloc_line

comment_line  : '[#' comment_string ']' | '\n'

comment_string : string_cte


symbol_line : hexa_number  type_symbol 

symbol_line : hexa_number  type_symbol name_symbol 

symbol_line : hexa_number  type_symbol name_symbol inline_comment

name_symbol : string_token

inline_comment : string_cte

type_symbol : 'data' | 'funct' | 'label' | 'ptr' 

string_cte : a string within a single line

string_token : a string without separator

hexa_number : a number with the form : 0xNNNNNNNN


reloc_line : 'xref' liste_adr_reloc

liste_adr_reloc : hexa_number

liste_adr_reloc : liste_adr_reloc | hexa_number

reloc_line : 'xref_array' adr_reloc_start adr_reloc_end

adr_reloc_start : hexa_number

adr_reloc_end : hexa_number


rq : the file is processed line by line , each line has 4096 max size

ex: 

# override default PEDasm beheavior to consider 0x0040a478	address as data
# and name it progConstant 

0x0040a478	Data	progConstant	this is a comment added in asm output

# manually add to reloc an array of pointers (ex : vtable)
xref_array	0x004050cc	0x004050d8 

# manually add a couple of reloc
xref 0x004050c0		0x004050c4	0x004050c8  


new in ver 0.33
===============

- bug update : weak code generating register text ( thanks to David )
- bug update : incorrectly adressing exported function rva in dll ( thanks to Jacky )

new in ver 0.32
===============

- improved config file , added manual reloc symbol features 
- removed small bugs in inst output 
- handled special c++ decorated name from c++ dll import

new in ver 0.31
===============

- removed some oddity in code
- moved plateform and compiler specific code to a single file "config.h"

NOTE : at this time  Giuseppe Guerrini is working on a port of PEDasm for Linux 
	here is his mail adresse : <giusguerrini@racine.ra.it>

new in ver 0.3
==============

- added assembly instruction banks choice
- now distributed as a GNU software

new in ver 0.2
==============

- improved speed
- bug correction : better output and robustness
- explicit support for dll input
- added command line options
- user overrides file for code data mixing interpretation

example of use
===============

One must have an masm compatible assembler and a resource extractor
Provided environment variables and path are set following sequence 
disassembles and reproduces calc.exe :


	pedasm calc.exe	-s calcConf.txt
	lib -machine:I386 /out:kernel32.lib  /def:kernel32.def 
	lib -machine:I386 /out:user32.lib  /def:user32.def 
	lib -machine:I386 /out:shell32.lib  /def:shell32.def 
	lib -machine:I386 /out:gdi32.lib  /def:gdi32.def 
	ml /c  /coff calc.asm
	link -entry:main -out:calcdasm.exe calc.res  -subsystem:windows -machine:I386 \
	calc.obj kernel32.lib user32.lib shell32.lib gdi32.lib 


remarks
=======


-  asm generated source contains line that produces error at compilation

some examples encountered while testing :

	call 0x00405c21 	
		:  pedasm missed that address -> try increase -p switch

	mov al , byte ptr Unknown3800dc 
		: pedasm marked this symbol as Unknown because it was outside the legal adress space
		of the exe while present in .reloc section
			
	call Data4174a0	
		: address 0x004174a0 was wrongly interpreted as data
		-> try config file with "0x004174a0 Funct" line

	add  byte ptr gs:[eax] , al
		: obviously garbage code, only selector pointed by fs may be used
		for structured exception handling and thread structures
		-> try a conf file line with a "data" keyword

- text segment begin with many line like the following :
	
	extern	_imp__RegQueryValueExA : dword
	RegQueryValueExA equ _imp__RegQueryValueExA

	dword 0BFEC15C4h

		: import dll data are usually placed in idata section but can be placed anywhere



