/*
 * * file name: grammar.cpp
 * * description: ...
 * * author: snow
 * * create time:2019 5月 22
 * */
#include "grammar.h"
#include <cassert>
#include "global_var.h"

namespace Pepper
{
static std::string _get_type(size_t len){
    if (len <= 0xFF)
    {
        return "uint8_t";
    }
    else if (len <= 0xFFFF)
    {
        return "uint16_t";
    }
    else if (len <= 0xFFFFFFFF)
    {
        return "uint32_t";
    }
    else
    {
        return "uint64_t";
    }
}
bool StrStruct::DeclareStr(stringstream& ss_, const string& prefix_) const
{

    // assert(type_message);
    ss_ << prefix_ << "struct " << name << "\n" << prefix_ << "{\n";
    string max_len_name = "_capcity";
    auto prefix = prefix_+ GlobalVar::indent;
    auto type = _get_type(len);

    ss_ << prefix << "static const "<<type<<" " << max_len_name << " = " << len << ";\n";
    if (GlobalVar::standard == CPP_STANDARD::CPP_98)
        ss_ << prefix << type<< " " << "count;\n";
    else
        ss_ << prefix << type<<" " << "count{"<<default_value.size()<<"};\n";

    if (GlobalVar::standard == CPP_STANDARD::CPP_98)
        ss_ << prefix << "char data" << "[" << max_len_name << "+1];\n";
    else
    {
        if (default_value.empty())
            ss_ << prefix << "char data" << "[" << max_len_name << "+1] = {0};\n";
        else
            ss_ << prefix << "char data" << "[" << max_len_name << "+1] = \""
                << default_value << "\";\n";
    }

    ss_ << prefix << "inline void Reset() {\n";
    ss_ << prefix << GlobalVar::indent << "count = " << default_value.size() << ";\n";
    if (default_value.empty())
    {
        ss_ << prefix << GlobalVar::indent << "data[0] = '\\0';\n";
    }
    else
    {
        ss_ << prefix << GlobalVar::indent << "strncpy(&" << "data[0], \"" << default_value << "\", "<< default_value.size() << ");\n";
        ss_ << prefix << GlobalVar::indent << "data[count]=0;\n";
    }
    ss_ << prefix << "}\n";

    ss_ << prefix << "inline void Clear() { count=0;}\n";

    ss_ << prefix << "inline bool From(const std::string& v){\n";
    ss_ << prefix << GlobalVar::indent << "count = v.size()<_capcity?v.size():_capcity;\n";
    ss_ << prefix << GlobalVar::indent << "strncpy(&data[0], v.data(), count);\n";
    ss_ << prefix << GlobalVar::indent << "data[count]='\\0';\n";
    ss_ << prefix << GlobalVar::indent << "return true;\n";
    ss_ << prefix << "}\n";

    ss_ << prefix << "inline bool To(std::string* v) const{\n";
    ss_ << prefix << GlobalVar::indent << "v->resize(count);\n";
    ss_ << prefix << GlobalVar::indent << "memcpy((void*)v->data(), (void*)data, count);\n";
    ss_ << prefix << GlobalVar::indent << "return true;\n";
    ss_ << prefix << "}\n";

    ss_ << prefix_ << "};\n";

    return true;
    
}

bool BytesStruct::DeclareStr(stringstream& ss_, const string& prefix_) const
{
    // assert(type_message);
    ss_ << prefix_ << "struct " << name << "\n" << prefix_ << "{\n";
    string max_len_name = "_capcity";
    auto prefix = prefix_+ GlobalVar::indent;
    auto type = _get_type(len);

    ss_ << prefix << "static const "<<type<<" " << max_len_name << " = " << len << ";\n";
    if (GlobalVar::standard == CPP_STANDARD::CPP_98)
        ss_ << prefix << type<< " " << "count;\n";
    else
        ss_ << prefix << type<<" " << "count{"<<default_value.size()<<"};\n";

    if (GlobalVar::standard == CPP_STANDARD::CPP_98)
        ss_ << prefix << "char data" << "[" << max_len_name << "+1];\n";
    else
    {
        if (default_value.empty())
            ss_ << prefix << "char data" << "[" << max_len_name << "+1] = {0};\n";
        else
            ss_ << prefix << "char data" << "[" << max_len_name << "+1] = \""
                << default_value << "\";\n";
    }

    ss_ << prefix << "inline void Reset() {\n";
    ss_ << prefix << GlobalVar::indent << "count = " << default_value.size() << ";\n";
    if (default_value.empty())
    {
        ss_ << prefix << GlobalVar::indent << "memset(data, 0, sizeof(data));\n";
    }
    else
    {
        ss_ << prefix << GlobalVar::indent << "memcpy(&" << "data[0], \"" << default_value << "\", "<< default_value.size() << ");\n";
    }
    ss_ << prefix << "};\n";

    ss_ << prefix << "inline void Clear() { count=0;}\n";

    ss_ << prefix << "inline bool From(const std::string& v){\n";
    ss_ << prefix << GlobalVar::indent << "count = v.size()<_capcity?v.size():_capcity;\n";
    ss_ << prefix << GlobalVar::indent << "memcpy((void*)&data[0], (void*)v.data(), count);\n";
    ss_ << prefix << GlobalVar::indent << "return true;\n";
    ss_ << prefix << "}\n";

    ss_ << prefix << "inline bool To(std::string* v) const{\n";
    ss_ << prefix << GlobalVar::indent << "v->resize(count);\n";
    ss_ << prefix << GlobalVar::indent << "memcpy((void*)v->data(), (void*)data, count);\n";
    ss_ << prefix << GlobalVar::indent << "return true;\n";
    ss_ << prefix << "}\n";

    

    ss_ << prefix_ << "};\n";

    return true;
    
}

bool MessageStruct::DeclareStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "struct " << name << "\n" << prefix_ << "{\n";

