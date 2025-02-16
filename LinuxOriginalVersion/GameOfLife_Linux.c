#define _XOPEN_SOURCE   600 /* NEEDED FOR usleep() */
#define _POSIX_C_SOURCE 200112L /* NEEDED FOR usleep() */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define ZERO 0
#define ONE 1
#define TWO 2
#define THREE 3
#define FOUR 4
#define FIVE 5

#define DEAD 0
#define LIVE 1

#define ROWS 30
#define COLS 50
#define ROWS_M_ONE (ROWS - ONE)
#define COLS_M_ONE (COLS - ONE)

#define FROM_ASCII_TO_DECIMAL 48

#define TIME_STOP 200000 /* Microseconds */

struct gameStates
{
    char gameState1[ROWS][COLS];
    char gameState2[ROWS][COLS];
};
typedef struct gameStates Gs;

struct userCXY
{
    int userChoice;
    int X;
    int Y;
};
typedef struct userCXY UserCXY;

void processGs_FoT(Gs *states);
void printGs(Gs *states);
int numOfL(Gs *states);
void usersDandL(Gs *states, UserCXY *uChoices, int);
void usersDandL_GUI(Gs *states);
void prebuiltPatterns(Gs *states, UserCXY *uChoices);
void deadEdge(Gs *states);
void printChoices(void);
int getChoice(void);
int getPosition(UserCXY *uChoices);
void cleanLeftovers(void);

int main(void)
{
    Gs states;
    UserCXY uChoices;
    int gen = ZERO;

    memset(states.gameState1, DEAD, sizeof(states.gameState1));
    memset(states.gameState2, DEAD, sizeof(states.gameState2));

    while(ONE)
    {
        printChoices();
        uChoices.userChoice = getChoice();
        if(uChoices.userChoice == -TWO) { usersDandL(&states, &uChoices, DEAD); }
        else if(uChoices.userChoice == -ONE) { usersDandL(&states, &uChoices, LIVE); }
        else if(uChoices.userChoice == ZERO) { usersDandL_GUI(&states); }
        else if(uChoices.userChoice == -THREE)
        {
            printf("%s", "\n*RESET COMPLETED*\n\n");
            memset(states.gameState1, DEAD, sizeof(states.gameState1));
            continue;
        }
        else if((uChoices.userChoice > ZERO) && (uChoices.userChoice < 8)) /* Vailid choice */
        {
            if(getPosition(&uChoices) == ZERO) { continue; /* inVailid choice */ }
            prebuiltPatterns(&states, &uChoices);
        }
        else
        {
            printf("%s", "\n*INVAILID CHOICE!*\n\n");
            continue;
        }

        printGs(&states);

        printf("\033[0m\033[40m%s", "Start game?\n1 => YES | 0 => NO\n");
        uChoices.userChoice = getChoice();
        system("clear");
        if(uChoices.userChoice == ONE) { break; }
        else { continue; }
    }

    deadEdge(&states);

    while(ONE)
    {
        printf("\033[0m%s%d%s%d%s", "Generation = ", gen++, "\nPopulation = ", numOfL(&states), "\n");
        printGs(&states);

        processGs_FoT(&states);

        usleep(TIME_STOP);
        system("clear");
    }

return ZERO;
}

void processGs_FoT(Gs *states)
{
    int row, col;
    /* process Game state */
    for(row = ONE; row < ROWS_M_ONE; row++)
    {
        for(col = ONE; col < COLS_M_ONE; col++)
        {
            int liveCount;

            liveCount = (states -> gameState1[row - ONE][col - ONE]) + (states -> gameState1[row - ONE][col]) + (states -> gameState1[row - ONE][col + ONE]) + (states -> gameState1[row][col - ONE]) + (states -> gameState1[row][col + ONE]) + (states -> gameState1[row + ONE][col - ONE]) + (states -> gameState1[row + ONE][col]) + (states -> gameState1[row + ONE][col + ONE]);

            if( ( (states -> gameState1[row][col]) == LIVE ) && (liveCount == TWO || liveCount == THREE) ) { (states -> gameState2[row][col]) = LIVE; }
            else if( ( (states -> gameState1[row][col]) == DEAD ) && (liveCount == THREE) ) { (states -> gameState2[row][col]) = LIVE; }
            else { (states -> gameState2[row][col]) = DEAD; }
        }
    }
    /* Flow of Time */
    for(row = ONE; row < ROWS_M_ONE; row++) { for(col = ONE; col < COLS_M_ONE; col++) { (states -> gameState1[row][col]) = (states -> gameState2[row][col]); } }

return;
}

