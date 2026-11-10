// #!/run/current-system/sw/bin/tcc -run

// Copyright (C) 2023-2025 Roy Pfund. All rights reserved.
//
// Permission is  hereby  granted,  free  of  charge,  to  any  person
// obtaining a copy of  this  software  and  associated  documentation
// files  (the  "Software"),  to  deal   in   the   Software   without
// restriction, including without limitation the rights to use,  copy,
// modify, merge, publish, distribute, sublicense, and/or sell  copies
// of the Software, and to permit persons  to  whom  the  Software  is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and  this  permission  notice  shall  be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT  WARRANTY  OF  ANY  KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES  OF
// MERCHANTABILITY,   FITNESS   FOR   A   PARTICULAR    PURPOSE    AND
// NONINFRINGEMENT.  IN  NO  EVENT  SHALL  THE  AUTHORS  OR  COPYRIGHT
// OWNER(S) BE LIABLE FOR  ANY  CLAIM,  DAMAGES  OR  OTHER  LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING  FROM,
// OUT OF OR IN CONNECTION WITH THE  SOFTWARE  OR  THE  USE  OR  OTHER
// DEALINGS IN THE SOFTWARE.

/* build, run, and destroy with:
gcc -m64 -O2 -std=gnu99 Pithon.c -o Pithon && ./Pithon && rm ./Pithon

or just uncomment the tcc shebang
*/

#include <unset_priv.h>//////////////////////////////////////////
// Annex B (informative)_Library_summary____________________ [p419N1256]
// assert inttypes signal  stdlib complex iso646 stdarg string
// ctype  limits   stdbool tgmath errno   locale stddef time
// fenv   math     stdint  wchar  float   setjmp stdio  wctype
#include <annexb.h> // ^^ includes the above .h files ^^     [p419N1256]
// ___________________________printf() directives provided by inttypes.h
// Hex      // %02"PRIX8" // %04"PRIX16" // %08"PRIX32" // %016"PRIX64"
// Signed   // %"PRIi8"   // %"PRIi16"   // %"PRIi32"   // %"PRIi64"
// Unsigned // %"PRIu8"   // %"PRIu16"   // %"PRIu32"   // %"PRIu64"
// Float    //  XXXXXX    //   XXXXXX    //   %.6E      // %.15E
// #include "eval_ops.h" // not used
#include <CALLDeck.h>
#include <PiChudnov.h>
#include <ncurses.h>

#include <set_priv.h>/*////////////////////////////////////////*/
// include automated local declarations last needs

// #include "PIthon.h" // generate with makeheaders

#include <unset_priv.h>//////////////////////////////////////////
const uint8_t framerate = 60;
const uint8_t applechar = '@';
const uint8_t emptychar = ' ';
const uint8_t width = 40;
const uint8_t height = 40;
const uint8_t moves_per_second = 8;
const uint32_t apple_spawn_interval = 60;

typedef struct { uint8_t x; uint8_t y; } coord;
DEFINE_CALLDECK(coord, 255)

typedef coord SnakeSegment;
typedef coord Apple;
typedef coord SpaceCoord;

typedef struct {
    LLDeck_CA_coord_255 *snake;
    LLDeck_CA_coord_255 *apples;
    LLDeck_CA_coord_255 *emptyspaces;

    uint8_t *snake_world;
    uint8_t *snake_chars; // "3.14159..." one char per segment, head first

    uint16_t snake_length;
    uint16_t apple_count;
    uint16_t space_count;

    uint32_t apple_spawn_timer;
    uint8_t frame_count;

    coord snake_head_new;

    int8_t direction_x;
    int8_t direction_y;
} GameState;

// Fill chars[0..len-1] with "3.14159...": len - 1 digits of pi plus the '.'
int fill_snake_chars(uint8_t *chars, size_t len) {
    size_t digits = len - 1;
    size_t words = pichudnov_words_needed(10, digits);
    uint64_t *packed = malloc(words * sizeof *packed);
    if (!packed) return PICHUDNOV_ERR_INTERNAL;

    int rc = pichudnov_bs(10, digits, packed, words);
    if (rc == PICHUDNOV_OK) {
        // digit i goes to slot i + 1, leaving slot 0 free for the '3'
        for (size_t i = 0; i < digits; i++)
            chars[i + 1] = '0' + ((packed[i / 16] >> (60 - 4 * (i % 16))) & 0xF);
        chars[0] = chars[1];
        chars[1] = '.';
    }
    free(packed);
    return rc;
}

