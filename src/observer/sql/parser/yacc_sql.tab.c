/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 2 "yacc_sql.y"


#include "sql/parser/parse_defs.h"
#include "sql/parser/yacc_sql.tab.h"
#include "sql/parser/lex.yy.h"
// #include "common/log/log.h" // 包含C++中的头文件

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct ParserContext {
  Query * ssql;
  size_t condition_length;
  size_t from_length;
  size_t attr_length;
  RelAttr attr[MAX_NUM];
  size_t value_length;
  Value values[MAX_NUM];
  Condition conditions[MAX_NUM];
  CompOp comp;
  char id[MAX_NUM];
  size_t join_condition_length;
  Condition join_conditions[MAX_NUM];
  size_t join_length;
  Join joins[MAX_NUM];
  size_t having_length;
  Condition havings[MAX_NUM];
  size_t subset_num;
  Subset subsets[MAX_NUM];
} ParserContext;

//获取子串
char *substr(const char *s,int n1,int n2)/*从s中提取下标为n1~n2的字符组成一个新字符串，然后返回这个新串的首地址*/
{
  char *sp = malloc(sizeof(char) * (n2 - n1 + 2));
  int i, j = 0;
  for (i = n1; i <= n2; i++) {
    sp[j++] = s[i];
  }
  sp[j] = 0;
  return sp;
}

void yyerror(yyscan_t scanner, const char *str)
{
  ParserContext *context = (ParserContext *)(yyget_extra(scanner));
  query_reset(context->ssql);
  context->ssql->flag = SCF_ERROR;
  context->condition_length = 0;
  context->from_length = 0;
  context->value_length = 0;
  context->join_length = 0;
  context->join_condition_length = 0;
  context->having_length = 0;
  context->ssql->sstr.insertion.value_num = 0;
  context->attr_length = 0;
  context->subset_num = 0;
  printf("parse sql failed. error=%s", str);
}

ParserContext *get_context(yyscan_t scanner)
{
  return (ParserContext *)yyget_extra(scanner);
}

#define CONTEXT get_context(scanner)


