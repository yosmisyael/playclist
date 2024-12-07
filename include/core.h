//
// Created by yosev on 12/7/24.
//

#ifndef CORE_H
#define CORE_H

#include "type.h"

typedef MenuState (*MenuHandler)(screen_t *scr, context_t *ctx);
MenuState main_menu(screen_t *scr, context_t *ctx);

extern MenuHandler menu_handlers[];

#endif //CORE_H
