#pragma once

#include "BaseWindow.hpp"

namespace PictureBrowser
{

#define ClassName(x) L#x

	class Widget : public BaseWindow
	{
	public:
		Widget() = default;

		Widget(
			DWORD extraStyle,
			const wchar_t* className,
			const wchar_t* windowName,
			DWORD style,
			int x,
			int y,
			int w,
			int h,
			BaseWindow* parent,
			HMENU menu,
			HINSTANCE instance,
			void* data);

		virtual ~Widget() = default;
		
		void Intercept(const BaseWindow* window);
		void Listen();

		virtual void HandleMessage(UINT, WPARAM, LPARAM);

	protected:
		BaseWindow* _parent = nullptr;
	};
};