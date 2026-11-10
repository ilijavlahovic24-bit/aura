// A Bison parser, made by GNU Bison 3.8.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.





#include "aura_parser.hpp"


// Unqualified %code blocks.
#line 16 "/mnt/d/Luminix/Aura/src/lang/parser.y"

    #include "lang/parser.hpp"
    #include <stdexcept>
    #include <string>

    // Bison podrazumevano stavlja ParserImpl u namespace 'yy'.
    using ParserImpl = yy::ParserImpl;

    static aura::lang::SourceLocation to_src_loc(const ParserImpl::location_type& loc) {
        return {static_cast<uint32_t>(loc.begin.line),
                static_cast<uint32_t>(loc.begin.column)};
    }

    static ParserImpl::location_type to_yy_loc(aura::lang::SourceLocation loc) {
        ParserImpl::location_type l;
        l.begin.line = loc.line;
        l.begin.column = loc.col;
        l.end = l.begin;
        return l;
    }

    static ParserImpl::symbol_type yylex(aura::lang::Lexer& lex) {
        using namespace aura::lang;
        Token t = lex.next();
        auto loc = to_yy_loc(t.loc);
        switch (t.kind) {
            case Tok::END:          return ParserImpl::make_END(loc);
            case Tok::KW_STRATEGY:  return ParserImpl::make_KW_STRATEGY(loc);
            case Tok::KW_PARAM:     return ParserImpl::make_KW_PARAM(loc);
            case Tok::KW_LET:       return ParserImpl::make_KW_LET(loc);
            case Tok::KW_ENTER:     return ParserImpl::make_KW_ENTER(loc);
            case Tok::KW_EXIT:      return ParserImpl::make_KW_EXIT(loc);
            case Tok::KW_LONG:      return ParserImpl::make_KW_LONG(loc);
            case Tok::KW_SHORT:     return ParserImpl::make_KW_SHORT(loc);
            case Tok::KW_WHEN:      return ParserImpl::make_KW_WHEN(loc);
            case Tok::KW_SIZE:      return ParserImpl::make_KW_SIZE(loc);
            case Tok::KW_TRUE:      return ParserImpl::make_KW_TRUE(loc);
            case Tok::KW_FALSE:     return ParserImpl::make_KW_FALSE(loc);
            case Tok::KW_INT:       return ParserImpl::make_KW_INT(loc);
            case Tok::KW_FLOAT:     return ParserImpl::make_KW_FLOAT(loc);
            case Tok::KW_BOOL:      return ParserImpl::make_KW_BOOL(loc);
            case Tok::INT_LIT:      return ParserImpl::make_INT_LIT(t.int_val, loc);
            case Tok::FLOAT_LIT:    return ParserImpl::make_FLOAT_LIT(t.float_val, loc);
            case Tok::IDENT:        return ParserImpl::make_IDENT(t.ident, loc);
            case Tok::PLUS:         return ParserImpl::make_PLUS(loc);
            case Tok::MINUS:        return ParserImpl::make_MINUS(loc);
            case Tok::STAR:         return ParserImpl::make_STAR(loc);
            case Tok::SLASH:        return ParserImpl::make_SLASH(loc);
            case Tok::PERCENT:      return ParserImpl::make_PERCENT(loc);
            case Tok::LT:           return ParserImpl::make_LT(loc);
            case Tok::LE:           return ParserImpl::make_LE(loc);
            case Tok::GT:           return ParserImpl::make_GT(loc);
            case Tok::GE:           return ParserImpl::make_GE(loc);
            case Tok::EQ:           return ParserImpl::make_EQ(loc);
            case Tok::NE:           return ParserImpl::make_NE(loc);
            case Tok::AND:          return ParserImpl::make_AND(loc);
            case Tok::OR:           return ParserImpl::make_OR(loc);
            case Tok::NOT:          return ParserImpl::make_NOT(loc);
            case Tok::ASSIGN:       return ParserImpl::make_ASSIGN(loc);
            case Tok::LPAREN:       return ParserImpl::make_LPAREN(loc);
            case Tok::RPAREN:       return ParserImpl::make_RPAREN(loc);
            case Tok::LBRACE:       return ParserImpl::make_LBRACE(loc);
            case Tok::RBRACE:       return ParserImpl::make_RBRACE(loc);
            case Tok::COMMA:        return ParserImpl::make_COMMA(loc);
            case Tok::SEMI:         return ParserImpl::make_SEMI(loc);
            case Tok::COLON:        return ParserImpl::make_COLON(loc);
            case Tok::LEX_ERROR:    throw std::runtime_error("lexical error at " + std::to_string(t.loc.line) + ":" + std::to_string(t.loc.col));
            default:                return ParserImpl::make_END(loc);
        }
    }

    static aura::lang::ExprPtr make_binary(aura::lang::SourceLocation loc, aura::lang::Tok op, aura::lang::ExprPtr l, aura::lang::ExprPtr r) {
        auto e = std::make_unique<aura::lang::BinaryExpr>();
        e->loc = loc; e->op = op; e->left = std::move(l); e->right = std::move(r);
        return e;
    }
    static aura::lang::ExprPtr make_unary(aura::lang::SourceLocation loc, aura::lang::Tok op, aura::lang::ExprPtr e) {
        auto u = std::make_unique<aura::lang::UnaryExpr>();
        u->loc = loc; u->op = op; u->expr = std::move(e);
        return u;
    }

    void ParserImpl::error(const location_type& loc, const std::string& msg) {
        errors_.push_back(std::to_string(loc.begin.line) + ":" + std::to_string(loc.begin.column) + ": error: " + msg);
    }

    namespace aura::lang {
        Parser::Parser(std::istream& in) : lexer_(&in) {}
        std::unique_ptr<Strategy> Parser::parse() {
            std::unique_ptr<Strategy> result;
            ParserImpl p(lexer_, errors_, result);
            if (p.parse() != 0) return nullptr;
            return result;
        }
    }

