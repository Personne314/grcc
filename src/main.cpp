#include "grammar.h"
#include "regex.h"

#include <iostream>
#include <filesystem>

#include "constants.h"


using std::filesystem::current_path, std::filesystem::absolute, std::filesystem::path;


int main(int argc, char *argv[]) {
	(void)argc;
	current_path(absolute(path(argv[0])).remove_filename());

	regexParser("^^[^^\\na-b]$$");

}