    for (auto child : nest_message)
    {
        if (child->DeclareStr(ss_, prefix_ + GlobalVar::indent))
            ss_ << "\n";
    }

    for (auto field : fields)
        field->DeclareStr(ss_, prefix_ + GlobalVar::indent, pb_full_name);

    ss_ << "\n";
    if (GlobalVar::standard == CPP_STANDARD::CPP_98)
        ConstructorDeclareStr(ss_, prefix_ + GlobalVar::indent);

    ResetDeclareStr(ss_, prefix_ + GlobalVar::indent);
    ClearDeclareStr(ss_, prefix_ + GlobalVar::indent);
    FromPbDeclareStr(ss_, prefix_ + GlobalVar::indent);
    ToPbDeclareStr(ss_, prefix_ + GlobalVar::indent);

    ss_ << prefix_ << "};\n";
    return true;
}

void MessageStruct::ConstructorDeclareStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << name << "();\n";
}

void MessageStruct::ResetDeclareStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "void Reset();\n";
}

void MessageStruct::ClearDeclareStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "void Clear();\n";
}

void MessageStruct::FromPbDeclareStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "bool From(const " << pb_full_name << "& msg_);\n";
}

void MessageStruct::ToPbDeclareStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "bool To(" << pb_full_name << "* msg_) const;\n";
}

bool MessageStruct::ImplStr(stringstream& ss_, const string& prefix_) const
{
    for (auto child : nest_message)
    {
        if (child->ImplStr(ss_, prefix_))
            ss_ << "\n";
    }

    if (GlobalVar::standard == CPP_STANDARD::CPP_98)
    {
        ConstructorImplStr(ss_, prefix_);
        ss_ << "\n";
    }

    ResetImplStr(ss_, prefix_);
    ss_ << "\n";
    ClearImplStr(ss_, prefix_);
    ss_ << "\n";
    FromPbImplStr(ss_, prefix_);
    ss_ << "\n";
    ToPbImplStr(ss_, prefix_);
    return true;
}

void MessageStruct::ConstructorImplStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << full_name << "::" << name << "()\n";
    ss_ << prefix_ << "{\n";
    ss_ << prefix_ << GlobalVar::indent << "Reset();\n";
    ss_ << prefix_ << "}\n";
}

void MessageStruct::ResetImplStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "void " << full_name << "::Reset()\n";
    ss_ << prefix_ << "{\n";
    for (auto field : fields)
        field->InitStr(ss_, prefix_ + GlobalVar::indent);
    ss_ << prefix_ << "}\n";
}

