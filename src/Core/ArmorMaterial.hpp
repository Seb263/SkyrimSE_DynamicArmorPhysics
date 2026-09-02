#pragma once

#include "DataHandler.hpp"
#include "SettingsIni.hpp"

#include "Core/Structure.h"

#include "Utils/ModUtils.hpp"
#include "Utils/NiUtils.hpp"

#include "API/CIF-API.h"

namespace ModCore
{
	using namespace ModData;
	using namespace CoreStructure;

	class ArmorMaterial
	{
	public:

		static void RegisterWorld(RE::bhkWorld* a_world)
		{
			if (!a_world) return;

			auto* hkpWorldPtr = a_world->GetWorld1();
			if (!hkpWorldPtr) return;

			RE::BSWriteLockGuard writeLock(a_world->worldLock);

			const auto it = std::find(hkpWorldPtr->contactListeners.begin(),hkpWorldPtr->contactListeners.end(), &s_listener);

			if (it == hkpWorldPtr->contactListeners.end()) {
				hkpWorldPtr->contactListeners.push_back(&s_listener);
				TRACE("ArmorMaterial->RegisterWorld: contact listener added to world [{}]", static_cast<void*>(a_world));
			}
		}

		static void OnRagdollContact(RE::Actor* a_actor, const bool force = false)
		{
			if (!a_actor || !SettingsIni::bArmorMaterial_Status || !ModData::CIF_API_Interface) return;

			if (!CanProceed(a_actor, force)) return;

			ApplyMaterials(a_actor);
		}

		static void ResetContactDebounce()
		{
			std::scoped_lock lock(contactMutex);
			lastActor = 0x0;
		}

		static void MaterialMaintenance()
		{
			static std::unordered_map<RE::MATERIAL_ID, bool> modifiedMaterials;

			for (const auto& [id, hadFlag] : modifiedMaterials) {
				if (auto* material = RE::BGSMaterialType::GetMaterialType(id)) {
					if (hadFlag) material->flags.set(RE::BGSMaterialType::FLAG::kArrowsStick);
					else material->flags.reset(RE::BGSMaterialType::FLAG::kArrowsStick);
				}
			}

			modifiedMaterials.clear();

			const std::array<RE::MATERIAL_ID, 4> materials = {
				ModUtils::GetMaterialIDFromString(SettingsIni::sArmorMaterial_NudeMaterialID),
				ModUtils::GetMaterialIDFromString(SettingsIni::sArmorMaterial_ClothMaterialID),
				ModUtils::GetMaterialIDFromString(SettingsIni::sArmorMaterial_LightMaterialID),
				ModUtils::GetMaterialIDFromString(SettingsIni::sArmorMaterial_HeavyMaterialID)
			};

			for (const auto& mat : materials) {
				if (auto* material = RE::BGSMaterialType::GetMaterialType(mat)) {
					if (!modifiedMaterials.contains(mat)) {
						modifiedMaterials.emplace(mat, material->flags.any(RE::BGSMaterialType::FLAG::kArrowsStick));
					}

					material->flags.set(RE::BGSMaterialType::FLAG::kArrowsStick);
				}
			}
		}

	private:

		class ContactListener : public RE::hkpContactListener
		{
		public:

			void ContactPointCallback(const RE::hkpContactPointEvent& a_event) override
			{
				const auto* prop = a_event.contactPointProperties;
				if (!prop || !a_event.contactPoint || !a_event.contactMgr ||
					prop->flags.none(RE::hkContactPointMaterial::Flag::kIsNew) ||
					prop->flags.any(RE::hkContactPointMaterial::Flag::kIsDisabled)) return;

				auto* rigidBodyA = a_event.bodies[0];
				if (!rigidBodyA) return;
				
				auto* collidableRef = RE::TESHavokUtilities::FindCollidableRef(rigidBodyA->collidable);
				if (!collidableRef || !collidableRef->Is3DLoaded()) return;

				auto* actor = collidableRef->As<RE::Actor>();
				if (!actor || (!actor->IsDead() && !actor->IsInRagdollState())) return;

				OnRagdollContact(actor);
			}
		};

		static bool CanProceed(RE::Actor* a_actor, bool a_force)
		{
			if (!a_actor) return false;
			std::scoped_lock lock(contactMutex);

			if (!a_force && lastActor == a_actor->formID) return false;

			if (auto* player = RE::PlayerCharacter::GetSingleton()) {
				if (player->GetDistance(a_actor) > SettingsIni::fArmorMaterial_EffectiveRadius) return false;
			}

			lastActor = a_actor->formID;
			return true;
		}

