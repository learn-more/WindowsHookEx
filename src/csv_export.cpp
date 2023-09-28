//
// WindowsHookEx - Test the behavior of the api SetWindowsHookEx
// Copyright (c) 2023 Mark Jansen
// UI Framework: Wizard-2020 Example from https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-1
// Copyright (c) 2020 Colin Finck, ENLYZE GmbH
// SPDX-License-Identifier: MIT
//

#include "Wizard-2020.h"

static void
write(HANDLE hFile, const char* text)
{
    DWORD cbWritten;
    DWORD cbLen = (DWORD)strlen(text);
    if (!WriteFile(hFile, text, cbLen, &cbWritten, nullptr))
        throw export_error("Unable to write data.\r\nError: " + std::to_string(GetLastError()));
    if (cbWritten != cbLen)
        throw export_error("Could not write all data.");
}

inline static BOOL
ListView_GetColumnA(HWND hwnd, int columnCount, LVCOLUMNA& column)
{
    return (BOOL)SendMessageA(hwnd, LVM_GETCOLUMNA, (WPARAM)columnCount, (LPARAM)&column);
}

inline static BOOL
ListView_GetItemA(HWND hwnd, LV_ITEMA& item)
{
    return (BOOL)SendMessageA(hwnd, LVM_GETITEMA, 0, (LPARAM)&item);
}

void
export_csv(HANDLE hFile, HWND hList)
{
    LVCOLUMNA column = { LVCF_TEXT };
    std::string Buffer(MAX_PATH, '\0');
    column.pszText = Buffer.data();
    column.cchTextMax = (int)Buffer.size();
    int columnCount = 0;

    write(hFile, "sep=,\r\n");  // Tell excel that we use a 'comma' as delimiter
    for (; ListView_GetColumnA(hList, columnCount, column); columnCount++)
    {
        // Each cell in the header is quoted, so we don't have to check for comma's in the data
        if (!columnCount)
            write(hFile, "\"");
        else
            write(hFile, "\",\"");
        write(hFile, Buffer.c_str());
    }
    write(hFile, "\"\r\n");

    const int itemCount = ListView_GetItemCount(hList);
    Buffer.resize(512);
    for (int index = 0; index < itemCount; index++)
    {
        LV_ITEMA item = {};
        for (int col = 0; col < columnCount; ++col)
        {
            item.iItem = index;
            item.iSubItem = col;
            item.mask = LVIF_TEXT;
            item.pszText = Buffer.data();
            item.cchTextMax = (int)Buffer.size();
            if (ListView_GetItemA(hList, item))
            {
                // Each cell in the data is quoted, so we don't have to check for comma's in the data
                if (!col)
                    write(hFile, "\"");
                else
                    write(hFile, "\",\"");

                write(hFile, Buffer.c_str());
            }
        }
        write(hFile, "\"\r\n");
    }
}
