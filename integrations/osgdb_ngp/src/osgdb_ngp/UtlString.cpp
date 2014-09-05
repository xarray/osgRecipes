/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** UtlString.cpp
** String utilities
**
** Author: Brian Bailey www.code-hammer.com
** -------------------------------------------------------------------------*/

#include "UtlString.h"
#include <algorithm>

//============================================================================
//============================================================================
std::string UtlString::toLower(const std::string &s)
{
    std::string lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
	return lower;
}

//============================================================================
//============================================================================
int UtlString::explode(std::string str, const std::string &separator, std::vector<std::string>* results)
{
	size_t found, count=0;
	found = str.find_first_of(separator);
	while(found != std::string::npos)
	{
		if (found == 0 && count > 0)
		{
			results->push_back(std::string("")); // empty string
		}
		else if(found > 0)
		{
			results->push_back(str.substr(0,found));
			count++;
		}
		str = str.substr(found+1);
		found = str.find_first_of(separator);
	}

	if(str.length() > 0)
	{
		results->push_back(str);
		count++;
	}

	return (int)count;
}