		static void ApplyMaterials(RE::Actor* actor)
		{
			const auto a_bipedBones = CIF_API_Interface->GetBipedBonesMap(actor, false);
			const auto& biped = actor->GetBiped(false);
			if (!biped) {
				TRACE("ArmorMaterial->ApplyMaterials: no biped object for actor {:X}", actor->formID);
				return;
			}

			auto* actorBase = actor->GetActorBase();
			if (!actorBase) return;

			auto* actorRace = actorBase->race;
			if (!actorRace) return;

			TRACE("ArmorMaterial->ApplyMaterials: begin for actor {:08X}", actor->formID);

			int processedCount = 0;
			std::unordered_set<RE::hkpShape*> processedShapes;

			for (const auto& [key, entry] : a_bipedBones) {
				RE::TESObjectARMO* bestArmor = nullptr;
				RE::MATERIAL_ID bestMaterial = ResolveMaterialForArmor(nullptr);
				int bestPriority = GetMaterialPriority(bestMaterial);
				int chosenSlot = -1;

				for (const auto& bipedSlot : entry.bipedSlots) {
					int normalized = (bipedSlot >= 30 && bipedSlot - 30 < RE::BIPED_OBJECTS::kTotal) ? (bipedSlot - 30) : 0;
					auto& bipedObject = biped->objects[normalized];

					RE::TESObjectARMO* candidate = nullptr;
					if (bipedObject.item && bipedObject.item->IsArmor()) {
						candidate = bipedObject.item->As<RE::TESObjectARMO>();
						if (candidate == actorRace->skin) candidate = nullptr;
					}

					const auto candidateMaterial = ResolveMaterialForArmor(candidate);
					const int candidatePriority = GetMaterialPriority(candidateMaterial);

					if (candidatePriority > bestPriority) {
						bestPriority = candidatePriority;
						bestMaterial = candidateMaterial;
						bestArmor = candidate;
						chosenSlot = bipedSlot;
					}
				}

				if (bestMaterial == RE::MATERIAL_ID::kNone) {
					auto* material = actorRace->bloodImpactMaterial;
					while (material && material->materialID == RE::MATERIAL_ID::kNone) {
						material = material->parentType;
					}

					if (material) bestMaterial = material->materialID;
				}

				const auto requestedMaterial = bestMaterial;
				TRACE("  -> Entry \"{}\" | Chosen slot \"{}\" | Requested material \"{}\"",
					key, chosenSlot, RE::MaterialIDToString(requestedMaterial));

				for (const auto& nodeName : entry.bipedNodes) {
					if (auto node = actor->GetNodeByName(nodeName)) {
						if (ApplyMaterialToNode(node, requestedMaterial, processedShapes)) {
							processedCount++;
						}
					} else {
						TRACE("  -> \"{}\" node could not be found on actor {:08X}", nodeName, actor->formID);
					}
				}
			}
			TRACE("ArmorMaterial->ApplyMaterials: end for actor {:08X}, {} shape(s) updated", actor->formID, processedCount);
		}

		static int GetMaterialPriority(RE::MATERIAL_ID a_material)
		{
			if (a_material == ModUtils::GetMaterialIDFromString(SettingsIni::sArmorMaterial_HeavyMaterialID)) return 3;
			if (a_material == ModUtils::GetMaterialIDFromString(SettingsIni::sArmorMaterial_LightMaterialID)) return 2;
			if (a_material == ModUtils::GetMaterialIDFromString(SettingsIni::sArmorMaterial_ClothMaterialID)) return 1;
			return 0;
		}

		static RE::MATERIAL_ID ResolveMaterialForArmor(RE::TESObjectARMO* a_armor)
		{
			if (!a_armor) return ModUtils::GetMaterialIDFromString(SettingsIni::sArmorMaterial_NudeMaterialID);
			if (a_armor->IsClothing()) return ModUtils::GetMaterialIDFromString(SettingsIni::sArmorMaterial_ClothMaterialID);
			if (a_armor->IsLightArmor()) return ModUtils::GetMaterialIDFromString(SettingsIni::sArmorMaterial_LightMaterialID);
			if (a_armor->IsHeavyArmor()) return ModUtils::GetMaterialIDFromString(SettingsIni::sArmorMaterial_HeavyMaterialID);
			return RE::MATERIAL_ID::kNone;
		}

		static bool ApplyMaterialToNode(RE::NiAVObject* a_node, RE::MATERIAL_ID a_material, std::unordered_set<RE::hkpShape*>& a_processedShapes)
		{
			if (!a_node || !a_node->AsNode()) return false;

			auto* hkpRB = NiUtils::GetRigidBody(a_node);
			if (!hkpRB) return false;

			auto* shape = const_cast<RE::hkpShape*>(hkpRB->GetShape());
			if (!shape || !shape->userData) return false;

			if (!a_processedShapes.insert(shape).second) return false;

			auto* bhkShape = static_cast<RE::bhkShape*>(shape->userData);

			if (bhkShape->materialID == a_material) {
				TRACE("  -> Node \"{}\" already has material \"{}\"", a_node->name, RE::MaterialIDToString(a_material));
				return false;
			}
			bhkShape->materialID = a_material;

			TRACE("  -> Node \"{}\" | Applied shape material : \"{}\"", a_node->name, RE::MaterialIDToString(a_material));
			return true;
		}

		static inline ContactListener s_listener{};
		static inline RE::FormID lastActor = 0x0;
		static inline std::mutex contactMutex;
	};
}