#line 143 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YY_USE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

namespace yy {
#line 235 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"

  /// Build a parser object.
   ParserImpl :: ParserImpl  (aura::lang::Lexer& lex_yyarg, std::vector<std::string>& errors__yyarg, std::unique_ptr<aura::lang::Strategy>& result_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      lex (lex_yyarg),
      errors_ (errors__yyarg),
      result (result_yyarg)
  {}

   ParserImpl ::~ ParserImpl  ()
  {}

   ParserImpl ::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/



  // by_state.
   ParserImpl ::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

   ParserImpl ::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
   ParserImpl ::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
   ParserImpl ::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

   ParserImpl ::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

   ParserImpl ::symbol_kind_type
   ParserImpl ::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

   ParserImpl ::stack_symbol_type::stack_symbol_type ()
  {}

   ParserImpl ::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_size_opt: // size_opt
      case symbol_kind::S_expr: // expr
      case symbol_kind::S_or_expr: // or_expr
      case symbol_kind::S_and_expr: // and_expr
      case symbol_kind::S_cmp_expr: // cmp_expr
      case symbol_kind::S_add_expr: // add_expr
      case symbol_kind::S_mul_expr: // mul_expr
      case symbol_kind::S_unary_expr: // unary_expr
      case symbol_kind::S_primary: // primary
        value.YY_MOVE_OR_COPY< aura::lang::ExprPtr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_let_decl: // let_decl
        value.YY_MOVE_OR_COPY< aura::lang::LetDecl > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_param_decl: // param_decl
        value.YY_MOVE_OR_COPY< aura::lang::ParamDecl > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_rule_decl: // rule_decl
        value.YY_MOVE_OR_COPY< aura::lang::RuleDecl > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_strategy: // strategy
        value.YY_MOVE_OR_COPY< aura::lang::Strategy > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FLOAT_LIT: // "float literal"
        value.YY_MOVE_OR_COPY< double > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_INT_LIT: // "integer literal"
        value.YY_MOVE_OR_COPY< int64_t > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENT: // "identifier"
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_arg_list: // arg_list
        value.YY_MOVE_OR_COPY< std::vector<aura::lang::ExprPtr> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_let_list: // let_list
        value.YY_MOVE_OR_COPY< std::vector<aura::lang::LetDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_param_list: // param_list
        value.YY_MOVE_OR_COPY< std::vector<aura::lang::ParamDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_rule_list: // rule_list
        value.YY_MOVE_OR_COPY< std::vector<aura::lang::RuleDecl> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

   ParserImpl ::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_size_opt: // size_opt
      case symbol_kind::S_expr: // expr
      case symbol_kind::S_or_expr: // or_expr
      case symbol_kind::S_and_expr: // and_expr
      case symbol_kind::S_cmp_expr: // cmp_expr
      case symbol_kind::S_add_expr: // add_expr
      case symbol_kind::S_mul_expr: // mul_expr
      case symbol_kind::S_unary_expr: // unary_expr
      case symbol_kind::S_primary: // primary
        value.move< aura::lang::ExprPtr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_let_decl: // let_decl
        value.move< aura::lang::LetDecl > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_param_decl: // param_decl
        value.move< aura::lang::ParamDecl > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_rule_decl: // rule_decl
        value.move< aura::lang::RuleDecl > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_strategy: // strategy
        value.move< aura::lang::Strategy > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FLOAT_LIT: // "float literal"
        value.move< double > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_INT_LIT: // "integer literal"
        value.move< int64_t > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENT: // "identifier"
        value.move< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_arg_list: // arg_list
        value.move< std::vector<aura::lang::ExprPtr> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_let_list: // let_list
        value.move< std::vector<aura::lang::LetDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_param_list: // param_list
        value.move< std::vector<aura::lang::ParamDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_rule_list: // rule_list
        value.move< std::vector<aura::lang::RuleDecl> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
   ParserImpl ::stack_symbol_type&
   ParserImpl ::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_size_opt: // size_opt
      case symbol_kind::S_expr: // expr
      case symbol_kind::S_or_expr: // or_expr
      case symbol_kind::S_and_expr: // and_expr
      case symbol_kind::S_cmp_expr: // cmp_expr
      case symbol_kind::S_add_expr: // add_expr
      case symbol_kind::S_mul_expr: // mul_expr
      case symbol_kind::S_unary_expr: // unary_expr
      case symbol_kind::S_primary: // primary
        value.copy< aura::lang::ExprPtr > (that.value);
        break;

      case symbol_kind::S_let_decl: // let_decl
        value.copy< aura::lang::LetDecl > (that.value);
        break;

      case symbol_kind::S_param_decl: // param_decl
        value.copy< aura::lang::ParamDecl > (that.value);
        break;

      case symbol_kind::S_rule_decl: // rule_decl
        value.copy< aura::lang::RuleDecl > (that.value);
        break;

      case symbol_kind::S_strategy: // strategy
        value.copy< aura::lang::Strategy > (that.value);
        break;

      case symbol_kind::S_FLOAT_LIT: // "float literal"
        value.copy< double > (that.value);
        break;

      case symbol_kind::S_INT_LIT: // "integer literal"
        value.copy< int64_t > (that.value);
        break;

      case symbol_kind::S_IDENT: // "identifier"
        value.copy< std::string > (that.value);
        break;

      case symbol_kind::S_arg_list: // arg_list
        value.copy< std::vector<aura::lang::ExprPtr> > (that.value);
        break;

      case symbol_kind::S_let_list: // let_list
        value.copy< std::vector<aura::lang::LetDecl> > (that.value);
        break;

      case symbol_kind::S_param_list: // param_list
        value.copy< std::vector<aura::lang::ParamDecl> > (that.value);
        break;

      case symbol_kind::S_rule_list: // rule_list
        value.copy< std::vector<aura::lang::RuleDecl> > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

   ParserImpl ::stack_symbol_type&
   ParserImpl ::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_size_opt: // size_opt
      case symbol_kind::S_expr: // expr
      case symbol_kind::S_or_expr: // or_expr
      case symbol_kind::S_and_expr: // and_expr
      case symbol_kind::S_cmp_expr: // cmp_expr
      case symbol_kind::S_add_expr: // add_expr
      case symbol_kind::S_mul_expr: // mul_expr
      case symbol_kind::S_unary_expr: // unary_expr
      case symbol_kind::S_primary: // primary
        value.move< aura::lang::ExprPtr > (that.value);
        break;

      case symbol_kind::S_let_decl: // let_decl
        value.move< aura::lang::LetDecl > (that.value);
        break;

      case symbol_kind::S_param_decl: // param_decl
        value.move< aura::lang::ParamDecl > (that.value);
        break;

      case symbol_kind::S_rule_decl: // rule_decl
        value.move< aura::lang::RuleDecl > (that.value);
        break;

      case symbol_kind::S_strategy: // strategy
        value.move< aura::lang::Strategy > (that.value);
        break;

      case symbol_kind::S_FLOAT_LIT: // "float literal"
        value.move< double > (that.value);
        break;

      case symbol_kind::S_INT_LIT: // "integer literal"
        value.move< int64_t > (that.value);
        break;

      case symbol_kind::S_IDENT: // "identifier"
        value.move< std::string > (that.value);
        break;

      case symbol_kind::S_arg_list: // arg_list
        value.move< std::vector<aura::lang::ExprPtr> > (that.value);
        break;

      case symbol_kind::S_let_list: // let_list
        value.move< std::vector<aura::lang::LetDecl> > (that.value);
        break;

      case symbol_kind::S_param_list: // param_list
        value.move< std::vector<aura::lang::ParamDecl> > (that.value);
        break;

      case symbol_kind::S_rule_list: // rule_list
        value.move< std::vector<aura::lang::RuleDecl> > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
   ParserImpl ::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
   ParserImpl ::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YY_USE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        YY_USE (yykind);
        yyo << ')';
      }
  }
