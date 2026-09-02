#pragma once

class MiscUtils
{
	public:

	template <typename T = RE::TESObjectREFR, typename HandleT>
	static T* ResolveHandle(const HandleT& handle)
	{
		auto ptr = handle ? handle.get() : nullptr;
		if (!ptr) return nullptr;

		return ptr->As<T>();
	}

	static bool IsFormIDValid(const RE::FormID formID)
	{
		return (formID > 0x0 && formID < 0xFFFFFFFF);
	}

	template <typename T = RE::TESObjectREFR>
	static T* GetValidReference(RE::FormID formID, const bool extraChecks = false)
	{
		if (!MiscUtils::IsFormIDValid(formID)) return nullptr;
		return GetValidReference<T>(RE::TESForm::LookupByID<RE::TESObjectREFR>(formID), extraChecks);
	}

	template <typename T = RE::TESObjectREFR>
	static T* GetValidReference(RE::TESObjectREFR* ref, const bool extraChecks = false)
	{
		using namespace ModData;

		if (!ref || !ref->As<T>() || !MiscUtils::IsFormIDValid(ref->formID) || ref->IsDeleted())
			return nullptr;

		if (extraChecks) {
			if (ref->IsDisabled() || ref->IsMarkedForDeletion())
				return nullptr;
		}

		if constexpr (std::is_same_v<T, RE::Actor>) {
			auto* refActor = ref->As<RE::Actor>();
			if (!refActor || !ref->Is(RE::FormType::ActorCharacter))
				return nullptr;

			if (extraChecks && (refActor->GetActorRuntimeData().criticalStage != RE::ACTOR_CRITICAL_STAGE::kNone))
				return nullptr;
		}

		return ref->As<T>();
	}

	static REL::Version GetPluginVersion(const char* a_moduleName)
	{
		const auto handle = GetModuleHandleA(a_moduleName);
		if (!handle) return REL::Version{};

		char path[MAX_PATH]{};
		if (!GetModuleFileNameA(handle, path, MAX_PATH)) return REL::Version{};

		DWORD dummy = 0;
		const DWORD size = GetFileVersionInfoSizeA(path, &dummy);
		if (size == 0) return REL::Version{};

		std::vector<std::byte> data(size);
		if (!GetFileVersionInfoA(path, 0, size, data.data())) return REL::Version{};

		VS_FIXEDFILEINFO* fileInfo = nullptr;
		UINT fileInfoLen = 0;
		if (!VerQueryValueA(data.data(), "\\", reinterpret_cast<LPVOID*>(&fileInfo), &fileInfoLen)) return REL::Version{};
		if (!fileInfo) return REL::Version{};

		return REL::Version{
			HIWORD(fileInfo->dwFileVersionMS),
			LOWORD(fileInfo->dwFileVersionMS),
			HIWORD(fileInfo->dwFileVersionLS),
			LOWORD(fileInfo->dwFileVersionLS)
		};
	}

	static float GetRandomNumber(float min = 0.0f, float max = 1.0f)
	{
		static std::mt19937 generator(std::random_device{}());
		std::uniform_real_distribution<float> distribution(min, max);
		return distribution(generator);
	}
};
