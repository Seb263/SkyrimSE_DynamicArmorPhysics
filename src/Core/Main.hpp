#pragma once

#include "DataHandler.hpp"
#include "SettingsIni.hpp"

#include "Core/Structure.h"
#include "Core/ArmorMaterial.hpp"
#include "Core/ArmorWeight.hpp"

#include "Utils/ModUtils.hpp"
#include "Utils/TimeUtils.hpp"

#include "API/CIF-API.h"

namespace ModCore
{
	using namespace CoreStructure;

	class Main
	{

	public:

		static void MaintainLoadedCells(const bool reset = false)
		{
			using namespace ModData;

			TRACE("Process Cells Maintenance Task.");

			{
				std::lock_guard<std::mutex> lock(attachedCellMutex);
				if (reset) attachedCells.clear();

				for (auto it = attachedCells.begin(); it != attachedCells.end();) {
					auto* cell = *it;

					if (!cell || !cell->IsAttached()) {
						TRACE("    -> Unloaded cell: [{:08X}].", cell ? cell->formID : 0);
						it = attachedCells.erase(it);
					} else {
						++it;
					}
				}
			}

			const auto processCell = [](RE::TESObjectCELL* cell) {
				if (!cell || !cell->IsAttached()) return;

				{
					std::lock_guard<std::mutex> lock(attachedCellMutex);
					if (!attachedCells.insert(cell).second) return;
				}

				TRACE("    -> Maintenance on cell [{:08X}]", cell->formID);

				cell->ForEachReference([](RE::TESObjectREFR* ref) -> RE::BSContainer::ForEachResult {
					if (!ref || ref->IsPlayerRef()) return RE::BSContainer::ForEachResult::kContinue;
	
					auto* actor = ref->As<RE::Actor>();
					if (!actor || !actor->Is3DLoaded() || (!actor->IsDead() && !actor->IsInRagdollState())) {
						return RE::BSContainer::ForEachResult::kContinue;
					}

					TRACE("Processing maintenance on actor [{:08X}]", actor->formID);
                
					MaintainActor(actor, false, true);

					return RE::BSContainer::ForEachResult::kContinue;
				});
			};

			ModUtils::ProcessGridCells(processCell);

			TRACE("Ended Cells Maintenance Task.");
		}

		static void MaintainActor(RE::Actor* a_actor, const bool armorMaterial = false, const bool armorWeight = false)
		{
			if (!a_actor || !CanProceed(a_actor->formID)) return;

			TimeUtils::WaitUntilRagdollReady(a_actor, [=](RE::TESObjectREFR* objectRef, const bool result) {
				if (!result || !objectRef) return;
				
				RE::Actor* target = objectRef->As<RE::Actor>();
				if (!target) return;

				if (armorMaterial) ArmorMaterial::OnRagdollContact(target, true);
				if (armorWeight) ArmorWeight::Process(target);
			}, 500ms);
		}

		static void ResetExecutions()
		{
			lastExecutions.clear();
		}

	private:

		static bool CanProceed(RE::FormID a_formID)
		{
			const auto now = Clock::now();

			std::scoped_lock lock(executionMutex);

			auto& lastExecution = lastExecutions[a_formID];
			if (now - lastExecution < std::chrono::milliseconds(100)) {
				return false;
			}

			lastExecution = now;
			return true;
		}

		using Clock = std::chrono::steady_clock;
		static inline std::unordered_map<RE::FormID, Clock::time_point> lastExecutions;
		static inline std::mutex executionMutex;

		static inline std::mutex attachedCellMutex;
		static inline std::unordered_set<RE::TESObjectCELL*> attachedCells;

	};
}