#line 141 "yacc_sql.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "yacc_sql.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_SEMICOLON = 3,                  /* SEMICOLON  */
  YYSYMBOL_CREATE = 4,                     /* CREATE  */
  YYSYMBOL_DROP = 5,                       /* DROP  */
  YYSYMBOL_TABLE = 6,                      /* TABLE  */
  YYSYMBOL_TABLES = 7,                     /* TABLES  */
  YYSYMBOL_NULLABLE = 8,                   /* NULLABLE  */
  YYSYMBOL_NULL_T = 9,                     /* NULL_T  */
  YYSYMBOL_IS = 10,                        /* IS  */
  YYSYMBOL_INDEX = 11,                     /* INDEX  */
  YYSYMBOL_SELECT = 12,                    /* SELECT  */
  YYSYMBOL_SHOW = 13,                      /* SHOW  */
  YYSYMBOL_SYNC = 14,                      /* SYNC  */
  YYSYMBOL_INSERT = 15,                    /* INSERT  */
  YYSYMBOL_DELETE = 16,                    /* DELETE  */
  YYSYMBOL_UPDATE = 17,                    /* UPDATE  */
  YYSYMBOL_LBRACE = 18,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 19,                    /* RBRACE  */
  YYSYMBOL_COMMA = 20,                     /* COMMA  */
  YYSYMBOL_TRX_BEGIN = 21,                 /* TRX_BEGIN  */
  YYSYMBOL_TRX_COMMIT = 22,                /* TRX_COMMIT  */
  YYSYMBOL_TRX_ROLLBACK = 23,              /* TRX_ROLLBACK  */
  YYSYMBOL_INT_T = 24,                     /* INT_T  */
  YYSYMBOL_STRING_T = 25,                  /* STRING_T  */
  YYSYMBOL_FLOAT_T = 26,                   /* FLOAT_T  */
  YYSYMBOL_DATE_T = 27,                    /* DATE_T  */
  YYSYMBOL_TEXT_T = 28,                    /* TEXT_T  */
  YYSYMBOL_HELP = 29,                      /* HELP  */
  YYSYMBOL_EXIT = 30,                      /* EXIT  */
  YYSYMBOL_DOT = 31,                       /* DOT  */
  YYSYMBOL_INTO = 32,                      /* INTO  */
  YYSYMBOL_VALUES = 33,                    /* VALUES  */
  YYSYMBOL_FROM = 34,                      /* FROM  */
  YYSYMBOL_WHERE = 35,                     /* WHERE  */
  YYSYMBOL_IN = 36,                        /* IN  */
  YYSYMBOL_EXISTS = 37,                    /* EXISTS  */
  YYSYMBOL_AND = 38,                       /* AND  */
  YYSYMBOL_SET = 39,                       /* SET  */
  YYSYMBOL_ON = 40,                        /* ON  */
  YYSYMBOL_LOAD = 41,                      /* LOAD  */
  YYSYMBOL_DATA = 42,                      /* DATA  */
  YYSYMBOL_INFILE = 43,                    /* INFILE  */
  YYSYMBOL_COUNT = 44,                     /* COUNT  */
  YYSYMBOL_SUM = 45,                       /* SUM  */
  YYSYMBOL_MAX = 46,                       /* MAX  */
  YYSYMBOL_MIN = 47,                       /* MIN  */
  YYSYMBOL_AVG = 48,                       /* AVG  */
  YYSYMBOL_EQ = 49,                        /* EQ  */
  YYSYMBOL_LT = 50,                        /* LT  */
  YYSYMBOL_GT = 51,                        /* GT  */
  YYSYMBOL_LE = 52,                        /* LE  */
  YYSYMBOL_GE = 53,                        /* GE  */
  YYSYMBOL_NE = 54,                        /* NE  */
  YYSYMBOL_LK = 55,                        /* LK  */
  YYSYMBOL_NOT = 56,                       /* NOT  */
  YYSYMBOL_UNIQUE = 57,                    /* UNIQUE  */
  YYSYMBOL_AS = 58,                        /* AS  */
  YYSYMBOL_ORDER = 59,                     /* ORDER  */
  YYSYMBOL_BY = 60,                        /* BY  */
  YYSYMBOL_GROUP = 61,                     /* GROUP  */
  YYSYMBOL_HAVING = 62,                    /* HAVING  */
  YYSYMBOL_ASC = 63,                       /* ASC  */
  YYSYMBOL_DESC = 64,                      /* DESC  */
  YYSYMBOL_INNER = 65,                     /* INNER  */
  YYSYMBOL_JOIN = 66,                      /* JOIN  */
  YYSYMBOL_NUMBER = 67,                    /* NUMBER  */
  YYSYMBOL_FLOAT = 68,                     /* FLOAT  */
  YYSYMBOL_ID = 69,                        /* ID  */
  YYSYMBOL_PATH = 70,                      /* PATH  */
  YYSYMBOL_DATE_STR = 71,                  /* DATE_STR  */
  YYSYMBOL_SSS = 72,                       /* SSS  */
  YYSYMBOL_SUBQUERY_STR = 73,              /* SUBQUERY_STR  */
  YYSYMBOL_EXPRESSION_STR = 74,            /* EXPRESSION_STR  */
  YYSYMBOL_STAR = 75,                      /* STAR  */
  YYSYMBOL_STRING_V = 76,                  /* STRING_V  */
  YYSYMBOL_YYACCEPT = 77,                  /* $accept  */
  YYSYMBOL_commands = 78,                  /* commands  */
  YYSYMBOL_command = 79,                   /* command  */
  YYSYMBOL_exit = 80,                      /* exit  */
  YYSYMBOL_help = 81,                      /* help  */
  YYSYMBOL_sync = 82,                      /* sync  */
  YYSYMBOL_begin = 83,                     /* begin  */
  YYSYMBOL_commit = 84,                    /* commit  */
  YYSYMBOL_rollback = 85,                  /* rollback  */
  YYSYMBOL_drop_table = 86,                /* drop_table  */
  YYSYMBOL_show_tables = 87,               /* show_tables  */
  YYSYMBOL_desc_table = 88,                /* desc_table  */
  YYSYMBOL_create_index = 89,              /* create_index  */
  YYSYMBOL_index_type = 90,                /* index_type  */
  YYSYMBOL_index_col_list = 91,            /* index_col_list  */
  YYSYMBOL_drop_index = 92,                /* drop_index  */
  YYSYMBOL_show_index = 93,                /* show_index  */
  YYSYMBOL_create_table = 94,              /* create_table  */
  YYSYMBOL_attr_def_list = 95,             /* attr_def_list  */
  YYSYMBOL_attr_def = 96,                  /* attr_def  */
  YYSYMBOL_number = 97,                    /* number  */
  YYSYMBOL_type = 98,                      /* type  */
  YYSYMBOL_ID_get = 99,                    /* ID_get  */
  YYSYMBOL_insert = 100,                   /* insert  */
  YYSYMBOL_value_list = 101,               /* value_list  */
  YYSYMBOL_more_value_list = 102,          /* more_value_list  */
  YYSYMBOL_value = 103,                    /* value  */
  YYSYMBOL_delete = 104,                   /* delete  */
  YYSYMBOL_update = 105,                   /* update  */
  YYSYMBOL_update_list = 106,              /* update_list  */
  YYSYMBOL_update_col = 107,               /* update_col  */
  YYSYMBOL_select = 108,                   /* select  */
  YYSYMBOL_select_attr = 109,              /* select_attr  */
  YYSYMBOL_aggregation = 110,              /* aggregation  */
  YYSYMBOL_agg_list = 111,                 /* agg_list  */
  YYSYMBOL_attr_list = 112,                /* attr_list  */
  YYSYMBOL_rel_list = 113,                 /* rel_list  */
  YYSYMBOL_join_list = 114,                /* join_list  */
  YYSYMBOL_join = 115,                     /* join  */
  YYSYMBOL_join_condition_list = 116,      /* join_condition_list  */
  YYSYMBOL_join_condition = 117,           /* join_condition  */
  YYSYMBOL_where = 118,                    /* where  */
  YYSYMBOL_condition_list = 119,           /* condition_list  */
  YYSYMBOL_condition = 120,                /* condition  */
  YYSYMBOL_order = 121,                    /* order  */
  YYSYMBOL_order_by_list = 122,            /* order_by_list  */
  YYSYMBOL_order_by = 123,                 /* order_by  */
  YYSYMBOL_group = 124,                    /* group  */
  YYSYMBOL_group_by_list = 125,            /* group_by_list  */
  YYSYMBOL_group_by = 126,                 /* group_by  */
  YYSYMBOL_group_having = 127,             /* group_having  */
  YYSYMBOL_comOp = 128,                    /* comOp  */
  YYSYMBOL_load_data = 129                 /* load_data  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   451

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  77
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  53
/* YYNRULES -- Number of rules.  */
#define YYNRULES  177
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  423

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   331


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   171,   171,   173,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   198,   203,   208,   214,   220,   226,   232,   238,
     244,   251,   260,   263,   267,   269,   276,   284,   291,   300,
     302,   306,   313,   322,   331,   343,   346,   347,   348,   349,
     350,   353,   362,   370,   372,   374,   376,   379,   382,   385,
     389,   394,   397,   403,   413,   423,   425,   428,   432,   444,
     465,   471,   477,   483,   489,   490,   498,   504,   510,   516,
     522,   528,   534,   540,   546,   552,   558,   564,   570,   576,
     582,   588,   589,   590,   591,   593,   595,   596,   598,   600,
     603,   605,   610,   612,   615,   622,   624,   627,   637,   646,
     655,   665,   667,   670,   672,   676,   687,   696,   707,   717,
     727,   738,   749,   753,   756,   766,   776,   786,   796,   806,
     816,   826,   836,   866,   868,   870,   872,   874,   876,   881,
     886,   891,   896,   901,   907,   909,   911,   913,   915,   917,
     922,   927,   929,   937,   945,   953,   961,   969,   977,   985,
     993,  1001,  1009,  1017,  1018,  1019,  1020,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,  1032,  1033,  1037
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "SEMICOLON", "CREATE",
  "DROP", "TABLE", "TABLES", "NULLABLE", "NULL_T", "IS", "INDEX", "SELECT",
  "SHOW", "SYNC", "INSERT", "DELETE", "UPDATE", "LBRACE", "RBRACE",
  "COMMA", "TRX_BEGIN", "TRX_COMMIT", "TRX_ROLLBACK", "INT_T", "STRING_T",
  "FLOAT_T", "DATE_T", "TEXT_T", "HELP", "EXIT", "DOT", "INTO", "VALUES",
  "FROM", "WHERE", "IN", "EXISTS", "AND", "SET", "ON", "LOAD", "DATA",
  "INFILE", "COUNT", "SUM", "MAX", "MIN", "AVG", "EQ", "LT", "GT", "LE",
  "GE", "NE", "LK", "NOT", "UNIQUE", "AS", "ORDER", "BY", "GROUP",
  "HAVING", "ASC", "DESC", "INNER", "JOIN", "NUMBER", "FLOAT", "ID",
  "PATH", "DATE_STR", "SSS", "SUBQUERY_STR", "EXPRESSION_STR", "STAR",
  "STRING_V", "$accept", "commands", "command", "exit", "help", "sync",
  "begin", "commit", "rollback", "drop_table", "show_tables", "desc_table",
  "create_index", "index_type", "index_col_list", "drop_index",
  "show_index", "create_table", "attr_def_list", "attr_def", "number",
  "type", "ID_get", "insert", "value_list", "more_value_list", "value",
  "delete", "update", "update_list", "update_col", "select", "select_attr",
  "aggregation", "agg_list", "attr_list", "rel_list", "join_list", "join",
  "join_condition_list", "join_condition", "where", "condition_list",
  "condition", "order", "order_by_list", "order_by", "group",
  "group_by_list", "group_by", "group_having", "comOp", "load_data", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-263)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -263,     9,  -263,     5,    70,   276,   166,    25,   -15,     1,
     -16,    82,    87,    89,   100,   125,   107,   -11,  -263,  -263,
    -263,  -263,  -263,  -263,  -263,  -263,  -263,  -263,  -263,  -263,
    -263,  -263,  -263,  -263,  -263,  -263,  -263,    62,  -263,   147,
      98,   103,   123,   182,   186,   189,   198,   201,    16,   207,
     207,   195,  -263,   228,   205,  -263,   173,   175,   210,  -263,
    -263,  -263,  -263,  -263,   203,   252,   239,  -263,   219,   271,
     272,   -26,   -21,    36,    47,    54,   276,   209,   211,  -263,
    -263,  -263,   213,  -263,   217,   243,   244,   220,   216,  -263,
     229,   230,  -263,  -263,    21,   171,   289,    32,   290,    37,
     190,   291,    69,   192,   292,    78,   214,   311,  -263,   207,
     207,   312,   328,   315,    -2,   331,   286,   316,   305,  -263,
     318,   197,   321,   207,   273,   274,   207,   275,  -263,   207,
     277,   207,   207,   278,   207,  -263,   207,   279,   207,  -263,
     207,   280,   207,  -263,  -263,  -263,   283,   288,  -263,   169,
    -263,  -263,  -263,   241,  -263,  -263,   263,  -263,   263,   302,
    -263,    74,   220,   244,   335,   229,   336,  -263,  -263,  -263,
    -263,  -263,    26,   285,  -263,  -263,   337,  -263,  -263,  -263,
     338,  -263,  -263,   339,  -263,  -263,   340,  -263,  -263,   341,
    -263,   312,   295,   244,   288,   342,   307,   296,   -14,  -263,
    -263,  -263,  -263,  -263,  -263,  -263,   -18,    46,   309,   116,
      85,    -2,  -263,  -263,  -263,   316,   363,   298,   318,   365,
    -263,   303,   360,   351,   207,   207,   207,   207,   207,  -263,
     304,   313,  -263,   169,   353,  -263,   251,   169,  -263,    11,
    -263,  -263,  -263,  -263,   344,  -263,   345,  -263,   302,  -263,
    -263,   374,  -263,  -263,  -263,   359,  -263,   310,   361,  -263,
    -263,  -263,  -263,  -263,   343,   322,   325,   342,   366,   308,
      -9,    66,   342,   169,  -263,   319,   320,  -263,  -263,  -263,
     351,   382,   112,   323,   327,   387,  -263,   373,   390,  -263,
     324,   364,  -263,  -263,   375,   342,  -263,  -263,  -263,  -263,
     367,   263,   358,   368,   380,   332,  -263,   169,  -263,  -263,
     333,  -263,   384,   346,   134,   112,  -263,   347,   323,   348,
      14,   385,   342,  -263,  -263,   263,   376,  -263,   358,  -263,
     380,   281,  -263,   349,  -263,  -263,   332,  -263,   389,   146,
     350,  -263,  -263,   386,   388,   391,   393,   394,   200,   385,
     366,   383,  -263,  -263,    55,   352,    57,    75,    91,  -263,
    -263,  -263,  -263,   354,   143,   246,   398,    76,   145,   400,
     403,   151,   405,   168,   406,  -263,   263,   357,   263,  -263,
     263,   362,   263,   369,  -263,   263,   370,  -263,   263,   371,
    -263,   169,   408,   169,   169,   409,   169,   410,   169,   411,
     169,   413,  -263,   263,  -263,  -263,   263,  -263,   263,  -263,
     263,  -263,   263,   169,   169,   169,   169,   169,  -263,  -263,
    -263,  -263,  -263
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     3,    21,
      20,    15,    16,    17,    18,     9,    10,    11,    12,    13,
      14,     8,     5,     7,     6,     4,    19,     0,    32,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    98,    98,
      98,     0,    74,     0,     0,    24,     0,     0,     0,    25,
      26,    27,    23,    22,     0,     0,     0,    33,     0,     0,
       0,    95,     0,    95,    95,    95,     0,     0,     0,    71,
      75,    70,     0,    29,     0,     0,   111,     0,     0,    30,
       0,     0,    28,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    99,    98,
      98,   100,     0,     0,     0,     0,     0,    65,     0,    51,
      39,     0,     0,    98,     0,     0,    98,     0,    91,    98,
       0,    98,    98,     0,    98,    92,    98,     0,    98,    93,
      98,     0,    98,    94,    73,    72,     0,   102,    37,     0,
      61,    57,    58,     0,    60,    59,     0,    62,     0,   113,
      63,     0,     0,   111,     0,     0,     0,    46,    47,    48,
      49,    50,    42,     0,    77,    97,     0,    76,    96,    80,
       0,    79,    86,     0,    85,    89,     0,    88,    83,     0,
      82,   100,     0,   111,   102,    53,   175,     0,     0,   167,
     168,   169,   170,   171,   172,   173,     0,     0,     0,     0,
       0,     0,   112,    68,    67,    65,     0,     0,    39,     0,
      43,     0,     0,    34,    98,    98,    98,    98,    98,   101,
       0,   144,   103,     0,     0,   176,     0,     0,   124,     0,
     174,   117,   128,   115,   129,   132,   118,   116,   113,    66,
      64,     0,    40,    38,    45,     0,    44,     0,     0,    78,
      81,    87,    90,    84,     0,     0,   133,    53,    55,     0,
       0,     0,    53,     0,   125,     0,     0,   114,   177,    41,
      34,     0,     0,   148,     0,     0,    54,     0,     0,   126,
       0,     0,   130,   119,     0,    53,   131,   120,    35,    31,
       0,     0,   105,   149,   146,   137,    69,     0,    52,   127,
       0,   122,     0,     0,     0,     0,   104,     0,   148,   151,
     138,   135,    53,   121,   123,     0,     0,   110,   105,   150,
     146,     0,   145,     0,   139,   140,   137,   134,     0,     0,
       0,   106,   147,     0,     0,     0,     0,     0,   141,   135,
      55,     0,   109,   108,    95,     0,    95,    95,    95,   142,
     143,   136,    56,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   107,     0,     0,     0,   163,
       0,     0,     0,     0,   164,     0,     0,   165,     0,     0,
     166,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   153,     0,   152,   155,     0,   159,     0,   161,
       0,   157,     0,     0,     0,     0,     0,     0,   154,   156,
     160,   162,   158
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -263,  -263,  -263,  -263,  -263,  -263,  -263,  -263,  -263,  -263,
    -263,  -263,  -263,  -263,   133,  -263,  -263,  -263,   215,   269,
    -263,  -263,  -263,  -263,  -262,    86,  -146,  -263,  -263,   222,
     282,  -263,   372,  -263,   -73,   -30,   250,   248,  -263,   115,
     120,  -151,   199,   234,  -263,    97,   113,  -263,   121,   132,
    -263,  -150,  -263
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    40,   258,    29,    30,    31,   166,   120,
     255,   172,   121,    32,   234,   288,   158,    33,    34,   163,
     117,    35,    51,    52,    96,    79,   147,   193,   194,   316,
     302,   115,   212,   159,   285,   337,   321,   266,   319,   304,
     332,   207,    36
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     101,   104,   107,   195,   237,   286,   209,   150,   210,     2,
     294,    37,   216,     3,     4,   214,    38,    56,   239,    80,
      81,     5,     6,     7,     8,     9,    10,   290,    55,   273,
      11,    12,    13,   312,   220,    57,    76,   240,    14,    15,
     123,   124,   231,    94,   221,   333,   240,    77,    97,    95,
      16,   129,   125,    58,    98,   150,   132,   124,    65,   238,
     338,   243,    39,   130,   247,   151,   152,   153,   133,   154,
     155,   156,   157,    17,    78,   150,    41,   334,   335,   144,
     145,    42,   222,   150,   274,    59,   271,   267,   136,   124,
      60,   272,    61,   174,   150,   380,   177,   140,   124,   179,
     137,   181,   182,    62,   184,    99,   185,   381,   187,   141,
     188,   100,   190,   151,   152,   241,   102,   154,   155,   242,
     157,   150,   103,   105,   364,   293,   368,   295,    63,   106,
     365,    66,   369,   151,   152,   291,   301,   154,   155,   292,
     157,   151,   152,   150,   371,   154,   155,   213,   157,    64,
     369,   314,   151,   152,   246,   150,   154,   155,    67,   157,
     373,   322,   376,   124,   382,   124,   369,    68,   327,   301,
     385,   124,    69,    53,   377,   339,   383,    54,   150,   151,
     152,   300,   386,   154,   155,   244,   157,   388,   124,   245,
     126,   127,    70,   352,   259,   260,   261,   262,   263,   389,
      71,   151,   152,   326,    72,   154,   155,    73,   157,   134,
     127,   138,   127,   151,   152,   351,    74,   154,   155,    75,
     157,   167,   168,   169,   170,   171,   391,    76,   393,    82,
     394,    83,   396,   142,   127,   398,   151,   152,   400,    84,
     154,   155,    85,   157,    86,   402,    88,   404,   405,    87,
     407,   196,   409,   413,   411,    89,   414,    90,   415,    91,
     416,   196,   417,   359,   360,   378,   127,   418,   419,   420,
     421,   422,   197,   196,    92,    93,   113,   198,   109,   114,
     110,   366,   111,   370,   372,   374,   112,   269,   118,   116,
     199,   200,   201,   202,   203,   204,   205,   206,   119,   122,
     199,   200,   201,   202,   203,   204,   205,   270,   128,   131,
     135,   139,   199,   200,   201,   202,   203,   204,   205,   208,
      43,    44,    45,    46,    47,   343,   344,   345,   346,   347,
     143,   148,   146,   149,   160,   161,   162,   164,   165,   173,
     211,   217,   175,   176,   178,    48,   180,   183,   186,   189,
      49,    50,   191,   192,   223,   219,   224,   225,   226,   227,
     228,   230,   233,   235,   240,   236,   250,   251,   253,   256,
     254,   257,   268,   264,   265,   275,   276,   278,   279,   280,
     281,   289,   283,   282,   284,   299,   287,   305,   296,   297,
     306,   307,   303,   308,   311,   310,   315,   309,   313,   317,
     318,   320,   323,   324,   354,   336,   355,   340,   350,   356,
     331,   357,   358,   298,   363,   325,   329,   379,   348,   353,
     127,   367,   384,   375,   387,   390,   392,   403,   406,   408,
     410,   395,   412,   252,   218,   328,   362,   249,   397,   399,
     401,   229,   232,   341,   215,   248,   361,   277,   108,   349,
     330,   342
};