void MessageStruct::ClearImplStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "void " << full_name << "::Clear()\n";
    ss_ << prefix_ << "{\n";
    ss_ << prefix_ << GlobalVar::indent << "std::memset(this, 0, sizeof(" << full_name << "));\n";
    ss_ << prefix_ << "}\n";
}

void MessageStruct::FromPbImplStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "bool " << full_name << "::From(const " << pb_full_name << "& msg_)\n";
    ss_ << prefix_ << "{\n";
    for (auto field : fields)
        field->GetPbStr(ss_, prefix_ + GlobalVar::indent);
    ss_ << prefix_ << GlobalVar::indent << "return true;\n";
    ss_ << prefix_ << "}\n";
}

void MessageStruct::ToPbImplStr(stringstream& ss_, const string& prefix_) const
{
    ss_ << prefix_ << "bool " << full_name << "::To(" << pb_full_name << "* msg_) const\n";
    ss_ << prefix_ << "{\n";
    for (auto field : fields)
        field->SetPbStr(ss_, prefix_ + GlobalVar::indent);
    ss_ << prefix_ << GlobalVar::indent << "return true;\n";
    ss_ << prefix_ << "}\n";
}

void Field::DeclareStr(stringstream& ss_, const string& prefix_, const string& pb_full_name_) const
{
    assert(type_message);
    //not repeated
    if(type_message->msg_type == MSG_TYPE::STRING || type_message->msg_type == MSG_TYPE::BYTES )
    {
        type_message->DeclareStr(ss_, prefix_);

    }
    if(array_len == 0){
        if (default_value.empty() || GlobalVar::standard == CPP_STANDARD::CPP_98)
            ss_ << prefix_ << type_message->name << " " << name << ";\n";
        else
            ss_ << prefix_ << type_message->name << " " << name << " = " << default_value << ";\n";
        
        return;
    }
    if( array_fixed_len)
    {
        auto aname = "fixed_array_"+name;
        ss_ << prefix_ << "struct " << aname << "\n" << prefix_ << "{\n";
        string max_len_name = "_capcity";
        auto prefix = prefix_ + GlobalVar::indent;
        auto type = _get_type(array_len);

        ss_ << prefix << "static const "<<type<<" " << max_len_name << " = " << array_len << ";\n";

        if (GlobalVar::standard == CPP_STANDARD::CPP_98)
            ss_ << prefix << type_message->name << " data[" << max_len_name << "];\n";
        else
        {
            if (default_value.empty())
                ss_ << prefix << type_message->name << " data[" << max_len_name << "];\n";
            else
                ss_ << prefix << type_message->name << " data[" << max_len_name << "] = {"
                    << default_value << "};\n";
            
        }

        ss_ << prefix << "inline void Reset() {\n";
        if(type_message->msg_type == MSG_TYPE::SIMPLE || type_message->msg_type == MSG_TYPE::ENUM)
            ss_ << prefix << GlobalVar::indent << "memset(&data[0], 0, sizeof(data));\n";
        else{
            ss_ << prefix << GlobalVar::indent << "for(size_t i = 0; i < _capcity; ++i){\n";
            ss_ << prefix << GlobalVar::indent << GlobalVar::indent << "data[i].Reset();\n";
            ss_ << prefix << GlobalVar::indent << "}\n";
        }
        ss_ << prefix << "}\n";

        ss_ << prefix << "inline void Clear() { memset(&data[0], 0, sizeof(data));}\n";

        ss_ << prefix << "inline bool To("<< pb_full_name_<<"* msg_) const{ \n";
        ss_ << prefix << GlobalVar::indent << "msg_->mutable_" << name << "()->Clear();\n";
        ss_ << prefix << GlobalVar::indent << "for (size_t i = 0; i < _capcity; ++i)\n";
        ss_ << prefix << GlobalVar::indent << "{\n";
        if(type_message->msg_type == MSG_TYPE::SIMPLE || type_message->msg_type == MSG_TYPE::ENUM){
            ss_ << prefix << GlobalVar::indent << GlobalVar::indent << "msg_->add_" << name << "(data[i]);\n";
        }
        else{
            ss_ << prefix << GlobalVar::indent << GlobalVar::indent << "if (!(data[i].To(msg_->mutable_" << name << "()->Add())"
                << "))\n";
            ss_ << prefix << GlobalVar::indent << GlobalVar::indent << GlobalVar::indent << "return false;\n";
        }
        ss_ << prefix << GlobalVar::indent << "}\n";
        ss_ << prefix << GlobalVar::indent << "return true;\n";
        ss_ << prefix << "}\n";

        ss_ << prefix << "inline bool From(const "<< pb_full_name_<<"& msg_) { \n";
        ss_ << prefix << GlobalVar::indent << "size_t count = msg_." << name << "_size();\n";
        ss_ << prefix << GlobalVar::indent << "if (count > _capcity)\n";
        ss_ << prefix << GlobalVar::indent << GlobalVar::indent << "count = _capcity;\n";
        ss_ << prefix << GlobalVar::indent << "for (size_t i = 0; i < count; ++i)\n";
        ss_ << prefix << GlobalVar::indent << "{\n";
        if(type_message->msg_type == MSG_TYPE::SIMPLE || type_message->msg_type == MSG_TYPE::ENUM){
            ss_ << prefix << GlobalVar::indent << GlobalVar::indent << "data[i] = msg_." << name << "(i);\n";
        }
        else {
            ss_ << prefix << GlobalVar::indent << GlobalVar::indent << "if (!data[i].From(msg_." << name << "(i)))\n";
            ss_ << prefix << GlobalVar::indent << GlobalVar::indent << GlobalVar::indent << "return false;\n";
        }
        ss_ << prefix << GlobalVar::indent  << "}\n";
        ss_ << prefix << GlobalVar::indent << "return true;\n";
        
        ss_ << prefix << "}\n";

        ss_ << prefix_ << "};\n";

        ss_ << prefix_ << aname << " "<< name <<";\n";
    }
    else{
        auto aname = "array_"+name;
        ss_ << prefix_ << "struct " << aname << "\n" << prefix_ << "{\n";
        string max_len_name = "_capcity";
        auto prefix = prefix_ + GlobalVar::indent;
        auto type = _get_type(array_len);

        ss_ << prefix << "static const "<<type<<" " << max_len_name << " = " << array_len << ";\n";

        if (GlobalVar::standard == CPP_STANDARD::CPP_98)
            ss_ << prefix << type<< " " << "count;\n";
        else
            ss_ << prefix << type<<" " << "count{0};\n";


        if (GlobalVar::standard == CPP_STANDARD::CPP_98)
            ss_ << prefix << type_message->name << " data[" << max_len_name << "];\n";
        else
        {

            if (default_value.empty())
                ss_ << prefix << type_message->name << " data[" << max_len_name << "];\n";
            else
                ss_ << prefix << type_message->name << " data[" << max_len_name << "] = {"
                    << default_value << "};\n";
            
        }

        ss_ << prefix << "inline void Reset() { count=0;}\n";
        ss_ << prefix << "inline void Clear() { count=0;}\n";
        

        ss_ << prefix << "inline bool To("<< pb_full_name_<<"* msg_) const{ \n";
        ss_ << prefix << GlobalVar::indent << "msg_->mutable_" << name << "()->Clear();\n";
        ss_ << prefix << GlobalVar::indent << "for (size_t i = 0; i < count; ++i)\n";
        ss_ << prefix << GlobalVar::indent << "{\n";
        if(type_message->msg_type == MSG_TYPE::SIMPLE || type_message->msg_type == MSG_TYPE::ENUM){
            ss_ << prefix << GlobalVar::indent << GlobalVar::indent << "msg_->add_" << name << "(data[i]);\n";
        }
        else{
            ss_ << prefix << GlobalVar::indent << GlobalVar::indent << "if (!(data[i].To(msg_->mutable_" << name << "()->Add())"
                << "))\n";
            ss_ << prefix << GlobalVar::indent << GlobalVar::indent << GlobalVar::indent << "return false;\n";
        }
        ss_ << prefix << GlobalVar::indent << "}\n";
        ss_ << prefix << GlobalVar::indent << "return true;\n";
        ss_ << prefix << "}\n";

        ss_ << prefix << "inline bool From(const "<< pb_full_name_<<"& msg_) { \n";
        ss_ << prefix << GlobalVar::indent << "if (msg_." << name << "_size() > count)\n";
        ss_ << prefix << GlobalVar::indent << GlobalVar::indent << "count = _capcity;\n";
        ss_ << prefix << GlobalVar::indent << "else\n";
        ss_ << prefix << GlobalVar::indent << GlobalVar::indent << "count = msg_." << name << "_size();\n";
        ss_ << prefix << GlobalVar::indent << "for (size_t i = 0; i < count; ++i)\n";
        ss_ << prefix << GlobalVar::indent << "{\n";
        if(type_message->msg_type == MSG_TYPE::SIMPLE || type_message->msg_type == MSG_TYPE::ENUM){
            ss_ << prefix << GlobalVar::indent << GlobalVar::indent << "data[i] = msg_." << name << "(i);\n";
        }
        else{
            ss_ << prefix << GlobalVar::indent << GlobalVar::indent << "if (!(data[i].From(msg_." << name << "(i))"
                << "))\n";
            ss_ << prefix << GlobalVar::indent << GlobalVar::indent << GlobalVar::indent << "return false;\n";
        }
        ss_ << prefix << GlobalVar::indent  << "}\n";
        ss_ << prefix << GlobalVar::indent << "return true;\n";
        
        
        ss_ << prefix << "}\n";

        ss_ << prefix_ << "};\n";

        ss_ << prefix_ << aname << " "<< name <<";\n";
    }
}

