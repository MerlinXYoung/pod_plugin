/*
 * * file name: grammar.cpp
 * * description: ...
 * * author: lemonxu
 * * create time:2019 5月 22
 * */
#include "grammar.h"
#include <cassert>
#include "format_conf.h"

namespace Pepper
{
bool MessageStruct::DeclareStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "struct " << name << "\n" << prefix_ << "{\n";

    for (auto child : nest_message)
    {
        if (child->DeclareStr(ss_, prefix_ + g_indent))
            ss_ << "\n";
    }

    for (auto field : fields)
        field->DeclareStr(ss_, prefix_ + g_indent);

    ss_ << "\n";
    FromPbDeclareStr(ss_, prefix_ + g_indent);
    ToPbDeclareStr(ss_, prefix_ + g_indent);

    ss_ << prefix_ << "};\n";
    return true;
}

void MessageStruct::FromPbDeclareStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "bool FromPb(const " << pb_full_name << "& msg_);\n";
}

void MessageStruct::ToPbDeclareStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "bool ToPb(" << pb_full_name << "* msg_) const;\n";
}

bool MessageStruct::ImplStr(stringstream& ss_, const string& prefix_) const
{
    for (auto child : nest_message)
    {
        if (child->ImplStr(ss_, prefix_))
            ss_ << "\n";
    }

    FromPbImplStr(ss_, prefix_);
    ss_ << "\n";
    ToPbImplStr(ss_, prefix_);
    return true;
}

void MessageStruct::FromPbImplStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "bool " << full_name << "::FromPb(const " << pb_full_name << "& msg_)\n";
    ss_ << prefix_ << "{\n";
    for (auto field : fields)
        field->GetPbStr(ss_, prefix_ + g_indent);
    ss_ << prefix_ << g_indent << "return true;\n";
    ss_ << prefix_ << "}\n";
}

void MessageStruct::ToPbImplStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "bool " << full_name << "::ToPb(" << pb_full_name << "* msg_) const\n";
    ss_ << prefix_ << "{\n";
    for (auto field : fields)
        field->SetPbStr(ss_, prefix_ + g_indent);
    ss_ << prefix_ << g_indent << "return true;\n";
    ss_ << prefix_ << "}\n";
}

void Field::DeclareStr(stringstream& ss_, const string& prefix_) const
{
    assert(type_message);
    if (len == 0)
        ss_ << prefix_ << type_message->name << " " << name << ";\n";
    else if (len <= 0xFF)
    {
        string max_len_name = string("max_") + name + string("_count");
        ss_ << prefix_ << "static const uint8_t " << max_len_name << " = " << len << ";\n";
        if (!fixed_len)
            ss_ << prefix_ << "uint8_t " << name << "_count;\n";
        ss_ << prefix_ << type_message->name << " " << name << "[" << max_len_name << "];\n";
    }
    else if (len <= 0xFFFF)
    {
        string max_len_name = string("max_") + name + string("_count");
        ss_ << prefix_ << "static const uint16_t " << max_len_name << " = " << len << ";\n";
        if (!fixed_len)
            ss_ << prefix_ << "uint16_t " << name << "_count;\n";
        ss_ << prefix_ << type_message->name << " " << name << "[" << max_len_name << "];\n";
    }
    else if (len <= 0xFFFFFFFF)
    {
        string max_len_name = string("max_") + name + string("_count");
        ss_ << prefix_ << "static const uint32_t " << max_len_name << " = " << len << ";\n";
        if (!fixed_len)
            ss_ << prefix_ << "uint32_t " << name << "_count;\n";
        ss_ << prefix_ << type_message->name << " " << name << "[" << max_len_name << "];\n";
    }
    else
    {
        string max_len_name = string("max_") + name + string("_count");
        ss_ << prefix_ << "static const uint64_t " << max_len_name << " = " << len << ";\n";
        if (!fixed_len)
            ss_ << prefix_ << "uint64_t " << name << "_count;\n";
        ss_ << prefix_ << type_message->name << " " << name << "[" << max_len_name << "];\n";
    }
}

void Field::SetPbStr(stringstream& ss_, const string& prefix_) const
{
    assert(type_message);
    if (type_message->msg_type == MSG_TYPE::STRING)
        SetStringStr(ss_, prefix_);
    else if (type_message->msg_type == MSG_TYPE::STRUCT)
    {
        if (len == 0)
            SetSingleMessageStr(ss_, prefix_);
        else if (fixed_len)
            SetFixedArrayMessageStr(ss_, prefix_);
        else
            SetArrayMessageStr(ss_, prefix_);
    }
    else
    {
        if (len == 0)
            SetSingleVarStr(ss_, prefix_);
        else if (fixed_len)
            SetFixedArrayVarStr(ss_, prefix_);
        else
            SetArrayVarStr(ss_, prefix_);
    }
}

void Field::SetSingleVarStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "msg_->set_" << name << "(" << name << ");\n";
}

void Field::SetSingleMessageStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "if (!(" << name << ".ToPb(msg_->mutable_" << name << "())"
        << "))\n";
    ss_ << prefix_ << g_indent << "return false;\n";
}

void Field::SetArrayVarStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "msg_->mutable_" << name << "()->Clear();\n";
    ss_ << prefix_ << "for (size_t i = 0; i < " << name << "_count && i < max_" << name << "_count; ++i)\n";
    ss_ << prefix_ << g_indent << "msg_->mutable_" << name << "()->Add(" << name << "[i]);\n";
}

void Field::SetFixedArrayVarStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "msg_->mutable_" << name << "()->Clear();\n";
    ss_ << prefix_ << "for (size_t i = 0; i < max_" << name << "_count; ++i)\n";
    ss_ << prefix_ << g_indent << "msg_->mutable_" << name << "()->Add(" << name << "[i]);\n";
}

