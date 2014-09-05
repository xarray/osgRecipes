/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** NgpFileName.cpp
** Class to parse ngp model file settings that are embedded into the file name.
**
** Author: Brian Bailey www.code-hammer.com
** -------------------------------------------------------------------------*/

#include "NgpFileName.h"
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include "UtlString.h"

//============================================================================
//============================================================================
NgpFileName::NgpFileName()
{
}

//============================================================================
//============================================================================
NgpFileName::NgpFileName(const std::string &fullPath)
{
	parseName(fullPath);
}

//============================================================================
//============================================================================
bool NgpFileName::hasScale() const
{
	if (fabs(_settings.scale - 1) > 0.001) return true;

	return false;
}

//============================================================================
//============================================================================
bool NgpFileName::parseName(const std::string &fullPath)
{
	_settings = Settings();

	std::string path = getPath(fullPath);
	std::string file = getFileName(fullPath);

	// explode all settings
	std::vector<std::string> vsettings;
	UtlString::explode(file, "#", &vsettings);
	if (!vsettings.size())
	{
		OSG_WARN << "Did not find anysettings: " << std::endl;

		_settings.filePath = path + file;
	}
	else
	{

		_settings.filePath = path + vsettings[vsettings.size()-1];
		vsettings.pop_back();
		parseSettings(vsettings, &_settings);
	}

	return true;
}

//============================================================================
//============================================================================
std::string NgpFileName::getPath(const std::string &filePath, bool includeLastSep)
{
	std::string path = osgDB::getFilePath(filePath);
	if (!path.size()) return path;

	if (includeLastSep)
	{
		char c = path.at(path.size()-1);
		if (c != '\\' && c != '/') path.push_back('/');
	}

	return path;
}

//============================================================================
//============================================================================
std::string NgpFileName::getFileName(const std::string &filePath)
{
	std::string name;
	bool foundSep = false;
	size_t lenToSep = 0;
	size_t lenTot = filePath.size();

	if (lenTot <= 0) return name;


	for (size_t i=lenTot-1; i>0; i--)
	{
		lenToSep++;
		if (filePath[i] == '\\' || filePath[i] == '/') 
		{
			foundSep = true;
			break;
		}
	}

	if (!foundSep)
	{
		return filePath;
	}

	name = filePath;
	int numToErase = lenTot - lenToSep + 1;
	if (numToErase <= 0) return name;
	name.erase(0, numToErase);

	return name;
}

//============================================================================
//============================================================================
bool NgpFileName::parseSettings(const std::vector<std::string> &strSettings, Settings *settings)
{
	for (unsigned int i=0; i<strSettings.size(); i++)
	{
		std::vector<std::string> results;
		UtlString::explode(strSettings[i], "=", &results);
		if (results.size() != 2)
		{
			OSG_WARN << "Unable to parse setting: " << strSettings[i] << std::endl;
			continue;
		}

		std::string setting = UtlString::toLower(results[0]);

		if (setting == "seed")
		{
			parseSeed(results[1], settings);
		}
		else if (setting == "trans")
		{
			parseTrans(results[1], settings);
		}
		else if (setting == "scale")
		{
			parseScale(results[1], settings);
		}
		else
		{
			OSG_WARN << "Unrecognized setting: " << results[0] << std::endl;
		}
	}

	return true;
}

//============================================================================
//============================================================================
bool NgpFileName::parseSeed(const std::string &setting, Settings *settings)
{
	if (!setting.size())
	{
		OSG_WARN << "Seed is empty, defaulting to 1." << std::endl;
		settings->seed = 1;
		return false;
	}

	settings->seed = atoi(setting.c_str());
	return true;
}

//============================================================================
//============================================================================
bool NgpFileName::parseTrans(const std::string &setting, Settings *settings)
{
	// default
	settings->alphaMode = Blend;

	if (!setting.size())
	{
		OSG_WARN << "Trans is empty, defaulting to BLEND." << std::endl;
		return false;
	}

	std::vector<std::string> results;
	UtlString::explode(setting, ",", &results);
	if (!results.size())
	{
		OSG_WARN << "Failed to parse trans " << setting << ", defaulting to BLEND." << std::endl;
		return false;
	}

	std::string value = UtlString::toLower(results[0]);
	if (value == "blend")
	{
		settings->alphaMode = Blend;
		return true;
	}

	if (value == "test")
	{
		settings->alphaMode = Test;
		if (results.size() < 2)
		{
			OSG_WARN << "Trans test threshold not specified, defaulting to .5 " << std::endl;
			settings->alphaThreshold = .5f;
			return true;
		}

		
		settings->alphaThreshold = atof(results[1].c_str());
		return true;
	}

	OSG_WARN << "Trans mode not recognized " << value << ", defaulting to BLEND" << std::endl;	
	return false;
}

//============================================================================
//============================================================================
bool NgpFileName::parseScale(const std::string &setting, Settings *settings)
{
	if (!setting.size())
	{
		OSG_WARN << "Scale is empty, defaulting to 1." << std::endl;
		settings->scale = 1;
		return false;
	}

	settings->scale = atof(setting.c_str());
	return true;
}