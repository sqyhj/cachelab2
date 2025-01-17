#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int is_valid;            // Cache line validity
    unsigned long long tag;  // Cache line tag
    int last_access_time;    // Last access time for LRU replacement policy
} CacheLine;

typedef struct {
    CacheLine* lines;  // Array of cache lines in this set
} CacheSet;

CacheSet* cache;
int set_bits, num_lines, block_bits;                    // Cache parameters
int hit_count = 0, miss_count = 0, eviction_count = 0;  // Tracking cache stats
int current_time = 0;                                   // Global counter to simulate time for LRU policy

void initialize_cache() {
    // Allocate memory for cache sets
    cache = (CacheSet*)malloc(sizeof(CacheSet) * (1 << set_bits));
    for (int i = 0; i < (1 << set_bits); i++) {
        // Allocate memory for cache lines in each set
        cache[i].lines = (CacheLine*)malloc(sizeof(CacheLine) * num_lines);
        for (int j = 0; j < num_lines; j++) {
            // Initialize all cache lines as invalid
            cache[i].lines[j].is_valid = 0;
        }
    }
}

void access_cache(unsigned long long address, char operation) {
    unsigned long long set_index = (address >> block_bits) & ((1 << set_bits) - 1);
    unsigned long long tag = address >> (set_bits + block_bits);
    CacheSet* set = &cache[set_index];
    int hit = 0;
    int empty_line_index = -1;
    int eviction_line_index = -1;
    unsigned long long min_time = (unsigned long long)-1;

    for (int i = 0; i < num_lines; i++) {
        CacheLine* line = &set->lines[i];
        if (line->is_valid) {
            if (line->tag == tag) {
                // Cache hit
                hit = 1;
                line->last_access_time = current_time++;  // Update access time
                hit_count++;
                break;
            }

            // Track the least recently used (LRU) line for potential eviction
            if (line->last_access_time < min_time) {
                min_time = line->last_access_time;
                eviction_line_index = i;
            }
        } else {
            if (empty_line_index == -1) {
                empty_line_index = i;  // Found an empty line
            }
        }
    }

    if (!hit) {
        miss_count++;
        if (empty_line_index != -1) {
            // Use an empty line
            set->lines[empty_line_index].is_valid = 1;
            set->lines[empty_line_index].tag = tag;
            set->lines[empty_line_index].last_access_time = current_time++;  // Update access time
        } else {
            // Evict the least recently used (LRU) line
            eviction_count++;
            set->lines[eviction_line_index].tag = tag;
            set->lines[eviction_line_index].last_access_time = current_time++;  // Update access time
        }
    }
}

void process_trace_file(FILE* trace_file) {
    char operation;
    unsigned long long address;
    int size;
    int reg;

    while (fscanf(trace_file, " %c %llx,%d %d", &operation, &address, &size, &reg) == 4) {
        if (operation == 'L' || operation == 'S') {
            // Access cache for load and store operations
            access_cache(address, operation);
        }
    }
}

void print_summary(int hits, int misses, int evictions) {
    // Print summary to console
    printf("hits: %d misses: %d evictions: %d\n", hits, misses, evictions);

    // Save the summary to a file
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
    fclose(output_fp);
}

int main(int argc, char* argv[]) {
    set_bits = 0;
    num_lines = 0;
    block_bits = 0;
    char* trace_file_name = NULL;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'v') {
                // Verbose flag, can be used for debugging or detailed output
            } else if (argv[i][1] == 's') {
                set_bits = atoi(argv[++i]);
            } else if (argv[i][1] == 'E') {
                num_lines = atoi(argv[++i]);
            } else if (argv[i][1] == 'b') {
                block_bits = atoi(argv[++i]);
            } else if (argv[i][1] == 't') {
                trace_file_name = argv[++i];
            }
        }
    }

    // Initialize cache
    initialize_cache();

    // Open trace file for processing
    FILE* trace_file = fopen(trace_file_name, "r");
    if (trace_file == NULL) {
        perror("Error opening trace file");
        return 1;
    }

    // Process the trace file and simulate cache accesses
    process_trace_file(trace_file);

    // Close trace file
    fclose(trace_file);

    // Print the final summary of cache simulation
    print_summary(hit_count, miss_count, eviction_count);

    // Clean up memory
    for (int i = 0; i < (1 << set_bits); i++) {
        free(cache[i].lines);
    }
    free(cache);

    return 0;
}
