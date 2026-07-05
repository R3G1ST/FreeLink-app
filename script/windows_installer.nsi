; =============================================
; FreeLink Installer - Modern Dark Theme
; =============================================
!include MUI2.nsh
!include nsDialogs.nsh
!include LogicLib.nsh
!include FileFunc.nsh
!include WinVer.nsh
!include x64.nsh
!addplugindir "../script"

; =============================================
; BRANDING
; =============================================
Name "FreeLink"
OutFile "FreeLink-Setup.exe"
InstallDir "$LOCALAPPDATA\FreeLink"
RequestExecutionLevel user
SetCompressor /SOLID /FINAL lzma
SetCompressorDictSize 64

!define MUI_ABORTWARNING

; =============================================
; PAGES
; =============================================
!define MUI_WELCOMEPAGE_TITLE "Welcome to FreeLink"
!define MUI_WELCOMEPAGE_TEXT "FreeLink is a cross-platform VPN client powered by Sing-box.$\n$\nThis installer will guide you through the installation.$\n$\nFeatures:$\n  Multi-protocol: VLESS, Trojan, Shadowsocks, Hysteria2, WireGuard$\n  System proxy & TUN mode$\n  Subscription & deeplink support"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "../LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_TITLE "FreeLink Installed"
!define MUI_FINISHPAGE_TEXT "FreeLink has been installed successfully.$\n$\nClick Finish to launch FreeLink."
!define MUI_FINISHPAGE_RUN "$INSTDIR\FreeLink.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch FreeLink"
!insertmacro MUI_PAGE_FINISH

; Uninstaller
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; =============================================
; VERSION INFO
; =============================================
VIProductVersion "1.0.0.0"
VIAddVersionKey "ProductName" "FreeLink"
VIAddVersionKey "FileDescription" "FreeLink VPN Client Installer"
VIAddVersionKey "LegalCopyright" "2026 FreeLink"
VIAddVersionKey "FileVersion" "1.0.0"

UninstallText "This will uninstall FreeLink. Do you wish to continue?"

; =============================================
; MACROS
; =============================================
!macro AbortOnRunningApp EXEName
  killModule:
  FindProcDLL::FindProc ${EXEName}
  Pop $R0
  IntCmp $R0 1 0 notRunning
    FindProcDLL::KillProc ${EXEName}
    Sleep 1000
    Goto killModule
  notRunning:
!macroend

; =============================================
; INSTALL SECTION
; =============================================
Section "FreeLink (Required)" SecMain
  SectionIn RO
  
  SetOutPath "$INSTDIR"
  SetOverwrite on
  
  ; Kill running instance
  !insertmacro AbortOnRunningApp "$INSTDIR\FreeLink.exe"
  
  ; Install binaries based on architecture
  ${If} ${IsNativeAMD64}
    File /oname=libcronet.dll "../deployment/windows-amd64/libcronet.dll"
    File /oname=FreeLinkCore.exe "../deployment/windows-amd64/FreeLinkCore.exe"
    File /oname=FreeLink.exe "../deployment/windows-amd64/FreeLink.exe"
    File /oname=updater.exe "../deployment/windows-amd64/updater.exe"
  ${Else}
    Abort "This installer only supports 64-bit Windows. Please download the legacy build for 32-bit systems."
  ${EndIf}
  
  ; Shortcuts
  CreateShortcut "$DESKTOP\FreeLink.lnk" "$INSTDIR\FreeLink.exe" "" "$INSTDIR\FreeLink.exe" 0
  CreateShortcut "$SMPROGRAMS\FreeLink.lnk" "$INSTDIR\FreeLink.exe" "" "$INSTDIR\FreeLink.exe" 0
  
  ; Registry
  WriteRegStr SHCTX "Software\FreeLink" "InstallPath" "$INSTDIR"
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\FreeLink" "DisplayName" "FreeLink"
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\FreeLink" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\FreeLink" "InstallLocation" "$INSTDIR"
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\FreeLink" "DisplayIcon" "$INSTDIR\FreeLink.exe"
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\FreeLink" "Publisher" "FreeLink"
  WriteRegDWORD SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\FreeLink" "NoModify" 1
  WriteRegDWORD SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\FreeLink" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
SectionEnd

; =============================================
; UNINSTALLER
; =============================================
Section "Uninstall"
  !insertmacro AbortOnRunningApp "$INSTDIR\FreeLink.exe"
  
  Delete "$SMPROGRAMS\FreeLink.lnk"
  Delete "$DESKTOP\FreeLink.lnk"
  RMDir "$SMPROGRAMS\FreeLink"
  
  RMDir /r "$INSTDIR"
  
  DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\FreeLink"
  DeleteRegKey SHCTX "Software\FreeLink"
SectionEnd
