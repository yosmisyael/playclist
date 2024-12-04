
// Created by yosev on 11/26/24.
//

#ifndef TYPE_H
#define TYPE_H
#include <ncurses.h>

typedef struct {
    int mnts;
    int secs;
} duration_t;

typedef struct {
    char* title;
    char* artist;
    int year;
    duration_t duration;
} song_t;

typedef struct{
    song_t* songs;
    size_t index;
    size_t size;
} song_list_t;

typedef struct {
    char *name;
    size_t index;
    size_t size;
    song_t *songs;
} playlist_t;

typedef struct {
    size_t index;
    size_t size;
    playlist_t *playlists;
} playlist_coll_t;

typedef struct {
    int song_payload;
    int playlist_payload;
    int main_menu_payload;
    song_list_t song_list;
    playlist_coll_t playlist_coll;
} context_t;

typedef struct {
    WINDOW *left_win;
    WINDOW *right_win;
    int pad_x;
    int pad_y;
} screen_t;

#endif
