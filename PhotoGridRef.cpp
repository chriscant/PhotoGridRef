/**************************************************************************
PhotoGridRef: Tool to find the UK or Irish Grid Reference from a photo's GPS info

See README.md and https://github.com/chriscant/

Copyright 2018 PHD Computer Consultants Ltd and contributors
www.phdcc.com

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

/*
//  Sample first bytes of some JPG files
//    FF D8 FF E0 00 10 4A 46 49 46
//    FF D8 FF E1 A3 FE 45 78 69 66
//    FF D8 FF E1 26 23 45 78 69 66
//
//  Statically linked so DLLs not needed: https://helloacm.com/how-to-static-link-vc-runtime-in-visual-c/
*/

/*
Could also link to OpenStreetMap or equivalent eg https://www.openstreetmap.org/#map=15/54.6896/-3.1342&layers=C
*/

#include "stdafx.h"
#include "PhotoGridRef.h"
#include "exif.h"
#include "os_coord.h"
#include "gt_irish.h"
#include "winreg.h"

const wchar_t MsgBoxTitle[] = L"Photo Grid Ref";

const wchar_t GridRefKey[] = L"Software\\Classes\\SystemFileAssociations\\image\\shell\\Grid Ref";
const wchar_t GridRefCommandKey[] = L"Software\\Classes\\SystemFileAssociations\\image\\shell\\Grid Ref\\command";
const wchar_t UninstallGridRefKey[] = L"Software\\Classes\\SystemFileAssociations\\image\\shell\\Uninstall Grid Ref";
const wchar_t UninstallGridRefCommandKey[] = L"Software\\Classes\\SystemFileAssociations\\image\\shell\\Uninstall Grid Ref\\command";


extern "C"
{
  // designated initializers removed
  #include "os_coord.h"
  #include "os_coord_math.h"
  #include "os_coord_data.h"
  #include "os_coord_transform.h"
  #include "os_coord_ordinance_survey.h"
}

void InstallHandler(const wchar_t *);
void UninstallHandlerQ(const wchar_t *);
void UninstallHandler(const wchar_t *);

/**
 * Show error message
 */
void ShowError(LPCTSTR lpText)
{
  MessageBoxW(NULL, lpText, MsgBoxTitle, MB_ICONWARNING | MB_OK);
}

/**
 * Get 2km sq tetrad letter from given coordinates, ie one of 25 letters A..N, P-Z missing out letter O
 * https://www.bto.org/volunteer-surveys/birdatlas/methods/correct-grid-references
 */
char GetTetradLetter(double E, double N) {
  int TetradE = (((int)E) % 10) / 2;
  int TetradN = (((int)N) % 10) / 2;
  int Tetrad = (TetradE * 5) + TetradN;
  if (Tetrad >= 14) Tetrad++;
  if (Tetrad < 0 || Tetrad > 25) return '?';
  return (char)(((int)'A') + Tetrad);
}

/**
 * Process open JPG ie
 *  do basic checks
 *  read entire contents
 *  extract EXIF information
 * If GPS info found, convert to GB grid ref, copy to clipboard and display
 *  or Eastings,Northings if shift key down
 *  or open in browser if Control key down
 */
