#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "generator.h"

/*
 * Given a direction, get its opposite
 *
 * Parameters:
 *  - dir: direction
 *
 * Returns:
 *  - the opposite direction to dir
 */
Direction get_opposite_dir(Direction dir) {
    if (dir == NORTH) {
        return SOUTH;
    } else if (dir == SOUTH) {
        return NORTH;
    } else if (dir == EAST) {
        return WEST;
    } else {
        return EAST;
    }
}

/*
 * Given an array of the four directions, randomizes the order of the directions
 *
 * Parameters:
 *  - directions: an array of Direction enums of size 4
 *
 * Returns:
 *  - nothing - the array should be shuffled in place
 */
void shuffle_array(Direction directions[]) {
    // include srand(time(NULL)) at beginning of main function 
    
    for (int i = 0; i < 4; i++) {
        Direction temp = directions[i];
        int random_num = rand() % (4);
        directions[i] = directions[random_num];
        directions[random_num] = temp; 
    }
}

/*
 * Recursive function for stepping through a maze and setting its
 *connections/walls using the drunken_walk algorithm
 *
 * Parameters:
 *  - row: row of the current room
 *  - col: column of the current room
 *  - num_rows: number of rows in the maze
 *  - num_cols: number of columns in the maze
 *  - maze: a 2D array of maze_room structs representing your maze
 * Returns:
 *  - nothing - the state of your maze should be saved in the maze array
 *being passed in (make sure to use pointers correctly!).
 */
void drunken_walk(int row, int col, int num_rows, int num_cols,
                  struct maze_room maze[num_rows][num_cols]) {
    
    // pointer for the current room you are in 
    struct maze_room *r = &maze[row][col]; 
    // set visited to true
    r->visited = 1; 
    // create array of four directions
    Direction directions[4] = {NORTH, SOUTH, EAST, WEST};
    // shuffle the array
    shuffle_array(directions); 
    // store walls where you would otherwise be walking out of bounds
    for (int i = 0; i < 4; i++) {
        Direction dir = directions[i];
        // locate the neighbor maze room for that given direction 
        struct maze_room *neighbor = 
        get_neighbor(num_rows, num_cols, maze, r, dir); 
        // if the neighbor is out of bounds 
        if (neighbor == 0) {
            r->walls[dir] = 1; 
        } else {
            // if neighbor has not yet been visited
            if (neighbor->visited == 0) {
                r->walls[dir] = 0;
                drunken_walk(neighbor->row, neighbor->column, num_rows, num_cols, 
                maze); 
            } else {
                Direction opposite_dir = get_opposite_dir(dir); 
                // if neighbor encounters a wall in opposite_dir
                if (neighbor->walls[opposite_dir] != -1) {
                    r->walls[dir] = neighbor->walls[opposite_dir];

                } else {
                    r->walls[dir] = 1;
                }
            }

        }
    }
}

/*
 * Represents a maze_room struct as an integer based on its connections
 *
 * Parameters:
 *  - room: a struct maze_room to be converted to an integer
 *
 * Returns:
 *  - the integer representation of room
 */

int power2(int i){
    if (i == 0) {return 1;}
    else {
        return 2*(power2(i-1));
    }
}

int encode_room(struct maze_room room) {
    int sum = 0;
    int power = 1;
    for (int i = 0; i < 4; i++){
        if (room.walls[i] == 1) {
            sum = sum + power;
        }
        power*=2;
    }
    return sum; 
}

/*
 * Represents a maze_room array as a hexadecimal array based on its connections
 *
 * Parameters:
 *  - num_rows: number of rows in the maze
 *  - num_cols: number of columns in the maze
 *  - maze: a 2D array of maze_room structs representing the maze
 *  - result: a 2D array of hexadecimal numbers representing the maze
 *
 * Returns:
 *  - nothing - each maze_room in the maze should be represented
 *    as a hexadecimal number and put in nums at the corresponding index
 */
void encode_maze(int num_rows, int num_cols,
                 struct maze_room maze[num_rows][num_cols],
                 int result[num_rows][num_cols]) {
    for (int i = 0; i < num_rows; i++) {
        for (int j = 0; j < num_cols; j++) {
          result[i][j] = encode_room(maze[i][j]);
        }
    }
}

/*
 * Writes encoded maze to an output file
 *
 * Parameters:
 *  - num_rows: number of rows in the maze
 *  - num_cols: number of columns in the maze
 *  - encoded_maze: a maze_room array as a hexadecimal array based on its
 *    connections
 *  - file_name: the name of the output file for the encoded maze
 *
 * Returns:
 *  - 1 if an error occurs, 0 otherwise
 */
int write_encoded_maze_to_file(int num_rows, int num_cols,
                               int encoded_maze[num_rows][num_cols],
                               char *file_name) {
    int err = 0;

    // open file (create it if necessary)
    FILE *f = fopen(file_name, "w+");
    if (f == NULL) {
        fprintf(stderr, "Error opening file.\n");
        return 1;
    }
    // write each room's hex value into file
    for (int i = 0; i < num_rows; i++) {
        for (int j = 0; j < num_cols; j++) {
            int encoded_room = encoded_maze[i][j];
            err = fprintf(f, "%x", encoded_room);
            if (err < 0) {
                fprintf(stderr, "Writing to file failed: %i\n", err);
                return 1;
            }
        }
        // add a newline between rows
        err = fprintf(f, "\n");
        if (err < 0) {
            fprintf(stderr, "Writing to file failed: %i\n", err);
            return 1;
        }
    }
    // close file
    int close = fclose(f);
    if (close == EOF) {
        fprintf(stderr, "Could not close file.\n");
        return 1;
    }
    return 0;
}

/*
 * Main function
 *
 * Parameters:
 *  - argc: the number of command line arguments - for this function 4
 *  - **argv: a pointer to the first element in the command line
 *            arguments array - for this function:
 *            ["generator", <output file>, <number of rows>, <number of
 *columns>]
 *
 * Returns:
 *  - 0 if program exits correctly, 1 if there is an error
 */

int main(int argc, char **argv) {
    srand(time(NULL));
    char *file_name;
    int num_rows;
    int num_cols;

    if (argc != 4) {
        printf("Incorrect number of arguments.\n");
        printf(
            "./generator <output file> <number of rows> <number of columns>\n");
        return 1;
    } else {
        file_name = argv[1];
        num_rows = atoi(argv[2]);
        num_cols = atoi(argv[3]);
    }
    if (num_rows <= 0 || num_cols <= 0) {
        return 1;
    }
    struct maze_room maze[num_rows][num_cols]; 
    initialize_maze(num_rows, num_cols, maze); 
    drunken_walk(0, 0, num_rows, num_cols, maze); 
    int result[num_rows][num_cols]; 
    encode_maze(num_rows, num_cols, maze, result); 
    write_encoded_maze_to_file(num_rows, num_cols, result, file_name);
    return 0; 
}
