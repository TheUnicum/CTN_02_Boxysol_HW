#pragma once

#include "Box.h"
#include <functional>
#include <unordered_map>
#include <typeinfo>

using TypePair = std::pair<		// in <Game.cpp> auto& tid0 = typeid(boxPtrs[0]->GetColorTrait());
	const std::type_info*,
	const std::type_info*
>;

namespace std
{
	template <>
	struct hash<TypePair>
	{
		size_t operator()(const TypePair& tp) const
		{
			const auto hash0 = tp.first->hash_code();
			return hash0 ^ (tp.second->hash_code() + 0x9e3779b9 + (hash0 << 6) + (hash0 >> 2));
		}
	};
	template <>
	struct equal_to<TypePair>
	{
		bool operator()(const TypePair& lhs, const TypePair& rhs) const
		{
			return *lhs.first == *rhs.first && *lhs.second == *rhs.second;
		}
	};
}

class PatternMatchingListener : public b2ContactListener
{
public:
	template<class T, class U, class F>
	void Case(F f)
	{
		handlers[{&typeid(T), & typeid(U)}] = f;
		handlers[{&typeid(U), & typeid(T)}] = std::bind(
			f, std::placeholders::_2, std::placeholders::_1
		);
	}
	template<class T, class U>
	void HasCase() const
	{
		return handlers.count({ &typedef(T),& typedef(U) }) > 0;
	}
	template<class T, class U>
	void ClearCase()
	{
		handlers.erase[{&typeid(T), & typeid(U)}];
		handlers.erase[{&typeid(U), & typeid(T)}];
	}
	template<class F>
	void Default(F f)
	{
		def = f;
	}
	void BeginContact(b2Contact* contact) override
	{
		const b2Body* bodyPtrs[] = { contact->GetFixtureA()->GetBody(),contact->GetFixtureB()->GetBody() };
		if (bodyPtrs[0]->GetType() == b2BodyType::b2_dynamicBody &&
			bodyPtrs[1]->GetType() == b2BodyType::b2_dynamicBody)
		{
			Switch(
				*reinterpret_cast<Box*>(bodyPtrs[0]->GetUserData()),
				*reinterpret_cast<Box*>(bodyPtrs[1]->GetUserData())
			);
		}
	}
private:
	void Switch(Box& a, Box& b)
	{
		auto i = handlers.find({
			&typeid(a.GetColorTrait()),
			&typeid(b.GetColorTrait())
		});
		if (i != handlers.end())
		{
			i->second(a, b);
		}
		else
		{
			def(a, b);
		}
	}

private:
	std::unordered_map<TypePair, std::function<void(Box&, Box&)>> handlers;
	std::function<void(Box&, Box&)> def = [](Box&, Box&) {};
};