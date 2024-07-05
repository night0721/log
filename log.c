#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ftw.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <libgen.h>
#include <linux/limits.h>

#include "config.h"

char *get_log_dir() {
    char *xdg_data_home = getenv("XDG_DATA_HOME");
    char *dirname = "log";
    char *log_dir = NULL;
    if (xdg_data_home == NULL) {
        xdg_data_home = getenv("HOME");
        if (xdg_data_home == NULL) {
            fprintf(stderr, "log: HOME and XDG_DATA_HOME environment variable not set\n");
            exit(EXIT_FAILURE);
        }
    }
    log_dir = malloc(sizeof(char) * (strlen(xdg_data_home) + strlen(dirname) + 2));
    if (log_dir == NULL) {
        fprintf(stderr, "log: Error allocating memory\n");
        exit(EXIT_FAILURE);
    }
    sprintf(log_dir, "%s/%s", xdg_data_home, dirname);
    struct stat dir_stat;
    if (!((stat(log_dir, &dir_stat) == 0) && S_ISDIR(dir_stat.st_mode))) { // check defined path is directory
        if (mkdir(log_dir, S_IRWXU)) { // 700
            fprintf(stderr, "log: Cannot create log directory\n");
            exit(EXIT_FAILURE);
        }
    }
    return log_dir;
}

int compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void tree(const char *basepath, int depth) {
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basepath);

    if (!dir)
        return;

    char *files[1000];
    int file_count = 0;

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            files[file_count] = strdup(dp->d_name);
            file_count++;
        }
    }

    qsort(files, file_count, sizeof(char *), compare);

    for (int i = 0; i < file_count; i++) {
        for (int j = 0; j < depth - 1; j++) {
            printf("│   ");
        }

        if (depth > 0) {
            printf("├── ");
        }

        printf("%s\n", files[i]);

        strcpy(path, basepath);
        strcat(path, "/");
        strcat(path, files[i]);

        tree(path, depth + 1);

        free(files[i]);
    }

    closedir(dir);
}

int list_dir(const char *dir) {
    if (dir == NULL || *dir == '\0') {
        fprintf(stderr, "log: Invalid directory path\n");
        return 1;
    }
    struct stat dir_stat;
    if (stat(dir, &dir_stat) == -1) {
        perror("log");
        return errno;
    }
    if (S_ISDIR(dir_stat.st_mode)) {
        tree(dir, 0);
        return 0;

    } else {
        fprintf(stderr, "log: %s is not a directory\n", dir);
        return 1;
    }
}

void find_note(const char *basepath, const char *filename, char **filepaths, int *filecount) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(basepath))) {
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", basepath, entry->d_name);
            find_note(path, filename, filepaths, filecount);
        } else {
            char *ext = strrchr(entry->d_name, '.');
            if (ext == NULL) {
                continue;
            }
            // remove extension
            char *bname = strdup(entry->d_name);
            bname[strlen(bname) - strlen(DEFAULT_EXT)] = '\0';
            if (strcmp(bname, filename) == 0) {
                int count = *filecount;
                size_t len = strlen(basepath) + strlen(bname) + 2 + strlen(DEFAULT_EXT);
                filepaths[count] = malloc(sizeof(char) * len);
                snprintf(filepaths[count], len, "%s/%s%s", basepath, bname, DEFAULT_EXT);
                *filecount += 1;
            }
        }
    }
    closedir(dir);
}

