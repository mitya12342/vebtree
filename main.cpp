#include <stdio.h>
#include <string.h>
#include <time.h>
#include "veb.c"
#include <random>
#include <chrono>
#include <vector>
#include <numeric>
#include <algorithm>
#include <set>
#include <unordered_set>

#define requests 1000000
#define comparison_steps 10

using namespace std;

unsigned int random_requests_array[requests];
mt19937 mt;

void fill_requests_array(unsigned int u, unsigned int* array){
    uniform_int_distribution<unsigned int> distrib(0, u-1);
    for (unsigned int i = 0; i < requests; i++)
    {
        array[i] = distrib(mt);
    }
}

int compare_files(FILE *f1, FILE *f2)
{
    fseek(f1, 0, SEEK_SET);
    fseek(f2, 0, SEEK_SET);
    char ch1 = getc(f1);
    char ch2 = getc(f2);
    int line = 1;
  
    while (!(ch1 == EOF && ch2 == EOF))
    {
        if (ch1 == '\n') line++;
        ch1 = getc(f1);
        ch2 = getc(f2);
        if (ch1 != ch2) return line;
    }
    return 0;
  
}

void veb_fill_random(veb* v, unsigned int n) {
    uniform_int_distribution<unsigned int> distrib(0, v->u-1);
    for (unsigned int i = 0; i < n; i++)
    {
        unsigned int num;
        while (1)
        {
            num = distrib(mt);
            if (!veb_tree_member(v, num))
            {
                veb_tree_insert(v, num);
                break;
            }
        }
    }
}

double veb_random_member_time(veb* v, unsigned int* random) {
    auto start = chrono::high_resolution_clock::now();
    for (unsigned int i = 0; i < requests; i++)
    {
        veb_tree_member(v, random[i]);
    }
    auto stop = chrono::high_resolution_clock::now();
    return chrono::duration<double, nano>(stop-start).count()/requests;
}

double veb_random_successor_time(veb* v, unsigned int* random) {
    auto start = chrono::high_resolution_clock::now();
    for (unsigned int i = 0; i < requests; i++)
    {
        veb_tree_successor(v, random[i]);
    }
    auto stop = chrono::high_resolution_clock::now();
    return chrono::duration<double, nano>(stop-start).count()/requests;
}

double veb_random_predecessor_time(veb* v, unsigned int* random) {
    auto start = chrono::high_resolution_clock::now();
    for (unsigned int i = 0; i < requests; i++)
    {
        veb_tree_predecessor(v, random[i]);
    }
    auto stop = chrono::high_resolution_clock::now();
    return chrono::duration<double, nano>(stop-start).count()/requests;
}

