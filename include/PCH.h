#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMMNOSOUND
#define NOMINMAX

#include <unordered_set>
#include <shared_mutex>
#include <future>

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <shellapi.h>

#include <spdlog/sinks/basic_file_sink.h>

#include <SimpleIni.h>
#include <magic_enum.hpp>

#include "re-fmt.h"

#include "Utils/Debug.hpp"

#define DLLEXPORT __declspec(dllexport)

namespace logger = SKSE::log;

using namespace std::literals;

namespace stl
{
	using namespace SKSE::stl;

	template <class T>
	void write_thunk_call(std::uintptr_t a_src)
	{
		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(14);

		T::func = trampoline.write_call<5>(a_src, T::thunk);
	}

	template <class F, size_t offset, class T>
	void write_vfunc()
	{
		REL::Relocation<std::uintptr_t> vtbl{ F::VTABLE[offset] };

		if constexpr (requires { T::idx(); }) {
			T::func = vtbl.write_vfunc(T::idx(), T::thunk);
		} else {
			T::func = vtbl.write_vfunc(T::idx, T::thunk);
		}
	}

	template <class F, class T>
	void write_vfunc()
	{
		write_vfunc<F, 0, T>();
	}

	inline std::string as_string(std::string_view a_view)
	{
		return { a_view.data(), a_view.size() };
	}
}
