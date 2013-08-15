#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int xPos;
    int yPos;
} Point;

typedef struct {
    int alph[26];
} Alphabet;

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
int check_letters(Alphabet);
MapInfo read_file(MapInfo, char *);
char** create_array(MapInfo);
char** populate_array(MapInfo, char *);
char reflector_nesw(char);
char reflector_nwse(char);
char reflector(char);
Point launchpad(Point, char);
int check_input(char, int, MapInfo);

int main(int argc, char *argv[]) {
    int maxSteps;
    MapInfo info;
    int currentSteps = 0;

    /* Check for correct number of inputs */
    if(argc == 2) {
        maxSteps = 10;
    } else if(argc == 3) {
        maxSteps = atoi(argv[2]);
        if(maxSteps>=1000 || maxSteps<=0) {
            fprintf(stderr, "Bad max steps.\n");
            return 2;
        }
    } else {
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
            if(currentSteps>=maxSteps) {
                printf("End of simulation.\n");
                break;
            }
            if(info.currentPos.xPos < 0 || info.currentPos.xPos >= info.xDim 
                || info.currentPos.yPos < 0 || 
                info.currentPos.yPos >= info.yDim) {
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

void print_map(MapInfo info) {
    /* Print the map to screen */ 
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

MapInfo start_sim(MapInfo info) {
    /* Take input where to start */
    char side, buffer[7];
    int sidePos, startRow, startCol;
    printf("(side pos)>");
    fgets(buffer, 7, stdin);
    sscanf(buffer, "%c%d", &side, &sidePos);

    if(feof(stdin)) {
        exit(0);
    }

    if(side == '\n') {
        info = start_sim(info);
        return info;
    }

    if(!check_input(side, sidePos, info)) {
        info = start_sim(info);
        return info;
    }

    /* Take one from sidepos as array starts at 0 */
    sidePos--;

    /* Drop ball in at startPos */
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

MapInfo check_square(MapInfo info) {
    int currentX = info.currentPos.xPos;
    int currentY = info.currentPos.yPos;
    char square = info.map[currentX][currentY];
    Point newPoint;

    if(square >= '1' && square < '9') {
        info.map[currentX][currentY] = ++square;
    } else if(square >= 'A' && square <= 'Z') {
        newPoint = search_map(info, ++square);
        if(newPoint.xPos == -1) {  /* Next char isn't found on map */
            newPoint = search_map(info, 'A');
            info.currentPos.xPos = newPoint.xPos;
            info.currentPos.yPos = newPoint.yPos;
            return info;
        } else { /* Next char is found */
            info.currentPos.xPos = newPoint.xPos;
            info.currentPos.yPos = newPoint.yPos;
            return info;
        }
    } else {
        switch(square) {
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

Point search_map(MapInfo info, char searchChar) {
    int i;
    int j;
    char onSquare;
    Point newPoint;

    for(i = 0; i<info.xDim; i++) {
        for(j = 0; j<info.yDim; j++) {
            onSquare = info.map[i][j];
            if((onSquare == searchChar)) {
                newPoint.xPos = i;
                newPoint.yPos = j;
                return newPoint; /* Position of searched character */
            }
        }
    }
    newPoint.xPos = -1;
    newPoint.yPos = -1;
    return newPoint;
}

MapInfo read_file(MapInfo info, char *filename) {
    /* Open file and store map dimensions */
    FILE *file;
    int testVar;
    char middleChar;

    file = fopen(filename, "r");
    
    if(file == 0) {
        fprintf(stderr, "Missing map file.\n");
        exit(3);
    } else {
        testVar = fscanf(file, "%d%c%d", &info.xDim, &middleChar, &info.yDim);
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

char** populate_array(MapInfo info, char *filename) {
    /* Store chars to array */
    FILE *file;
    int row = -1, column = 0, rowCount = 0, j, firstLinePassed = 0;
    char ch;
    Alphabet alph;
    for(j = 0; j<26; j++) {
        alph.alph[j] = 0;
    }
    file = fopen(filename, "r");
    while((ch = fgetc(file)) != EOF) {
        if(!firstLinePassed && ch!='\n') {
            continue;
        } else {
            firstLinePassed = 1;
        }
        if((ch>='A' && ch<='Z')) { /* Check if a letter */
            if (alph.alph[ch-65]) {
                fprintf(stderr, "Missing letters.\n");
                exit(7);
            } else {
                alph.alph[ch-65] = 1;
            }
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
                if(!(ch=='.' || ch=='/' || ch=='\\' || ch=='@' || ch=='=' || 
                    (ch>='A' && ch<='Z'))) {
                    fprintf(stderr, "Bad map char.\n");
                    exit(5);
                }
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
    if(!check_letters(alph)) {
        fprintf(stderr, "Missing letters.\n");
        exit(7);
    }
    /* Check map ended the right size */
    if(row!=info.xDim) {
        fprintf(stderr, "Map file is the wrong size.\n");
        exit(6);
    }
    return info.map;
}

/* Returns 1 if all letters valid, 0 otherwise */
int check_letters(Alphabet alph) { 
    int noEarlyLetter = 0;
    int i;
    int letterCount = 0;

    for(i = 0; i<26; i++) {
        if(alph.alph[i]) {
            letterCount++;
        }
        if(!alph.alph[i] && !noEarlyLetter) {
            noEarlyLetter = 1;
        }
        if(alph.alph[i] && noEarlyLetter) {
            return 0;
        }
    }
    if(!letterCount || letterCount>1) {
        return 1;
    } else {
        return 0;
    }
}

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