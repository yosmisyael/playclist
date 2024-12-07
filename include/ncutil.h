//
// Created by yosev on 11/26/24.
//

#ifndef NCUTIL_H
#define NCUTIL_H

#include "type.h"

void memcheck (const void *pointer);

void redraw_frm(WINDOW *win);

void rend_logo(WINDOW *win, int start_y, int start_x);

void redraw_left_win(screen_t *scr, context_t *ctx);

void wget_in(const WINDOW* win,
    bool is_dialog,
    int start_y,
    int start_x,
    const char* prom,
    char* buff,
    size_t buff_size
    );

void seed_song(song_list_t *song_list);

#endif //NCUTIL_H
