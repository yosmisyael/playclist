//
// Created by yosev on 11/30/24.
//

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ncutil.h"

void memcheck (const void *pointer) {
    if (pointer == NULL) {
        perror("failed allocating memory");
        exit(EXIT_FAILURE);
    }
}

void redraw_frm(WINDOW *win) {
    wclear(win);
    box(win, 0, 0);
    wrefresh(win);
};

void rend_logo(WINDOW *win, int start_y, int start_x) {
    int width = getmaxx(win);
    char *logo[] = {
        "       _                 ",
        " _ __ | | __ _ _   _     ",
        "| '_ \\| |/ _` | | | |    ",
        "| |_) | | (_| | |_| |   ",
        "| .__/|_|\\__,_|\\__, |    ",
        "|_|___ _     __|___/ _   ",
        " / ___| |   |_ _|___| |_  ",
        "| |   | |    | |/ __| __|",
        "| |___| |___ | |\\__ | |_ ",
        " \\____|_____|___|___/\\__|",
        NULL,
    };
    int mid_x = (width - (int)strlen(logo[0]) - start_x) / 2;

    for (int i = 0; logo[i] != NULL; ++i) {
        mvwprintw(win, start_y + i, mid_x, "%s", logo[i]);
    }
}

void redraw_left_win(screen_t *scr, context_t *ctx) {
    redraw_frm(scr->left_win);
    int trk_y = scr->pad_y;
    int width = getmaxx(scr->left_win) - scr->pad_x * 4;
    char *opts[] = {"Song", "Playlist", "Exit"};
    int opts_len = sizeof(opts) / sizeof(char*);
    rend_logo(scr->left_win, trk_y, scr->pad_x);
    trk_y += 11;

    for (int i = 0; i < opts_len; i++) {
        if (i == ctx->main_menu_payload) {
            wattron(scr->left_win, A_REVERSE);
            mvwprintw(scr->left_win, i + trk_y, scr->pad_x, " > %-*s", width, opts[i]);
            wattroff(scr->left_win, A_REVERSE);
        } else {
            mvwprintw(scr->left_win, i + trk_y, scr->pad_x, "   %-*s", width, opts[i]);
        }
    }
    wrefresh(scr->left_win);
}

void wget_in(const WINDOW* win,
    bool is_dialog,
    int start_y,
    int start_x,
    const char* prom,
    char* buff,
    size_t buff_size
    ) {
    const int marg = 2;
    const int width = getmaxx(win) - (int)(getmaxx(win) - buff_size), height = 3;
    int key;
    int cur_x = 2;
    size_t index = 0;
    buff[index] = '\0';

    curs_set(true);
    WINDOW *field = newwin(height, width + marg * 2, start_y + marg, start_x + marg);
    box(field, 0, 0);

    // apply style for dialog mode
    if (is_dialog) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLUE);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        wbkgd(field, COLOR_PAIR(1));
        wattron(field, A_BOLD);
    }

    // render input box
    mvwprintw(field, height - 3, 1, "%s", prom);
    mvwprintw(field, height - 1, getmaxx(field) - 7, "%lu/%lu", strlen(buff), buff_size);
    wmove(field, height - 2, marg);
    wrefresh(field);
    keypad(field, true);

    // handle input
    while ((key = wgetch(field)) != KEY_RETURN) {
        switch (key) {
            case KEY_RIGHT: {
                if (cur_x - marg < strlen(buff)) {
                    ++index;
                    ++cur_x;
                }
                break;
            }
            case KEY_LEFT: {
                if (index > 0) {
                    --index;
                    --cur_x;
                }
                break;
            }
            case KEY_BACKSPACE: {
                if (index > 0) {
                    --cur_x;
                    --index;
                    memmove(&buff[index], &buff[index + 1], strlen(buff) + 1 - index);
                }
                break;
            }
            default: {
                if (isprint(key) && index < buff_size) {
                    memmove(&buff[index + 1], &buff[index], strlen(buff) + 1 - index);
                    buff[index++] = (char)key;
                    ++cur_x;
                }
                break;
            }
        }

        // control cursor position, clear screen from deleted chars and rerender input box
        wmove(field, height - 2, marg);
        wclrtoeol(field);
        wmove(field, height - 1, marg);
        wclrtoeol(field);
        box(field, 0, 0);
        mvwprintw(field, 0, 1, "%s", prom);
        mvwprintw(field, 1, marg, "%s", buff);
        mvwprintw(field, 2, getmaxx(field) - 7, "%lu/%lu", strlen(buff), buff_size);
        wmove(field, 1, cur_x);
        wrefresh(field);
    }
    curs_set(false);
    delwin(field);
}

