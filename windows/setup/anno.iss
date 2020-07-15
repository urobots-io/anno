; visual studio output folder
#define ProductReleaseFolder "..\x64\Release\"
; Qt source path
#define PATH_FILE FileOpen(ProductReleaseFolder + "_qt_path.txt")
#define SourceQtFolder FileRead(PATH_FILE)
#expr FileClose(PATH_FILE)
#undef PATH_FILE

#define MyAppName "anno"
#define MyAppExeName "anno.exe"
#define MyAppVersion GetFileVersion(ProductReleaseFolder + MyAppExeName)
#define MyAppPublisher "urobots GmbH"
#define MyAppURL "http://www.urobots.io/"
#define MyAppIcon "../../anno/Resources/anno.ico"
#define MyAppMutex "C8F8B7AE-2E70-4B3D-BD87-59D8C74CBA72"

#expr Exec(SourcePath + "\prepare_binaries.bat", "", SourcePath)

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{EFFA4009-442C-46F1-8220-CDD310572316}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
ArchitecturesInstallIn64BitMode=x64
DefaultDirName={pf}\urobots\anno
DefaultGroupName=urobots\{#MyAppName}
DisableProgramGroupPage=yes
OutputBaseFilename=anno_setup_{#MyAppVersion}
Compression=lzma
SolidCompression=yes
SetupIconFile={#MyAppIcon}
AppMutex={#MyAppMutex}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1
Name: "annoAssociation"; Description: "Associate ""anno"" extension"; GroupDescription: File extensions:
Name: "annoProtocol"; Description: "Associate ""anno"" URL protocol"; GroupDescription: URL protocol:

[Registry]
Root: HKCR; Subkey: ".anno"; ValueType: string; ValueName: ""; ValueData: "UrobotsAnnotationTool"; Flags: uninsdeletevalue; Tasks: annoAssociation
Root: HKCR; Subkey: "UrobotsAnnotationTool"; ValueType: string; ValueName: ""; ValueData: "Annotation File"; Flags: uninsdeletekey; Tasks: annoAssociation
Root: HKCR; Subkey: "UrobotsAnnotationTool\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\anno.ico"; Tasks: annoAssociation
Root: HKCR; Subkey: "UrobotsAnnotationTool\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\anno.exe"" ""%1"""; Tasks: annoAssociation

Root: HKCR; Subkey: "anno"; ValueType: string; ValueName: ""; ValueData: "URL:Anno Protocol"; Tasks: annoProtocol
Root: HKCR; Subkey: "anno"; ValueType: string; ValueName: "URL Protocol"; ValueData: ""; Tasks: annoProtocol
Root: HKCR; Subkey: "anno\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\anno.ico"; Tasks: annoProtocol
Root: HKCR; Subkey: "anno\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\anno.exe"" ""%1"""; Tasks: annoProtocol

[Files]
Source: "temp/*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs
Source: "{#MyAppIcon}"; DestDir: "{app}";
; data
Source: "..\..\anno\data\examples\*"; DestDir: "{app}\examples"; Flags: ignoreversion recursesubdirs
; Open CV
Source: "{#ProductReleaseFolder}opencv_*.dll"; DestDir: {app};
; External installations
Source: "vc_redist.x64.exe"; DestDir: {tmp}; DestName:"vs_redist_setup.exe"; Flags: deleteafterinstall

[Run]
Filename: {tmp}\vs_redist_setup.exe; Parameters: "/install /passive /norestart"; StatusMsg: Installing VC++ Redistributables... 

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\anno.exe"; IconFilename:"{app}/anno.ico"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\anno.exe"; Tasks: desktopicon; IconFilename:"{app}/anno.ico"

