#include <stdio.h>
#include <string.h>
#include <time.h>
#include "veb.c"

int compareFiles(FILE *f1, FILE *f2)
{
    fseek(f1, 0, SEEK_SET);
    fseek(f2, 0, SEEK_SET);
    char ch1 = getc(f1);
    char ch2 = getc(f2);
    int line = 1;
  
    while (ch1 != EOF && ch2 != EOF)
    {
        if (ch1 == '\n' && ch2 == '\n') line++;
        if (ch1 != ch2) return line;
        ch1 = getc(f1);
        ch2 = getc(f2);
    }
    return 0;
  
}

int ms_time(clock_t start, clock_t end) {
    return (int)((double) (end - start)) / (CLOCKS_PER_SEC / 1000);
}

int main(int argc, char const *argv[])
{
    int current_test = 0;
    int successfull_tests = 0;
    int failed_tests = 0;

    while (1)
    {
        char in_file_path[40];
        char out_file_path[40];
        char result_path[40];
        int creation_time, commands_time, destruction_time;
        clock_t start_time, end_time;
        sprintf(in_file_path, "tests/%i/in.txt", current_test);
        sprintf(out_file_path, "tests/%i/out.txt", current_test);
        FILE* in_file = fopen(in_file_path, "r");
        if (!in_file) break;
        sprintf(result_path, "results/%i.txt", current_test);
        FILE* result_file = fopen(result_path, "r+w");
        
        unsigned int tree_size;
        fscanf(in_file, "%u", &tree_size);
        start_time = clock();
        veb* v = create_veb(tree_size);
        end_time = clock();
        if (!v)
        {
            printf("Test %d: failed to create tree of size %d\n", current_test, tree_size);
            break;
        }
        
        creation_time = (ms_time(start_time, end_time));
        char command_type;
        start_time = clock();
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
            default:
                break;
            }
            // print_veb_contents(v);
            // printf("\n");
        }
        end_time = clock();
        commands_time = (ms_time(start_time, end_time));
        start_time = clock();
        destroy_veb(v);
        end_time = clock();
        destruction_time = (ms_time(start_time, end_time));
        FILE* out_file = fopen(out_file_path, "r");
        int wrong_line = compareFiles(result_file, out_file);

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
    return 0;
}
