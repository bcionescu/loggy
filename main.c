#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

void check_if_file_exists(const char *path, struct stat *st);
void is_file_valid(const char *path, struct stat *st);
FILE *read_file(const char *path);
void is_file_empty(struct stat st);
void compile_regex(regex_t *pattern, const char *regex, int regex_flags);
int determine_case(int flag_i);
void check_matches_flag(int flag_m, int matches_count, const char *regex);
void check_debugging_flag(int flag_i, int flag_n, int flag_a, int flag_m, const char *path, int flag_d, const char *regex);

int main(int argc, char *argv[]) {

    int flag_i = 0, flag_n = 0, flag_a = 0, flag_m = 0, flag_d = 0;
    const char *regex = NULL;
    const char *path = NULL;

    int i = 1;
    for (; i < argc; i++) {
	if (argv[i][0] == '-') {
	    if (argv[i][1] == '\0') {
		break;
	    }

	    for (int j = 1; argv[i][j] != '\0'; j++) {
		switch (argv[i][j]) {
		    case 'i': flag_i = 1; break;
		    case 'n': flag_n = 1; break;
		    case 'a': flag_a = 1; break;
		    case 'm': flag_m = 1; break;
		    case 'd': flag_d = 1; break;
		    default:
			fprintf(stderr, "Unknown option: %c\n", argv[i][j]);
			return 1;
		}
	    }
	} else {
	    break;
	}
    }

    if (i < argc - 1) {
	regex = argv[i++];
    }

    if (i < argc) {
	path = argv[i];
    }

    if (!path || !regex) {
	fprintf(stderr, "Usage: %s [-i] [-n] [-a] [-m] [-d] [regex] <path-to-log>\n", argv[0]);
	return 1;
    }

    struct stat st;

    check_if_file_exists(path, &st);

    is_file_valid(path, &st);

    FILE *file = read_file(path);

    is_file_empty(st);

    regex_t pattern;
    regmatch_t match;

    int regex_flags = determine_case(flag_i);

    compile_regex(&pattern, regex, regex_flags);

    char line[1024];
    int matches_count = 0;
    int line_number = 0;

    while (fgets(line, sizeof(line), file) != NULL ) {
	line_number++;

	if (regexec(&pattern, line, 1, &match, 0) == 0) {
	    if (flag_a) {
		if (flag_n) printf("%d:", line_number);

		fwrite(line, 1, match.rm_so, stdout);

		printf("\x1b[31m");
		fwrite(line + match.rm_so, 1, match.rm_eo - match.rm_so, stdout);
		printf("\x1b[0m");

		printf("%s", line + match.rm_eo);
	    } else {
		if (flag_n) printf("%d:", line_number);
		printf("%s", line);
		}
	matches_count++;
	}
    }

    fclose(file);
    regfree(&pattern);

    check_matches_flag(flag_m, matches_count, regex);

    check_debugging_flag(flag_i, flag_n, flag_a, flag_m, path, flag_d, regex);

    return 0;
}

void check_if_file_exists(const char *path, struct stat *st) {
    if (stat(path, st) != 0) {
	perror("stat");
	fprintf(stderr, "Error: cannot access file: %s\n", path);
	exit(EXIT_FAILURE);
    }
}

void is_file_valid(const char *path, struct stat *st) {
    if (!S_ISREG(st -> st_mode)) {
	fprintf(stderr, "Error: path is not a regular file: %s\n", path);
	exit(EXIT_FAILURE);
    }
}

FILE *read_file(const char *path) {
    return fopen(path, "r");
}

void is_file_empty(struct stat st) {
    if (st.st_size == 0) {
	printf("The file provided is empty.\n");
	exit(EXIT_FAILURE);
    }
}

void compile_regex(regex_t *pattern, const char *regex, int regex_flags) {
    int ret = regcomp(pattern, regex, regex_flags);
    if (ret) {
    char errbuf[128];
	regerror(ret, pattern, errbuf, sizeof(errbuf));
	fprintf(stderr, "Could not compile regex: %s\n", errbuf);
	exit(EXIT_FAILURE);
    }
}

int determine_case(int flag_i) {
    
    int regex_flags = REG_EXTENDED;

    if (flag_i) {
	regex_flags |= REG_ICASE;
    }
    
    return regex_flags;
}

void check_matches_flag(int flag_m, int matches_count, const char *regex) {

    if (flag_m == 1) {
	printf("\nA total of %d matches were found for the \"%s\" pattern.\n", matches_count, regex);
    }

}

void check_debugging_flag(int flag_i, int flag_n, int flag_a, int flag_m, const char *path, int flag_d, const char *regex) {
    if (flag_d == 1) {
	printf("\nDebugging info:\n");
	printf("Flags: -i=%d -n=%d -a=%d -m=%d -d=%d\n", flag_i, flag_n, flag_a, flag_m, flag_d);
	printf("Regex: %s\n", regex ? regex : "(none)");
	printf("Path: %s\n", path);
    }

}

