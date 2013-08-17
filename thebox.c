#include <stdio.h>
#include <stdlib.h>

/* Stores co-ordinates */
typedef struct {
    int xPos;
    int yPos;
} Point;

/* Stores an array of 26 spaces for all letters */
typedef struct {
    int alph[26];
} Alphabet;

/* Stores all variables related to the map */
typedef struct {
    char** map;
    int xDim;
    int yDim;
    char direction;
    Point currentPos;
} MapInfo;

/* Function prototypes */
void print_map(MapInfo);
MapInfo start_sim(MapInfo);
MapInfo check_square(MapInfo);
MapInfo move_ball(MapInfo);
Point search_map(MapInfo, char);
void check_letters(Alphabet);
MapInfo read_file(MapInfo, char *);
char** create_array(MapInfo);
char** populate_array(MapInfo, char *);
char reflector_nesw(char);
char reflector_nwse(char);
char reflector(char);
Point launchpad(Point, char);
int check_input(char, int, MapInfo);
void dump_line(FILE*);
MapInfo fire_ball(MapInfo, char, char);
Alphabet add_letters(Alphabet, char);
void check_valid_chars(char);
void check_map_size(MapInfo, int);


int main(int argc, char *argv[]) {
    int maxSteps; /* Stores the maximum allowed steps */
    MapInfo info; /* Current state of the map */
    int currentSteps = 0; /* Keeps track of steps taken */

    /* Check for correct number of user inputs and store variables*/
    if(argc == 2) { 
        maxSteps = 10; /* Store default max steps when not given */
    } else if(argc == 3) {
        maxSteps = atoi(argv[2]); /* Store given maximum number of steps */
        if(maxSteps>=1000 || maxSteps<=0) { /* Check for valid max steps */
            fprintf(stderr, "Bad max steps.\n");
            return 2;
        }
    } else { /* Incorrent number of arguments given */
        fprintf(stderr, "Usage: thebox mapfile [maxsteps]\n");
        return 1;
    }

    /* Main game loop */
    while(1) {
        info = read_file(info, argv[1]);
        print_map(info);
        info = start_sim(info);
        currentSteps = 0;
        while(1) {
            if(currentSteps>=maxSteps) { /* End when max steps exceeded */
                printf("End of simulation.\n");
                break;
            }
            if(info.currentPos.xPos < 0 || info.currentPos.xPos >= info.xDim 
                || info.currentPos.yPos < 0 || 
                info.currentPos.yPos >= info.yDim) {/*End when ball exits map*/
                printf("End of simulation.\n");
                break;
            }
            info = check_square(info);
            info = move_ball(info);
            currentSteps++;
            print_map(info);
        }
    }

    return 0;

}

/* Print the map to the screen */
void print_map(MapInfo info) { 
    int row1;
    int column1;
    for(row1 = 0; row1<info.xDim; row1++) {
        for(column1 = 0; column1<info.yDim; column1++) {
            printf("%c", info.map[row1][column1]);
        }
        printf("%c", '\n');
    }
    printf("%c", '\n');
    
}

/* Get user input and fire the ball */
MapInfo start_sim(MapInfo info) {
    char side, buffer[7], newLine;
    int sidePos, scanResult;
    printf("(side pos)>");
    fgets(buffer, 7, stdin);
    scanResult = sscanf(buffer, "%c%d%c", &side, &sidePos, &newLine);

    if(feof(stdin)) { /* Exit when ctrl+d pressed */
        exit(0);
    } else if(side == '\n') { /* Reprompt when line empty */
        info = start_sim(info);
        return info;
    } else if(newLine != '\n') { /* Reprompt if excess characters */
        dump_line(stdin);
        info = start_sim(info);
        return info;
    } else if(!check_input(side, sidePos, info)) { /* Reprompt if bad inputs */
        info = start_sim(info);
        return info;
    } else if(scanResult == 1) { /* Reprompt if lacking characters */
        dump_line(stdin);
        info = start_sim(info);
        return info;
    }

    /* Take one from sidepos as array starts at 0 */
    sidePos--;

    info = fire_ball(info, side, sidePos);
    
    return info;
}

/* Fire ball in at starting position */
MapInfo fire_ball(MapInfo info, char side, char sidePos) {
    int startCol, startRow;
    switch(side) {
        case 'N':
            startRow = 0;
            startCol = sidePos;
            info.direction = 'S';
            break;

        case 'E':
            startCol = info.yDim-1;
            startRow = sidePos;
            info.direction = 'W';
            break;

        case 'S':
            startRow = info.xDim-1;
            startCol = sidePos;
            info.direction = 'N';
            break;

        case 'W':
            startCol = 0;
            startRow = sidePos;
            info.direction = 'E';
            break;

        default:            ;

    }

    info.currentPos.xPos = startRow;
    info.currentPos.yPos = startCol;
    return info;
}

