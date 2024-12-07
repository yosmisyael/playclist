#include <ncurses.h>
#include <stdlib.h>
#include "ncutil.h"
#include "type.h"
#include "core.h"

int main() {
    MenuState current_state = STATE_MAIN_MENU;
    initscr();
    noecho(); cbreak(); curs_set(false);
    ESCDELAY = 0;

    song_t *song_arr = malloc(sizeof(song_t));
    memcheck(song_arr);
    song_list_t song_list = {
        .songs = song_arr,
        .index = 0,
        .size = 10,
    };
    seed_song(&song_list);

    playlist_t *playlists = malloc(sizeof(playlist_t));
    memcheck(playlists);
    playlist_coll_t playlist_coll = {
        .playlists = playlists,
        .index = 0,
        .size = 1,
    };

    screen_t scr = {
        .left_win = newwin(getmaxy(stdscr), getmaxx(stdscr) / 3, 0, 0),
        .right_win = newwin(getmaxy(stdscr), getmaxx(stdscr) * 2 / 3, 0, getmaxx(stdscr) / 3),
        .pad_x = 2,
        .pad_y = 2,
    };

    context_t context = {
        .playlist_payload = -1,
        .song_payload = -1,
        .main_menu_payload = -1,
        .song_list = song_list,
        .playlist_coll = playlist_coll,
    };

    while (current_state != STATE_EXIT) {
        current_state = menu_handlers[current_state](&scr, &context);
    }

    delwin(scr.left_win);
    delwin(scr.right_win);
    endwin();

    free(song_arr);
    free(playlists);
    return 0;
}