#endif

  void
   ParserImpl ::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
   ParserImpl ::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
   ParserImpl ::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
   ParserImpl ::debug_stream () const
  {
    return *yycdebug_;
  }

  void
   ParserImpl ::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


   ParserImpl ::debug_level_type
   ParserImpl ::debug_level () const
  {
    return yydebug_;
  }

  void
   ParserImpl ::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

   ParserImpl ::state_type
   ParserImpl ::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
   ParserImpl ::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
   ParserImpl ::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
   ParserImpl ::operator() ()
  {
    return parse ();
  }

  int
   ParserImpl ::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex (lex));
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case symbol_kind::S_size_opt: // size_opt
      case symbol_kind::S_expr: // expr
      case symbol_kind::S_or_expr: // or_expr
      case symbol_kind::S_and_expr: // and_expr
      case symbol_kind::S_cmp_expr: // cmp_expr
      case symbol_kind::S_add_expr: // add_expr
      case symbol_kind::S_mul_expr: // mul_expr
      case symbol_kind::S_unary_expr: // unary_expr
      case symbol_kind::S_primary: // primary
        yylhs.value.emplace< aura::lang::ExprPtr > ();
        break;

      case symbol_kind::S_let_decl: // let_decl
        yylhs.value.emplace< aura::lang::LetDecl > ();
        break;

      case symbol_kind::S_param_decl: // param_decl
        yylhs.value.emplace< aura::lang::ParamDecl > ();
        break;

      case symbol_kind::S_rule_decl: // rule_decl
        yylhs.value.emplace< aura::lang::RuleDecl > ();
        break;

      case symbol_kind::S_strategy: // strategy
        yylhs.value.emplace< aura::lang::Strategy > ();
        break;

      case symbol_kind::S_FLOAT_LIT: // "float literal"
        yylhs.value.emplace< double > ();
        break;

      case symbol_kind::S_INT_LIT: // "integer literal"
        yylhs.value.emplace< int64_t > ();
        break;

      case symbol_kind::S_IDENT: // "identifier"
        yylhs.value.emplace< std::string > ();
        break;

      case symbol_kind::S_arg_list: // arg_list
        yylhs.value.emplace< std::vector<aura::lang::ExprPtr> > ();
        break;

      case symbol_kind::S_let_list: // let_list
        yylhs.value.emplace< std::vector<aura::lang::LetDecl> > ();
        break;

      case symbol_kind::S_param_list: // param_list
        yylhs.value.emplace< std::vector<aura::lang::ParamDecl> > ();
        break;

      case symbol_kind::S_rule_list: // rule_list
        yylhs.value.emplace< std::vector<aura::lang::RuleDecl> > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // program: strategy
