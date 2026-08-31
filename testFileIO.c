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
/////////////////////////////////

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
#include "testFileIO.h" // generate with makeheaders
#include "FileIO.h"     // generate with makeheaders
/////////////////////////////////////////////////////////////////

/*///////////////////////////////
This C code reads a file "in.txt" to a data struct
then writes it to another file "out.txt"
example output:

Restored timestamp from file: 2026.08.05T12.15.15Z
New timestamp written to ./timestamp.buf
timestamp written was, 2026.08.05T13.06.35Z
Original comparison ('tacos'): MATCH (true)
Modified comparison ('bacos'): MISMATCH (false)

/*///////////////////////////////
int main() {
    // read/write to structs with objct2deck() deck2objct()

    date dt;

    file2deck2date(&dt); // read date from file
    printf("\nRestored timestamp from file: ");
    print_date(dt); // Display the restored timestamp

    dt = get_system_date(); // update date
    date2deck2file(&dt);
    printf("\nNew timestamp written to ./timestamp.buf");

    printf("\ntimestamp written was, ");
    print_date(dt); // Display it

    taco_test();

    return 0;
}

/////////////////////////////////
// FileIO functions (objct2deck, deck2file_nobuf, file2deck,
// deck2objct, OBJcmprd2deck) are in FileIO.c / FileIO.h
/////////////////////////////////

void taco_test(void) {
    char test_cstr[] = "tacos"; // Includes trailing '\0' (6 bytes total)
    size_t data_size = sizeof(test_cstr);

    // 1. Load data into deck
    LLDeck_uint8_t *deck = objct2deck(test_cstr, data_size);

    // 2. Compare original data (Expect: true)
    bool is_match = OBJcmprd2deck(test_cstr, data_size, &deck);
    printf("\nOriginal comparison ('tacos'): %s\n", is_match ? "MATCH (true)" : "MISMATCH (false)");

    // 3. Edit 1 character in the array
    test_cstr[0] = 'b'; // Changed to "bacos"

    // 4. Compare modified data (Expect: false)
    bool is_modified_match = OBJcmprd2deck(test_cstr, data_size, &deck);
    printf("Modified comparison ('bacos'): %s\n", is_modified_match ? "MATCH (true)" : "MISMATCH (false)");
}
/////////////////////////////////
// N1256 Section 6.7.2.1 subsection 12-13
// 
// Each non-bit-field member of a structure or union object is 
// aligned in an implementation- defined manner appropriate to its 
// type.
// 
// Within a structure object, the non-bit-field members and the 
// units in which bit-fields reside have addresses that increase in 
// the order in which they are declared.
// 
// A pointer to a structure object, suitably converted, points to 
// its initial member (or if that member is a bit-field, then to the 
// unit in which it resides), and vice versa. There may be unnamed 
// padding within a structure object, but not at its beginning.
/////////////////////////////////

// What this means:
// A byte is the smallest addressable item for C pointers.
// The order members of a struct are saved in memory must 
// match the order they appear in the struct declaration.

// uint32_t = member_count

#if INTERFACE
typedef struct { // 2 bits encode 4 scenarios(1, 2, 4, 8)
    uint8_t v0 : 2; // max_val = 3
    uint8_t v1 : 2; // max_val = 3
    uint8_t v2 : 2; // max_val = 3
    uint8_t v3 : 2; // max_val = 3
} byte4vars;
#endif // EXPORT_INTERFACE

// offsetof(byte4vars, v0) = 0
// offsetof(byte4vars, v1) = 0
// offsetof(byte4vars, v2) = 0
// offsetof(byte4vars, v3) = 0

/////////////////////////////////

// Set a 2-bit value at a specific index
void set_2bit(uint8_t *array, int index, uint8_t value) {
    int byte_index = index / 4;
    int bit_offset = (index % 4) * 2;
    
    // Clear the 2 bits at the target position, then OR the new value (masked to 2 bits)
    array[byte_index] &= ~(0x03 << bit_offset);
    array[byte_index] |= (value & 0x03) << bit_offset;
}

// Get a 2-bit value from a specific index
uint8_t get_2bit(const uint8_t *array, int index) {
    int byte_index = index / 4;
    int bit_offset = (index % 4) * 2;
    
    return (array[byte_index] >> bit_offset) & 0x03;
}

/////////////////////////////////

#if INTERFACE
typedef struct {
    uint16_t y; // max_val = ?
    uint16_t m : 4; // max_val = 12
    uint16_t d : 5; // max_val = 31
    uint16_t h : 5; // max_val = 24
    uint16_t n : 6; // max_val = 60
    uint16_t s : 6; // max_val = 60
} date;
#endif

// Function to get the current system time and pack it into the 'date' struct
date get_system_date() {
    time_t rawtime;
    time(&rawtime);
    
    // Convert to local time structure (requires time.h which is covered in Annex B)
    struct tm *info = localtime(&rawtime);
    
    date dt;
    dt.y = (uint16_t)(info->tm_year + 1900); // tm_year counts years since 1900
    dt.m = (uint16_t)(info->tm_mon + 1);    // tm_mon is 0-11, so add 1
    dt.d = (uint16_t)info->tm_mday;           // Day of the month [1-31]
    dt.h = (uint16_t)info->tm_hour;           // Hours [0-23]
    dt.n = (uint16_t)info->tm_min;            // Minutes [0-59]
    dt.s = (uint16_t)info->tm_sec;            // Seconds [0-59]
    
    return dt;
}

// Function to display the 'date' struct using your requested format
void print_date(date dt) {
    printf("%04d.%02d.%02dT%02d.%02d.%02dZ", 
           dt.y, dt.m, dt.d, dt.h, dt.n, dt.s);
}

void date_test() {
    // printing size of structure
    printf("Size of date is %lu bytes\n",
           sizeof(date));
    printf("Size of time_t is %lu bytes\n",
           sizeof(time_t));
    // date dt = { 2014, 12, 31, 23, 59, 59 };
    date Unix_Epoch = { 1970, 1, 1, 0, 0, 0 };
    date dt = get_system_date(); // Fetch live system time
    print_date(dt); // Display it
    
    print_date(Unix_Epoch);
    // print_date( get_system_date() ); // same as
    // printf("Date is %d.%d.%dT%d.%d.%dZ", dt.y, dt.m, dt.d, dt.h, dt.n, dt.s);
    // 2026.07.23T17.38.48Z
}

// read file back into deck and deserialize to a date struct
void file2deck2date(date * restored_dt) {
    // Read the serialized file back into a deck
    LLDeck_uint8_t *date_deck = file2deck("./timestamp.buf");
    // Deserialize the deck into the date structure only if file existed
    if (Size_LLDeck_uint8_t(&date_deck) >= sizeof(date)) {
        deck2objct(restored_dt, sizeof(date), &date_deck);
    }
    free(date_deck);
}

void date2deck2file(date * dt) {
    // write object to deck
    LLDeck_uint8_t * date_deck = objct2deck(dt, sizeof(date));
    // Write deck to file
    deck2file_nobuf("./timestamp.buf", &date_deck);
    free(date_deck);
}

