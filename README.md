Loggy is a log parser written in C.

I'm actively building out and improving the project as I learn C, so please don't mind any glaring issues that might exist with the code :)

## To-Do:

- [x] Take a bunch of optional arguments.
- [x] Take a regex pattern to search for.
- [x] Take a path to a log file.
- [x] Check that the file exists.
- [x] Check that it is a file, and not a directory.
- [x] Check if the file is empty.
- [x] Check that the regex pattern is valid and compiles correctly.
- [x] If -a is provided as an argument, color the matches accordingly.
- [x] If -i is provided, make the regex pattern search case-insensitive.
- [x] If -n is provided, display the line number for each match.
- [x] If -m is provided, display the total number of matches found in the file.
- [x] If -d is provided, display information useful for debugging, such as which flags were provided, what the regex is, and what the file path is.
- [ ] Handle an edge case in which the first element of the regex is a hyphen, as that would not work with the current logic.
- [ ] Have the code handle multiple matches per line, not just the first.
- [ ] Modify the debugging function to reflect the fact that the regex pattern is mandatory.
- [ ] Check if the file contains any null bytes, as that would cause portions of the log file to be silently skipped.
- [ ] Come up with a bunch of test scenarios and document them.
- [ ] Write documentation for the program in this GitHub repo.

## Logic

Let's first talk about the logic behind the code, how the parser works, and then we can dive deep into the program and see how it all works.

`Loggy` takes a few arguments, some optional, and others not. At the very least, `loggy` expects a regular expression and the path to a file. Here is an example.

```bash
loggy ".et" logs/logs.log
```

The above will check the contents of logs.log and look for any matches for the regex provided. If found, the relevant lines will be output to the terminal.

However, the user is also able to provide various arguments, which will change the behavior of the program. Here is a list of each flag and what it does.

`-i` → Enables case insensitivity when searching for matches.
`-a` → Colors the matches, making it easier to distinguish them.
`-n` → Displays the line number for each line that contains a match.
`-m` → Displays the total number of matches found at the end of the output.
`-d` → Displays information that is useful for debugging, such as which flags were passed, the regex provided, as well as the path to the file.

```bash
loggy -a -i ".et" logs/logs.log
```

You are also able to pass multiple arguments at once, without having to type multiple hyphens.

```bash
loggy -danim ".et" logs/logs.log
```

The program also handles a bunch of potential errors gracefully, all of which will be detailed in the next section.

