#include <stdio.h>
#include <string.h>

/**************** Cross-platform | Start ****************/
/*
 * Cross-platform handling of:
 * - usleep (Windows needs Sleep-based workaround)
 * - non-blocking input detection (kbhit or select)
 * - single-key capture (getch on Windows, termios on Linux/macOS)
 * - hiding the cursor
 * - printing colored text
 */

#ifdef _WIN32

    #include <windows.h>
    #include <conio.h>  /* kbhit(), getch() */

    /*
     * Redefine usleep on Windows:
     * usleep(x) => Sleep(x/1000)
     * Because Sleep() is in milliseconds,
     * while usleep() is in microseconds.
     */
    #define usleep(usec) Sleep((usec)/1000)

    /* Input detection (Windows) */
    #define input_available()  kbhit()

    /* Single key press (Windows) */
    #define get_key_pressed()  getch()

    /* Hide cursor (Windows) */
    void hide_cursor(void)
    {
        HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 100;
        info.bVisible = FALSE;
        SetConsoleCursorInfo(console_handle, &info);

        return;
    }

    void show_cursor(void)
    {
        HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 100;
        info.bVisible = TRUE; /* Show cursor */
        SetConsoleCursorInfo(console_handle, &info);

        return;
    }

    #define CLEAR_SCREEN "cls"
    void moveCursorHome(void)
    {
        /* Get a handle to the standard output (your console) */
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        /* Create a COORD structure that sets the position to column=0, row=0 */
        COORD home = {0, 0};

        /* This call repositions the cursor in the console window to (0,0) */
        SetConsoleCursorPosition(hConsole, home);

        return;
    }

#else /* Linux / macOS */

    #include <unistd.h>      /* usleep() */
    #include <sys/select.h>  /* select() */
    #include <termios.h>     /* termios, tcgetattr, tcsetattr */

    /*
     * Check if input is available without blocking (Linux/macOS).
     * Returns non-zero if a key is ready.
     */
    int input_available(void)
    {
        struct timeval tv = {0, 0}; /* zero timeout => no blocking */
        fd_set fds;

        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        /*
         * select returns > 0 if input is ready in stdin.
         * STDIN_FILENO + 1 => number of file descriptors to check.
         */
        return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    }

    /*
     * Equivalent to getch() on Linux/macOS using termios.
     * Reads a single keypress without waiting for Enter,
     * and without echoing the character.
     */
    int get_key_pressed(void)
    {
        struct termios oldt, newt;
        int ch;

        /* Save current terminal settings */
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;

        /* Turn off canonical mode and echo */
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        /* Read one character */
        ch = getchar();

        /* Restore the old settings */
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

        return ch;
    }

    /* Hide cursor (Linux/macOS) using ANSI escape code */
    #define hide_cursor() printf("\x1b[?25l")
    #define show_cursor() printf("\x1b[?25h")

    #define CLEAR_SCREEN "clear"
    void moveCursorHome(void)
    {
        printf("\x1b[H"); /* Move cursor to (0,0) */

        return;
    }

#endif /* _WIN32 */

/**************** Cross-platform | End ****************/

#define ENTER 13
#define SPACE ' '
#define ESC 27
#define UP_ARROW 72
#define DOWN_ARROW 80
#define LEFT_ARROW 75
#define RIGHT_ARROW 77

typedef enum { false = 0, true = 1 } bool;

void clear_line(void)
{
    printf("%*s", 40, "");

    return;
}

/* For old times' sake */
void create_game_of_life(void);
void gamestate_print(void);
void gamestate_apply_rules(void);

void title_print(void)
{
    int play_demo = 0;

start:
    show_cursor();
    printf("%s", "\n  === Game of Life ===\n\n     Press any key to start...\r");

    for (;;)
        if (input_available())
        {
            get_key_pressed(); /* If D is pressed! */
            break;
        }
        else if (play_demo++ == 100000)
        {
            hide_cursor();
            create_game_of_life();

            moveCursorHome();
            gamestate_print(); usleep(800000);
            for (;;)
            {
                moveCursorHome();
                gamestate_print();
                gamestate_apply_rules();

                if (input_available())
                {
                    get_key_pressed();
                    system(CLEAR_SCREEN);
                    play_demo = 0;
                    goto start;
                }

                usleep(100000);
            }
            break;
        }

    hide_cursor();
    clear_line();

    return;
}

/**************** Menu | Start ****************/

#define NUM_ITEMS 4
const char* options[NUM_ITEMS] = {"Play", "DEMO", "Manual", "Exit"};

