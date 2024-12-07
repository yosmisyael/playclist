#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "type.h"

#define KEY_DELETE  4
#define KEY_EDIT    5
#define KEY_SEARCH  6
#define KEY_RETURN  10
#define KEY_RENAME  18
#define KEY_ESCAPE  27
#define KEY_SPACE   32

// utils func
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
        // {"Eleanor Rigby", "The Beatles", 1966, 2, 8},
        {"While My Guitar Gently Weeps", "The Beatles", 1968, 4, 45},
        {"Master of Puppets", "Metallica", 1986, 8, 36},
        // {"One", "Metallica", 1988, 7, 27},
        {"Nothing Else Matters", "Metallica", 1991, 6, 28},
        {"Enter Sandman", "Metallica", 1991, 5, 32},
        {"Seek & Destroy", "Metallica", 1983, 6, 55},
        {"Stairway to Heaven", "Led Zeppelin", 1971, 8, 2},
        // {"Kashmir", "Led Zeppelin", 1975, 8, 37},
        // {"Whole Lotta Love", "Led Zeppelin", 1969, 5, 34},
        {"Smoke on the Water", "Deep Purple", 1972, 5, 40},
        {"Highway Star", "Deep Purple", 1972, 6, 5},
        {"Bohemian Rhapsody", "Queen", 1975, 5, 55},
        {"Shine On You Crazy Diamond", "Pink Floyd", 1975, 13, 31},
        // {"Smells Like Teen Spirit", "Nirvana", 1991, 5, 1},
        {"Hotel California", "Eagles", 1976, 6, 25},
        // {"Time in a Bottle", "Jim Croce", 1972, 2, 27},
        {"Lemon", "Kenshi Yonezu", 2020, 4, 15},
        {"Live My Life", "aespa", 2024, 2, 41},
        {"Over the Moon", "TXT", 2024, 2, 37},
        {"Imagine", "John Lennon", 1971, 3, 4},
        {"Another Brick in the Wall", "Pink Floyd", 1979, 5, 20},
        // {"Like a Rolling Stone", "Bob Dylan", 1965, 6, 13},
        {"Hotel California", "Eagles", 1976, 6, 25},
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

void init_playlist(playlist_t *playlist, const char *name) {
    playlist->name = strdup(name);
    playlist->index = 0;
    playlist->size = 1;
    playlist->songs = NULL;
}

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

MenuState main_menu(screen_t *scr, context_t *ctx);
MenuState playlist_menu(screen_t *scr, context_t *ctx);
MenuState song_menu(screen_t *scr, context_t *ctx);
MenuState add_song(screen_t *scr, context_t *ctx);
MenuState list_song(screen_t *scr, context_t *ctx);
MenuState list_playlist(screen_t *scr, context_t *ctx);
MenuState get_playlist(screen_t *scr, context_t *ctx);
MenuState search_song(screen_t *scr, context_t *ctx);
MenuState delete_song(screen_t *scr, context_t *ctx);
MenuState create_playlist(screen_t *scr, context_t *ctx);
MenuState rename_playlist(screen_t *scr, context_t *ctx);
MenuState edit_playlist(screen_t *scr, context_t *ctx);
MenuState delete_playlist(screen_t *scr, context_t *ctx);

typedef MenuState (*MenuHandler)(screen_t *scr, context_t *ctx);

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