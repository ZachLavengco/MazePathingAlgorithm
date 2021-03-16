#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* This program will read the first 3 lines of input
   and prints a static 2D maze*/

typedef struct mazeStruct
{
    char arr[32][32];  /* allows for a maze of size 30x30 plus outer walls */
    int xsize, ysize;
    int xstart, ystart;
    int xend, yend;
} maze;

// linked list structure
typedef struct Node {
    int col;
    int row;
    struct Node* next;
} node;

// prototype functions
void addNode(node** head, int x, int y);
void addNode2(node** head, int x, int y);
void init(node** head);
void removeNode(node** head);
void removeNode2(node** head);
int checkIfEmpty(node* head);
int evalNeighbors(maze* maze, node** head);
void push(maze* maze, node** head, int* coinCount);
void invertList(node** head);

// debugmode initialization
#define FALSE 0
#define TRUE 1
int dMode = FALSE;

int main(int argc, char** argv)
{
    maze m1;
    setbuf(stdout, NULL);  // makes sure stderr and stdout works properly

    int xpos, ypos;
    int i, j;


    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            dMode = TRUE;
            break;
        }
    }

    /* verify the proper number of command line arguments were given */
    if (argc > 2 && dMode == FALSE) {
        printf("Too many input files.\n");
        exit(-1);
    }

    FILE* src;
    /* Try to open the input file. */
    if ((src = fopen(argv[1], "r")) == NULL)
    {
        printf("Can't open input file: %s", argv[1]);
        exit(-1);
    }

    // checks if file size is valid or not
    int ch;
    int lines = 0;
    while (EOF != (ch = getc(src))) {
        if ('\n' == ch) {
            ++lines;
        }
    }
    if (lines <= 3) {
        printf("Invalid data file.\n");
        exit(-1);
    }
    rewind(src);

    /* read in the size, starting and ending positions in the maze */
    fscanf(src, "%d %d", &m1.xsize, &m1.ysize);
    fscanf(src, "%d %d", &m1.xstart, &m1.ystart);
    fscanf(src, "%d %d", &m1.xend, &m1.yend);

    // checks if the sizes are valid or not
    if (m1.xsize < 1 || m1.ysize < 1) {
        fprintf(stderr, "%s", "Maze sizes must be greater than 0.\n");
        exit(-1);
    }

    // checks if the start/end positions are valid or not
    if (m1.xstart < 1 || m1.xstart > m1.xsize ||
        m1.ystart < 1 || m1.ystart > m1.ysize ||
        m1.xend < 1 || m1.xend > m1.xsize ||
        m1.yend < 1 || m1.yend > m1.ysize) {
        fprintf(stderr, "%s", "Start/End position outside of maze range.\n");
        exit(-1);
    }

    /* print them out to verify the input */
    printf("size: %d, %d\n", m1.xsize, m1.ysize);
    printf("start: %d, %d\n", m1.xstart, m1.ystart);
    printf("end: %d, %d\n", m1.xend, m1.yend);

    /* initialize the maze to empty */
    for (i = 0; i < m1.xsize + 2; i++)
        for (j = 0; j < m1.ysize + 2; j++)
            m1.arr[i][j] = '.';

    /* mark the borders of the maze with *'s */
    for (i = 0; i < m1.xsize + 2; i++)
    {
        m1.arr[i][0] = '*';
        m1.arr[i][m1.ysize + 1] = '*';
    }
    for (i = 0; i < m1.ysize + 2; i++)
    {
        m1.arr[0][i] = '*';
        m1.arr[m1.xsize + 1][i] = '*';
    }

    /* mark the starting and ending positions in the maze */
    m1.arr[m1.xstart][m1.ystart] = 's';
    m1.arr[m1.xend][m1.yend] = 'e';

    /*Complete this code by reading the rest of the input
    and placing blocked and coin positions. */
    char c;
    while (fscanf(src, "%d %d %c", &xpos, &ypos, &c) != EOF) {
        if (xpos > m1.xsize || ypos > m1.ysize) {
            fprintf(stderr, "%s", "Invalid coordinates: outside of maze range.\n");
            continue;
        }
        if ((xpos == m1.xstart && ypos == m1.ystart) ||
            (xpos == m1.xend && ypos == m1.yend)) {
            fprintf(stderr, "%s", "Invalid coordinates: attempting to block start/end position.\n");
            continue;
        }
        if (c != 'b' && c != 'c') {
            fprintf(stderr, "%s", "Invalid type: type is not recognized.\n");
            continue;
        }
        if (c == 'b')
            m1.arr[xpos][ypos] = '*';
        if (c == 'c')
            m1.arr[xpos][ypos] = 'C';
    }

    /*Close the file*/
    fclose(src);

    /* print out the initial maze */
    for (i = 0; i < m1.xsize + 2; i++) {
        for (j = 0; j < m1.ysize + 2; j++)
            printf("%c", m1.arr[i][j]);
        printf("\n");
    }

    // the maze algorithm
    int coinCount = 0;
    node* head = NULL;
    m1.arr[m1.xstart][m1.ystart] = '*';
    addNode(&head, m1.xstart, m1.ystart);

    while (m1.arr[head->col][head->row] != 'e' && checkIfEmpty(head) == FALSE) {
        // checks if top of the stack is the end posiiton
        if (m1.arr[head->col][head->row] == 'e')
            break;
        // checks if top of the stack has and unvisited and unblocked neighbor
        else if (evalNeighbors(&m1, &head) == TRUE)
            push(&m1, &head, &coinCount);
        else
            removeNode(&head);
    }

    // prints results
    if (checkIfEmpty(head) == TRUE) {
        printf("\nThis maze has no solution.\n");
        return 0;
    }
    else {
        printf("\nThe maze has a solution.");
        printf("\nThe amount of coins collected: %d\n", coinCount);
        return 0;
    }
}

