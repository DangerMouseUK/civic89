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
#include "FileIoDialog.h"
#include "../CityIo.h"

#include <filesystem>
#include <stdexcept>

#include "nfd.hpp"


FileIoDialog::FileIoDialog(SDL_Window&)
{
    if(NFD::Init() != NFD_OKAY)
    {
        throw std::runtime_error("Unable to initialise file picker library.");
    }
    
    mSeparator = std::filesystem::path::preferred_separator;
}


FileIoDialog::~FileIoDialog()
{
    NFD::Quit();
}


void FileIoDialog::clearSaveFilename()
{
    mFileName.clear();
}


bool FileIoDialog::filePicked() const
{
    return !mFileName.empty();
}


/**
 * \return Returns true if a file name was selected, false otherwise.
 */
bool FileIoDialog::pickSaveFile(RulesetId ruleset)
{
    const auto filePicked = showFileDialog(FileOperation::Save,ruleset);
    
    return filePicked;
}


/**
 * \return Returns true if a file name was selected, false otherwise.
 */
bool FileIoDialog::pickOpenFile()
{
    const auto filePicked = showFileDialog(FileOperation::Open);

    return filePicked;
}


/**
 * \return Returns true if a file name has been picked. False otherwise.
 */
bool FileIoDialog::pickImportFile() { return showFileDialog(FileOperation::Import); }
bool FileIoDialog::pickExportFile() { return showFileDialog(FileOperation::Export); }

bool FileIoDialog::showFileDialog(FileOperation operation, RulesetId ruleset)
{
    const auto* definition=findRuleset(ruleset);
    if (!definition) { return false; }
    NFD::UniquePath outPath;
    nfdfilteritem_t filterItem[2] = {{"Classic city", "cty"},{"Enhanced city", "c89"}};
    const bool opening=operation==FileOperation::Open || operation==FileOperation::Import;
    const nfdfiltersize_t count=operation==FileOperation::Open ? 2 : 1;
    if (operation==FileOperation::Save && ruleset==RulesetId::EnhancedV1) { filterItem[0]=filterItem[1]; }
       
    const auto result = opening ? NFD::OpenDialog(outPath, filterItem, count) : NFD::SaveDialog(outPath, filterItem, count);
    if (result == NFD_CANCEL) { return false; }
    if (result != NFD_OKAY)
    {
        if (mErrorHandler) { mErrorHandler(NFD::GetError() ? NFD::GetError() : "File picker failed."); }
        return false;
    }
    if (opening) { mOpenPath = outPath.get(); return true; }
    auto selected=pathFromUtf8(outPath.get());
    if (!selected.has_extension()) { selected += operation==FileOperation::Export ? ".cty" : std::string(definition->extension); }
    const auto selectedUtf8=selected.u8string();
    const std::string filename(selectedUtf8.begin(),selectedUtf8.end());
    if (operation==FileOperation::Export) { mExportPath=filename; return true; }
    mFileName = filename;
    
    std::size_t location = mFileName.find_last_of(mSeparator);
    mSavePath = mFileName.substr(0, location);
    mFileName = mFileName.substr(location + 1);

    return true;
}
