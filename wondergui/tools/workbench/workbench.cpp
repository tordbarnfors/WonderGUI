/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/

#include "workbench.h"

#include <algorithm>
#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace wg;

namespace workbench
{
	static const char c_resourceDir[] = "resources/";

	//____ _registry() ________________________________________________________
	//
	// Tests register themselves while static objects are constructed, so the
	// list is made on first use rather than being a static object itself.

	static std::vector<Test>& _registry()
	{
		static std::vector<Test> s_tests;
		return s_tests;
	}

	static bool s_bSorted = false;

	//____ registerTest() _____________________________________________________

	bool registerTest(const char* pGroup, const char* pName, TestFunc func)
	{
		_registry().push_back({ pGroup, pName, func });
		s_bSorted = false;
		return true;
	}

	//____ tests() ____________________________________________________________

	const std::vector<Test>& tests()
	{
		auto& tests = _registry();

		if (!s_bSorted)
		{
			std::stable_sort(tests.begin(), tests.end(), [](const Test& a, const Test& b)
			{
				return a.group != b.group ? a.group < b.group : a.name < b.name;
			});
			s_bSorted = true;
		}

		return tests;
	}

	//____ Context::~Context() ________________________________________________

	Context::~Context()
	{
		for (auto& func : m_exitFuncs)
			func();

		for (auto id : m_routes)
			Base::msgRouter()->deleteRoute(id);
	}

	//____ Context::loadBlob() ________________________________________________

	Blob_p Context::loadBlob(const std::string& path)
	{
		std::string fullPath = c_resourceDir + path;

		FILE* fp = fopen(fullPath.c_str(), "rb");
		if (!fp)
		{
			printf("Workbench: Could not open '%s'\n", fullPath.c_str());
			return nullptr;
		}

		fseek(fp, 0, SEEK_END);
		size_t size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		Blob_p pBlob = Blob::create((int)size);
		size_t nRead = fread(pBlob->data(), 1, size, fp);
		fclose(fp);

		if (nRead < size)
		{
			printf("Workbench: Could not read '%s'\n", fullPath.c_str());
			return nullptr;
		}

		return pBlob;
	}

	//____ Context::loadSurface() _____________________________________________
	//
	// Loads any image stb_image can read. Size and pixel format are taken from
	// the image, the rest of the blueprint is used as given.

	Surface_p Context::loadSurface(const std::string& path, const Surface::Blueprint& blueprint)
	{
		std::string fullPath = c_resourceDir + path;

		int width, height, channels;
		stbi_uc* pPixels = stbi_load(fullPath.c_str(), &width, &height, &channels, 4);
		if (!pPixels)
		{
			printf("Workbench: Could not load '%s'\n", fullPath.c_str());
			return nullptr;
		}

		// stb_image gives us R, G, B, A bytes.

		PixelDescription desc;
		desc.type = PixelType::Chunky;
		desc.bits = 32;
		desc.bigEndian = false;
		desc.R_mask = 0x000000FF;
		desc.G_mask = 0x0000FF00;
		desc.B_mask = 0x00FF0000;
		desc.A_mask = 0xFF000000;

		Surface::Blueprint bp = blueprint;
		bp.size = { width, height };
		bp.format = (channels == 4 || channels == 2) ? PixelFormat::ARGB_8 : PixelFormat::XRGB_8;

		auto pSurface = Base::defaultSurfaceFactory()->createSurface(bp, pPixels, desc, width * 4);

		stbi_image_free(pPixels);
		return pSurface;
	}

	//____ Context::addRoute() ________________________________________________

	RouteId Context::addRoute(Object* pSource, MsgType type, std::function<void(Msg* pMsg)> func)
	{
		RouteId id = Base::msgRouter()->addRoute(pSource, type, func);
		m_routes.push_back(id);
		return id;
	}

	RouteId Context::addRoute(MsgType type, std::function<void(Msg* pMsg)> func)
	{
		RouteId id = Base::msgRouter()->addRoute(type, func);
		m_routes.push_back(id);
		return id;
	}

	//____ Context::onUpdate() ________________________________________________

	void Context::onUpdate(std::function<void(int64_t microsec)> func)
	{
		m_updateFuncs.push_back(func);
	}

	//____ Context::onExit() __________________________________________________

	void Context::onExit(std::function<void()> func)
	{
		m_exitFuncs.push_back(func);
	}

	//____ Context::_update() _________________________________________________

	void Context::_update(int64_t microsec)
	{
		for (auto& func : m_updateFuncs)
			func(microsec);
	}
}
