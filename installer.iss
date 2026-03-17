#define MyAppName "OpenBMS"
#define MyAppVersion "0.1"
#define MyAppPublisher "OpenBMS"
#define MyAppURL "https://github.com/OpenBMS"
#define MyAppExeName "appOpenBMS.exe"

; Path to the deployed application folder (created by deploy.bat)
#define DeployDir "deploy"

[Setup]
AppId={{E8A2F3D1-7B4C-4E5A-9F6D-2C8B1A3E7F09}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir=.
OutputBaseFilename=OpenBMS_Setup
SetupIconFile=icon.ico
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayIcon={app}\{#MyAppExeName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#DeployDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Messages]
WelcomeLabel1=Welcome to OpenBMS Setup
WelcomeLabel2=This will install [name/ver] on your computer.%n%nOpenBMS is an open-source desktop application for monitoring and configuring JK-BMS battery management systems via Bluetooth Low Energy (BLE).%n%nIt is recommended that you close all other applications before continuing.

[Code]
function InitializeSetup(): Boolean;
begin
  Result := True;
end;
