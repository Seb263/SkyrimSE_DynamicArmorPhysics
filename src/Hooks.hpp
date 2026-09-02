#pragma once

#include "DataHandler.hpp"
#include "SettingsIni.hpp"

#include "Core/Main.hpp"
#include "Core/ArmorMaterial.hpp"

#include "Utils/TimeUtils.hpp"

namespace Events
{
	using namespace ModData;
	using namespace ModCore;

	class Hooks
	{
	public:
		// Initialization of hooks and template functions
		static void InstallHooks()
		{
			SKSE::AllocTrampoline(1 << 8);

			REL::Relocation<std::uintptr_t> player_vt{ RE::PlayerCharacter::VTABLE[0] };
			_UpdatePlayer = player_vt.write_vfunc(0xAD, UpdatePlayerTemplate);
			logger::info("UpdatePlayer hooked at address: 0x{:X}", _UpdatePlayer.address());

			NotifyGraphHandler::InstallGraphNotifyHook();
			//ArmorMaterial::AMFContactListener::InstallHook();
		}

	private:

		struct NotifyGraphHandler
		{
		public:
			static bool InstallGraphNotifyHook()
			{
				REL::Relocation<uintptr_t> vtblChar{ RE::VTABLE_Character[3] };
				_origCharacter = vtblChar.write_vfunc(0x1, OnCharacter);

				REL::Relocation<uintptr_t> vtblPlayer{ RE::VTABLE_PlayerCharacter[3] };
				_origPlayer = vtblPlayer.write_vfunc(0x1, OnPlayer);

				return true;
			}

		private:
			static bool OnCharacter(RE::IAnimationGraphManagerHolder* a_this, const RE::BSFixedString& a_eventName)
			{
				HandleRagdollEvent(a_this, a_eventName);
				return _origCharacter(a_this, a_eventName);
			}
			static inline REL::Relocation<decltype(OnCharacter)> _origCharacter;

			static bool OnPlayer(RE::IAnimationGraphManagerHolder* a_this, const RE::BSFixedString& a_eventName)
			{
				HandleRagdollEvent(a_this, a_eventName);
				return _origPlayer(a_this, a_eventName);
			}
			static inline REL::Relocation<decltype(OnPlayer)> _origPlayer;

			static void HandleRagdollEvent(RE::IAnimationGraphManagerHolder* a_this, const RE::BSFixedString& a_eventName)
			{
				static const RE::BSFixedString ragdollTag{ "Ragdoll" };
				if (a_eventName != ragdollTag) return;

				auto* actor = skyrim_cast<RE::Actor*>(a_this);
				if (!actor) return;

				TRACE("\"{:08X}\" goes into Ragdoll State", actor->formID);

				Main::MaintainActor(actor);
			}
		};

		static void UpdatePlayerTemplate(RE::PlayerCharacter* a_this, float a_delta)
		{
			_UpdatePlayer(a_this, a_delta);
			if (!a_this) return;

			static RE::bhkWorld* currentWorld = nullptr;

			const auto* cell = a_this->GetParentCell();
			const auto currentCell = cell ? cell->formID : 0x0;
			if (currentCell != previousCell && a_this->Is3DLoaded()) {
				const bool reset = previousCell == 0x0;
				previousCell = currentCell;
				TRACE("Player moved to new cell: [{:08X}].", currentCell);

				if (cell) {
					if (auto* world = cell->GetbhkWorld(); world && world != currentWorld) {
						currentWorld = world;
						ArmorMaterial::RegisterWorld(world);
					}
				}

				TimeUtils::WaitAndCall(300ms, [reset](TimeUtils::CallResult result, std::chrono::nanoseconds) {
					if (result == TimeUtils::CallResult::kEndDone) {
						Main::MaintainLoadedCells(reset);
					}
					return true;
				}, false);
			}
		}
		static inline REL::Relocation<decltype(UpdatePlayerTemplate)*> _UpdatePlayer;
	};
};
