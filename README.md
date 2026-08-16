

What is HuC?
------------

HuC is a C compiler for the NEC PC Engine consoles (CoreGrafx/TurboGrafx), initially developed by David Michel around 1999. HuC was an upgraded version of Ron Cain's [Small-C](https://en.wikipedia.org/wiki/Small-C), and as such it inherited most of Small-C's limitations.

What is HuCC?
-------------

HuCC is a replacement for HuC that leverages the improvements made in the toolchain over the last few years. It incorporates some of the PC Engine knowledge and techniques that weren't uncovered until after the original HuC developers stopped maintaining their toolkit around 2005.

At its core, HuCC is still the same HuC compiler, and is 90+% compatible with existing HuC projects, but it uses a rewritten library, and a very different underlying code-generation scheme to provide a 20%-40% reduction in the size of the generated code, with a significant boost in performance (which will depend upon the type of C code that is used in the project).

The latest automated builds of HuCC for Windows, Linux and MacOS are available [here](/releases/). HuCC's archive also includes the last published version of HuC4.


What are the new features in HuCC?
----------------------------------

HuCC natively supports the SuperGrafx.

![magicianlord.sgx](/pic/magicianlord.gif "SuperGrafx proof-of-concept of Magician Lord")

HuC's legacy `scroll()` function has been superseded by a new optional `scroll_split()` function that is much faster, supports more scrolling regions, and supports both background layers on the SuperGrafx.

![demonblazon.sgx](/pic/demonblazon.apng "SuperGrafx slideshow of Demon's Blazon")

HuCC also implements color fading routines, as well as a brand new tilemapping library that massively improves upon HuC's legacy `#inctile` macro. Animated tiles are possible thanks to a fast VRAM to VRAM (DMA) copy function.

![multiblk.pce](/pic/multiblk.webp "multiblk.pce demo based on The Legend of Xanadu 2")

From a C language point-of-view, the big new "feature" is that function-pointers are now working again after being broken for 20+ years. That will allow C developers to implement game-entity behaviors in a faster and much more sensible way than the `switch()` statements, which HuC developers have traditionally had to use.

`switch()` statements are also twice as fast as HuC, for those very common times when function-pointers would be inappropriate. When the `case` values are sequential, HuCC can now optimize the `switch()` into a simple jump table instead of a set of comparisons, making the code even faster.

You can also now create C const arrays that contain the *bank* of data labels (such as sprites), to complement the existing ability to create C const arrays that contain the 16-bit *address* of data labels (such as sprites). This finally allows developers to create arrays of C `far` pointers to data in their HuCARD or CD overlay.

C structures were added to HuC4 by Ulrich Hecht around 2015, taken from the `struct` support added to Small-C back in the late 1980s, though it is a somewhat limited implementation. In particular, you can't statically initialize structures; you can only initialize individual variables and single-dimension arrays.

Individual global, static and "-fno-recursive" structs were fairly fast in HuC4, thanks to some cunning but slightly naughty tricks that Uli implemented. But both arrays of structs and pointers to structs are pretty slow.

HuCC is really only going to give you decent results if you use arrays of values (chars, ints or pointers). This is the classic 6502 recommendation to use "structs made of arrays" instead of "arrays made of structs".

The main thing to remember is that you really want to keep array and structure sizes <= 256 bytes. If you're going to use pointers, then declare/define them in zero-page with `__zp`. Try as hard as you can to avoid using the stack (i.e. parameters to functions and local variables). In HuCC, the stack goes in zero-page and is very small.

What is the focus of HuCC?
--------------------------

The primary focus for compiled code has been upon improving the existing code-generation so that things like function parameters, local variables, array accesses and conditional statements are no longer the complete embarrassment that they used to be.

The other focus has been on switching away from the restrictive [MagicKit](http://magicengine.com/mkit/) library and program structure, which made it so incredibly hard for assembly language developers to add new capabilities to the HuC library that C developers could then use.

The program structure that HuCC uses by default is based upon how the original PC Engine development studios wrote the most complex games for the console, such as ***The Legend of Xanadu*** and ***Anearth Fantasy Stories***, rather than MagicKit's use of a model similar to how early HuCARD games were written.

HuCC uses a modular library where virtually everything is optional and can be easily replaced, including the entire default program structure, should an adventurous developer really wish to do so.


Where can I get help with using HuCC?
-------------------------------------

Help can be found on the [PC Engine Forum](https://pcengine.proboards.com/) as well as on [PC Engine Fans](https://www.pcenginefans.com/).

If you prefer the Discord interface, here is an invite to the [HuCC Development](https://discord.gg/Pv85Tv5ft2) channel.

For an exhaustive list of functions supported by HuCC, check the [HuCC Function Reference](/doc/hucc/hucc-function-reference.md) document.


Main contributors over the years
--------------------------------

- David Michel
- Dave Shadoff
- Brent Garner
- Paul Clifford
- Olivier Jolly
- Xavier Carmona
- Rick Leverton
- Ulrich Hecht
- Artemio Urbina
- John Brandwood
