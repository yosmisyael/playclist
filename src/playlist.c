//
// Created by yosev on 12/7/24.
//

#include "playlist.h"
#include "ncutil.h"
#include <stdlib.h>
#include <string.h>


MenuState playlist_menu(screen_t *scr, context_t *ctx) {
    char *opts[] = {"View Playlist", "Create Playlist", "Back"};
    int opts_len = sizeof(opts) / sizeof(char*);
    int sel = 0;
    int width = getmaxx(scr->right_win) - scr->pad_x * 4;
    redraw_frm(scr->right_win);
    box(scr->right_win, 0, 0);
    keypad(scr->right_win, true);

    while (1) {
        // render options
        for (int i = 0; i < opts_len; i++) {
            if (i == sel) {
                wattron(scr->right_win, A_REVERSE);
                mvwprintw(scr->right_win, i + scr->pad_y, scr->pad_x, " > %-*s", width, opts[i]);
                wattroff(scr->right_win, A_REVERSE);
            } else {
                mvwprintw(scr->right_win, i + scr->pad_y, scr->pad_x, "   %-*s", width, opts[i]);
            }
        }

        // handle input
        int key = wgetch(scr->right_win);
        switch (key) {
            case KEY_UP: {
                if (sel > 0) --sel;
                break;
            }
            case KEY_DOWN: {
                if (sel < opts_len - 1) ++sel;
                break;
            }
            default:
                if (key == KEY_RETURN) {
                    switch (sel) {
                        case 0: return STATE_LIST_PLAYLIST;
                        case 1: return STATE_CREATE_PLAYLIST;
                        default: return STATE_MAIN_MENU;
                    }
                }
                break;
        }
    }
};

