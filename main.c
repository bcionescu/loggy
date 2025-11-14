#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <regex.h>

int main(int argc, char *argv[]) {

    // Take path to log file as argument, as well as something to look for.
    int flag_i = 0, flag_n = 0, flag_a = 0;
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

    if (!path) {
	fprintf(stderr, "Usage: %s [-i] [-n] [-a] [regex] <path-to-log>\n", argv[0]);
	return 1;
    }

    // Check that the file exists.
    struct stat st;
    
    if (stat(path, &st) != 0) {
	perror("stat");
	fprintf(stderr, "Error: cannot access file: %s\n", path);
	return 1;
    }

    if (!S_ISREG(st.st_mode)) {
	fprintf(stderr, "Error: path is not a regular file: %s\n", path);
	return 1;
    }

    // Open the file
    FILE *file;
    file = fopen(path, "r");

    char *empty = "";

    // Check if the file is empty
    if (st.st_size == 0) {
	empty = "empty";
    } else {
	empty = "not empty";
    }

    // Compile the regex
    regex_t pattern;
    regmatch_t match;

    int regex_flags = REG_EXTENDED;

    if (flag_i) {
	regex_flags |= REG_ICASE;
    }

    int ret = regcomp(&pattern, regex, regex_flags);
    if (ret) {
    char errbuf[128];
	regerror(ret, &pattern, errbuf, sizeof(errbuf));
	fprintf(stderr, "Could not compile regex: %s\n", errbuf);
	return 1;
    }

    // Read the file line by line
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

    regfree(&pattern);

    printf("Flags: -i=%d -n=%d -a=%d\n", flag_i, flag_n, flag_a);
    printf("Regex: %s\n", regex ? regex : "(none)");
    printf("Path: %s\n", path);
    printf("Matches: %d\n", matches_count);
    printf("The file has been read and is %s.", empty);

    // Check that the file is readable, and does not contain any null bytes, causing
    // portions of the file be silently skipped.

    // Logs are usually in UTF-8, but not always. This might need to be addressed?

    // Show the relevant lines, using ANSII to color the matches, if the flag was provided.

    // Display extra information, such as the number of occurences.

    return 0;

}
