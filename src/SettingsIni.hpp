#pragma once

#include "DataHandler.hpp"

namespace SettingsIni
{
	// Initialization
	inline int iGeneral_VerboseMode = 1;
	inline bool bGeneral_ShouldIgnoreMaintenanceChecks = false;
	inline bool bGeneral_OverwriteInvalidScripts = true;
	inline bool bGeneral_ExtractScriptSources = false;

	// Armor Weight
	inline bool bArmorWeight_Status = true;
	inline float fArmorWeight_NudeMult = 1.0f;
	inline float fArmorWeight_ClothMult = 1.1f;
	inline float fArmorWeight_LightMult = 1.5f;
	inline float fArmorWeight_HeavyMult = 2.0f;
	inline float fArmorWeight_MassLimit = 80.0f;

	// Armor Material
	inline bool bArmorMaterial_Status = true;
	inline std::string sArmorMaterial_NudeMaterialID = "None";
	inline std::string sArmorMaterial_ClothMaterialID = "Cloth";
	inline std::string sArmorMaterial_LightMaterialID = "ArmorLight";
	inline std::string sArmorMaterial_HeavyMaterialID = "ArmorHeavy";
	inline float fArmorMaterial_EffectiveRadius = 1280.0f;

	class SettingsManager
	{
	public:
		static SettingsManager& GetSingleton()
		{
			static SettingsManager instance;
			return instance;
		}

		SettingsManager()
		{
			bindings = {
				// General
				{ "General", "iVerboseMode", &iGeneral_VerboseMode },
				{ "General", "bShouldIgnoreMaintenanceChecks", &bGeneral_ShouldIgnoreMaintenanceChecks },
				{ "General", "bOverwriteInvalidScripts", &bGeneral_OverwriteInvalidScripts },
				{ "General", "bExtractScriptSources", &bGeneral_ExtractScriptSources },

				// Armor Weight
				{ "ArmorWeight", "bStatus", &bArmorWeight_Status },
				{ "ArmorWeight", "fNudeMult", &fArmorWeight_NudeMult },
				{ "ArmorWeight", "fClothMult", &fArmorWeight_ClothMult },
				{ "ArmorWeight", "fLightMult", &fArmorWeight_LightMult },
				{ "ArmorWeight", "fHeavyMult", &fArmorWeight_HeavyMult },
				{ "ArmorWeight", "fMassLimit", &fArmorWeight_MassLimit },

				// Armor Material
				{ "ArmorMaterial", "bStatus", &bArmorMaterial_Status },
				{ "ArmorMaterial", "sNudeMaterialID", &sArmorMaterial_NudeMaterialID },
				{ "ArmorMaterial", "sClothMaterialID", &sArmorMaterial_ClothMaterialID },
				{ "ArmorMaterial", "sLightMaterialID", &sArmorMaterial_LightMaterialID },
				{ "ArmorMaterial", "sHeavyMaterialID", &sArmorMaterial_HeavyMaterialID },
				{ "ArmorMaterial", "fEffectiveRadius", &fArmorMaterial_EffectiveRadius }
			};

			for (auto& bind : bindings) {
				std::visit([&](auto* ptr) {
					bind.defaultValue = *ptr;
				}, bind.var);
			}
		}