void menu_print(int selected, char animated_char, const char* options_local[], int num_local)
{
    char buffer[100]; /* Temporary buffer for building the menu */
    int offset = 0;
    int i;

    /* Move cursor to the start of the line and overwrite */
    offset += sprintf(buffer + offset, "\r ");
    for (i = 1; i <= num_local; i++)
    {
        if (i == selected)
            offset += sprintf(buffer + offset, "%c> %s ", animated_char, options_local[i - 1]);
        else
            offset += sprintf(buffer + offset, "%d) %s ", i, options_local[i - 1]);
    }
    printf("%s", buffer);

    return;
}

int menu_loop(const char* options_local[], int num_local)
{
    char animated_char = '-';
    int selected = 1;
    char input;

    menu_print(selected, animated_char, options_local, num_local);
    for (;;)
    {
        /* Flip char */
        animated_char = (animated_char == '-') ? '=' : '-';
        menu_print(selected, animated_char, options_local, num_local);

        if (input_available())
        {
            /* Read the input */
            input = get_key_pressed();

            /* Update selected accordingly */
            if (input == 'd' || input == 'D' || input == RIGHT_ARROW) selected++;
            if (input == 'a' || input == 'A' || input == LEFT_ARROW) selected--;
            if (input == ENTER) break;

            /* Clamp the selection between 1 and NUM_ITEMS */
            if (selected < 1)              selected = 1;
            else if (selected > num_local) selected = num_local;
        }

        usleep(50000);
    }
    printf("%s", "\r");

    return selected;
}

/**************** Menu | End ****************/

/**************** GameState | Start ****************/

#define ROWS 28
#define COLS 55
char gamestate[ROWS][COLS];

void gamestate_initialize(char gamestate_local[ROWS][COLS])
{
    memset(gamestate_local, 0, sizeof(gamestate));

    return;
}

/**************** Patterns | Start ****************/

void place_pattern(int x, int y, char gamestate_local[ROWS][COLS],
                   int pattern_rows, int pattern_cols, int pattern[])
{
    int i, j;
    for (i = 0; i < pattern_rows; i++)
        for (j = 0; j < pattern_cols; j++)
            /* Ensure within bounds! */
            if (((x + i) >= 0 && (x + i) < ROWS
             && (y + j) >= 0 && (y + j) < COLS)
             && pattern[i * pattern_cols + j] == 1)
                /* Compute 2D index manually! */
                gamestate_local[x + i][y + j] = pattern[i * pattern_cols + j];

    return;
}

