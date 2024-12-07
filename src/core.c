//
// Created by yosev on 12/7/24.
//

#include "core.h"
#include "ncutil.h"
#include "song.h"
#include "playlist.h"

MenuState main_menu(screen_t *scr, context_t *ctx) {
    int trk_y = scr->pad_y;
    int width = getmaxx(scr->left_win) - scr->pad_x * 4;
    char *opts[] = {"Song", "Playlist", "Exit"};
    int opts_len = sizeof(opts) / sizeof(opts[0]);
    int sel = ctx->main_menu_payload == -1 ? 0 : ctx->main_menu_payload;

    keypad(scr->left_win, true);
    redraw_frm(scr->right_win);
    box(scr->left_win, 0, 0);
    box(scr->right_win, 0, 0);
    rend_logo(scr->left_win, trk_y, scr->pad_x);
    trk_y += 11;

    while (1) {
        // render options
        for (int i = 0; i < opts_len; i++) {
            if (i == sel) {
                wattron(scr->left_win, A_REVERSE);
                mvwprintw(scr->left_win, i + trk_y, scr->pad_x, " > %-*s", width, opts[i]);
                wattroff(scr->left_win, A_REVERSE);
            } else {
                mvwprintw(scr->left_win, i + trk_y, scr->pad_x, "   %-*s", width, opts[i]);
            }
        }

        // handle key input
        int key = wgetch(scr->left_win);
        switch (key) {
            case KEY_UP: {
                if (sel > 0) --sel;
                break;
            }
            case KEY_DOWN: {
                if (sel < opts_len - 1) ++sel;
                break;
            }
            default: {
                if (key == KEY_RETURN) {
                    ctx->main_menu_payload = sel;
                    switch (sel) {
                        case 0: return STATE_SONG_MENU;
                        case 1: return STATE_PLAYLIST_MENU;
                        default: return STATE_EXIT;
                    }
                }
                break;
            }
        }
    }
}

MenuHandler menu_handlers[] = {
    main_menu,              // STATE_MAIN_MENU
    song_menu,              // STATE_SONG_MENU
    list_song,              // STATE_LIST_SONG
    search_song,            // STATE_SEARCH_SONG
    add_song,               // STATE_ADD_SONG
    delete_song,            // STATE_DELETE_SONG
    playlist_menu,          // STATE_PLAYLIST_MENU
    list_playlist,          // STATE_LIST_PLAYLIST
    get_playlist,           // STATE_GET_PLAYLIST
    create_playlist,        // STATE_CREATE_PLAYLIST
    rename_playlist,        // STATE_RENAME_PLAYLIST
    edit_playlist,          // STATE_EDIT_PLAYLIST
    delete_playlist,        // STATE_DELETE_PLAYLIST
};