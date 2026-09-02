#pragma once

#include "DataHandler.hpp"
#include "SettingsIni.hpp"

class ModUtils
{
public:

	static void ProcessGridCells(const std::function<void(RE::TESObjectCELL*)>& processCell)
	{
		auto* tes = RE::TES::GetSingleton();
		if (!tes) return;

		if (auto* cell = tes->interiorCell; cell && cell->IsAttached()) {
			processCell(cell);
		} else {
			if (const auto gridLength = tes->gridCells ? tes->gridCells->length : 0; gridLength > 0) {
				for (std::uint32_t x = 0; x < gridLength; ++x) {
					for (std::uint32_t y = 0; y < gridLength; ++y) {
						auto* cell = tes->gridCells->GetCell(x, y);
						if (!cell || !cell->IsAttached()) continue;

						processCell(cell);
					}
				}
			}
		}
	}

	static RE::MATERIAL_ID GetMaterialIDFromString(const std::string& a_material)
	{
		static const std::unordered_map<std::string, RE::MATERIAL_ID> materials = {
			{ "StoneBroken", RE::MATERIAL_ID::kStoneBroken },
			{ "BlockBlade1Hand", RE::MATERIAL_ID::kBlockBlade1Hand },
			{ "Meat", RE::MATERIAL_ID::kMeat },
			{ "CarriageWheel", RE::MATERIAL_ID::kCarriageWheel },
			{ "MetalLight", RE::MATERIAL_ID::kMetalLight },
			{ "WoodLight", RE::MATERIAL_ID::kWoodLight },
			{ "Snow", RE::MATERIAL_ID::kSnow },
			{ "Gravel", RE::MATERIAL_ID::kGravel },
			{ "ChainMetal", RE::MATERIAL_ID::kChainMetal },
			{ "Bottle", RE::MATERIAL_ID::kBottle },
			{ "Wood", RE::MATERIAL_ID::kWood },
			{ "Ash", RE::MATERIAL_ID::kAsh },
			{ "Skin", RE::MATERIAL_ID::kSkin },
			{ "BlockBlunt", RE::MATERIAL_ID::kBlockBlunt },
			{ "DLC1DeerSkin", RE::MATERIAL_ID::kDLC1DeerSkin },
			{ "Insect", RE::MATERIAL_ID::kInsect },
			{ "Barrel", RE::MATERIAL_ID::kBarrel },
			{ "CeramicMedium", RE::MATERIAL_ID::kCeramicMedium },
			{ "Basket", RE::MATERIAL_ID::kBasket },
			{ "Ice", RE::MATERIAL_ID::kIce },
			{ "GlassStairs", RE::MATERIAL_ID::kGlassStairs },
			{ "StoneStairs", RE::MATERIAL_ID::kStoneStairs },
			{ "Water", RE::MATERIAL_ID::kWater },
			{ "DraugrSkeleton", RE::MATERIAL_ID::kDraugrSkeleton },
			{ "Blade1Hand", RE::MATERIAL_ID::kBlade1Hand },
			{ "Book", RE::MATERIAL_ID::kBook },
			{ "Carpet", RE::MATERIAL_ID::kCarpet },
			{ "MetalSolid", RE::MATERIAL_ID::kMetalSolid },
			{ "Axe1Hand", RE::MATERIAL_ID::kAxe1Hand },
			{ "BlockBlade2Hand", RE::MATERIAL_ID::kBlockBlade2Hand },
			{ "OrganicLarge", RE::MATERIAL_ID::kOrganicLarge },
			{ "Amulet", RE::MATERIAL_ID::kAmulet },
			{ "WoodStairs", RE::MATERIAL_ID::kWoodStairs },
			{ "Mud", RE::MATERIAL_ID::kMud },
			{ "BoulderSmall", RE::MATERIAL_ID::kBoulderSmall },
			{ "SnowStairs", RE::MATERIAL_ID::kSnowStairs },
			{ "StoneHeavy", RE::MATERIAL_ID::kStoneHeavy },
			{ "CharacterBumper", RE::MATERIAL_ID::kCharacterBumper },
			{ "Trap", RE::MATERIAL_ID::kTrap },
			{ "BowsStaves", RE::MATERIAL_ID::kBowsStaves },
			{ "Alduin", RE::MATERIAL_ID::kAlduin },
			{ "BlockBowsStaves", RE::MATERIAL_ID::kBlockBowsStaves },
			{ "WoodAsStairs", RE::MATERIAL_ID::kWoodAsStairs },
			{ "SteelGreatSword", RE::MATERIAL_ID::kSteelGreatSword },
			{ "Grass", RE::MATERIAL_ID::kGrass },
			{ "BoulderLarge", RE::MATERIAL_ID::kBoulderLarge },
			{ "StoneAsStairs", RE::MATERIAL_ID::kStoneAsStairs },
			{ "Blade2Hand", RE::MATERIAL_ID::kBlade2Hand },
			{ "BottleSmall", RE::MATERIAL_ID::kBottleSmall },
			{ "BoneActor", RE::MATERIAL_ID::kBoneActor },
			{ "Sand", RE::MATERIAL_ID::kSand },
			{ "MetalHeavy", RE::MATERIAL_ID::kMetalHeavy },
			{ "DLC1SabreCatPelt", RE::MATERIAL_ID::kDLC1SabreCatPelt },
			{ "IceForm", RE::MATERIAL_ID::kIceForm },
			{ "Dragon", RE::MATERIAL_ID::kDragon },
			{ "Blade1HandSmall", RE::MATERIAL_ID::kBlade1HandSmall },
			{ "SkinSmall", RE::MATERIAL_ID::kSkinSmall },
			{ "PotsPans", RE::MATERIAL_ID::kPotsPans },
			{ "SkinSkeleton", RE::MATERIAL_ID::kSkinSkeleton },
			{ "Blunt1Hand", RE::MATERIAL_ID::kBlunt1Hand },
			{ "StoneStairsBroken", RE::MATERIAL_ID::kStoneStairsBroken },
			{ "SkinLarge", RE::MATERIAL_ID::kSkinLarge },
			{ "Organic", RE::MATERIAL_ID::kOrganic },
			{ "Bone", RE::MATERIAL_ID::kBone },
			{ "WoodHeavy", RE::MATERIAL_ID::kWoodHeavy },
			{ "Chain", RE::MATERIAL_ID::kChain },
			{ "Dirt", RE::MATERIAL_ID::kDirt },
			{ "Ghost", RE::MATERIAL_ID::kGhost },
			{ "SkinMetalLarge", RE::MATERIAL_ID::kSkinMetalLarge },
			{ "BlockAxe", RE::MATERIAL_ID::kBlockAxe },
			{ "ArmorLight", RE::MATERIAL_ID::kArmorLight },
			{ "ShieldLight", RE::MATERIAL_ID::kShieldLight },
			{ "Coin", RE::MATERIAL_ID::kCoin },
			{ "BlockBlunt2Hand", RE::MATERIAL_ID::kBlockBlunt2Hand },
			{ "ShieldHeavy", RE::MATERIAL_ID::kShieldHeavy },
			{ "ArmorHeavy", RE::MATERIAL_ID::kArmorHeavy },
			{ "Arrow", RE::MATERIAL_ID::kArrow },
			{ "Glass", RE::MATERIAL_ID::kGlass },
			{ "Stone", RE::MATERIAL_ID::kStone },
			{ "WaterPuddle", RE::MATERIAL_ID::kWaterPuddle },
			{ "Cloth", RE::MATERIAL_ID::kCloth },
			{ "SkinMetalSmall", RE::MATERIAL_ID::kSkinMetalSmall },
			{ "Ward", RE::MATERIAL_ID::kWard },
			{ "Web", RE::MATERIAL_ID::kWeb },
			{ "TrailerSteelSword", RE::MATERIAL_ID::kTrailerSteelSword },
			{ "Blunt2Hand", RE::MATERIAL_ID::kBlunt2Hand },
			{ "DLC1SwingingBridge", RE::MATERIAL_ID::kDLC1SwingingBridge },
			{ "BoulderMedium", RE::MATERIAL_ID::kBoulderMedium }
		};

		auto it = materials.find(a_material);
		if (it != materials.end()) return it->second;

		return RE::MATERIAL_ID::kNone;
	}
};