#line 154 "/mnt/d/Luminix/Aura/src/lang/parser.y"
             { result = std::make_unique<aura::lang::Strategy>(std::move(yystack_[0].value.as < aura::lang::Strategy > ())); }
#line 911 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 3: // strategy: "strategy" "identifier" "{" param_list let_list rule_list size_opt "}"
#line 158 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                                                                           {
        yylhs.value.as < aura::lang::Strategy > ().name = yystack_[6].value.as < std::string > ();
        yylhs.value.as < aura::lang::Strategy > ().params = std::move(yystack_[4].value.as < std::vector<aura::lang::ParamDecl> > ());
        yylhs.value.as < aura::lang::Strategy > ().lets = std::move(yystack_[3].value.as < std::vector<aura::lang::LetDecl> > ());
        yylhs.value.as < aura::lang::Strategy > ().rules = std::move(yystack_[2].value.as < std::vector<aura::lang::RuleDecl> > ());
        yylhs.value.as < aura::lang::Strategy > ().size = std::move(yystack_[1].value.as < aura::lang::ExprPtr > ());
        yylhs.value.as < aura::lang::Strategy > ().loc = to_src_loc(yystack_[7].location);
    }
#line 924 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 4: // strategy: "strategy" error "}"
#line 166 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                               { yyerrok; }
#line 930 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 5: // param_list: %empty
#line 170 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                { yylhs.value.as < std::vector<aura::lang::ParamDecl> > () = std::vector<aura::lang::ParamDecl>(); }
#line 936 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 6: // param_list: param_list param_decl
#line 171 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                            { yylhs.value.as < std::vector<aura::lang::ParamDecl> > () = std::move(yystack_[1].value.as < std::vector<aura::lang::ParamDecl> > ()); yylhs.value.as < std::vector<aura::lang::ParamDecl> > ().push_back(std::move(yystack_[0].value.as < aura::lang::ParamDecl > ())); }
#line 942 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 7: // param_decl: "param" "identifier" ":" "int" "=" expr ";"
#line 175 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                                                 {
        yylhs.value.as < aura::lang::ParamDecl > ().name = yystack_[5].value.as < std::string > (); yylhs.value.as < aura::lang::ParamDecl > ().type = aura::lang::Tok::KW_INT; yylhs.value.as < aura::lang::ParamDecl > ().init = std::move(yystack_[1].value.as < aura::lang::ExprPtr > ()); yylhs.value.as < aura::lang::ParamDecl > ().loc = to_src_loc(yystack_[6].location);
    }
#line 950 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 8: // param_decl: "param" "identifier" ":" "float" "=" expr ";"
#line 178 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                                                     {
        yylhs.value.as < aura::lang::ParamDecl > ().name = yystack_[5].value.as < std::string > (); yylhs.value.as < aura::lang::ParamDecl > ().type = aura::lang::Tok::KW_FLOAT; yylhs.value.as < aura::lang::ParamDecl > ().init = std::move(yystack_[1].value.as < aura::lang::ExprPtr > ()); yylhs.value.as < aura::lang::ParamDecl > ().loc = to_src_loc(yystack_[6].location);
    }
#line 958 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 9: // param_decl: "param" error ";"
#line 181 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                          { yyerrok; }
#line 964 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 10: // let_list: %empty
#line 185 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                { yylhs.value.as < std::vector<aura::lang::LetDecl> > () = std::vector<aura::lang::LetDecl>(); }
#line 970 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 11: // let_list: let_list let_decl
#line 186 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                        { yylhs.value.as < std::vector<aura::lang::LetDecl> > () = std::move(yystack_[1].value.as < std::vector<aura::lang::LetDecl> > ()); yylhs.value.as < std::vector<aura::lang::LetDecl> > ().push_back(std::move(yystack_[0].value.as < aura::lang::LetDecl > ())); }
#line 976 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 12: // let_decl: "let" "identifier" "=" expr ";"
#line 190 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                                  {
        yylhs.value.as < aura::lang::LetDecl > ().name = yystack_[3].value.as < std::string > (); yylhs.value.as < aura::lang::LetDecl > ().expr = std::move(yystack_[1].value.as < aura::lang::ExprPtr > ()); yylhs.value.as < aura::lang::LetDecl > ().loc = to_src_loc(yystack_[4].location);
    }
