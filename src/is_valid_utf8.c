// gcc -o is_valid_utf8 is_valid_utf8.c -Wall -g

#include <stdio.h>
#include <stdlib.h>

int is_valid_utf8(const unsigned char *data, size_t length) {
    size_t i = 0;
    while (i < length) {
        if (data[i] <= 0x7F) {
            // 1-byte character (ASCII)
            i++;
        } else if ((data[i] & 0xE0) == 0xC0) {
            // 2-byte character
            if (i + 1 >= length || (data[i + 1] & 0xC0) != 0x80) return 0;
            i += 2;
        } else if ((data[i] & 0xF0) == 0xE0) {
            // 3-byte character
            if (i + 2 >= length || (data[i + 1] & 0xC0) != 0x80 || (data[i + 2] & 0xC0) != 0x80) return 0;
            i += 3;
        } else if ((data[i] & 0xF8) == 0xF0) {
            // 4-byte character
            if (i + 3 >= length || (data[i + 1] & 0xC0) != 0x80 || (data[i + 2] & 0xC0) != 0x80 || (data[i + 3] & 0xC0) != 0x80) return 0;
            i += 4;
        } else {
            return 0; // Invalid first byte
        }
    }
    return 1; // Valid UTF-8
}

#ifndef FLASH
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *buffer = malloc(length);
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(file);
        return EXIT_FAILURE;
    }

    fread(buffer, 1, length, file);
    fclose(file);

    if (is_valid_utf8(buffer, length)) {
        printf("The file is valid UTF-8.\n");
    } else {
        printf("The file is not valid UTF-8.\n");
    }

    free(buffer);
    return EXIT_SUCCESS;
}
#endif
