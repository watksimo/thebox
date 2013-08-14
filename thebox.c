#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int xPos;
    int yPos;
} point;

typedef struct {
    char** map;
    int xDim;
    int yDim;
    char direction;
    int currentXPos;
    int currentYPos;
} mapInfo;

void printMap(mapInfo);
mapInfo startSim(mapInfo);
mapInfo checkSquare(mapInfo);
mapInfo moveBall(mapInfo);
point searchMap(mapInfo, char);

int main(int argc, char *argv[]) {
    int maxSteps;
    mapInfo info;

    /* Check for correct number of inputs */
    if(argc == 2) {
        maxSteps = 10;
    } else if(argc == 3) {
        maxSteps = atoi(argv[2]);
    } else {
        fprintf(stderr, "Usage: thebox mapfile [maxsteps]");
        return 1;
    }

    /* Main game loop */
    int playing = 1;
    int first = 1;
    while(1) {

        /* Open file and store map dimensions */
        FILE *file;
        file = fopen(argv[1], "r");
        
        if(file == 0) {
            fprintf(stderr, "Missing map file.\n");
            return 3;
        } else {
            fscanf(file, "%d %d", &info.xDim, &info.yDim);
        }
        
        if(first) {
            /* Create 2d array of chars */
            int i;
            info.map = (char**) malloc(info.xDim*sizeof(char*));
            for(i=0; i<info.xDim; i++) {
                info.map[i] = (char*) malloc(info.yDim*sizeof(char));
            }
            first = 0;
        }
       
        /* Store chars to array */
        int row = -1;
        int column = 0;
        char ch;
		int rowCount = 0;
		char nextLetter = '*';
        while((ch=fgetc(file)) != EOF) {
			if((ch>='A' && ch<='Z')) {
				if(nextLetter=='*') {
					nextLetter = ++ch;
				} else {
					if(ch!=nextLetter) {
						fprintf(stderr, "Missing letters.\n");
						return 7;
					}
				}
			}
            if(row != -1) {
                if(ch == '\n') {
                    row++;
                    column = 0;
					rowCount++;
					if(rowCount>info.xDim) {
						fprintf(stderr, "Map file is the wrong size.\n");
						return 4;
					}
                } else {
					if(!(ch=='.' || ch=='/' || ch=='\\' || ch=='@' || ch=='=' || (ch>='A' && ch<='Z'))) {
						fprintf(stderr, "Bad map char.\n");
						return 5;
					}
                    info.map[row][column] = ch;
                    column++;
					if(column > info.yDim) {
						fprintf(stderr, "Map file is the wrong size.\n");
						return 4;
					}
                }
            } else {
                row++;
            }
        }

        printMap(info);
        info = startSim(info);
        printMap(info);
        int currentSteps = 0;
        while(playing) {
            if(currentSteps>maxSteps) {
                break;
            }
            info = checkSquare(info);
            info = moveBall(info);
            currentSteps++;
            printMap(info);
            printf("info.xDim: %d\n", info.currentXPos); /* DEBUGGING!!!!!!!!!!! */
            if(info.currentXPos < 0 || info.currentXPos >= info.xDim 
                || info.currentYPos < 0 || info.currentYPos > info.yDim) {
                break;
            }
            printf("CurrentXPos: %d\n", info.currentXPos); /* DEBUGGING!!!!!!!!!!! */
        }
    }

    return 0;

}

void printMap(mapInfo info) {
    /* Print the map to screen */ 
    int row1;
    int column1;
    for(row1=0; row1<info.yDim; row1++) {
        for(column1=0; column1<info.xDim; column1++ ) {
            printf("%c", info.map[row1][column1]);
        }
    printf("%c", '\n');
    }
    printf("%c", '\n');
    
}

