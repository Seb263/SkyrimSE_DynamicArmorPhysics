#pragma once

#include "API/CIF-API.h"
#include "API/NGD-API.h"

namespace ModData
{
	constexpr std::string_view MOD_NAME = "Dynamic Armor Physics";

	inline auto lastLoadPoint = std::chrono::steady_clock::now();
	inline std::atomic<RE::FormID> previousCell = 0x0;

	struct PluginForm
	{
		std::string_view name;
		void** formPtr;
		uint32_t formID;
		std::string_view pluginName;
		bool optional = false;
	};

	// Properties storing game form references
	static inline const std::vector<PluginForm> pluginForms = {};

	inline RE::TESDataHandler* TESdataHandler;

	inline CIF_API::Interface* CIF_API_Interface = nullptr;
	inline NGDecapitationsAPI::NGDecapitationsAPI* NGD_API_Interface = nullptr;
}
