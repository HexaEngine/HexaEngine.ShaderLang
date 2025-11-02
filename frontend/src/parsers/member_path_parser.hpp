#ifndef MEMBER_PATH_PARSER
#define MEMBER_PATH_PARSER

#include "sub_parser.hpp"

namespace HXSL
{
	class MemberPathParser
	{
		bool parsing;
		bool wantsIdentifier;
		Parser& parser;
		TokenStream& stream;
		Expression* root;
		ChainExpression* chainExpr;

		template <typename T>
		void Chain(T* newExpr)
		{
			if (chainExpr)
			{
				chainExpr->SetNextExpression(newExpr);
			}
			else
			{
				root = newExpr;
			}
			chainExpr = newExpr;
		}

		template <typename T>
		void ChainEnd(T* newExpr)
		{
			Chain(newExpr);
			parsing = false;
		}

		void ChainOrEnd(ChainExpression* current)
		{
			auto t = stream.Current();
			if (stream.TryGetOperator(Operator_MemberAccess) || t.isDelimiterOf('['))
			{
				Chain(current);
				wantsIdentifier = !t.isDelimiterOf('[');
			}
			else
			{
				ChainEnd(current);
			}
		}

	public:
		MemberPathParser(Parser& parser, TokenStream& stream)
			: parsing(true), wantsIdentifier(true), parser(parser), stream(stream), root(nullptr), chainExpr(nullptr)
		{
		}

		Expression* TakeRoot()
		{
			return root;
		}

		Expression* Head() const noexcept { return chainExpr; }

		bool TryParse();
	};
}

#endif