void handle_input(int8_t *direction_x, int8_t *direction_y) {
    int ch = getch();
    if (ch != ERR) {
        int8_t new_dx = *direction_x;
        int8_t new_dy = *direction_y;

        switch (ch) {
            case KEY_UP: new_dx = 0; new_dy = -1; break;
            case KEY_DOWN: new_dx = 0; new_dy = 1; break;
            case KEY_LEFT: new_dx = -1; new_dy = 0; break;
            case KEY_RIGHT: new_dx = 1; new_dy = 0; break;
            default: return;
        }

        if (new_dx != -*direction_x || new_dy != -*direction_y) {
            *direction_x = new_dx;
            *direction_y = new_dy;
        }
    }
}

void sleepframetime() {
    nanosleep(&(struct timespec){ .tv_sec = 0, .tv_nsec = 1000000000L / framerate }, NULL);
}

void update_world(GameState *gs) {

    if (gs->frame_count >= framerate / moves_per_second) {
        gs->frame_count = 0;

        memset(gs->snake_world, emptychar, width * height);

        coord snake_head_current;
        if (popStart_CALLDECK_coord_255(&snake_head_current, &gs->snake)) {

            pushStart_CALLDECK_coord_255(&snake_head_current, &gs->snake);

            // compute new head
            gs->snake_head_new.x = (snake_head_current.x + gs->direction_x + width) % width;
            gs->snake_head_new.y = (snake_head_current.y + gs->direction_y + height) % height;

            bool ate_apple = false;

            // apple collision check
            for (uint16_t i = 0; i < gs->apple_count; i++) {
                coord apple_coord;
                popStart_CALLDECK_coord_255(&apple_coord, &gs->apples);

                if (memcmp(&apple_coord, &gs->snake_head_new, sizeof(coord)) == 0) {
                    ate_apple = true;
                } else {
                    pushEnd_CALLDECK_coord_255(&apple_coord, &gs->apples);
                }
            }

            // push new head
            pushStart_CALLDECK_coord_255(&gs->snake_head_new, &gs->snake);

            if (ate_apple) {
                gs->snake_length++;
                gs->apple_count--;
            } else {
                coord dummy;
                popEnd_CALLDECK_coord_255(&dummy, &gs->snake);
            }
        }

        // clear board
        for (uint8_t y = 0; y < height; ++y)
            for (uint8_t x = 0; x < width; ++x)
                gs->snake_world[y * width + x] = emptychar;

        // draw snake + detect self collision
        for (uint16_t i = 0; i < gs->snake_length; i++) {
            coord snake_seg;
            popStart_CALLDECK_coord_255(&snake_seg, &gs->snake);

            if (i > 0 && memcmp(&snake_seg, &gs->snake_head_new, sizeof(coord)) == 0) {
                endwin();
                printf("Game Over! Snake collided with itself.\n");
                exit(0);
            }

            gs->snake_world[snake_seg.y * width + snake_seg.x] = gs->snake_chars[i];
            pushEnd_CALLDECK_coord_255(&snake_seg, &gs->snake);
        }

        // draw apples
        for (uint16_t i = 0; i < gs->apple_count; i++) {
            coord apple_coord;
            popStart_CALLDECK_coord_255(&apple_coord, &gs->apples);
            gs->snake_world[apple_coord.y * width + apple_coord.x] = applechar;
            pushEnd_CALLDECK_coord_255(&apple_coord, &gs->apples);
        }

        // rebuild emptyspaces
        gs->space_count = 0;
        coord tmp;

        while (popStart_CALLDECK_coord_255(&tmp, &gs->emptyspaces)) {}

        for (uint8_t y = 0; y < height; ++y) {
            for (uint8_t x = 0; x < width; ++x) {
                if (gs->snake_world[y * width + x] == emptychar) {
                    coord space = { x, y };
                    pushEnd_CALLDECK_coord_255(&space, &gs->emptyspaces);
                    gs->space_count++;
                }
            }
        }

        // spawn apple
        gs->apple_spawn_timer++;
        if (gs->apple_spawn_timer >= moves_per_second && gs->space_count > 0) {

            uint16_t r = rand() % gs->space_count;
            coord chosen;

            for (uint16_t i = 0; i < r; i++) {
                popStart_CALLDECK_coord_255(&chosen, &gs->emptyspaces);
                pushEnd_CALLDECK_coord_255(&chosen, &gs->emptyspaces);
            }

            popStart_CALLDECK_coord_255(&chosen, &gs->emptyspaces);
            pushStart_CALLDECK_coord_255(&chosen, &gs->apples);

            gs->apple_count++;
            gs->apple_spawn_timer = 0;
        }
    }
}