void printGs(Gs *states)
{
    int row, col;

    for(row = ONE; row < ROWS_M_ONE; row++)
    {
        for(col = ONE; col < COLS_M_ONE; col++)
        {
            if(states -> gameState1[row][col] == DEAD) { printf("\033[30m\033[40m%s", "[]"); /* BLACK LETTERS & BLACK BG */ }
            else { printf("\033[97m\033[107m%s", "[]"); /* WHITE LETTERS & WHITE BG */ }
        }
        printf("\033[30m\033[40m%s", "\n");
    }

return;
}

int numOfL(Gs *states)
{
    int row, col, liveCount = ZERO;

    for(row = ONE; row < ROWS_M_ONE; row++) { for(col = ONE; col < COLS_M_ONE; col++) { liveCount += (states -> gameState1[row][col]); } }

return liveCount;
}

void usersDandL(Gs *states, UserCXY *uChoices, int LorD)
{
    printf("%s", "|Give 0 to X for termination|\n");
    while(ONE)
    {
        printf("%s", "\n"); /* Clean Enviro */
        if((getPosition(uChoices) == ZERO) && (uChoices -> X != ZERO)) { continue; /* inVailid choice for X */ }
        if(uChoices -> X == ZERO) { return; }

        if(LorD == LIVE) { states -> gameState1[uChoices -> Y][uChoices -> X] = LIVE; }
        else if(LorD == DEAD) { states -> gameState1[uChoices -> Y][uChoices -> X] = DEAD; }
    }
}

void usersDandL_GUI(Gs *states)
{
    FILE *file;
    int row, col, DL;

    if((file = fopen("_gamestate_.txt", "wt")) == NULL)
    {
        printf("%s", "\n*ERROR*\nCannot load state in GUI input Mode!\n");

    return;
    }

    for(row = ONE; row < ROWS_M_ONE; row++)
    {
        for(col = ONE; col < COLS_M_ONE; col++) { fprintf(file, "%d", states -> gameState1[row][col]); }
        fprintf(file, "%s", "\n");
    }
    fprintf(file, "%d\n%d", ROWS, COLS);

    fclose(file);

    system("python3 GoL-GUI_gamestate_input.py");

    if((file = fopen("_gamestate_.txt", "rt")) == NULL)
    {
        printf("%s", "\n*ERROR*\nCannot import state from GUI input Mode!\n");

    return;
    }

    row = ONE; col = ONE;
    while((DL = fgetc(file)) != EOF)
    {
        if(row == ROWS_M_ONE) { break; }
        if(DL == '\n') { continue; }

        states -> gameState1[row][col] = DL - FROM_ASCII_TO_DECIMAL;
        col++;

        if(col == COLS_M_ONE) { col = ONE; row++; }
    }

    fclose(file);

return;
}