void processJPG(HANDLE hJPG, LPWSTR lpCmdLine) {
  SHORT shiftDown = GetAsyncKeyState(VK_SHIFT) & 0x8000;  // If shift key down then return Eastings,Northings; otherwise 10 figure grid ref
  SHORT controlDown = GetAsyncKeyState(VK_CONTROL) & 0x8000;  // If control key down then open streetmap in default browser at that location

  // Get file size
  DWORD FileSizeHigh = 0;
  DWORD fs = GetFileSize(hJPG, &FileSizeHigh);
  // JPG must be at least 3 bytes long
  if ((FileSizeHigh==0) && (fs <= 3)) {
    ShowError(L"Empty file");
    return;
  }
  // JPG must start with FF D8 FF
  unsigned char first3[3];
  DWORD byteWritten = 0;
  BOOL ReadFileReturn = ReadFile(hJPG, first3, 3, &byteWritten, NULL);
  if (first3[0] != 0xFF || first3[1] != 0xD8 || first3[2] != 0xFF) {
    ShowError(L"Not a JPG");
    return;
  }
  SetFilePointer(hJPG, 0, NULL, FILE_BEGIN);

  // Anything this big can't be right
  if (FileSizeHigh) {
    ShowError(L"File too big");
    return;
  }

  // Read entire file into memory
  unsigned char* buf = new unsigned char[fs];
  if (buf == NULL) {
    ShowError(L"Out of memory!");
    return;
  }
  ReadFileReturn = ReadFile(hJPG, buf, fs, &byteWritten, NULL);
  if (ReadFileReturn)
  {
    if (byteWritten != fs) {
      ShowError(L"Not all file read");
      delete[] buf;
      return;
    }

    // Read EXIF info
    easyexif::EXIFInfo result;

    result.parseFrom(buf, fs);

    // Give up if no GPS info
    if (result.GeoLocation.Latitude == 0 && result.GeoLocation.Longitude == 0) {
      ShowError(L"No GPS info");
      delete[] buf;
      return;
    }
    double lat = result.GeoLocation.Latitude;
    double lng = result.GeoLocation.Longitude;
    /* Tetrad test points:
    lat = 52.872747351908075; // 52.872747351908075, 0.5578569871674302, TF7225033650, 572250, 333650, TF73, TF73G, TF7233, TF722336
    lng = 0.5578569871674302;
    lat = 53.00384865064986; // 53.00384865064986,-3.012544608621884, 332150, 345650, SJ321456, SJ3245, SJ34H
    lng = -3.012544608621884;
    lat = 52.95734995859699; // 52.95734995859699,-3.0360202539714254, 330500, 340500, SJ34A, SJ3040
    lng = -3.0360202539714254;
    // 330500, 342500, SJ34B, SJ3042+SJ3142+SJ3043+SJ3143
    lat = 52.96671874221891; // 52.96671874221891,-2.9915755823585135, 332500, 340500, SJ34F, SJ3240+SJ3340+SJ3241+SJ3341
    lng = -2.9915755823585135;*/

    PhotoGridRef::GT_WGS84 wgs84(lat, lng, result.GeoLocation.Altitude);

    os_grid_ref_t home_grid_ref;
    os_eas_nor_t home_en_airy30;
    if (wgs84.isIreland()) {
      //convert to Irish
      PhotoGridRef::GT_Irish irish = wgs84.getIrish();
      home_en_airy30.e = irish.eastings;
      home_en_airy30.n = irish.northings;
      home_en_airy30.h = irish.height;
      home_grid_ref = irish.getGridRef();
    }
    else {

      // Convert to OS Grid Reference
      os_lat_lon_t   home_ll_wgs84 = { DEG_2_RAD(lat), DEG_2_RAD(lng), result.GeoLocation.Altitude };

      // Convert from the WGS-84 ellipsoid to 3D Cartesian coordinates
      os_cartesian_t home_c_wgs84 = os_lat_lon_to_cartesian(home_ll_wgs84, OS_EL_WGS84);

      // Perform the Helmert transform to (approximately) correct for differences in datum
      os_cartesian_t home_c_airy30 = os_helmert_transform(home_c_wgs84, OS_HE_WGS84_TO_OSGB36);

      // Convert back to latitude, longitude and ellipsoidal height to give coordinates on the Airy 1830 ellipsoid.
      os_lat_lon_t home_ll_airy30 = os_cartesian_to_lat_lon(home_c_airy30, OS_EL_AIRY_1830);

      // Project the coordinates to the National Grid's 2D projection
      home_en_airy30 = os_lat_lon_to_tm_eas_nor(home_ll_airy30, OS_TM_NATIONAL_GRID);

      // Turn these Eastings and Northings into a grid references
      home_grid_ref = os_eas_nor_to_grid_ref(home_en_airy30, OS_GR_NATIONAL_GRID);
    }

    // Generate output string
    int MAX_BUF = 2000;
    wchar_t* complete = (wchar_t*)malloc(MAX_BUF * sizeof(wchar_t));
    const wchar_t* title = MsgBoxTitle;

    wchar_t* filename = wcsrchr(lpCmdLine, L'\\');
    if (filename == NULL) filename = lpCmdLine;
    else filename++;

    int length = 0;
    length += swprintf_s(complete + length, MAX_BUF - length, L"Filename:\t%s\r\n", filename);
    length += swprintf_s(complete + length, MAX_BUF - length, L"Date-time:\t%hs\r\n\r\n", &result.DateTime[0]);

    length += swprintf_s(complete + length, MAX_BUF - length, L"Lat,Long:\t\t%.6f, %.6f\r\n", lat, lng);
    length += swprintf_s(complete + length, MAX_BUF - length, L"\t\t%.0f'%.0f\"%.2f%c, %.0f'%.0f\"%.2f%c\r\n", result.GeoLocation.LatComponents.degrees, result.GeoLocation.LatComponents.minutes, result.GeoLocation.LatComponents.seconds, result.GeoLocation.LatComponents.direction, result.GeoLocation.LonComponents.degrees, result.GeoLocation.LonComponents.minutes, result.GeoLocation.LonComponents.seconds, result.GeoLocation.LonComponents.direction);
    if (result.GeoLocation.Altitude > 0) {
      length += swprintf_s(complete + length, MAX_BUF - length, L"Altitude:\t\t%.0fm\r\n", result.GeoLocation.Altitude);
    }
    length += swprintf_s(complete + length, MAX_BUF - length, L"\r\n");

    BOOL clipped = FALSE;   // Set clipboard data
  // If a GB Grid Reference:
    if (home_grid_ref.code[0] != '\0') {

      length += swprintf_s(complete + length, MAX_BUF - length, L"System:\t\t%s\r\n", wgs84.isIreland()?L"Ireland":L"Great Britain");
      length += swprintf_s(complete + length, MAX_BUF - length, L"10 Figure:\t%hs %05.0f %05.0f\t1m sq\r\n", home_grid_ref.code, home_grid_ref.e, home_grid_ref.n);
      length += swprintf_s(complete + length, MAX_BUF - length, L"All Figure:\t%06.0f,%06.0f\t1m sq\r\n", home_en_airy30.e, home_en_airy30.n);
      double E = floor(home_grid_ref.e / 100);
      double N = floor(home_grid_ref.n / 100);
      length += swprintf_s(complete + length, MAX_BUF - length, L"6 Figure:\t\t%hs%03.0f%03.0f\t100m sq\r\n", home_grid_ref.code, E, N);
      E = floor(home_grid_ref.e / 1000);
      N = floor(home_grid_ref.n / 1000);
      length += swprintf_s(complete + length, MAX_BUF - length, L"Monad:\t\t%hs%02.0f%02.0f\t\t1km sq\r\n", home_grid_ref.code, E, N);
      char TetradLetter = GetTetradLetter(E, N);
      E = floor(home_grid_ref.e / 10000);
      N = floor(home_grid_ref.n / 10000);
      length += swprintf_s(complete + length, MAX_BUF - length, L"Tetrad:\t\t%hs%01.0f%01.0f%c\t\t2km sq\r\n", home_grid_ref.code, E, N, TetradLetter);
      length += swprintf_s(complete + length, MAX_BUF - length, L"Hectad:\t\t%hs%01.0f%01.0f\t\t10km sq\r\n", home_grid_ref.code, E, N);

      /*for (double EE = 0; EE < 10; EE += 1) {
        for (double NN = 0; NN < 10; NN += 1) {
          TetradLetter = GetTetradLetter(EE, NN);
          length += swprintf_s(complete + length, MAX_BUF - length, L"%1.0f,%1.0f,%c-", EE, NN, TetradLetter);
        }
        length += swprintf_s(complete + length, MAX_BUF - length, L"\r\n");
      }*/

      // Generate text form of Grid Reference
      wchar_t gridref[100];
      int len;
      if (shiftDown) {
        len = swprintf_s(gridref, 100, L"%06.0f,%06.0f", home_en_airy30.e, home_en_airy30.n);
      }
      else {
        len = swprintf_s(gridref, 100, L"%hs%05.0f%05.0f", home_grid_ref.code, home_grid_ref.e, home_grid_ref.n);
      }

      // Copy to clipboard
      HGLOBAL hdst = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (len + 1) * sizeof(WCHAR)); // Allocate string for cwd
      LPWSTR dst = (LPWSTR)GlobalLock(hdst);
      memcpy(dst, gridref, len * sizeof(WCHAR));
      dst[len] = 0;
      GlobalUnlock(hdst);

      if (OpenClipboard(NULL)) {
        EmptyClipboard();
        if (SetClipboardData(CF_UNICODETEXT, hdst)) {
          clipped = TRUE;
        }
        CloseClipboard();
      }
      if (clipped) title = L"Grid Ref Copied to Clipboard";
      else title = L"UNABLE TO COPY TO CLIPBOARD";

      if (controlDown) {
        wchar_t url[300];
        int len = swprintf_s(url, 300, L"https://streetmap.co.uk/map.srf?X=%06.0f&Y=%06.0f&A=Y&Z=115", home_en_airy30.e, home_en_airy30.n);
        ShellExecuteW(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
      }
    }
    else {
      length += swprintf_s(complete + length, MAX_BUF - length, L"\r\nLocation not within GB");
    }
    length += swprintf_s(complete + length, MAX_BUF - length, L"\r\n\r\nResults only as accurate as your phone or camera.");
    if (!controlDown) {
      // Show message to user
      MessageBoxW(NULL, complete, title, MB_OK);
    }
    free(complete);
  }
  delete[] buf;
}

