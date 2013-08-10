#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char** map;
    int xDim;
    int yDim;
    char direction;
} mapInfo;

void printMap(mapInfo);
mapInfo startSim(mapInfo);

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

    /* Open file and store map dimensions */
    FILE *file;
    file = fopen(argv[1], "r");
    
    if(file == 0) {
        fprintf(stderr, "Missing map file\n");
        return 3;
    } else {
        fscanf(file, "%d %d", &info.xDim, &info.yDim);
    }
    
    /* Create 2d array of chars */
    int i;
    info.map = (char**) malloc(info.xDim*sizeof(char*));
    for(i=0; i<info.xDim; i++) {
        info.map[i] = (char*) malloc(info.yDim*sizeof(char));
    }
   
    /* Store chars to array */
    int row = -1;
    int column = 0;
    char ch;    
    while((ch=fgetc(file)) != EOF) {
        if(row != -1) {
            if(ch == '\n') {
                row++;
                column = 0;
            } else {
                info.map[row][column] = ch;
                column++;
            }
        } else {
            row++;
        }
    }
    
    printMap(info);

    info = startSim(info);

    /* Test prints */
    printf("Max steps: %d\n", maxSteps);
    printf("Dimensions: %d,%d\n", info.xDim, info.yDim);
    printf("%c\n", info.direction);

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
        printf("Direction: %c",'\n');
    }
}

mapInfo startSim(mapInfo info) {
    /* Take input where to start */
    char side;
    int sidePos;
    printf("(Side Pos)> ");
    scanf("%c%d",&side,&sidePos);

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
            startCol = info.xDim-1;
            startRow = sidePos;
            info.direction = 'W';
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
    return info;
}
