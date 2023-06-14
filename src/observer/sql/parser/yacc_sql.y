
%{

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

%}

%define api.pure full
%lex-param { yyscan_t scanner }
%parse-param { void *scanner }

//标识tokens
%token  SEMICOLON
        CREATE
        DROP
        TABLE
        TABLES
		NULLABLE
		NULL_T
		IS
        INDEX
        SELECT
        SHOW
        SYNC
        INSERT
        DELETE
        UPDATE
        LBRACE
        RBRACE
        COMMA
        TRX_BEGIN
        TRX_COMMIT
        TRX_ROLLBACK
        INT_T
        STRING_T
        FLOAT_T
		DATE_T
		TEXT_T
        HELP
        EXIT
        DOT //QUOTE
        INTO
        VALUES
        FROM
        WHERE
		IN
		EXISTS
        AND
        SET
        ON
        LOAD
        DATA
        INFILE
		COUNT
		SUM
		MAX
		MIN
		AVG
        EQ
        LT
        GT
        LE
        GE
        NE
		LK
		NOT
		UNIQUE
		AS
		ORDER
		BY
		GROUP
		HAVING
		ASC
		DESC
		INNER
		JOIN

%union {
  struct _Attr *attr;
  struct _Condition *condition1;
  struct _Value *value1;
  char *string;
  int number;
  float floats;
	char *position;
}

%token <number> NUMBER
%token <floats> FLOAT 
%token <string> ID
%token <string> PATH
%token <string> DATE_STR
%token <string> SSS
%token <string> SUBQUERY_STR
%token <string> EXPRESSION_STR
%token <string> STAR
%token <string> STRING_V
//非终结符

%type <number> type;
%type <condition1> condition;
%type <value1> value;
%type <number> number;

%%

commands:		//commands or sqls. parser starts here.
    /* empty */
    | commands command
    ;

command:
	  select  
	| insert
	| update
	| delete
	| create_table
	| drop_table
	| show_tables
	| desc_table
	| create_index	
	| drop_index
	| show_index
	| sync
	| begin
	| commit
	| rollback
	| load_data
	| help
	| exit
    ;

exit:			
    EXIT SEMICOLON {
        CONTEXT->ssql->flag=SCF_EXIT;//"exit";
    };

help:
    HELP SEMICOLON {
        CONTEXT->ssql->flag=SCF_HELP;//"help";
    };

sync:
    SYNC SEMICOLON {
      CONTEXT->ssql->flag = SCF_SYNC;
    }
    ;

begin:
    TRX_BEGIN SEMICOLON {
      CONTEXT->ssql->flag = SCF_BEGIN;
    }
    ;

commit:
    TRX_COMMIT SEMICOLON {
      CONTEXT->ssql->flag = SCF_COMMIT;
    }
    ;

rollback:
    TRX_ROLLBACK SEMICOLON {
      CONTEXT->ssql->flag = SCF_ROLLBACK;
    }
    ;

drop_table:		/*drop table 语句的语法解析树*/
    DROP TABLE ID SEMICOLON {
        CONTEXT->ssql->flag = SCF_DROP_TABLE;//"drop_table";
        drop_table_init(&CONTEXT->ssql->sstr.drop_table, $3);
    };

show_tables:
    SHOW TABLES SEMICOLON {
      CONTEXT->ssql->flag = SCF_SHOW_TABLES;
    }
    ;

desc_table:
    DESC ID SEMICOLON {
      CONTEXT->ssql->flag = SCF_DESC_TABLE;
      desc_table_init(&CONTEXT->ssql->sstr.desc_table, $2);
    }
    ;

create_index:		/*create index 语句的语法解析树*/
    CREATE index_type ID ON ID LBRACE ID index_col_list RBRACE SEMICOLON 
		{
			CONTEXT->ssql->flag = SCF_CREATE_INDEX;//"create_index";
			char *temp=$7; 
			create_index_append_attribute(&CONTEXT->ssql->sstr.create_index, temp);
			create_index_init(&CONTEXT->ssql->sstr.create_index, $3, $5);
		}
    ;