void Field::SetArrayMessageStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "msg_->mutable_" << name << "()->Clear();\n";
    ss_ << prefix_ << "for (size_t i = 0; i < " << name << "_count && i < max_" << name << "_count; ++i)\n";
    ss_ << prefix_ << "{\n";
    ss_ << prefix_ << g_indent << "if (!(" << name << "[i].ToPb(msg_->mutable_" << name << "()->Add())"
        << "))\n";
    ss_ << prefix_ << g_indent << g_indent << "return false;\n";
    ss_ << prefix_ << "}\n";
}

void Field::SetFixedArrayMessageStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "msg_->mutable_" << name << "()->Clear();\n";
    ss_ << prefix_ << "for (size_t i = 0; i < max_" << name << "_count; ++i)\n";
    ss_ << prefix_ << "{\n";
    ss_ << prefix_ << g_indent << "if (!(" << name << "[i].ToPb(msg_->mutable_" << name << "()->Add())"
        << "))\n";
    ss_ << prefix_ << g_indent << g_indent << "return false;\n";
    ss_ << prefix_ << "}\n";
}

void Field::SetStringStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "msg_->set_" << name << "(" << name << ", strlen(" << name << "));\n";
}

void Field::GetPbStr(stringstream& ss_, const string& prefix_) const
{
    assert(type_message);
    if (type_message->msg_type == MSG_TYPE::STRING)
        GetStringStr(ss_, prefix_);
    else if (type_message->msg_type == MSG_TYPE::STRUCT)
    {
        if (len == 0)
            GetSingleMessageStr(ss_, prefix_);
        else if (fixed_len)
            GetFixedArrayMessageStr(ss_, prefix_);
        else
            GetArrayMessageStr(ss_, prefix_);
    }
    else
    {
        if (len == 0)
            GetSingleVarStr(ss_, prefix_);
        else if (fixed_len)
            GetFixedArrayVarStr(ss_, prefix_);
        else
            GetArrayVarStr(ss_, prefix_);
    }
}

void Field::GetSingleVarStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << name << " = msg_." << name << "();\n";
}

void Field::GetSingleMessageStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "if (msg_.has_" << name << "())\n";
    ss_ << prefix_ << "{\n";
    ss_ << prefix_ << g_indent << "if (!(" << name << ".FromPb(msg_." << name << "())"
        << "))\n";
    ss_ << prefix_ << g_indent << g_indent << "return false;\n";
    ss_ << prefix_ << "}\n";
}

void Field::GetArrayVarStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "if (msg_." << name << "_size() > max_" << name << "_count)\n";
    ss_ << prefix_ << g_indent << name << "_count = max_" << name << "_count;\n";
    ss_ << prefix_ << "else\n";
    ss_ << prefix_ << g_indent << name << "_count = msg_." << name << "_size();\n";
    ss_ << prefix_ << "for (size_t i = 0; i < " << name << "_count; ++i)\n";
    ss_ << prefix_ << g_indent << name << "[i] = msg_." << name << "(i);\n";
}

void Field::GetFixedArrayVarStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "size_t _" << name << "_count_ = msg_." << name << "_size();\n";
    ss_ << prefix_ << "if (msg_." << name << "_size() > max_" << name << "_count)\n";
    ss_ << prefix_ << g_indent << "_" << name << "_count_ = max_" << name << "_count;\n";
    ss_ << prefix_ << "for (size_t i = 0; i < _" << name << "_count_; ++i)\n";
    ss_ << prefix_ << g_indent << name << "[i] = msg_." << name << "(i);\n";
}

void Field::GetArrayMessageStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "if (msg_." << name << "_size() > max_" << name << "_count)\n";
    ss_ << prefix_ << g_indent << name << "_count = max_" << name << "_count;\n";
    ss_ << prefix_ << "else\n";
    ss_ << prefix_ << g_indent << name << "_count = msg_." << name << "_size();\n";
    ss_ << prefix_ << "for (size_t i = 0; i < " << name << "_count; ++i)\n";
    ss_ << prefix_ << "{\n";
    ss_ << prefix_ << g_indent << "if (!(" << name << "[i].FromPb(msg_." << name << "(i))"
        << "))\n";
    ss_ << prefix_ << g_indent << g_indent << "return false;\n";
    ss_ << prefix_ << "}\n";
}

void Field::GetFixedArrayMessageStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "size_t _" << name << "_count_ = msg_." << name << "_size();\n";
    ss_ << prefix_ << "if (msg_." << name << "_size() > max_" << name << "_count)\n";
    ss_ << prefix_ << g_indent << "_" << name << "_count_ = max_" << name << "_count;\n";
    ss_ << prefix_ << "for (size_t i = 0; i < _" << name << "_count_; ++i)\n";
    ss_ << prefix_ << "{\n";
    ss_ << prefix_ << g_indent << "if (!(" << name << "[i].FromPb(msg_." << name << "(i))"
        << "))\n";
    ss_ << prefix_ << g_indent << g_indent << "return false;\n";
    ss_ << prefix_ << "}\n";
}

void Field::GetStringStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "if (msg_.has_" << name << "())\n";
    ss_ << prefix_ << "{\n";
    ss_ << prefix_ << g_indent << "if (msg_." << name << "().length() > max_" << name << "_count)\n";
    ss_ << prefix_ << g_indent << g_indent << "return false;\n";
    ss_ << prefix_ << g_indent << "msg_." << name << "().copy(" << name << ", msg_." << name << "().length());\n";
    ss_ << prefix_ << "}\n";
}

}  // namespace Pepper