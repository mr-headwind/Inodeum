
#  Copyright (C) 2017 Anthony Buckley
# 
#  This file is part of Inodeum.
# 
#  Inodeum is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
# 
#  Inodeum is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with Inodeum.  If not, see <http://www.gnu.org/licenses/>.

CC=cc
CFLAGS=-I. `pkg-config --cflags gtk+-3.0 libsecret-1` 
CFLAGS2=-Wno-deprecated-declarations
DEPS = defs.h main.h isp.h cairo_chart.h version.h
OBJ = um_main.o callbacks.o main_ui.o utility.o service.o ssl_socket.o socket.o overview.o history.o about.o monitor.o prefs.o version.o user_login_ui.o date_util.o css.o view_file_ui.o cairo_chart.o cairo_util.o calendar_ui.o
LIBS = `pkg-config --libs gtk+-3.0 libsecret-1 cairo`
LIBS2 = -lssl -lcrypto -lpthread -lm -lpcap
#LIBS2 = -lpthread

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(CFLAGS2)

inodeum: $(OBJ)
	$(CC) -o $@ $^ $(LIBS) $(LIBS2)

clean:
	rm -f $(OBJ)
