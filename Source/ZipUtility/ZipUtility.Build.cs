// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.IO;
using Microsoft.Win32;

public class ZipUtility : ModuleRules
{
    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/")); }
    }

    private string SevenZppPath
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "7zpp")); }
    }

    private string VsDirectory
    {
        get
        {
            //Registry variant of fetch doesn't work since 2017
            // If failed - using the most common install path
            string vsDefaultBasePath = @"C:\Program Files (x86)\Microsoft Visual Studio\2019";
            string vsVersion = "Community"; //Directory.GetDirectories(vsDefaultBasePath)[0];
            string vsPath = Path.Combine(vsDefaultBasePath, vsVersion);
            Console.WriteLine("ZipUtility Info: Using default VS path: " + vsPath);
            return vsPath;
        }
    }

    private string ATLPath
    {
        get
        {
            // Trying to find ATL path similar to:
            // C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.14.26428/atlmfc
            string atlPath = "";
            try
            {
                var vsDir = VsDirectory;
                if (!string.IsNullOrEmpty(vsDir))
                {

                    string msvcPath = Path.Combine(vsDir, @"VC\Tools\MSVC\");
                    var directories = Directory.GetDirectories(msvcPath);

                    //Directory.GetDirectories(msvcPath)
                    
                    foreach (string msvcVersion in directories)
                    {
                        atlPath = Path.Combine(msvcPath, msvcVersion, "atlmfc");
                        if (Directory.Exists(atlPath))
                        {
                            break;
                        }
                    }
                    
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("ZipUtility Error: can't find VS path: " + ex.ToString());
            }

            if (!Directory.Exists(atlPath))
            {
                Console.WriteLine("ZipUtility Error: Couldn't find an ATLPath, fix it in ZipUtility.Build.cs");
            }
            Console.WriteLine("ZipUtility Info: Using ATL path " + atlPath);
            return atlPath;
        }
    }

    public ZipUtility(ReadOnlyTargetRules Target) : base(Target)
    {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivatePCHHeaderFile = "Private/ZipUtilityPrivatePCH.h";


		PublicIncludePaths.AddRange(
            new string[] {
				Path.Combine(ModuleDirectory, "Public"),
				// ... add public include paths required here ...
			}
            );

        PrivateIncludePaths.AddRange(
            new string[] {
				Path.Combine(ModuleDirectory, "Private"),
                Path.Combine(SevenZppPath, "Include"),
				Path.Combine(ATLPath, "include"),
				// ... add other private include paths required here ...
			}
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "WindowsFileUtility"
				// ... add other public dependencies that you statically link with here ...
			}
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Projects"
				// ... add private dependencies that you statically link with here ...	
			}
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );

        LoadLib(Target);
    }

    public bool LoadLib(ReadOnlyTargetRules Target)
    {
        bool isLibrarySupported = false;

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            isLibrarySupported = true;

            string PlatformSubPath = "Win64";
            string LibrariesPath = Path.Combine(SevenZppPath, "Lib");
            string DLLPath = Path.Combine(SevenZppPath, "dll");

            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, PlatformSubPath, "atls.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, PlatformSubPath, "7zpp_u.lib"));
            //PublicLibraryPaths.Add(Path.Combine(LibrariesPath, PlatformSubPath));

            PublicDelayLoadDLLs.Add("7z.dll");
            RuntimeDependencies.Add(Path.Combine(DLLPath, PlatformSubPath, "7z.dll"));
        }

        if (isLibrarySupported)
        {
            // Include path
            //PublicIncludePaths.Add(Path.Combine(SevenZppPath, "Include"));
        }

        return isLibrarySupported;
    }
}
