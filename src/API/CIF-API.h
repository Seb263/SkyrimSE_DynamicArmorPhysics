#pragma once

/*******************************************************************
* CORE IMPACT FRAMEWORK - API
* Do not forget to include this source file to your project!
*******************************************************************/

/* How to create a hook to the API and use it:
SKSE::GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) 
{
    switch (message->type) 
    {
        case MessagingInterface::kPostLoadGame:
        case MessagingInterface::kNewGame:
        {
            if (auto* apiInterface = static_cast<CIF_API::Interface*>(CIF_API::GetAPI())) {
				auto apiVersion = apiInterface->GetVersion().string(".");
				logger::info("Core Impact Framework API v{} registered successfully.", apiVersion);
			} else {
				logger::warn("Core Impact Framework API not found.");
			}
        }
        break;
    }
});
*/

namespace CIF_API
{
	inline void* g_Interface = nullptr;

	enum class InterfaceVersion : uint8_t
	{
		V1,
		Latest = V1
	};

    class Interface_V1
    {
    public:
		virtual ~Interface_V1() = default;

		struct Filter
		{
			enum class ArmorClassType
			{
				kAny, // For internal use only
				kDefault,
				kCloth,
				kLight,
				kHeavy,
				kWeaponDefault,
				kWeaponWood,
				kShieldHeavy,
				kShieldLight
			};

			enum class WeaponType
			{
				kAny, // For internal use only
				kOneHandSword,
				kTwoHandSword,
				kOneHandAxe,
				kTwoHandAxe,
				kOneHandMace,
				kTwoHandMace,
				kDagger,
				kRanged,
				kMagic,
				kHandToHand,
				kBeast,
				kOther
			};

			enum class AttackFilter
			{
				kAny, // For internal use only
				kRegular,
				kPower,
				kBash
			};

			enum class SourceFilter
			{
				kAny, // For internal use only
				kRightHand,
				kLeftHand,
				kDualHand,
				kShout,
				kOther
			};

			enum class StateFilter
			{
				kAny, // For internal use only
				kAlive,
				kDying,
				kKillmove,
				kDead
			};

			enum class ActorSex
			{
				kAny, // For internal use only
				kMale,
				kFemale
			};

			enum class BlockedFilter
			{
				kAny, // For internal use only
				kShieldHeavy,
				kShieldLight,
				kWeapon,
				kNo
			};

			enum class CriticalAttackFilter
			{
				kAny, // For internal use only
				kYes,
				kNo
			};

			enum class SneakAttackFilter
			{
				kAny, // For internal use only
				kYes,
				kNo
			};
		};

		struct BipedEntry
		{
			std::string bipedNode;
			std::string bipedLimb;
			std::vector<std::string> bipedKeys;
			std::vector<int> bipedSlots;
		};

		struct RuntimeHitContext
		{
			// Victim
			RE::FormID              victimFormID = 0x0;
			RE::FormID              victimBaseFormID = 0x0;
			RE::FormID              victimRaceFormID = 0x0;
			RE::FormID              victimMaterialFormID = 0x0;
			std::vector<RE::FormID> victimWornFormIDs = {};
			std::vector<RE::FormID> victimWornKeywordsFormIDs = {};
			RE::FormID              victimSkinFormID = 0x0;
			std::vector<RE::FormID> victimKeywordsFormIDs = {};
			std::vector<RE::FormID> victimPerksFormIDs = {};
			std::vector<RE::FormID> victimSpellsFormIDs = {};
			std::vector<RE::FormID> victimMagicEffectsFormIDs = {};
			Filter::ActorSex        victimSex;

			// Attacker
			RE::FormID              attackerFormID = 0x0;
			RE::FormID              attackerBaseFormID = 0x0;
			RE::FormID              attackerRaceFormID = 0x0;
			RE::FormID              attackerMaterialFormID = 0x0;
			std::vector<RE::FormID> attackerWornFormIDs = {};
			std::vector<RE::FormID> attackerWornKeywordsFormIDs = {};
			RE::FormID              attackerSkinFormID = 0x0;
			std::vector<RE::FormID> attackerKeywordsFormIDs = {};
			std::vector<RE::FormID> attackerPerksFormIDs = {};
			std::vector<RE::FormID> attackerSpellsFormIDs = {};
			std::vector<RE::FormID> attackerMagicEffectsFormIDs = {};
			Filter::ActorSex        attackerSex;

