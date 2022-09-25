#ifndef PASCAL_ENVIRONMENT_HPP
#define PASCAL_ENVIRONMENT_HPP

#include <string>
#include <unordered_map>
#include <stdexcept>
#include <memory>

namespace Pascal
{
	template <typename T>
	class Environment
	{
	public:
		Environment()
		{
			m_Enclosing = nullptr;
		}

		Environment(std::shared_ptr<Environment<T>> enclocing)
		{ 
			m_Enclosing = enclocing;
		}

		// return value - is new
		bool define(std::string const& name, T obj)
		{
			bool isNew = !m_Map.count(name);
			m_Map.insert({ name, obj });
			return isNew;
		}

		T& lookup(std::string const& name)
		{
			if (m_Map.count(name))
			{
				return m_Map.at(name);
			}
			else throw std::out_of_range("Environment");
		}

		T& lookupAndAncestors(std::string const& name)
		{
			if (has(name)) return lookup(name);
			if (m_Enclosing != nullptr) return m_Enclosing->lookupAndAncestors(name);

			throw std::out_of_range("Environment");
		}

		std::shared_ptr<Environment<T>> getEnclosing()
		{
			return m_Enclosing;
		}

		Environment<T>* getAncestor(unsigned distance)
		{
			// TODO: mmm, shared_ptrs?
			Environment<T>* current = this;
			for (; distance > 0; distance--)
			{
				current = current->getEnclosing().get();
			}
			return current;
		}

		bool has(std::string const& name)
		{
			return m_Map.count(name);
		}

		bool hasAndAncestors(std::string const& name)
		{
			if (has(name)) return true;
			if (m_Enclosing != nullptr) return m_Enclosing->hasAndAncestors(name);

			return false;
		}

		std::unordered_map<std::string, T> getMap()
		{
			return m_Map;
		}

		void defineBuiltins(T placeholder)
		{
			define("integer", placeholder);
			define("long", placeholder);
			define("make_bcd", placeholder);
			define("debug_print_bcd", placeholder);
			define("debug_print_bcd_high", placeholder);
			define("cls", placeholder);
		}

	private:
		std::shared_ptr<Environment<T>> m_Enclosing;
		std::unordered_map<std::string, T> m_Map;
	};
}

#endif // PASCAL_ENVIRONMENT_HPP