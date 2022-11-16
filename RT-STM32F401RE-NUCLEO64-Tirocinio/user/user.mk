# Users files.
USERSRC = ./user/thdgps.c \
			./user/thdmems.c \
			./user/quaternion.c\
			./user/thdSD.c

USERINC = ./user
include ./user/nmea/minmea.mk
include ./user/MadgwickAHRS/AHRS.mk
include ./user/Fusion/Fusion.mk
include $(CHIBIOS)/os/ex/devices/ST/lsm6dsl.mk
include $(CHIBIOS)/os/ex/devices/ST/lsm303agr.mk
# Shared variables
ALLCSRC += $(USERSRC)
ALLINC  += $(USERINC)
