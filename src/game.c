#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<time.h>
#include<string.h>
#include<ncurses.h>
#include<unistd.h>

// Struct for representing the world
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
        w->matrix = calloc(x*y, sizeof(char)); // Use calloc for initialized values
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
                        if(rand() < 0.7 * ((double)RAND_MAX + 1.0)){ // 70% of cells dead at start
                                w->matrix[y*w->x + x] = 0;
                        } else {
                                w->matrix[y*w->x + x] = 1;
                        }
                }
        }
}

// Return number of neighbours of cell in x,y
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
                        v = (w->y + y + j) % w->y; // Add w->x and w->y to avoid negative numbers
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
                if(num < 2 || num > 3){ // If less than 2 or more than 3 neighbours, cell dies
                        return 0;
                } 
        } else { // Cell is dead
                if(num == 3){ // If exactly 3 neighbours, cell turns alive
                        return 1;
                }
        }
        return w->matrix[y*w->x + x];
}

// Update the world one step
void update_world(world *w)
{
        // Size in bytes of the array that stores the world
        int size = w->x*w->y*sizeof(char);

        // Create temporary matrix to write the new world in
        char* new = malloc(size);
        memcpy(new, w->matrix, size);
        for(int y = 0; y < w->y; y++){
                for(int x = 0; x < w->x; x++){
                       new[y*w->x + x] = update_cell(x, y, w); 
                }
        }
        // free the old world
        free(w->matrix);

        // Use the new world
        w->matrix = new;
}

// Print to stdsrc (curses)
void print_curses(world *w)
{
        char c;
        attron(COLOR_PAIR(1));
        mvaddch(0, 0, '\\');
        for(int i=1; i<w->x; i++)
                mvaddch(0, i, '=');
        attroff(COLOR_PAIR(1));
        for(int y = 0; y < w->y; y++){
                attron(COLOR_PAIR(1));
                mvaddch(y + 1, 0, '|');
                attroff(COLOR_PAIR(1));
                attron(COLOR_PAIR(2));
                for(int x = 0; x < w->x; x++){
                       c = w->matrix[y*w->x + x] ? '#' : ' ';
                       mvaddch(y + 1, x + 1, c);
                }
                if(y == 0){
                        attron(COLOR_PAIR(1));
                        mvaddch(0, w->x + 1, '/');
                        attroff(COLOR_PAIR(1));
                }
                attron(COLOR_PAIR(1));
                mvaddch(y + 1, w->x + 1, '|');
                attroff(COLOR_PAIR(1));
        }
        attroff(COLOR_PAIR(2));
        attron(COLOR_PAIR(1));
        mvaddch(w->y + 1, 0, '/');
        for(int i=1; i<w->x; i++)
                mvaddch(w->y + 1, i, '=');
        mvaddch(w->y + 1, w->x + 1, '\\');
        attroff(COLOR_PAIR(1));
        refresh();
}

// Initialize curses window
void init_curses()
{
        initscr();
        noecho();
        raw();
        nodelay(stdscr, true);
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
}

int main(int argc, char* argv[])
{
        int x, y;
        if(argc != 3){
                x = 167;
                y = 42;
        } else {
                x = atoi(argv[1]);
                y = atoi(argv[2]);
        }

        struct timespec tim;
        tim.tv_sec = 0;
        tim.tv_nsec = 100000000L;

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
                nanosleep(&tim, NULL);
        }
        endwin();
        update_world(w);

        destroy_world(w);
        return 0;
}
