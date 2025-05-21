#ifndef TERNARY_SEARCH_TREE_DICTIONARY_H
#define TERNARY_SEARCH_TREE_DICTIONARY_H

#include "allocator.h"
#include "span.hpp"

#include "pch/std.hpp"

namespace HXSL
{
	template <typename T, typename Equals = std::equal_to<T>>
	class TernarySearchTreeDictionary
	{
		static constexpr size_t TST_INVALID_INDEX = static_cast<size_t>(-1);

	private:
		struct Node
		{
			size_t ParentIndex;
			char SplitChar;
			size_t Left;
			size_t Middle;
			size_t Right;
			T Value;

			Node(size_t parentIndex, char splitChar, T value)
				: ParentIndex(parentIndex), SplitChar(splitChar), Left(TST_INVALID_INDEX), Middle(TST_INVALID_INDEX), Right(TST_INVALID_INDEX), Value(value)
			{
			}
		};

		std::vector<Node> nodes;
		T defaultValue;
		Equals equals;

		size_t AddNode(size_t parentIndex, char splitChar)
		{
			size_t index = nodes.size();
			nodes.push_back(Node(parentIndex, splitChar, defaultValue));
			return index;
		}

		void RemoveNode(size_t index)
		{
			// Could potentially degrade performance because of the loss of cache locality when looking up, but it's as not bad as a linked list.
			size_t last = Count() - 1;
			if (last != index)
			{
				auto lastNode = nodes[last];
				nodes[index] = lastNode;
				size_t parentIndex = lastNode.ParentIndex;
				if (parentIndex != -1)
				{
					auto parent = &nodes[parentIndex];
					if (parent->Right == last)
						parent->Right = index;
					if (parent->Middle == last)
						parent->Middle = index;
					if (parent->Left == last)
						parent->Left = index;
				}
			}

			nodes.pop_back();
		}

		size_t InsertChar(size_t& index, char c)
		{
			while (true)
			{
				Node node = nodes[index];

				if (node.SplitChar == '\0')
				{
					node.SplitChar = c;

					size_t newIndex = AddNode(index, '\0');
					node.Middle = newIndex;

					nodes[index] = node;

					return newIndex;
				}

				if (c < node.SplitChar)
				{
					if (node.Left == -1)
					{
						node.Left = AddNode(index, '\0');
						nodes[index] = node;
					}
					index = node.Left;
				}
				else if (c > node.SplitChar)
				{
					if (node.Right == -1)
					{
						node.Right = AddNode(index, '\0');
						nodes[index] = node;
					}
					index = node.Right;
				}
				else
				{
					if (node.Middle == -1)
					{
						node.Middle = AddNode(index, '\0');
						nodes[index] = node;
					}
					return node.Middle;
				}
			}
		}

		bool LookupNode(const StringSpan& key, size_t& nodeIndex, size_t& matchedLength) const
		{
			size_t index = 0;
			size_t lastMatchedIndex = TST_INVALID_INDEX;
			matchedLength = 0;

			size_t i = 0;
			while (i < key.length)
			{
				Node node = nodes[index];
				char c = key[i];

				if (c < node.SplitChar)
				{
					if (node.Left == -1)
					{
						break;
					}

					index = node.Left;
				}
				else if (c > node.SplitChar)
				{
					if (node.Right == -1)
					{
						break;
					}

					index = node.Right;
				}
				else
				{
					i++;

					if (node.Middle == -1)
					{
						break;
					}

					lastMatchedIndex = node.Middle;
					matchedLength = i;
					index = node.Middle;
				}
			}

			if (lastMatchedIndex != -1)
			{
				nodeIndex = lastMatchedIndex;
				return true;
			}

			nodeIndex = TST_INVALID_INDEX;
			return false;
		}

	public:
		TernarySearchTreeDictionary(T defaultVal = T(), Equals equalsFunc = Equals()) : defaultValue(defaultVal), equals(equalsFunc)
		{
			AddNode(TST_INVALID_INDEX, '\0');
		}

		virtual ~TernarySearchTreeDictionary()
		{
		}

		size_t Count() const
		{
			return nodes.size();
		}

		void ShrinkToFit()
		{
			nodes.resize(nodes.size());
		}

		void Clear()
		{
			nodes.clear();
			AddNode(-1, '\0');
		}

		bool RemoveKey(const StringSpan& key)
		{
			if (Count() == 0)
			{
				return false;
			}

			size_t index;
			size_t matchedLength;
			if (!LookupNode(key, index, matchedLength) || matchedLength != key.length)
			{
				return false;
			}

			while (index != -1)
			{
				const Node& node = nodes[index];
				if (equals(node.Value, defaultValue) && node.Left == -1 && node.Middle == -1 && node.Right == -1)
				{
					size_t parentIndex = node.ParentIndex;

					if (parentIndex != -1)
					{
						Node& parent = nodes[parentIndex];
						if (parent.Left == index)
							parent.Left = -1;
						if (parent.Middle == index)
							parent.Middle = -1;
						if (parent.Right == index)
							parent.Right = -1;
					}

					RemoveNode(index);

					index = parentIndex;
				}
				else
				{
					break;
				}
			}

			return true;
		}

		void InsertRange(const std::vector<std::pair<const StringSpan&, T>>& dict)
		{
			for (const auto& pair : dict)
			{
				const StringSpan& key = pair.first;
				const T& value = pair.second;
				Insert(key, value);
			}
		}

		void Insert(const StringSpan& key, T value)
		{
			size_t index = 0;

			for (char c : key)
			{
				index = InsertChar(index, c);
			}

			nodes[index].Value = value;
		}

		bool MatchLongestPrefix(const StringSpan& text, T& value, size_t& matchedLength) const
		{
			if (Count() == 1)
			{
				value = defaultValue;
				matchedLength = 0;
				return false;
			}

			size_t nodeIndex;
			if (LookupNode(text, nodeIndex, matchedLength))
			{
				auto& node = nodes[nodeIndex];
				value = node.Value;
				return true;
			}
			value = defaultValue;
			return false;
		}

		T operator[](const StringSpan& key) const
		{
			size_t index;
			size_t len;
			if (!LookupNode(key, index, len) || key.length != len)
			{
				throw std::runtime_error("Key not found.");
			}
			return nodes[index].Value;
		}
	};
}

#endif // TERNARY_SEARCH_TREE_DICTIONARY_H