void create_game_of_life(void)
{
    int pattern[15][50] =
    {
        {0,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,1,0,0,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0},
        {1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0},
        {1,0,0,1,1,1,1,0,1,0,0,0,0,0,1,0,1,0,0,1,0,0,1,0,1,1,1,1,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1,1,1,1,1,0,0},
        {1,0,0,0,0,0,1,0,1,1,1,1,1,1,1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0},
        {1,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0},
        {0,1,1,1,1,1,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,0,1,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,0,0,0,0,0,0,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,0,0,0,0,0,0,0,0,1,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,1,0,1,1,1,0,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    };
    gamestate_initialize(gamestate);
    place_pattern(6, 2, gamestate, 15, 50, (int*) pattern);

    return;
}

void create_pulsar(int x, int y, char gamestate_local[ROWS][COLS])
{
    int pattern[13][13] =
    {
        {0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1},
        {0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0},
        {1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0}
    };
    place_pattern(x, y, gamestate_local, 13, 13, (int*) pattern);

    return;
}

void create_penta_decathlon(int x, int y, char gamestate_local[ROWS][COLS])
{
    int pattern[10][9] =
    {
        {0, 0, 0, 1, 1, 1, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 1, 1, 1, 0, 0, 0}
    };
    place_pattern(x, y, gamestate_local, 10, 9, (int*) pattern);

    return;
}

void create_lwss(int x, int y, char gamestate_local[ROWS][COLS])
{
    int pattern[4][5] =
    {
        {0, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
        {1, 0, 0, 1, 0}
    };
    place_pattern(x, y, gamestate_local, 4, 5, (int*) pattern);

    return;
}

void create_beacon(int x, int y, char gamestate_local[ROWS][COLS])
{
    int pattern[4][4] =
    {
        {1, 1, 0, 0},
        {1, 1, 0, 0},
        {0, 0, 1, 1},
        {0, 0, 1, 1}
    };
    place_pattern(x, y, gamestate_local, 4, 4, (int*) pattern);

    return;
}

void create_boat(int x, int y, char gamestate_local[ROWS][COLS])
{
    int pattern[3][3] =
    {
        {1, 1, 0},
        {1, 0, 1},
        {0, 1, 0}
    };
    place_pattern(x, y, gamestate_local, 3, 3, (int*) pattern);

    return;
}

void create_R_pentomino(int x, int y, char gamestate_local[ROWS][COLS])
{
    int pattern[3][3] =
    {
        {0, 1, 1},
        {1, 1, 0},
        {0, 1, 0}
    };
    place_pattern(x, y, gamestate_local, 3, 3, (int*) pattern);

    return;
}

/**************** Patterns | End ****************/

void gamestate_print(void)
{
    /* Tiles + (20 * \n) + Termination character + Safe*/
    char buffer[(ROWS * COLS * 2) + ROWS + 2];
    int offset = 0;
    int alive = 0;

    int i, j;
    for (i = 0; i < ROWS; i++)
    {
        for (j = 0; j < COLS; j++)
        {
            if (gamestate[i][j] == 1)
            {
                offset += sprintf(buffer + offset, "[]");
                alive++;
            }
            else
                offset += sprintf(buffer + offset, "  ");
        }
        offset += sprintf(buffer + offset, "\n");
    }
    sprintf(buffer + offset, "%s%4d", "Population = ", alive);
    /* Extra spaces clear old digits when the population decreases! */
    fwrite(buffer, 1, strlen(buffer), stdout); /* faster output */

    return;
}

char gamestate_temp[ROWS][COLS];

#define NEIGHBOURS_POS 8
const int neighbours[NEIGHBOURS_POS][2] = {
    {-1, -1}, {-1, 0}, {-1, 1},
    { 0, -1}, /* ME */ { 0, 1},
    { 1, -1}, { 1, 0}, { 1, 1}
};

void gamestate_apply_rules(void)
{
    int i, j;
    for (i = 0; i < ROWS; i++)
    {
        for (j = 0; j < COLS; j++)
        {
            int neighbours_sum = 0;

            int k;
            for (k = 0; k < NEIGHBOURS_POS; k++)
            {
                int row = i + neighbours[k][0];
                int col = j + neighbours[k][1];

                {   /* Make the gamestate a ball! */
                    if (row == -1)
                        row = ROWS - 1;
                    else if (row == ROWS)
                        row = 0;

                    if (col == -1)
                        col = COLS - 1;
                    else if (col == COLS)
                        col = 0;
                }

                neighbours_sum += gamestate[row][col];
            }
            /*
             * 1) Any live cell with fewer than two live neighbours dies,
                  as if by underpopulation.
             * 2) Any live cell with two or three live neighbours
                  lives on to the next generation.
             * 3) Any live cell with more than three live neighbours dies,
                  as if by overpopulation.
             * 4) Any dead cell with exactly three live neighbours
                  becomes a live cell, as if by reproduction.
             */
            if (neighbours_sum < 2 || neighbours_sum > 3)
                gamestate_temp[i][j] = 0;
            else if (neighbours_sum == 3)
                gamestate_temp[i][j] = 1;
            else
                gamestate_temp[i][j] = gamestate[i][j];
        }
    }
    memcpy(gamestate, gamestate_temp, sizeof(gamestate));

    return;
}

void gamestate_demo(void)
{
    gamestate_initialize(gamestate);

    create_pulsar(2, 2, gamestate);
    create_beacon(4, 22, gamestate);
    create_boat(10, 23, gamestate);
    create_penta_decathlon(4, 30, gamestate);
    create_lwss(20, 15, gamestate);

    return;
}

void add_gamestates(void)
{
    /* Add gamestate and gamestate_temp, *
     * return result in gamestate        */
    int i, j;
    for (i = 0; i < ROWS; i++)
        for (j = 0; j < COLS; j++)
            gamestate[i][j] = gamestate[i][j] | gamestate_temp[i][j];
            /* Bitwise OR on ASCII values!           *
             * '1' | '1' = '1', '1' | '0' = '1', ... */

    return;
}

/**************** GameState | End ****************/

/**************** GamePlay | Start ****************/

#define NUM_PATTERNS 8
const char* patterns[NUM_PATTERNS] =
{
    "Start", "Pulsar", "Penta-decathlon",
    "LWSS", "Beacon", "Boat", "R-pentomino", "Set cell"
};

void gameplay_select(void)
{
    void (*add_patterns[NUM_PATTERNS])(int, int, char [ROWS][COLS]) =
    {
        NULL, create_pulsar, create_penta_decathlon,
        create_lwss, create_beacon, create_boat,
        create_R_pentomino, NULL
    };
    char gamestate_copy[ROWS][COLS];
    gamestate_initialize(gamestate_copy);

    gamestate_initialize(gamestate);
    gamestate_initialize(gamestate_temp);
    system(CLEAR_SCREEN);

    for (;;)
    {
        int selected = menu_loop(patterns, NUM_PATTERNS) - 1;
        int input, x = 0, y = 0;
        system(CLEAR_SCREEN);

        if (selected == 0) /* Run State! */
            return;
        else if (selected == 7) /* Set cell */
            gamestate[x][y] = 1;
        else if (add_patterns[selected] != NULL) /* Set pattern */
            add_patterns[selected](x, y, gamestate);

        for (;;)
        {
            moveCursorHome();
            gamestate_print();

            if (input_available())
            {
                input = get_key_pressed();
                if (input == 'd' || input == 'D' || input == RIGHT_ARROW)
                    if (y < COLS)
                        y++;
                if (input == 'a' || input == 'A' || input == LEFT_ARROW)
                    if (y > 0)
                        y--;
                if (input == 'w' || input == 'W' || input == UP_ARROW)
                    if (x > 0)
                        x--;
                if (input == 's' || input == 'S' || input == DOWN_ARROW)
                    if (x < ROWS)
                        x++;
                if (input == ENTER) break;

                if (selected == 7) /* Set cell */
                {
                    gamestate_initialize(gamestate);
                    add_gamestates();
                    gamestate[x][y] = 1;
                    if (input == SPACE)
                        gamestate_temp[x][y] = (gamestate_temp[x][y] == 1) ? 0 : 1;
                }
                else if (add_patterns[selected] != NULL) /* Set pattern */
                {
                    gamestate_initialize(gamestate);
                    add_gamestates();
                    add_patterns[selected](x, y, gamestate);
                }
            }
        }
        memcpy(gamestate_copy, gamestate, sizeof(gamestate));
        memcpy(gamestate_temp, gamestate_copy, sizeof(gamestate));
    }

    return;
}

bool gameplay_loop(void)
{
    long int generation = 0;
    int input;

    /* MAIN LOOP FOR THE GAMEPLAY! */
    for (;;)
    {
        moveCursorHome();
        gamestate_print();
        printf("%s%ld", " | Generation = ", generation++);
        gamestate_apply_rules();

        if (input_available())
        {
            input = get_key_pressed();
            if (input == ENTER)
            {
                system(CLEAR_SCREEN);

                return true;
            }
            else if (input == SPACE)
            {
                printf("%s", "\nPAUSED\r");
                for (;;)
                    if (input_available())
                        break;
                get_key_pressed();
                clear_line();
            }
            else if (input == ESC)
                break;
        }

        usleep(175000);
    }

    return false;
}

/**************** GamePlay | End ****************/

void print_manual(void)
{
    system(CLEAR_SCREEN);

    printf("\n\t\t========== MANUAL ==========\n\n");

    printf("Controls:\n");
    printf("  - Arrow keys (or W/A/S/D) to move cursor.              [][]  \n");
    printf("  - ENTER to confirm selections in menus.                []  []\n");
    printf("  - SPACE to pause/unpause while the simulation runs.      []  \n");
    printf("  - ESC to completely exit during simulation.\n\n");

    printf("Usage:\n");
    printf("  1) Select 'Play' to choose patterns or set cells.\n");
    printf("  2) Select 'DEMO' to watch a pre-seeded example.                  []    \n");
    printf("  3) Press SPACE during simulation to pause.                     []  []  \n");
    printf("  4) Press ENTER to return to the main menu.                       []  \n\n");

    printf("Press any key to return to the main menu...");

    printf("\n\n\n\t\t[][]  [][]\n\t\t[][]  [][]");

    /* Wait for a key press to return! */
    while (!input_available()) { /* DO NOTHING */ }
    get_key_pressed();

    system(CLEAR_SCREEN);

    return;
}

int main(void)
{
    int selected;

    /* Disable stdout buffering for instant printf() output */
    setvbuf(stdout, NULL, _IONBF, 0);

start:
    title_print();

    selected = menu_loop(options, NUM_ITEMS);
    switch (selected)
    {
        case (1):
            gameplay_select();
            if (gameplay_loop())
                goto start;

            break;
        case (2):
            gamestate_demo();
            if (gameplay_loop())
                goto start;

            break;
        case (3):
            print_manual();
            goto start;

            break;
        case (4):
            break;
    }

    printf("%s", "\n\nExiting...\n");

    return 0;
}

/* hide_cursor() => https://learn.microsoft.com/en-us/windows/console/setconsolecursorinfo?redirectedfrom=MSDN */
