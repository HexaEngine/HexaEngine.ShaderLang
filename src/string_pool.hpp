#ifndef STRING_POOL_H
#define STRING_POOL_H

#include <string>

namespace HXSL
{
	class StringPool
	{
	public:
		const std::string& add(std::string& str)
		{
			auto r = std::make_unique<std::string>(str);
			strings_.push_back(std::move(r));
			return *strings_.back().get();
		}

		const std::string& add(const char* str)
		{
			auto r = std::make_unique<std::string>(str);
			strings_.push_back(std::move(r));
			return *strings_.back().get();
		}

		const char* get_cstr(size_t index) const
		{
			if (index < strings_.size())
			{
				return strings_[index]->c_str();
			}
			return nullptr;
		}

		void clear()
		{
			strings_.clear();
		}

		~StringPool()
		{
		}

	private:
		std::vector<std::unique_ptr<std::string>> strings_;
	};
}
#endif