/**
 * Get version number and copyright string from the given path, ie this exe
 * Sets Version
 *
 * Returns:
 *  Copyright string
 */
std::wstring GetFileVersionCopyright(wchar_t* Filename, DWORD Version[4])
{
  std::wstring copyright;

  memset(Version, 0, sizeof(Version));

  DWORD UnusedHandle;
  DWORD fvis = GetFileVersionInfoSize(Filename, &UnusedHandle);
  if (fvis == 0)
    return copyright;

  bool GotInfo = false;
  BYTE* fvi = new BYTE[fvis];
  if (GetFileVersionInfo(Filename, UnusedHandle, fvis, fvi))
  {
    LPVOID data;
    UINT bytes;
    if (VerQueryValue(fvi, TEXT("\\"), &data, &bytes))
    {
      VS_FIXEDFILEINFO* vsffi = (VS_FIXEDFILEINFO*)data;
      Version[0] = HIWORD(vsffi->dwFileVersionMS);
      Version[1] = LOWORD(vsffi->dwFileVersionMS);
      Version[2] = HIWORD(vsffi->dwFileVersionLS);
      Version[3] = LOWORD(vsffi->dwFileVersionLS);
      GotInfo = true;
    }

    UINT chars;
    if (VerQueryValue(fvi, TEXT("\\StringFileInfo\\080904B0\\LegalCopyright"),		// UK
      &data, &chars))
    {
      copyright.resize(chars + 1);
      wcsncpy_s(&copyright[0], chars+1, (const wchar_t *)data, chars);
    }
    else if (VerQueryValue(fvi, TEXT("\\StringFileInfo\\040904B0\\LegalCopyright"),	// US
      &data, &chars))
    {
      copyright.resize(chars + 1);
      wcsncpy_s(&copyright[0], chars + 1, (const wchar_t *)data, chars);
    }
  }

  delete[] fvi;
  return copyright;
}

