/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** UtlString.h
** String utilities
**
** Author: Brian Bailey www.code-hammer.com
** -------------------------------------------------------------------------*/

#ifndef UTLSTRING_H
#define UTLSTRING_H

#include <string>
#include <vector>

class UtlString
{
public:
	static std::string toLower(const std::string &s);
	static int explode(std::string str, const std::string &separator, std::vector<std::string>* results);
};

#endif