#line 984 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 13: // let_decl: "let" error ";"
#line 193 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                        { yyerrok; }
#line 990 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 14: // rule_list: %empty
#line 197 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                { yylhs.value.as < std::vector<aura::lang::RuleDecl> > () = std::vector<aura::lang::RuleDecl>(); }
#line 996 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 15: // rule_list: rule_list rule_decl
#line 198 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                          { yylhs.value.as < std::vector<aura::lang::RuleDecl> > () = std::move(yystack_[1].value.as < std::vector<aura::lang::RuleDecl> > ()); yylhs.value.as < std::vector<aura::lang::RuleDecl> > ().push_back(std::move(yystack_[0].value.as < aura::lang::RuleDecl > ())); }
#line 1002 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 16: // rule_decl: "enter" "long" "when" expr ";"
#line 202 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                                       { yylhs.value.as < aura::lang::RuleDecl > ().is_enter = true; yylhs.value.as < aura::lang::RuleDecl > ().is_long = true; yylhs.value.as < aura::lang::RuleDecl > ().cond = std::move(yystack_[1].value.as < aura::lang::ExprPtr > ()); yylhs.value.as < aura::lang::RuleDecl > ().loc = to_src_loc(yystack_[4].location); }
#line 1008 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 17: // rule_decl: "enter" "short" "when" expr ";"
#line 203 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                                          { yylhs.value.as < aura::lang::RuleDecl > ().is_enter = true; yylhs.value.as < aura::lang::RuleDecl > ().is_long = false; yylhs.value.as < aura::lang::RuleDecl > ().cond = std::move(yystack_[1].value.as < aura::lang::ExprPtr > ()); yylhs.value.as < aura::lang::RuleDecl > ().loc = to_src_loc(yystack_[4].location); }
#line 1014 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 18: // rule_decl: "exit" "long" "when" expr ";"
#line 204 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                                        { yylhs.value.as < aura::lang::RuleDecl > ().is_enter = false; yylhs.value.as < aura::lang::RuleDecl > ().is_long = true; yylhs.value.as < aura::lang::RuleDecl > ().cond = std::move(yystack_[1].value.as < aura::lang::ExprPtr > ()); yylhs.value.as < aura::lang::RuleDecl > ().loc = to_src_loc(yystack_[4].location); }
#line 1020 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 19: // rule_decl: "exit" "short" "when" expr ";"
#line 205 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                                         { yylhs.value.as < aura::lang::RuleDecl > ().is_enter = false; yylhs.value.as < aura::lang::RuleDecl > ().is_long = false; yylhs.value.as < aura::lang::RuleDecl > ().cond = std::move(yystack_[1].value.as < aura::lang::ExprPtr > ()); yylhs.value.as < aura::lang::RuleDecl > ().loc = to_src_loc(yystack_[4].location); }
#line 1026 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 20: // rule_decl: "enter" error ";"
#line 206 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                          { yyerrok; }
#line 1032 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 21: // rule_decl: "exit" error ";"
#line 207 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                          { yyerrok; }
#line 1038 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 22: // size_opt: %empty
#line 211 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                { yylhs.value.as < aura::lang::ExprPtr > () = nullptr; }
#line 1044 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 23: // size_opt: "size" "=" expr ";"
#line 212 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                               { yylhs.value.as < aura::lang::ExprPtr > () = std::move(yystack_[1].value.as < aura::lang::ExprPtr > ()); }
#line 1050 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 24: // expr: or_expr
#line 216 "/mnt/d/Luminix/Aura/src/lang/parser.y"
            { yylhs.value.as < aura::lang::ExprPtr > () = std::move(yystack_[0].value.as < aura::lang::ExprPtr > ()); }
#line 1056 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 25: // or_expr: and_expr
#line 220 "/mnt/d/Luminix/Aura/src/lang/parser.y"
             { yylhs.value.as < aura::lang::ExprPtr > () = std::move(yystack_[0].value.as < aura::lang::ExprPtr > ()); }
#line 1062 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 26: // or_expr: or_expr "||" and_expr
#line 221 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                          { yylhs.value.as < aura::lang::ExprPtr > () = make_binary(to_src_loc(yystack_[1].location), aura::lang::Tok::OR, std::move(yystack_[2].value.as < aura::lang::ExprPtr > ()), std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1068 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 27: // and_expr: cmp_expr
#line 225 "/mnt/d/Luminix/Aura/src/lang/parser.y"
             { yylhs.value.as < aura::lang::ExprPtr > () = std::move(yystack_[0].value.as < aura::lang::ExprPtr > ()); }
#line 1074 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 28: // and_expr: and_expr "&&" cmp_expr
#line 226 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                            { yylhs.value.as < aura::lang::ExprPtr > () = make_binary(to_src_loc(yystack_[1].location), aura::lang::Tok::AND, std::move(yystack_[2].value.as < aura::lang::ExprPtr > ()), std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1080 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 29: // cmp_expr: add_expr
#line 230 "/mnt/d/Luminix/Aura/src/lang/parser.y"
             { yylhs.value.as < aura::lang::ExprPtr > () = std::move(yystack_[0].value.as < aura::lang::ExprPtr > ()); }
