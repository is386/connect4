#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include "csapp.h"

// Struct that holds the gameboard
struct Board
{
    char **b;
};

void child(int id, int pfd[2], int cfd[2]);
void printboard(int d, char **board);
int place(int d, int pos, char **board, char piece);
int check4(int d, char **board, char piece);

int main(int argc, char *argv[])
{
    int status, c, d, childPos, myPos, wins = 0, losses = 0;

    // Check if testing and for args
    if (argc < 2)
    {
        printf("Pass the amount of games as arg 1!\n");
        return 1;
    }
    else if (argc < 3)
    {
        printf("Pass the number of columns for the board as arg 2!\n");
        return 1;
    }
    else
    {
        c = atoi(argv[1]);
        d = atoi(argv[2]);

        if (c <= 0 || d <= 3)
        {
            printf("Number of Games or Dimension of Board too low!\n");
            return 0;
        }
    }

    int cols[c];
    pid_t pid[c];
    int pfd[c][2], cfd[c][2];
    char buf[MAXLINE], msg[MAXLINE];
    char line[d];
    struct Board boards[c];
    rio_t rio;
    srand(time(NULL));

    // Creates a row of the boards based on the given dimension, d.
    for (int i = 0; i < d; i++)
    {
        line[i] = '-';
    }
    line[d] = '\0';

    // Creates a parent and child pipe for each requested child, c.
    for (int i = 0; i < c; i++)
    {
        if (pipe(cfd[i]) < 0)
        {
            printf("Child Pipe Creation Error\n");
        }
        if (pipe(pfd[i]) < 0)
        {
            printf("Parent Pipe Creation Error\n");
        }
    }

    // Iterates over every child.
    for (int i = 0; i < c; i++)
    {
        // Mallocs a board for the child.
        boards[i].b = malloc(sizeof(char *) * d);

        // Mallocs the rows of the board.
        for (int j = 0; j < d; j++)
        {
            boards[i].b[j] = malloc(sizeof(char) * d);
            strcpy(boards[i].b[j], line);
        }

        // Forks the child and the child goes to the child function.
        pid[i] = fork();
        if (pid[i] == 0)
        {
            child(i, pfd[i], cfd[i]);
            return 0;
        }
        else
        {
            // The parent picks a random column that it will place its pieces in all game. This gives the parent a higher chance of winning.
            cols[i] = rand() % ((d - 1) + 1);
        }
    }

    // After all the children have been forked, the games begin.
    while (1)
    {
        // Iterates over all children
        for (int i = 0; i < c; i++)
        {
            strcpy(msg, "");
            strcpy(buf, "");
            myPos = cols[i];
            // Sends a message to the child containing the parent's designated column, and the range of the columns of the board
            sprintf(msg, "%d %d %d\n", 0, d - 1, myPos);
            rio_writen(pfd[i][1], msg, strlen(msg));
            rio_readinitb(&rio, cfd[i][0]);

            // Waits for the child to send a message back.
            while (1)
            {
                int rdval = rio_readlineb(&rio, buf, MAXLINE);
                if (rdval != 0)
                {
                    break;
                }
            }

            // Tokenizes the child message that contains the column they will place their piece in.
            char *token = strtok(buf, "\n");
            childPos = atoi(token);

            // The child goes first. If it returns a 1, the parent less and all pipes are closed and the board is printed.
            if (place(d, childPos, boards[i].b, 'C'))
            {
                losses += 1;
                rio_writen(pfd[i][1], "done", MAXLINE);
                printboard(d, boards[i].b);
                close(pfd[i][0]);
                close(pfd[i][1]);
                close(cfd[i][0]);
                close(cfd[i][1]);
            }
            // The parent goes first. If it returns a 1, the parent won and all pipes are closed and the board is printed.
            else if (place(d, myPos, boards[i].b, 'P'))
            {
                wins += 1;
                rio_writen(pfd[i][1], "done", MAXLINE);
                printboard(d, boards[i].b);
                close(pfd[i][0]);
                close(pfd[i][1]);
                close(cfd[i][0]);
                close(cfd[i][1]);
            }

            // Prints what happened during this turn
            printf("Game %d: Parent placed a chip in column %d.\n", i + 1, myPos);
            printf("Game %d: Child placed a chip in column %d.\n\n", i + 1, childPos);
        }
        // If the amount of wins and losses equals the amount of children, the games are done. Otherwise, continue playing.
        if ((wins + losses) == c)
        {
            break;
        }
    }

    printf("\nAll Games Completed. Wins: %d, Losses: %d\n", wins, losses);

    // Waits for each child and frees the boards.
    for (int i = 0; i < c; i++)
    {
        wait(NULL);
        for (int j = 0; j < d; j++)
        {
            free(boards[i].b[j]);
        }
        free(boards[i].b);
    }

    return 0;
}

