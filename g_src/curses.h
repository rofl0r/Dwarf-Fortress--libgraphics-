extern "C" {
#include "GL/glew.h"
#ifdef unix
# include <ncurses.h>
# undef COLOR_BLUE
# undef COLOR_CYAN
# undef COLOR_RED
# undef COLOR_YELLOW
#endif
}