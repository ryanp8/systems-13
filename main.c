#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

struct pop_entry {
    int year;
    int population;
    char boro[15];
};

void print_error();
void print_entry(int idx, struct pop_entry *entry);
void print_entries(struct pop_entry *entries);

int data_size();
int num_rows();

char **split_rows(char *data, int file_size);
void write_entry(struct pop_entry *entry);
int write_row(int row[6]);
int write_data(char **rows);
void read_csv();

struct pop_entry *read_data();
void add_data();
void update_data();



int main(int argc, char *argv[]) {

    if (argc > 1) {
        if (strcmp(argv[1], "-read_csv") == 0) {
            read_csv();
        }
        if (strcmp(argv[1], "-read_data") == 0) {
            struct pop_entry *entries = read_data();
            print_entries(entries);
        }
        if (strcmp(argv[1], "-add_data") == 0) {
            add_data();
        }
        if (strcmp(argv[1], "-update_data") == 0) {
            update_data();
        }
    }
    

    return 0;
}


void print_error() {
    printf("Error %d: %s\n", errno, strerror(errno));
}


int data_size() {
    struct stat s;
    stat("nyc.data", &s);
    return s.st_size;
}


int num_rows() {
    int fd = open("nyc_pop.csv", O_RDONLY);
    struct stat s;
    stat("nyc_pop.csv", &s);
    char *buffer = malloc(s.st_size);
    int res = read(fd, buffer, s.st_size);
    if (res == -1) {
        print_error();
        return -1;
    }

    int total = 0;
    int i;
    for (i = 0; i < s.st_size; i++) {
        if (buffer[i] == '\n') {
            total++;
        }
    }
    return total + 1;
}


char **split_rows(char *data, int file_size) {
    char **arr = malloc(8 * num_rows());
    arr[0] = malloc(100);
    int current_row = 0;
    int row_pos = 0;
    int i;
    for (i = 0; i < file_size; i++) {
        if (data[i] == '\n') {
            current_row++;
            arr[current_row] = malloc(100);
            row_pos = 0;
        } 
        else {
            arr[current_row][row_pos] = data[i];
            row_pos++;
        }
    }

    return arr;
}


void write_entry(struct pop_entry *entry) {
    int fd = open("nyc.data", O_WRONLY | O_APPEND);
    int res = write(fd, entry, sizeof(struct pop_entry));
    if (res == -1) {
        print_error();
        return;
    }
    close(fd);
}

int write_row(int row[6]) {
    char boros[5][15];
    strcpy(boros[0], "Manhattan");
    strcpy(boros[1], "Brooklyn");
    strcpy(boros[2], "Queens");
    strcpy(boros[3], "Bronx");
    strcpy(boros[4], "Staten Island");
    int bytes_written = 0;

    struct pop_entry entry;
    int j;
    for (j = 1; j < 6; j++) {
        entry.year = row[0];
        entry.population = row[j];
        strcpy(entry.boro, boros[j - 1]);
        write_entry(&entry);
        bytes_written += sizeof(struct pop_entry);
    }

    return bytes_written;
}


int write_data(char **rows) {
    int fd = open("nyc.data", O_EXCL | O_WRONLY | O_TRUNC, 0644);
    struct stat s;
    stat("nyc.data", &s);
    if (fd == -1) {
        print_error();
        return -1;
    }
    int bytes_written = 0;
    int nm = num_rows();
    int i;
    for (i = 1; i < nm;i++) {
        int row[6];
        sscanf(rows[i], "%d,%d,%d,%d,%d,%d", row, row + 1, row + 2, row + 3, row + 4, row + 5);
        bytes_written += write_row(row);
    }
    close(fd);

    return bytes_written;
}


void read_csv() {
    printf("reading nyc_pop.csv\n");
    int fd = open("nyc_pop.csv", O_RDONLY);
    struct stat s;
    stat("nyc_pop.csv", &s);

    char *buffer = malloc(s.st_size);
    int res = read(fd, buffer, s.st_size);
    if (res == -1) {
        print_error();
        return;
    } 

    char **rows = split_rows(buffer, s.st_size);
    int bytes_written = write_data(rows);
    printf("wrote %d bytes to nyc.data\n", bytes_written);
    close(fd);
}


void print_entry(int idx, struct pop_entry *entry) {
    printf("%d: { boro: %s, year: %d, population: %d }\n", idx, entry->boro, entry->year, entry->population);
}


void print_entries(struct pop_entry *entries) {
    int fd = open("nyc.data", O_RDONLY);
    int ds = data_size();
    int id = 0;
    while (id < ds / sizeof(struct pop_entry)) {
        printf("%d: { boro: %s, year: %d, population: %d }\n", id, entries[id].boro, entries[id].year, entries[id].population);
        id++;    
    }
    close(fd);
}

struct pop_entry *read_data() {
    int fd = open("nyc.data", O_RDONLY);
    int ds = data_size();
    struct pop_entry *entries = malloc(ds);
    int res = read(fd, entries, ds);
    if (res == -1) {
        print_error();
        return NULL;
    }

    close(fd);
    return entries;
}


void add_data() {
    printf("Enter year boro pop: ");
    char buffer[100];
    fgets(buffer, sizeof(buffer), stdin);
    struct pop_entry new_entry;
    sscanf(buffer, "%d %s %d\n", &(new_entry.year), new_entry.boro, &(new_entry.population));
    printf("appended data to file: year: %d\tboro: %s\t pop: %d\n", new_entry.year, new_entry.boro, new_entry.population);
    write_entry(&new_entry);
}


void update_data() {

    struct pop_entry *current = read_data();
    print_entries(current);

    printf("entry to update: ");
    char entry_buffer[100];
    fgets(entry_buffer, sizeof(entry_buffer), stdin);
    int target_entry;
    sscanf(entry_buffer, "%d\n", &target_entry);

    printf("Enter year boro pop: ");
    char data_buffer[100];
    fgets(data_buffer, sizeof(data_buffer), stdin);
    struct pop_entry new_entry;
    sscanf(data_buffer, "%d %s %d", &(new_entry.year), new_entry.boro, &(new_entry.population));
    
    if (!data_buffer[0] || !entry_buffer[0]) {
        printf("Invalid entry\n");
        return;
    }
    
    strcpy(current[target_entry].boro, new_entry.boro);
    current[target_entry].population = new_entry.population;
    current[target_entry].year = new_entry.year;
    
    int ds = data_size();
    int fd = open("nyc.data", O_WRONLY | O_TRUNC);
    
    int res = write(fd, current, ds);
    if (res == -1) {
        print_error();
        return;
    }

    printf("File updated\n");
    close(fd);
}

