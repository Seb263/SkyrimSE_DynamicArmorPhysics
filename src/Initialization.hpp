#pragma once

#include "DataHandler.hpp"
#include "Events.h"
#include "Hooks.hpp"
#include "SettingsIni.hpp"

#include "Core/ArmorMaterial.hpp"

#include "Utils/MiscUtils.hpp"
#include "Utils/TimeUtils.hpp"

#include "API/CIF-API.h"
#include "API/NGD-API.h"

namespace ModData
{
	class DataHandler
	{
	public:
		bool preLoaded = false;
		bool postLoaded = false;
		bool postLoadedAlternate = false;

		static DataHandler* GetSingleton()
		{
			static DataHandler singleton;
			return &singleton;
		}

		void PreLoadData()
		{
			if (preLoaded) return;
			preLoaded = true;

			TESdataHandler = RE::TESDataHandler::GetSingleton();

			ExtractGameAssets();
			LoadPluginsForms();

			ModCore::ArmorMaterial::MaterialMaintenance();
			Events::Hooks::InstallHooks();
			Events::ModEventSink::LoadEvents();
		}

		void PostLoadData()
		{
			if (postLoaded) return;
			postLoaded = true;

			if (!LoadCIFApi()) return;

			if (NGDecapitationsAPI::LoadAPI()) {
				NGD_API_Interface = NGDecapitationsAPI::g_API;
				const auto version = NGD_API_Interface->GetVersion();

				const auto major = (version >> 16) & 0xFF;
				const auto minor = (version >> 8) & 0xFF;
				const auto patch = version & 0xFF;

				logger::info("Next-Gen Decapitations API v{}.{}.{}.0 registered successfully.", major, minor, patch);
			}
		}

		void PostLoadDataAlternate()
		{
			if (postLoadedAlternate) return;
			postLoadedAlternate = true;

			TimeUtils::DoWhile(100ms, [](TimeUtils::CallResult result, std::chrono::nanoseconds) {
				if (TimeUtils::IsEnd(result)) return true;

				auto player = RE::PlayerCharacter::GetSingleton();
				if (player && player->Is3DLoaded() && player->GetParentCell() && player->GetParentCell()->IsAttached()) {
					GetSingleton()->PostLoadData();
					return false;
				}

				return true;
			}, true);
		}

	private:
		static inline void LoadPluginsForms()
		{
			logger::info("Loading Plugins Froms Data...");

			for (const auto& formInfo : pluginForms) {
				*formInfo.formPtr = TESdataHandler->LookupForm(formInfo.formID, formInfo.pluginName.data());
				if (!*formInfo.formPtr && !formInfo.optional) {
					REPORT_AND_FAIL("ERROR: Form \"{}\" not found in \"{}\".", formInfo.name, formInfo.pluginName);
				}
			}

			logger::info("Loading Plugins Froms Data: DONE");
		}

		static inline void ExtractGameAssets()
		{
			constexpr unsigned char PscBytes[] = {
				#include "DynamicArmorPhysics.psc.h"
			};

			constexpr unsigned char PexBytes[] = {
				#include "DynamicArmorPhysics.pex.h"
			};

			const std::string_view PscData{ reinterpret_cast<const char*>(PscBytes), sizeof(PscBytes) - 1 };
			const std::string_view PexData{ reinterpret_cast<const char*>(PexBytes), sizeof(PexBytes) - 1 };

			struct AssetEntry
			{
				std::string_view data;
				std::string_view dest;
				bool isSource;
			};

			const std::array<AssetEntry, 2> assets{{
				{ PscData, "Data/Source/Scripts/DynamicArmorPhysics.psc", true },
				{ PexData, "Data/Scripts/DynamicArmorPhysics.pex", false }
			}};

			for (const auto& asset : assets) {
				if (asset.isSource && !SettingsIni::bGeneral_ExtractScriptSources) {
					TRACE("ExtractGameAssets: Skipping source script '{}' (ExtractScriptSources disabled).", asset.dest);
					continue;
				}
				try {
					const std::size_t srcHash = std::hash<std::string_view>{}(asset.data);
					const std::filesystem::path destPath(asset.dest);
					if (std::filesystem::exists(destPath)) {
						std::ifstream existing(destPath, std::ios::binary);
						if (existing) {
							const std::string destData{
								std::istreambuf_iterator<char>{existing},
								std::istreambuf_iterator<char>{}
							};
							const std::size_t destHash = std::hash<std::string>{}(destData);
							if (srcHash == destHash) {
								TRACE("ExtractGameAssets: Asset '{}' is up-to-date, skipping.", asset.dest);
								continue;
							}
							if (!SettingsIni::bGeneral_OverwriteInvalidScripts) {
								TRACE("ExtractGameAssets: Asset '{}' differs but overwrite is disabled, skipping.", asset.dest);
								continue;
							}
							TRACE("ExtractGameAssets: Asset '{}' differs, replacing.", asset.dest);
						}
					}
					std::filesystem::create_directories(destPath.parent_path());

					std::ofstream out(destPath, std::ios::binary | std::ios::trunc);
					if (!out) {
						logger::error("ExtractGameAssets: Failed to open output stream for '{}'.", asset.dest);
						continue;
					}

					out.write(asset.data.data(), static_cast<std::streamsize>(asset.data.size()));
					if (!out) {
						std::filesystem::remove(destPath);
						logger::error("ExtractGameAssets: Failed to write '{}'.", asset.dest);
						continue;
					}
					TRACE("ExtractGameAssets: Asset '{}' extracted successfully.", asset.dest);
				} catch (const std::exception& e) {
					logger::error("ExtractGameAssets: Exception extracting '{}': {}", asset.dest, e.what());
				}
			}
		}

		static inline bool LoadCIFApi()
		{
			constexpr REL::Version kRequiredVersion{ 2, 0, 0, 0 };
			const auto dllVersion = MiscUtils::GetPluginVersion("CoreImpactFramework.dll");
			const bool versionOk = dllVersion != REL::Version{} && dllVersion >= kRequiredVersion;

			auto* apiInterface = versionOk ? static_cast<CIF_API::Interface*>(CIF_API::GetAPI()) : nullptr;

			if (!apiInterface) {
				logger::error("Core Impact Framework API not found or version insufficient.");

				const std::string title = fmt::format("{}: Missing Requirement", MOD_NAME);
				const std::string msg_box = fmt::format(
					"The Core Impact Framework version {} or higher is required to run {}.\n\n"
					"Would you like to close the game and open the download page?",
					kRequiredVersion.string("."), MOD_NAME);

				if (REX::W32::MessageBoxA(nullptr, msg_box.c_str(), title.c_str(), MB_ICONWARNING | MB_YESNO) == IDYES) {
					::ShellExecuteA(nullptr, "open", "https://www.seb263.fr/short-url/cif-v2", nullptr, nullptr, SW_SHOWNORMAL);
					REX::W32::TerminateProcess(REX::W32::GetCurrentProcess(), EXIT_FAILURE);
				}
				return false;
			}

			CIF_API_Interface = apiInterface;
			logger::info("Core Impact Framework API v{} registered successfully.", apiInterface->GetVersion().string("."));

			return true;
		}
	};
}