mapInfo startSim(mapInfo info) {
    /* Take input where to start */
    char side;
    int sidePos;
    char leftOver;
    printf("(Side Pos)> ");
    scanf("%c%d%c",&side,&sidePos, &leftOver);

    /* Take one from sidepos as array starts at 0 */
    sidePos--;

    /* Drop ball in at startPos */
    int startRow;
    int startCol;
	
    switch(side) {
        case 'N':
            startRow = 0;
            startCol = sidePos;
            info.direction = 'S';
            break;

        case 'E':
			printf("StartSim xDim: %d\n", info.xDim);
            startCol = info.xDim-1;
            startRow = sidePos;
            info.direction = 'W';
			printf("Intended Current Pos(startSim): %d, %d\n", startRow, startCol);
            break;

        case 'S':
            startRow = info.yDim-1;
            startCol = sidePos;
            info.direction = 'N';
            break;

        case 'W':
            startCol = 0;
            startRow = sidePos;
            info.direction = 'E';
            break;

        default:;

    }
    info.currentXPos = startRow;
    info.currentYPos = startCol;
	printf("Current Pos(startSim): %d, %d\n", info.currentXPos, info.currentYPos);
    return info;
}

mapInfo checkSquare(mapInfo info) {
    int currentX = info.currentXPos;
    int currentY = info.currentYPos;
    char square = info.map[currentX][currentY];
    point newPoint;
	

    if(square >= '1' && square < '9') {
        info.map[currentX][currentY] = ++square;
    } else if(square >= 'A' && square <= 'Z') {
        newPoint = searchMap(info, ++square);
        printf("New Point: %d, %d\n", newPoint.xPos, newPoint.yPos);
        if(newPoint.xPos == -1) {  /* Next char isn't found on map */
            newPoint = searchMap(info, 'A');
            info.currentXPos = newPoint.xPos;
            info.currentYPos = newPoint.yPos;
            return info;
        } else { /* Next char is found */
            info.currentXPos = newPoint.xPos;
            info.currentYPos = newPoint.yPos;
            return info;
        }
    } else {
        switch(square) {
            case '.':
                info.map[currentX][currentY] = '1';
                break;

            case '/':
                switch(info.direction) {
                    case 'N':
                        info.direction = 'E';
                        break;
                    case 'E':
                        info.direction = 'N';
                        break;
                    case 'S':
                        info.direction = 'W';
                        break;
                    case 'W':
                        info.direction = 'S';
                        break;
                }
                break;

            case '\\':
                switch(info.direction) {
                    case 'N':
                        info.direction = 'W';
                        break;
                    case 'E':
                        info.direction = 'S';
                        break;
                    case 'S':
                        info.direction = 'E';
                        break;
                    case 'W':
                        info.direction = 'N';
                        break;
                }
                break;

            case '=':
                switch(info.direction) {
                    case 'N':
                        info.direction = 'S';
                        break;
                    case 'E':
                        info.direction = 'W';
                        break;
                    case 'S':
                        info.direction = 'N';
                        break;
                    case 'W':
                        info.direction = 'E';
                        break;
                }
                break;

            case '@':
                switch(info.direction) {
                    case 'N':
                        info.currentXPos -= 4;
                        break;
                    case 'E':
                        info.currentYPos += 4;
                        break;
                    case 'S':
                        info.currentXPos += 4;
                        break;
                    case 'W':
                        info.currentYPos -= 4;
                        break;
                }
                break;
        }
    }
    return info;
}

mapInfo moveBall(mapInfo info) {
    switch(info.direction) {
        case 'N':
            info.currentXPos--;
            break;
        case 'E':
            info.currentYPos++;
            break;
        case 'S':
            info.currentXPos++;
            break;
        case 'W':
            info.currentYPos--;
            break;
    }
    return info;
}

point searchMap(mapInfo info, char searchChar) {
    int i;
    int j;
    char onSquare;
    point newPoint;

    for(i=0; i<info.xDim; i++) {
        for(j=0; j<info.yDim; j++) {
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