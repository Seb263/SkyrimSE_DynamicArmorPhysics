#pragma once

#include "DataHandler.hpp"
#include "SettingsIni.hpp"

#include "Utils/MiscUtils.hpp"

class NiUtils
{
public:

	static bool IsReferenceRagdollReady(RE::TESObjectREFR* ref)
	{
		if (!ref || !ref->Is3DLoaded()) return false;

		RE::NiAVObject* niAVObject = ref->Get3D();
		if (!niAVObject) return false;

		auto* hkpRigidBody = GetRigidBody(niAVObject);
		if (hkpRigidBody && hkpRigidBody->world && hkpRigidBody->motion.GetMass() > 0.0f) return true;

		return false;
	}

	static RE::hkpRigidBody* GetRigidBody(RE::NiAVObject* a_object)
	{
		if (!a_object) return nullptr;

		auto* collisionObject = a_object->GetCollisionObject();
		if (!collisionObject) return nullptr;

		auto bhkRigidBody = RE::NiPointer<RE::bhkRigidBody>(collisionObject->GetRigidBody());
		if (!bhkRigidBody || !bhkRigidBody->referencedObject) return nullptr;

		auto* hkpRigidBody = static_cast<RE::hkpRigidBody*>(bhkRigidBody->referencedObject.get());
		return hkpRigidBody;
	}

	/**
	 * ============================================================================
	 * Generic helpers for Ni*ExtraData
	 * ----------------------------------------------------------------------------
	 * Provides templates to store, retrieve, and update typed ExtraData
	 * (bool, int, float, string) on NiAVObject without code duplication.
	 * ============================================================================
	 */

	template <class T>
	struct ExtraDataTraits;

	template <>
	struct ExtraDataTraits<RE::NiBooleanExtraData>
	{
		using ValueType = bool;

		static RE::NiBooleanExtraData* Create(const std::string& name, ValueType value)
		{
			return RE::NiBooleanExtraData::Create(name, value);
		}

		static ValueType& GetValue(RE::NiBooleanExtraData* data)
		{
			return data->data;
		}
	};

	template <>
	struct ExtraDataTraits<RE::NiIntegerExtraData>
	{
		using ValueType = std::int32_t;

		static RE::NiIntegerExtraData* Create(const std::string& name, ValueType value)
		{
			return RE::NiIntegerExtraData::Create(name, value);
		}

		static ValueType& GetValue(RE::NiIntegerExtraData* data)
		{
			return data->value;
		}
	};

	template <>
	struct ExtraDataTraits<RE::NiFloatExtraData>
	{
		using ValueType = float;

		static RE::NiFloatExtraData* Create(const std::string& name, ValueType value)
		{
			return RE::NiFloatExtraData::Create(name, value);
		}

		static ValueType& GetValue(RE::NiFloatExtraData* data)
		{
			return data->value;
		}
	};

	template <>
	struct ExtraDataTraits<RE::NiStringExtraData>
	{
		using ValueType = const char*;

		static RE::NiStringExtraData* Create(const std::string& name, ValueType value)
		{
			auto* data = new RE::NiStringExtraData();
			data->name = name;
			data->value = _strdup(value);
			return data;
		}

		static char*& GetValue(RE::NiStringExtraData* data)
		{
			return data->value;
		}
	};

	template <class T>
	static void StoreExtraData(RE::NiAVObject* a_object, const std::string& name, const typename ExtraDataTraits<T>::ValueType& value, const bool overrideExisting = false)
	{
		if (!a_object) return;

		if (auto* existing = a_object->GetExtraData<T>(name)) {
			if (overrideExisting) {
				ExtraDataTraits<T>::GetValue(existing) = value;
			}
			return;
		}

		auto* newData = ExtraDataTraits<T>::Create(name, value);
		a_object->InsertExtraData(newData);
	}

	template <class T>
	static std::optional<typename ExtraDataTraits<T>::ValueType> GetExtraDataValue(RE::NiAVObject* a_object, const std::string& name)
	{
		if (!a_object) return std::nullopt;

		auto* extra = a_object->GetExtraData<T>(name);
		if (!extra) return std::nullopt;

		return ExtraDataTraits<T>::GetValue(extra);
	}

	template <class T>
	static bool UpdateExtraDataValue(RE::NiAVObject* a_object, const std::string& name, typename ExtraDataTraits<T>::ValueType& outValue)
	{
		auto newValue = GetExtraDataValue<T>(a_object, name);
		if (!newValue.has_value()) return false;

		if (outValue != *newValue) {
			outValue = *newValue;
			return true;
		}

		return false;
	}
};
