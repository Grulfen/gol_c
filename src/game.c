#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<ncurses.h>
#include<unistd.h>

typedef struct {
        int x;
        int y;
        char *matrix;
} world;

// Create a new world initialized to 0
world* create_world(int x, int y)
{
        world* w = malloc(sizeof(world));
        w->x = x;
        w->y = y;
        w->matrix = calloc(x*y, sizeof(char));
        return w;
}

// Free the world
void destroy_world(world *w)
{
        free(w->matrix);
        free(w);
}

// Print world to stdout
void print_world(world *w)
{
        for(int y = 0; y < w->y; y++){
                for(int x = 0; x < w->x; x++){
                        if(w->matrix[y*w->x + x]){
                                putchar('#');
                        } else {
                                putchar('.');
                        }
                }
                putchar('\n');
        }
        putchar('\n');
}

// Randomize the world
void randomize_world(world* w)
{
        srand(time(NULL));
        for(int y = 0; y < w->y; y++){
                for(int x = 0; x < w->x; x++){
                        if(rand() < 0.7 * ((double)RAND_MAX + 1.0)){
                                w->matrix[y*w->x + x] = 0;
                        } else {
                                w->matrix[y*w->x + x] = 1;
                        }
                }
        }
}

int num_neighbours(int x, int y, world *w)
{
        int count = 0;
        int u, v;
        for(int i = -1; i < 2 ; i++){
                for(int j = -1; j < 2 ; j++){
                        if(!(i || j)){
                                continue;
                        }
                        u = (w->x + x + i) % w->x; // Use a cyclic world
                        v = (w->y + y + j) % w->y;
                        count += w->matrix[v*w->x + u];
                }
        }
        return count;
}

// Return the new value of a cell
char update_cell(int x, int y, world *w)
{
        int num;
        num = num_neighbours(x, y, w);
        if(w->matrix[y*w->x + x]){ // Cell is alive
                if(num < 2 || num > 3){
                        return 0;
                } 
        } else { // Cell is dead
                if(num == 3){
                        return 1;
                }
        }
        return w->matrix[y*w->x + x];
}

// Update the world one step
void update_world(world *w)
{
        // Create temporary matrix to store the old data
        int size = w->x*w->y*sizeof(char);
        char* old = malloc(size);
        memcpy(old, w->matrix, size);
        for(int y = 0; y < w->y; y++){
                for(int x = 0; x < w->x; x++){
                       old[y*w->x + x] = update_cell(x, y, w); 
                }
        }
        memcpy(w->matrix, old, size);
        free(old);
}

// Print to stdsrc (curses)
void print_curses(world *w)
{
        char c;
        mvaddch(0, 0, '\\');
        for(int i=1; i<w->x; i++)
                mvaddch(0, i, '=');
        for(int y = 0; y < w->y; y++){
                mvaddch(y + 1, 0, '|');
                for(int x = 0; x < w->x; x++){
                       c = w->matrix[y*w->x + x] ? '#' : ' ';
                       mvaddch(y + 1, x + 1, c);
                }
                if(y == 0){
                        mvaddch(0, w->x + 1, '/');
                }
                mvaddch(y + 1, w->x + 1, '|');
        }
        mvaddch(w->y + 1, 0, '/');
        for(int i=1; i<w->x; i++)
                mvaddch(w->y + 1, i, '=');
        mvaddch(w->y + 1, w->x + 1, '\\');
        refresh();
}

// Initialize curses window
void init_curses()
{
        initscr();
}

int main(int argc, char* argv[])
{
        if(argc != 3){
                printf("Usage: ./game x y\n");
                return 1;
        }
        int x = atoi(argv[1]);
        int y = atoi(argv[2]);
        world* w = create_world(x, y);
        randomize_world(w);

        init_curses();
        char c = 'n';
        while (c != 'q'){
                if(c == 'r'){
                        randomize_world(w);
                }
                print_curses(w);
                update_world(w);

                c = getch();
        }
        endwin();
        update_world(w);

        destroy_world(w);
        return 0;
}
