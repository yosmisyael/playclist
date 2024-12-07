//
// Created by yosev on 12/7/24.
//

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "type.h"

MenuState playlist_menu(screen_t *scr, context_t *ctx);
MenuState list_playlist(screen_t *scr, context_t *ctx);
MenuState get_playlist(screen_t *scr, context_t *ctx);
MenuState create_playlist(screen_t *scr, context_t *ctx);
MenuState rename_playlist(screen_t *scr, context_t *ctx);
MenuState edit_playlist(screen_t *scr, context_t *ctx);
MenuState delete_playlist(screen_t *scr, context_t *ctx);

#endif //PLAYLIST_H