/**
 * If tool run without any parameters, install, check or uninstall the context menu handlers
 *
 * Registry entries checked/added for a "static verb" shortcut menu.
 * The verbs are added for the "image" type; it could be more restrictive ie just JPG or similar, but several different extensions are JPEG files.
 */
void InstallOrUninstallHandler(wchar_t *ExePath) {
  // Form our desired main command line
  wchar_t cmdLine[_MAX_PATH+1];
  int len = swprintf_s(cmdLine, _MAX_PATH, L"\"%ws\" %%1", ExePath);
  DWORD bnLen = (len + 1) << 1;

  // Try to read current main context menu command
  DWORD dataSize;
  DWORD retCode = ::RegGetValue(HKEY_CURRENT_USER, GridRefCommandKey, NULL, RRF_RT_REG_SZ, NULL, NULL, &dataSize);  // Get size
  if (retCode == ERROR_SUCCESS) {
    std::wstring gridRefCommand;
    gridRefCommand.resize(dataSize / sizeof(wchar_t));

    retCode = ::RegGetValue(HKEY_CURRENT_USER, GridRefCommandKey, NULL, RRF_RT_REG_SZ, NULL, &gridRefCommand[0], &dataSize);  // Read default string
    if (retCode == ERROR_SUCCESS) {
      // Compare to expected command line
      if (wcscmp(&gridRefCommand[0], cmdLine) == 0) {
        // MAIN COMMAND MATCH
        if (MessageBoxW(NULL, L"Tool already installed\r\n\r\nDo you want to remove it?", MsgBoxTitle, MB_OKCANCEL) == IDOK) {
          UninstallHandler(ExePath);
        }
        return;
      }
      // MISMATCH
      if (MessageBoxW(NULL, L"Do you want to re-install Grid Ref?", MsgBoxTitle, MB_OKCANCEL) == IDOK) {
        InstallHandler(ExePath);
      }
      else {
        UninstallHandlerQ(ExePath);
      }
      return;
    }
    else {
      ShowError(L"RegGetValue FAIL 2");
    }
  }
  else {
    // If not present, then install
    InstallHandler(ExePath);
  }
}

