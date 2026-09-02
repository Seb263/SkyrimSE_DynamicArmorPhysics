#include "Events.h"

namespace Events
{
	RE::BSEventNotifyControl ModEventSink::ProcessEvent(const RE::TESLoadGameEvent* event, RE::BSTEventSource<RE::TESLoadGameEvent>*)
	{
		using namespace ModCore;
		
		ModData::lastLoadPoint = std::chrono::steady_clock::now();
		ModData::previousCell = 0x0;

		Main::ResetExecutions();
		ArmorMaterial::ResetContactDebounce();

		return continueEvent;
	}

	RE::BSEventNotifyControl ModEventSink::ProcessEvent(const RE::TESDeathEvent* event, RE::BSTEventSource<RE::TESDeathEvent>*)
	{
		using namespace ModCore;

		if (event->dead) return continueEvent;

		RE::Actor* target = event->actorDying && event->actorDying.get() ? event->actorDying->As<RE::Actor>() : nullptr;
		if (!target) return continueEvent;

		Main::MaintainActor(target, true, true);

		return continueEvent;
	}

	RE::BSEventNotifyControl ModEventSink::ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*)
	{
		using namespace ModCore;

		RE::Actor* target = event->actor ? event->actor->As<RE::Actor>() : nullptr;
		if (!target || !target->IsDead()) return continueEvent;

		Main::MaintainActor(target, true, true);

		return continueEvent;
	}
}