static const yytype_int16 yycheck[] =
{
      73,    74,    75,   149,    18,   267,   156,     9,   158,     0,
     272,     6,   163,     4,     5,   161,    11,    32,    36,    49,
      50,    12,    13,    14,    15,    16,    17,    36,     3,    18,
      21,    22,    23,   295,     8,    34,    20,    55,    29,    30,
      19,    20,   193,    69,    18,    31,    55,    31,    69,    75,
      41,    19,    31,    69,    75,     9,    19,    20,    69,    73,
     322,   207,    57,    31,   210,    67,    68,    69,    31,    71,
      72,    73,    74,    64,    58,     9,     6,    63,    64,   109,
     110,    11,    56,     9,    73,     3,   236,   233,    19,    20,
       3,   237,     3,   123,     9,    19,   126,    19,    20,   129,
      31,   131,   132,     3,   134,    69,   136,    31,   138,    31,
     140,    75,   142,    67,    68,    69,    69,    71,    72,    73,
      74,     9,    75,    69,    69,   271,    69,   273,     3,    75,
      75,    69,    75,    67,    68,    69,   282,    71,    72,    73,
      74,    67,    68,     9,    69,    71,    72,    73,    74,    42,
      75,   301,    67,    68,    69,     9,    71,    72,    11,    74,
      69,   307,    19,    20,    19,    20,    75,    69,   314,   315,
      19,    20,    69,     7,    31,   325,    31,    11,     9,    67,
      68,    69,    31,    71,    72,    69,    74,    19,    20,    73,
      19,    20,    69,   339,   224,   225,   226,   227,   228,    31,
      18,    67,    68,    69,    18,    71,    72,    18,    74,    19,
      20,    19,    20,    67,    68,    69,    18,    71,    72,    18,
      74,    24,    25,    26,    27,    28,   376,    20,   378,    34,
     380,     3,   382,    19,    20,   385,    67,    68,   388,    34,
      71,    72,    69,    74,    69,   391,    43,   393,   394,    39,
     396,    10,   398,   403,   400,     3,   406,    18,   408,    40,
     410,    10,   412,    63,    64,    19,    20,   413,   414,   415,
     416,   417,    31,    10,     3,     3,    33,    36,    69,    35,
      69,   354,    69,   356,   357,   358,    69,    36,    72,    69,
      49,    50,    51,    52,    53,    54,    55,    56,    69,    69,
      49,    50,    51,    52,    53,    54,    55,    56,    19,    19,
      19,    19,    49,    50,    51,    52,    53,    54,    55,    56,
      44,    45,    46,    47,    48,    44,    45,    46,    47,    48,
      19,     3,    20,    18,     3,    49,    20,    32,    20,    18,
      38,     6,    69,    69,    69,    69,    69,    69,    69,    69,
      74,    75,    69,    65,    69,    19,    19,    19,    19,    19,
      19,    66,    20,    56,    55,    69,     3,    69,     3,     9,
      67,    20,    19,    69,    61,    31,    31,     3,    19,    69,
      19,    73,    60,    40,    59,     3,    20,    60,    69,    69,
       3,    18,    69,     3,    19,    31,    38,    73,    31,    31,
      20,    69,    69,    19,    18,    20,    18,    31,    19,    18,
      62,    18,    18,   280,    31,    69,    69,    19,    69,    69,
      20,    69,    19,    69,    19,    19,    69,    19,    19,    19,
      19,    69,    19,   218,   165,   315,   350,   215,    69,    69,
      69,   191,   194,   328,   162,   211,   349,   248,    76,   336,
     318,   330
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    78,     0,     4,     5,    12,    13,    14,    15,    16,
      17,    21,    22,    23,    29,    30,    41,    64,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    92,
      93,    94,   100,   104,   105,   108,   129,     6,    11,    57,
      90,     6,    11,    44,    45,    46,    47,    48,    69,    74,
      75,   109,   110,     7,    11,     3,    32,    34,    69,     3,
       3,     3,     3,     3,    42,    69,    69,    11,    69,    69,
      69,    18,    18,    18,    18,    18,    20,    31,    58,   112,
     112,   112,    34,     3,    34,    69,    69,    39,    43,     3,
      18,    40,     3,     3,    69,    75,   111,    69,    75,    69,
      75,   111,    69,    75,   111,    69,    75,   111,   109,    69,
      69,    69,    69,    33,    35,   118,    69,   107,    72,    69,
      96,    99,    69,    19,    20,    31,    19,    20,    19,    19,
      31,    19,    19,    31,    19,    19,    19,    31,    19,    19,
      19,    31,    19,    19,   112,   112,    20,   113,     3,    18,
       9,    67,    68,    69,    71,    72,    73,    74,   103,   120,
       3,    49,    20,   106,    32,    20,    95,    24,    25,    26,
      27,    28,    98,    18,   112,    69,    69,   112,    69,   112,
      69,   112,   112,    69,   112,   112,    69,   112,   112,    69,
     112,    69,    65,   114,   115,   103,    10,    31,    36,    49,
      50,    51,    52,    53,    54,    55,    56,   128,    56,   128,
     128,    38,   119,    73,   103,   107,   118,     6,    96,    19,
       8,    18,    56,    69,    19,    19,    19,    19,    19,   113,
      66,   118,   114,    20,   101,    56,    69,    18,    73,    36,
      55,    69,    73,   103,    69,    73,    69,   103,   120,   106,
       3,    69,    95,     3,    67,    97,     9,    20,    91,   112,
     112,   112,   112,   112,    69,    61,   124,   103,    19,    36,
      56,   128,   103,    18,    73,    31,    31,   119,     3,    19,
      69,    19,    40,    60,    59,   121,   101,    20,   102,    73,
      36,    69,    73,   103,   101,   103,    69,    69,    91,     3,
      69,   103,   117,    69,   126,    60,     3,    18,     3,    73,
      31,    19,   101,    31,   128,    38,   116,    31,    20,   125,
      69,   123,   103,    69,    19,    69,    69,   103,   117,    69,
     126,    62,   127,    31,    63,    64,    20,   122,   101,   128,
      31,   116,   125,    44,    45,    46,    47,    48,    69,   123,
      19,    69,   103,    69,    18,    18,    18,    18,    18,    63,
      64,   122,   102,    31,    69,    75,   111,    69,    69,    75,
     111,    69,   111,    69,   111,    69,    19,    31,    19,    19,
      19,    31,    19,    31,    19,    19,    31,    19,    19,    31,
      19,   128,    69,   128,   128,    69,   128,    69,   128,    69,
     128,    69,   103,    19,   103,   103,    19,   103,    19,   103,
      19,   103,    19,   128,   128,   128,   128,   128,   103,   103,
     103,   103,   103
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    77,    78,    78,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    90,    91,    91,    92,    93,    94,    95,
      95,    96,    96,    96,    96,    97,    98,    98,    98,    98,
      98,    99,   100,   101,   101,   102,   102,   103,   103,   103,
     103,   103,   103,   104,   105,   106,   106,   107,   107,   108,
     109,   109,   109,   109,   109,   109,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   111,   111,   111,   112,   112,
     113,   113,   114,   114,   115,   116,   116,   117,   117,   117,
     117,   118,   118,   119,   119,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   121,   121,   122,   122,   123,   123,   123,
     123,   123,   123,   123,   124,   124,   125,   125,   126,   126,
     126,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   129
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     2,     2,     2,     2,     4,     3,
       3,    10,     1,     2,     0,     3,     4,     5,     8,     0,
       3,     5,     2,     3,     4,     1,     1,     1,     1,     1,
       1,     1,    10,     0,     3,     0,     6,     1,     1,     1,
       1,     1,     1,     5,     7,     0,     3,     3,     3,    10,
       2,     2,     4,     4,     1,     2,     5,     5,     7,     5,
       5,     7,     5,     5,     7,     5,     5,     7,     5,     5,
       7,     4,     4,     4,     4,     0,     3,     3,     0,     2,
       0,     3,     0,     2,     6,     0,     3,     7,     5,     5,
       3,     0,     3,     0,     3,     3,     3,     3,     3,     5,
       5,     7,     6,     7,     3,     4,     5,     6,     3,     3,
       5,     5,     3,     0,     4,     0,     3,     0,     1,     2,
       2,     3,     4,     4,     0,     5,     0,     3,     0,     1,
       3,     0,     7,     7,     9,     7,     9,     7,     9,     7,
       9,     7,     9,     5,     5,     5,     5,     1,     1,     1,
       1,     1,     1,     1,     2,     1,     2,     8
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (scanner, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *scanner)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (scanner);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *scanner)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, scanner);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, void *scanner)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, scanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, void *scanner)
{
  YY_USE (yyvaluep);
  YY_USE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *scanner)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 22: /* exit: EXIT SEMICOLON  */