index_type:
	INDEX {
		create_index_unique(&CONTEXT->ssql->sstr.create_index, 0);
	}
	| UNIQUE INDEX {
		create_index_unique(&CONTEXT->ssql->sstr.create_index, 1);
	}
	;
index_col_list:
	/* empty */
	| COMMA ID index_col_list {
		char *temp=$2; 
		create_index_append_attribute(&CONTEXT->ssql->sstr.create_index, temp);
	}
	;

drop_index:			/*drop index 语句的语法解析树*/
    DROP INDEX ID  SEMICOLON 
		{
			CONTEXT->ssql->flag=SCF_DROP_INDEX;//"drop_index";
			drop_index_init(&CONTEXT->ssql->sstr.drop_index, $3);
		}
    ;

show_index:
    SHOW INDEX FROM ID SEMICOLON {
      CONTEXT->ssql->flag = SCF_SHOW_INDEX;
	  show_index_init(&CONTEXT->ssql->sstr.show_index, $4);
    }
    ;
	
create_table:		/*create table 语句的语法解析树*/
    CREATE TABLE ID LBRACE attr_def attr_def_list RBRACE SEMICOLON 
		{
			CONTEXT->ssql->flag=SCF_CREATE_TABLE;//"create_table";
			// CONTEXT->ssql->sstr.create_table.attribute_count = CONTEXT->value_length;
			create_table_init_name(&CONTEXT->ssql->sstr.create_table, $3);
			//临时变量清零	
			CONTEXT->value_length = 0;
		}
    ;
attr_def_list:
    /* empty */
    | COMMA attr_def attr_def_list {    }
    ;
    
attr_def:
    ID_get type LBRACE number RBRACE 
		{
			AttrInfo attribute;
			attr_info_init(&attribute, CONTEXT->id, $2, $4, 0);
			create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
			CONTEXT->value_length++;
		}
    |ID_get type
		{
			AttrInfo attribute;
			int len = 4;
			if($2 == TEXTS) len = 4096;
			attr_info_init(&attribute, CONTEXT->id, $2, len, 0);
			create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
			CONTEXT->value_length++;
		}
    |ID_get type NULLABLE
		{
			AttrInfo attribute;
			int len = 4;
			if($2 == TEXTS) len = 4096;
			attr_info_init(&attribute, CONTEXT->id, $2, len, 1);
			create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
			CONTEXT->value_length++;
		}
    |ID_get type NOT NULL_T
		{
			AttrInfo attribute;
			int len = 4;
			if($2 == TEXTS) len = 4096;
			attr_info_init(&attribute, CONTEXT->id, $2, len, 0);
			create_table_append_attribute(&CONTEXT->ssql->sstr.create_table, &attribute);
			CONTEXT->value_length++;
		}
    ;
    ;
number:
		NUMBER {$$ = $1;}
		;
type:
	INT_T { $$=INTS; }
       | STRING_T { $$=CHARS; }
       | FLOAT_T { $$=FLOATS; }
	   | DATE_T { $$=DATES; }
	   | TEXT_T { $$=TEXTS; }
       ;
ID_get:
	ID 
	{
		char *temp=$1; 
		snprintf(CONTEXT->id, sizeof(CONTEXT->id), "%s", temp);
	}
	;

	
insert:				/*insert   语句的语法解析树*/
    INSERT INTO ID VALUES LBRACE value value_list RBRACE more_value_list SEMICOLON 
	{
			CONTEXT->ssql->flag=SCF_INSERT;//"insert";
			inserts_init(&CONTEXT->ssql->sstr.insertion, $3, CONTEXT->values, CONTEXT->value_length);
      //临时变量清零
      CONTEXT->value_length=0;
    }

value_list:
    /* empty */
    | COMMA value value_list  {}
    ;
more_value_list:
	/* empty */
	| COMMA LBRACE value value_list RBRACE more_value_list {}
	;
