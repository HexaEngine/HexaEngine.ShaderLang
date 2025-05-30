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
		ast_ptr<Expression> root;
		ChainExpression* chainExpr;

		template <typename T>
		void Chain(ast_ptr<T> newExpr)
		{
			auto next = newExpr.get();
			if (chainExpr)
			{
				chainExpr->SetNextExpression(std::move(newExpr));
			}
			else
			{
				root = std::move(newExpr);
			}
			chainExpr = next;
		}

		template <typename T>
		void ChainEnd(ast_ptr<T> newExpr)
		{
			Chain(std::move(newExpr));
			parsing = false;
		}

		void ChainOrEnd(ast_ptr<ChainExpression> current)
		{
			auto t = stream.Current();
			if (stream.TryGetOperator(Operator_MemberAccess) || t.isDelimiterOf('['))
			{
				Chain(std::move(current));
				wantsIdentifier = !t.isDelimiterOf('[');
			}
			else
			{
				ChainEnd(std::move(current));
			}
		}

	public:
		MemberPathParser(Parser& parser, TokenStream& stream)
			: parsing(true), wantsIdentifier(true), parser(parser), stream(stream), root(nullptr), chainExpr(nullptr)
		{
		}

		ast_ptr<Expression> TakeRoot()
		{
			return std::move(root);
		}

		Expression* Head() const noexcept { return chainExpr; }

		bool TryParse();
	};
}

#endif