#line 198 "yacc_sql.y"
                   {
        CONTEXT->ssql->flag=SCF_EXIT;//"exit";
    }
#line 1556 "yacc_sql.tab.c"
    break;

  case 23: /* help: HELP SEMICOLON  */
#line 203 "yacc_sql.y"
                   {
        CONTEXT->ssql->flag=SCF_HELP;//"help";
    }
#line 1564 "yacc_sql.tab.c"
    break;

  case 24: /* sync: SYNC SEMICOLON  */
#line 208 "yacc_sql.y"
                   {
      CONTEXT->ssql->flag = SCF_SYNC;
    }
#line 1572 "yacc_sql.tab.c"
    break;

  case 25: /* begin: TRX_BEGIN SEMICOLON  */
#line 214 "yacc_sql.y"
                        {
      CONTEXT->ssql->flag = SCF_BEGIN;
    }
#line 1580 "yacc_sql.tab.c"
    break;

  case 26: /* commit: TRX_COMMIT SEMICOLON  */
#line 220 "yacc_sql.y"
                         {
      CONTEXT->ssql->flag = SCF_COMMIT;
    }
#line 1588 "yacc_sql.tab.c"
    break;

  case 27: /* rollback: TRX_ROLLBACK SEMICOLON  */
#line 226 "yacc_sql.y"
                           {
      CONTEXT->ssql->flag = SCF_ROLLBACK;
    }
#line 1596 "yacc_sql.tab.c"
    break;

  case 28: /* drop_table: DROP TABLE ID SEMICOLON  */
#line 232 "yacc_sql.y"
                            {
        CONTEXT->ssql->flag = SCF_DROP_TABLE;//"drop_table";
        drop_table_init(&CONTEXT->ssql->sstr.drop_table, (yyvsp[-1].string));
    }
#line 1605 "yacc_sql.tab.c"
    break;

  case 29: /* show_tables: SHOW TABLES SEMICOLON  */
#line 238 "yacc_sql.y"
                          {
      CONTEXT->ssql->flag = SCF_SHOW_TABLES;
    }
#line 1613 "yacc_sql.tab.c"
    break;

  case 30: /* desc_table: DESC ID SEMICOLON  */
#line 244 "yacc_sql.y"
                      {
      CONTEXT->ssql->flag = SCF_DESC_TABLE;
      desc_table_init(&CONTEXT->ssql->sstr.desc_table, (yyvsp[-1].string));
    }
#line 1622 "yacc_sql.tab.c"
    break;

  case 31: /* create_index: CREATE index_type ID ON ID LBRACE ID index_col_list RBRACE SEMICOLON  */
#line 252 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_CREATE_INDEX;//"create_index";
			char *temp=(yyvsp[-3].string); 
			create_index_append_attribute(&CONTEXT->ssql->sstr.create_index, temp);
			create_index_init(&CONTEXT->ssql->sstr.create_index, (yyvsp[-7].string), (yyvsp[-5].string));
		}
#line 1633 "yacc_sql.tab.c"
    break;

  case 32: /* index_type: INDEX  */
#line 260 "yacc_sql.y"
              {
		create_index_unique(&CONTEXT->ssql->sstr.create_index, 0);
	}
#line 1641 "yacc_sql.tab.c"
    break;

  case 33: /* index_type: UNIQUE INDEX  */
#line 263 "yacc_sql.y"
                       {
		create_index_unique(&CONTEXT->ssql->sstr.create_index, 1);
	}
#line 1649 "yacc_sql.tab.c"
    break;

  case 35: /* index_col_list: COMMA ID index_col_list  */
#line 269 "yacc_sql.y"
                                  {
		char *temp=(yyvsp[-1].string); 
		create_index_append_attribute(&CONTEXT->ssql->sstr.create_index, temp);
	}
#line 1658 "yacc_sql.tab.c"
    break;

  case 36: /* drop_index: DROP INDEX ID SEMICOLON  */
#line 277 "yacc_sql.y"
                {
			CONTEXT->ssql->flag=SCF_DROP_INDEX;//"drop_index";
			drop_index_init(&CONTEXT->ssql->sstr.drop_index, (yyvsp[-1].string));
		}
#line 1667 "yacc_sql.tab.c"
    break;

  case 37: /* show_index: SHOW INDEX FROM ID SEMICOLON  */
#line 284 "yacc_sql.y"
                                 {
      CONTEXT->ssql->flag = SCF_SHOW_INDEX;
	  show_index_init(&CONTEXT->ssql->sstr.show_index, (yyvsp[-1].string));
    }
#line 1676 "yacc_sql.tab.c"
    break;

  case 38: /* create_table: CREATE TABLE ID LBRACE attr_def attr_def_list RBRACE SEMICOLON  */
#line 292 "yacc_sql.y"
                {
			CONTEXT->ssql->flag=SCF_CREATE_TABLE;//"create_table";
			// CONTEXT->ssql->sstr.create_table.attribute_count = CONTEXT->value_length;
			create_table_init_name(&CONTEXT->ssql->sstr.create_table, (yyvsp[-5].string));
			//临时变量清零	
			CONTEXT->value_length = 0;
		}
#line 1688 "yacc_sql.tab.c"
    break;

  case 40: /* attr_def_list: COMMA attr_def attr_def_list  */
#line 302 "yacc_sql.y"
                                   {    }
#line 1694 "yacc_sql.tab.c"
    break;

  case 41: /* attr_def: ID_get type LBRACE number RBRACE  */