			// HitContext
			RE::FormID                   weaponFormID = 0x0;
			std::vector<RE::FormID>      weaponKeywordsFormIDs = {};
			RE::FormID                   magicItemFormID = 0x0;
			RE::FormID                   projectileFormID = 0x0;
			RE::FormID                   magicEffectFormID = 0x0;
			RE::FormID                   armorFormID = 0x0;
			std::vector<RE::FormID>      armorKeywordsFormIDs = {};
			Filter::WeaponType           weaponType;
			Filter::ArmorClassType       armorClass;
			Filter::BlockedFilter        blocked;
			Filter::StateFilter          state;
			Filter::AttackFilter         attack;
			Filter::SourceFilter         source;
			Filter::CriticalAttackFilter criticalAttack;
			Filter::SneakAttackFilter    sneakAttack;
			float                        percentHealth = 0.0f;
			float                        percentDamage = 0.0f;
			float                        limbHealth = 100.0f;
			BipedEntry                   bipedEntry;
			RE::NiPoint3                 hitPosition;
			RE::NiPoint3                 hitDirection;

			// Modifiers
			float damageMult = 1.0f;
			float damageCalc = 0.0f;
			float damageLimbMult = 1.0f;
		};

		struct BipedBonesEntry
		{
			int priority;
			bool isLimbEntry;
			std::vector<std::string> bipedNodes;
			std::vector<int> bipedSlots;
		};
		using BipedBonesMap = std::unordered_map<std::string, BipedBonesEntry>;

		using IniValue = std::variant<bool, int, float, std::string>;

		virtual REL::Version GetVersion() noexcept = 0;

		virtual IniValue GetIniValue(const std::string& key_section, const IniValue& defaultValue = {}) noexcept = 0;

		virtual bool SetIniValue(const std::string& key_section, const IniValue& value) noexcept = 0;

		virtual BipedBonesMap GetBipedBonesMap(RE::Actor* actor, const bool limbEntriesOnly) noexcept = 0;
		
		virtual RuntimeHitContext GenerateContext(RE::Actor* actor, RE::Actor* attacker = nullptr) noexcept = 0;

		virtual RE::BGSCollisionLayer* GetBloodCollisionLayer() noexcept = 0;

		virtual bool CastBloodSpray(RE::TESObjectREFR* caster, RE::SpellItem* spell, RE::NiPoint3 position, RE::NiPoint3 direction, float power = 1.0f, float spread = -1.0f) noexcept = 0;

		using PreHitCallback = std::function<void(const std::string eventName, RE::HitData& hitData)>;
		virtual void RegisterPreHitCallback(const std::string& eventName, int priority, PreHitCallback callback) noexcept = 0;

		using PostHitCallback = std::function<void(const std::string eventName, const RuntimeHitContext context)>;
		virtual void RegisterPostHitCallback(const std::string& eventName, int priority, PostHitCallback callback) noexcept = 0;
		virtual void RegisterPostDeferredHitCallback(const std::string& eventName, int priority, PostHitCallback callback) noexcept = 0;
	};

	using Interface = Interface_V1;

	using _RequestPluginAPI = void* (*)(InterfaceVersion version, const char* pluginName, REL::Version pluginVersion);

    inline void* GetAPI(InterfaceVersion version = InterfaceVersion::Latest)
    {
        if (g_Interface) return g_Interface;

        const auto handle = GetModuleHandleA("CoreImpactFramework.dll");
        if (!handle) return nullptr;

        const auto request = reinterpret_cast<_RequestPluginAPI>(GetProcAddress(handle, "RequestPluginAPI"));
        if (!request) return nullptr;

        const auto plugin = SKSE::PluginDeclaration::GetSingleton();
        g_Interface = request(version, std::string(plugin->GetName()).c_str(), plugin->GetVersion());

        return g_Interface;
    }
}