int main(int argc, char **argv) {

    srand(time(NULL));

    uint8_t *snake_chars = malloc(width * height);
    if (!snake_chars || fill_snake_chars(snake_chars, width * height) != PICHUDNOV_OK) {
        fprintf(stderr, "PIthon: could not compute the digits of pi\n");
        return 1;
    }

    // Initialize ncurses
    initscr();
    cbreak();           // Disable line buffering
    noecho();           // Don't echo pressed keys
    keypad(stdscr, TRUE); // Enable arrow keys
    nodelay(stdscr, TRUE); // Non-blocking input
    curs_set(0);        // Hide cursor
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);

    // static LLDeck_CA_coord_255 *emptyspaces = NULL;
    // if (!emptyspaces) emptyspaces = zalloc(sizeof(LLDeck_CA_coord_255));

    // LLDeck_CA_coord_255 *snake  = zalloc(sizeof(LLDeck_CA_coord_255));
    // LLDeck_CA_coord_255 *apples = zalloc(sizeof(LLDeck_CA_coord_255));
    GameState gs = {
        .snake = zalloc(sizeof(LLDeck_CA_coord_255)),
        .apples = zalloc(sizeof(LLDeck_CA_coord_255)),
        .emptyspaces = zalloc(sizeof(LLDeck_CA_coord_255)),
        .snake_world = malloc(width * height),
        .snake_chars = snake_chars,
        .snake_length = 4,
        .apple_count = 0,
        .space_count = 0,
        .apple_spawn_timer = 0,
        .frame_count = 0,
        .direction_x = -1,
        .direction_y = 0
    };

    // "3.14" heading left, so it reads left to right at the start
    for (uint8_t i = 0; i < gs.snake_length; i++) {
        coord snake_seg = { width / 2 + i, height / 2 };
        pushEnd_CALLDECK_coord_255(&snake_seg, &gs.snake);
    }

    // uint8_t *snake_world = malloc(width * height);
    if (!gs.snake_world) { endwin(); return 1; }

    SnakeSegment initial_head = { width / 2, height / 2 };
    gs.snake_world[initial_head.y * width + initial_head.x] = 1;

    gs.snake_head_new = initial_head;

    for (;;) {

        handle_input(&gs.direction_x, &gs.direction_y);

        update_world(&gs);

        erase();
        for (uint8_t y = 0; y < height; ++y)
            for (uint8_t x = 0; x < width; ++x) {
                uint8_t cell = gs.snake_world[y * width + x];
                if (cell == applechar) {
                    attrset(COLOR_PAIR(1));
                    mvaddch(y, x, cell);
                    attroff(COLOR_PAIR(1));
                } else {
                    mvaddch(y, x, cell);
                }
            }

        mvprintw(height + 1, 0,
                 "Apples: %d  Snake: %d  Timer: %d  Dir:(%d,%d)",
                 gs.apple_count, gs.snake_length, gs.apple_spawn_timer,
                 gs.direction_x, gs.direction_y);

        refresh();

        ++gs.frame_count;
        sleepframetime();
    }

    endwin();
    free(gs.snake_world);
    free(gs.snake_chars);
    return 0;
}