value:
    NUMBER{	
  		value_init_integer(&CONTEXT->values[CONTEXT->value_length++], $1);
		}
    |FLOAT{
  		value_init_float(&CONTEXT->values[CONTEXT->value_length++], $1);
		}
    |SSS {
			$1 = substr($1,1,strlen($1)-2);
  		value_init_string(&CONTEXT->values[CONTEXT->value_length++], $1);
		}
	|DATE_STR{
			$1 = substr($1,1,strlen($1)-2);
  		int ret = value_init_date(&CONTEXT->values[CONTEXT->value_length++], $1);
		if(ret < 0) return -10;		
	}
	|NULL_T {
		value_init_null(&CONTEXT->values[CONTEXT->value_length++]);
	}
	|EXPRESSION_STR {
		value_init_expression(&CONTEXT->values[CONTEXT->value_length++], $1);
	}
    ;
    
delete:		/*  delete 语句的语法解析树*/
    DELETE FROM ID where SEMICOLON 
		{
			CONTEXT->ssql->flag = SCF_DELETE;//"delete";
			deletes_init_relation(&CONTEXT->ssql->sstr.deletion, $3);
			deletes_set_conditions(&CONTEXT->ssql->sstr.deletion, 
					CONTEXT->conditions, CONTEXT->condition_length);
			CONTEXT->condition_length = 0;	
    }
    ;
update:			/*  update 语句的语法解析树*/
    UPDATE ID SET update_col update_list where SEMICOLON
		{
			CONTEXT->ssql->flag = SCF_UPDATE;//"update";
			// Value *value = &CONTEXT->values[0];
			// updates_init(&CONTEXT->ssql->sstr.update, $2, $4, value, CONTEXT->conditions, CONTEXT->condition_length);
			updates_init(&CONTEXT->ssql->sstr.update, $2, CONTEXT->conditions, CONTEXT->condition_length);
			CONTEXT->condition_length = 0;
			CONTEXT->value_length = 0;
		}
    ;
update_list:
	/* empty */
	| COMMA update_col update_list {}
	;
update_col:
	ID EQ value {
		Value *value = &CONTEXT->values[CONTEXT->value_length - 1];
		update_append_value(&CONTEXT->ssql->sstr.update, $1, value);
	}
	| ID EQ SUBQUERY_STR {
		Value *value = (Value*)malloc(sizeof(Value));
		value_init_string(value, $3);
		value->type = SUBQUERY;
		update_append_value(&CONTEXT->ssql->sstr.update, $1, value);
	}
	//| ID EQ LBRACE select RBRACE {
	//	update_append_subquery(&CONTEXT->ssql->sstr.update, $1, &CONTEXT->ssql->sstr.selection);
	//}
	;

