#ifndef _hucc_legacy_h
#define _hucc_legacy_h

/****************************************************************************
; ***************************************************************************
;
; huc.h
;
; HuCC wrapper file to include a default set of HuC-compatible functions.
;
; Copyright John Brandwood 2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; When the HuCC compiler is invoked with the "--legacy" option to compile old
; projects, then this "huc.h" file is automatically included.
;
; This file includes a set of the individual HuCC headers which correspond to
; HuC's built-in functions and MagicKit library.
;
; New HuCC projects should, preferably, include the specific headers that are
; needed, which will then allow them to replace parts of the standard library
; with newer functions, or they can just manually include this "huc.h" file.
;
; ***************************************************************************
; **************************************************************************/

#include "hucc-systemcard.h"
#include "hucc-baselib.h"
#include "hucc-gfx.h"
#include "hucc-old-scroll.h"
#include "hucc-old-spr.h"
#include "hucc-old-map.h"
#include "hucc-old-line.h"

#include <stdlib.h>
#include <string.h>

#endif // _hucc_legacy_h