// adds a node into the linked list
void addNode(node** head, int x, int y) {
    // initializes new node
    node* temp;
    init(&temp);
    temp->col = x;
    temp->row = y;
    temp->next = *head;

    // assigns head to temp
    *head = temp;
    if (dMode == TRUE)
        printf("(%d,%d) pushed into the stack.\n", temp->col, temp->row);
}

// addNode function but doesn't have debugger
void addNode2(node** head, int x, int y) {
    // initializes new node
    node* temp;
    init(&temp);
    temp->col = x;
    temp->row = y;
    temp->next = *head;

    // assigns head to temp
    *head = temp;
}

// allocates space for new node
void init(node** head) {
    *head = (node*)malloc(sizeof(node));
}

// removes the first node in the linked list
void removeNode(node** head) {
    // initialize a temp pointer used to be delete first node
    node* remove = *head;
    if (dMode == TRUE)
        printf("(%d,%d) pushed into the stack.\n", remove->col, remove->row);

    // sends head pointer to second node in linked list and deletes the first
    if (remove != NULL) {
        *head = remove->next;
        free(remove);
    }
}

// removeNode function but doesn't have debugger
void removeNode2(node** head) {
    // initialize a temp pointer used to be delete first node
    node* remove = *head;

    // sends head pointer to second node in linked list and deletes the first
    if (remove != NULL) {
        *head = remove->next;
        free(remove);
    }
}

// checks if linked list is empty
int checkIfEmpty(node* head) {
    if (head == NULL) { return TRUE; }
    else { return FALSE; }
}

// checks the neighbors of current point in maze
int evalNeighbors(maze* maze, node** head) {
    // check right
    if (maze->arr[(*head)->col + 1][(*head)->row] != '*')
        return TRUE;
    // check left
    else if (maze->arr[(*head)->col - 1][(*head)->row] != '*')
        return TRUE;
    // check down
    else if (maze->arr[(*head)->col][(*head)->row + 1] != '*')
        return TRUE;
    // check up
    else if (maze->arr[(*head)->col][(*head)->row - 1] != '*')
        return TRUE;
    else  // occurs when there's no barriers around the point
        return FALSE;
}

void push(maze* maze, node** head, int* coinCount) {
    // check right
    if (maze->arr[(*head)->col + 1][(*head)->row] != '*') {
        if (maze->arr[(*head)->col + 1][(*head)->row] != 'e')
            (maze->arr[(*head)->col + 1][(*head)->row] = '*');

        if (maze->arr[(*head)->col + 1][(*head)->row] == 'C')
            (*coinCount)++;
        addNode(head, (*head)->col + 1, (*head)->row);

    // check left
    }
    else if (maze->arr[(*head)->col - 1][(*head)->row] != '*') {
        if (maze->arr[(*head)->col - 1][(*head)->row] != 'e')
            (maze->arr[(*head)->col - 1][(*head)->row] = '*');

        if (maze->arr[(*head)->col - 1][(*head)->row] == 'C')
            (*coinCount)++;
        addNode(head, (*head)->col - 1, (*head)->row);

    // check down
    }
    else if (maze->arr[(*head)->col][(*head)->row + 1] != '*') {
        if (maze->arr[(*head)->col][(*head)->row + 1] != 'e')
            (maze->arr[(*head)->col][(*head)->row + 1] = '*');

        if (maze->arr[(*head)->col][(*head)->row + 1] == 'C')
            (*coinCount)++;
        addNode(head, (*head)->col, (*head)->row + 1);

    // check up
    }
    else if (maze->arr[(*head)->col][(*head)->row - 1] != '*') {
        if (maze->arr[(*head)->col][(*head)->row - 1] != 'e')
            (maze->arr[(*head)->col][(*head)->row - 1] = '*');

        if (maze->arr[(*head)->col][(*head)->row - 1] == 'C')
            (*coinCount)++;
        addNode(head, (*head)->col, (*head)->row - 1);

    /* function did not work correctly */
    }
    else {
        printf("push failed!\n");
        exit(-1);
    }
}

void invertList(node** head) {
    node* display = NULL;
    node* inverted = NULL;

    // inverts the list order into new stack
    while (checkIfEmpty(*head) == FALSE) {
        addNode2(&inverted, (*head)->col, (*head)->row);
        removeNode2(head);
    }

    // prints out inverted list
    printf("The path from start to end:\n");
    while (checkIfEmpty(inverted) == FALSE) {
        printf("(%d,%d)", inverted->col, inverted->row);
        if (inverted->next != NULL)
            printf(" ");
        addNode2(&display, inverted->col, inverted->row);
        removeNode2(&inverted);
    }
    free(inverted);
    printf("\n");

    // used for the autograder
    while (checkIfEmpty(display) == FALSE) {
        removeNode(&display);
    }
    free(display);
}