#pragma once

#include <iostream>
#include <string>

using std::string;

namespace grcc {

	// Printing prefixes.
	constexpr char *INFO_STR = const_cast<char*>("\e[1mgrcc:\e[0;0m ");
	constexpr char *ERR_STR = const_cast<char*>("\e[1mgrcc:\e[0;0m \e[1;31merror:\e[0;0m "); 

	// Wrapper around cerr to print the grcc error prefix.
	// Use the () operator to get the normal cout if you dont want to print the prefix.
	class grcc_cerr {
	public:
		template <typename T>
		std::ostream& operator<<(const T& value) const {
			return std::cerr << ERR_STR << value;
		}
		std::ostream& operator()() const {return std::cerr;}
	};
	constexpr grcc_cerr cerr;

	// Wrapper around cout to print the grcc prefix.
	// Use the () operator to get the normal cout if you dont want to print the prefix.
	class grcc_cout {
	public:
		template <typename T>
		std::ostream& operator<<(const T& value) const {
			return std::cout << INFO_STR << value;
		}
		std::ostream& operator()() const {return std::cout;}

	};
	constexpr grcc_cout cout;

	// Namespace for text formating using ansi escape codes.
	namespace ansi {
		constexpr char *RED = const_cast<char*>("\e[0;31m");
		constexpr char *BOLD = const_cast<char*>("\e[1m");
		constexpr char *RST = const_cast<char*>("\e[0;0m");
	};

};