		bool ReadSettings()
		{
			std::wstring   wpath_str(path.begin(), path.end());
			const wchar_t* wpath = wpath_str.c_str();

			bool readStatus = false;

			logger::info("Trying to read INI file at path: {}", path);

			if (std::filesystem::exists(path)) {
				CSimpleIniA ini;
				ini.SetUnicode();

				if (ini.LoadFile(wpath) >= 0) {
					for (const auto& bind : bindings) {
						std::visit([&](auto* ptr) {
							using T = std::decay_t<decltype(*ptr)>;
							if constexpr (std::is_same_v<T, bool>) {
								*ptr = ini.GetBoolValue(bind.section, bind.key, *ptr);
							} else if constexpr (std::is_same_v<T, int>) {
								*ptr = static_cast<int>(ini.GetLongValue(bind.section, bind.key, *ptr));
							} else if constexpr (std::is_same_v<T, float>) {
								*ptr = static_cast<float>(ini.GetDoubleValue(bind.section, bind.key, *ptr));
							} else if constexpr (std::is_same_v<T, std::string>) {
								*ptr = ini.GetValue(bind.section, bind.key, ptr->c_str());
							}
						}, bind.var);
					}
					readStatus = true;
				} else {
					logger::error("Failed to load INI file at {}", path);
				}
			} else {
				logger::warn("INI file does not exist at {}", path);
			}

			// Clamping logic
			iGeneral_VerboseMode = std::clamp(iGeneral_VerboseMode, 0, 3);

			fArmorMaterial_EffectiveRadius = std::clamp(fArmorMaterial_EffectiveRadius, 256.0f, 4096.0f);

			// External data
			[&]() {
				using namespace ModData;
				debugVerboseMode = iGeneral_VerboseMode;
			}();

			return readStatus;
		}

		std::optional<std::variant<bool, int, float, std::string>> GetValueVariant(const std::string& key_section)
		{
			auto sep = key_section.rfind(':');
			if (sep == std::string::npos) {
				logger::error("GetValueVariant: Invalid key_section format: '{}'", key_section);
				return std::nullopt;
			}
			std::string section = key_section.substr(0, sep);
			std::string key     = key_section.substr(sep + 1);
			for (const auto& bind : bindings) {
				if (key == bind.key && section == bind.section) {
					if (auto v = std::get_if<bool*>        (&bind.var)) return **v;
					if (auto v = std::get_if<int*>         (&bind.var)) return **v;
					if (auto v = std::get_if<float*>       (&bind.var)) return **v;
					if (auto v = std::get_if<std::string*> (&bind.var)) return **v;
				}
			}
			return std::nullopt;
		}

		template <typename T>
		T GetValue(const std::string& key_section, const T& defaultValue = T{})
		{
			auto opt = GetValueVariant(key_section);
			if (!opt) {
				logger::error("GetValue: No binding found for '{}'", key_section);
				return defaultValue;
			}

			return std::visit([&](auto&& val) -> T {
				using V = std::decay_t<decltype(val)>;
				if constexpr (std::is_same_v<T, std::string>) {
					if constexpr (std::is_same_v<V, std::string>) return val;
				} else if constexpr (std::is_same_v<T, bool>) {
					if constexpr (std::is_same_v<V, bool>)  return val;
					if constexpr (std::is_same_v<V, int>)   return val != 0;
					if constexpr (std::is_same_v<V, float>) return val != 0.0f;
				} else if constexpr (std::is_same_v<T, int>) {
					if constexpr (std::is_same_v<V, int>)   return val;
					if constexpr (std::is_same_v<V, float>) return static_cast<int>(val);
					if constexpr (std::is_same_v<V, bool>)  return val ? 1 : 0;
				} else if constexpr (std::is_same_v<T, float>) {
					if constexpr (std::is_same_v<V, float>) return val;
					if constexpr (std::is_same_v<V, int>)   return static_cast<float>(val);
					if constexpr (std::is_same_v<V, bool>)  return val ? 1.0f : 0.0f;
				}
				logger::error("GetValue: Type mismatch for '{}'", key_section);
				return defaultValue;
			}, *opt);
		}

		std::optional<std::variant<bool, int, float, std::string>> GetDefaultValueVariant(const std::string& key_section)
		{
			auto sep = key_section.rfind(':');
			if (sep == std::string::npos) {
				logger::error("GetDefaultValueVariant: Invalid key_section format: '{}'", key_section);
				return std::nullopt;
			}
			std::string section = key_section.substr(0, sep);
			std::string key = key_section.substr(sep + 1);

			for (const auto& bind : bindings) {
				if (key == bind.key && section == bind.section) {
					return bind.defaultValue;
				}
			}
			return std::nullopt;
		}