If you wish to skip the explanations, you can [jump straight to the testing portion of this README file](#testing), where we'll try out various scenarios to test the robustness of the program.

## Explanation

Let's dive into each portion of the code and see what it does.

### Handling Arguments

```c
int main(int argc, char *argv[]) {

    int flag_i = 0, flag_n = 0, flag_a = 0, flag_m = 0, flag_d = 0;
    const char *regex = NULL;
    const char *path = NULL;

    struct stat st;
    regex_t pattern;

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
	fprintf(stderr, "Usage: %s [-i] [-n] [-a] [-m] [-d] <regex> <path-to-log>\n", argv[0]);
	return 1;
    }
```

Ok, there's a bit to unpack here. First off, we initialise a bunch of variables, such as `flag_i`, which are set to `0` initially, but which will be changed to `1` if their flags are passed by the user.

```c
int flag_i = 0, flag_n = 0, flag_a = 0, flag_m = 0, flag_d = 0;
```

We also declare two pointers to constant character arrays, which will contain the `regex` pattern and `path` passed by the user.

```c
const char *regex = NULL;
const char *path = NULL;
```

Next up, we declare the `st` struct, which will contain a bunch of file metadata, and `pattern` which will contain the compiled regex object.

```c
struct stat st;
regex_t pattern;
```

We then go through a loop that will iterate through `argc`, which is the number of arguments provided. The reason why the iterator `i` is placed outside of the loop is that we need to access its value after the loop has finished. If we initiated it inside of it, it would only exist within the local scope of the `for` loop. Also, the reason why we start at `argv[1]` is that `argv[0]` is always the name of the program.

```c
int i = 1;
for (; i < argc; i++) {
```

When we look at `argv[1][0]`, we're looking at the first element of what could be the flags provided. If it's a `-`, it means that we might get some flags. If not, it means that the flags were not provided, and so we can break out of the `if` statement and continue iterating through `argv`.

```c
if (argv[i][0] == '-') {
```

`if (argv[i][1] == '\0')` also handles an edge case where a hyphen is provided, but then nothing follows it. The program will find a null byte, realise that this is the end of the string, and then break, moving on to the next element of `argv`.

```c
if (argv[i][1] == '\0') {
```

Ok, so if we do have a bunch of flags to iterate through, this code does just that.

```c
for (int j = 1; argv[i][j] != '\0'; j++) {
```

For each element that we do find, we run it through a switch statement. If the element provided is one of `[inamd]`, then its corresponding `flag` variable is set to `1`, indicating that it has been passed by the user. We then break out of the switch statement, and we move on to the next element.

Alternatively, if the flag is unknown, a message is printed to `stderr` with `fprintf`, which works just like `printf`, but it allows you to specify the output stream.

```c
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
```

Once we run out of arguments to parse, we break out of the loop, and we continue with the program. This is where we will access the value of the iterator in order to determine where we are.

So, for example, let's say that we have the following input from the user:

```bash
loggy -danim ".et" logs/logs.log
```

If we look at the `argv` array, it will contain the following values:

`argv[0]` = `"loggy"`
`argv[1]` = `"-danim"`
`argv[2]` = `".et"`
`argv[3]` = `"logs/logs.log`
`argv[4]` = `NULL`

So we have four elements, we start at `argv[1]`, and we iterate through the flags. Given that we have five elements (including the null byte terminator), the `for` loop will stop after the third element, which will be skipped, as it does not start with a `-`.

As I write this, I realise that this will be an issue, as starting the regex with a hyphen will break the logic. This is covered more in the testing portion of the README file. Regardless, for now, the value of `i` will be `2`.

The `if` statement below checks if `2` is less than `4 - 1`, which is true. This means that we must be looking at the regex, and thus, the variable `regex` inherits the value of `argv[2]`, after which `i` gets incremented by `1`.

```c
if (i < argc - 1) {
	regex = argv[i++];
    }
```

Next, we check to see if `i`, which is now `3`, is less than `4`, which it is, and that means that this is the path.

```c
   if (i < argc) {
	path = argv[i];
    }
```

Finally, if either a regex or a path was not provided, the program throws an error, explaining the usage of the program.

```c
if (!path || !regex) {
	fprintf(stderr, "Usage: %s [-i] [-n] [-a] [-m] [-d] <regex> <path-to-log>\n", argv[0]);
	return 1;
    }
```

### check_if_file_exists(path, &st);

Now that we've parsed all the arguments provided, it's time to start doing stuff with the data we now have. The very first step is to check if the file actually exists.

```c
void check_if_file_exists(const char *path, struct stat *st) {
    if (stat(path, st) != 0) {
	perror("stat");
	fprintf(stderr, "Error: cannot access file: %s\n", path);
	exit(EXIT_FAILURE);
    }
}
```

We pass the function a pointer to the path, as well as a pointer to the `st` struct.

In the `if` statement, we call `stat()` and we pass it the path to the file, as well as the `st` struct, which will be populated with the metadata of the file.

If `stat()` returns `0`, it means everything is fine, and because we passed a pointer to `st`, it is now globally populated with the data, not just in this function.

If we receive any value other than `0`, it means that something went wrong, and an error will be printed.

### is_file_valid(path, &st);

Now that we've established the file is there, we now need to make sure that the file is valid, and it is not a directory, for example.

We're passing pointers once more, for the sake of efficiency, after which `st_mode` checks both the type and permissions on the file.

```c
void is_file_valid(const char *path, struct stat *st) {
    if (!S_ISREG(st -> st_mode)) {
	fprintf(stderr, "Error: path is not a regular file: %s\n", path);
	exit(EXIT_FAILURE);
    }
}
```

### FILE *file = read_file(path);

This function takes the path, reads the contents of the file, and then returns it to `main()` so that the data may be stored in the variable `file`.

```c
FILE *read_file(const char *path) {
    return fopen(path, "r");
}
```

### is_file_empty(&st);

This function checks the `st_size` member of the `st` struct to determine the size of the file. If it is `0`, that means it is empty, and an error is thrown.

```c
void is_file_empty(struct stat *st) {
    if (st->st_size == 0) {
	printf("The file provided is empty.\n");
	exit(EXIT_FAILURE);
    }
}
```

### int regex_flags = determine_case(flag_i);

In order to determine if case sensitivity was requested, we pass `flag_i` over to the function. First, we attribute `REG_EXTENDED` to `regex_flags`, which will tell the regex compiler that we want extended regex functionality.

Then, if `flag_i` is not `0`, we add `REG_ICASE`, which will enable case insensitivity. The value of `regex_flags` is then returned to `main()`.

```c
int determine_case(int flag_i) {
    
    int regex_flags = REG_EXTENDED;

    if (flag_i) {
	regex_flags |= REG_ICASE;
    }
    
    return regex_flags;
}
```

### int matches_count = print_matches(file, &pattern, flag_a, flag_n);

```c
int print_matches(FILE *file, regex_t *pattern, int flag_a, int flag_n) {

    char line[1024];
    int matches_count = 0;
    int line_number = 0;
    regmatch_t match[1];

    while (fgets(line, sizeof(line), file) != NULL ) {
	line_number++;

	if (regexec(pattern, line, 1, match, 0) == 0) {

	    if (flag_a) {
		if (flag_n) printf("%d:", line_number);

                fwrite(line, 1, match[0].rm_so, stdout);

		printf("\x1b[31m");
		fwrite(line + match[0].rm_so, 1, match[0].rm_eo - match[0].rm_so, stdout);
		printf("\x1b[0m");

		printf("%s", line + match[0].rm_eo);
	    } else {
		if (flag_n) printf("%d:", line_number);
		printf("%s", line);
		}
	matches_count++;
	}
    }

    fclose(file);
    regfree(pattern);

    return matches_count;
}
```

Ok, now it's time to actually look for regex matches. First, we pass a pointer to the file, the compiled regex pattern, and the state of the `-a` and `-n` flags.

```c
int print_matches(FILE *file, regex_t *pattern, int flag_a, int flag_n) {
```

Next, we initialise a few variables. We do `line` first, and we assume it'll be a maximum of 1024 bytes. We also initialise the variable that will count the number of matches, the current line number, and an array `match` where we'll store the first match for every line.

I'll need to improve this in the near future, so that it stores more than the first match per line.

```c
char line[1024];
int matches_count = 0;
int line_number = 0;
regmatch_t match[1];
```

Next, we iterate through the lines until there are no more lines, and we check for a regex match in each.

```c
while (fgets(line, sizeof(line), file) != NULL ) {
	line_number++;

	if (regexec(pattern, line, 1, match, 0) == 0) {
```

Before we do anything else, we check to see what the value of `flag_a` is.

```c
if (flag_a) {
```

If it is not zero, that means it's been passed by the user, and so, we'll use ANSI to color the matches. Additionally, we check for the value of `flag_n`, which, if passed, will first print the line number before printing the string with the ANSI escape code.

```c
if (flag_a) {
		if (flag_n) printf("%d:", line_number);

                fwrite(line, 1, match[0].rm_so, stdout);

		printf("\x1b[31m");
		fwrite(line + match[0].rm_so, 1, match[0].rm_eo - match[0].rm_so, stdout);
		printf("\x1b[0m");

		printf("%s", line + match[0].rm_eo);
	    }
```

If the `-a` flag was not provided, but the `-n` flag was, we print the string without ANSI escape codes.

```c
} else {
		if (flag_n) printf("%d:", line_number);
		printf("%s", line);
		}
```

The loop also iterates two variables, `line_number++` and `matches_count++`. If the `-n` flag was provided, the local value of `line_number` is used. The value of `matches_count` is passed back to main, as it is used in a different function.

Once we're finished, we need to do two things. One is to close the file, and two is to free up any internal memory that might have been used by `pattern`, preventing potential memory leaks.

```c
fclose(file);
regfree(pattern);
```

### check_matches_flag(flag_m, matches_count, regex);

Once all this has been done, it's time to check if we need to print out the number of matches.

The two variables `flag_m` and `matches_count` are passed, as they contain small values. After them, the pointer to `regex` is passed, as the output contains the regex itself.

```c
void check_matches_flag(int flag_m, int matches_count, const char *regex) {

    if (flag_m) {
	printf("\nA total of %d matches were found for the \"%s\" pattern.\n", matches_count, regex);
    }

}
```

### check_debugging_flag(flag_i, flag_n, flag_a, flag_m, path, flag_d, regex);

```c
void check_debugging_flag(int flag_i, int flag_n, int flag_a, int flag_m, const char *path, int flag_d, const char *regex) {
    if (flag_d) {
	printf("\nDebugging info:\n");
	printf("Flags: -i=%d -n=%d -a=%d -m=%d -d=%d\n", flag_i, flag_n, flag_a, flag_m, flag_d);
	printf("Regex: %s\n", regex ? regex : "(none)");
	printf("Path: %s\n", path);
    }

}
```

All the flags are passed to it, as well as the pointer to `path`, and the pointer to `regex`. If this flag is passed, a bunch of information useful for debugging is passed.

The data includes which flags were passed, the regex used, and the path to the file. The code currently assumes that the regex path is optional, as this was the case early in the code, so this will have to be fixed in a future version.

## Testing

Testing will cover several options, ensuring that the program is as robust as possible. In order to ensure consistency, we will use the same regex in all tests but a few, specifically `ad.[in]+`. This will match the word `admin` in our log file.

For context, here is a sample of the log file.

```log
192.168.0.5 - - [13/Nov/2025:12:01:03 +0000] "GET /index.html HTTP/1.1" 200 5123
10.0.0.14 - - [13/Nov/2025:12:01:04 +0000] "POST /login HTTP/1.1" 302 122
172.16.5.44 - - [13/Nov/2025:12:01:04 +0000] "GET /dashboard HTTP/1.1" 200 9812
192.168.0.5 - - [13/Nov/2025:12:01:07 +0000] "GET /images/logo.png HTTP/1.1" 200 23211
203.0.113.77 - - [13/Nov/2025:12:01:09 +0000] "GET /robots.txt HTTP/1.1" 404 321
10.0.0.14 - - [13/Nov/2025:12:01:10 +0000] "GET /profile HTTP/1.1" 200 1821
192.168.0.8 - - [13/Nov/2025:12:01:13 +0000] "GET /settings HTTP/1.1" 403 712
203.0.113.77 - - [13/Nov/2025:12:01:14 +0000] "GET /favicon.ico HTTP/1.1" 200 1321
192.0.2.101 - - [13/Nov/2025:12:01:15 +0000] "POST /api/v1/upload HTTP/1.1" 201 55321
172.16.5.44 - - [13/Nov/2025:12:01:16 +0000] "GET /logout HTTP/1.1" 200 231
192.168.0.5 - - [13/Nov/2025:12:01:18 +0000] "GET /dashboard HTTP/1.1" 200 9780
203.0.113.77 - - [13/Nov/2025:12:01:22 +0000] "GET /secret/admin HTTP/1.1" 401 94
10.0.0.14 - - [13/Nov/2025:12:01:25 +0000] "GET /reports HTTP/1.1" 200 7120
```

### No arguments provided

```input
./loggy
```

```output
Usage: ./loggy [-i] [-n] [-a] [-m] [-d] <regex> <path-to-log>
```

### No RegEx provided

```input
./loggy -dainm logs/logs.log
```

```output
Usage: ./loggy [-i] [-n] [-a] [-m] [-d] <regex> <path-to-log>
```

### No path provided

```input
./loggy -dainm "ad.[in]+"
```

```output
Usage: ./loggy [-i] [-n] [-a] [-m] [-d] <regex> <path-to-log>
```

### No flags provided

```input
./loggy "ad.[in]+" logs/logs.log
```

```output
203.0.113.77 - - [13/Nov/2025:12:01:22 +0000] "GET /secret/admin HTTP/1.1" 401 94
203.0.113.77 - - [13/Nov/2025:12:02:16 +0000] "GET /secret/admin HTTP/1.1" 401 93
```

### A non-existing path provided

```input
./loggy "ad.[in]+" logs/nologs.log
```

```output
stat: No such file or directory
Error: cannot access file: logs/nologs.log
```

### A directory passed as a path

```input
./loggy "ad.[in]+" logs
```

```output
Error: path is not a regular file: logs
```

### An empty file passed as a path

```input
./loggy "ad.[in]+" logs/empty.log
```

```output
The file provided is empty.
```

### The `-a` flag being passed

```input
./loggy -a "ad.[in]+" logs/logs.log
```

```output
203.0.113.77 - - [13/Nov/2025:12:01:22 +0000] "GET /secret/admin HTTP/1.1" 401 94
203.0.113.77 - - [13/Nov/2025:12:02:16 +0000] "GET /secret/admin HTTP/1.1" 401 93
```

Note that in a terminal, the word `admin` would be red in color.
### The `-i` flag being passed

```input
./loggy -i "Ad.[in]+" logs/logs.log
```

```output
203.0.113.77 - - [13/Nov/2025:12:01:22 +0000] "GET /secret/admin HTTP/1.1" 401 94
203.0.113.77 - - [13/Nov/2025:12:02:16 +0000] "GET /secret/admin HTTP/1.1" 401 93
```

Even though the regex pattern is capitalised, the lowercase `admin` was still matched.

### The `-n` flag being passed

```input
./loggy -n "ad.[in]+" logs/logs.log
```

```output
12:203.0.113.77 - - [13/Nov/2025:12:01:22 +0000] "GET /secret/admin HTTP/1.1" 401 94
30:203.0.113.77 - - [13/Nov/2025:12:02:16 +0000] "GET /secret/admin HTTP/1.1" 401 93
```

The two matches for `admin` can be found on lines 12 and 30.

### The `-m` flag being passed

```input
./loggy -m "ad.[in]+" logs/logs.log
```

```output
203.0.113.77 - - [13/Nov/2025:12:01:22 +0000] "GET /secret/admin HTTP/1.1" 401 94
203.0.113.77 - - [13/Nov/2025:12:02:16 +0000] "GET /secret/admin HTTP/1.1" 401 93

A total of 2 matches were found for the "ad.[in]+" pattern.
```

### The `-d` flag being passed

```input
./loggy -d "ad.[in]+" logs/logs.log
```

```output
203.0.113.77 - - [13/Nov/2025:12:01:22 +0000] "GET /secret/admin HTTP/1.1" 401 94
203.0.113.77 - - [13/Nov/2025:12:02:16 +0000] "GET /secret/admin HTTP/1.1" 401 93

Debugging info:
Flags: -i=0 -n=0 -a=0 -m=0 -d=1
Regex: ad.[in]+
Path: logs/logs.log
```

### Multiple flags passed independently

```input
./loggy -d -a -n -i -m "Ad.[in]+" logs/logs.log
```

```output
12:203.0.113.77 - - [13/Nov/2025:12:01:22 +0000] "GET /secret/admin HTTP/1.1" 401 94
30:203.0.113.77 - - [13/Nov/2025:12:02:16 +0000] "GET /secret/admin HTTP/1.1" 401 93

A total of 2 matches were found for the "Ad.[in]+" pattern.

Debugging info:
Flags: -i=1 -n=1 -a=1 -m=1 -d=1
Regex: Ad.[in]+
Path: logs/logs.log
```

### Multiple flags passed in a group

```input
./loggy -danim "Ad.[in]+" logs/logs.log
```

```output
12:203.0.113.77 - - [13/Nov/2025:12:01:22 +0000] "GET /secret/admin HTTP/1.1" 401 94
30:203.0.113.77 - - [13/Nov/2025:12:02:16 +0000] "GET /secret/admin HTTP/1.1" 401 93

A total of 2 matches were found for the "Ad.[in]+" pattern.

Debugging info:
Flags: -i=1 -n=1 -a=1 -m=1 -d=1
Regex: Ad.[in]+
Path: logs/logs.log
```

### An invalid regex passed

```input
./loggy -danim "Ad.[in]??+" logs/logs.log
```

```output
Could not compile regex: repetition-operator operand invalid
```

### A few different regex patterns being passed

```input
./loggy -dainm "delete" logs/logs.log
```

```output
14:192.0.2.101 - - [13/Nov/2025:12:01:30 +0000] "DELETE /api/v1/upload HTTP/1.1" 204 0

A total of 1 matches were found for the "delete" pattern.

Debugging info:
Flags: -i=1 -n=1 -a=1 -m=1 -d=1
Regex: delete
Path: logs/logs.log
```

```input
./loggy -dainm "Po[st]+" logs/logs.log
```

```output
2:10.0.0.14 - - [13/Nov/2025:12:01:04 +0000] "POST /login HTTP/1.1" 302 122
9:192.0.2.101 - - [13/Nov/2025:12:01:15 +0000] "POST /api/v1/upload HTTP/1.1" 201 55321
23:192.0.2.101 - - [13/Nov/2025:12:01:54 +0000] "POST /api/v1/upload HTTP/1.1" 500 112
28:192.0.2.101 - - [13/Nov/2025:12:02:09 +0000] "POST /api/v1/upload HTTP/1.1" 201 55320
35:192.0.2.101 - - [13/Nov/2025:12:02:27 +0000] "POST /api/v1/upload HTTP/1.1" 500 110

A total of 5 matches were found for the "Po[st]+" pattern.

Debugging info:
Flags: -i=1 -n=1 -a=1 -m=1 -d=1
Regex: Po[st]+
Path: logs/logs.log
```

```input
./loggy -dainm "f[av]*icon" logs/logs.log
```

```output
8:203.0.113.77 - - [13/Nov/2025:12:01:14 +0000] "GET /favicon.ico HTTP/1.1" 200 1321
16:203.0.113.77 - - [13/Nov/2025:12:01:38 +0000] "GET /favicon.ico HTTP/1.1" 304 0
25:203.0.113.77 - - [13/Nov/2025:12:02:00 +0000] "GET /favicon.ico HTTP/1.1" 200 1330
38:203.0.113.77 - - [13/Nov/2025:12:02:34 +0000] "GET /favicon.ico HTTP/1.1" 200 1329

A total of 4 matches were found for the "f[av]*icon" pattern.

Debugging info:
Flags: -i=1 -n=1 -a=1 -m=1 -d=1
Regex: f[av]*icon
Path: logs/logs.log
```

### A regex starting with a `-` being passed

```input
❯ ./loggy -dainm "-icon" logs/logs.log
```

```output
Unknown option: c
```

As mentioned earlier in the documentation, this edge case needs to be resolved. Due to the logic of the program, it interprets the `-` in the regex pattern as a `-` before the flags. In this case, it recognised the `i` as a flag, but not `c`, and crashed.

```input
./loggy -dainm "-dainm" logs/logs.log
```

```output
Usage: ./loggy [-i] [-n] [-a] [-m] [-d] <regex> <path-to-log>
```

If I give it a regex pattern that happens to contain characters that the flags might contain, it throws an error, as it likely assumes that the path is the regex, and then thinks that a path was not given.
