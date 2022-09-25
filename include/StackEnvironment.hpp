#ifndef PASCAL_STACK_ENVIRONMENT_HPP
#define PASCAL_STACK_ENVIRONMENT_HPP

#include <vector>
#include <string>
#include <memory>

#include <cstdint>
#include <cassert>

namespace Pascal
{
	class StackEnvironment
	{
	public:
		StackEnvironment()
			: enclosing(nullptr)
		{ }
		
		StackEnvironment(std::shared_ptr<StackEnvironment> enclosing)
			: enclosing(enclosing)
		{ }

		unsigned size()
		{
			unsigned res = 0;
			for (auto e : vec) res += e.size;
			return res;
		}

		unsigned count()
		{
			return vec.size();
		}

		void push(std::string name, uint8_t size)
		{
			vec.push_back({ name, size });
		}

		unsigned offset(std::string const& name)
		{
			unsigned res = 0;
			for (auto e : vec)
			{
				if (e.name == name)
				{
					return res;
				}
				else
				{
					res += e.size;
				}
			}

			assert(enclosing != nullptr);
			return enclosing->offset(name) - enclosing->size() - 1;
		}

		unsigned enclosingsCount(std::string const& name)
		{
			for (auto e : vec)
			{
				if (e.name == name) return 0;
			}

			assert(enclosing != nullptr);
			return enclosing->enclosingsCount(name) + 1;
		}

		void resetBP()
		{
			bperr = 0;

			if (enclosing) enclosing->resetBP();
		}

		unsigned lazyOffset(std::string const& name)
		{
			unsigned res = 0;
			for (auto e : vec)
			{
				if (e.name == name)
				{
					int oldbperr = bperr;
					bperr = res;
					return res - oldbperr;
				}
				else
				{
					res += e.size;
				}
			}

			// TODO: Test
			assert(enclosing != nullptr);
			enclosing->resetBP();
			unsigned res2 = enclosing->offset(name) - enclosing->size();
			enclosing->resetBP();
			
			int oldbperr = bperr;
			bperr = res2;
			return res2 - oldbperr;
		}

		auto getEnclosing() -> std::shared_ptr<StackEnvironment>
		{
			return enclosing;
		}

		struct Attribs
		{
			std::string name;
			uint8_t size;
		};

		auto getVector() -> std::vector<Attribs>
		{
			return vec;
		}

	private:
		std::shared_ptr<StackEnvironment> enclosing;
		std::vector<Attribs> vec;

		int bperr = 0;
	};
}

#endif // PASCAL_STACK_ENVIRONMENT_HPP