// check if note exists, if yes return the path
char *check_note_exist(char *filename) {
    char *log_dir = get_log_dir();
    char **file_paths = malloc(sizeof(char *) * 128); // max 128 same file names
    int file_count = 0;
    find_note(log_dir, (const char *) filename, file_paths, &file_count);
    if (file_count == 0) {
        free(log_dir);
        return NULL;
    } else if (file_count == 1) {
        char *filepath = malloc(sizeof(char) * (strlen(file_paths[0]) + 1));
        if (filepath == NULL) {
            fprintf(stderr, "log: Error allocating memory\n");
            free(log_dir);
            exit(EXIT_FAILURE);
        }
        strcpy(filepath, file_paths[0]);
        free(log_dir);
        return filepath;
    } else {
        printf("Multiple files found with the same name:\n");
        for (int i = 0; i < file_count; i++) {
            printf("%d. %s\n", i + 1, file_paths[i]);
        }
        printf("Enter the number corresponding to the file you want to use: ");
        int choice;
        scanf("%d", &choice);
        if (choice < 1 || choice > file_count) {
            fprintf(stderr, "log: Invalid choice\n");
            free(log_dir);
            return NULL;
        }
        char *filepath = malloc(sizeof(char) * (strlen(file_paths[choice - 1]) + 1));
        if (filepath == NULL) {
            fprintf(stderr, "log: Error allocating memory\n");
            free(log_dir);
            exit(EXIT_FAILURE);
        }
        strcpy(filepath, file_paths[choice - 1]);
        free(log_dir);
        return filepath;
    }
}

