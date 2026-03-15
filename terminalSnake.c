#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>

#define true 1
#define false 0
#define mapRows 15
#define mapCols 15

char map[mapRows][mapCols];
    
int alive = true;
int appleX = -1;
int appleY = -1;
int appleExists = false;
int speed = 300 * 1000; // (300ms)

// set up terminal for non-blocking keyboard input
struct termios oldt;

void enableRawMode()
{
    struct termios newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

typedef struct snakeNode{
        int x;
        int y;
        struct snakeNode* next;
        struct snakeNode* prev;
} snakeNode;

typedef struct snake{
        struct snakeNode* head;
        struct snakeNode* tail;
} snake;

typedef enum {
    up,
    down,
    left,
    right
} direction;

direction dir = right; // default direction

void addSnakeNode(snakeNode **head, int x , int y)
{
    struct snakeNode *newSnakeNode = (snakeNode*)malloc(sizeof(snakeNode));

    newSnakeNode->x = x;
    newSnakeNode->y = y;
    newSnakeNode->next = *head;
    newSnakeNode->prev = NULL;

    if(*head != NULL)
    {
        (*head)->prev = newSnakeNode;
    }

    *head = newSnakeNode;
}

void clearScreen()
{
    printf("\033[2J");
}

void resetCursor()
{
    printf("\033[H"); // reset cursor to top left
}

// print current map
void printMap(int rows, int cols, char map[rows][cols])
{
    for(int row = 0; row < rows; row++)
        {
             for(int col = 0; col < cols; col++)
             {
                printf("%c", map[row][col]);
             }
             printf("\n");
        }
}

void readInput()
{
    char c;

    if(read(STDIN_FILENO, &c, 1) > 0)
    {
        if(c == 'w' && dir != down) dir = up;
        if(c == 's' && dir != up) dir = down;
        if(c == 'a' && dir != right) dir = left;
        if(c == 'd' && dir != left) dir = right;
    }
}

void moveSnake(snake *gameSnake)
{
    int newX = gameSnake->head->x;
    int newY = gameSnake->head->y;

    if(dir == up) newY--;
    if(dir == down) newY++;
    if(dir == left) newX--;
    if(dir == right) newX++;

    // boundary wall collision
    if(newX < 0 || newX >= mapCols || newY < 0 || newY >= mapRows)
    {
        alive = false;
        return;
    }

    // self collide check
    snakeNode *curr = gameSnake->head;
    while(curr != NULL)
    {
        if(curr->x == newX && curr->y == newY)
        {
            alive = false;
            return;
        }
        curr = curr->next;
    }

    int ateApple = false;

    if(newX == appleX && newY == appleY)
    {
        ateApple = true;
        appleExists = false;

        if(speed > 80 * 1000) // max speed is 80ms
        {
            speed -= 10000;
        }
    }

    snakeNode *newHead = malloc(sizeof(snakeNode));

    newHead->x = newX;
    newHead->y = newY;
    newHead->prev = NULL;
    newHead->next = gameSnake->head;

    gameSnake->head->prev = newHead;
    gameSnake->head = newHead;

    if(!ateApple)
    {
        snakeNode *oldTail = gameSnake->tail;
        gameSnake->tail = oldTail->prev;
        
        if(gameSnake->tail->next != NULL)
        {
            gameSnake->tail->next = NULL;
        }
        free(oldTail);
    }

}

void generateApple(snake *gameSnake)
{
    if(appleExists == true)
    {
        return;
    }

    int row, col;
    int valid = false;

    while(!valid)
    {
        row = rand() % mapRows;
        col = rand() % mapCols;
        valid = true;

        snakeNode *curr = gameSnake->head;

        
        while(curr != NULL)
        {
            if(curr->x == col && curr->y == row)
            {
                valid = false;
                break;
            }
            curr = curr->next;
        }
    }

    appleX = col;
    appleY = row;
    appleExists = true;
}

int main(void){
    
    srand(time(NULL));
    
    clearScreen();

    // initialize snake head and first body segment
    snakeNode *head = malloc(sizeof(snakeNode));
    snakeNode *body0 = malloc(sizeof(snakeNode));
    snake gameSnake;
    
    head->x = 7;
    head->y = 7;
    head->prev = NULL;

    body0->x = 7;
    body0->y = 8;
    body0->next = NULL;

    head->next = body0;
    body0->prev = head;

    // linked list works backward so head -> body 0 -> body 1 ... -> tail

    gameSnake.head = head;
    gameSnake.tail = body0;

    map[head->y][head->x] = '0';
    map[body0->y][body0->x] = 'O';

    enableRawMode();
    atexit(disableRawMode);
    clearScreen();

    while(alive == true)
    {
        readInput();
        moveSnake(&gameSnake);
        generateApple(&gameSnake);

        resetCursor();

        // reset map
        for(int row = 0; row < mapRows; row++)
        {
            for(int col = 0; col < mapCols; col++)
            {
                map[row][col] = ' ';
            }
        }

        snakeNode *curr = gameSnake.head;

        while(curr != NULL)
        {
            map[curr->y][curr->x] = 'O';
            curr = curr->next;
        }

        if(appleExists == true)
        {
            map[appleY][appleX] = 'A';
        }

        printMap(mapRows, mapCols, map);

        usleep(speed); // (game speed handler)
    }

    disableRawMode();
}