#line 307 "yacc_sql.y"
                {
			AttrInfo attribute;
			attr_info_init(&attribute, CONTEXT->id, (yyvsp[-3].number), (yyvsp[-1].number), 0);
			create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
			CONTEXT->value_length++;
		}
#line 1705 "yacc_sql.tab.c"
    break;

  case 42: /* attr_def: ID_get type  */
#line 314 "yacc_sql.y"
                {
			AttrInfo attribute;
			int len = 4;
			if((yyvsp[0].number) == TEXTS) len = 4096;
			attr_info_init(&attribute, CONTEXT->id, (yyvsp[0].number), len, 0);
			create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
			CONTEXT->value_length++;
		}
#line 1718 "yacc_sql.tab.c"
    break;

  case 43: /* attr_def: ID_get type NULLABLE  */
#line 323 "yacc_sql.y"
                {
			AttrInfo attribute;
			int len = 4;
			if((yyvsp[-1].number) == TEXTS) len = 4096;
			attr_info_init(&attribute, CONTEXT->id, (yyvsp[-1].number), len, 1);
			create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
			CONTEXT->value_length++;
		}
#line 1731 "yacc_sql.tab.c"
    break;

  case 44: /* attr_def: ID_get type NOT NULL_T  */
#line 332 "yacc_sql.y"
                {
			AttrInfo attribute;
			int len = 4;
			if((yyvsp[-2].number) == TEXTS) len = 4096;
			attr_info_init(&attribute, CONTEXT->id, (yyvsp[-2].number), len, 0);
			create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
			CONTEXT->value_length++;
		}
#line 1744 "yacc_sql.tab.c"
    break;

  case 45: /* number: NUMBER  */
#line 343 "yacc_sql.y"
                       {(yyval.number) = (yyvsp[0].number);}
#line 1750 "yacc_sql.tab.c"
    break;

  case 46: /* type: INT_T  */
#line 346 "yacc_sql.y"
              { (yyval.number)=INTS; }
#line 1756 "yacc_sql.tab.c"
    break;

  case 47: /* type: STRING_T  */
#line 347 "yacc_sql.y"
                  { (yyval.number)=CHARS; }
#line 1762 "yacc_sql.tab.c"
    break;

  case 48: /* type: FLOAT_T  */
#line 348 "yacc_sql.y"
                 { (yyval.number)=FLOATS; }
#line 1768 "yacc_sql.tab.c"
    break;

  case 49: /* type: DATE_T  */
#line 349 "yacc_sql.y"
                    { (yyval.number)=DATES; }
#line 1774 "yacc_sql.tab.c"
    break;

  case 50: /* type: TEXT_T  */
#line 350 "yacc_sql.y"
                    { (yyval.number)=TEXTS; }
#line 1780 "yacc_sql.tab.c"
    break;

  case 51: /* ID_get: ID  */
#line 354 "yacc_sql.y"
        {
		char *temp=(yyvsp[0].string); 
		snprintf(CONTEXT->id, sizeof(CONTEXT->id), "%s", temp);
	}
#line 1789 "yacc_sql.tab.c"
    break;

  case 52: /* insert: INSERT INTO ID VALUES LBRACE value value_list RBRACE more_value_list SEMICOLON  */
#line 363 "yacc_sql.y"
        {
			CONTEXT->ssql->flag=SCF_INSERT;//"insert";
			inserts_init(&CONTEXT->ssql->sstr.insertion, (yyvsp[-7].string), CONTEXT->values, CONTEXT->value_length);
      //临时变量清零
      CONTEXT->value_length=0;
    }
#line 1800 "yacc_sql.tab.c"
    break;

  case 54: /* value_list: COMMA value value_list  */
#line 372 "yacc_sql.y"
                              {}
#line 1806 "yacc_sql.tab.c"
    break;

  case 56: /* more_value_list: COMMA LBRACE value value_list RBRACE more_value_list  */
#line 376 "yacc_sql.y"
                                                               {}
#line 1812 "yacc_sql.tab.c"
    break;

  case 57: /* value: NUMBER  */
#line 379 "yacc_sql.y"
          {	
  		value_init_integer(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].number));
		}
#line 1820 "yacc_sql.tab.c"
    break;

  case 58: /* value: FLOAT  */
#line 382 "yacc_sql.y"
          {
  		value_init_float(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].floats));
		}
#line 1828 "yacc_sql.tab.c"
    break;

  case 59: /* value: SSS  */
#line 385 "yacc_sql.y"
         {
			(yyvsp[0].string) = substr((yyvsp[0].string),1,strlen((yyvsp[0].string))-2);
  		value_init_string(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].string));
		}
#line 1837 "yacc_sql.tab.c"
    break;

  case 60: /* value: DATE_STR  */
#line 389 "yacc_sql.y"
                 {
			(yyvsp[0].string) = substr((yyvsp[0].string),1,strlen((yyvsp[0].string))-2);
  		int ret = value_init_date(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].string));
		if(ret < 0) return -10;		
	}
#line 1847 "yacc_sql.tab.c"
    break;

  case 61: /* value: NULL_T  */
#line 394 "yacc_sql.y"
                {
		value_init_null(&CONTEXT->values[CONTEXT->value_length++]);
	}
#line 1855 "yacc_sql.tab.c"
    break;

  case 62: /* value: EXPRESSION_STR  */
#line 397 "yacc_sql.y"
                        {
		value_init_expression(&CONTEXT->values[CONTEXT->value_length++], (yyvsp[0].string));
	}
#line 1863 "yacc_sql.tab.c"
    break;

  case 63: /* delete: DELETE FROM ID where SEMICOLON  */
#line 404 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_DELETE;//"delete";
			deletes_init_relation(&CONTEXT->ssql->sstr.deletion, (yyvsp[-2].string));
			deletes_set_conditions(&CONTEXT->ssql->sstr.deletion, 
					CONTEXT->conditions, CONTEXT->condition_length);
			CONTEXT->condition_length = 0;	
    }
#line 1875 "yacc_sql.tab.c"
    break;

  case 64: /* update: UPDATE ID SET update_col update_list where SEMICOLON  */
#line 414 "yacc_sql.y"
                {
			CONTEXT->ssql->flag = SCF_UPDATE;//"update";
			// Value *value = &CONTEXT->values[0];
			// updates_init(&CONTEXT->ssql->sstr.update, $2, $4, value, CONTEXT->conditions, CONTEXT->condition_length);
			updates_init(&CONTEXT->ssql->sstr.update, (yyvsp[-5].string), CONTEXT->conditions, CONTEXT->condition_length);
			CONTEXT->condition_length = 0;
			CONTEXT->value_length = 0;
		}
#line 1888 "yacc_sql.tab.c"
    break;

  case 66: /* update_list: COMMA update_col update_list  */
#line 425 "yacc_sql.y"
                                       {}
#line 1894 "yacc_sql.tab.c"
    break;

  case 67: /* update_col: ID EQ value  */
#line 428 "yacc_sql.y"
                    {
		Value *value = &CONTEXT->values[CONTEXT->value_length - 1];
		update_append_value(&CONTEXT->ssql->sstr.update, (yyvsp[-2].string), value);
	}
#line 1903 "yacc_sql.tab.c"
    break;

  case 68: /* update_col: ID EQ SUBQUERY_STR  */
#line 432 "yacc_sql.y"
                             {
		Value *value = (Value*)malloc(sizeof(Value));
		value_init_string(value, (yyvsp[0].string));
		value->type = SUBQUERY;
		update_append_value(&CONTEXT->ssql->sstr.update, (yyvsp[-2].string), value);
	}
#line 1914 "yacc_sql.tab.c"
    break;

  case 69: /* select: SELECT select_attr FROM ID rel_list join_list where group order SEMICOLON  */
#line 445 "yacc_sql.y"
        {
			selects_append_relation(&CONTEXT->ssql->sstr.selection, (yyvsp[-6].string));
			//selects_append_attributes(&CONTEXT->ssql->sstr.selection, CONTEXT->attr, CONTEXT->attr_length);
			selects_append_conditions(&CONTEXT->ssql->sstr.selection, CONTEXT->conditions, CONTEXT->condition_length);
			selects_append_havings(&CONTEXT->ssql->sstr.selection, CONTEXT->havings, CONTEXT->having_length);
			selects_append_joins(&CONTEXT->ssql->sstr.selection, CONTEXT->joins, CONTEXT->join_length);
			CONTEXT->ssql->flag=SCF_SELECT;//"select";
			//临时变量清零
			CONTEXT->condition_length=0;
			CONTEXT->from_length=0;
			CONTEXT->value_length = 0;
			CONTEXT->join_length = 0;
			CONTEXT->join_condition_length = 0;
  			CONTEXT->attr_length = 0;
			CONTEXT->having_length = 0;
			CONTEXT->subset_num = 0;
	}
#line 1936 "yacc_sql.tab.c"
    break;

  case 70: /* select_attr: STAR attr_list  */
