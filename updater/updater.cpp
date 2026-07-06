/**
 * FreeLink Updater
 * 
 * Simple updater that:
 * 1. Finds FreeLink.zip in current directory
 * 2. Extracts to FreeLink_update/
 * 3. Kills running FreeLink.exe
 * 4. Copies new files to current directory
 * 5. Cleans up temp files
 * 6. Starts FreeLink.exe
 * 
 * Build: g++ -O2 -o updater.exe updater.cpp
 */

#include <iostream>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

namespace fs = std::filesystem;

// Kill FreeLink process
void killProcess(const std::string& processName) {
#ifdef _WIN32
    std::string cmd = "taskkill /f /im " + processName + " >nul 2>&1";
    system(cmd.c_str());
#else
    std::string cmd = "killall " + processName + " 2>/dev/null";
    system(cmd.c_str());
#endif
}

// Wait for process to exit
bool isProcessRunning(const std::string& processName) {
#ifdef _WIN32
    std::string cmd = "tasklist /fi \"imagename eq " + processName + "\" 2>nul | find /i \"" + processName + "\" >nul 2>&1";
    return system(cmd.c_str()) == 0;
#else
    std::string cmd = "pgrep " + processName + " > /dev/null 2>&1";
    return system(cmd.c_str()) == 0;
#endif
}

// Extract ZIP using system tools
bool extractZip(const std::string& zipPath, const std::string& destDir) {
#ifdef _WIN32
    // Use PowerShell to extract
    std::string cmd = "powershell -Command \"Expand-Archive -Path '" + zipPath + "' -DestinationPath '" + destDir + "' -Force\"";
#else
    // Use unzip
    std::string cmd = "unzip -o '" + zipPath + "' -d '" + destDir + "' > /dev/null 2>&1";
#endif
    return system(cmd.c_str()) == 0;
}

// Copy directory recursively
void copyDir(const fs::path& src, const fs::path& dst) {
    if (!fs::exists(dst)) {
        fs::create_directories(dst);
    }
    
    for (const auto& entry : fs::directory_iterator(src)) {
        const auto& srcPath = entry.path();
        auto dstPath = dst / srcPath.filename();
        
        if (fs::is_directory(srcPath)) {
            copyDir(srcPath, dstPath);
        } else {
            // Retry copy a few times (file might be locked)
            for (int i = 0; i < 5; i++) {
                try {
                    fs::copy_file(srcPath, dstPath, fs::copy_options::overwrite_existing);
                    break;
                } catch (const fs::filesystem_error&) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            }
        }
    }
}

// Remove directory recursively
void removeDir(const fs::path& dir) {
    if (fs::exists(dir)) {
        fs::remove_all(dir);
    }
}

int main(int argc, char* argv[]) {
    // Get current directory (where FreeLink.exe is)
    fs::path appDir = fs::current_path();
    fs::path zipPath = appDir / "FreeLink.zip";
    fs::path updateDir = appDir / "FreeLink_update";
    fs::path updateSubDir = updateDir / "FreeLink";
    
    // Check if update package exists
    if (!fs::exists(zipPath)) {
#ifdef _WIN32
        MessageBoxA(NULL, "No update package found.\nPlease download FreeLink.zip first.", "FreeLink Updater", MB_OK | MB_ICONERROR);
#else
        std::cerr << "No update package found. Please download FreeLink.zip first." << std::endl;
#endif
        return 1;
    }
    
#ifdef _WIN32
    MessageBoxA(NULL, "Starting update...", "FreeLink Updater", MB_OK | MB_ICONINFORMATION);
#endif
    
    // Kill running FreeLink
    killProcess("FreeLink.exe");
    killProcess("FreeLinkCore.exe");
    
    // Wait for process to exit
    for (int i = 0; i < 10; i++) {
        if (!isProcessRunning("FreeLink.exe")) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    // Extract ZIP
    removeDir(updateDir);
    if (!extractZip(zipPath.string(), updateDir.string())) {
#ifdef _WIN32
        MessageBoxA(NULL, "Failed to extract update package.", "FreeLink Updater", MB_OK | MB_ICONERROR);
#else
        std::cerr << "Failed to extract update package." << std::endl;
#endif
        return 1;
    }
    
    // Check if extracted directory exists
    fs::path sourceDir = updateSubDir;
    if (!fs::exists(sourceDir)) {
        // ZIP might not have root folder, try updateDir directly
        sourceDir = updateDir;
    }
    
    // Remove old files (DLLs, etc.)
    for (const auto& entry : fs::directory_iterator(appDir)) {
        if (entry.path().extension() == ".dll" || 
            entry.path().extension() == ".dmp" ||
            entry.path().filename() == "FreeLink.zip") {
            try {
                fs::remove(entry.path());
            } catch (...) {}
        }
    }
    
    // Copy new files
    copyDir(sourceDir, appDir);
    
    // Cleanup
    removeDir(updateDir);
    try {
        fs::remove(zipPath);
    } catch (...) {}
    
    // Start FreeLink
#ifdef _WIN32
    fs::path exePath = appDir / "FreeLink.exe";
    ShellExecuteA(NULL, "open", exePath.string().c_str(), NULL, appDir.string().c_str(), SW_SHOWNORMAL);
#else
    std::string cmd = "\"" + (appDir / "FreeLink").string() + "\" &";
    system(cmd.c_str());
#endif
    
    return 0;
}