int main(int argc, char const *argv[])
{
    printf("Comparison tests\n");
    int current_test = 0;
    int successfull_tests = 0;
    int failed_tests = 0;

    while (1)
    {
        char in_file_path[40];
        char out_file_path[40];
        char result_path[40];
        unsigned int creation_time, commands_time, destruction_time;
        sprintf(in_file_path, "tests/%i/in.txt", current_test);
        sprintf(out_file_path, "tests/%i/out.txt", current_test);
        FILE* in_file = fopen(in_file_path, "r");
        if (!in_file) break;
        sprintf(result_path, "results/%i.txt", current_test);
        FILE* result_file = fopen(result_path, "w");
        
        unsigned int tree_upper_bound;
        fscanf(in_file, "%u", &tree_upper_bound);
        auto start_time = chrono::high_resolution_clock::now();
        veb* v = create_veb(tree_upper_bound);
        auto end_time = chrono::high_resolution_clock::now();
        if (!v)
        {
            printf("Test %d: failed to create tree of size %d\n", current_test, tree_upper_bound);
            exit(1);
        }
        
        creation_time = chrono::duration_cast<chrono::milliseconds>(end_time-start_time).count();
        char command_type;
        start_time = chrono::high_resolution_clock::now();
        while (fscanf(in_file, "\n%c", &command_type) != EOF)
        {
            unsigned int command_argument;
            
            if (!(command_type == 'a' || command_type == 'b')) {
                fscanf(in_file, "%u", &command_argument);
            }
            switch (command_type)
            {
            case 'i':
                veb_tree_insert(v, command_argument);
                break;
            case 'c':
            {
                unsigned int result = veb_tree_member(v, command_argument);
                fprintf(result_file, "%u\n", result);
            }
                break;
            case 's':
            {
                unsigned int result = veb_tree_successor(v, command_argument);
                if (result == NIL) {
                    fprintf(result_file, "NIL\n");
                } else {
                    fprintf(result_file, "%u\n", result);
                }
            }
                break;
            case 'p':
            {
                unsigned int result = veb_tree_predecessor(v, command_argument);
                if (result == NIL) {
                    fprintf(result_file, "NIL\n");
                } else {
                    fprintf(result_file, "%u\n", result);
                }
            }
                break;
            case 'b':
            {
                unsigned int result = veb_tree_minimum(v);
                if (result == NIL) {
                    fprintf(result_file, "NIL\n");
                } else {
                    fprintf(result_file, "%u\n", result);
                }
            }
                break;
            case 'a':
            {
                unsigned int result = veb_tree_maximum(v);
                if (result == NIL) {
                    fprintf(result_file, "NIL\n");
                } else {
                    fprintf(result_file, "%u\n", result);
                }
            }
                break;
            case 'd':
                veb_tree_delete(v, command_argument);
                break;
            default:
                break;
            }
            // print_veb_contents(v);
            // printf("\n");
        }
        end_time = chrono::high_resolution_clock::now();
        commands_time = chrono::duration_cast<chrono::milliseconds>(end_time-start_time).count();
        start_time = chrono::high_resolution_clock::now();
        destroy_veb(v);
        end_time = chrono::high_resolution_clock::now();
        destruction_time = chrono::duration_cast<chrono::milliseconds>(end_time-start_time).count();
        FILE* out_file = fopen(out_file_path, "r");
        fclose(result_file);
        result_file = fopen(result_path, "r");
        int wrong_line = compare_files(result_file, out_file);

        fclose(in_file);
        fclose(out_file);
        fclose(result_file);

        printf("Test %d: ", current_test);
        if (wrong_line) {
            failed_tests++;
            printf("FAIL at command %d | ", wrong_line);
        } else
        {
            successfull_tests++;
            printf("OK | ");
        }
        printf("Creation: %dms, Test: %dms, Destruction: %dms", creation_time, commands_time, destruction_time);
        printf("\n");
        current_test++; 
    }
    printf("OK: %d, FAIL: %d \n", successfull_tests, failed_tests);

    printf("Performance tests\n");

    FILE* veb_perf_file = fopen("perf.txt", "w");

    for (int i = 17; i <= 24; i++)
    {
        mt.seed(42);
        unsigned int tree_upper_bound = 1U<<i;
        printf("2^%d\n", i);
        fprintf(veb_perf_file, "%d", tree_upper_bound);
        fill_requests_array(tree_upper_bound, random_requests_array);

        auto start_time = chrono::high_resolution_clock::now();
        veb* a = create_veb(tree_upper_bound);
        auto end_time = chrono::high_resolution_clock::now();
        if (a == NULL) {
            printf("\nFailed to create tree of size 2^%d\n", i);
            exit(1);
        };
        int time = chrono::duration_cast<chrono::milliseconds>(end_time-start_time).count();
        fprintf(veb_perf_file, " %d", time);

        // start_time = chrono::high_resolution_clock::now();
        veb_fill_random(a, 1000);
        fprintf(veb_perf_file, " %f", veb_random_member_time(a, random_requests_array));
        fprintf(veb_perf_file, " %f", veb_random_predecessor_time(a, random_requests_array));
        fprintf(veb_perf_file, " %f", veb_random_successor_time(a, random_requests_array));
        veb_fill_random(a, 9000);
        fprintf(veb_perf_file, " %f", veb_random_member_time(a, random_requests_array));
        fprintf(veb_perf_file, " %f", veb_random_predecessor_time(a, random_requests_array));
        fprintf(veb_perf_file, " %f", veb_random_successor_time(a, random_requests_array));
        veb_fill_random(a, 90000);
        fprintf(veb_perf_file, " %f", veb_random_member_time(a, random_requests_array));
        fprintf(veb_perf_file, " %f", veb_random_predecessor_time(a, random_requests_array));
        fprintf(veb_perf_file, " %f", veb_random_successor_time(a, random_requests_array));

        // end_time = chrono::high_resolution_clock::now();
        start_time = chrono::high_resolution_clock::now();
        destroy_veb(a);
        end_time = chrono::high_resolution_clock::now();
        time = chrono::duration_cast<chrono::milliseconds>(end_time-start_time).count();
        fprintf(veb_perf_file, " %d", time);

        fprintf(veb_perf_file, "\n");
    }
    fclose(veb_perf_file);

    printf("Performance comparison\n");
    FILE* veb_perf_comp_file = fopen("perf_comp.txt", "w");
    printf("Preparing unique random numbers\n");
    mt.seed(321);
    vector<unsigned int> unique_random(1<<26);
    iota(unique_random.begin(), unique_random.end(), 0);
    shuffle(unique_random.begin(), unique_random.end(), mt);
    printf("Preparing random requests\n");
    mt.seed(123);
    fill_requests_array(1<<26, random_requests_array);
    printf("Creating 2^26 veb tree \n");
    veb* test_veb = create_veb(1<<26);
    printf("Testing veb tree\n");
    unsigned int step_size = (1<<26)/comparison_steps;
    mt.seed(42);
    for (int i = 0; i < comparison_steps; i++)
    {
        for (unsigned int j = 0; j < step_size; j++)
        {
            veb_tree_insert(test_veb, unique_random[step_size*i+j]);
        }
        fprintf(veb_perf_comp_file, "%d %f %f %f\n",
                                    step_size*(i+1),
                                    veb_random_member_time(test_veb, random_requests_array),
                                    veb_random_successor_time(test_veb, random_requests_array),
                                    veb_random_predecessor_time(test_veb, random_requests_array));
        printf("%d\n", i);
    }
    fprintf(veb_perf_comp_file, "#\n");
    printf("Destroying veb tree \n");
    destroy_veb(test_veb);
    printf("Testing set\n");
    set<unsigned int> test_set;
    mt.seed(42);
    for (int i = 0; i < comparison_steps; i++)
    {
        for (unsigned int j = 0; j < step_size; j++)
        {
            test_set.insert(unique_random[step_size*i+j]);
        }
        auto start = chrono::high_resolution_clock::now();
        for (unsigned int j = 0; j < requests; j++)
        {
            test_set.find(random_requests_array[j]);
        }
        auto stop = chrono::high_resolution_clock::now();
        double find = chrono::duration<double, nano>(stop-start).count()/requests;
        start = chrono::high_resolution_clock::now();
        for (unsigned int j = 0; j < requests; j++)
        {
            test_set.upper_bound(random_requests_array[j]);
        }
        stop = chrono::high_resolution_clock::now();
        double successor = chrono::duration<double, nano>(stop-start).count()/requests;
        start = chrono::high_resolution_clock::now();
        for (unsigned int j = 0; j < requests; j++)
        {
            test_set.lower_bound(random_requests_array[j]);
        }
        stop = chrono::high_resolution_clock::now();
        double predecessor = chrono::duration<double, nano>(stop-start).count()/requests;
        fprintf(veb_perf_comp_file, "%f %f %f\n", find, successor, predecessor);
        printf("%d\n", i);
    }
    test_set.clear();
    fprintf(veb_perf_comp_file, "#\n");
    printf("Testing unordered set\n");
    unordered_set<unsigned int> test_unordered_set;
    mt.seed(42);
    for (int i = 0; i < comparison_steps; i++)
    {
        for (unsigned int j = 0; j < step_size; j++)
        {
            test_unordered_set.insert(unique_random[step_size*i+j]);
        }
        auto start = chrono::high_resolution_clock::now();
        for (unsigned int j = 0; j < requests; j++)
        {
            test_unordered_set.find(random_requests_array[j]);
        }
        auto stop = chrono::high_resolution_clock::now();
        fprintf(veb_perf_comp_file, "%f\n", chrono::duration<double, nano>(stop-start).count()/requests);
        printf("%d\n", i);
    }
    test_unordered_set.clear();

    
    return 0;
}
