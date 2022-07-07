
#include    "CodeGenerator.h"

typedef struct variable {
    char *name;
    int type;
    int address;
    int size;
} Variable;

typedef struct symbol_table {
    struct variable var;
    struct symbol_table* next;
    
} Symbol_table;

Symbol_table *ST=NULL;

void declaration(treenode *dec_node,Symbol_table* st){
	leafnode * leafn=(leafnode *)dec_node;
    if (!dec_node){
        return;
	}
	if(dec_node->hdr.which==LEAF_T && leafn->hdr.type==TN_IDENT){
		st->var.name=leafn->data.sval->str;
	}
	if(dec_node->hdr.which==LEAF_T && leafn->hdr.type==TN_TYPE){
		st->var.type=leafn->hdr.tok;
		st->var.size=1;
	}
	declaration(dec_node->lnode,st);
	declaration(dec_node->rnode,st);
}

void add(treenode *dec_node){
    Symbol_table *new_st;
    new_st=(Symbol_table*)malloc(sizeof(Symbol_table));
	if(new_st==NULL){return;}
	declaration(dec_node,new_st);
    if(ST==NULL){
        new_st->var.address=5;
    }else{new_st->var.address=ST->var.address+ST->var.size;}
    new_st->next=ST;
    ST=new_st;
}

Variable* find(char *var_name){
    Symbol_table* tmp=ST;
    while(tmp!=NULL&&tmp->next!=NULL && strcmp(tmp->var.name,var_name)!=0){
        tmp=tmp->next;
    }
    if(tmp!=NULL&&strcmp(tmp->var.name,var_name)==0){
        return &(tmp->var);
    }else{return NULL;}
}

int create_st(treenode *root){
	if(!root){
		return SUCCESS;
	}
	if(root->hdr.which==FOR_T){
		for_node *forn=(for_node *)root;
		if(forn->hdr.type==TN_FUNC_DEF){
			create_st(forn->stemnt);
		}
	}
	if(root->hdr.type==TN_DECL){
		add(root);
		return SUCCESS;
	}
	create_st(root->lnode);
	create_st(root->rnode);
	return SUCCESS;
}

void free_ST(){
	Symbol_table *tmp_next;
	while(ST!=NULL){
		tmp_next=ST->next;
		free(ST);
		ST=tmp_next;
	}
}

void print_variable(Variable variable){
	char *tmp;
	switch(variable.type){
        case INT:
			tmp="int";
            break;
        case FLOAT:
            tmp="float";
            break;
        case DOUBLE:
            tmp="double";
            break;
    	}
	printf("%2s%10s%10d%10d\n",variable.name,tmp,variable.address,variable.size);
}

void print_st(Symbol_table* st){
	if(!st){return;}
	print_st(st->next);
	print_variable(st->var);
}

void ldc_address(char *id){
	if(find(id)!=NULL){
		printf("ldc %d\n",find(id)->address);}
}

void ldc_value(char *id){
	if(find(id)!=NULL){
		printf("ldc %d\nind\n",find(id)->address);
	}
}

void int_real(treenode *root){
	leafnode *leaf= (leafnode*)root;  
	if (!root)
    	return;
	if(root->hdr.which==LEAF_T && leaf->hdr.type==TN_INT){
		leaf->hdr.type=TN_REAL;
		leaf->data.dval=(double)leaf->data.ival;
	}
	int_real(root->lnode);
	int_real(root->rnode);
}

/*
 *    This recursive function is the main method for Code Generation
 *    Input: treenode (AST)
 *    Output: prints the Pcode on the console
 */

int label=-1;

