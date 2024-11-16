#pragma once

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>

typedef struct TestMenuFilm TestMenuFilm;

TestMenuFilm* test_menu_film_alloc();
void test_menu_film_free(TestMenuFilm* app); 