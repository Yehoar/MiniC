#include "AST.h"

ASTNode::ASTNode()
{
	this->InitChild();
}

ASTNode::ASTNode(Token tk, StmtType st) : token(tk), stmtType(st)
{
	this->InitChild();
}
ASTNode::ASTNode(Token tk) : token(tk), stmtType(StmtType::NONE)
{
	this->InitChild();
}

ASTNode::ASTNode(StmtType st) : stmtType(st)
{
	this->InitChild();
}

ASTNode::~ASTNode()
{
	if (this->child != nullptr)
	{
		delete[](this->child);
		this->child = nullptr;
	}
}

void ASTNode::InitChild()
{
	this->child = new ASTNodePointer[ASTNode::MAXCHILD]{nullptr};
	// for (auto idx = 0; idx < this->MAXCHILD; ++idx)
	// {
	// 	this->child[idx] = nullptr;
	// }
}

void ASTNode::AddSibling(ASTNodePointer node)
{
	if (node != nullptr && node != this)
	{
		if (this->sibling == nullptr)
		{
			this->sibling = node;
		}
		else
		{
			ASTNodePointer ptr = this->sibling;
			while (ptr->sibling != nullptr)
			{
				ptr = ptr->sibling;
			}
			ptr->sibling = node;
		}
	}
}
void ASTNode::AddChild(ASTNodePointer node)
{
	if (this->childIdx < ASTNode::MAXCHILD)
	{
		this->child[this->childIdx++] = node;
	}
}

void ASTNode::AddChild(Token tk, StmtType st)
{
	if (this->childIdx < ASTNode::MAXCHILD)
	{
		this->child[this->childIdx++] = new ASTNode(tk, st);
	}
}

bool ASTNode::IsTypeOf(const StmtType &st)
{
	return this->stmtType == st;
}

bool ASTNode::IsTypeOf(const ExpType &et)
{
	return this->expType == et;
}

AST::~AST()
{
	this->Destroy(root);
	this->root = nullptr;
}

void AST::Destroy(ASTNodePointer tree)
{
	if (tree != nullptr)
	{
		if (tree->child != nullptr)
		{
			for (auto idx = 0; idx < ASTNode::MAXCHILD; ++idx)
			{
				Destroy(tree->child[idx]);
			}
		}
		Destroy(tree->sibling);
		// printf(tree->token.val.c_str());
		delete tree;
	}
}

void AST::PrintTree()
{

	string str = this->ToString();
	Logger::Print(str.c_str());
}

string AST::ToString()
{
	string buf;
	buf.reserve(1024 * 10);
	buf.append("--------------------------------------------\n");
	buf.append("Abstract Syntax Tree:\n");
	buf.append("--------------------------------------------\n");
	this->ToString(root, buf, 0);
	buf.append("--------------------------------------------\n");
	return buf;
}

