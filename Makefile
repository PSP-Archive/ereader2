# $Id: Makefile 78 2007-11-06 16:39:35Z soarchin $

# Copyright 2007 aeolusc

# This file is part of eReader2.

# eReader2 is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# eReader2 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

all:
	./Revision.sh
	make -f Makefile.PSP

clean:
	make -f Makefile.PSP clean

200+:
	./Revision.sh
	PSP_FW_VERSION=200 BUILD_PRX=1 make -f Makefile.PSP

200+clean:
	PSP_FW_VERSION=200 BUILD_PRX=1 make -f Makefile.PSP clean

371:
	make -C Prx
	mv ./Prx/eReader2_p.prx .
	./Revision.sh
	PSP_FW_VERSION=371 BUILD_PRX=1 OBJS=Prx/eReader2_p.o make -f Makefile.PSP 

371clean:
	make -C Prx clean
	rm -f eReader2_p.prx ./Prx/eReader2_p.S
	PSP_FW_VERSION=371 BUILD_PRX=1 OBJS=Prx/eReader2_p.o make -f Makefile.PSP clean