void add_boiler_plate(FILE *file, const char *filename, const char *ext) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    if (strcmp(ext, ".md") == 0) {
        fprintf(file, "---\n");
        fprintf(file, "title: %s\n", filename);
        fprintf(file, "description: \n");
        fprintf(file, "tags: \n");
        fprintf(file, "created: \"%d-%02d-%02d\"\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
        fprintf(file, "lastmod: \"%d-%02d-%02d\"\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
        fprintf(file, "---\n");
    } else if (strcmp(ext, ".txt") == 0) {
        fprintf(file, "Title: %s\n", filename);
        fprintf(file, "Description: \n");
        fprintf(file, "Tags: \n");
        fprintf(file, "Created: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        fprintf(file, "Last Modified: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    } else if (strcmp(ext, ".org") == 0) {
        fprintf(file, "#+TITLE: %s\n", filename);
        fprintf(file, "#+DESCRIPTION: \n");
        fprintf(file, "#+TAGS: \n");
        fprintf(file, "#+CREATED: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        fprintf(file, "#+LASTMOD: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    } else if (strcmp(ext, ".html") == 0) {
        fprintf(file, "<!DOCTYPE html>\n");
        fprintf(file, "<html>\n");
        fprintf(file, "\t<head>\n");
        fprintf(file, "\t\t<title>%s</title>\n", filename);
        fprintf(file, "\t\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
        fprintf(file, "\t\t<meta charset=\"UTF-8\">\n");
        fprintf(file, "\t</head>\n");
        fprintf(file, "\t<body>\n");
        fprintf(file, "\t\t<h1>%s</h1>\n", filename);
        fprintf(file, "\t\t<p>Description: </p>\n");
        fprintf(file, "\t\t<p>Tags: </p>\n");
        fprintf(file, "\t\t<p>Created: %d-%02d-%02d %02d:%02d:%02d</p>\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        fprintf(file, "\t\t<p>Last Modified: %d-%02d-%02d %02d:%02d:%02d</p>\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        fprintf(file, "\t</body>\n");
        fprintf(file, "</html>\n");
    } else {
        fprintf(stderr, "log: Unsupported file extension\n");
    }
}
void print_note(char *filename) {
    if (filename == NULL) {
        fprintf(stderr, "log: filename is required\n");
        return;
    }
    char *filepath = check_note_exist(filename);
    if (filepath != NULL) {
        FILE *file = fopen(filepath, "r");
        if (file == NULL) {
            perror("log");
            free(filepath);
            return;
        }
        char *line = NULL;
        size_t len = 0;
        while (getline(&line, &len, file) != -1) {
            printf("%s", line);
        }
        free(line);
        fclose(file);
    } else {
        fprintf(stderr, "log: %s is not in log\n", filename);
    }
}

void edit_note(char *filename) {
    char *filepath = check_note_exist(filename);
    char *editor = getenv("EDITOR");
    if (editor == NULL) {
        fprintf(stderr, "log: EDITOR environment variable must be set\n");
        free(filepath);
        return;
    }
    if (filepath != NULL) {
        char *cmd = malloc(sizeof(char) * (strlen(editor) + strlen(filepath) + 2));
        if (cmd == NULL) {
            fprintf(stderr, "log: Error allocating memory\n");
            free(filepath);
            return;
        }
        sprintf(cmd, "%s %s", editor, filepath);
        system(cmd);
        free(cmd);
    } else {
        char *log_dir = get_log_dir();
        filepath = malloc(sizeof(char) * (strlen(log_dir) + strlen(filename) + 2 + strlen(DEFAULT_EXT)));
        if (filepath == NULL) {
            fprintf(stderr, "log: Error allocating memory\n");
            free(log_dir);
            return;
        }
        sprintf(filepath, "%s/%s%s", log_dir, filename, DEFAULT_EXT);
        FILE *file = fopen(filepath, "w");
        if (file == NULL) {
            perror("log");
            free(filepath);
            return;
        }
        add_boiler_plate(file, filename, DEFAULT_EXT);
        fclose(file);
        char *cmd = malloc(sizeof(char) * (strlen(editor) + strlen(filepath) + 2));
        if (cmd == NULL) {
            fprintf(stderr, "log: Error allocating memory\n");
            free(filepath);
            return;
        }
        sprintf(cmd, "%s %s", editor, filepath);
        system(cmd);
        free(cmd);
    }
}

void remove_note(char *filename) {
    char *filepath = check_note_exist(filename);
    if (filepath != NULL) {
        if (remove(filepath) == -1) {
            perror("log");
        } else {
            printf("log: %s is now removed from log\n", filename);
        }
    } else {
        fprintf(stderr, "log: %s is not in log\n", filename);
    }
}

void search_note(char *filename) {
    char *filepath = check_note_exist(filename);
    if (filepath != NULL) {
        printf("Path of %s: %s\n", filename, filepath);
    } else {
        fprintf(stderr, "log: %s is not in log\n", filename);
    }
}

void usage(char **argv) {
    fprintf(stderr, "Usage: %s [-l] [-i] [-v] [-h]\n", argv[0]);
    fprintf(stderr, "       %s [-I|-Q|-R|-S] [filename]\n", argv[0]);
}

int main(int argc, char **argv) {
    int res;
    char *log_dir = get_log_dir();
    if (log_dir == NULL) {
        return 1;
    }

    if (argc == 2) {
        if (strcmp(argv[1], "-l") == 0) {
            // read dir and print recursively
            res = list_dir(log_dir);
            free(log_dir);
            return res;
        } else if (strcmp(argv[1], "-i") == 0) {
            printf("log directory: %s\n", log_dir);
        } else if (strcmp(argv[1], "-v") == 0) {
            printf("log: %s\n", VERSION);
        } else if (strcmp(argv[1], "-h") == 0) {
            usage(argv);
            printf("log is a minimalistic command line note manager written in C.\n");
            printf("Options:\n");
            printf("  -l\t\tLists the notes in the directory in a tree format.\n");
            printf("  -i\t\tPrints the directory where the notes are stored.\n");
            printf("  -v\t\tPrints the version of the program.\n");
            printf("  -h\t\tShow the help message.\n");
            printf("  -I [filename]\tCreates a new note with the name specified in the filename.\n");
            printf("  -Q [filename]\tSearches for the note with specified name and prints its path to the stdout.\n");
            printf("  -R [filename]\tRemoves the note specified in the filename.\n");
            printf("  -S [filename]\tPrints the content of the note specified in the filename.\n");
        } else {
            usage(argv);
            return 0;
        }
    } else if (argc == 3) {
        if (strcmp(argv[1], "-I") == 0) {
            if (argv[2] != NULL && argv[2][0] != '-') {
                edit_note(argv[2]);
            } else {
                printf("log: filename cannot be start with '-'\n");
                usage(argv);
            }
        } else if (strcmp(argv[1], "-Q") == 0) {
            search_note(argv[2]);
        } else if (strcmp(argv[1], "-R") == 0) {
            remove_note(argv[2]);
        } else if (strcmp(argv[1], "-S") == 0) {
            print_note(argv[2]);
        } else {
            usage(argv);
            return 0;
        }
    } else {
        usage(argv);
        return 0;
    }
}