select:				/*  select 语句的语法解析树*/
    SELECT select_attr FROM ID rel_list join_list where group order SEMICOLON
	{
			selects_append_relation(&CONTEXT->ssql->sstr.selection, $4);
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
	;

select_attr:
    STAR attr_list{  
			RelAttr attr;
			relation_attr_init(&attr, NULL, "*", NULL, NULL);
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
    | ID attr_list {
			RelAttr attr;
			relation_attr_init(&attr, NULL, $1, NULL, NULL);
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
    | ID AS ID attr_list {
			RelAttr attr;
			relation_attr_init(&attr, NULL, $1, $3, NULL);
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
  	| ID DOT ID attr_list {
			RelAttr attr;
			relation_attr_init(&attr, $1, $3, NULL, NULL);
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
	| aggregation {}
	| EXPRESSION_STR attr_list {
		RelAttr attr;
		relation_attr_init(&attr, NULL, $1, NULL, "expression");
		selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
	}
	;

aggregation:
	COUNT LBRACE STAR RBRACE attr_list {
			RelAttr attr;
			relation_attr_init(&attr, NULL, "*", NULL, "count");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
	| COUNT LBRACE ID RBRACE attr_list {
			RelAttr attr;
			relation_attr_init(&attr, NULL, $3, NULL, "count");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
	| COUNT LBRACE ID DOT ID RBRACE attr_list {
			RelAttr attr;
			relation_attr_init(&attr, $3, $5, NULL, "count");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
	| SUM LBRACE STAR RBRACE attr_list {
			RelAttr attr;
			relation_attr_init(&attr, NULL, "*", NULL, "sum");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
	| SUM LBRACE ID RBRACE attr_list {
			RelAttr attr;
			relation_attr_init(&attr, NULL, $3, NULL, "sum");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
	| SUM LBRACE ID DOT ID RBRACE attr_list {
			RelAttr attr;
			relation_attr_init(&attr, $3, $5, NULL, "sum");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
	| AVG LBRACE STAR RBRACE attr_list {
			RelAttr attr;
			relation_attr_init(&attr, NULL, "*", NULL, "avg");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
	| AVG LBRACE ID RBRACE attr_list {
			RelAttr attr;
			relation_attr_init(&attr, NULL, $3, NULL, "avg");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
	| AVG LBRACE ID DOT ID RBRACE attr_list {
			RelAttr attr;
			relation_attr_init(&attr, $3, $5, NULL, "avg");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
	| MAX LBRACE STAR RBRACE attr_list {
			RelAttr attr;
			relation_attr_init(&attr, NULL, "*", NULL, "max");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
	| MAX LBRACE ID RBRACE attr_list {
			RelAttr attr;
			relation_attr_init(&attr, NULL, $3, NULL, "max");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
	| MAX LBRACE ID DOT ID RBRACE attr_list {
			RelAttr attr;
			relation_attr_init(&attr, $3, $5, NULL, "max");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
	| MIN LBRACE STAR RBRACE attr_list {
			RelAttr attr;
			relation_attr_init(&attr, NULL, "*", NULL, "min");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
	| MIN LBRACE ID RBRACE attr_list {
			RelAttr attr;
			relation_attr_init(&attr, NULL, $3, NULL, "min");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
	| MIN LBRACE ID DOT ID RBRACE attr_list {
			RelAttr attr;
			relation_attr_init(&attr, $3, $5, NULL, "min");
			selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
			//CONTEXT->attr[CONTEXT->attr_length++] = attr;
		}
	| COUNT LBRACE agg_list RBRACE { return -20; }
	| MAX LBRACE agg_list RBRACE { return -20; }
	| MIN LBRACE agg_list RBRACE { return -20; }
	| AVG LBRACE agg_list RBRACE { return -20; }
    ;
agg_list:
	/* empty */
	| STAR COMMA ID {}
	| ID COMMA ID {}
	;
attr_list:
    /* empty */
    | COMMA select_attr {}
	;

rel_list:
    /* empty */
    | COMMA ID rel_list {	
		selects_append_relation(&CONTEXT->ssql->sstr.selection, $2);
	}
    ;

join_list:
	/* empty */ 
	| join join_list {}
	;
join:
	INNER JOIN ID ON join_condition join_condition_list {
		Join join;
		join_init(&join, $3, CONTEXT->join_conditions, CONTEXT->join_condition_length);
		CONTEXT->joins[CONTEXT->join_length++] = join;
		CONTEXT->join_condition_length = 0;
	}
	;
join_condition_list:
	/* empty */ 
	| AND join_condition join_condition_list {}
	;
join_condition:
		ID DOT ID comOp ID DOT ID {
			RelAttr left_attr;
			relation_attr_init(&left_attr, $1, $3, NULL, NULL);
			RelAttr right_attr;
			relation_attr_init(&right_attr, $5, $7, NULL, NULL);

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 1, &right_attr, NULL);
			CONTEXT->join_conditions[CONTEXT->join_condition_length++] = condition;
		}
		| value comOp ID DOT ID {
			Value *left_value = &CONTEXT->values[CONTEXT->value_length - 1];
			RelAttr right_attr;
			relation_attr_init(&right_attr, $3, $5, NULL, NULL);

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 1, &right_attr, NULL);
			CONTEXT->join_conditions[CONTEXT->join_condition_length++] = condition;
		}
		| ID DOT ID comOp value {
			RelAttr left_attr;
			relation_attr_init(&left_attr, $1, $3, NULL, NULL);
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->join_conditions[CONTEXT->join_condition_length++] = condition;
		}
		| value comOp value {
			Value *left_value = &CONTEXT->values[CONTEXT->value_length - 2];
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 0, NULL, right_value);
			CONTEXT->join_conditions[CONTEXT->join_condition_length++] = condition;
		}
		;

where:
    /* empty */ 
    | WHERE condition condition_list {	
			}
    ;
condition_list:
    /* empty */
    | AND condition condition_list {
			}
    ;
condition:
    ID comOp value 
		{
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, $1, NULL, NULL);

			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
		|value comOp value 
		{
			Value *left_value = &CONTEXT->values[CONTEXT->value_length - 2];
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 0, NULL, right_value);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
		|ID comOp ID 
		{
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, $1, NULL, NULL);
			RelAttr right_attr;
			relation_attr_init(&right_attr, NULL, $3, NULL, NULL);

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 1, &right_attr, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
    |value comOp ID
		{
			Value *left_value = &CONTEXT->values[CONTEXT->value_length - 1];
			RelAttr right_attr;
			relation_attr_init(&right_attr, NULL, $3, NULL, NULL);

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 1, &right_attr, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
    |ID DOT ID comOp value
		{
			RelAttr left_attr;
			relation_attr_init(&left_attr, $1, $3, NULL, NULL);
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;				
    }
    |value comOp ID DOT ID
		{
			Value *left_value = &CONTEXT->values[CONTEXT->value_length - 1];

			RelAttr right_attr;
			relation_attr_init(&right_attr, $3, $5, NULL, NULL);

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 1, &right_attr, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
    }
    |ID DOT ID comOp ID DOT ID
		{
			RelAttr left_attr;
			relation_attr_init(&left_attr, $1, $3, NULL, NULL);
			RelAttr right_attr;
			relation_attr_init(&right_attr, $5, $7, NULL, NULL);

			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 1, &right_attr, NULL);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
    }
	| ID IN LBRACE value value_list RBRACE {
		selects_append_subset(&CONTEXT->ssql->sstr.selection, $1, "in", CONTEXT->values, CONTEXT->value_length);
		CONTEXT->value_length = 0;
	}
	| ID NOT IN LBRACE value value_list RBRACE {
		selects_append_subset(&CONTEXT->ssql->sstr.selection, $1, "not_in", CONTEXT->values, CONTEXT->value_length);
	}
	| ID IN SUBQUERY_STR {
		RelAttr left_attr;
		relation_attr_init(&left_attr, NULL, $1, NULL, NULL);
		Value *right_value = (Value *)malloc(sizeof(Value));
  		value_init_string(right_value, $3);
		right_value->type = SUBQUERY;
		Condition condition;
		condition_init(&condition, IN_SUBQUERY, 1, &left_attr, NULL, 0, NULL, right_value);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
	| ID NOT IN SUBQUERY_STR {
		RelAttr left_attr;
		relation_attr_init(&left_attr, NULL, $1, NULL, NULL);
		Value *right_value = (Value *)malloc(sizeof(Value));
  		value_init_string(right_value, $4);
		right_value->type = SUBQUERY;
		Condition condition;
		condition_init(&condition, NOT_IN_SUBQUERY, 1, &left_attr, NULL, 0, NULL, right_value);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
	| ID DOT ID IN SUBQUERY_STR {
		RelAttr left_attr;
		relation_attr_init(&left_attr, $1, $3, NULL, NULL);
		Value *right_value = (Value *)malloc(sizeof(Value));
  		value_init_string(right_value, $5);
		right_value->type = SUBQUERY;
		Condition condition;
		condition_init(&condition, IN_SUBQUERY, 1, &left_attr, NULL, 0, NULL, right_value);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
	| ID DOT ID NOT IN SUBQUERY_STR {
		RelAttr left_attr;
		relation_attr_init(&left_attr, $1, $3, NULL, NULL);
		Value *right_value = (Value *)malloc(sizeof(Value));
  		value_init_string(right_value, $6);
		right_value->type = SUBQUERY;
		Condition condition;
		condition_init(&condition, NOT_IN_SUBQUERY, 1, &left_attr, NULL, 0, NULL, right_value);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
	| ID comOp SUBQUERY_STR {
		RelAttr left_attr;
		relation_attr_init(&left_attr, NULL, $1, NULL, NULL);
		Value *right_value = (Value *)malloc(sizeof(Value));
  		value_init_string(right_value, $3);
		right_value->type = SUBQUERY;
		Condition condition;
		condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
	| SUBQUERY_STR comOp ID {
		Value *left_value = (Value *)malloc(sizeof(Value));
  		value_init_string(left_value, $1);
		left_value->type = SUBQUERY;
		RelAttr right_attr;
		relation_attr_init(&right_attr, NULL, $3, NULL, NULL);
		Condition condition;
		condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 1, &right_attr, NULL);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
	| ID DOT ID comOp SUBQUERY_STR {
		RelAttr left_attr;
		relation_attr_init(&left_attr, $1, $3, NULL, NULL);
		Value *right_value = (Value *)malloc(sizeof(Value));
  		value_init_string(right_value, $5);
		right_value->type = SUBQUERY;
		Condition condition;
		condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
	| SUBQUERY_STR comOp ID DOT ID{
		Value *left_value = (Value *)malloc(sizeof(Value));
  		value_init_string(left_value, $1);
		left_value->type = SUBQUERY;
		RelAttr right_attr;
		relation_attr_init(&right_attr, $3, $5, NULL, NULL);
		Condition condition;
		condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 1, &right_attr, NULL);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
	| SUBQUERY_STR comOp SUBQUERY_STR {
		Value *left_value = (Value *)malloc(sizeof(Value));
  		value_init_string(left_value, $1);
		left_value->type = SUBQUERY;
		Value *right_value = (Value *)malloc(sizeof(Value));
		value_init_string(right_value, $3);
		right_value->type = SUBQUERY;

		Condition condition;
		condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 0, NULL, right_value);
		CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	}
	//| SUBQUERY_STR {
	//	char* str1;
	//	char* str2;
	//	CompOp compop;
	//	str1 = subquery_split_str1($1, str1, str2, &compop);
	//	str2 = subquery_split_str2($1, str1, str2, &compop);
	//	Value *left_value = (Value *)malloc(sizeof(Value));
	//	left_value->data = strdup(str1);
	//	left_value->type = SUBQUERY;
	//	Value *right_value = (Value *)malloc(sizeof(Value));
	//	right_value->data = strdup(str2);
	//	right_value->type = SUBQUERY;
	//	Condition condition;
	//	condition_init(&condition, compop, 0, NULL, left_value, 0, NULL, right_value);
	//	CONTEXT->conditions[CONTEXT->condition_length++] = condition;
	//}
    ;

order:
	/* empty */ 
	| ORDER BY order_by order_by_list {}
	;
order_by_list:
	/* empty */ 
	| COMMA order_by order_by_list {}
	;
order_by:
    /* empty */ 
    | ID  {
		RelAttr attr;
		relation_attr_init(&attr, NULL, $1, "asc", NULL);
		selects_append_order_by(&CONTEXT->ssql->sstr.selection, &attr);
	}
	| ID ASC  {
		RelAttr attr;
		relation_attr_init(&attr, NULL, $1, "asc", NULL);
		selects_append_order_by(&CONTEXT->ssql->sstr.selection, &attr);
	}
	|  ID DESC  {
		RelAttr attr;
		relation_attr_init(&attr, NULL, $1, "desc", NULL);
		selects_append_order_by(&CONTEXT->ssql->sstr.selection, &attr);
	}
	|  ID DOT ID  {
		RelAttr attr;
		relation_attr_init(&attr, $1, $3, "asc", NULL);
		selects_append_order_by(&CONTEXT->ssql->sstr.selection, &attr);
	}
	|  ID DOT ID ASC  {
		RelAttr attr;
		relation_attr_init(&attr, $1, $3, "asc", NULL);
		selects_append_order_by(&CONTEXT->ssql->sstr.selection, &attr);
	}
	| ID DOT ID DESC  {
		RelAttr attr;
		relation_attr_init(&attr, $1, $3, "desc", NULL);
		selects_append_order_by(&CONTEXT->ssql->sstr.selection, &attr);
	}
	;
group:
	/* empty */ 
	| GROUP BY group_by group_by_list group_having {}
	;
group_by_list:
	/* empty */ 
	| COMMA group_by group_by_list {}
	;
group_by:
    /* empty */ 
	| ID  {
		RelAttr attr;
		relation_attr_init(&attr, NULL, $1, NULL, NULL);
		selects_append_group_by(&CONTEXT->ssql->sstr.selection, &attr);
	}
	|  ID DOT ID  {
		RelAttr attr;
		relation_attr_init(&attr, $1, $3, NULL, NULL);
		selects_append_group_by(&CONTEXT->ssql->sstr.selection, &attr);
	}
group_having:
	/* empty */ 
	|HAVING COUNT LBRACE STAR RBRACE comOp value {
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, "*", NULL, "count");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
	|HAVING COUNT LBRACE ID RBRACE comOp value {
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, $4, NULL, "count");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
	|HAVING COUNT LBRACE ID DOT ID RBRACE comOp value {
			RelAttr left_attr;
			relation_attr_init(&left_attr, $4, $6, NULL, "count");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
	|HAVING SUM LBRACE ID RBRACE comOp value {
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, $4, NULL, "sum");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
	|HAVING SUM LBRACE ID DOT ID RBRACE comOp value {
			RelAttr left_attr;
			relation_attr_init(&left_attr, $4, $6, NULL, "sum");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
	|HAVING AVG LBRACE ID RBRACE comOp value {
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, $4, NULL, "avg");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
	|HAVING AVG LBRACE ID DOT ID RBRACE comOp value {
			RelAttr left_attr;
			relation_attr_init(&left_attr, $4, $6, NULL, "avg");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
	|HAVING MAX LBRACE ID RBRACE comOp value {
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, $4, NULL, "max");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
	|HAVING MAX LBRACE ID DOT ID RBRACE comOp value {
			RelAttr left_attr;
			relation_attr_init(&left_attr, $4, $6, NULL, "max");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
	|HAVING MIN LBRACE ID RBRACE comOp value {
			RelAttr left_attr;
			relation_attr_init(&left_attr, NULL, $4, NULL, "min");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
	|HAVING MIN LBRACE ID DOT ID RBRACE comOp value {
			RelAttr left_attr;
			relation_attr_init(&left_attr, $4, $6, NULL, "min");
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			CONTEXT->havings[CONTEXT->having_length++] = condition;	
		}
	|HAVING COUNT LBRACE agg_list RBRACE { return -20; }
	|HAVING MAX LBRACE agg_list RBRACE { return -20; }
	|HAVING MIN LBRACE agg_list RBRACE { return -20; }
	|HAVING AVG LBRACE agg_list RBRACE { return -20; }
    ;
	;
comOp:
  	  EQ { CONTEXT->comp = EQUAL_TO; }
    | LT { CONTEXT->comp = LESS_THAN; }
    | GT { CONTEXT->comp = GREAT_THAN; }
    | LE { CONTEXT->comp = LESS_EQUAL; }
    | GE { CONTEXT->comp = GREAT_EQUAL; }
    | NE { CONTEXT->comp = NOT_EQUAL; }
	| LK { CONTEXT->comp = LIKE; }
	| NOT LK { CONTEXT->comp = NOT_LIKE; }
	| IS { CONTEXT->comp = IS_NULL; }
	| IS NOT { CONTEXT->comp = IS_NOT_NULL; }
    ;

load_data:
		LOAD DATA INFILE SSS INTO TABLE ID SEMICOLON
		{
		  CONTEXT->ssql->flag = SCF_LOAD_DATA;
			load_data_init(&CONTEXT->ssql->sstr.load_data, $7, $4);
		}
		;
%%
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
