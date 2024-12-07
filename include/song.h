//
// Created by yosev on 11/26/24.
//

#ifndef SONG_H
#define SONG_H

#include "type.h"

MenuState song_menu(screen_t *scr, context_t *ctx);
MenuState add_song(screen_t *scr, context_t *ctx);
MenuState list_song(screen_t *scr, context_t *ctx);
MenuState search_song(screen_t *scr, context_t *ctx);
MenuState delete_song(screen_t *scr, context_t *ctx);

#endif //SONG_H
