/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** NgpFileName.h
** Class to parse ngp model file settings that are embedded into the file name.
**
** Author: Brian Bailey www.code-hammer.com
** -------------------------------------------------------------------------*/

#ifndef NGPFILENAME_H
#define NGPFILENAME_H

#include <string>
#include <vector>

class NgpFileName
{
public:
	enum AlphaMode
	{
		Blend = 1,
		Test
	};

	struct Settings
	{
		std::string filePath;
		int seed;
		float scale;
		int alphaMode;
		float alphaThreshold;

		Settings()
		{
			seed = 1;
			scale = 1;
			alphaMode = Blend;
			alphaThreshold = .05f;
		}
	};

public:
	NgpFileName();
	NgpFileName(const std::string &fullPath);

	bool parseName(const std::string &fullPath);

	bool hasScale() const;

	const Settings& settings() const { return _settings; }
	void setFilePath(std::string &filePath) { _settings.filePath = filePath; }

protected:
	std::string getPath(const std::string &filePath, bool includeLastSep=true);
	std::string getFileName(const std::string &filePath);

	bool parseSettings(const std::vector<std::string> &strSettings, Settings *settings);
	bool parseSeed(const std::string &strSetting, Settings *settings);
	bool parseTrans(const std::string &strSetting, Settings *settings);
	bool parseScale(const std::string &strSetting, Settings *settings);
	
protected:
	Settings _settings;

};

#endif