void Field::SetPbStr(stringstream& ss_, const string& prefix_) const
{
    assert(type_message);
    if (type_message->msg_type == MSG_TYPE::STRING || type_message->msg_type == MSG_TYPE::BYTES){
        if( array_len == 0)
            ss_ << prefix_ << name << ".To(msg_->mutable_" << name << "());\n";
        else
            ss_ << prefix_ << name << ".To(msg_);\n";
    }
    else if (type_message->msg_type == MSG_TYPE::STRUCT)
    {
        if (array_len == 0)
            ss_ << prefix_ << name << ".To(msg_->mutable_" << name << "());\n";
        else
            ss_ << prefix_ << name << ".To(msg_);\n";
    }
    else
    {
        if (array_len == 0)
            ss_ << prefix_ << "msg_->set_" << name << "(" << name << ");\n";
        else
            ss_ << prefix_ << name << ".To(msg_);\n";
    }
}


void Field::GetPbStr(stringstream& ss_, const string& prefix_) const
{
    assert(type_message);
    if (type_message->msg_type == MSG_TYPE::STRING || type_message->msg_type == MSG_TYPE::BYTES)
    {
        if (array_len == 0)
            ss_ << prefix_ << name << ".From(msg_." << name << "());\n";
        else
            ss_ << prefix_ << name << ".From(msg_);\n"; 
    }
    else if (type_message->msg_type == MSG_TYPE::STRUCT)
    {
        if (array_len == 0)
            ss_ << prefix_ << name << ".From(msg_." << name << "());\n";
        else
        {
            ss_ << prefix_ << name << ".From(msg_);\n"; 
        }
        
    }
    else
    {
        if (array_len == 0)
            ss_ << prefix_ << name << " = msg_." << name << "();\n";
        else
            ss_ << prefix_ << name << ".From(msg_);\n";
    }
}

void Field::InitStr(stringstream& ss_, const string& prefix_) const
{
    assert(type_message);
    if (type_message->msg_type == MSG_TYPE::STRING || type_message->msg_type == MSG_TYPE::BYTES){
        ss_ << prefix_ << name<<".Reset();\n";
    }
    else if (type_message->msg_type == MSG_TYPE::STRUCT)
    {
            ss_ << prefix_ << name<<".Reset();\n";
    }
    else
    {
        if (array_len == 0)
            ss_ << prefix_ << name << " = " << default_value << ";\n";
        else 
            ss_ << prefix_ << name<<".Reset();\n";
    }
}


}  // namespace Pepper
