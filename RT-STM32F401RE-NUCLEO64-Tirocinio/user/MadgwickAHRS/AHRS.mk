# Users files.
AHRSINC = ./user/MadgwickAHRS
AHRSSRC = $(wildcard $(AHRSINC)/*.c)


# Shared variables
ALLCSRC += $(AHRSSRC)
ALLINC  += $(AHRSINC)
