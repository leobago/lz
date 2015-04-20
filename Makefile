## * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
##* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
##=======================================================================
##Copyright (C) 2014-2015 Leonardo A. BAUTISTA GOMEZ
##This program is free software; you can redistribute it and/or modify
##it under the terms of the GNU General Public License (GPL) as published
##of the License, or (at your option) any later version.
##
##This program is distributed in the hope that it will be useful,
##but WITHOUT ANY WARRANTY; without even the implied warranty of
##MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##GNU General Public License for more details.
##
##To read the license please visit http://www.gnu.org/copyleft/gpl.html
##=======================================================================

CC 		= gcc
AR		= ar
FLAGS		= -W -Wall

all: 		lib example

lib:		miniz.c lz.c
	$(CC) $(FLAGS) -c miniz.c
	$(CC) $(FLAGS) -c lz.c
	$(AR) rvs liblz.a miniz.o lz.o

example:	lib example.c
	$(CC) $(FLAGS) -o example example.c -L. -llz

test:
	./example 64

check:
	diff doubleDataset doubleDataset.umz
	diff doubleDataset doubleDataset.ulz

clean:
	rm -f *.o doubleDataset* example

.PHONY:		example test check clean