#line 1086 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 30: // cmp_expr: add_expr "<" add_expr
#line 231 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                           { yylhs.value.as < aura::lang::ExprPtr > () = make_binary(to_src_loc(yystack_[1].location), aura::lang::Tok::LT, std::move(yystack_[2].value.as < aura::lang::ExprPtr > ()), std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1092 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 31: // cmp_expr: add_expr "<=" add_expr
#line 232 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                           { yylhs.value.as < aura::lang::ExprPtr > () = make_binary(to_src_loc(yystack_[1].location), aura::lang::Tok::LE, std::move(yystack_[2].value.as < aura::lang::ExprPtr > ()), std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1098 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 32: // cmp_expr: add_expr ">" add_expr
#line 233 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                           { yylhs.value.as < aura::lang::ExprPtr > () = make_binary(to_src_loc(yystack_[1].location), aura::lang::Tok::GT, std::move(yystack_[2].value.as < aura::lang::ExprPtr > ()), std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1104 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 33: // cmp_expr: add_expr ">=" add_expr
#line 234 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                           { yylhs.value.as < aura::lang::ExprPtr > () = make_binary(to_src_loc(yystack_[1].location), aura::lang::Tok::GE, std::move(yystack_[2].value.as < aura::lang::ExprPtr > ()), std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1110 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 34: // cmp_expr: add_expr "==" add_expr
#line 235 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                           { yylhs.value.as < aura::lang::ExprPtr > () = make_binary(to_src_loc(yystack_[1].location), aura::lang::Tok::EQ, std::move(yystack_[2].value.as < aura::lang::ExprPtr > ()), std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1116 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 35: // cmp_expr: add_expr "!=" add_expr
#line 236 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                           { yylhs.value.as < aura::lang::ExprPtr > () = make_binary(to_src_loc(yystack_[1].location), aura::lang::Tok::NE, std::move(yystack_[2].value.as < aura::lang::ExprPtr > ()), std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1122 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 36: // add_expr: mul_expr
#line 240 "/mnt/d/Luminix/Aura/src/lang/parser.y"
             { yylhs.value.as < aura::lang::ExprPtr > () = std::move(yystack_[0].value.as < aura::lang::ExprPtr > ()); }
#line 1128 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 37: // add_expr: add_expr "+" mul_expr
#line 241 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                             { yylhs.value.as < aura::lang::ExprPtr > () = make_binary(to_src_loc(yystack_[1].location), aura::lang::Tok::PLUS, std::move(yystack_[2].value.as < aura::lang::ExprPtr > ()), std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1134 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 38: // add_expr: add_expr "-" mul_expr
#line 242 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                              { yylhs.value.as < aura::lang::ExprPtr > () = make_binary(to_src_loc(yystack_[1].location), aura::lang::Tok::MINUS, std::move(yystack_[2].value.as < aura::lang::ExprPtr > ()), std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1140 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 39: // mul_expr: unary_expr
#line 246 "/mnt/d/Luminix/Aura/src/lang/parser.y"
               { yylhs.value.as < aura::lang::ExprPtr > () = std::move(yystack_[0].value.as < aura::lang::ExprPtr > ()); }
#line 1146 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 40: // mul_expr: mul_expr "*" unary_expr
#line 247 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                               { yylhs.value.as < aura::lang::ExprPtr > () = make_binary(to_src_loc(yystack_[1].location), aura::lang::Tok::STAR, std::move(yystack_[2].value.as < aura::lang::ExprPtr > ()), std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1152 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 41: // mul_expr: mul_expr "/" unary_expr
#line 248 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                                { yylhs.value.as < aura::lang::ExprPtr > () = make_binary(to_src_loc(yystack_[1].location), aura::lang::Tok::SLASH, std::move(yystack_[2].value.as < aura::lang::ExprPtr > ()), std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1158 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 42: // mul_expr: mul_expr "%" unary_expr
#line 249 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                                  { yylhs.value.as < aura::lang::ExprPtr > () = make_binary(to_src_loc(yystack_[1].location), aura::lang::Tok::PERCENT, std::move(yystack_[2].value.as < aura::lang::ExprPtr > ()), std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1164 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 43: // unary_expr: primary
#line 253 "/mnt/d/Luminix/Aura/src/lang/parser.y"
            { yylhs.value.as < aura::lang::ExprPtr > () = std::move(yystack_[0].value.as < aura::lang::ExprPtr > ()); }
#line 1170 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 44: // unary_expr: "-" unary_expr
#line 254 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                                    { yylhs.value.as < aura::lang::ExprPtr > () = make_unary(to_src_loc(yystack_[1].location), aura::lang::Tok::MINUS, std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1176 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 45: // unary_expr: "!" unary_expr
#line 255 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                     { yylhs.value.as < aura::lang::ExprPtr > () = make_unary(to_src_loc(yystack_[1].location), aura::lang::Tok::NOT, std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1182 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 46: // primary: "integer literal"
#line 259 "/mnt/d/Luminix/Aura/src/lang/parser.y"
            { auto e = std::make_unique<aura::lang::IntExpr>(); e->loc = to_src_loc(yystack_[0].location); e->value = yystack_[0].value.as < int64_t > (); yylhs.value.as < aura::lang::ExprPtr > () = std::move(e); }
#line 1188 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 47: // primary: "float literal"
#line 260 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                { auto e = std::make_unique<aura::lang::FloatExpr>(); e->loc = to_src_loc(yystack_[0].location); e->value = yystack_[0].value.as < double > (); yylhs.value.as < aura::lang::ExprPtr > () = std::move(e); }
#line 1194 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 48: // primary: "true"
#line 261 "/mnt/d/Luminix/Aura/src/lang/parser.y"
              { auto e = std::make_unique<aura::lang::BoolExpr>(); e->loc = to_src_loc(yystack_[0].location); e->value = true; yylhs.value.as < aura::lang::ExprPtr > () = std::move(e); }
#line 1200 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 49: // primary: "false"
#line 262 "/mnt/d/Luminix/Aura/src/lang/parser.y"
               { auto e = std::make_unique<aura::lang::BoolExpr>(); e->loc = to_src_loc(yystack_[0].location); e->value = false; yylhs.value.as < aura::lang::ExprPtr > () = std::move(e); }