/**
 * Install the context menu handler
 */
void InstallHandler(const wchar_t *ExePath) {
  HKEY hKey;
  DWORD disp;

  // Create/Open main context menu command key
  LSTATUS retCode = ::RegCreateKeyExW(HKEY_CURRENT_USER, GridRefCommandKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &disp);
  if (retCode != ERROR_SUCCESS) {
    ShowError(L"No permission to install: RegCreateKeyExW #1");
    return;
  }
  // Write main command string to (Default)
  wchar_t cmdLine[_MAX_PATH + 1];
  int len = swprintf_s(cmdLine, _MAX_PATH, L"\"%ws\" %%1", ExePath);
  DWORD bnLen = (len + 1) << 1;
  retCode = ::RegSetValueExW(hKey, NULL, 0, REG_SZ, (const BYTE *)cmdLine, bnLen);
  ::RegCloseKey(hKey);
  if (retCode != ERROR_SUCCESS) {
    ShowError(L"No permission to install: RegSetValueExW #1");
    return;
  }

  // Create/Open Uninstall context menu key
  retCode = ::RegCreateKeyExW(HKEY_CURRENT_USER, UninstallGridRefKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &disp);
  if (retCode != ERROR_SUCCESS) {
    ShowError(L"No permission to install: RegCreateKeyExW #2");
    return;
  }
  // Set empty "Extended" value to indicate that verb only shows when Shift key pressed
  retCode = ::RegSetValueExW(hKey, L"Extended", 0, REG_SZ, NULL, 0);
  ::RegCloseKey(hKey);
  if (retCode != ERROR_SUCCESS) {
    ShowError(L"No permission to install: RegSetValueExW #2");
    return;
  }
  // Create/Open Uninstall context menu command key
  retCode = ::RegCreateKeyExW(HKEY_CURRENT_USER, UninstallGridRefCommandKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &disp);
  if (retCode != ERROR_SUCCESS) {
    ShowError(L"No permission to install: RegCreateKeyExW #3");
    return;
  }
  // Write uninstall command string to (Default)
  len = swprintf_s(cmdLine, _MAX_PATH, L"\"%ws\" -u", ExePath);
  bnLen = (len + 1) << 1;
  retCode = ::RegSetValueExW(hKey, NULL, 0, REG_SZ, (const BYTE *)cmdLine, bnLen);
  ::RegCloseKey(hKey);
  if (retCode != ERROR_SUCCESS) {
    ShowError(L"No permission to install: RegSetValueExW #3");
    return;
  }

  MessageBoxW(NULL, L"Tool installed", MsgBoxTitle, MB_OK);
}