// Places the given piece on the given board at the given position.
int place(int d, int pos, char **board, char piece)
{
    int x;

    for (x = 0; x < d; x++)
    {
        if (board[x][pos] != '-')
        {
            board[x - 1][pos] = piece;
            return check4(d, board, piece);
        }
    }

    board[x - 1][pos] = piece;
    return check4(d, board, piece);
}

// Checks for 4 in a row on the given board for the given piece. Returns 1 for winner.
int check4(int d, char **board, char piece)
{
    // Checks Vertically
    for (int x = 0; x < d - 3; x++)
    {
        for (int y = 0; y < d; y++)
        {
            if (board[x][y] == piece && board[x + 1][y] == piece && board[x + 2][y] == piece && board[x + 3][y] == piece)
            {
                printf("Four in a Row!\n");
                return 1;
            }
        }
    }
    // Checks Horizontally
    for (int x = 0; x < d; x++)
    {
        for (int y = 0; y < d - 3; y++)
        {
            if (board[x][y] == piece && board[x][y + 1] == piece && board[x][y + 2] == piece && board[x][y + 3] == piece)
            {
                printf("Four in a Row!\n");
                return 1;
            }
        }
    }
    // Checks Diagonally in the NE direction
    for (int x = 0; x < d - 3; x++)
    {
        for (int y = 3; y < d; y++)
        {
            if (board[x][y] == piece && board[x + 1][y - 1] == piece && board[x + 2][y - 2] == piece && board[x + 3][y - 3] == piece)
            {
                printf("Four in a Row!\n");
                return 1;
            }
        }
    }
    // Checks Diagonally in the NW direction
    for (int x = 0; x < d - 3; x++)
    {
        for (int y = 0; y < d - 3; y++)
        {
            if (board[x][y] == piece && board[x + 1][y + 1] == piece && board[x + 2][y + 2] == piece && board[x + 3][y + 3] == piece)
            {
                printf("Four in a Row!\n");
                return 1;
            }
        }
    }
    return 0;
}

// Prints the game board
void printboard(int d, char **board)
{
    for (int i = 0; i < d; i++)
    {
        printf("%s\n", board[i]);
    }
}

// The child process runs this
void child(int id, int pfd[2], int cfd[2])
{
    int min, max, pcol, mycol = 0;
    rio_t rio;
    char buf[MAXLINE];
    char msg[MAXLINE];
    rio_readinitb(&rio, pfd[0]);
    srand(time(NULL) + id + mycol);

    printf("Game Board %d has a process id of %d\n", id + 1, getpid());

    // Loop waits for the parent to send it some message.
    while (1)
    {
        strcpy(msg, "");
        strcpy(buf, "");
        rio_readlineb(&rio, buf, MAXLINE);
        // If the message is done, the child exits the function and closes the pipes.
        if (strcmp(buf, "done") == 0)
        {
            break;
        }

        // Parses the parent's message
        char *token = strtok(buf, " ");
        min = atoi(token);
        token = strtok(NULL, " ");
        max = atoi(token);
        token = strtok(NULL, " ");
        pcol = atoi(token);

        // Randomly picks a column that the parent didn't pick. This gives the parent a higher chance of winning.
        while (1)
        {
            mycol = rand() % (max + 1 - min) + min;
            if (mycol != pcol)
            {
                break;
            }
        }

        // Sends the desired column to the parent for placement.
        sprintf(msg, "%d\n", mycol);
        rio_writen(cfd[1], msg, strlen(msg));
    }

    close(pfd[0]);
    close(pfd[1]);
    close(cfd[0]);
    close(cfd[1]);
    return;
}