#line 465 "yacc_sql.y"
                  {  
			RelAttr attr;
			relation_attr_init(&attr, NULL, "*", NULL, NULL);
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 1947 "yacc_sql.tab.c"
    break;

  case 71: /* select_attr: ID attr_list  */
#line 471 "yacc_sql.y"
                   {
			RelAttr attr;
			relation_attr_init(&attr, NULL, (yyvsp[-1].string), NULL, NULL);
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 1958 "yacc_sql.tab.c"
    break;

  case 72: /* select_attr: ID AS ID attr_list  */
#line 477 "yacc_sql.y"
                         {
			RelAttr attr;
			relation_attr_init(&attr, NULL, (yyvsp[-3].string), (yyvsp[-1].string), NULL);
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 1969 "yacc_sql.tab.c"
    break;

  case 73: /* select_attr: ID DOT ID attr_list  */
#line 483 "yacc_sql.y"
                              {
			RelAttr attr;
			relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string), NULL, NULL);
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 1980 "yacc_sql.tab.c"
    break;

  case 74: /* select_attr: aggregation  */
#line 489 "yacc_sql.y"
                      {}
#line 1986 "yacc_sql.tab.c"
    break;

  case 75: /* select_attr: EXPRESSION_STR attr_list  */
#line 490 "yacc_sql.y"
                                   {
		RelAttr attr;
		relation_attr_init(&attr, NULL, (yyvsp[-1].string), NULL, "expression");
		selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
	}
#line 1996 "yacc_sql.tab.c"
    break;

  case 76: /* aggregation: COUNT LBRACE STAR RBRACE attr_list  */
#line 498 "yacc_sql.y"
                                           {
			RelAttr attr;
			relation_attr_init(&attr, NULL, "*", NULL, "count");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 2007 "yacc_sql.tab.c"
    break;

  case 77: /* aggregation: COUNT LBRACE ID RBRACE attr_list  */
#line 504 "yacc_sql.y"
                                           {
			RelAttr attr;
			relation_attr_init(&attr, NULL, (yyvsp[-2].string), NULL, "count");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 2018 "yacc_sql.tab.c"
    break;

  case 78: /* aggregation: COUNT LBRACE ID DOT ID RBRACE attr_list  */
#line 510 "yacc_sql.y"
                                                  {
			RelAttr attr;
			relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string), NULL, "count");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 2029 "yacc_sql.tab.c"
    break;

  case 79: /* aggregation: SUM LBRACE STAR RBRACE attr_list  */
#line 516 "yacc_sql.y"
                                           {
			RelAttr attr;
			relation_attr_init(&attr, NULL, "*", NULL, "sum");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 2040 "yacc_sql.tab.c"
    break;

  case 80: /* aggregation: SUM LBRACE ID RBRACE attr_list  */
#line 522 "yacc_sql.y"
                                         {
			RelAttr attr;
			relation_attr_init(&attr, NULL, (yyvsp[-2].string), NULL, "sum");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 2051 "yacc_sql.tab.c"
    break;

  case 81: /* aggregation: SUM LBRACE ID DOT ID RBRACE attr_list  */
#line 528 "yacc_sql.y"
                                                {
			RelAttr attr;
			relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string), NULL, "sum");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 2062 "yacc_sql.tab.c"
    break;

  case 82: /* aggregation: AVG LBRACE STAR RBRACE attr_list  */
#line 534 "yacc_sql.y"
                                           {
			RelAttr attr;
			relation_attr_init(&attr, NULL, "*", NULL, "avg");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 2073 "yacc_sql.tab.c"
    break;

  case 83: /* aggregation: AVG LBRACE ID RBRACE attr_list  */
#line 540 "yacc_sql.y"
                                         {
			RelAttr attr;
			relation_attr_init(&attr, NULL, (yyvsp[-2].string), NULL, "avg");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 2084 "yacc_sql.tab.c"
    break;

  case 84: /* aggregation: AVG LBRACE ID DOT ID RBRACE attr_list  */
#line 546 "yacc_sql.y"
                                                {
			RelAttr attr;
			relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string), NULL, "avg");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 2095 "yacc_sql.tab.c"
    break;

  case 85: /* aggregation: MAX LBRACE STAR RBRACE attr_list  */
#line 552 "yacc_sql.y"
                                           {
			RelAttr attr;
			relation_attr_init(&attr, NULL, "*", NULL, "max");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 2106 "yacc_sql.tab.c"
    break;

  case 86: /* aggregation: MAX LBRACE ID RBRACE attr_list  */
#line 558 "yacc_sql.y"
                                         {
			RelAttr attr;
			relation_attr_init(&attr, NULL, (yyvsp[-2].string), NULL, "max");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 2117 "yacc_sql.tab.c"
    break;

  case 87: /* aggregation: MAX LBRACE ID DOT ID RBRACE attr_list  */
#line 564 "yacc_sql.y"
                                                {
			RelAttr attr;
			relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string), NULL, "max");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 2128 "yacc_sql.tab.c"
    break;

  case 88: /* aggregation: MIN LBRACE STAR RBRACE attr_list  */
#line 570 "yacc_sql.y"
                                           {
			RelAttr attr;
			relation_attr_init(&attr, NULL, "*", NULL, "min");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 2139 "yacc_sql.tab.c"
    break;

  case 89: /* aggregation: MIN LBRACE ID RBRACE attr_list  */
#line 576 "yacc_sql.y"
                                         {
			RelAttr attr;
			relation_attr_init(&attr, NULL, (yyvsp[-2].string), NULL, "min");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 2150 "yacc_sql.tab.c"
    break;

  case 90: /* aggregation: MIN LBRACE ID DOT ID RBRACE attr_list  */
#line 582 "yacc_sql.y"
                                                {
			RelAttr attr;
			relation_attr_init(&attr, (yyvsp[-4].string), (yyvsp[-2].string), NULL, "min");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
#line 2161 "yacc_sql.tab.c"
    break;

  case 91: /* aggregation: COUNT LBRACE agg_list RBRACE  */
#line 588 "yacc_sql.y"
                                       { return -20; }
#line 2167 "yacc_sql.tab.c"
    break;

  case 92: /* aggregation: MAX LBRACE agg_list RBRACE  */
#line 589 "yacc_sql.y"
                                     { return -20; }
#line 2173 "yacc_sql.tab.c"
    break;

  case 93: /* aggregation: MIN LBRACE agg_list RBRACE  */
#line 590 "yacc_sql.y"
                                     { return -20; }
#line 2179 "yacc_sql.tab.c"
    break;

  case 94: /* aggregation: AVG LBRACE agg_list RBRACE  */
#line 591 "yacc_sql.y"
                                     { return -20; }
#line 2185 "yacc_sql.tab.c"
    break;

  case 96: /* agg_list: STAR COMMA ID  */
#line 595 "yacc_sql.y"
                        {}
#line 2191 "yacc_sql.tab.c"
    break;

  case 97: /* agg_list: ID COMMA ID  */
#line 596 "yacc_sql.y"
                      {}
#line 2197 "yacc_sql.tab.c"
    break;

  case 99: /* attr_list: COMMA select_attr  */
#line 600 "yacc_sql.y"
                        {}
#line 2203 "yacc_sql.tab.c"
    break;

  case 101: /* rel_list: COMMA ID rel_list  */
#line 605 "yacc_sql.y"
                        {	
		selects_append_relation(&CONTEXT->ssql->sstr.selection, (yyvsp[-1].string));
	}
#line 2211 "yacc_sql.tab.c"
    break;

  case 103: /* join_list: join join_list  */
#line 612 "yacc_sql.y"
                         {}
#line 2217 "yacc_sql.tab.c"
    break;

  case 104: /* join: INNER JOIN ID ON join_condition join_condition_list  */
#line 615 "yacc_sql.y"
                                                            {
		Join join;
		join_init(&join, (yyvsp[-3].string), CONTEXT->join_conditions, CONTEXT->join_condition_length);
		CONTEXT->joins[CONTEXT->join_length++] = join;
		CONTEXT->join_condition_length = 0;
	}
#line 2228 "yacc_sql.tab.c"
    break;

  case 106: /* join_condition_list: AND join_condition join_condition_list  */
#line 624 "yacc_sql.y"
                                                 {}
#line 2234 "yacc_sql.tab.c"
    break;

  case 107: /* join_condition: ID DOT ID comOp ID DOT ID  */
#line 627 "yacc_sql.y"
                                          {
			RelAttr left_attr;
			relation_attr_init(&left_attr, (yyvsp[-6].string), (yyvsp[-4].string), NULL, NULL);
			RelAttr right_attr;
			relation_attr_init(&right_attr, (yyvsp[-2].string), (yyvsp[0].string), NULL, NULL);

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 1, &right_attr, NULL);
			CONTEXT->join_conditions[CONTEXT->join_condition_length++] = condition;
		}
#line 2249 "yacc_sql.tab.c"
    break;

  case 108: /* join_condition: value comOp ID DOT ID  */
#line 637 "yacc_sql.y"
                                        {
			Value *left_value = &CONTEXT->values[CONTEXT->value_length - 1];
			RelAttr right_attr;
			relation_attr_init(&right_attr, (yyvsp[-2].string), (yyvsp[0].string), NULL, NULL);

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 1, &right_attr, NULL);
			CONTEXT->join_conditions[CONTEXT->join_condition_length++] = condition;
		}
#line 2263 "yacc_sql.tab.c"
    break;

  case 109: /* join_condition: ID DOT ID comOp value  */
#line 646 "yacc_sql.y"
                                        {
			RelAttr left_attr;
			relation_attr_init(&left_attr, (yyvsp[-4].string), (yyvsp[-2].string), NULL, NULL);
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->join_conditions[CONTEXT->join_condition_length++] = condition;
		}
#line 2277 "yacc_sql.tab.c"
    break;

  case 110: /* join_condition: value comOp value  */
#line 655 "yacc_sql.y"
                                    {
			Value *left_value = &CONTEXT->values[CONTEXT->value_length - 2];
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 0, NULL, right_value);
			CONTEXT->join_conditions[CONTEXT->join_condition_length++] = condition;
		}
#line 2290 "yacc_sql.tab.c"
    break;

  case 112: /* where: WHERE condition condition_list  */
#line 667 "yacc_sql.y"
                                     {	
			}
#line 2297 "yacc_sql.tab.c"
    break;

  case 114: /* condition_list: AND condition condition_list  */
#line 672 "yacc_sql.y"
                                   {
			}
#line 2304 "yacc_sql.tab.c"
    break;

  case 115: /* condition: ID comOp value  */
#line 677 "yacc_sql.y"
                {
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, (yyvsp[-2].string), NULL, NULL);

			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2319 "yacc_sql.tab.c"
    break;

  case 116: /* condition: value comOp value  */
#line 688 "yacc_sql.y"
                {
			Value *left_value = &CONTEXT->values[CONTEXT->value_length - 2];
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 0, NULL, right_value);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2332 "yacc_sql.tab.c"
    break;

  case 117: /* condition: ID comOp ID  */
#line 697 "yacc_sql.y"
                {
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, (yyvsp[-2].string), NULL, NULL);
			RelAttr right_attr;
			relation_attr_init(&right_attr, NULL, (yyvsp[0].string), NULL, NULL);

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 1, &right_attr, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2347 "yacc_sql.tab.c"
    break;

  case 118: /* condition: value comOp ID  */
#line 708 "yacc_sql.y"
                {
			Value *left_value = &CONTEXT->values[CONTEXT->value_length - 1];
			RelAttr right_attr;
			relation_attr_init(&right_attr, NULL, (yyvsp[0].string), NULL, NULL);

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 1, &right_attr, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 2361 "yacc_sql.tab.c"
    break;

  case 119: /* condition: ID DOT ID comOp value  */
#line 718 "yacc_sql.y"
                {
			RelAttr left_attr;
			relation_attr_init(&left_attr, (yyvsp[-4].string), (yyvsp[-2].string), NULL, NULL);
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;				
    }
#line 2375 "yacc_sql.tab.c"
    break;

  case 120: /* condition: value comOp ID DOT ID  */
#line 728 "yacc_sql.y"
                {
			Value *left_value = &CONTEXT->values[CONTEXT->value_length - 1];

			RelAttr right_attr;
			relation_attr_init(&right_attr, (yyvsp[-2].string), (yyvsp[0].string), NULL, NULL);

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 1, &right_attr, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
    }
#line 2390 "yacc_sql.tab.c"
    break;

  case 121: /* condition: ID DOT ID comOp ID DOT ID  */
#line 739 "yacc_sql.y"
                {
			RelAttr left_attr;
			relation_attr_init(&left_attr, (yyvsp[-6].string), (yyvsp[-4].string), NULL, NULL);
			RelAttr right_attr;
			relation_attr_init(&right_attr, (yyvsp[-2].string), (yyvsp[0].string), NULL, NULL);

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 1, &right_attr, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
    }
#line 2405 "yacc_sql.tab.c"
    break;

  case 122: /* condition: ID IN LBRACE value value_list RBRACE  */
#line 749 "yacc_sql.y"
                                               {
		selects_append_subset(&CONTEXT->ssql->sstr.selection, (yyvsp[-5].string), "in", CONTEXT->values, CONTEXT->value_length);
		CONTEXT->value_length = 0;
	}
#line 2414 "yacc_sql.tab.c"
    break;

  case 123: /* condition: ID NOT IN LBRACE value value_list RBRACE  */
#line 753 "yacc_sql.y"
                                                   {
		selects_append_subset(&CONTEXT->ssql->sstr.selection, (yyvsp[-6].string), "not_in", CONTEXT->values, CONTEXT->value_length);
	}
#line 2422 "yacc_sql.tab.c"
    break;

  case 124: /* condition: ID IN SUBQUERY_STR  */
#line 756 "yacc_sql.y"
                             {
		RelAttr left_attr;
		relation_attr_init(&left_attr, NULL, (yyvsp[-2].string), NULL, NULL);
		Value *right_value = (Value *)malloc(sizeof(Value));
  		value_init_string(right_value, (yyvsp[0].string));
		right_value->type = SUBQUERY;
		Condition condition;
		condition_init(&condition, IN_SUBQUERY, 1, &left_attr, NULL, 0, NULL, right_value);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
#line 2437 "yacc_sql.tab.c"
    break;

  case 125: /* condition: ID NOT IN SUBQUERY_STR  */
#line 766 "yacc_sql.y"
                                 {
		RelAttr left_attr;
		relation_attr_init(&left_attr, NULL, (yyvsp[-3].string), NULL, NULL);
		Value *right_value = (Value *)malloc(sizeof(Value));
  		value_init_string(right_value, (yyvsp[0].string));
		right_value->type = SUBQUERY;
		Condition condition;
		condition_init(&condition, NOT_IN_SUBQUERY, 1, &left_attr, NULL, 0, NULL, right_value);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
#line 2452 "yacc_sql.tab.c"
    break;

  case 126: /* condition: ID DOT ID IN SUBQUERY_STR  */
#line 776 "yacc_sql.y"
                                    {
		RelAttr left_attr;
		relation_attr_init(&left_attr, (yyvsp[-4].string), (yyvsp[-2].string), NULL, NULL);
		Value *right_value = (Value *)malloc(sizeof(Value));
  		value_init_string(right_value, (yyvsp[0].string));
		right_value->type = SUBQUERY;
		Condition condition;
		condition_init(&condition, IN_SUBQUERY, 1, &left_attr, NULL, 0, NULL, right_value);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
#line 2467 "yacc_sql.tab.c"
    break;

  case 127: /* condition: ID DOT ID NOT IN SUBQUERY_STR  */
#line 786 "yacc_sql.y"
                                        {
		RelAttr left_attr;
		relation_attr_init(&left_attr, (yyvsp[-5].string), (yyvsp[-3].string), NULL, NULL);
		Value *right_value = (Value *)malloc(sizeof(Value));
  		value_init_string(right_value, (yyvsp[0].string));
		right_value->type = SUBQUERY;
		Condition condition;
		condition_init(&condition, NOT_IN_SUBQUERY, 1, &left_attr, NULL, 0, NULL, right_value);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
#line 2482 "yacc_sql.tab.c"
    break;

  case 128: /* condition: ID comOp SUBQUERY_STR  */
#line 796 "yacc_sql.y"
                                {
		RelAttr left_attr;
		relation_attr_init(&left_attr, NULL, (yyvsp[-2].string), NULL, NULL);
		Value *right_value = (Value *)malloc(sizeof(Value));
  		value_init_string(right_value, (yyvsp[0].string));
		right_value->type = SUBQUERY;
		Condition condition;
		condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
#line 2497 "yacc_sql.tab.c"
    break;

  case 129: /* condition: SUBQUERY_STR comOp ID  */
#line 806 "yacc_sql.y"
                                {
		Value *left_value = (Value *)malloc(sizeof(Value));
  		value_init_string(left_value, (yyvsp[-2].string));
		left_value->type = SUBQUERY;
		RelAttr right_attr;
		relation_attr_init(&right_attr, NULL, (yyvsp[0].string), NULL, NULL);
		Condition condition;
		condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 1, &right_attr, NULL);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
#line 2512 "yacc_sql.tab.c"
    break;

  case 130: /* condition: ID DOT ID comOp SUBQUERY_STR  */
#line 816 "yacc_sql.y"
                                       {
		RelAttr left_attr;
		relation_attr_init(&left_attr, (yyvsp[-4].string), (yyvsp[-2].string), NULL, NULL);
		Value *right_value = (Value *)malloc(sizeof(Value));
  		value_init_string(right_value, (yyvsp[0].string));
		right_value->type = SUBQUERY;
		Condition condition;
		condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
#line 2527 "yacc_sql.tab.c"
    break;

  case 131: /* condition: SUBQUERY_STR comOp ID DOT ID  */
#line 826 "yacc_sql.y"
                                      {
		Value *left_value = (Value *)malloc(sizeof(Value));
  		value_init_string(left_value, (yyvsp[-4].string));
		left_value->type = SUBQUERY;
		RelAttr right_attr;
		relation_attr_init(&right_attr, (yyvsp[-2].string), (yyvsp[0].string), NULL, NULL);
		Condition condition;
		condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 1, &right_attr, NULL);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
#line 2542 "yacc_sql.tab.c"
    break;

  case 132: /* condition: SUBQUERY_STR comOp SUBQUERY_STR  */
#line 836 "yacc_sql.y"
                                          {
		Value *left_value = (Value *)malloc(sizeof(Value));
  		value_init_string(left_value, (yyvsp[-2].string));
		left_value->type = SUBQUERY;
		Value *right_value = (Value *)malloc(sizeof(Value));
		value_init_string(right_value, (yyvsp[0].string));
		right_value->type = SUBQUERY;

		Condition condition;
		condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 0, NULL, right_value);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
#line 2559 "yacc_sql.tab.c"
    break;

  case 134: /* order: ORDER BY order_by order_by_list  */
#line 868 "yacc_sql.y"
                                          {}
#line 2565 "yacc_sql.tab.c"
    break;

  case 136: /* order_by_list: COMMA order_by order_by_list  */
#line 872 "yacc_sql.y"
                                       {}
#line 2571 "yacc_sql.tab.c"
    break;

  case 138: /* order_by: ID  */
#line 876 "yacc_sql.y"
          {
		RelAttr attr;
		relation_attr_init(&attr, NULL, (yyvsp[0].string), "asc", NULL);
		selects_append_order_by(&CONTEXT->ssql->sstr.selection, &attr);
	}
#line 2581 "yacc_sql.tab.c"
    break;

  case 139: /* order_by: ID ASC  */
#line 881 "yacc_sql.y"
                  {
		RelAttr attr;
		relation_attr_init(&attr, NULL, (yyvsp[-1].string), "asc", NULL);
		selects_append_order_by(&CONTEXT->ssql->sstr.selection, &attr);
	}
#line 2591 "yacc_sql.tab.c"
    break;

  case 140: /* order_by: ID DESC  */
#line 886 "yacc_sql.y"
                    {
		RelAttr attr;
		relation_attr_init(&attr, NULL, (yyvsp[-1].string), "desc", NULL);
		selects_append_order_by(&CONTEXT->ssql->sstr.selection, &attr);
	}
#line 2601 "yacc_sql.tab.c"
    break;

  case 141: /* order_by: ID DOT ID  */
#line 891 "yacc_sql.y"
                      {
		RelAttr attr;
		relation_attr_init(&attr, (yyvsp[-2].string), (yyvsp[0].string), "asc", NULL);
		selects_append_order_by(&CONTEXT->ssql->sstr.selection, &attr);
	}
#line 2611 "yacc_sql.tab.c"
    break;

  case 142: /* order_by: ID DOT ID ASC  */
#line 896 "yacc_sql.y"
                          {
		RelAttr attr;
		relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string), "asc", NULL);
		selects_append_order_by(&CONTEXT->ssql->sstr.selection, &attr);
	}
#line 2621 "yacc_sql.tab.c"
    break;

  case 143: /* order_by: ID DOT ID DESC  */
#line 901 "yacc_sql.y"
                          {
		RelAttr attr;
		relation_attr_init(&attr, (yyvsp[-3].string), (yyvsp[-1].string), "desc", NULL);
		selects_append_order_by(&CONTEXT->ssql->sstr.selection, &attr);
	}
#line 2631 "yacc_sql.tab.c"
    break;

  case 145: /* group: GROUP BY group_by group_by_list group_having  */
#line 909 "yacc_sql.y"
                                                       {}
#line 2637 "yacc_sql.tab.c"
    break;

  case 147: /* group_by_list: COMMA group_by group_by_list  */
#line 913 "yacc_sql.y"
                                       {}
#line 2643 "yacc_sql.tab.c"
    break;

  case 149: /* group_by: ID  */
#line 917 "yacc_sql.y"
              {
		RelAttr attr;
		relation_attr_init(&attr, NULL, (yyvsp[0].string), NULL, NULL);
		selects_append_group_by(&CONTEXT->ssql->sstr.selection, &attr);
	}
#line 2653 "yacc_sql.tab.c"
    break;

  case 150: /* group_by: ID DOT ID  */
#line 922 "yacc_sql.y"
                      {
		RelAttr attr;
		relation_attr_init(&attr, (yyvsp[-2].string), (yyvsp[0].string), NULL, NULL);
		selects_append_group_by(&CONTEXT->ssql->sstr.selection, &attr);
	}
#line 2663 "yacc_sql.tab.c"
    break;

  case 152: /* group_having: HAVING COUNT LBRACE STAR RBRACE comOp value  */
#line 929 "yacc_sql.y"
                                                     {
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, "*", NULL, "count");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
#line 2676 "yacc_sql.tab.c"
    break;

  case 153: /* group_having: HAVING COUNT LBRACE ID RBRACE comOp value  */
#line 937 "yacc_sql.y"
                                                   {
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, (yyvsp[-3].string), NULL, "count");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
#line 2689 "yacc_sql.tab.c"
    break;

  case 154: /* group_having: HAVING COUNT LBRACE ID DOT ID RBRACE comOp value  */
#line 945 "yacc_sql.y"
                                                          {
			RelAttr left_attr;
			relation_attr_init(&left_attr, (yyvsp[-5].string), (yyvsp[-3].string), NULL, "count");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
#line 2702 "yacc_sql.tab.c"
    break;

  case 155: /* group_having: HAVING SUM LBRACE ID RBRACE comOp value  */
#line 953 "yacc_sql.y"
                                                 {
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, (yyvsp[-3].string), NULL, "sum");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
#line 2715 "yacc_sql.tab.c"
    break;

  case 156: /* group_having: HAVING SUM LBRACE ID DOT ID RBRACE comOp value  */
#line 961 "yacc_sql.y"
                                                        {
			RelAttr left_attr;
			relation_attr_init(&left_attr, (yyvsp[-5].string), (yyvsp[-3].string), NULL, "sum");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
#line 2728 "yacc_sql.tab.c"
    break;

  case 157: /* group_having: HAVING AVG LBRACE ID RBRACE comOp value  */
#line 969 "yacc_sql.y"
                                                 {
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, (yyvsp[-3].string), NULL, "avg");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
#line 2741 "yacc_sql.tab.c"
    break;

  case 158: /* group_having: HAVING AVG LBRACE ID DOT ID RBRACE comOp value  */
#line 977 "yacc_sql.y"
                                                        {
			RelAttr left_attr;
			relation_attr_init(&left_attr, (yyvsp[-5].string), (yyvsp[-3].string), NULL, "avg");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
#line 2754 "yacc_sql.tab.c"
    break;

  case 159: /* group_having: HAVING MAX LBRACE ID RBRACE comOp value  */
#line 985 "yacc_sql.y"
                                                 {
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, (yyvsp[-3].string), NULL, "max");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
#line 2767 "yacc_sql.tab.c"
    break;

  case 160: /* group_having: HAVING MAX LBRACE ID DOT ID RBRACE comOp value  */
#line 993 "yacc_sql.y"
                                                        {
			RelAttr left_attr;
			relation_attr_init(&left_attr, (yyvsp[-5].string), (yyvsp[-3].string), NULL, "max");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
#line 2780 "yacc_sql.tab.c"
    break;

  case 161: /* group_having: HAVING MIN LBRACE ID RBRACE comOp value  */
#line 1001 "yacc_sql.y"
                                                 {
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, (yyvsp[-3].string), NULL, "min");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
#line 2793 "yacc_sql.tab.c"
    break;

  case 162: /* group_having: HAVING MIN LBRACE ID DOT ID RBRACE comOp value  */
#line 1009 "yacc_sql.y"
                                                        {
			RelAttr left_attr;
			relation_attr_init(&left_attr, (yyvsp[-5].string), (yyvsp[-3].string), NULL, "min");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
#line 2806 "yacc_sql.tab.c"
    break;

  case 163: /* group_having: HAVING COUNT LBRACE agg_list RBRACE  */
#line 1017 "yacc_sql.y"
                                             { return -20; }
#line 2812 "yacc_sql.tab.c"
    break;

  case 164: /* group_having: HAVING MAX LBRACE agg_list RBRACE  */
#line 1018 "yacc_sql.y"
                                           { return -20; }
#line 2818 "yacc_sql.tab.c"
    break;

  case 165: /* group_having: HAVING MIN LBRACE agg_list RBRACE  */
#line 1019 "yacc_sql.y"
                                           { return -20; }
#line 2824 "yacc_sql.tab.c"
    break;

  case 166: /* group_having: HAVING AVG LBRACE agg_list RBRACE  */
#line 1020 "yacc_sql.y"
                                           { return -20; }
#line 2830 "yacc_sql.tab.c"
    break;

  case 167: /* comOp: EQ  */
#line 1024 "yacc_sql.y"
             { CONTEXT->comp = EQUAL_TO; }
#line 2836 "yacc_sql.tab.c"
    break;

  case 168: /* comOp: LT  */
#line 1025 "yacc_sql.y"
         { CONTEXT->comp = LESS_THAN; }
#line 2842 "yacc_sql.tab.c"
    break;

  case 169: /* comOp: GT  */
#line 1026 "yacc_sql.y"
         { CONTEXT->comp = GREAT_THAN; }
#line 2848 "yacc_sql.tab.c"
    break;

  case 170: /* comOp: LE  */
#line 1027 "yacc_sql.y"
         { CONTEXT->comp = LESS_EQUAL; }
#line 2854 "yacc_sql.tab.c"
    break;

  case 171: /* comOp: GE  */
#line 1028 "yacc_sql.y"
         { CONTEXT->comp = GREAT_EQUAL; }
#line 2860 "yacc_sql.tab.c"
    break;

  case 172: /* comOp: NE  */
#line 1029 "yacc_sql.y"
         { CONTEXT->comp = NOT_EQUAL; }
#line 2866 "yacc_sql.tab.c"
    break;

  case 173: /* comOp: LK  */
#line 1030 "yacc_sql.y"
             { CONTEXT->comp = LIKE; }
#line 2872 "yacc_sql.tab.c"
    break;

  case 174: /* comOp: NOT LK  */
#line 1031 "yacc_sql.y"
                 { CONTEXT->comp = NOT_LIKE; }
#line 2878 "yacc_sql.tab.c"
    break;

  case 175: /* comOp: IS  */
#line 1032 "yacc_sql.y"
             { CONTEXT->comp = IS_NULL; }
#line 2884 "yacc_sql.tab.c"
    break;

  case 176: /* comOp: IS NOT  */
#line 1033 "yacc_sql.y"
                 { CONTEXT->comp = IS_NOT_NULL; }
#line 2890 "yacc_sql.tab.c"
    break;

  case 177: /* load_data: LOAD DATA INFILE SSS INTO TABLE ID SEMICOLON  */
#line 1038 "yacc_sql.y"
                {
		  CONTEXT->ssql->flag = SCF_LOAD_DATA;
			load_data_init(&CONTEXT->ssql->sstr.load_data, (yyvsp[-1].string), (yyvsp[-4].string));
		}
#line 2899 "yacc_sql.tab.c"
    break;


#line 2903 "yacc_sql.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (scanner, YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, scanner);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (scanner, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 1043 "yacc_sql.y"

//_____________________________________________________________________
extern void scan_string(const char *str, yyscan_t scanner);

int sql_parse(const char *s, Query *sqls){
	ParserContext context;
	memset(&context, 0, sizeof(context));

	yyscan_t scanner;
	yylex_init_extra(&context, &scanner);
	context.ssql = sqls;
	scan_string(s, scanner);
	int result = yyparse(scanner);
	yylex_destroy(scanner);
	return result;
}
