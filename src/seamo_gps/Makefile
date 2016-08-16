#
# /* Makefile for the VHO Core packages
# *  (c) SeaMo, version 0.1, 2011, ECE Department, IISc, Bangalore &
# *  Department of IT, MCIT, Government of India
# *
# *  Copyright (c) 2009 - 2011
# *  MIP Project group, ECE Department, Indian
# *  Institute of Science, Bangalore and Department of IT, Ministry of
# *  Communications and IT, Government of India. All rights reserved.
# *      
# *  Authors: Seema K   <seema at eis.iisc.ernet.in>
# *           Anand SVR <anand at eis.iisc.ernet.in>
# *	      Chandana S Deshpande <deshpandesanvi@gmail.com>   -- 2016
# *
# *  This program is free software: you can redistribute it and/or modify
# *  it under the terms of the GNU General Public License as published by
# *  the Free Software Foundation, either version 3 of the License, or
# *  (at your option) any later version.
# *
# *  This program is distributed in the hope that it will be useful,
# *  but WITHOUT ANY WARRANTY; without even the implied warranty of
# *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# *  GNU General Public License for more details.
# *      
# *  See the file "license.terms" for information on usage and redistribution
# *  of this file.
# *
# */     
CC = gcc
LIBS = libgps.so -lm -lpthread $(shell pkg-config --libs glib-2.0 dbus-glib-1)
CFLAGS= -g -W -Wall $(shell pkg-config --cflags glib-2.0 dbus-glib-1)
COMPILE= $(CC) $(LIBS) $(CFLAGS)
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))

all:seamo_gps

seamo_gps: $(OBJS)
	$(COMPILE) -o seamo_gps $(OBJS) $(LIBS)

clean:
	rm -rf *.o seamo_gps
