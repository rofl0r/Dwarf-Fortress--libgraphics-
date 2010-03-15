// Map DF's CP437 to Unicode
// see: http://dwarffortresswiki.net/index.php/Character_table
static int charmap[256] = {
  ' ', 0x263A, 0x263B, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022,
  0x25D8, 0x25CB, 0x25D9, 0x2642, 0x2640, 0x266A, 0x266B, 0x263C,
  0x25BA, 0x25C4, 0x2195, 0x203C, 0x00B6, 0x00A7, 0x25AC, 0x21A8,
  0x2191, 0x2193, 0x2192, 0x2190, 0x221F, 0x2194, 0x25B2, 0x25BC,
  /* 0x20 */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x2302,
  /* 0x80 */
  0xC7, 0xFC, 0xE9, 0xE2, 0xE4, 0xE0, 0xE5, 0xE7,
  0xEA, 0xEB, 0xE8, 0xEF, 0xEE, 0xEC, 0xC4, 0xC5,
  0xC9, 0xE6, 0xC6, 0xF4, 0xF6, 0xF2, 0xFB, 0xF9,
  0xFF, 0xD6, 0xDC, 0xA2, 0xA3, 0xA5, 0x20A7, 0x192,
  0xE1, 0xED, 0xF3, 0xFA, 0xF1, 0xD1, 0xAA, 0xBA,
  0xBF, 0x2310, 0xAC, 0xBD, 0xBC, 0xA1, 0xAB, 0xBB,
  0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
  0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
  0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
  0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
  0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
  0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
  0x3B1, 0xDF/*yay*/, 0x393, 0x3C0, 0x3A3, 0x3C3, 0xB5, 0x3C4,
  0x3A6, 0x398, 0x3A9, 0x3B4, 0x221E, 0x3C6, 0x35B, 0x2229,
  0x2261, 0xB1, 0x2265, 0x2264, 0x2320, 0x2321, 0xF7, 0x2248,
  0xB0, 0x2219, 0xB7, 0x221A, 0x207F, 0xB2, 0x25A0, 0xA0
};


class renderer_curses : public renderer {
  std::map<std::pair<int,int>,int> color_pairs; // For curses. MOVEME.
  
  // Map from DF color to ncurses color
  static int ncurses_map_color(int color) {
    if (color < 0) abort();
    switch (color) {
    case 0: return 0;
    case 1: return 4;
    case 2: return 2;
    case 3: return 6;
    case 4: return 1;
    case 5: return 5;
    case 6: return 3;
    case 7: return 7;
    default: return ncurses_map_color(color - 7);
    }
  }

  // Look up, or create, a curses color pair
  int lookup_pair(pair<int,int> color) {
    map<pair<int,int>,int>::iterator it = color_pairs.find(color);
    if (it != color_pairs.end()) return it->second;
    // We don't already have it. Generate a new pair if possible.
    if (color_pairs.size() < COLOR_PAIRS - 1) {
      const short pair = color_pairs.size() + 1;
      init_pair(pair, ncurses_map_color(color.first), ncurses_map_color(color.second));
      color_pairs[color] = pair;
      return pair;
    }
    // We don't have it, and there's no space for more. Panic!
    endwin();
    puts("Ran out of space for color pairs! Ask Baughn to implement a fallback!");
    exit(EXIT_FAILURE);
  }

public:

  void update_tile(int x, int y) {
    const int ch   = gps.screen[x*gps.dimy*4 + y*4 + 0];
    const int fg   = gps.screen[x*gps.dimy*4 + y*4 + 1];
    const int bg   = gps.screen[x*gps.dimy*4 + y*4 + 2];
    const int bold = gps.screen[x*gps.dimy*4 + y*4 + 3];

    const string utf8 = encode_utf8(charmap[ch] ? charmap[ch] : ch);
    const int pair    = lookup_pair(make_pair<int,int>(fg,bg));
    attron(COLOR_PAIR(pair));
    if (bold) attron(A_BOLD);
    mvprintw(y, x, "%s", utf8.c_str());
    if (bold) attroff(A_BOLD);
  }

  void update_all() {
    for (int x = 0; x < init.display.grid_x; x++)
      for (int y = 0; y < init.display.grid_y; y++)
        update_tile(x, y);
  }

  void render() {
    refresh();
  }

  renderer_curses() {
    // Initialize curses
    initscr();
    raw();
    noecho();
    keypad(stdscr, true);
    nodelay(stdscr, true);
    start_color();
    curs_set(0);
  }
};

// Reads from getch, collapsing utf-8 encoding to the actual unicode
// character.  Ncurses symbols (left arrow, etc.) are returned as
// positive values, unicode as negative. Error returns 0.
static int getch_utf8() {
  int byte = getch();
  if (byte == ERR) return 0;
  if (byte > 0xff) return byte;
  int len = decode_utf8_predict_length(byte);
  if (!len) return 0;
  string input(len,0); input[0] = byte;
  for (int i = 1; i < len; i++) input[i] = getch();
  return -decode_utf8(input);
}

void enablerst::eventLoop_ncurses() {
  int x, y, oldx = 0, oldy = 0;

  while (loopvar) {
    // Check for terminal resize
    getmaxyx(stdscr, y, x);
    if (y != oldy || x != oldx) {
      gps.allocate(x, y);
      erase();
      oldx = x; oldy = y;
    }
    
    // Deal with input
    Uint32 now = SDL_GetTicks();
    // Read keyboard input, if any, and transform to artificial SDL
    // events for enabler_input.
    int key;
    while ((key = getch_utf8())) {
      bool esc = false;
      if (key == -27) { // esc
        int second = getch_utf8();
        if (second) { // That was an escape sequence
          esc = true;
          key = second;
        }
      }
      add_input_ncurses(key, now, esc);
    }

    // Run the game logic
    do_frame();
  }
}