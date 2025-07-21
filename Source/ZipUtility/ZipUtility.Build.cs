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
            // Try to find Visual Studio 2022 first, then fall back to 2019
            string[] vsPaths = {
                @"E:\Program Files\Microsoft Visual Studio\2022\Community",
                @"C:\Program Files\Microsoft Visual Studio\2022\Community",
                @"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community"
            };

            foreach (string vsPath in vsPaths)
            {
                if (Directory.Exists(vsPath))
                {
                    Console.WriteLine("ZipUtility Info: Using VS path: " + vsPath);
                    return vsPath;
                }
            }

            // If none found, use the default 2022 path
            string defaultPath = @"E:\Program Files\Microsoft Visual Studio\2022\Community";
            Console.WriteLine("ZipUtility Info: Using default VS path: " + defaultPath);
            return defaultPath;
        }
    }

    private string ATLPath
    {
        get
        {
            // Trying to find ATL path similar to:
            // E:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.38.33130/atlmfc
            string atlPath = "";
            try
            {
                var vsDir = VsDirectory;
                if (!string.IsNullOrEmpty(vsDir))
                {
                    string msvcPath = Path.Combine(vsDir, @"VC\Tools\MSVC\");
                    if (Directory.Exists(msvcPath))
                    {
                        var directories = Directory.GetDirectories(msvcPath);

                        // Sort directories to get the latest version
                        Array.Sort(directories);
                        Array.Reverse(directories);

                        foreach (string msvcVersionDir in directories)
                        {
                            string testAtlPath = Path.Combine(msvcVersionDir, "atlmfc");
                            if (Directory.Exists(testAtlPath))
                            {
                                atlPath = testAtlPath;
                                break;
                            }
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
                Console.WriteLine("ZipUtility Warning: Couldn't find an ATLPath, using empty path");
                atlPath = "";
            }
            else
            {
                Console.WriteLine("ZipUtility Info: Using ATL path " + atlPath);
            }
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
				// ... add other private include paths required here ...
			}
            );

        // Add ATL include path only if it exists
        string atlPath = ATLPath;
        if (!string.IsNullOrEmpty(atlPath) && Directory.Exists(atlPath))
        {
            PrivateIncludePaths.Add(Path.Combine(atlPath, "include"));
        }


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

        // Add preprocessor definitions to help with Windows API compatibility
        PublicDefinitions.AddRange(new string[] {
            "WIN32_LEAN_AND_MEAN=1",
            "NOMINMAX=1",
            "_WIN32_WINNT=0x0601"  // Windows 7 or later
        });

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