void AST::ToString(ASTNodePointer subTree, string &buf, int indent)
{
	if (subTree == nullptr)
	{
		return;
	}
	buf.append(indent, ' ').append("|---");
	auto s_child = subTree->child;
	auto printChild = true;

	switch (subTree->stmtType)
	{
	case StmtType::VAR_CALL:
	{
		buf.append("VAR_CALL: ");
		buf.append(subTree->token.val);
		buf.append("\n");
		printChild = false;
		break;
	}
	case StmtType::NUM:
	{
		buf.append("NUM: ");
		buf.append(subTree->token.val);
		buf.append("\n");
		printChild = false;
		break;
	}
	case StmtType::ARR_DECL:
	{
		buf.append("ARR_DECL: ");
		buf.append(subTree->token.val);
		buf.append("\n");
		break;
	}
	case StmtType::ARR_LEN:
	{
		buf.append("ARR_LEN: ");
		buf.append(subTree->token.val);
		buf.append("\n");
		printChild = false;
		break;
	}
	case StmtType::ARR_CALL:
	{
		buf.append("ARR_CALL: ");
		buf.append(subTree->token.val);
		buf.append("\n");
		break;
	}
	case StmtType::PARAM_INT:
	{
		buf.append("INT: ");
		buf.append(subTree->token.val);
		buf.append("\n");
		printChild = false;
		break;
	}
	case StmtType::PARAM_ARR:
	{
		buf.append("ARR: ");
		buf.append(subTree->token.val);
		buf.append("\n");
		printChild = false;
		break;
	}
	case StmtType::VAR_DECL:
	{
		buf.append("VAR_DECL: ");
		buf.append(subTree->token.val);
		buf.append("\n");
		printChild = false;
		break;
	}
	case StmtType::RELOP:
	{
		buf.append("RELOP: ");
		buf.append(subTree->token.val);
		buf.append("\n");
		break;
	}
	case StmtType::ADDOP:
	{
		buf.append("ADDOP: ");
		buf.append(subTree->token.val);
		buf.append("\n");
		break;
	}
	case StmtType::MULOP:
	{
		buf.append("MULOP: ");
		buf.append(subTree->token.val);
		buf.append("\n");
		break;
	}
	case StmtType::ASSIGN_STMT:
	{
		buf.append("ASSIGN: \n");
		break;
	}

	case StmtType::IF_STMT:
	{
		buf.append("IF_STMT: \n");
		// 条件
		buf.append(indent + INDENT, ' ').append("|---");
		buf.append("IF_COND: \n");
		ToString(s_child[0], buf, indent + INDENT * 2);
		// 条件为真
		buf.append(indent + INDENT, ' ').append("|---");
		buf.append("IF_TRUE: \n");
		ToString(s_child[1], buf, indent + INDENT * 2);
		// else
		if (subTree->child[2] != nullptr)
		{
			buf.append(indent + INDENT, ' ').append("|---");
			buf.append("IF_FALSE: \n");
			ToString(s_child[2], buf, indent + INDENT * 2);
		}
		printChild = false;
		break;
	}
	case StmtType::ITER_STMT:
	{
		buf.append("ITER_STMT: \n");
		// 条件
		buf.append(indent + INDENT, ' ').append("|---");
		buf.append("ITER_COND: \n");
		ToString(s_child[0], buf, indent + INDENT * 2);
		// 循环体
		buf.append(indent + INDENT, ' ').append("|---");
		buf.append("ITER_BODY: \n");
		ToString(s_child[1], buf, indent + INDENT * 2);
		printChild = false;
		break;
	}
	case StmtType::FUNC_CALL:
	{
		buf.append("FUNC_CALL: ");
		buf.append(subTree->token.val);
		buf.append("\n");
		// 实参列表
		buf.append(indent + INDENT, ' ').append("|---");
		if (s_child[0] == nullptr)
		{
			buf.append("ARGS: VOID \n");
		}
		else
		{
			buf.append("ARGS: \n");
			ToString(s_child[0], buf, indent + INDENT * 2);
		}
		printChild = false;
		break;
	}
	case StmtType::FUNC_DECL:
	{ //函数名
		buf.append("FUNC_DECL: ");
		buf.append(subTree->token.val);
		buf.append("\n");
		//返回值
		buf.append(indent + INDENT, ' ').append("|---");
		buf.append("RETURN_TYPE: ");
		buf.append(s_child[0]->token.val);
		buf.append("\n");

		//参数
		buf.append(indent + INDENT, ' ').append("|---");
		if (s_child[1] == nullptr || s_child[1]->IsTypeOf(StmtType::PARAM_VOID))
		{
			buf.append("PARAM: void\n");
		}
		else
		{
			buf.append("PARAM: \n");
			ToString(s_child[1], buf, indent + INDENT * 2);
		}
		// 函数体
		buf.append(indent + INDENT, ' ').append("|---");
		if (s_child[2] == nullptr)
		{
			buf.append("FUNC_BODY: EMPTY\n");
			break;
		}
		buf.append("FUNC_BODY: \n");
		ToString(s_child[2], buf, indent + INDENT * 2);
		printChild = false;
		break;
	}
	case StmtType::RET_STMT:
	{
		if (s_child[0] == nullptr)
		{
			buf.append("RETURN: void \n");
			break;
		}
		buf.append("RETURN: \n");
		ToString(s_child[0], buf, indent + INDENT);
		printChild = false;
		break;
	}
	case StmtType::COMP_STMT:
	{
		//复合语句
		if (s_child[0] == nullptr && s_child[1] == nullptr)
		{
			buf.append("COMP_STMT: EMPTY\n");
			printChild = false;
			break;
		}
		buf.append("COMP_STMT: \n");
		break;
	}
	default:
	{
		// 打印错误
		buf.append("???");
		buf.append(subTree->token.val);
		buf.append("\n");
		break;
	}
	}
	if (printChild)
	{
		for (auto idx = 0; idx < ASTNode::MAXCHILD; ++idx)
		{
			ToString(s_child[idx], buf, indent + INDENT);
		}
	}
	ToString(subTree->sibling, buf, indent);
}