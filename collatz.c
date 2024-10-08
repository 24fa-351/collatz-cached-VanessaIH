#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//tend to not write comments, working on adding more for study purposes later on
//comments are mainly added at the end(orginal comments are more abstract and error code)

//typedef is struct MyStruct(idea is still a bit confusing)
typedef struct cachenode {
    int number_of_callatz_steps;
    int numbers_that_reach_to_one;
    struct cachenode* next;
    struct cachenode* previous;
}   cachenode;

typedef struct cache {
    cachenode** hash_table;
    int number_of_elements;
    int max_capacity;
    int hit_count;
    int miss_count;
    cachenode* head;
    cachenode* tail;
} cache;

cache* making_a_new_cach(int max_capacity) {
    if (max_capacity <= 0) {
        return NULL;
    }
    //malloc is memory allocation, allocing the size of the cache
    cache* new_cache = (cache*)malloc(sizeof(cache));

    if(!new_cache) {
        return NULL;
    }
    new_cache->hash_table = (cachenode**)malloc(sizeof(cachenode*) * max_capacity);
    for (int i = 0; i < max_capacity; i++) {
        new_cache->hash_table[i] = NULL;
    }
    new_cache->number_of_elements = 0;
    new_cache->max_capacity = max_capacity;
    new_cache->hit_count = 0;
    new_cache->miss_count = 0;
    new_cache->head = NULL;
    new_cache->tail = NULL;
    return new_cache;
}

//gdb signalled arithmetic exception, adding in checkers to kill system to satisfy
unsigned int hash(int number_of_callatz_steps, int max_capacity) {
    if (max_capacity == 0) {
        exit(EXIT_FAILURE);
    }
    return number_of_callatz_steps % max_capacity;
}

//replacement for fifo, realized when rereading the assignment...
//assumed to be allowed a choice when skimmping the section of the code
void removing_with_lru(cache* cache) {
    if (cache->tail == NULL) {
        return;
    }
    cachenode* old_cache = cache->tail;
    cache->tail = old_cache->previous;

    if (cache->tail != NULL) {
        cache->tail->next = NULL;
    } else {
        cache->head = NULL;
    }
    //reusing code from trashed fifo, errors arised to to different labels to fit new format
    unsigned int index = hash(old_cache->numbers_that_reach_to_one, cache->max_capacity);
    cachenode* new_node = cache->hash_table[index];
    cachenode* previous_node = NULL;
    while (new_node != NULL) {
        if (new_node->numbers_that_reach_to_one == old_cache->numbers_that_reach_to_one) {
            if (previous_node == NULL) {
                cache->hash_table[index] = new_node->next;
            } else {
                previous_node->next = new_node->next;
            }
            free(new_node);
            break;
        }
        previous_node = new_node;
        new_node = new_node->next;
    }
    cache->number_of_elements--;
    //created an extra node, clean up(not needed in rr)
    free(old_cache);
}

void removing_with_rr(cache* cache) {
    if (cache->number_of_elements == 0) {
        return;
    }
    int random_index = rand() % cache->max_capacity;
    cachenode* new_node = cache->hash_table[random_index];
    if (new_node == NULL) {
        return;
    }
    int count = 0;
    cachenode* holder_for_random_index_node = NULL;
    while (new_node != NULL) {
        if (rand() % (++count) == 0) {
            holder_for_random_index_node = new_node;
        }
    }
    new_node = new_node->next;
    //reusing code from removing_with_lru, changes in the labling
    if (holder_for_random_index_node != NULL) {
        unsigned int index = hash(holder_for_random_index_node->numbers_that_reach_to_one, cache->max_capacity);
        new_node = cache->hash_table[index];
        cachenode* previous_node = NULL;
        while (new_node != NULL) {
            if (new_node != NULL) {
                if (previous_node == NULL) {
                    cache->hash_table[index] = new_node->next;
                } else {
                    previous_node->next = new_node->next;
                }
                free(new_node);
                cache->number_of_elements--;
                return;
            }
            previous_node = new_node;
            new_node = new_node->next;
        }
    }
}

