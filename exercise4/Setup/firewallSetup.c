#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

struct rule_list {
  char *device;
  int size;
  int port;
  struct rule_list *next;
};

static void mem_dealloc(struct rule_list *rules){
    struct rule_list *temp;

    while(rules){
        temp = rules;
        rules = rules->next;
        free(temp->device);
        free(temp);
    }
}

int main (int argc, char **argv) {

    FILE *rules_file; /* rules file descriptor */
    int fd, port, num_vars, num_bytes_read;
    int checked = 0;
    char *file_line = NULL;
    char *rule, *filename; 
    struct rule_list *rules = NULL;
    struct rule_list *head = NULL;
    struct rule_list *temp = NULL;
    size_t len = 0;

    /* ioctl  can be performed only on opened device */
    fd = open ("/proc/firewallExtension", O_RDWR);
    if (fd < 0) {
        fprintf (stderr, "Could not open file /proc/firewallExtension, exiting!\n");
        exit (1);
    }

    switch(argc){
      case 2:
        read(fd, NULL, 0);
        break;

      case 3:

        filename = argv[2];
        rules_file = fopen(filename, "r+");
        if (!rules_file) {
            fprintf (stderr, "Could not open file %s, exiting!\n", filename);
            exit (1);
        }

        while ((num_bytes_read = getline(&file_line, &len, rules_file)) != -1) {
            rule = calloc(num_bytes_read, sizeof(char));
            num_vars = sscanf(file_line, "%d %s\n", &port, rule);

            if(num_vars != 2) {
                fprintf(stderr, "ERROR: Ill-formed file\n");
                free(file_line);
                free(rule);
                mem_dealloc(head);
                fclose (rules_file);
                exit(1);
            }

            if(access(rule, F_OK) == -1 ) {
                fprintf(stderr, "ERROR: Ill-formed file\n");
                free(file_line);
                free(rule);
                mem_dealloc(head);
                fclose (rules_file);
                exit(1);
            }

            if(access(rule, X_OK) == -1) {
                fprintf(stderr, "ERROR: Cannot execute file\n");
                free(file_line);
                free(rule);
                mem_dealloc(head);
                fclose (rules_file);
                exit(1);
            }

            temp = malloc(sizeof(struct rule_list));
            if(!temp){
                fprintf(stderr, "Error! Failed to malloc\n");
                free(file_line);
                free(rule);
                mem_dealloc(head);
                fclose (rules_file);
                exit(1);
            }

            temp->port = port;
            temp->device = rule;
            temp->size = strlen(rule);
            temp->next = NULL;

            if(!checked){
                rules = temp;
                head = temp;
                checked=1;
                continue;
            }

            rules->next = temp;
            rules = rules->next;
        }

        write(fd, head, sizeof(head));

        fclose (rules_file);
        free(file_line);
        mem_dealloc(head);
    
        break;

      default:
        printf("ERROR: file does not exist\n");
    }

    close(fd);
    return 0;
}

