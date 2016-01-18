// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class ZipUtility : ModuleRules
{
    private string ModulePath
    {
        get { return Path.GetDirectoryName(RulesCompiler.GetModuleFilename(this.GetType().Name)); }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
    }
    private string SevenZppPath
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "7zpp")); }
    }

    public ZipUtility(TargetInfo Target)
	{

        PublicIncludePaths.AddRange(
			new string[] {
				"ZipUtility/Public"
				// ... add public include paths required here ...
			}
			);
				
		PrivateIncludePaths.AddRange(
			new string[] {
				"ZipUtility/Private",
                Path.Combine(SevenZppPath, "Include"),
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
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
    public bool LoadLib(TargetInfo Target)
    {
        bool isLibrarySupported = false;

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            isLibrarySupported = true;

            string PlatformSubPath = (Target.Platform == UnrealTargetPlatform.Win64) ? "Win64" : "Win32";
            string LibrariesPath = Path.Combine(SevenZppPath, "Lib");
            
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, PlatformSubPath, "7zpp_u.lib"));
        }

        if (isLibrarySupported)
        {
            // Include path
            //PublicIncludePaths.Add(Path.Combine(SevenZppPath, "Include"));
        }

        return isLibrarySupported;
    }
}
