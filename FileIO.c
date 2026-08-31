// #!/run/current-system/sw/bin/tcc -run

// Copyright (C) 2023-2026 Roy Pfund. All rights reserved.
//
// Permission is hereby granted, free of charge,  to  any  person
// obtaining a copy of this software and associated documentation
// files (the  "Software"),  to  deal  in  the  Software  without
// restriction, including without limitation the rights  to  use,
// copy, modify, merge, publish, distribute,  sublicense,  and/or
// sell copies of the Software, and to permit persons to whom the
// Software is furnished to  do  so,  subject  to  the  following
// conditions:
//
// THE SOFTWARE IS PROVIDED "AS  IS",  WITHOUT  WARRANTY  OF  ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT  NOT  LIMITED  TO  THE
// WARRANTIES  OF  MERCHANTABILITY,  FITNESS  FOR  A   PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS  OR
// COPYRIGHT OWNER(S) BE LIABLE FOR ANY CLAIM, DAMAGES  OR  OTHER
// LIABILITY,  WHETHER  IN  AN  ACTION  OF  CONTRACT,   TORT   OR
// OTHERWISE, ARISING FROM, OUT OF  OR  IN  CONNECTION  WITH  THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
///////////////////////////////

// The absence of the clause:

// The above copyright notice and this permission notice shall be
// included  in  all  copies  or  substantial  portions  of   the
// Software.

// puts this file into public domain.

#include <unset_priv.h>/*//////////////////////////////////////*/
/////////////////////////////////////////////////////////////////
// Annex B (informative)_Library_summary //////////// [p419N1256]
// assert inttypes signal  stdlib complex iso646 stdarg string
// ctype  limits   stdbool tgmath errno   locale stddef time
// fenv   math     stdint  wchar  float   setjmp stdio  wctype
#include <annexb.h> // ^^ includes above .h files ^^  [p419N1256]
/* eval_ops.h excluded from FileIO.c — not used here */

#include <lldeck.h>
#include <NuClear.h>

#if !INTERFACE
DEFINE_LLDECK(uint8_t)
#endif

#include <set_priv.h>/*////////////////////////////////////////*/
// include automated local declarations last needs
#include "FileIO.h" // generate with makeheaders
/////////////////////////////////////////////////////////////////

/*///////////////////////////////
load your data to a deck
/*///////////////////////////////
LLDeck_uint8_t * objct2deck(void *xref, size_t size) {
    uint8_t *bytes = (uint8_t *)xref;
    LLDeck_uint8_t* deck = NewClear(LLDeck_uint8_t);
    for (size_t i = 0; i < size; i++) {
        uint8_t val = bytes[i];
        pushEnd_LLDeck_uint8_t(&val, &deck);
    }

    return deck;
}

// save that deck to a file
void deck2file_nobuf(const char *path, LLDeck_uint8_t **deck) {
    FILE *fp = fopen(path, "wb");
    assert(fp != NULL);
    uint8_t byte;
    while (popStart_LLDeck_uint8_t(&byte, deck)) {
        size_t nwritten = fwrite(&byte, 1, 1, fp);
        assert(nwritten == 1);
    }
    fclose(fp);
}

// when the program starts back up read that file into a deck
LLDeck_uint8_t * file2deck(const char *path) {
    FILE *fp = fopen(path, "rb");
    LLDeck_uint8_t* deck = NewClear(LLDeck_uint8_t);
    if (fp) { // file exists
        uint8_t byte;
        while (fread(&byte, 1, 1, fp) == 1) {
            pushEnd_LLDeck_uint8_t(&byte, &deck);
        }
        fclose(fp);
    } else {
        fprintf(stderr, "Error: file '%s' does not exist or cannot be opened.\n", path);
    }
    return deck;
}

// load that deck into you data structures
void deck2objct(void *xref, size_t size, LLDeck_uint8_t **deck) {
    uint8_t *bytes = (uint8_t *)xref;
    for (size_t i = 0; i < size; i++) {
        uint8_t val;
        bool ok = popStart_LLDeck_uint8_t(&val, deck);
        assert(ok);
        bytes[i] = val;
    }
}

// compare bytes to deck
bool OBJcmprd2deck(void *xref, size_t size, LLDeck_uint8_t **deck) {
    if (deck == NULL || *deck == NULL) return false;
    // if (Size_LLDeck_uint8_t(deck) != size) return false; // size matches
    if (size == 0) return true;
    if (Size_LLDeck_uint8_t(deck) < size) return false; // deck is large enough
    LLNode_uint8_t *curr = NULL;
    if (!Start_LLDeck_uint8_t(&curr, deck)) return false;

    const uint8_t *bytes = (const uint8_t *)xref;
    for (size_t i = 0; i < size; i++) {
        if (curr == NULL || curr->data != bytes[i]) {
            return false;
        }
        curr = curr->next;
    }
    return true;
}
