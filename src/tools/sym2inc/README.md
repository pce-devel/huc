# sym2inc

This utility extracts a set of symbol names, listed in a ".S2I" text file,
from a PCEAS ".SYM" file, and it then uses them to create a PCEAS ".S"
file suitable for including in another project.

Its primary use is so that overlay programs can call functions, and use
variables, that are defined in a chunk of shared library code that is
built seperately.

Usage   : sym2inc [<options>] <filename.s2i>

Options :
  -h      Print this help message
