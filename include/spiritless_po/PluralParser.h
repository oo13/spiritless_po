/* PluralParser.h

Copyright (c) 2019 OOTA, Masato

spiritless_po is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

spiritless_po is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef SRIRITLESS_PO_PLURAL_PARSER_H_
#define SRIRITLESS_PO_PLURAL_PARSER_H_

#include <algorithm>
#include <cctype>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <string>

namespace spiritless_po {
	// This is a parser of the plural expression.
	// This parser is an LL(1) parser.
	namespace PluralParser {
		// Integer type for plural forms.
		typedef unsigned long int NumT;
		
		// The type of a plural form function.
		typedef std::function<NumT(NumT)> FunctionType;
		
		// The type of intermediate functions. For internal used.
		// First parameter is the value of variable n.
		// Second parameter is the top of stack.
		typedef std::function<NumT(NumT, NumT)> ExpFuncType;
		
		// Iterator type.
		typedef std::string::const_iterator InP;
		
		// The type of exception when raised by a parse error.
		class ExpressionError : public std::runtime_error {
		public:
			explicit ExpressionError(const std::string &whatArg, const InP &it);
			explicit ExpressionError(const char *whatArg, const InP &it);
			
			const InP &Where() const noexcept;
			
		private:
			InP pos;
		};
		
		
		
		inline ExpressionError::ExpressionError(const std::string &whatArg, const InP &it)
			: std::runtime_error(whatArg), pos(it)
		{
		}
		
		ExpressionError::ExpressionError(const char *whatArg, const InP &it)
			: std::runtime_error(whatArg), pos(it)
		{
		}
		
		const InP &ExpressionError::Where() const noexcept
		{
			return pos;
		}
		
		
		
		// Forward declarations.
		NumT GetNumber(InP &it, const InP &end);
		std::pair<InP, InP> GetExpression(const InP &it, const InP &end, const std::string &keyword);
		FunctionType ParseExpression(InP &it, const InP &end);
		
		// Plural forms parser.
		inline std::pair<NumT, FunctionType> Parse(const std::string &plural_exp)
		{
			const InP begin = plural_exp.cbegin();
			const InP end = plural_exp.cend();
			auto npluralsRange = GetExpression(begin, end, "nplurals");
			const NumT nplurals = GetNumber(npluralsRange.first, npluralsRange.second);
			
			auto pluralRange = GetExpression(begin, end, "plural");
			const auto f = ParseExpression(pluralRange.first, pluralRange.second);
			
			return std::make_pair(nplurals, f);
		}
		
		
		
		// Skip spaces (Utility function)
		inline void SkipSpaces(InP &it, const InP &end)
		{
			while(it != end && std::isspace(static_cast<unsigned char>(*it)))
				++it;
		}
		
		
		
		// get a number. (Utility function)
		inline NumT GetNumber(InP &it, const InP &end)
		{
			std::string s;
			while(it != end && std::isdigit(static_cast<unsigned char>(*it)))
				s += *it++;
			if(!s.empty())
				return std::stoi(s);
			throw ExpressionError("Parse error: '0'..'9' is expected.", it);
		}
		
		
		
		// get a expression. (Utility function)
		inline std::pair<InP, InP> GetExpression(const InP &it, const InP &end, const std::string &keyword)
		{
			auto curIt = std::find_end(it, end, keyword.cbegin(), keyword.cend());
			if(curIt == end)
				throw ExpressionError("Parse error: '" + keyword + "' is not found.", it);
			std::advance(curIt, keyword.length());
			SkipSpaces(curIt, end);
			if(*curIt != '=')
				throw ExpressionError("'=' is expected.", curIt);
			++curIt;
			SkipSpaces(curIt, end);
			const InP begin = curIt;
			while(curIt != end && *curIt != ';')
				++curIt;
			if(*curIt != ';')
				throw ExpressionError("';' is expected.", curIt);
			return std::make_pair(begin, curIt);
		}
		
		
		
		// Forward declarations.
		ExpFuncType ParseTerm7(InP &it, const InP &end);
		ExpFuncType ParseTerm71(InP &it, const InP &end);
		ExpFuncType ParseTerm6(InP &it, const InP &end);
		ExpFuncType ParseTerm61(InP &it, const InP &end);
		ExpFuncType ParseTerm5(InP &it, const InP &end);
		ExpFuncType ParseTerm51(InP &it, const InP &end);
		ExpFuncType ParseTerm4(InP &it, const InP &end);
		ExpFuncType ParseTerm41(InP &it, const InP &end);
		ExpFuncType ParseTerm3(InP &it, const InP &end);
		ExpFuncType ParseTerm31(InP &it, const InP &end);
		ExpFuncType ParseTerm2(InP &it, const InP &end);
		ExpFuncType ParseTerm21(InP &it, const InP &end);
		ExpFuncType ParseTerm1(InP &it, const InP &end);
		ExpFuncType ParseTerm11(InP &it, const InP &end);
		ExpFuncType ParseTerm0(InP &it, const InP &end);
		ExpFuncType ParseValue(InP &it, const InP &end);
		
		// This is a parser of the plural expression, and returns the decision function.
		// InP is an input iterator type.
		// start = term7;
		inline FunctionType ParseExpression(InP &it, const InP &end)
		{
			auto term7F = ParseTerm7(it, end);
			SkipSpaces(it, end);
			if(it == end)
				return [=](NumT n){ return term7F(n, 0); };
			else
				throw ExpressionError("Parse error: Invalid character is detected.", it);
		}
		
		
		
		// Lower level parsers.
		// term7 = term6, term71;
		inline ExpFuncType ParseTerm7(InP &it, const InP &end)
		{
			auto term6F = ParseTerm6(it, end);
			auto term71F = ParseTerm71(it, end);
			return [=](NumT n, NumT x){ return term71F(n, term6F(n, 0)); };
		}
		
		
		
		// term71 = e | '?', term7, ':', term7;
		inline ExpFuncType ParseTerm71(InP &it, const InP &end)
		{
			SkipSpaces(it, end);
			if(it != end && *it == '?')
			{
				++it;
				auto term7TrueF = ParseTerm7(it, end);
				SkipSpaces(it, end);
				if(it != end && *it == ':')
				{
					++it;
					auto term7FalseF = ParseTerm7(it, end);
					return [=](NumT n, NumT x){ return x ? term7TrueF(n, 0) : term7FalseF(n, 0); };
				} else
					throw ExpressionError("Parse error: ':' is expected.", it);
			}
			return [=](NumT n, NumT x){ return x; };
		}
		
		
		
		// term6 = term5, term61;
		inline ExpFuncType ParseTerm6(InP &it, const InP &end)
		{
			auto term5F = ParseTerm5(it, end);
			auto term61F = ParseTerm61(it, end);
			return [=](NumT n, NumT x){ return term61F(n, term5F(n, 0)); };
		}
		
		
		
		// term61 = e | '||', term6;
		inline ExpFuncType ParseTerm61(InP &it, const InP &end)
		{
			SkipSpaces(it, end);
			if(it != end && *it == '|')
			{
				++it;
				if(it != end && *it == '|')
				{
					++it;
					auto term6F = ParseTerm6(it, end);
					return [=](NumT n, NumT x){ return x || term6F(n, 0); };
				}
				else
					throw ExpressionError("Parse error: '|' is expected.", it);
			}
			return [=](NumT n, NumT x){ return x; };
		}
		
		
		
		// term5 = term4, term51;
		inline ExpFuncType ParseTerm5(InP &it, const InP &end)
		{
			auto term4F = ParseTerm4(it, end);
			auto term51F = ParseTerm51(it, end);
			return [=](NumT n, NumT x){ return term51F(n, term4F(n, 0)); };
		}
		
		
		
		// term51 = e | '&&', term5;
		inline ExpFuncType ParseTerm51(InP &it, const InP &end)
		{
			SkipSpaces(it, end);
			if(it != end && *it == '&')
			{
				++it;
				if(it != end && *it == '&')
				{
					++it;
					auto term5F = ParseTerm5(it, end);
					return [=](NumT n, NumT x){ return x && term5F(n, 0); };
				}
				else
					throw ExpressionError("Parse error: '&' is expected.", it);
			}
			return [=](NumT n, NumT x){ return x; };
		}
		
		
		
		// term4 = term3, term41;
		inline ExpFuncType ParseTerm4(InP &it, const InP &end)
		{
			auto term3F = ParseTerm3(it, end);
			auto term41F = ParseTerm41(it, end);
			return [=](NumT n, NumT x){ return term41F(n, term3F(n, 0)); };
		}
		
		
		
		// term41 = e | '==', term4 | '!=', term4;
		inline ExpFuncType ParseTerm41(InP &it, const InP &end)
		{
			SkipSpaces(it, end);
			if(it != end && (*it == '=' || *it == '!'))
			{
				const bool eq = *it == '=';
				++it;
				if(it != end && *it == '=')
				{
					++it;
					auto term4F = ParseTerm4(it, end);
					if(eq)
						return [=](NumT n, NumT x){ return x == term4F(n, 0); };
					else
						return [=](NumT n, NumT x){ return x != term4F(n, 0); };
				}
				else
					throw ExpressionError("Parse error: '=' is expected.", it);
			}
			return [=](NumT n, NumT x){ return x; };
		}
		
		
		
		// term3 = term2, term31;
		inline ExpFuncType ParseTerm3(InP &it, const InP &end)
		{
			auto term2F = ParseTerm2(it, end);
			auto term31F = ParseTerm31(it, end);
			return [=](NumT n, NumT x){ return term31F(n, term2F(n, 0)); };
		}
		
		
		
		// term31 = e | '<', term3 | '<=', term3 | '>', term3 | '<=', term3;
		inline ExpFuncType ParseTerm31(InP &it, const InP &end)
		{
			SkipSpaces(it, end);
			if(it != end && (*it == '<' || *it == '>'))
			{
				const bool lt = *it == '<';
				++it;
				const bool eq = it != end && *it == '=';
				if(eq)
					++it;
				auto term3F = ParseTerm3(it, end);
				if(lt)
				{
					if(eq)
						return [=](NumT n, NumT x){ return x <= term3F(n, 0); };
					else
						return [=](NumT n, NumT x){ return x < term3F(n, 0); };
				} else {
					if(eq)
						return [=](NumT n, NumT x){ return x >= term3F(n, 0); };
					else
						return [=](NumT n, NumT x){ return x > term3F(n, 0); };
				}
			}
			return [=](NumT n, NumT x){ return x; };
		}
		
		
		
		// term2 = term1, term21;
		inline ExpFuncType ParseTerm2(InP &it, const InP &end)
		{
			auto term1F = ParseTerm1(it, end);
			auto term21F = ParseTerm21(it, end);
			return [=](NumT n, NumT x){ return term21F(n, term1F(n, 0)); };
		}
		
		
		
		// term21 = e | '+', term2 | '-', term2;
		inline ExpFuncType ParseTerm21(InP &it, const InP &end)
		{
			SkipSpaces(it, end);
			if(it != end && (*it == '+' || *it == '-'))
			{
				const bool plus = *it == '+';
				++it;
				auto term2F = ParseTerm2(it, end);
				if(plus)
					return [=](NumT n, NumT x){ return x + term2F(n, 0); };
				else
					return [=](NumT n, NumT x){ return x - term2F(n, 0); };
			}
			return [=](NumT n, NumT x){ return x; };
		}
		
		
		
		// term1 = term0, term11;
		inline ExpFuncType ParseTerm1(InP &it, const InP &end)
		{
			auto term0F = ParseTerm0(it, end);
			auto term11F = ParseTerm11(it, end);
			return [=](NumT n, NumT x){ return term11F(n, term0F(n, 0)); };
		}
		
		
		
		// term11 = e | '*', term1 | '/', term1 | '%', term1;
		inline ExpFuncType ParseTerm11(InP &it, const InP &end)
		{
			SkipSpaces(it, end);
			if(it != end && (*it == '*' || *it == '/' || *it == '%'))
			{
				const char op = *it;
				++it;
				auto term1F = ParseTerm1(it, end);
				if(op == '*')
					return [=](NumT n, NumT x){ return x * term1F(n, 0); };
				else if(op == '/')
					return [=](NumT n, NumT x){ return x / term1F(n, 0); };
				else
					return [=](NumT n, NumT x){ return x % term1F(n, 0); };
			}
			return [=](NumT n, NumT x){ return x; };
		}
		
		
		
		// term0 = {'!'} value;
		inline ExpFuncType ParseTerm0(InP &it, const InP &end)
		{
			bool isNot = false;
			for(;;)
			{
				SkipSpaces(it, end);
				if(it != end && *it == '!')
				{
					++it;
					isNot = !isNot;
				}
				else
					break;
			}
			auto valueF = ParseValue(it, end);
			if(isNot)
				return [=](NumT n, NumT x){ return !valueF(n, 0); };
			else
				return [=](NumT n, NumT x){ return valueF(n, 0); };
		}
		
		
		
		// value = 'n' | digit, {digit} | '(', term7, ')';
		// digit = '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9';
		inline ExpFuncType ParseValue(InP &it, const InP &end)
		{
			SkipSpaces(it, end);
			if(it != end)
			{
				if(*it == 'n')
				{
					++it;
					return [=](NumT n, NumT x){ return n; };
				}
				else if(*it == '(')
				{
					++it;
					auto term7F = ParseTerm7(it, end);
					SkipSpaces(it, end);
					if(it != end && *it == ')')
					{
						++it;
						return [=](NumT n, NumT x){ return term7F(n, 0); };
					}
					else
						throw ExpressionError("Parse error: ')' is expected.", it);
				}
				else
				{
					try
					{
						const NumT v = GetNumber(it, end);
						return [=](NumT n, NumT x){ return v; };
					}
					catch (ExpressionError &e)
					{
						// fall throuth
					}
				}
			}
			throw ExpressionError("Parse error: 'n' or '(', '0'..'9' is expected.", it);
		}
	} // namespace PluralParser
} // namespace spiritless_po

#endif // SRIRITLESS_PO_PLURAL_PARSER_H_