#line 1206 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 50: // primary: "identifier"
#line 263 "/mnt/d/Luminix/Aura/src/lang/parser.y"
            { auto e = std::make_unique<aura::lang::IdentExpr>(); e->loc = to_src_loc(yystack_[0].location); e->name = yystack_[0].value.as < std::string > (); yylhs.value.as < aura::lang::ExprPtr > () = std::move(e); }
#line 1212 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 51: // primary: "identifier" "(" arg_list ")"
#line 264 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                                   {
        auto e = std::make_unique<aura::lang::CallExpr>();
        e->loc = to_src_loc(yystack_[3].location); e->name = yystack_[3].value.as < std::string > (); e->args = std::move(yystack_[1].value.as < std::vector<aura::lang::ExprPtr> > ()); yylhs.value.as < aura::lang::ExprPtr > () = std::move(e);
    }
#line 1221 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 52: // primary: "(" expr ")"
#line 268 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                         { yylhs.value.as < aura::lang::ExprPtr > () = std::move(yystack_[1].value.as < aura::lang::ExprPtr > ()); }
#line 1227 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 53: // arg_list: %empty
#line 272 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                { yylhs.value.as < std::vector<aura::lang::ExprPtr> > () = std::vector<aura::lang::ExprPtr>(); }
#line 1233 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 54: // arg_list: expr
#line 273 "/mnt/d/Luminix/Aura/src/lang/parser.y"
           { yylhs.value.as < std::vector<aura::lang::ExprPtr> > () = std::vector<aura::lang::ExprPtr>(); yylhs.value.as < std::vector<aura::lang::ExprPtr> > ().push_back(std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1239 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;

  case 55: // arg_list: arg_list "," expr
#line 274 "/mnt/d/Luminix/Aura/src/lang/parser.y"
                          { yylhs.value.as < std::vector<aura::lang::ExprPtr> > () = std::move(yystack_[2].value.as < std::vector<aura::lang::ExprPtr> > ()); yylhs.value.as < std::vector<aura::lang::ExprPtr> > ().push_back(std::move(yystack_[0].value.as < aura::lang::ExprPtr > ())); }
#line 1245 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"
    break;


#line 1249 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
   ParserImpl ::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  const char *
   ParserImpl ::symbol_name (symbol_kind_type yysymbol)
  {
    static const char *const yy_sname[] =
    {
    "end of file", "error", "invalid token", "strategy", "param", "let",
  "enter", "exit", "long", "short", "when", "size", "true", "false", "int",
  "float", "bool", "integer literal", "float literal", "identifier", "+",
  "-", "*", "/", "%", "<", "<=", ">", ">=", "==", "!=", "&&", "||", "!",
  "=", "(", ")", "{", "}", ",", ";", ":", "UMINUS", "$accept", "program",
  "strategy", "param_list", "param_decl", "let_list", "let_decl",
  "rule_list", "rule_decl", "size_opt", "expr", "or_expr", "and_expr",
  "cmp_expr", "add_expr", "mul_expr", "unary_expr", "primary", "arg_list", YY_NULLPTR
    };
    return yy_sname[yysymbol];
  }



  //  ParserImpl ::context.
   ParserImpl ::context::context (const  ParserImpl & yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
   ParserImpl ::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

    const int yyn = yypact_[+yyparser_.yystack_[0].state];
    if (!yy_pact_value_is_default_ (yyn))
      {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  In other words, skip the first -YYN actions for
           this state because they are default actions.  */
        const int yyxbegin = yyn < 0 ? -yyn : 0;
        // Stay within bounds of both yycheck and yytname.
        const int yychecklim = yylast_ - yyn + 1;
        const int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != symbol_kind::S_YYerror
              && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
            {
              if (!yyarg)
                ++yycount;
              else if (yycount == yyargn)
                return 0;
              else
                yyarg[yycount++] = YY_CAST (symbol_kind_type, yyx);
            }
      }

    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }






  int
   ParserImpl ::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
   ParserImpl ::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char  ParserImpl ::yypact_ninf_ = -43;

  const signed char  ParserImpl ::yytable_ninf_ = -1;

  const signed char
   ParserImpl ::yypact_[] =
  {
       3,     0,    26,   -43,    27,    29,   -43,   -43,   -43,    60,
       8,   -43,    62,    28,    30,    14,   -43,    25,   -43,    20,
      32,    36,     9,    47,    39,   -43,    31,    42,    43,   -43,
      -5,    34,    68,    69,    41,    70,    72,    -5,   -43,    -5,
      -5,   -43,   -43,   -43,   -43,    48,    -5,    -5,    -5,    44,
      53,    55,   -43,    17,    35,   -43,   -43,   -43,    -5,    -5,
     -43,    -5,    -5,    49,    50,    51,    -5,   -43,   -43,    52,
     -43,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,    54,    56,    57,    58,   -43,   -43,
     -43,   -43,   -16,   -43,    55,   -43,    35,    35,    40,    40,
      40,    40,    40,    40,   -43,   -43,   -43,   -43,   -43,   -43,
     -43,   -43,    -5,   -43
  };

  const signed char
   ParserImpl ::yydefact_[] =
  {
       0,     0,     0,     2,     0,     0,     1,     4,     5,    10,
       0,     6,    14,     0,     0,     0,    11,    22,     9,     0,
       0,     0,     0,     0,     0,    15,     0,     0,     0,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     3,     0,
       0,    48,    49,    46,    47,    50,     0,     0,     0,     0,
      24,    25,    27,    29,    36,    39,    43,    20,     0,     0,
      21,     0,     0,     0,     0,     0,    53,    44,    45,     0,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    23,     7,
       8,    54,     0,    52,    26,    28,    37,    38,    30,    31,
      32,    33,    34,    35,    40,    41,    42,    16,    17,    18,
      19,    51,     0,    55
  };

  const signed char
   ParserImpl ::yypgoto_[] =
  {
     -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,   -43,
     -37,   -43,    16,    21,   -26,   -11,   -42,   -43,   -43
  };

  const signed char
   ParserImpl ::yydefgoto_[] =
  {
       0,     2,     3,     9,    11,    12,    16,    17,    25,    26,
      49,    50,    51,    52,    53,    54,    55,    56,    92
  };

  const signed char
   ParserImpl ::yytable_[] =
  {
      63,     4,    64,    65,    67,    68,     1,    41,    42,    13,
      31,    69,    43,    44,    45,    20,    46,    32,    33,     5,
     111,    84,    85,   112,    86,    87,     6,    14,    47,    91,
      48,    22,    23,    21,    27,    28,    24,    73,    74,   104,
     105,   106,    75,    76,    77,    78,    79,    80,    34,    98,
      99,   100,   101,   102,   103,    35,    36,    81,    82,    83,
      73,    74,    96,    97,    10,     7,     8,    15,    18,    38,
      30,    19,    29,    37,    57,   113,    39,    40,    58,    59,
      61,    60,    62,    66,    70,    71,    72,    94,    93,    88,
      89,    90,     0,    95,   107,     0,   108,   109,   110
  };

  const signed char
   ParserImpl ::yycheck_[] =
  {
      37,     1,    39,    40,    46,    47,     3,    12,    13,     1,
       1,    48,    17,    18,    19,     1,    21,     8,     9,    19,
      36,    58,    59,    39,    61,    62,     0,    19,    33,    66,
      35,     6,     7,    19,    14,    15,    11,    20,    21,    81,
      82,    83,    25,    26,    27,    28,    29,    30,     1,    75,
      76,    77,    78,    79,    80,     8,     9,    22,    23,    24,
      20,    21,    73,    74,     4,    38,    37,     5,    40,    38,
      34,    41,    40,    34,    40,   112,    34,    34,    10,    10,
      10,    40,    10,    35,    40,    32,    31,    71,    36,    40,
      40,    40,    -1,    72,    40,    -1,    40,    40,    40
  };

  const signed char
   ParserImpl ::yystos_[] =
  {
       0,     3,    44,    45,     1,    19,     0,    38,    37,    46,
       4,    47,    48,     1,    19,     5,    49,    50,    40,    41,
       1,    19,     6,     7,    11,    51,    52,    14,    15,    40,
      34,     1,     8,     9,     1,     8,     9,    34,    38,    34,
      34,    12,    13,    17,    18,    19,    21,    33,    35,    53,
      54,    55,    56,    57,    58,    59,    60,    40,    10,    10,
      40,    10,    10,    53,    53,    53,    35,    59,    59,    53,
      40,    32,    31,    20,    21,    25,    26,    27,    28,    29,
      30,    22,    23,    24,    53,    53,    53,    53,    40,    40,
      40,    53,    61,    36,    55,    56,    58,    58,    57,    57,
      57,    57,    57,    57,    59,    59,    59,    40,    40,    40,
      40,    36,    39,    53
  };

  const signed char
   ParserImpl ::yyr1_[] =
  {
       0,    43,    44,    45,    45,    46,    46,    47,    47,    47,
      48,    48,    49,    49,    50,    50,    51,    51,    51,    51,
      51,    51,    52,    52,    53,    54,    54,    55,    55,    56,
      56,    56,    56,    56,    56,    56,    57,    57,    57,    58,
      58,    58,    58,    59,    59,    59,    60,    60,    60,    60,
      60,    60,    60,    61,    61,    61
  };

  const signed char
   ParserImpl ::yyr2_[] =
  {
       0,     2,     1,     8,     3,     0,     2,     7,     7,     3,
       0,     2,     5,     3,     0,     2,     5,     5,     5,     5,
       3,     3,     0,     4,     1,     1,     3,     1,     3,     1,
       3,     3,     3,     3,     3,     3,     1,     3,     3,     1,
       3,     3,     3,     1,     2,     2,     1,     1,     1,     1,
       1,     4,     3,     0,     1,     3
  };




#if YYDEBUG
  const short
   ParserImpl ::yyrline_[] =
  {
       0,   154,   154,   158,   166,   170,   171,   175,   178,   181,
     185,   186,   190,   193,   197,   198,   202,   203,   204,   205,
     206,   207,   211,   212,   216,   220,   221,   225,   226,   230,
     231,   232,   233,   234,   235,   236,   240,   241,   242,   246,
     247,   248,   249,   253,   254,   255,   259,   260,   261,   262,
     263,   264,   268,   272,   273,   274
  };

  void
   ParserImpl ::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
   ParserImpl ::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


} // yy
#line 1737 "/mnt/d/Luminix/Aura/build-asan/aura_parser.cpp"

#line 277 "/mnt/d/Luminix/Aura/src/lang/parser.y"
