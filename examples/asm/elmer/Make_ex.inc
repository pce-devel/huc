CC  ::= ../../../../bin/huc
AS  ::= ../../../../bin/pceas
IL  ::= ../../../../bin/isolink
S2I ::= ../../../../bin/sym2inc
W2V ::= ../../../../bin/wav2vox

PCE_INCLUDE ::= ../include;../font
PCE_PCEAS ::= $(AS)

ifneq ($(OS),Windows_NT)
PCE_INCLUDE ::= $(subst ;,:,$(PCE_INCLUDE))
endif

export PCE_INCLUDE
export PCE_PCEAS
