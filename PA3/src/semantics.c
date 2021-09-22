
#include <stdlib.h>
#include <string.h>

#include "../include/type.h"
#include "../include/semantics.h"
#include "../include/symboltable.h"

static Semantic check_var_declaration(Pair* ast, SymbolTable* table, Type** o_type, char const** o_id);

static Semantic check_statement(Pair* ast, SymbolTable* table, PrimativeType return_type);

static Semantic check_expression(Pair* ast, SymbolTable* table, PrimativeType* result_type) {

    Pair*         node;
    Semantic      result;
    PrimativeType lhs, rhs;

    switch (ast->val) {

        case ASType_NUM: {
            char const* str = ast->dyn;
            long int    num = atol(str);

            if (num > 2147483647L)
                return Semantic_BAD_LITERAL_VALUE;

            ast->num = num;
            if (result_type) *result_type = PrimativeType_INT;
            return Semantic_OK;
        }

        case ASType_LE:
        case ASType_LT:
        case ASType_GT:
        case ASType_GE:
        case ASType_EQ:
        case ASType_NE:
        case ASType_ADD:
        case ASType_SUB:
        case ASType_MUL:
        case ASType_DIV:

            node = ast->cdr;
            result = check_expression(node->car, table, &lhs);
            if (result != Semantic_OK) return result;

            node = node->cdr;
            result = check_expression(node->car, table, &rhs);
            if (result != Semantic_OK) return result;

            if (rhs != PrimativeType_INT || lhs != PrimativeType_INT)
                return Semantic_TYPE_ERROR;

            if (result_type) *result_type = PrimativeType_INT;
            return Semantic_OK;

        case ASType_CALL: {

            Type*      type;
            Scope*     scope; // unused remove param
            Parameter* param;

            node   = ast->cdr;
            result = SymbolTable_lookup(table, node->car->dyn, &type, &scope);
            if (result != Semantic_OK) return result;

            if (type->definition != DefinitionType_FUNCTION)
                return Semantic_TYPE_ERROR;

            /* get arguments */
            node  = node->cdr->car->cdr;
            param = type->function.params;

            if (type->function.nparams == 1 && param->type == PrimativeType_VOID && node == NULL) {
                if (result_type) *result_type = type->function.type;
                return Semantic_OK;
            }

            if (node == NULL) return Semantic_ARITY_MISMATCH; 

            while (node) {

                result = check_expression(node->car, table, &rhs);
                if (result != Semantic_OK) return result;

                if (param->type != rhs)
                    return Semantic_TYPE_ERROR;

                node  = node->cdr;
                param = param->next;

                if ((node && !param) || (!node && param))
                    return Semantic_ARITY_MISMATCH;
            }

            if (result_type) *result_type = type->function.type;
            return Semantic_OK;
        }

        case ASType_VAR: {

            Type*  type;
            Scope* scope; // unused remove param

            node   = ast->cdr;
            result = SymbolTable_lookup(table, node->car->dyn, &type, &scope);
            if (result != Semantic_OK) return result;

            if (type->definition != DefinitionType_VARIABLE)
                return Semantic_TYPE_ERROR;

            node = node->cdr;

            /* can't subscript a non-array */
            if (node) {
                if (type->variable != PrimativeType_ARRAY) return Semantic_TYPE_ERROR;

                 /* make sure the literal is valid */
                result = check_expression(node->car, table, &rhs);

                if (result != Semantic_OK)    return result;
                if (rhs != PrimativeType_INT) return Semantic_TYPE_ERROR;
            }

            if (result_type) {
                if (node)
                    *result_type = PrimativeType_INT;
                else
                    *result_type = type->variable;
            }

            return Semantic_OK;
        }

        case ASType_SET:
            
            node = ast->cdr;
            result = check_expression(node->car, table, &lhs);
            if (result != Semantic_OK) return result;

            node = node->cdr;
            result = check_expression(node->car, table, &rhs);
            if (result != Semantic_OK) return result;

            if (rhs != lhs) return Semantic_TYPE_ERROR;

            if (result_type) *result_type = lhs;
            return Semantic_OK;

        default:
            return Semantic_INVALID_STATE;
    }
}

