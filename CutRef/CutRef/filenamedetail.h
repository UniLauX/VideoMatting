#pragma  once
#include <string>

namespace FileName
{
	std::string find_filename_path(const std::string &filename);

	std::string find_filename_name(const std::string &filename);

	std::string find_filename_name_without_ext(const std::string &filename);

	std::string find_filename_ext(const std::string &filename);

	std::string find_filename_path_name_without_ext(const std::string &filename);

	int find_filename_number(const std::string &filename);

	std::string increase_filename_number(const std::string &filename, int incr);
}