// #!/run/current-system/sw/bin/tcc -run

/*///markdown

# SOPs gives us 4 scenarios of `.h` Header files for `C` projects

## 1. Standalone Header Files (`.h` without a corresponding `.c`)

A standalone header is an include-only unit and must never
produce object code.

**Allowed:**
- Macros
- `static inline` functions
- Type definitions (`struct`, `enum`, `union`, `typedef`)
- `extern` declarations
- Function prototypes for inline functions
- compile cleanly with -Wall -Wextra -Werror when tested via
```
#include "foo.h"
int main(void) { return 0; }
```

**Forbidden:**
- Non-inline function definitions
- Global variable definitions
- Static data objects (except `static const`)
- Any code requiring a `.o` or `.so`

Standalone headers must be self-contained, include their own
dependencies, use include guards, and compile cleanly when
included alone.

## 2. A C-file with main() it is not a library, it is a program

the target is a executable binary,
it might result in a `.o` file as an intermediate step to making
the binary, but it will not provide a `.so`file

## 3. System includes `#include <foo.h>` generated from `.c` files
the target is a `.so` file, it will not provide a `.o`file

-fpic (lowercase): Generates smaller, slightly faster code, but 
has an architecture-dependent limit on the total size of the 
lookup table.

-fPIC (uppercase): Avoids any size limits on the lookup table, 
making it the safer, universally preferred choice for heavy or 
complex shared libraries.

## 4. local includes `#include "foo.h"` generated from `.c` files
the target is a `.o` file, it will not provide a `.so`file

----

# This file shal serve as an example, of category 2

# Source/Header Pairs, give us the other scenarios 2 to 4

each `.c` file WILL have corresponding `.h` Generation Target

*e.g.* `foo.h` is made from `foo.c`, in this example foo.h is a
target output file based on `foo.c`.

We don't have to worry too much about these `.h` files: If a `.c`
file is newer than a `.h` file; a call to `makeheaders` will
regenerate the `.h` file.

However we do need to worry about the structure of the `.c`
files so they work cleanly with `makeheaders`

> NOTE: categories 3 and 4 are the same except other files
> don't include c files in category 2.

makeheaders requires INTERFACE directive wrapped around
Pre-Defined Constants & Data Structures accessible in other
scopes. Declarations provide the interface and the definitions
are the implementation.

```bash
makeheaders -local ./pargs.c
gcc -m64 -O2 -std=gnu99 pargs.c -o pargs
./pargs Hello to you pargs!
```
outputs:
Hello World!
Hello to you pargs!

```c
/*C*/

#include <unset_priv.h>/*//////////////////////////////////////*/
//assert/inttypes/signal /stdlib/complex/iso646/stdarg/string//
//ctype /limits  /stdbool/tgmath/errno  /locale/stddef/time  //
//fenv  /math    /stdint /wchar /float  /setjmp/stdio /wctype//
#include <annexb.h> // ^^ includes above .h files ^^  [p419N1256]

// #include "somefile.h" // include files here


#include <set_priv.h>/*////////////////////////////////////////*/
// include automated local declarations last needs
#include "pargs.h" // generate with pargs.sh
/////////////////////////////////////////////////////////////////

#if INTERFACE/* ___makeheaders Pre-Defined Constants INTERFACE */

/* ____________________makeheaders Pre-Defined Data Structures */

#endif // EXPORT_INTERFACE

/*///markdown
```

## Function Implementations

```c
*///////////



int64_t main(int argc, char **argv){// *argv++ is *((char **)(argv++))
    // https://stackoverflow.com/a/72203145/144020
    // argv++; while (*argv) printf("%s ", *argv++); argv = argv - argc;
    pargs( argc, (uint8_t **)argv);
    test( argc, argv);
    return 0;
}

// pargs -  describes data types of argc an argv by printing
// command line args
void pargs(int argc, uint8_t **argv){// *argv++ is *((char **)(argv++))
// argv argc style of pointers to null terminated strings
/* arguments were entered on the command line when the program was started.
 * argv (argument vector) - an array of pointers to arrays of chars
 * argc (argument count) number of elements in the vector */
    printf("\n");
    while (*argv) printf("%s ", *argv++); argv = argv - argc;
    // -argc to reset argv to original
    printf("\n");
}

/*///markdown
```

## Helper Function Implementations

```c
/*C*/

#if PARGS_PRIV_INTERFACE // ALL STATIC FUNCTIONS NEED TO BE IN
/* Makeheaders will process this prototype for the local header file */
// static functions are unavailable outside the file they are defined.
static void test(int argc, char **argv){
    // there can be another function with the same name "test" in
    // another c file, this is normally invalid.
    void * sum_ptr = zalloc_unsafe(10);
    free(sum_ptr);
    printf("test complete\n");
}
#endif
#undef PARGS_PRIV_INTERFACE

/*
```
*///markdown
