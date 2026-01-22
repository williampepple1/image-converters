; Inno Setup Script for Image Converter

[Setup]
AppName=Image Converter
AppVersion=1.0
AppPublisher=Image Converter
DefaultDirName={autopf}\ImageConverter
DefaultGroupName=Image Converter
OutputDir=.
OutputBaseFilename=setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Files]
Source: "dist\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs

[Icons]
Name: "{group}\Image Converter"; Filename: "{app}\image-converters.exe"
Name: "{group}\Uninstall Image Converter"; Filename: "{uninstallexe}"
Name: "{autodesktop}\Image Converter"; Filename: "{app}\image-converters.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional icons:"

[Run]
Filename: "{app}\image-converters.exe"; Description: "Launch Image Converter"; Flags: nowait postinstall skipifsilent
