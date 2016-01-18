# ZipUtility Plugin
Event driven, blueprint accessible flexible 7zip compression, archiver, and file manipulation plugin for Unreal Engine 4. Built on [7zip-cpp](https://github.com/getnamo/7zip-cpp) modernization of the [SevenZip++](http://bitbucket.org/cmcnab/sevenzip/wiki/Home) C++ wrapper for accessing the 7-zip COM-like API in 7z.dll and 7za.dll.

Supports the following compression algorithms:
7Zip, GZip, BZip2, RAR, TAR, ISO, CAB, LZMA, LZMA86.


Plugin works in Windows only.

[Main Forum Thread](https://forums.unrealengine.com/showthread.php?95022-Plugin-ZipUtility-(7zip))


## Quick Install & Setup ##

 1.	[Download](https://github.com/getnamo/ZipUtility-ue4/releases)
 2.	Create new or choose project.
 3.	Browse to your project folder (typically found at Documents/Unreal Project/{Your Project Root})
 4.	Copy *Plugins* folder into your Project root.
 5.	Restart the Editor and open your project again. Plugin is now ready to use.


### Blueprint Access

Right click anywhere in a desired blueprint to access the plugin Blueprint Function Library methods. The plugin is completely multi-threaded and will not block your game thread, fire and forget.

![Right Click](http://i.imgur.com/ERYn5sM.png)

*Optional but highly recommended:* Add ZipUtilityInterface to your blueprint if you wish to be notified of the progress, e.g. when your archive has finished unzipping or if you wish to display a progress bar.

![Add Interface](http://i.imgur.com/9NrpOfm.png)

After you've added the interface and hit Compile on your blueprint you'll have access to the Progress and List events

![Events](http://i.imgur.com/fQFtkgA.png)

They're explained in further detail below.

##Zipping and Compressing Files

To Zip up a folder or file, right click your event graph and add the *Zip* function.

Specify a path to a folder or file as a target.

Leave the Compression format to the default SevenZip or specify a format of choice, the plugin automatically appends the default extension based on the compression format. Note that not all compression formats work for compression (e.g. RAR is extract only).

![Zip Function Call](http://i.imgur.com/pd5l0rx.png)

If you wish to receive progress updates, pass a reference to self and ensure you have *ZipUtilityInterface* added to your blueprint. Then you can use any of the three main callbacks shown in the image. To show a progress bar, use the *OnProgress* event. Callbacks will be received on your game thread.

##Unzipping and Extracting Files


To Unzip up a file, right click your event graph and add the *Unzip* function.

Specify the full path to a suitable archive file.

The plugin automatically detects the compression format used in the archive, but you can alternatively specify a specific format using the *UnzipWithFormat* method.

![Unzip Function Call](http://i.imgur.com/sDEzOKJ.png)

If you wish to receive progress updates, pass a reference to self and ensure you have *ZipUtilityInterface* added to your blueprint. Then you can use any of the three main callbacks shown in the image. To show a progress bar, use the *OnProgress* event. Callbacks will be received on your game thread.

##Listing Contents in an Archive

To list files in your archive, right click your event graph and add the *ListFilesInArchive* function.

Specify the full path to a suitable archive file. This function requires the use of the ZipUtilityInterface callback *OnFileFound*, so ensure you have *ZipUtilityInterface* added to your blueprint. 

![List Files Function Call](http://i.imgur.com/uqkI2Gn.png)

The *OnFileFound* event gets called for every file in the archive with its path and size given in bytes. This function does not extract the contents, but instead allows you to inspect files before committing to extracting their contents.

##Convenience File Functions

###Move/Rename a File

Specify full path for the file you wish to move and it's destination

![Move File](http://i.imgur.com/aPu6gdJ.png)

To rename it, simply change the destination name

![Rename Folder](http://i.imgur.com/RICA41e.png)


###Create/Make Directory

![Make Directory](http://i.imgur.com/8ocCOPF.png)


##License

MIT for ZipUtility and 7z-cpp

LGPL for 7za.dll, LGPL + Unrar for 7z.dll

See license file for details.

##Todo
-Add C++ documentation/gifs