void seed_song(song_list_t *song_list) {
    struct {
        char *title;
        char *artist;
        int year;
        int mnts;
        int secs;
    } seed_data[] = {
        {"Hey Jude", "The Beatles", 1968, 7, 11},
        {"Let It Be", "The Beatles", 1970, 4, 3},
        {"Come Together", "The Beatles", 1969, 4, 20},
        {"Something", "The Beatles", 1969, 3, 2},
        {"Here Comes the Sun", "The Beatles", 1969, 3, 6},
        {"Help!", "The Beatles", 1965, 2, 18},
        {"A Day in the Life", "The Beatles", 1967, 5, 33},
        {"Yesterday", "The Beatles", 1965, 2, 5},
        {"Eleanor Rigby", "The Beatles", 1966, 2, 8},
        {"While My Guitar Gently Weeps", "The Beatles", 1968, 4, 45},
        {"Master of Puppets", "Metallica", 1986, 8, 36},
        {"One", "Metallica", 1988, 7, 27},
        {"Nothing Else Matters", "Metallica", 1991, 6, 28},
        {"Enter Sandman", "Metallica", 1991, 5, 32},
        {"Seek & Destroy", "Metallica", 1983, 6, 55},
        {"Stairway to Heaven", "Led Zeppelin", 1971, 8, 2},
        {"Kashmir", "Led Zeppelin", 1975, 8, 37},
        {"Whole Lotta Love", "Led Zeppelin", 1969, 5, 34},
        {"Smoke on the Water", "Deep Purple", 1972, 5, 40},
        {"Highway Star", "Deep Purple", 1972, 6, 5},
        {"Bohemian Rhapsody", "Queen", 1975, 5, 55},
        {"Shine On You Crazy Diamond", "Pink Floyd", 1975, 13, 31},
        {"Smells Like Teen Spirit", "Nirvana", 1991, 5, 1},
        {"Hotel California", "Eagles", 1976, 6, 25},
        {"Time in a Bottle", "Jim Croce", 1972, 2, 27},
        {"Lemon", "Kenshi Yonezu", 2020, 4, 15},
        {"Live My Life", "aespa", 2024, 2, 41},
        {"Over the Moon", "TXT", 2024, 2, 37},
        {"Imagine", "John Lennon", 1971, 3, 4},
        {"Another Brick in the Wall", "Pink Floyd", 1979, 5, 20},
        {"Like a Rolling Stone", "Bob Dylan", 1965, 6, 13},
        {"Hotel California", "Eagles", 1976, 6, 25}
    };
    int data_len = sizeof(seed_data) / sizeof(seed_data[0]);
    if (song_list->size - song_list->index <= data_len) {
        song_t *tmp = realloc(song_list->songs, (data_len + song_list->size) * sizeof(song_t));
        memcheck(tmp);
        song_list->songs = tmp;
        song_list->size += data_len;
    }
    int tmp_index = (int) song_list->index;
    for (int i = 0; i < data_len; ++i) {
        song_list->songs[i + tmp_index] = (song_t) {
            .title = strdup(seed_data[i].title),
            .artist = strdup(seed_data[i].artist),
            .year = seed_data[i].year,
            .duration = { .mnts = seed_data[i].mnts, .secs = seed_data[i].secs, }
        };
        song_list->index++;
    }
}

