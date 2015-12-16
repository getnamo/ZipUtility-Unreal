#pragma once

namespace SevenZip
{
	class ListCallback
	{
	public:
		virtual ~ListCallback() {}

		/*
		Called for each file found in the archive. Size in bytes.
		*/
		virtual void OnFileFound(WCHAR* path, int size) {}
	};
}
