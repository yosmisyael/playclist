//
// Created by yosev on 11/26/24.
//

#include "song.h"
#include "ncutil.h"
#include "type.h"
#include <stdlib.h>
#include <string.h>

MenuState song_menu(screen_t *scr, context_t *ctx)  {
    char *opts[] = {"List Song", "Add Song", "Back"};
    int opts_len = sizeof(opts) / sizeof(opts[0]);
    int sel = 0;
    int width = getmaxx(scr->right_win) - scr->pad_x * 4;
    redraw_frm(scr->right_win);
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

        // handle key input
        const int key = wgetch(scr->right_win);
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
                    switch (sel) {
                        case 0: return STATE_LIST_SONG;
                        case 1: return STATE_ADD_SONG;
                        case 2:
                        default: return STATE_MAIN_MENU;
                    }
                }
                break;
            }
        }
    }
};

MenuState add_song(screen_t *scr, context_t *ctx) {
    redraw_frm(scr->right_win);

    // memory management for insert song operation
    if (ctx->song_list.index == ctx->song_list.size - 1) {
        ctx->song_list.size *= 2;
        song_t *temp = realloc(ctx->song_list.songs, ctx->song_list.size * sizeof(song_t));
        if (!temp) {
            perror("failed to allocate memory");
            exit(EXIT_FAILURE);
        }
        ctx->song_list.songs = temp;
    }

    // get user input
    char buffer[86] = {0};

    // get title input
    wget_in(scr->right_win, false, 0, getmaxx(stdscr) / 3,
        "Enter title of the song: ", buffer, sizeof(buffer));
    ctx->song_list.songs[ctx->song_list.index].title = strdup(buffer);
    memset(buffer, 0, sizeof(buffer));

    // get artist input
    wget_in(scr->right_win, false, 0, getmaxx(stdscr) / 3,
        "Enter artist of the song: ", buffer, sizeof(buffer));
    ctx->song_list.songs[ctx->song_list.index].artist = strdup(buffer);
    memset(buffer, 0, sizeof(buffer));

    // get year input
    wget_in(scr->right_win, false, 0, getmaxx(stdscr) / 3,
        "Enter year of the song: ", buffer, sizeof(buffer));
    ctx->song_list.songs[ctx->song_list.index].year = atoi(buffer);
    memset(buffer, 0, sizeof(buffer));

    // get duration minutes input
    wget_in(scr->right_win, false, 0, getmaxx(stdscr) / 3,
        "Enter duration (minutes): ", buffer, sizeof(buffer));
    ctx->song_list.songs[ctx->song_list.index].duration.mnts = atoi(buffer);
    memset(buffer, 0, sizeof(buffer));

    // get duration seconds input
    wget_in(scr->right_win, false, 0, getmaxx(stdscr) / 3,
        "Enter duration (seconds): ", buffer, sizeof(buffer));
    ctx->song_list.songs[ctx->song_list.index].duration.secs = atoi(buffer);
    memset(buffer, 0, sizeof(buffer));

    ctx->song_list.index++;
    return STATE_SONG_MENU;
};