static Semantic check_compound_stmt(Pair* ast, SymbolTable* table, PrimativeType return_type) {

    Pair* node = ast->cdr;
    if (!node) return Semantic_OK;

    Semantic result;

    do {

        switch (node->car->val) {

            case ASType_NUM:
            case ASType_LE:
            case ASType_LT:
            case ASType_GT:
            case ASType_GE:
            case ASType_EQ:
            case ASType_NE:
            case ASType_ADD:
            case ASType_SUB:
            case ASType_MUL:
            case ASType_DIV:
            case ASType_CALL:
            case ASType_VAR:
            case ASType_SET: {

                PrimativeType type;

                result = check_expression(node->car, table, &type);
                if (result != Semantic_OK)       return result;
                if (type == PrimativeType_ARRAY) return Semantic_TYPE_ERROR;
                break;
            }

            case ASType_EMPTY_STMT:
            case ASType_RETURN_STMT:
            case ASType_ITERATION_STMT:
            case ASType_SELECTION_STMT:
            case ASType_COMPOUND_STMT:

                result = check_statement(node->car, table, return_type);
                if (result != Semantic_OK) return result;
                break;


            case ASType_VAR_DECLARATION:

                result = check_var_declaration(node->car, table, NULL, NULL);
                if (result != Semantic_OK) return result;
                break;

            default:
                return Semantic_INVALID_STATE;
        }

    } while ((node = node->cdr));

    return Semantic_OK;
}

static Semantic check_statement(Pair* ast, SymbolTable* table, PrimativeType return_type) {

    Pair*         node;
    Semantic      result;
    PrimativeType result_type;

    switch (ast->val) {

        case ASType_EMPTY_STMT:
            return Semantic_OK;

        case ASType_RETURN_STMT:

            /* can't return a value from a void function */
            if (return_type == PrimativeType_VOID) {
                if (ast->cdr) {
                    return Semantic_TYPE_ERROR;
                } else {
                    return Semantic_OK;
                }
            }

            node = ast->cdr;

            /* not void ser there MUST be an expression */
            if (!node) return Semantic_TYPE_ERROR;

            result = check_expression(node->car, table, &result_type);

            if (result != Semantic_OK)
                return result;

            if (return_type != result_type)
                return Semantic_TYPE_ERROR;

            return Semantic_OK;

        case ASType_ITERATION_STMT:

            node = ast->cdr;

            result = check_expression(node->car, table, &result_type);
            
            if (result != Semantic_OK)
                return result;

            if (result_type != PrimativeType_INT)
                return Semantic_TYPE_ERROR;

            node = node->cdr;

            return check_statement(node->car, table, return_type);

        case ASType_SELECTION_STMT:

            node = ast->cdr;

            result = check_expression(node->car, table, &result_type);

            if (result != Semantic_OK)
                return result;

            if (result_type != PrimativeType_INT)
                return Semantic_TYPE_ERROR;

            node = node->cdr;

            result = check_statement(node->car, table, return_type);

            if ((node = node->cdr)) {

                if (result != Semantic_OK)
                    return result;

                result = check_statement(node->car, table, return_type);
            }

            return result;

        case ASType_COMPOUND_STMT:

            if (!SymbolTable_enter_scope(table)) return Semantic_INTERNAL_ERROR;

            result = check_compound_stmt(ast, table, return_type);

            SymbolTable_exit_scope(table);

            return result;

        default:
            return Semantic_INVALID_STATE;
    }
}

static Semantic check_var_declaration(Pair* ast, SymbolTable* table, Type** o_type, char const** o_id) {

    Semantic      result;
    PrimativeType basic;
    char const*   identifer;
    Pair*         node = ast->cdr;

    /* basic type variable is declared with */
    if (node->car->val == ASType_VOID) return Semantic_VOID_VAR;
    basic = PrimativeType_INT;

    /* get the identifier */
    node      = node->cdr;
    identifer = node->car->dyn;

    /* check if it's an array */
    if ((node = node->cdr)) {
        switch (node->car->val) {
            case ASType_NUM:
                result = check_expression(node->car, table, NULL);
                if (result != Semantic_OK) return result;
                /* fallthrough */
            case ASType_ARRAY:
                basic = PrimativeType_ARRAY;
                break;

            default:
                return Semantic_INVALID_STATE;
        }
    }

    /* create type */
    Type*      type = Type_new(DefinitionType_VARIABLE, basic);
    if (!type) return Semantic_INTERNAL_ERROR;

    result = SymbolTable_define(table, identifer, type);

    if (result != Semantic_OK) {
        Type_free(type);
    }

    if (o_type) *o_type = type;
    if (o_id)   *o_id   = identifer;

    return result;
}

