#
#  compiler - Pascal to MEPA compiler
#
#  Copyright (C) 2015  Rafael Ravedutti Lucio Machado
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

compilador: lex.yy.c y.tab.c compilador.o compilador.h
	gcc lex.yy.c compilador.tab.c compilador.o -o compilador -lfl -ly -lc -Wall

lex.yy.c: compilador.l compilador.h
	flex compilador.l

y.tab.c: compilador.y compilador.h
	bison compilador.y -d -v

compilador.o : compilador.h compiladorF.c
	gcc -c compiladorF.c -o compilador.o

clean : 
	rm -f compilador.tab.* lex.yy.c compilador compilador.o 