MenuState list_song(screen_t *scr, context_t *ctx) {
    int number_w = 3;
    int title_w = 35;
    int artist_w = 20;
    int year_w = 6;
    int duration_w = 8;
    int curr_sel = ctx->song_payload != -1 ? ctx->song_payload : 0;
    int width = getmaxx(scr->right_win) - 2 * 2;
    int trk_y = scr->pad_y, trk_x = scr->pad_x;

    redraw_frm(scr->right_win);
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    keypad(scr->right_win, true);

    // rendering control keys
    // esc key
    wattron(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(scr->right_win, trk_y, trk_x, "[ESC]");
    wattroff(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    wattron(scr->right_win, COLOR_PAIR(2));
    mvwprintw(scr->right_win, trk_y, trk_x + 6, " Back to Menu");
    wattroff(scr->right_win, COLOR_PAIR(2));
    // search key
    wattron(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(scr->right_win, trk_y, trk_x + 21, "[^F]");
    wattroff(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    wattron(scr->right_win, COLOR_PAIR(2));
    mvwprintw(scr->right_win, trk_y, trk_x + 26, " Search Song");
    wattroff(scr->right_win, COLOR_PAIR(2));
    // delete key
    wattron(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(scr->right_win, trk_y, trk_x + 41, "[^D]");
    wattroff(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    wattron(scr->right_win, COLOR_PAIR(2));
    mvwprintw(scr->right_win, trk_y++, trk_x + 46, " Delete Song");
    wattroff(scr->right_win, COLOR_PAIR(2));
    // render title bar
    mvwhline(scr->right_win, trk_y++, trk_x, '-', width);
    mvwprintw(scr->right_win, trk_y++, trk_x, "%-*s %-*s %-*s %-*s %-*s",
            number_w, "No",
            title_w, "Title",
            artist_w, "Artist",
            year_w, "Year",
            duration_w, "Duration");
    mvwhline(scr->right_win, trk_y++, trk_x, '-', width);

    while (1) {
        // render songs
        for (int i = 0; i < ctx->song_list.index; ++i) {
            // formatting songs data output
            char fmt_number[number_w];
            char fmt_title[title_w];
            char fmt_artist[artist_w];
            char fmt_year[year_w];
            char fmt_dur[duration_w];
            snprintf(fmt_number, sizeof(fmt_number), "%d", i + 1);
            snprintf(fmt_title, sizeof(fmt_title), "%.*s",
                title_w, ctx->song_list.songs[i].title);
            snprintf(fmt_artist, sizeof(fmt_artist), "%.*s",
                artist_w, ctx->song_list.songs[i].artist);
            snprintf(fmt_year, sizeof(fmt_year), "%d",
                ctx->song_list.songs[i].year);
            snprintf(fmt_dur, sizeof(fmt_dur), "%02d:%02d",
                     ctx->song_list.songs[i].duration.mnts,
                     ctx->song_list.songs[i].duration.secs);
            if (i == curr_sel) {
                wattron(scr->right_win, A_STANDOUT);
                mvwprintw(scr->right_win, i + trk_y, 2,
                    "%-*s %-*s %-*s %-*s %-*s",
                    number_w, fmt_number,
                    title_w, fmt_title,
                    artist_w, fmt_artist,
                    year_w, fmt_year,
                    duration_w, fmt_dur);
                wattroff(scr->right_win, A_STANDOUT);
            } else {
                mvwprintw(scr->right_win, i + trk_y, 2,
                    "%-*s %-*s %-*s %-*s %-*s",
                    number_w, fmt_number,
                    title_w, fmt_title,
                    artist_w, fmt_artist,
                    year_w, fmt_year,
                    duration_w, fmt_dur);
            }
        }

        // render options
        int key = wgetch(scr->right_win);
        switch (key) {
            case KEY_UP: {
                if (curr_sel > 0) ctx->song_payload = --curr_sel;
                break;
            }
            case KEY_DOWN: {
                if (curr_sel < (int)ctx->song_list.index - 1) ctx->song_payload = ++curr_sel;
                break;
            }
            case KEY_ESCAPE: {
                if (ctx->song_payload > -1) ctx->song_payload = -1;
                return STATE_SONG_MENU;
            }
            case KEY_SEARCH: {
                if (ctx->song_payload > -1) ctx->song_payload = -1;
                return STATE_SEARCH_SONG;
            }
            default: {
                // disabling delete key when no valid song is selected
                if (key == KEY_DELETE && ctx->song_list.songs[curr_sel].title != NULL) {
                    ctx->song_payload = curr_sel;
                    return STATE_DELETE_SONG;
                }
                break;
            }
        }
    }
};

MenuState search_song(screen_t *scr, context_t *ctx) {
    int trk_y = scr->pad_y;

    // render search box
    redraw_frm(scr->right_win);
    char query[86] = {0};
    wget_in(scr->right_win, false, 0, getmaxx(stdscr) / 3,
        "Enter song title or artist", query, sizeof(query));

    // render search results
    redraw_frm(scr->right_win);
    int match_found = 0;
    int width = getmaxx(scr->right_win) - 2 * scr->pad_x;
    int number_w = 3;
    int title_w = 35;
    int artist_w = 20;
    int year_w = 6;
    int duration_w = 8;

    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    // render nav button
    wattron(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(scr->right_win, trk_y, scr->pad_x, "[*]");
    wattroff(scr->right_win, COLOR_PAIR(1) | A_BOLD);
    wattron(scr->right_win, COLOR_PAIR(2));
    mvwprintw(scr->right_win, trk_y++, scr->pad_x + 5, " Back to Menu");
    wattroff(scr->right_win, COLOR_PAIR(2));
    // render title bar
    mvwprintw(scr->right_win, trk_y++, 2, "Search results for: %s", query);
    mvwhline(scr->right_win, trk_y++, scr->pad_x, '-', width);
    mvwprintw(scr->right_win, trk_y++, scr->pad_x, "%-*s %-*s %-*s %-*s %-*s",
              number_w, "No",
              title_w, "Title",
              artist_w, "Artist",
              year_w, "Year",
              duration_w, "Duration");
    mvwhline(scr->right_win, trk_y++, scr->pad_x, '-', width);
    // render search result list
    int trk_y_list = 0;
    for (size_t i = 0; i < ctx->song_list.index; ++i) {
        if (strcasestr(ctx->song_list.songs[i].title, query) != NULL ||
            strcasestr(ctx->song_list.songs[i].artist, query) != NULL) {
            char fmt_number[number_w];
            char fmt_title[title_w];
            char fmt_artist[artist_w];
            char fmt_year[year_w];
            char fmt_dur[duration_w];
            snprintf(fmt_number, sizeof(number_w), "%d", (int)i + 1);
            snprintf(fmt_title, sizeof(fmt_title), "%.*s",
                title_w, ctx->song_list.songs[i].title);
            snprintf(fmt_artist, sizeof(fmt_artist), "%.*s",
                artist_w, ctx->song_list.songs[i].artist);
            snprintf(fmt_year, sizeof(fmt_year), "%d",
                ctx->song_list.songs[i].year);
            snprintf(fmt_dur, sizeof(fmt_dur), "%02d:%02d",
                     ctx->song_list.songs[i].duration.mnts,
                     ctx->song_list.songs[i].duration.secs);
            mvwprintw(scr->right_win, trk_y_list + trk_y, scr->pad_x,
                    "%-*s %-*s %-*s %-*s %-*s",
                    number_w, fmt_number,
                    title_w, fmt_title,
                    artist_w, fmt_artist,
                    year_w, fmt_year,
                    duration_w, fmt_dur);
            ++trk_y_list;
            ++match_found;
        }
    }
    if (!match_found) {
        mvwprintw(scr->right_win, trk_y, scr->pad_x, "No matching songs found. Press any key to continue.");
    }
    wrefresh(scr->right_win);
    wgetch(scr->right_win);
    return STATE_LIST_SONG;
}

MenuState delete_song(screen_t *scr, context_t *ctx) {
    char *prompt = "are you sure want to remove this item?";
    const int prom_len = (int)strlen(prompt);
    int width = prom_len * 3 / 2, height = 6;
    int curr_sel = 0;
    char* opts[] = {"no", "yes"};
    int opts_len = sizeof(opts) / sizeof(char *);
    int trk_x = (width - (5 + width / 4)) / 2;

    // render dialog box
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
        // render dialog options
        int curr_x = trk_x;
        for (int i = 0; i < opts_len; i++) {
            if (curr_sel == i) {
                wattron(dialog, COLOR_PAIR(2));
                mvwprintw(dialog, 4, curr_x, "%s", opts[i]);
                wattroff(dialog, COLOR_PAIR(2));
            } else {
                mvwprintw(dialog, 4, curr_x, "%s", opts[i]);
            }
            curr_x += width / 4;
        }

        // handle inputs
        int key = wgetch(dialog);
        switch (key) {
            case KEY_LEFT: {
                if (curr_sel > 0) --curr_sel;
                break;
            }
            case KEY_RIGHT: {
                if (curr_sel < opts_len - 1) ++curr_sel;
                break;
            }
            default:
                if (key == KEY_RETURN) {
                    if (curr_sel == 1) {
                        int to_del = ctx->song_payload;
                        song_list_t *song_list = &ctx->song_list;
                        if (to_del >= 0 && to_del < song_list->index) {
                            for (size_t i = to_del; i < song_list->index - 1; ++i) {
                                song_list->songs[i] = song_list->songs[i + 1];
                            }
                            if (to_del == song_list->index - 1) ctx->song_payload--;
                            song_list->songs[--song_list->index] = (song_t){0};
                        }
                    }
                    delwin(dialog);
                    redraw_left_win(scr, ctx);
                    return STATE_LIST_SONG;
                }
                break;
        }
    }
}