/* Check the character on the current square and modify attributes */
MapInfo check_square(MapInfo info) {
    int currentX = info.currentPos.xPos;
    int currentY = info.currentPos.yPos;
    char square = info.map[currentX][currentY];
    Point newPoint;

    if(square >= '1' && square < '9') { /* Check if number on square */
        info.map[currentX][currentY] = ++square;
    } else if(square >= 'A' && square <= 'Z') { /* Check if letter on square */
        newPoint = search_map(info, ++square);
        if(newPoint.xPos == -1) {  /* Next char isn't found on map */
            newPoint = search_map(info, 'A'); /* Move ball to 'A' */
            info.currentPos.xPos = newPoint.xPos;
            info.currentPos.yPos = newPoint.yPos;
            return info;
        } else { /* Next char is found - move ball there */
            info.currentPos.xPos = newPoint.xPos;
            info.currentPos.yPos = newPoint.yPos;
            return info;
        }
    } else {
        switch(square) { /* Make appropriate changes based on the square */
            case '.':
                info.map[currentX][currentY] = '1';
                break;

            case '/':
                info.direction = reflector_nesw(info.direction);
                break;

            case '\\':
                info.direction = reflector_nwse(info.direction);
                break;

            case '=':
                info.direction = reflector(info.direction);
                break;

            case '@':
                info.currentPos = launchpad(info.currentPos, info.direction);
                break;
        }
    }
    return info;
}

/* Move ball to new position */
MapInfo move_ball(MapInfo info) {
    switch(info.direction) {
        case 'N':
            info.currentPos.xPos--;
            break;
        case 'E':
            info.currentPos.yPos++;
            break;
        case 'S':
            info.currentPos.xPos++;
            break;
        case 'W':
            info.currentPos.yPos--;
            break;
    }
    return info;
}

/* Find and return co-ordinates of given character - [-1,-1] if not found */
Point search_map(MapInfo info, char searchChar) {
    int i;
    int j;
    char onSquare;
    Point newPoint;

    /* Iterate through map */
    for(i = 0; i<info.xDim; i++) {
        for(j = 0; j<info.yDim; j++) {
            onSquare = info.map[i][j]; /* Save character on the square */
            if((onSquare == searchChar)) {
                newPoint.xPos = i;
                newPoint.yPos = j;
                return newPoint; /* Position of searched character */
            }
        }
    }

    /* Searched character not found */
    newPoint.xPos = -1;
    newPoint.yPos = -1;
    return newPoint;
}

/* Open the given file, and store all map details */
MapInfo read_file(MapInfo info, char *filename) {
    FILE *file;
    int testVar;
    char middleChar;

    file = fopen(filename, "r");
    
    if(file == 0) { /* Exit if file open fails */
        fprintf(stderr, "Missing map file.\n");
        exit(3);
    } else { /* Store map dimensions */
        testVar = fscanf(file, "%d%c%d", &info.xDim, &middleChar, &info.yDim);
        
	/* Exit if invalid dimensions given */
        if(testVar != 3 || middleChar != ' ') {
            fprintf(stderr, "Bad map dimensions.\n");
            exit(4);
        }
        if(info.xDim <= 0 || info.yDim <= 0) {
            fprintf(stderr, "Bad map dimensions.\n");
            exit(4);
        }
    }

    info.map = create_array(info);
    info.map = populate_array(info, filename);

    return info;
}

/* Create array of given size */
char** create_array(MapInfo info) {
    int first = 1;
    if(first) {
        /* Create 2d array of chars */
        int i;
        info.map = (char**) malloc(info.xDim*sizeof(char*));
        for(i = 0; i<info.xDim; i++) {
            info.map[i] = (char*) malloc(info.yDim*sizeof(char));
        }
        first = 0;
    }

    return info.map;
}