void prebuiltPatterns(Gs *states, UserCXY *uChoices)
{/* MAIN GOAL: NO NEGATIVE number addition IN COORDINATES!!! */
    int i;

    if(uChoices -> userChoice == ONE) { for(i = ZERO; i < THREE; i++) { states -> gameState1[uChoices -> Y][uChoices -> X + i] = LIVE; } }

    else if(uChoices -> userChoice == TWO)
    {
        for(i = ONE; i < FOUR; i++) { states -> gameState1[uChoices -> Y][uChoices -> X + i] = LIVE; }
        for(i = ZERO; i < THREE; i++) { states -> gameState1[uChoices -> Y + ONE][uChoices -> X + i] = LIVE; }
    }

    else if(uChoices -> userChoice == THREE)
    {
        for(i = ZERO; i < TWO; i++) { states -> gameState1[uChoices -> Y][uChoices -> X + i] = LIVE; }
        for(i = TWO; i < FOUR; i++) { states -> gameState1[uChoices -> Y + i][uChoices -> X + THREE] = LIVE; }
        states -> gameState1[uChoices -> Y + THREE][uChoices -> X + TWO] = LIVE;
        states -> gameState1[uChoices -> Y + ONE][uChoices -> X] = LIVE;
    }

    else if(uChoices -> userChoice == FOUR)
    {
        for(i = ZERO; i < TWO; i++) { states -> gameState1[uChoices -> Y + i][uChoices -> X + ONE + i] = LIVE; }
        for(i = ZERO; i < THREE; i++) { states -> gameState1[uChoices -> Y + TWO][uChoices -> X + i] = LIVE; }
    }

    else if(uChoices -> userChoice == FIVE)
    {
        for(i = TWO; i < FIVE; i++)
        {
            int y = (uChoices -> Y) + i, x = (uChoices -> X) + i, y2 = y + 6, x2 = x + 6;

            states -> gameState1[y][uChoices -> X] = LIVE;
            states -> gameState1[y][uChoices -> X + FIVE] = LIVE;
            states -> gameState1[y][uChoices -> X + 7] = LIVE;
            states -> gameState1[y][uChoices -> X + 12] = LIVE;

            states -> gameState1[uChoices -> Y][x] = LIVE;
            states -> gameState1[uChoices -> Y + FIVE][x] = LIVE;
            states -> gameState1[uChoices -> Y + 7][x] = LIVE;
            states -> gameState1[uChoices -> Y + 12][x] = LIVE;

            states -> gameState1[y2][uChoices -> X] = LIVE;
            states -> gameState1[y2][uChoices -> X + FIVE] = LIVE;
            states -> gameState1[y2][uChoices -> X + 7] = LIVE;
            states -> gameState1[y2][uChoices -> X + 12] = LIVE;

            states -> gameState1[uChoices -> Y][x2] = LIVE;
            states -> gameState1[uChoices -> Y + FIVE][x2] = LIVE;
            states -> gameState1[uChoices -> Y + 7][x2] = LIVE;
            states -> gameState1[uChoices -> Y + 12][x2] = LIVE;
        }
    }

    else if(uChoices -> userChoice == 6)
    {
        for(i = ZERO; i < THREE; i++) { states -> gameState1[uChoices -> Y + i][uChoices -> X + ONE] = LIVE; }
        states -> gameState1[uChoices -> Y + ONE][uChoices -> X] = LIVE;
        states -> gameState1[uChoices -> Y][uChoices -> X + TWO] = LIVE;
    }

    else if(uChoices -> userChoice == 7)
    {
        states -> gameState1[uChoices -> Y][uChoices -> X + ONE] = LIVE;
        for(i = TWO; i < 6; i++) { states -> gameState1[uChoices -> Y + i][uChoices -> X + ONE] = LIVE; }
        states -> gameState1[uChoices -> Y + 7][uChoices -> X + ONE] = LIVE;
        for(i = ZERO; i < 8; i++)
        {
            states -> gameState1[uChoices -> Y + i][uChoices -> X] = LIVE;
            states -> gameState1[uChoices -> Y + i][uChoices -> X + TWO] = LIVE;
        }
    }

return;
}

void deadEdge(Gs *states)
{
    int row, col;

    for(row = ZERO; row < ROWS; row++)
    {
        states -> gameState1[row][ZERO] = DEAD;
        states -> gameState1[row][COLS_M_ONE] = DEAD;
    }
    for(col = ZERO; col < COLS; col++)
    {
        states -> gameState1[ZERO][col] = DEAD;
        states -> gameState1[ROWS_M_ONE][col] = DEAD;
    }

return;
}

void printChoices(void)
{
    printf("%s", "-2) Enter dead cells individually\n"
           "-1) Enter live cells individually\n"
           " 0) Enter cells' state in GUI input Mode\n\n"
           "Patterns to choose from:\n"
           " 1) Blinker (period 2)\n"
           " 2) Toad (period 2)\n"
           " 3) Beacon (period 2)\n"
           " 4) Glider\n"
           " 5) Pulsar (period 3)\n"
           " 6) The R-pentomino\n"
           " 7) Penta-decathlon (period 15)\n"
           "\n-2) *RESET*\n\n");

return;
}

int getChoice(void)
{
    int userChoice;

    printf("%s", "Enter choice: ");
    scanf("%d", &userChoice);
    cleanLeftovers();

return userChoice;
}

int getPosition(UserCXY *uChoices) /* RETURN 1 => OK! */
{
    printf("%s", "X position -> ");
    uChoices -> X = getChoice();
    if(uChoices -> X < ONE || uChoices -> X > (COLS_M_ONE - ONE))
    {
        if( (uChoices -> X == ZERO) && ( (uChoices -> userChoice == ZERO) || (uChoices -> userChoice == -ONE) ) ) { return ZERO; }
        printf("%s%d%s", "\n*INVAILID VALUE!*\n1 < X < ", (COLS_M_ONE - ONE), "\n\n");

    return ZERO;
    }

    printf("%s", "Y position -> ");
    uChoices -> Y = getChoice();
    if(uChoices -> Y < ONE || uChoices -> Y > (ROWS_M_ONE - ONE))
    {
        printf("%s%d%s", "\n*INVAILID VALUE!*\n1 < Y < ", (ROWS_M_ONE - ONE), "\n\n");

    return ZERO;
    }

return ONE;
}

void cleanLeftovers(void)
{ /* "KATHARISE" TO input THS scanf() */
    char trow;
    /* "PETA" XARAKTHRES MEXRI NA BREIS TO '\n'(DHLADH, TO TELOS THS input TOY XRHSTH) */
    while(scanf("%c", &trow), trow != '\n') {}

return;
}