/**
 * Ask whether to uninstall the context menu handler
 */
void UninstallHandlerQ(const wchar_t *ExePath) {
  if (MessageBoxW(NULL, L"Do you want to uninstall Grid Ref?", MsgBoxTitle, MB_OKCANCEL) != IDOK)
    return;

  UninstallHandler(ExePath);
}

/**
 * Uninstall the context menu handler
 */
void UninstallHandler(const wchar_t *ExePath) {
  LSTATUS retCode = ::RegDeleteKeyW(HKEY_CURRENT_USER, GridRefCommandKey);
  if (retCode != ERROR_SUCCESS) {
    ShowError(L"No permission to uninstall: RegDeleteKeyW #1");
    return;
  }
  retCode = ::RegDeleteKeyW(HKEY_CURRENT_USER, GridRefKey);
  if (retCode != ERROR_SUCCESS) {
    ShowError(L"No permission to uninstall: RegDeleteKeyW #2");
    return;
  }
  retCode = ::RegDeleteKeyW(HKEY_CURRENT_USER, UninstallGridRefCommandKey);
  if (retCode != ERROR_SUCCESS) {
    ShowError(L"No permission to uninstall: RegDeleteKeyW #3");
    return;
  }
  retCode = ::RegDeleteKeyW(HKEY_CURRENT_USER, UninstallGridRefKey);
  if (retCode != ERROR_SUCCESS) {
    ShowError(L"No permission to uninstall: RegDeleteKeyW #4");
    return;
  }

  // Try to delete own exe on reboot but seems to fail with GetLastError() = 5 if not admin
  MoveFileExW(ExePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT); 

  MessageBoxW(NULL, L"Tool uninstalled", MsgBoxTitle, MB_OK);
}

/**
 * Main application entry point
 */
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

    // Get our path and so Version and Copyright, in case needed
    wchar_t ExePath[_MAX_PATH];
    GetModuleFileName(NULL, ExePath, _MAX_PATH);
    DWORD ExeVersion[4];
    std::wstring copyright = GetFileVersionCopyright(ExePath, ExeVersion);

    // Handle install and uninstall handler cases
    if (wcslen(lpCmdLine) == 0) {
      InstallOrUninstallHandler(ExePath);
      return 0;
    }
    if (wcscmp(lpCmdLine, L"-u")==0) {
      UninstallHandlerQ(ExePath);
      return 0;
    }

    // Open given file path - entire command line
    HANDLE hJPG = CreateFileW(lpCmdLine, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hJPG == INVALID_HANDLE_VALUE) {
      wchar_t error[300];
      int len = swprintf_s(error, 300, L"Could not open %ws", lpCmdLine);
      ShowError(error);
      return 0;
    }
    // Process file and close
    processJPG(hJPG, lpCmdLine);
    CloseHandle(hJPG);
    return 0;
}

