#include <string>
#include "filenamedetail.h"
#include <math.h>

namespace FileName
{
	std::string find_filename_path(const std::string &filename)
	{
		std::string::size_type pos = filename.rfind('/');
		std::string::size_type pos1 = filename.rfind('\\');

		if (pos == std::string::npos && pos1 != std::string::npos)
		{
			return filename.substr(0, pos1+1);			
		}
		
		if (pos != std::string::npos && pos1 == std::string::npos)
		{
			return filename.substr(0, pos+1);
		}

		if (pos != std::string::npos && pos1 != std::string::npos)
		{
			if (pos1 > pos)
			{
				return filename.substr(0, pos1+1);
			}
			else
			{
				return filename.substr(0, pos+1);
			}
		}
		return std::string();
	}

	std::string find_filename_name(const std::string &filename)
	{
		std::string::size_type pos = filename.rfind('/');
		std::string::size_type pos1 = filename.rfind('\\');

		if (pos == std::string::npos && pos1 != std::string::npos)
		{
			return filename.substr(pos1+1, filename.size());			
		}

		if (pos != std::string::npos && pos1 == std::string::npos)
		{
			return filename.substr(pos+1, filename.size());
		}

		if (pos != std::string::npos && pos1 != std::string::npos)
		{
			if (pos1 > pos)
			{
				return filename.substr(pos1+1, filename.size());
			}
			else
			{
				return filename.substr(pos+1, filename.size());
			}
		}
		return filename;
	}

	std::string find_filename_name_without_ext(const std::string &filename)
	{
		std::string::size_type pos = filename.rfind('/');
		std::string::size_type pos1 = filename.rfind('\\');

		std::string::size_type pos2 = filename.rfind('.');
		if (pos2 == std::string::npos)
		{
			pos2 = filename.size();
		}

		if (pos == std::string::npos && pos1 != std::string::npos)
		{
			return filename.substr(pos1+1, pos2 - pos1 - 1);			
		}

		if (pos != std::string::npos && pos1 == std::string::npos)
		{
			return filename.substr(pos+1, pos2 - pos - 1);
		}

		if (pos != std::string::npos && pos1 != std::string::npos)
		{
			if (pos1 > pos)
			{
				return filename.substr(pos1+1, pos2 - pos1 - 1);
			}
			else
			{
				return filename.substr(pos+1, pos2 - pos - 1);
			}
		}
		return std::string();
	}

	std::string find_filename_ext(const std::string &filename)
	{
		std::string::size_type pos = filename.rfind('.');

		if (pos != std::string::npos)
		{
			return filename.substr(pos+1, filename.size());			
		}
		return std::string();
	}

	std::string find_filename_path_name_without_ext(const std::string &filename)
	{
		std::string::size_type pos = filename.rfind('.');

		if(pos != std::string::npos)
		{
			return filename.substr(0, pos);
		}
		return filename;
	}

	int find_filename_number(const std::string &filename)
	{
		std::string strnum;
		int i, j, length, number;

		length = (int) filename.length();
		i = length;
		while(filename[i--] != '.' && i >= 0);
		j = i + 1;
		while(isdigit(filename[i]) && i >= 0)
			i--;
		i++;//prefix characters number;
		if(i == j)
		{
			number = -1;
		}
		else
		{
			strnum = filename.substr(i, j - i);
			number = atoi(strnum.c_str());
		}

		return number;
	}

	std::string increase_filename_number(const std::string &filename, int incr)
	{
		std::string strnum, fileext, result;
		int i, j, length, number, formatwidth;
		char strbuffer[100];

		length = (int) filename.length();
		i = length;
		fileext = find_filename_ext(filename);

		while(filename[i--] != '.' && i >= 0);
		j = i + 1;
		while(isdigit(filename[i]) && i >= 0)
			i--;
		i++;//prefix characters number
		if(i != 0)
			result = filename.substr(0, i);
		formatwidth = j - i;
		if(i == j)
		{
			number = incr;
		}
		else
		{
			strnum = filename.substr(i, j - i);
			number = atoi(strnum.c_str());
			number += incr;
		}

		if(formatwidth < log10((double) number))
			formatwidth = (int) (log10((double) number));

		switch(formatwidth){
		case 2:
			sprintf(strbuffer, "%.2d", number);
			break;
		case 3:
			sprintf(strbuffer, "%.3d", number);
			break;
		case 4:
			sprintf(strbuffer, "%.4d", number);
			break;
		case 5:
			sprintf(strbuffer, "%.5d", number);
			break;
		case 6:
			sprintf(strbuffer, "%.6d", number);
			break;
		case 7:
			sprintf(strbuffer, "%.7d", number);
			break;
		case 8:
			sprintf(strbuffer, "%.8d", number);
			break;
		case 9:
			sprintf(strbuffer, "%.9d", number);
			break;
		case 10:
			sprintf(strbuffer, "%.10d", number);
			break;
		default:
			sprintf(strbuffer, "%d", number);
			break;
		}
		strnum = strbuffer;
		result += strnum;
		result += ".";
		result += fileext;
		return result;
	}
}