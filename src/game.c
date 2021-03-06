#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<time.h>
#include<string.h>
#include<curses.h>
#include<unistd.h>
#include<omp.h>

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
#pragma omp parallel for schedule(dynamic)
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
void print_curses(world *w, int color)
{
        char c;

        // Print border
        attron(COLOR_PAIR(1));
        mvaddch(0, 0, '\\');
        mvaddch(w->y + 1, w->x + 1, '\\');
        mvaddch(0, w->x + 1, '/');
        mvaddch(w->y + 1, 0, '/');
        for(int i=1; i<w->x + 1; i++){
                mvaddch(0, i, '=');
                mvaddch(w->y + 1, i, '=');
        }
        for(int j=1; j<w->y + 1; j++){
                mvaddch(j, 0, '|');
                mvaddch(j, w->x + 1, '|');
        }
        attroff(COLOR_PAIR(1));

        // Print world
        attron(COLOR_PAIR(color));
        for(int y = 0; y < w->y; y++){
                for(int x = 0; x < w->x; x++){
                       c = w->matrix[y*w->x + x] ? '#' : ' ';
                       mvaddch(y + 1, x + 1, c);
                }
        }
        attroff(COLOR_PAIR(color));
        refresh();
}

// Initialize curses window
void init_curses()
{
        initscr();
        keypad(stdscr, true);
        noecho();
        cbreak();
        nodelay(stdscr, true);
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_BLUE, COLOR_BLACK);
        init_pair(4, COLOR_YELLOW, COLOR_BLACK);
}

void run_test()
{
        double start_time = omp_get_wtime();
        world *w = create_world(200, 200);
        randomize_world(w);
        for(int i=0;i<300;i++){
                update_world(w); // Update the world
        }
        double end_time = omp_get_wtime();
        printf("Time: %.2f\n", end_time - start_time);
        destroy_world(w);
}

int main(int argc, char** argv)
{
        int opt;
        char test = 0;
        int threads = 4;
        while((opt = getopt(argc, argv, "t:")) != -1){
                switch(opt){
                        case 't':
                                test = 1;
                                threads = atoi(optarg);
                                break;
                        case '?':
                                exit(1);
                                break;
                }
        }

        omp_set_num_threads(threads);

        int color = 2;
        int row, col;
        long wait_inc = 10000000L;

        struct timespec tim;
        tim.tv_sec = 0;
        tim.tv_nsec = 10*wait_inc;

        world *w;
        if(test){ // Run test without curses
                run_test();
                exit(0);
        } 
        init_curses();
        getmaxyx(stdscr, row, col);
        w = create_world(col - 2, row - 2);
        randomize_world(w);

        int c = 'n';
        while (c != 'q'){


                switch (c) {
                        case 'r' : randomize_world(w);
                                   break;
                        case 'g' : color = 2; // Change color to green
                                   break;
                        case 'b' : color = 3; // Change color to blue
                                   break;
                        case 'y' : color = 4; // Change color to green
                                   break;
                        case KEY_UP:  // Make game run faster
                                   if(tim.tv_nsec < 30000000L){ // Avoid timer to wrap around
                                           tim.tv_nsec = 30000000L;
                                   } else {
                                           tim.tv_nsec -= wait_inc;
                                   }
                                   break;
                        case KEY_DOWN: // Make game run slower
                                   if(tim.tv_nsec > 400000000){ // Avoid timer to wrap around
                                           tim.tv_nsec = 400000000;
                                   } else {
                                           tim.tv_nsec += wait_inc;
                                   }
                                   break;
                        case KEY_RESIZE:
                                   // Terminal size changed
                                   // reallocate world to new size
                                   // New world is a little skewed :(
                                   getmaxyx(stdscr, row, col);
                                   w->x = col-2;
                                   w->y = row-2;
                                   w->matrix = realloc(w->matrix, w->x*w->y*sizeof(char));
                                   break;
                }
                print_curses(w, color); // Print world to screen
                update_world(w); // Update the world
                nanosleep(&tim, NULL);

                c = getch(); // Check if user input is available - non-blocking call
                flushinp(); // Flush the input queue to avoid lag (input in the input queue is discarded)

        }

        endwin();

        destroy_world(w);
        return 0;
}