void move_to_the_front_for_LRU(cache* cache, cachenode* node) {
    if (node == cache->head) {
        return;
    } else if (node == cache->tail) {
        cache->tail = node->previous;
        cache->tail->next = NULL;
    } else {
        node->previous->next = node->next;
        if (node->next != NULL) {
            node->next->previous = node->previous;
        }
    }
    node->next = cache->head;
    node->previous = NULL;
    if (cache->head != NULL) {
        cache->head->previous = node;
    }
    cache->head = node;
    if (cache->tail == NULL) {
        cache->tail = node;
    }
}

void deleting_a_cache(cache* cache) {
        for (int i = 0; i < cache->max_capacity; i++) {
            cachenode* node = cache->hash_table[i];
            while (node) {
                cachenode* temp_for_node = node;
                node = node->next;
                free(temp_for_node);
            }
        }
        while (cache->head != NULL) {
            cachenode* temporary = cache->head;
            cache->head = cache->head->next;
            free(temporary);
        }
        free(cache->hash_table);
        free(cache);
}
 //errors appeared due to referencing number of steps not numbers that reach to one
void put_cache(cache* cache, int number_of_callatz_steps, int numbers_that_reach_to_one, char* wanted) {
    if (cache->number_of_elements >= cache->max_capacity) {
        if (wanted[0] == 'l') {
            removing_with_lru(cache);
        } else {
            removing_with_rr(cache);
        }
    }

    unsigned int index = hash(number_of_callatz_steps, cache->max_capacity);
    cachenode* new_node = (cachenode*)malloc(sizeof(cachenode));
    
    new_node->number_of_callatz_steps = number_of_callatz_steps;
    //error was happening here
    new_node->numbers_that_reach_to_one = numbers_that_reach_to_one;
    new_node->next = cache->hash_table[index];
    cache->hash_table[index] = new_node;
    cache->number_of_elements++;

    new_node->previous = NULL;
    new_node->next = cache->head;
    if (cache->head != NULL) {
        cache->head->previous = new_node;
    }
    cache->head = new_node;
    if(cache->tail == NULL) {
        cache->tail = new_node;
    }
}

int get_cache(cache* cache, int number_of_callatz_steps) {
    unsigned int index = hash(number_of_callatz_steps, cache->max_capacity);
    cachenode* node = cache->hash_table[index];
    while (node != NULL) {
        if (node->number_of_callatz_steps == number_of_callatz_steps) {
            cache->hit_count++;
            move_to_the_front_for_LRU(cache, node);
            return node->numbers_that_reach_to_one;
        }
        node = node->next;
    }
    cache->miss_count++;
    return -1;
}

int calculcating_steps(int n) {
    int number_of_steps = 0;
    while (n != 1) {
        if (n % 2 == 0) {
            n = n / 2;
        } else {
            n = 3 * (n + 1);
        }
        number_of_steps++;
    }
    return number_of_steps;
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        printf("Not enough arguements.");
        return 1;
    }

    int N = atoi(argv[1]);
    int MIN = atoi(argv[2]);
    int MAX = atoi(argv[3]);
    int size_of_cache = atoi(argv[4]);
    char* function = argv[5];

    if(size_of_cache <= 0) {
        return 1;
    }
    cache* cache = making_a_new_cach(size_of_cache);
    double total_time = 0;

    if (!cache) {
        return 1;
    }
    srand(time(NULL));

    for (int i = 0; i < N; i++) {
        //formating of a random number to calc steps
        int random_number = rand() % (MAX - MIN + 1) + MIN;
        //CPU clock ticks is in clock_t(can hold large numbers)
        clock_t start = clock();

        int steps = get_cache(cache, random_number);
        if (steps == -1) {
            steps = calculcating_steps(random_number);
            put_cache(cache, random_number, steps, function);
        }
        //clocks_per_sec converts and (double) makes the format into doubles
        total_time += (double)(clock() - start) / CLOCKS_PER_SEC;
        //could be one line, but this is personal preferred formatting
        printf("Random Number: %d\n", random_number);
        printf("Steps: %d\n", steps);
    }
    double mean_time = total_time / N;
    double cache_hit_count = (double)cache->hit_count / (cache->hit_count + cache->miss_count) * 100;
    printf("Meantime: %.3f seconds\n", mean_time);
    printf("Cache Hit Count: %.3f%%\n", cache_hit_count);

    deleting_a_cache(cache);
    return 0;
}