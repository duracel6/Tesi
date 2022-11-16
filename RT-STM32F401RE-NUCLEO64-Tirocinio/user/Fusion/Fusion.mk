# Users files.
FUSIONINC = ./user/Fusion
FUSIONSRC = $(wildcard $(FUSIONINC)/*.c)


# Shared variables
ALLCSRC += $(FUSIONSRC)
ALLINC  += $(FUSIONINC)
