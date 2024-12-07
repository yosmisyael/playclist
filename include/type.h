
// Created by yosev on 11/26/24.
//

#ifndef TYPE_H
#define TYPE_H
#include <ncurses.h>

#define KEY_DELETE  4
#define KEY_EDIT    5
#define KEY_SEARCH  6
#define KEY_RETURN  10
#define KEY_RENAME  18
#define KEY_ESCAPE  27
#define KEY_SPACE   32

typedef enum {
    STATE_MAIN_MENU,
    STATE_SONG_MENU,
    STATE_LIST_SONG,
    STATE_SEARCH_SONG,
    STATE_ADD_SONG,
    STATE_DELETE_SONG,
    STATE_PLAYLIST_MENU,
    STATE_LIST_PLAYLIST,
    STATE_GET_PLAYLIST,
    STATE_CREATE_PLAYLIST,
    STATE_RENAME_PLAYLIST,
    STATE_EDIT_PLAYLIST,
    STATE_DELETE_PLAYLIST,
    STATE_EXIT
} MenuState;

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
