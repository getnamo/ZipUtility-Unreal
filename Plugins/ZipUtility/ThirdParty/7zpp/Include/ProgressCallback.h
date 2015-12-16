#pragma once

namespace SevenZip
{
	class ProgressCallback
	{
	public:
		virtual ~ProgressCallback() {}

		/*
		Called Whenever progress has updated with a float 0-100.0
		*/
		virtual void OnProgress(float progress) {}

		/*
		Called When progress has reached 100%
		*/
		virtual void OnDone() {}
	};
}
