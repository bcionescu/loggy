#include <stdio.h>

int main() {

	// Take path to log file as argument, as well as something to look for.
	
    // Check that the file exists, that it is not empty.
	
	// Check that the regex compiles properly, and in a reasonable amount of time.

	// Check that the file is readable, and does not contain any null bytes, causing
	// portions of the file be silently skipped.

	// Logs are usually in UTF-8, but not always. This might need to be addressed?

	// Read the contents of the file.

	// Go through it line by line, and use regex to look for requested data.

	// Show the relevant lines, using ANSII to color the matches, if the flag was provided.

	// Display extra information, such as the number of occurences.

	// Take extra arguments:
	// -i for case insensitivity
	// -n to display the line number for each matching line
	// -a to enable ANSII, as some users may prefer to not have it by default

}