MenuState list_playlist(screen_t *scr, context_t *ctx) {
    int width = getmaxx(scr->right_win) - scr->pad_x * 4;
    int curr_sel = ctx->playlist_payload == -1 ? 0 : ctx->playlist_payload;
    int number_w = 3;
    int name_w = 25;
    int songs_w = 10;
    int trk_y = scr->pad_y, trk_x = scr->pad_x;
    redraw_frm(scr->right_win);
    keypad(scr->right_win, true);

    // render nav keys
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    // escape key
    wattron(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(scr->right_win, trk_y, trk_x, "[ESC]");
    wattroff(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    wattron(scr->right_win, COLOR_PAIR(2));
    mvwprintw(scr->right_win, trk_y, trk_x + 5, " Back to Menu");
    wattroff(scr->right_win, COLOR_PAIR(2));
    // rename key
    wattron(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(scr->right_win, trk_y, trk_x + 20, "[^R]");
    wattroff(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    wattron(scr->right_win, COLOR_PAIR(2));
    mvwprintw(scr->right_win, trk_y, trk_x + 25, " Rename Playlist");
    wattroff(scr->right_win, COLOR_PAIR(2));
    // delete key
    wattron(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(scr->right_win, trk_y, trk_x + 45, "[^D]");
    wattroff(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    wattron(scr->right_win, COLOR_PAIR(2));
    mvwprintw(scr->right_win, trk_y, trk_x + 50, " Delete Playlist");
    wattroff(scr->right_win, COLOR_PAIR(2));
    // enter key
    wattron(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(scr->right_win, trk_y, trk_x + 68, "[ENTER]");
    wattroff(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    wattron(scr->right_win, COLOR_PAIR(2));
    mvwprintw(scr->right_win, trk_y++, trk_x + 76, " Show");
    wattroff(scr->right_win, COLOR_PAIR(2));
    // edit key
    wattron(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(scr->right_win, trk_y, trk_x, "[^E]");
    wattroff(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    wattron(scr->right_win, COLOR_PAIR(2));
    mvwprintw(scr->right_win, trk_y++, trk_x + 5, " Edit Playlist");
    wattroff(scr->right_win, COLOR_PAIR(2));

    mvwhline(scr->right_win, trk_y++, trk_x, '-', width);
    mvwprintw(scr->right_win, trk_y++, trk_x, "%-*s %-*s %-*s",
              number_w, "No",
              name_w, "Playlist Name",
              songs_w, "Songs");
    mvwhline(scr->right_win, trk_y++, trk_x, '-', width);

    while (1) {
        // render data
        for (int i = 0; i < ctx->playlist_coll.index; ++i) {
            char fmt_number[number_w];
            char fmt_name[name_w];
            char fmt_songs[songs_w];
            snprintf(fmt_number, sizeof(fmt_number), "%d", i + 1);
            snprintf(fmt_name, sizeof(fmt_name), "%.*s", name_w,
                ctx->playlist_coll.playlists[i].name);
            snprintf(fmt_songs, sizeof(fmt_songs), "%-*lu", songs_w,
                ctx->playlist_coll.playlists[i].index);
            if (i == curr_sel) {
                wattron(scr->right_win, A_STANDOUT);
                mvwprintw(scr->right_win, i + trk_y, trk_x,
                    "%-*s %-*s %-*s",
                    number_w, fmt_number,
                    name_w, fmt_name,
                    songs_w, fmt_songs);
                wattroff(scr->right_win, A_STANDOUT);
            } else {
                mvwprintw(scr->right_win, i + trk_y, trk_x,
                    "%-*s %-*s %-*s",
                    number_w, fmt_number,
                    name_w, fmt_name,
                    songs_w, fmt_songs);
            }
        }

        // handle key input
        int key = wgetch(scr->right_win);
        switch (key) {
            case KEY_UP: {
                if (curr_sel > 0) --curr_sel;
                break;
            }
            case KEY_DOWN: {
                if (curr_sel < (int)ctx->playlist_coll.index - 1) ++curr_sel;
                break;
            }
            case KEY_ESCAPE: return STATE_PLAYLIST_MENU;
            default:
                // disabling all keys when there is no valid playlist selected
                ctx->playlist_payload = curr_sel;
                if (ctx->playlist_coll.playlists[curr_sel].name != NULL) {
                    if (key == KEY_RENAME) return STATE_RENAME_PLAYLIST;
                    if (key == KEY_RETURN) return STATE_GET_PLAYLIST;
                    if (key == KEY_EDIT) return STATE_EDIT_PLAYLIST;
                    if (key == KEY_DELETE) return STATE_DELETE_PLAYLIST;
                }
        }
    }
};

MenuState get_playlist(screen_t *scr, context_t *ctx) {
    int width = getmaxx(scr->right_win) - 4;
    int curr_sel = 0;
    int payload = ctx->playlist_payload;
    int trk_y = scr->pad_y, trk_x = scr->pad_x;
    int number_w = 3;
    int title_w = 35;
    int artist_w = 20;
    int year_w = 6;
    int duration_w = 8;
    keypad(scr->right_win, true);
    redraw_frm(scr->right_win);

    // render escape key
    wattron(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(scr->right_win, trk_y, trk_x, "[ESC]");
    wattroff(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    wattron(scr->right_win, COLOR_PAIR(2));
    mvwprintw(scr->right_win, trk_y++, trk_x + 5, " Back to Menu");
    wattroff(scr->right_win, COLOR_PAIR(2));

    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    // render playlist title bar
    char name_fmt[width + 1];
    wattron(scr->right_win, A_STANDOUT);
    int name_len = (int)strlen(ctx->playlist_coll.playlists[payload].name);
    int padding = (width - name_len) / 2;
    if (padding < 0) padding = 0;
    snprintf(name_fmt, sizeof(name_fmt), "%*s%-*s", padding, "", width - padding,
             ctx->playlist_coll.playlists[payload].name);
    mvwprintw(scr->right_win, trk_y++, trk_x, "%s", name_fmt);
    wattroff(scr->right_win, A_STANDOUT);
    // render table header
    mvwhline(scr->right_win, trk_y++, trk_x, '-', width);
    mvwprintw(scr->right_win, trk_y++, trk_x, "%-*s %-*s %-*s %-*s %-*s",
              number_w, "No",
              title_w, "Title",
              artist_w, "Artist",
              year_w, "Year",
              duration_w, "Duration");
    mvwhline(scr->right_win, trk_y++, trk_x, '-', width);

    while (1) {
        for (int i = 0; i < ctx->playlist_coll.playlists[payload].index; ++i) {
            char number_fmt[number_w];
            char title_fmt[title_w];
            char artist_fmt[artist_w];
            char year_fmt[year_w];
            char dur_fmt[duration_w];
            snprintf(number_fmt, sizeof(number_fmt), "%d", i + 1);
            snprintf(title_fmt, sizeof(title_fmt), "%.*s", title_w,
                ctx->playlist_coll.playlists[payload].songs[i].title);
            snprintf(artist_fmt, sizeof(artist_fmt), "%.*s", artist_w,
                ctx->playlist_coll.playlists[payload].songs[i].artist);
            snprintf(year_fmt, sizeof(year_fmt), "%d",
                ctx->playlist_coll.playlists[payload].songs[i].year);
            snprintf(dur_fmt, sizeof(dur_fmt), "%02d:%02d",
                     ctx->playlist_coll.playlists[payload].songs[i].duration.mnts,
                     ctx->playlist_coll.playlists[payload].songs[i].duration.secs);
            if (i == curr_sel) {
                wattron(scr->right_win, A_STANDOUT);
                mvwprintw(scr->right_win, i + trk_y, trk_x,
                    "%-*s %-*s %-*s %-*s %-*s",
                    number_w, number_fmt,
                    title_w, title_fmt,
                    artist_w, artist_fmt,
                    year_w, year_fmt,
                    duration_w, dur_fmt);
                wattroff(scr->right_win, A_STANDOUT);
            } else {
                mvwprintw(scr->right_win, i + trk_y, trk_x,
                    "%-*s %-*s %-*s %-*s %-*s",
                    number_w, number_fmt,
                    title_w, title_fmt,
                    artist_w, artist_fmt,
                    year_w, year_fmt,
                    duration_w, dur_fmt);
            }
        }

        // handle key input
        int key = wgetch(scr->right_win);
        switch (key) {
            case KEY_UP: {
                if (curr_sel > 0) --curr_sel;
                break;
            };
            case KEY_DOWN: {
                if (curr_sel < ctx->playlist_coll.playlists[payload].index - 1) ++curr_sel;
                break;
            }
            default:
                if (key == KEY_ESCAPE) return STATE_LIST_PLAYLIST;
                break;
        }

    }
};

MenuState create_playlist(screen_t *scr, context_t *ctx) {
    int *index = (int*) &ctx->playlist_coll.index;
    int *size = (int*) &ctx->playlist_coll.size;
    char buff[86] = {0};

    // memory allocation for playlist insert operation
    if (*index == ctx->playlist_coll.size) {
        *size *= 2;
        playlist_t *temp = realloc(ctx->playlist_coll.playlists, *size * sizeof(playlist_t));
        memcheck(temp);
        ctx->playlist_coll.playlists = temp;
    }

    // render input box for playlist name
    wget_in(scr->right_win, true, getmaxy(stdscr) / 2 - 5,
        (getmaxx(stdscr) - (int)sizeof(buff)) / 2 , "Enter playlist name: ",
        buff, sizeof(buff));
    ctx->playlist_coll.playlists[*index] = (playlist_t){
        .name = strdup(buff),
        .index = 0,
        .size = 1,
        .songs = malloc(sizeof(song_t)),
    };
    ++(*index);
    redraw_left_win(scr, ctx);
    return STATE_LIST_PLAYLIST;
};

MenuState rename_playlist(screen_t *scr, context_t *ctx) {
    char buff[86];
    int width = (int)sizeof(buff) + scr->pad_x, height = 3;

    // render dialog input
    WINDOW* dialog = newwin(height, width, (getmaxy(stdscr) - height) / 2, (getmaxx(stdscr) - width) / 2);
    keypad(dialog, true);
    wget_in(dialog, true, (getmaxy(stdscr) - height) / 2 - height,
        (getmaxx(stdscr) - width) / 2, "Enter new playlist name:", buff, sizeof(buff));
    ctx->playlist_coll.playlists[ctx->playlist_payload].name = strdup(buff);
    redraw_left_win(scr, ctx);
    return STATE_LIST_PLAYLIST;
}

MenuState edit_playlist(screen_t *scr, context_t *ctx) {
    int trk_y = scr->pad_y, trk_x = scr->pad_x;
    int curr_sel = ctx->playlist_payload == -1 ? 0 : ctx->playlist_payload ;
    const int cnt_song = (int) ctx->song_list.index;
    int *sel_ids = calloc(cnt_song, sizeof(int));
    memcheck(sel_ids);

    redraw_frm(scr->right_win);
    keypad(scr->right_win, true);
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    int width = getmaxx(scr->right_win) - 2 * scr->pad_x;

    // render nav keys
    // esc key
    wattron(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(scr->right_win, trk_y, trk_x, "[ESC]");
    wattroff(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    wattron(scr->right_win, COLOR_PAIR(2));
    mvwprintw(scr->right_win, trk_y, trk_x + 5, " Back to Menu");
    wattroff(scr->right_win, COLOR_PAIR(2));
    // space key
    wattron(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(scr->right_win, trk_y, trk_x + 20, "[SPACE]");
    wattroff(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    wattron(scr->right_win, COLOR_PAIR(2));
    mvwprintw(scr->right_win, trk_y++, trk_x + 27, " Select Song");
    wattroff(scr->right_win, COLOR_PAIR(2));

    mvwhline(scr->right_win, trk_y++, trk_x, '-', width);
    char *win_text = "Select Songs from List";
    int name_len = (int)strlen(win_text);
    int padding = (width - name_len) / 2;
    if (padding < 0) padding = 0;
    char text_fmt[width + 1];
    snprintf(text_fmt, sizeof(text_fmt), "%*s%-*s", padding, "", width - padding,
             win_text);
    mvwprintw(scr->right_win, trk_y++, trk_x, "%s", text_fmt);
    mvwhline(scr->right_win, trk_y++, trk_x, '-', width);

    // marking previous songs in playlist
    for (int i = 0; i < cnt_song; ++i) {
        for (int j = 0; j < ctx->playlist_coll.playlists[ctx->playlist_payload].index; ++j) {
            if (strcmp(ctx->playlist_coll.playlists[ctx->playlist_payload].songs[j].title,
                   ctx->song_list.songs[i].title) == 0 &&
            strcmp(ctx->playlist_coll.playlists[ctx->playlist_payload].songs[j].artist,
                   ctx->song_list.songs[i].artist) == 0 &&
            ctx->playlist_coll.playlists[ctx->playlist_payload].songs[j].duration.mnts ==
                ctx->song_list.songs[i].duration.mnts &&
            ctx->playlist_coll.playlists[ctx->playlist_payload].songs[j].duration.secs ==
                ctx->song_list.songs[i].duration.secs) {
                sel_ids[i] = 1;
                break;
            }
        }
    }

    while (1) {
        // render songs list
        for (size_t i = 0; i < cnt_song; ++i) {
            if (i == curr_sel) {
                wattron(scr->right_win, A_STANDOUT);
            }
            mvwprintw(scr->right_win, (int) i + trk_y, trk_x, "[%c] %s - %s",
                      sel_ids[i] ? '*' : ' ',
                      ctx->song_list.songs[i].title,
                      ctx->song_list.songs[i].artist);
            if (i == curr_sel) {
                wattroff(scr->right_win, A_STANDOUT);
            }
        }

        // handle input key
        int key = wgetch(scr->right_win);
        switch (key) {
            case KEY_UP:
                if (curr_sel > 0) ctx->song_payload = --curr_sel;
                break;
            case KEY_DOWN:
                if (curr_sel < ctx->song_list.index - 1) ctx->song_payload = ++curr_sel;
                break;
            case KEY_SPACE:
                sel_ids[curr_sel] = !sel_ids[curr_sel];
                break;
            case KEY_ESCAPE:
                free(sel_ids);
                return STATE_LIST_PLAYLIST;
            default:
                if (key == KEY_RETURN) {
                    // count selected songs total
                    size_t cnt_sel_song = 0;
                    playlist_t *p_playlist = &ctx->playlist_coll.playlists[ctx->playlist_payload];
                    for (int i = 0; i < cnt_song; ++i) {
                        if (sel_ids[i]) ++cnt_sel_song;
                    }

                    // resetting all songs previously inserted
                    for (int i = 0; i < p_playlist->index; ++i) {
                        p_playlist->songs[i] = (song_t){0};
                    }

                    // resetting index to 0 if empty playlist happened
                    if (p_playlist->index != cnt_sel_song) {
                        p_playlist->index = 0;
                    }

                    // perform insert operation if there is at least a selected song
                    if (cnt_sel_song > 0) {
                        p_playlist->index = 0;
                        if (p_playlist->size < cnt_sel_song + p_playlist->index) {
                            p_playlist->size = cnt_sel_song + 1;
                            song_t *tmp = realloc(p_playlist->songs, (p_playlist->size) * sizeof(song_t));
                            memcheck(tmp);
                            p_playlist->songs = tmp;
                        }

                        // insert selected songs
                        size_t j = p_playlist->index;
                        for (int i = 0; i < cnt_song; ++i) {
                            if (sel_ids[i]) {
                                p_playlist->songs[j++] = ctx->song_list.songs[i];
                                ++(p_playlist->index);
                            }
                        }

                    }
                    free(sel_ids);
                    return STATE_LIST_PLAYLIST;
                }

        }
    }
}

MenuState delete_playlist(screen_t *scr, context_t *ctx) {
    char *prompt = "are you sure want to remove this item?";
    const int prom_len = (int)strlen(prompt);
    char* opts[] = {"no", "yes"};
    int opts_len = sizeof(opts) / sizeof(char *);
    int curr_sel = 0;
    int width = prom_len * 3 / 2, height = 6;
    int trk_x = (width - (5 + width / 4)) / 2;

    // rendering dialog box
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    WINDOW* dialog = newwin(height, width, (getmaxy(stdscr) - height) / 2, (getmaxx(stdscr) - width) / 2);
    keypad(dialog, true);
    wbkgd(dialog, COLOR_PAIR(1));
    wattron(dialog, A_BOLD);
    box(dialog, 0, 0);
    mvwprintw(dialog, 2, (width - prom_len) / 2, "%s", prompt);

    while (1) {
        // track cursor x position
        int cur_x = trk_x;
        for (int i = 0; i < opts_len; i++) {
            if (curr_sel == i) {
                wattron(dialog, COLOR_PAIR(2));
                mvwprintw(dialog, 4, cur_x, "%s", opts[i]);
                wattroff(dialog, COLOR_PAIR(2));
            } else {
                mvwprintw(dialog, 4, cur_x, "%s", opts[i]);
            }
            cur_x += width / 4;
        }

        // handle input key
        int key = wgetch(dialog);
        switch (key) {
            case KEY_LEFT: {
                --curr_sel;
                if (curr_sel > 0) ++curr_sel;
                break;
            }
            case KEY_RIGHT: {
                ++curr_sel;
                if (curr_sel < opts_len - 1) --curr_sel;
                break;
            }
            default: {
                if (key == KEY_RETURN) {
                    if (curr_sel == 1) {
                        // perform playlist delete operation
                        int to_del = ctx->playlist_payload;
                        playlist_coll_t *p_playlist_coll = &ctx->playlist_coll;
                        if (to_del >= 0 && to_del < p_playlist_coll->index) {
                            // freeing previously allocated mem for old songs
                            free(p_playlist_coll->playlists[to_del].songs);
                            // shift songs to left by one
                            for (size_t i = to_del; i < p_playlist_coll->index - 1; ++i) {
                                if (to_del < p_playlist_coll->index - 1) {
                                    p_playlist_coll->playlists[i] = p_playlist_coll->playlists[i + 1];
                                }
                            }
                            // reset the last index song to be uninitialized
                            if (to_del == p_playlist_coll->index - 1) ctx->playlist_payload--;
                            if (p_playlist_coll->index > 0) {
                                p_playlist_coll->playlists[--p_playlist_coll->index] = (playlist_t) {0};
                            }
                        }
                    }
                    delwin(dialog);
                    redraw_left_win(scr, ctx);
                    return STATE_LIST_PLAYLIST;
                }
                break;

            }
        }
    }
}