/* Add characters in file to the array */
char** populate_array(MapInfo info, char *filename) {
    /* Store chars to array */
    FILE *file;
    int row = -1, column = 0, rowCount = 0, j, firstLinePassed = 0;
    char ch;
    Alphabet alph;

    for(j = 0; j<26; j++) { /* Populate alph with all 0's */
        alph.alph[j] = 0;
    }

    file = fopen(filename, "r");

    /* Add all characters in the file to the array */
    while((ch = fgetc(file)) != EOF) {
        if(!firstLinePassed && ch!='\n') { /* Skip first line (only has \n) */
            continue;
        } else {
            firstLinePassed = 1;
        }
        if((ch>='A' && ch<='Z')) { /* Check if a letter */
            alph = add_letters(alph, ch);
        }
        if(row != -1) {
            if(ch == '\n') {
                row++;
                rowCount++;
                if(row>info.xDim || column!=info.yDim) {
                    fprintf(stderr, "Map file is the wrong size.\n");
                    exit(6);
                }
                column = 0;
            } else {
                check_valid_chars(ch);
                info.map[row][column] = ch;
                column++;
                if(column > info.yDim) {
                    fprintf(stderr, "Map file is the wrong size.\n");
                    exit(6);
                }
            }
        } else {
            row++;
        }
    }
    check_letters(alph);
    /* Check map ended the right size */
    check_map_size(info, row);
    return info.map;
}

/* Returns 1 if all letters valid, 0 otherwise */
void check_letters(Alphabet alph) { 
    int noEarlyLetter = 0; /* Is 0 when an empty space is found in alph */
    int i;
    int letterCount = 0;

    for(i = 0; i<26; i++) {
        if(alph.alph[i]) {
            letterCount++;
        }
	/* 1st empty space in alph found */
        if(!alph.alph[i] && !noEarlyLetter) {
            noEarlyLetter = 1;
        }
	/* Letter found in alph after empty space - exit program */
        if(alph.alph[i] && noEarlyLetter) {
            fprintf(stderr, "Missing letters.\n");
            exit(7);
        }
    }
/* Returns bad if only one letter has been found */
    if(!letterCount || letterCount>1) {
        return;
    } else {
        fprintf(stderr, "Missing letters.\n");
        exit(7);
    }
}

/* Changes balls direction in response to '/' */
char reflector_nesw(char direction) {
    char newDirection;
    switch(direction) {
        case 'N':
            newDirection = 'E';
            break;
        case 'E':
            newDirection = 'N';
            break;
        case 'S':
            newDirection = 'W';
            break;
        case 'W':
            newDirection = 'S';
            break;
    }
    return newDirection;
}

/* Changes balls direction in response to '\' */
char reflector_nwse(char direction) {
    char newDirection;
    switch(direction) {
        case 'N':
            newDirection = 'W';
            break;
        case 'E':
            newDirection = 'S';
            break;
        case 'S':
            newDirection = 'E';
            break;
        case 'W':
            newDirection = 'N';
            break;
    }
    return newDirection;
}

/* Changes balls direction in response to '=' */
char reflector(char direction) {
    char newDirection;
    switch(direction) {
        case 'N':
            newDirection = 'S';
            break;
        case 'E':
            newDirection = 'W';
            break;
        case 'S':
            newDirection = 'N';
            break;
        case 'W':
            newDirection = 'E';
            break;
    }
    return newDirection;
}

/* Changes balls position in response to '@' */
Point launchpad(Point currentPos, char direction) {
    Point newPos = currentPos;
    switch(direction) {
        case 'N':
            newPos.xPos -= 4;
            break;
        case 'E':
            newPos.yPos += 4;
            break;
        case 'S':
            newPos.xPos += 4;
            break;
        case 'W':
            newPos.yPos -= 4;
            break;
    }
    return newPos;
}

int check_input(char side, int sidePos, MapInfo info) {
    if(!(side == 'N' || side == 'E' || side == 'S' || side == 'W') || 
        sidePos <= 0) {
        return 0;
    }

    if((side == 'N' || side == 'S') && (sidePos > info.xDim)) {
        return 0;
    }

    if((side == 'E' || side == 'W') && (sidePos > info.yDim)) {
        return 0;
    }

    return 1;
}

/* Eat in all characters on the line in FILE given */
void dump_line(FILE *fp) {
    int ch;

    while((ch = fgetc(fp)) != EOF && ch != '\n');
}

/* Add letter to encountered letters in Alphabet */
Alphabet add_letters(Alphabet alph, char ch) {
    if (alph.alph[ch-65]) { /* Exit if letter already entered */
        fprintf(stderr, "Missing letters.\n");
        exit(7);
    } else {
        alph.alph[ch-65] = 1; /* Add letter to alph */
    }

    return alph;
}

/* Check if a valid character */
void check_valid_chars(char ch) {
    if(!(ch=='.' || ch=='/' || ch=='\\' || ch=='@' || ch=='=' || 
        (ch>='A' && ch<='Z'))) {
        fprintf(stderr, "Bad map char.\n");
        exit(5);
    }
}

/* Checks that the map is the right size */
void check_map_size(MapInfo info, int row) {
    if(row!=info.xDim) {
        fprintf(stderr, "Map file is the wrong size.\n");
        exit(6);
    }
}