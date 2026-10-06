// This file is part of Micropolis-SDLPP
// Micropolis-SDLPP is based on Micropolis
//
// Copyright © 2022 - 2026 Leeor Dicker
//
// Portions Copyright © 1989-2007 Electronic Arts Inc.
//
// Micropolis-SDLPP is free software; you can redistribute it and/or modify
// it under the terms of the GNU GPLv3, with additional terms. See the README
// file, included in this distribution, for details.
#pragma once

#include <SDL3/SDL.h>
//#include <SDL3/SDL_syswm.h>

#include <string>
#include <functional>
#include "../Ruleset.h"

class FileIoDialog
{
public:
	FileIoDialog() = delete;
	FileIoDialog(const FileIoDialog&) = delete;
	const FileIoDialog& operator=(const FileIoDialog&) = delete;

	FileIoDialog(SDL_Window& window);
	~FileIoDialog();

	const std::string& savePath() const { return mSavePath; }
	const std::string& fileName() const { return mFileName; }
	const std::string fullPath() const { return mSavePath + mSeparator + mFileName; }

    void clearSaveFilename();
    const std::string& openPath() const { return mOpenPath; }
    const std::string& exportPath() const { return mExportPath; }
    void errorHandler(std::function<void(const std::string&)> handler) { mErrorHandler = std::move(handler); }
    
	bool pickSaveFile(RulesetId = RulesetId::ClassicV1);
	bool pickOpenFile();
    bool pickImportFile();
    bool pickExportFile();

	bool filePicked() const;

private:
	enum class FileOperation { Open, Save, Import, Export };

	bool showFileDialog(FileOperation, RulesetId = RulesetId::ClassicV1);

	std::string mSavePath;
    std::string mOpenPath;
    std::string mExportPath;
    std::function<void(const std::string&)> mErrorHandler;
	std::string mFileName;
    std::string mSeparator;
};
