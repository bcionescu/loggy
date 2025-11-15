A log parser written in C.

I'm actively building out and improving the project as I learn C, so please don't mind any glaring issues that might exist with the code :)

To-Do:

- [x] Take a bunch of optional arguments.
- [x] Take a regex pattern to search for.
- [x] Take a path to a log file.
- [x] Check that the file exists.
- [x] Check that it is a file, and not a directory.
- [x] Check if the file is empty.
- [x] Check that the regex pattern is valid, and compiles correctly.
- [x] If -a is provided as an argument, color the matches accordingly.
- [x] If -i is provided, make the regex pattern search case insensitive.
- [x] If -n is provided, display the line number for each match.
- [x] If -m is provided, display the total number of matches found in the file.
- [x] If -d is provided, display information useful for debugging, such as which flags were provided, what the regex is, and what the file path is.
- [ ] Check if the file contains any null bytes, as that would cause portions of the log file to be silently skipped.
- [ ] Logs are usually in UTF-8. Think about whether this will cause any problems.
- [ ] Come up with a bunch of test scenarios, and document them.
- [ ] Write documentation for the program in this GitHub repo.
