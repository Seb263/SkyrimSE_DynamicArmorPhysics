#pragma once

#include "DataHandler.hpp"
#include "SettingsIni.hpp"

#include "Core/Structure.h"

#include "Utils/ModUtils.hpp"
#include "Utils/NiUtils.hpp"

#include "API/CIF-API.h"
#include "API/NGD-API.h"

namespace ModCore
{
	using namespace ModData;
	using namespace CoreStructure;

	class ArmorWeight
	{
	public:

		static void Process(RE::Actor* actor)
		{
			if (!SettingsIni::bArmorWeight_Status || !ModData::CIF_API_Interface) return;
			if (!actor || !NiUtils::IsReferenceRagdollReady(actor)) return;

			if (NGD_API_Interface) {
				if (NGD_API_Interface->IsHead(actor) || actor->IsActivationBlocked()) return;
			}

			const auto a_bipedBones = ModData::CIF_API_Interface->GetBipedBonesMap(actor, false);
			const auto& biped = actor->GetBiped(false);
			if (!biped) {
				TRACE("ArmorWeight->Process: no biped object for actor {:X}", actor->formID);
				return;
			}

			auto* actorBase = actor->GetActorBase();
			if (!actorBase) return;
			
			auto* actorRace = actorBase->race;
			if (!actorRace) return;

			TRACE("ArmorWeight->Process: begin for actor {:08X}", actor->formID);

			for (const auto& [key, entry] : a_bipedBones) {
				RE::TESObjectARMO* bestArmor = nullptr;
				int bestPriority = GetArmorPriority(bestArmor);
				int chosenSlot = -1;

				for (const auto& bipedSlot : entry.bipedSlots) {
					const int normalized = (bipedSlot >= 30 && bipedSlot - 30 < RE::BIPED_OBJECTS::kTotal) ? (bipedSlot - 30) : 0;
					auto& bipedObject = biped->objects[normalized];

					RE::TESObjectARMO* candidate = nullptr;
					if (bipedObject.item && bipedObject.item->IsArmor()) {
						candidate = bipedObject.item->As<RE::TESObjectARMO>();
						if (candidate == actorRace->skin) candidate = nullptr;
					}

					const int candidatePriority = GetArmorPriority(candidate);

					if (candidatePriority > bestPriority) {
						bestPriority = candidatePriority;
						bestArmor = candidate;
						chosenSlot = bipedSlot;
					}
				}

				const float massMult = ResolveMassMultForArmor(bestArmor);

				TRACE("  -> Entry \"{}\" | Chosen slot \"{}\" | massMult \"{}\"",
					key, chosenSlot, massMult);

				for (const auto& nodeName : entry.bipedNodes) {
					if (auto node = actor->GetNodeByName(nodeName)) {
						SetNodeShapeMass(node, massMult);
					} else {
						TRACE("  -> \"{}\" node could not be found on actor {:08X}", nodeName, actor->formID);
					}
				}
			}

			TRACE("ArmorWeight->Process: end for actor {:08X}", actor->formID);
		}

	private:

		static int GetArmorPriority(RE::TESObjectARMO* a_armor)
		{
			if (!a_armor) return 0;
			if (a_armor->IsHeavyArmor()) return 3;
			if (a_armor->IsLightArmor()) return 2;
			if (a_armor->IsClothing()) return 1;
			return 0;
		}

		static float ResolveMassMultForArmor(RE::TESObjectARMO* a_armor)
		{
			if (!a_armor) return SettingsIni::fArmorWeight_NudeMult;
			if (a_armor->IsClothing()) return SettingsIni::fArmorWeight_ClothMult;
			if (a_armor->IsHeavyArmor()) return SettingsIni::fArmorWeight_HeavyMult;
			if (a_armor->IsLightArmor()) return SettingsIni::fArmorWeight_LightMult;
			return SettingsIni::fArmorWeight_ClothMult;
		}

		static void SetNodeShapeMass(RE::NiAVObject* a_object, const float& a_massMult)
		{
			if (!a_object) return;

			auto* hkpRigidBody = NiUtils::GetRigidBody(a_object);
			if (!hkpRigidBody) return;

			const float currentMass = hkpRigidBody->motion.GetMass();
			if (currentMass <= 0.1f || a_massMult <= 0.0f) return;

			auto defaultMassOpt = NiUtils::GetExtraDataValue<RE::NiFloatExtraData>(a_object, "OriginMass");
			const float defaultMass = defaultMassOpt ? *defaultMassOpt : currentMass;
			if (!defaultMassOpt) NiUtils::StoreExtraData<RE::NiFloatExtraData>(a_object, "OriginMass", defaultMass);

			float newMass = defaultMass * a_massMult;
			if (newMass > SettingsIni::fArmorWeight_MassLimit) newMass = SettingsIni::fArmorWeight_MassLimit;

			hkpRigidBody->motion.SetMass(newMass);
		}
	};
};
