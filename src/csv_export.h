//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2023 Mark Jansen
// UI Framework: Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#pragma once

class export_error : public std::runtime_error
{
public:
    export_error(const std::string& error)
        :runtime_error(error)
    {
    }
};

void
export_csv(HANDLE hFile, HWND hList);

