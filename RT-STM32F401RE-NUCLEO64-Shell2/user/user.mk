# Users files.
USERSRC = ./user/thdgps.c \
			./user/thdmems.c 

USERINC = ./user
include ./user/nmea/minmea.mk
include $(CHIBIOS)/os/ex/devices/ST/lsm6dsl.mk
# Shared variables
ALLCSRC += $(USERSRC)
ALLINC  += $(USERINC)