int  code_recur(treenode *root)
{
    if_node  *ifn;
    for_node *forn;
    leafnode *leaf;
    
    if (!root)
        return SUCCESS;
    
    switch (root->hdr.which){
        case LEAF_T:
            leaf = (leafnode *) root;
            switch (leaf->hdr.type) {
                case TN_LABEL:
                    /* Maybe you will use it later */
                    break;
                    
                case TN_IDENT:
                    /* variable case */
                    /*
                     *    In order to get the identifier name you have to use:
                     *    leaf->data.sval->str
                     */
                    if(strcmp(leaf->data.sval->str, "main")==0){
						break;
					}else if(strcmp(leaf->data.sval->str, "true")==0){
						printf("ldc 1\n");
						break;
					}else if(strcmp(leaf->data.sval->str, "false")==0){
						printf("ldc 0\n");
						break;
					}
					ldc_value(leaf->data.sval->str);
					break;
                    
                case TN_COMMENT:
                    /* Maybe you will use it later */
                    break;
                    
                case TN_ELLIPSIS:
                    /* Maybe you will use it later */
                    break;
                    
                case TN_STRING:
                    /* Maybe you will use it later */
                    break;
                    
                case TN_TYPE:
                    /* Maybe you will use it later */
                    break;
                    
                case TN_INT:
                    /* Constant case */
                    /*
                     *    In order to get the int value you have to use:
                     *    leaf->data.ival
                     */
                    printf("ldc %d\n",leaf->data.ival);
                    break;
                    
                case TN_REAL:
                    /* Constant case */
                    /*
                     *    In order to get the real value you have to use:
                     *    leaf->data.dval
                     */
                    printf("ldc %lf\n",leaf->data.dval);
                    break;
            }
            break;
            
        case IF_T:
            ifn = (if_node *) root;
            label++;
            int tmp_label=label;
            switch (ifn->hdr.type) {
                case TN_IF:
                    if (ifn->else_n == NULL) {
                        /* if case (without else)*/
                        code_recur(ifn->cond);
                        printf("fjp end_if%d\n",tmp_label);
                        code_recur(ifn->then_n);
                        printf("end_if%d:\n",tmp_label);
                    }
                    else {
                        /* if - else case*/
                        code_recur(ifn->cond);
                        printf("fjp ifelse%d\n",tmp_label);
                        code_recur(ifn->then_n);
                        printf("ujp end_ifelse%d\nifelse%d:\n",tmp_label,tmp_label);
                        code_recur(ifn->else_n);
                        printf("end_ifelse%d:\n",tmp_label);
                    }
                    break;
                    
                case TN_COND_EXPR:
                    /* (cond)?(exp):(exp); */
                    code_recur(ifn->cond);
                    printf("fjp cond_else%d\n",tmp_label);
                    code_recur(ifn->then_n);
                    printf("ujp end_cond%d\ncond_else%d:\n",tmp_label,tmp_label);
                    code_recur(ifn->else_n);
                    printf("ujp end_cond%d\n",tmp_label);
                    break;
                    
                default:
                    /* Maybe you will use it later */
                    code_recur(ifn->cond);
                    printf("fjp label%d\n",tmp_label);
                    code_recur(ifn->then_n);
                    printf("ujp end_label%d\nlabel%d:\n",tmp_label,tmp_label);
                    code_recur(ifn->else_n);
                    printf("ujp end_label%d\n",tmp_label);
            }
            break;
            
        case FOR_T:
            forn = (for_node *) root;
            label++;
            switch (forn->hdr.type) {
                    
                case TN_FUNC_DEF:
                    /* Function definition */
                    /* e.g. int main(...) { ... } */
                    /* Look at the output AST structure! */
                    code_recur(forn->init);
                    code_recur(forn->test);
                    code_recur(forn->incr);
                    code_recur(forn->stemnt);
                    free_ST();
                    break;
                    
                case TN_FOR:
                    /* For case*/
                    /* e.g. for(i=0;i<5;i++) { ... } */
                    /* Look at the output AST structure! */
                    code_recur(forn->init);
                    int tmp1_label=label;
                    printf("for_label%d:\n",tmp1_label);
                    code_recur(forn->test);
                    printf("fjp end_for%d\n",tmp1_label);
                    code_recur(forn->stemnt);
                    code_recur(forn->incr);
                    printf("ujp for_label%d\nend_for%d:\n",tmp1_label,tmp1_label);
                    break;
                    
                default:
                    /* Maybe you will use it later */
                    code_recur(forn->init);
                    code_recur(forn->test);
                    code_recur(forn->stemnt);
                    code_recur(forn->incr);
            }
            break;
            
        case NODE_T:
            switch (root->hdr.type) {
                case TN_PARBLOCK:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_PARBLOCK_EMPTY:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_TRANS_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_FUNC_DECL:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_FUNC_CALL:
                    /* Function call */
                    if (strcmp(((leafnode*)root->lnode)->data.sval->str, "printf") == 0) {
                        /* printf case */
                        /* The expression that you need to print is located in */
                        /* the currentNode->right->right sub tree */
                        /* Look at the output AST structure! */
                        code_recur(root->rnode->rnode);
                        printf("print\n");
                    }
                    else {
                        /* other function calls - for HW3 */
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                    }
                    break;
                    
                case TN_BLOCK:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_ARRAY_DECL:
                    /* array declaration - for HW2 */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_EXPR_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_NAME_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_ENUM_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_FIELD_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_PARAM_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_IDENT_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_TYPE_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_COMP_DECL:
                    /* struct component declaration - for HW2 */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_DECL:
                    /* structs declaration - for HW2 */
                    add(root);
                    break;
                    
                case TN_DECL_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_DECLS:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_STEMNT_LIST:
                    /* Maybe you will use it later */
                    
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_STEMNT:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_BIT_FIELD:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_PNTR:
                    /* pointer - for HW2! */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_TYPE_NME:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_INIT_LIST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_INIT_BLK:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_OBJ_DEF:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_OBJ_REF:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_CAST:
                    /* Maybe you will use it later */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_JUMP:
                    if (root->hdr.tok == RETURN) {
                        /* return jump - for HW2! */
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                    }
                    else if (root->hdr.tok == BREAK) {
                        /* break jump - for HW2! */
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                    }
                    else if (root->hdr.tok == GOTO) {
                        /* GOTO jump - for HW2! */
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                    }
                    break;
                    
                case TN_SWITCH:
                    /* Switch case - for HW2! */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_INDEX:
                    /* call for array - for HW2! */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_DEREF:
                    /* pointer derefrence - for HW2! */
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                case TN_SELECT:
                    /* struct case - for HW2! */
                    if (root->hdr.tok == ARROW){
                        /* Struct select case "->" */
                        /* e.g. struct_variable->x; */
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                    }
                    else{
                        /* Struct select case "." */
                        /* e.g. struct_variable.x; */
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                    }
                    break;
                    
                case TN_ASSIGN:
                    leaf=((leafnode*)root->lnode);
                    if(root->hdr.tok == EQ){
                        /* Regular assignment "=" */
                        /* e.g. x = 5; */
                        ldc_address(leaf->data.sval->str);
						if(find(leaf->data.sval->str)->type==DOUBLE){
							int_real(root->rnode);
						}
                        code_recur(root->rnode);
                        printf("sto\n");
                    }
                    else if (root->hdr.tok == PLUS_EQ){
                        /* Plus equal assignment "+=" */
                        /* e.g. x += 5; */
						ldc_address(leaf->data.sval->str);
						if(find(leaf->data.sval->str)->type==DOUBLE){
							int_real(root->rnode);
						}
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                        printf("add\nsto\n");
                        
                    }
                    else if (root->hdr.tok == MINUS_EQ){
                        /* Minus equal assignment "-=" */
                        /* e.g. x -= 5; */
						ldc_address(leaf->data.sval->str);
						if(find(leaf->data.sval->str)->type==DOUBLE){
							int_real(root->rnode);
						}
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                        printf("sub\nsto\n");
                    }
                    else if (root->hdr.tok == STAR_EQ){
                        /* Multiply equal assignment "*=" */
                        /* e.g. x *= 5; */
						ldc_address(leaf->data.sval->str);
						if(find(leaf->data.sval->str)->type==DOUBLE){
							int_real(root->rnode);
						}
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                        printf("mul\nsto\n");
                    }
                    else if (root->hdr.tok == DIV_EQ){
                        /* Divide equal assignment "/=" */
                        /* e.g. x /= 5; */
						ldc_address(leaf->data.sval->str);
						if(find(leaf->data.sval->str)->type==DOUBLE){
							int_real(root->rnode);
						}
                        code_recur(root->lnode);
                        code_recur(root->rnode);
                        printf("div\nsto\n");
                    }
                    break;
                    
                case TN_EXPR:
                    switch (root->hdr.tok) {
                        case CASE:
                            /* you should not get here */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            break;
                            
                        case INCR:
                            /* Increment token "++" */
                            if(((leafnode*)root->rnode)==NULL){
                                code_recur(root->lnode);
                                ldc_address(((leafnode*)root->lnode)->data.sval->str);
                                ldc_value(((leafnode*)root->lnode)->data.sval->str);
                                printf("inc 1\nsto\n");
                            }else{
                                ldc_address(((leafnode*)root->rnode)->data.sval->str);
                                ldc_value(((leafnode*)root->rnode)->data.sval->str);
                                printf("inc 1\nsto\n");
                                code_recur(root->rnode);
                            }
                            break;
                            
                        case DECR:
                            /* Decrement token "--" */
                            if(((leafnode*)root->rnode)==NULL){
                                code_recur(root->lnode);
                                printf("sto\n");
                                ldc_address(((leafnode*)root->lnode)->data.sval->str);
                                ldc_value(((leafnode*)root->lnode)->data.sval->str);
                                printf("dec 1\n");
                            }else{
                                ldc_address(((leafnode*)root->rnode)->data.sval->str);
                                ldc_value(((leafnode*)root->rnode)->data.sval->str);
                                printf("dec 1\nsto\n");
                                code_recur(root->rnode);
                            }
                            break;
                            
                        case PLUS:
                            /* Plus token "+" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("add\n");
                            break;
                            
                        case MINUS:
                            /* Minus token "-" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            if(root->lnode==NULL){printf("neg\n");}else{printf("sub\n");}
                            break;
                            
                        case DIV:
                            /* Divide token "/" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("div\n");
                            break;
                            
                        case STAR:
                            /* multiply token "*" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("mul\n");
                            break;
                            
                        case AND:
                            /* And token "&&" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("and\n");
                            break;
                            
                        case OR:
                            /* Or token "||" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("or\n");
                            break;
                            
                        case NOT:
                            /* Not token "!" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("not\n");
                            break;
                            
                        case GRTR:
                            /* Greater token ">" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("grt\n");
                            break;
                            
                        case LESS:
                            /* Less token "<" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("les\n");
                            break;
                            
                        case EQUAL:
                            /* Equal token "==" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("equ\n");
                            break;
                            
                        case NOT_EQ:
                            /* Not equal token "!=" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("neq\n");
                            break;
                            
                        case LESS_EQ:
                            /* Less or equal token "<=" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("leq\n");
                            break;
                            
                        case GRTR_EQ:
                            /* Greater or equal token ">=" */
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            printf("geq\n");
                            break;
                        default:
                            code_recur(root->lnode);
                            code_recur(root->rnode);
                            break;
                    }
                    break;
                    
                case TN_WHILE:
                    /* While case */
                    label++;
                    printf("while_loop%d:\n",label);
                    int tmp2_label=label;
                    code_recur(root->lnode);
                    printf("fjp end_while%d\n",tmp2_label);
                    code_recur(root->rnode);
                    printf("ujp while_loop%d\nend_while%d:\n",tmp2_label,tmp2_label);
                    break;
                    
                case TN_DOWHILE:
                    /* Do-While case */
                    label++;
                    printf("dowhile_loop%d:\n",label);
                    int tmp3_label=label;
                    code_recur(root->rnode);
                    printf("fjp end_dowhile%d\n",tmp3_label);
                    code_recur(root->lnode);
                    printf("ujp dowhile_loop%d\nend_dowhile%d:\n",tmp3_label,tmp3_label);
                    break;
                    
                case TN_LABEL:
                    code_recur(root->lnode);
                    code_recur(root->rnode);
                    break;
                    
                default:
                    code_recur(root->lnode);
                    code_recur(root->rnode);
            }
            break;
            
        case NONE_T:
            printf("Error: Unknown node type!\n");
            exit(FAILURE);
    }
    return SUCCESS;
}

/*
 *    This function prints all the variables on your symbol table with their data
 *    Input: treenode (AST)
 *    Output: prints the Sumbol Table on the console
 */


void print_symbol_table(treenode *root) {
    printf("---------------------------------------\n");
    printf("Showing the Symbol Table:\n");
    printf("Name  Type  Address  Size\n");
    /*
     *    add your code here
     */
    create_st(root);
    print_st(ST);
    free_ST();
}


