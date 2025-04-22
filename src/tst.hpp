#ifndef TERNARY_SEARCH_TREE_DICTIONARY_H
#define TERNARY_SEARCH_TREE_DICTIONARY_H

#include "allocator.h"
#include "text_span.h"

#include <utility>
#include <stdexcept>
#include <string>
#include <vector>

namespace HXSL 
{
	template <typename T>
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

			Node(size_t parentIndex, char splitChar)
				: ParentIndex(parentIndex), SplitChar(splitChar), Left(TST_INVALID_INDEX), Middle(TST_INVALID_INDEX), Right(TST_INVALID_INDEX), Value(T()) {
			}
		};

		Node* nodes;
		size_t count;
		size_t capacity;

		void Grow(size_t value)
		{
			size_t newCapacity = count == 0 ? 4 : 2 * count;
			if (newCapacity < value)
			{
				newCapacity = value;
			}

			Capacity(newCapacity);
		}

		void AddWithResize(Node node)
		{
			size_t index = count;
			Grow(index + 1);
			count = index + 1;
			nodes[index] = node;
		}

		size_t AddNode(size_t parentIndex, char splitChar)
		{
			Node node = Node(parentIndex, splitChar);
			size_t index = count;
			if (index < capacity)
			{
				count++;
				nodes[index] = node;
			}
			else
			{
				AddWithResize(node);
			}

			return index;
		}

		void RemoveNode(size_t index)
		{
			// Could potentially degrade performance because of the loss of cache locality when looking up, but it's as not bad as a linked list.
			size_t last = count - 1;
			if (last != index)
			{
				var lastNode = nodes[last];
				nodes[index] = lastNode;
				size_t parentIndex = lastNode.ParentIndex;
				if (parentIndex != -1)
				{
					var parent = &nodes[parentIndex];
					if (parent->Right == last)
						parent->Right = index;
					if (parent->Middle == last)
						parent->Middle = index;
					if (parent->Left == last)
						parent->Left = index;
				}
			}

			count--;
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

		bool LookupNode(TextSpan key, size_t& nodeIndex, size_t& matchedLength)
		{
			size_t index = 0;
			size_t lastMatchedIndex = TST_INVALID_INDEX;
			matchedLength = 0;

			size_t i = 0;
			while (i < key.Length)
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

			nodeIndex = T();
			return false;
		}

	public:
		TernarySearchTreeDictionary() : nodes(nullptr), count(0), capacity(0)
		{
			AddNode(TST_INVALID_INDEX, '\0');
		}

		~TernarySearchTreeDictionary()
		{
			if (nodes != nullptr)
			{
				free(nodes);
				nodes = nullptr;
			}

			count = 0;
			capacity = 0;
		}

		size_t Count() const
		{
			return count;
		}

		size_t Capacity() const
		{
			return capacity;
		}

		void Capacity(size_t value)
		{
			if (nodes == nullptr)
			{
				nodes = (Node*)Alloc(value * sizeof(Node));
				capacity = value;
			}
			else
			{
				nodes = (Node*)ReAlloc(nodes, value * sizeof(Node));
				capacity = value;
				count = capacity < count ? capacity : count;
			}
		}

		void ShrinkToFit()
		{
			Capacity(count);
		}

		void Clear()
		{
			count = 0;
			AddNode(-1, '\0');
		}

		bool RemoveKey(TextSpan key)
		{
			if (nodes == null || count == 0)
			{
				return false;
			}

			if (!LookupNode(key, out var index, out size_t matchedLength) || matchedLength != key.Length)
			{
				return false;
			}

			while (index != -1)
			{
				Node node = nodes[index];

				if (node.Value.equals(T()) && node.Left == -1 && node.Middle == -1 && node.Right == -1)
				{
					size_t parentIndex = node.ParentIndex;

					if (parentIndex != -1)
					{
						Node* parent = &nodes[parentIndex];
						if (parent->Left == index)
							parent->Left = -1;
						if (parent->Middle == index)
							parent->Middle = -1;
						if (parent->Right == index)
							parent->Right = -1;
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

		void InsertRange(const std::vector<std::pair<TextSpan, T>>& dict)
		{
			for (const auto& pair : dict)
			{
				const TextSpan& key = pair.first;
				const T& value = pair.second;
				Insert(key, value);
			}
		}

		void Insert(TextSpan key, T value)
		{
			size_t index = 0;

			for (char c : key)
			{
				index = InsertChar(index, c);
			}

			(&nodes[index])->Value = value;
		}

		void Insert(const std::string& key, T value)
		{
			size_t index = 0;

			for (char c : key)
			{
				index = InsertChar(index, c);
			}

			(&nodes[index])->Value = value;
		}

		bool MatchLongestPrefix(TextSpan text, T& value, size_t& matchedLength)
		{
			if (count == 1)
			{
				value = T();
				matchedLength = 0;
				return false;
			}

			size_t nodeIndex;
			if (LookupNode(text, nodeIndex, matchedLength))
			{
				auto node = nodes[nodeIndex];
				value = node.Value;
				return true;
			}
			value = T();
			return false;
		}

		T operator[](TextSpan key)
		{
			if (!LookupNode(key, out size_t index, out size_t len) || key.Length != len)
				throw new KeyNotFoundException();
			return nodes[index].Value;
		}
	};
}

#endif // TERNARY_SEARCH_TREE_DICTIONARY_H