		template <typename T>
		T GetDefaultValue(const std::string& key_section, const T& fallback = T{})
		{
			auto opt = GetDefaultValueVariant(key_section);
			if (!opt) {
				logger::error("GetDefaultValue: No binding found for '{}'", key_section);
				return fallback;
			}

			return std::visit([&](auto&& val) -> T {
				using V = std::decay_t<decltype(val)>;
				if constexpr (std::is_same_v<T, bool>) {
					if constexpr (std::is_same_v<V, bool>)  return val;
					if constexpr (std::is_same_v<V, int>)   return val != 0;
					if constexpr (std::is_same_v<V, float>) return val != 0.0f;
				} else if constexpr (std::is_same_v<T, int>) {
					if constexpr (std::is_same_v<V, int>)   return val;
					if constexpr (std::is_same_v<V, float>) return static_cast<int>(val);
					if constexpr (std::is_same_v<V, bool>)  return val ? 1 : 0;
				} else if constexpr (std::is_same_v<T, float>) {
					if constexpr (std::is_same_v<V, float>) return val;
					if constexpr (std::is_same_v<V, int>)   return static_cast<float>(val);
					if constexpr (std::is_same_v<V, bool>)  return val ? 1.0f : 0.0f;
				} else if constexpr (std::is_same_v<T, std::string>) {
					if constexpr (std::is_same_v<V, std::string>) return val;
				}
				logger::error("GetDefaultValue: Type mismatch for '{}'", key_section);
				return fallback;
			}, *opt);
		}

		template <typename T>
		bool SetValue(const std::string& key_section, const T& value)
		{
			auto sep = key_section.rfind(':');
			if (sep == std::string::npos) {
				logger::error("SetValue: Invalid key_section format: '{}'", key_section);
				return false;
			}

			std::string section = key_section.substr(0, sep);
			std::string key = key_section.substr(sep + 1);

			if (section.empty() || key.empty()) {
				logger::error("SetValue: Empty section or key in '{}'", key_section);
				return false;
			}

			for (auto& bind : bindings) {
				if (section == bind.section && key == bind.key) {
					bool matched = std::visit([&](auto* ptr) -> bool {
						using PtrType = std::decay_t<decltype(*ptr)>;
						if constexpr (std::is_same_v<PtrType, T>) {
							*ptr = value;
							return true;
						}
						return false;
					}, bind.var);

					if (!matched) {
						logger::error("SetValue: Type mismatch for '{}:{}'", section, key);
						return false;
					}

					CSimpleIniA ini;
					ini.SetUnicode();
					if (std::filesystem::exists(path)) ini.LoadFile(path.c_str());

					if constexpr (std::is_same_v<T, bool>) {
						ini.SetBoolValue(section.c_str(), key.c_str(), value);
					} else if constexpr (std::is_same_v<T, int>) {
						ini.SetLongValue(section.c_str(), key.c_str(), value);
					} else if constexpr (std::is_same_v<T, float>) {
						ini.SetDoubleValue(section.c_str(), key.c_str(), value);
					} else if constexpr (std::is_same_v<T, std::string>) {
						ini.SetValue(section.c_str(), key.c_str(), value.c_str());
					} else {
						return false;
					}

					if (ini.SaveFile(path.c_str()) < 0) {
						logger::error("SetValue: Failed to save INI file at '{}'", path);
						return false;
					}

					return true;
				}
			}

			logger::error("SetValue: No binding found for '{}:{}'", section, key);
			return false;
		}

	private:
		inline static std::string path = "Data/SKSE/Plugins/DynamicArmorPhysics.ini";
		inline static std::string prefix = "DAP";

		using IniValue = std::variant<bool*, int*, float*, std::string*>;

		struct IniBinding
		{
			const char* section;
			const char* key;
			IniValue var;
			bool syncGlobal = false;
			std::variant<bool, int, float, std::string> defaultValue;
		};

		std::vector<IniBinding> bindings;
	};
}