static Semantic check_fun_declaration(Pair* ast, SymbolTable* table, Type** o_type, char const** o_id) {

    Semantic result;

    PrimativeType basic;
    char const*   identifer;
    Pair*         node = ast->cdr;

    /* function return type */
    basic = PrimativeType_of(node->car->val);
    if (basic == PrimativeType_ARRAY || basic == PrimativeType_FAIL)
        return Semantic_INVALID_STATE;

    /* get the identifier */
    node         = node->cdr;
    identifer    = node->car->dyn;

    /* the base type for the function */
    Type* fntype = Type_new(DefinitionType_FUNCTION, basic);
    if (!fntype) return Semantic_INTERNAL_ERROR;

    /* deal with paramenters */
    node         = node->cdr;
    Pair* params = node->car;

    /* create a new scope to store params */
    if (!SymbolTable_enter_scope(table)) {
        result = Semantic_INTERNAL_ERROR;
        goto fail_1;
    }

    if ((params = params->cdr)) {

        /* we have a list of params */
        Type*         param_type;
        char const*   param_id;

        do {
            /* declare this param in the function scope */
            result = check_var_declaration(params->car, table, &param_type, &param_id);

            if (result == Semantic_OK) {
                /* add the parameter type to the  function */
                fntype = Type_takes(fntype, param_type->variable);

                if (!fntype) {
                    result = Semantic_INTERNAL_ERROR;
                    goto fail_2;
                }

            } else {
                goto fail_2;
            }

        } while ((params = params->cdr));

    } else {

        /* void param, add it to the function and that's it */
        fntype = Type_takes(fntype, PrimativeType_VOID);
        if(!fntype) return Semantic_INTERNAL_ERROR;

    }

    /* define the function in the global scope */
    result = SymbolTable_global(table, identifer, fntype);
    if (result != Semantic_OK) goto fail_2;

    result = check_compound_stmt(node->cdr->car, table, fntype->function.type);

    // IDTable_write(stdout, table->here->symbols);

    SymbolTable_exit_scope(table);

    if (o_type) *o_type = fntype;
    if (o_id)   *o_id   = identifer;

    return result;

fail_2:
    SymbolTable_exit_scope(table);
fail_1:
    Type_free(fntype);
    return result;
}

static Semantic check_declaraction(Pair* ast, SymbolTable* table, Type** o_type, char const** o_id) {

    switch (ast->val) {

        case ASType_FUN_DECLARATION:
            return check_fun_declaration(ast, table, o_type, o_id);

        case ASType_VAR_DECLARATION:
            return check_var_declaration(ast, table, o_type, o_id);

        default:
            return Semantic_INVALID_STATE;
    }
}

Semantic check_semantics(Pair* ast) {

    Semantic result;
    Pair* node = ast;

    /* create the type for main */
    Type* main_type = Type_takes(
        Type_new(DefinitionType_FUNCTION, PrimativeType_VOID), PrimativeType_VOID
    );

    /* out parameters to check delcarations */
    Type*       d_type = NULL;
    char const* d_id   = NULL;

    /* create the global symbol table */
    SymbolTable* table = SymbolTable_new();

    if (!table) {
        result = Semantic_INTERNAL_ERROR;
        goto fail_1;
    }

    /* skip program node to get into declaration list */
    node = node->cdr;

    while (node) {
        
        /* check the declaration in the car of this node */
        result = check_declaraction(node->car, table, &d_type, &d_id);
        if (result != Semantic_OK) goto fail_2;

        node = node->cdr;
    }

    /* ensure there were actually delcarations */
    if (!d_type || !d_id) {
        result = Semantic_NO_FINAL_VOID_MAIN_VOID;
        goto fail_2;
    }

    /* ensure that "void main(void)" is the last declaration */
    if (!Type_equals(d_type, main_type) || strcmp(d_id, "main") != 0) {
        result = Semantic_NO_FINAL_VOID_MAIN_VOID;
        goto fail_2;
    }

    // IDTable_write(stdout, table->here->symbols);

    result = Semantic_OK;

fail_2:

    SymbolTable_free(table);

fail_1:

    return result;
}