/*
 * * file name: grammar.h
 * * description: ...
 * * author: snow
 * * create time:2019 5月 21
 * */

#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <sstream>
#include <string>
#include <vector>
#include "global_var.h"

using std::string;
using std::stringstream;
using std::vector;

namespace Pepper
{
enum class MSG_TYPE : uint8_t
{
    SIMPLE = 0,
    STRING = 1,
    STRUCT = 2,
    ENUM = 3,
    BYTES = 4,
};

struct BaseStruct
{
    string name;
    string full_name;
    MSG_TYPE msg_type;

    BaseStruct(const string& name_, const string full_name_, MSG_TYPE type_)
        : name(name_), full_name(full_name_), msg_type(type_)
    {
    }
    virtual ~BaseStruct() = default;
    // return false if is nothing output
    virtual bool DeclareStr(stringstream& ss_, const string& prefix_) const = 0;
    // return false if is nothing output
    virtual bool ImplStr(stringstream& ss_, const string& prefix_) const = 0;
};

struct BuiltInStruct : public BaseStruct
{
    BuiltInStruct(const string& name_, const string& full_name_, MSG_TYPE type_ = MSG_TYPE::SIMPLE)
        : BaseStruct{name_, full_name_, type_}
    {
    }
    virtual ~BuiltInStruct() = default;
    virtual bool DeclareStr(stringstream& ss_, const string& prefix_) const final { return false; };
    virtual bool ImplStr(stringstream& ss_, const string& prefix_) const final { return false; };
};


struct BaseMessageStruct;
struct EnumStruct : public BaseStruct
{
    const BaseMessageStruct* parent_message = nullptr;

    EnumStruct() : BaseStruct{"", "", MSG_TYPE::ENUM} {}
    virtual ~EnumStruct() = default;
    virtual bool DeclareStr(stringstream& ss_, const string& prefix_) const final { return false; };
    virtual bool ImplStr(stringstream& ss_, const string& prefix_) const final { return false; };
};

struct BaseMessageStruct : public BaseStruct
{
    string pb_full_name;

    BaseMessageStruct() : BaseStruct{"", "", MSG_TYPE::STRUCT}, pb_full_name("") {}
    virtual ~BaseMessageStruct() = default;
    virtual bool DeclareStr(stringstream& ss_, const string& prefix_) const override { return false; }
    virtual bool ImplStr(stringstream& ss_, const string& prefix_) const override { return false; }
};

struct BaseStrStruct : public BaseStruct
{
    
    size_t len = 1;
    string default_value;

    BaseStrStruct(const string& name_, const string& full_name_, size_t _len, 
        const string& _default_value,  MSG_TYPE type_) 
        : BaseStruct(name_, full_name_, type_), len(_len), default_value(_default_value)
         {}

    virtual ~BaseStrStruct() = default;
    virtual bool DeclareStr(stringstream& ss_, const string& prefix_) const override { return false; }
    virtual bool ImplStr(stringstream& ss_, const string& prefix_) const override { return false; }
};

struct StrStruct : public BaseStrStruct
{
    
    StrStruct(const string& name_, const string& full_name_, size_t _len, 
        const string& _default_value) 
        : BaseStrStruct(name_, full_name_, _len, _default_value, MSG_TYPE::STRING){}
    virtual ~StrStruct() = default;
    virtual bool DeclareStr(stringstream& ss_, const string& prefix_) const final ;
    virtual bool ImplStr(stringstream& ss_, const string& prefix_) const final;
};

struct BytesStruct : public BaseStrStruct
{
    BytesStruct(const string& name_, const string& full_name_, size_t _len, 
        const string& _default_value) 
        : BaseStrStruct(name_, full_name_, _len, _default_value, MSG_TYPE::BYTES){}
    virtual ~BytesStruct() = default;
    virtual bool DeclareStr(stringstream& ss_, const string& prefix_) const final ;
    virtual bool ImplStr(stringstream& ss_, const string& prefix_) const final ;
};

struct Field;
struct MessageStruct : public BaseMessageStruct
{
    vector<BaseStruct*> nest_message;
    vector<Field*> fields;

    MessageStruct() = default;
    virtual ~MessageStruct() = default;
    virtual bool DeclareStr(stringstream& ss_, const string& prefix_) const final;
    virtual bool ImplStr(stringstream& ss_, const string& prefix_) const final;

private:
    void ConstructorDeclareStr(stringstream& ss_, const string& prefix_) const;
    void ResetDeclareStr(stringstream& ss_, const string& prefix_) const;
    void ClearDeclareStr(stringstream& ss_, const string& prefix_) const;
    void FromPbDeclareStr(stringstream& ss_, const string& prefix_) const;
    void ToPbDeclareStr(stringstream& ss_, const string& prefix_) const;

    void ConstructorImplStr(stringstream& ss_, const string& prefix_) const;
    void ResetImplStr(stringstream& ss_, const string& prefix_) const;
    void ClearImplStr(stringstream& ss_, const string& prefix_) const;
    void FromPbImplStr(stringstream& ss_, const string& prefix_) const;
    void ToPbImplStr(stringstream& ss_, const string& prefix_) const;
};

// an unknow struct is can not found declare in this proto file,
// it may be declare in dependendent files
struct UnknowStruct : public BaseStruct
{
    UnknowStruct() : UnknowStruct("", "") {}
    UnknowStruct(const string& name_, const string& full_name_) : BaseStruct{name_, full_name_, MSG_TYPE::STRUCT} {}
    virtual ~UnknowStruct() = default;
    virtual bool DeclareStr(stringstream& ss_, const string& prefix_) const final { return false; };
    virtual bool ImplStr(stringstream& ss_, const string& prefix_) const final { return false; };
};

struct Field
{
    string name;
    string default_value;
    const BaseStruct* type_message = nullptr;
    
    size_t len = 0;
    
    size_t array_len = 0;
    bool array_fixed_len = false;

    void DeclareStr(stringstream& ss_, const string& prefix_, const string& pb_full_name_) const;

    void ImplStr(stringstream& ss_, const string& prefix_, const string& pb_full_name_) const;

    void SetPbStr(stringstream& ss_, const string& prefix_) const;

    void GetPbStr(stringstream& ss_, const string& prefix_) const;

    void InitStr(stringstream& ss_, const string& prefix_) const;

};

struct SyntaxTree
{
    vector<string> includes;
    vector<string> public_includes;
    string space;
    vector<BaseStruct*> root;
};

}  // namespace Pepper

#endif
