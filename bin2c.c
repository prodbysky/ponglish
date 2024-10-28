#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    FILE* file = fopen(argv[1], "rb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open %s", argv[1]);
        return 1;
    }
    fseek(file, 0, SEEK_END);
    const size_t len = ftell(file);
    fseek(file, 0, SEEK_SET);

    // "Remove the extension"
    *strrchr(argv[1], '.') = '\0';
    char buf[1024]         = {0};
    snprintf(buf, 20, "%s.h", argv[1]);
    FILE* output = fopen(buf, "wb");
    fprintf(output, "#pragma once\n");
    fprintf(output, "static const unsigned char %s[] = {\n", argv[1]);

    int i = 0;
    while (i <= len) {
        uint8_t byte;
        fread(&byte, 1, 1, file);
        fprintf(output, "%d, ", byte);
        i++;
    }
    fprintf(output, "\n};");

    fclose(file);
    fclose(output);
}
