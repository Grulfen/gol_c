#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>

typedef struct {
        int x;
        int y;
        char *matrix;
} world;

world* create_world(int x, int y)
{
        world* w = malloc(sizeof(world));
        w->x = x;
        w->y = y;
        w->matrix = calloc(x*y, sizeof(char));
        return w;
}

void destroy_world(world *w)
{
        free(w->matrix);
        free(w);
}

void print_world(world *w)
{
        for(int y = 0; y < w->y; y++){
                for(int x = 0; x < w->x; x++){
                        if(w->matrix[y*w->y + x]){
                                putchar('#');
                        } else {
                                putchar('.');
                        }
                }
                putchar('\n');
        }
        putchar('\n');
}

void randomize_world(world* w)
{
        srand(time(NULL));
        for(int y = 0; y < w->y; y++){
                for(int x = 0; x < w->x; x++){
                        if(rand() < 0.5 * ((double)RAND_MAX + 1.0)){
                                w->matrix[y*w->y + x] = 0;
                        } else {
                                w->matrix[y*w->y + x] = 1;
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
                        u = (x + i) % w->x; // Use a cyclic world
                        v = (y + j) % w->y;
                        count += w->matrix[v*w->y + u];
                }
        }
        return count;
}

char update_cell(int x, int y, world *w)
{
        int num;
        num = num_neighbours(x, y, w);
        if(w->matrix[y*w->y + x]){ // Cell is alive
                if(num < 2 || num > 3){
                        return 0;
                } 
        } else { // Cell is dead
                if(num == 3){
                        return 0;
                }
        }
        return w->matrix[y*w->y + x];
}

void update_world(world *w)
{
        // Create temporary matrix to store the old data
        int size = w->x*w->y*sizeof(char);
        char* old = malloc(size);
        memcpy(old, w->matrix, size);
        for(int y = 0; y < w->y; y++){
                for(int x = 0; x < w->x; x++){
                       old[y*w->y + x] = update_cell(x, y, w); 
                }
        }
        memcpy(w->matrix, old, size);
        free(old);
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

        print_world(w);
        update_world(w);
        putchar('\n');
        print_world(w);

        destroy_world(w);
        return 0;
}
