/*
 * * file name: parser.cpp
 * * description: ...
 * * author: lemonxu
 * * create time:2019 5月 21
 * */

#include "parser.h"
#include <google/protobuf/descriptor.pb.h>
#include <cctype>
#include <sstream>
#include "format_conf.h"
#include "pod_options.pb.h"

using google::protobuf::Descriptor;
using google::protobuf::FieldDescriptor;
using std::stringstream;

namespace Pepper
{

string base_name(const string &str_)
{
    vector<string> res;
    stringstream input(str_);
    string temp;
    std::getline(input, temp, '.');
    return temp;
}

enum class FIELD_TYPE : size_t
{
    UNKNOW = 0,
    UINT8 = 1,
    INT8 = 2,
    UINT16 = 3,
    INT16 = 4,
    UINT32 = 5,
    INT32 = 6,
    UINT64 = 7,
    INT64 = 8,
    FLOAT = 9,
    DOUBLE = 10,
    BOOL = 11,
    CHAR = 12,
    ENUM = 13,
    STRUCT = 14,
    MAX_TYPE,
};

const BuiltInStruct *g_builtin[static_cast<size_t>(FIELD_TYPE::MAX_TYPE)] = {nullptr};
const BuiltInStruct *real_type_message(INT_TYPE type_)
{
    switch (type_)
    {
        case UINT8:
            return g_builtin[static_cast<size_t>(FIELD_TYPE::UINT8)];
        case INT8:
            return g_builtin[static_cast<size_t>(FIELD_TYPE::INT8)];
        case UINT16:
            return g_builtin[static_cast<size_t>(FIELD_TYPE::UINT16)];
        case INT16:
            return g_builtin[static_cast<size_t>(FIELD_TYPE::INT16)];
        default:
            return g_builtin[static_cast<size_t>(FIELD_TYPE::UINT32)];
    }
}

const BuiltInStruct *cpp_type_2_message(FieldDescriptor::CppType type_)
{
    switch (type_)
    {
        case FieldDescriptor::CPPTYPE_INT32:
            return g_builtin[static_cast<size_t>(FIELD_TYPE::INT32)];
        case FieldDescriptor::CPPTYPE_INT64:
            return g_builtin[static_cast<size_t>(FIELD_TYPE::INT64)];
        case FieldDescriptor::CPPTYPE_UINT32:
            return g_builtin[static_cast<size_t>(FIELD_TYPE::UINT32)];
        case FieldDescriptor::CPPTYPE_UINT64:
            return g_builtin[static_cast<size_t>(FIELD_TYPE::UINT64)];
        case FieldDescriptor::CPPTYPE_DOUBLE:
            return g_builtin[static_cast<size_t>(FIELD_TYPE::DOUBLE)];
        case FieldDescriptor::CPPTYPE_FLOAT:
            return g_builtin[static_cast<size_t>(FIELD_TYPE::FLOAT)];
        case FieldDescriptor::CPPTYPE_BOOL:
            return g_builtin[static_cast<size_t>(FIELD_TYPE::BOOL)];
        case FieldDescriptor::CPPTYPE_ENUM:
            return g_builtin[static_cast<size_t>(FIELD_TYPE::ENUM)];
        case FieldDescriptor::CPPTYPE_STRING:
            return g_builtin[static_cast<size_t>(FIELD_TYPE::CHAR)];
        case FieldDescriptor::CPPTYPE_MESSAGE:
            return g_builtin[static_cast<size_t>(FIELD_TYPE::STRUCT)];
        default:
            return g_builtin[static_cast<size_t>(FIELD_TYPE::UNKNOW)];
    }
}

//////////////////////////////////////////////////

PodMessage::~PodMessage()
{
    for (auto it : m_message_mgr)
        delete it.second;

    for (auto it : m_fields)
        delete it;
}

void PodMessage::Prepare()
{
    m_base_file_name = base_name(m_file->name());
    for (int i = 0; i < m_file->message_type_count(); ++i)
    {
        auto message = m_file->message_type(i);
        auto &options = message->options();
        auto value = options.GetExtension(gen_pod);
        // todo 这里非pod的message内嵌pod的message并不会有效，是否要生效？
        if (value == 1)
            m_message_vec.push_back(message);
    }
}

void PodMessage::InitBaseMessage()
{
    g_builtin[static_cast<size_t>(FIELD_TYPE::UNKNOW)] = nullptr;
    g_builtin[static_cast<size_t>(FIELD_TYPE::UINT8)] = new BuiltInStruct("uint8_t", "uint8_t");
    g_builtin[static_cast<size_t>(FIELD_TYPE::INT8)] = new BuiltInStruct("int8_t", "int8_t");
    g_builtin[static_cast<size_t>(FIELD_TYPE::UINT16)] = new BuiltInStruct("uint16_t", "uint16_t");
    g_builtin[static_cast<size_t>(FIELD_TYPE::INT16)] = new BuiltInStruct("int16_t", "int16_t");
    g_builtin[static_cast<size_t>(FIELD_TYPE::UINT32)] = new BuiltInStruct("uint32_t", "uint32_t");
    g_builtin[static_cast<size_t>(FIELD_TYPE::INT32)] = new BuiltInStruct("int32_t", "int32_t");
    g_builtin[static_cast<size_t>(FIELD_TYPE::UINT64)] = new BuiltInStruct("uint64_t", "uint64_t");
    g_builtin[static_cast<size_t>(FIELD_TYPE::INT64)] = new BuiltInStruct("int64_t", "int64_t");
    g_builtin[static_cast<size_t>(FIELD_TYPE::FLOAT)] = new BuiltInStruct("float", "float");
    g_builtin[static_cast<size_t>(FIELD_TYPE::DOUBLE)] = new BuiltInStruct("double", "double");
    g_builtin[static_cast<size_t>(FIELD_TYPE::BOOL)] = new BuiltInStruct("bool", "bool");
    g_builtin[static_cast<size_t>(FIELD_TYPE::CHAR)] = new BuiltInStruct("char", "char", MSG_TYPE::STRING);
    g_builtin[static_cast<size_t>(FIELD_TYPE::ENUM)] = nullptr;
    g_builtin[static_cast<size_t>(FIELD_TYPE::STRUCT)] = nullptr;
}

bool PodMessage::Parse()
{
    if (!HasPodMessage())
        return true;

    InitBaseMessage();

    m_tree.space = m_file->package();
    for (int i = 0; i < m_file->dependency_count(); ++i)
        m_tree.includes.emplace_back(base_name(m_file->dependency(i)->name()));

    for (int i = 0; i < m_file->public_dependency_count(); ++i)
        m_tree.public_includes.emplace_back(base_name(m_file->public_dependency(i)->name()));

    for (int i = 0; i < m_file->enum_type_count(); ++i)
    {
        auto enum_info = ParseEnum(m_file->enum_type(i));
        if (!enum_info)
            return false;
        m_tree.root.push_back(enum_info);
    }

    for (auto it : m_message_vec)
    {
        auto message = ParseMessage(it);
        if (!message)
            return false;
        m_tree.root.push_back(message);
    }

    return true;
}

MessageStruct *PodMessage::ParseMessage(const Descriptor *desc_, const MessageStruct *parent_)
{
    if (m_message_mgr.find(desc_->full_name()) != m_message_mgr.end())
    {
        m_err_msg = desc_->full_name() + " is not uniq";
        return nullptr;
    }

    std::unique_ptr<MessageStruct> m(new MessageStruct);
    m->name = g_message_prefix + desc_->name();

    auto parent_desc = desc_->containing_type();
    if ((parent_desc && !parent_) || (!parent_desc && parent_))
    {
        m_err_msg = "containing_type not accordance parent";
        return nullptr;
    }

    if (parent_)
    {
        m->full_name = parent_->full_name + "::" + m->name;
        m->pb_full_name = parent_->pb_full_name + "::" + desc_->name();
    }
    else
    {
        m->full_name = m->name;
        m->pb_full_name = desc_->name();
    }

    for (int i = 0; i < desc_->enum_type_count(); ++i)
    {
        auto enum_info = ParseEnum(desc_->enum_type(i), m.get());
        if (!enum_info)
            return nullptr;
        m->nest_message.push_back(enum_info);
    }

    for (int i = 0; i < desc_->nested_type_count(); ++i)
    {
        auto child_desc = desc_->nested_type(i);
        if (child_desc->options().GetExtension(gen_pod) != 1)
            continue;

        auto child = ParseMessage(child_desc, m.get());
        if (!child)
            return nullptr;
        m->nest_message.push_back(child);
    }

    for (int i = 0; i < desc_->field_count(); ++i)
    {
        auto field = ParseField(desc_->field(i));
        if (!field)
            return nullptr;
        m->fields.push_back(field);
    }

    if (!(m_message_mgr.insert(std::make_pair(desc_->full_name(), m.get())).second))
    {
        m_err_msg = desc_->full_name() + " is not uniq, ERROR";
        return nullptr;
    }

    return m.release();
}

Field *PodMessage::ParseField(const FieldDescriptor *desc_)
{
    // ignore extension field
    if (desc_->is_extension())
        return nullptr;

    std::unique_ptr<Field> f(new Field);
    f->name = desc_->name();
    f->type_message = nullptr;

    switch (desc_->cpp_type())
    {
        case FieldDescriptor::CPPTYPE_INT32:
        case FieldDescriptor::CPPTYPE_INT64:
        case FieldDescriptor::CPPTYPE_UINT32:
        case FieldDescriptor::CPPTYPE_UINT64:
        {
            if (desc_->options().HasExtension(int_type))
                f->type_message = real_type_message(desc_->options().GetExtension(int_type));
            else
                f->type_message = cpp_type_2_message(desc_->cpp_type());
            break;
        }
        case FieldDescriptor::CPPTYPE_BOOL:
        case FieldDescriptor::CPPTYPE_DOUBLE:
        case FieldDescriptor::CPPTYPE_FLOAT:
        {
            f->type_message = cpp_type_2_message(desc_->cpp_type());
            break;
        }
        case FieldDescriptor::CPPTYPE_STRING:
        {
            // string cannot repeated
            if (desc_->is_repeated())
            {
                m_err_msg = "string can not be repeated";
                return nullptr;
            }

            if (desc_->options().HasExtension(str_len))
                f->len = desc_->options().GetExtension(str_len);
            else
                f->len = 1;

            f->fixed_len = true;
            f->type_message = cpp_type_2_message(desc_->cpp_type());
            break;
        }
        case FieldDescriptor::CPPTYPE_ENUM:
        {
            // todo maybe declare in other file
            auto it = m_message_mgr.find(desc_->enum_type()->full_name());
            if (it == m_message_mgr.end())
            {
                m_err_msg = string(desc_->enum_type()->full_name()) + " is not found";
                return nullptr;
            }

            f->type_message = it->second;
            break;
        }
        case FieldDescriptor::CPPTYPE_MESSAGE:
        {
            // todo maybe declare in other file
            auto it = m_message_mgr.find(desc_->message_type()->full_name());
            if (it == m_message_mgr.end())
            {
                m_err_msg = string(desc_->message_type()->full_name()) + " is not found";
                return nullptr;
            }

            f->type_message = it->second;
            break;
        }
        default:
        {
            m_err_msg = "unknow cpp_type, ERROR";
            return nullptr;
        }
    }

    if (!(f->type_message))
    {
        m_err_msg = desc_->full_name() + " is unknow";
        return nullptr;
    }

    if (desc_->is_repeated())
    {
        if (desc_->options().HasExtension(max_count))
        {
            f->len = desc_->options().GetExtension(max_count);
            f->fixed_len = false;
        }
        else if (desc_->options().HasExtension(fixed_max_count))
        {
            f->len = desc_->options().GetExtension(fixed_max_count);
            f->fixed_len = true;
        }
    }

    m_fields.push_back(f.get());
    return f.release();
}

EnumStruct *PodMessage::ParseEnum(const ::google::protobuf::EnumDescriptor *desc_, const MessageStruct *parent_)
{
    std::unique_ptr<EnumStruct> e(new EnumStruct);
    e->name = desc_->name();

    auto parent_desc = desc_->containing_type();
    if ((parent_desc && !parent_) || (!parent_desc && parent_))
    {
        m_err_msg = "containing_type not accordance parent";
        return nullptr;
    }

    if (parent_)
    {
        e->full_name = parent_->pb_full_name + "::" + e->name;
        // enum was declare in pb.h file, so we used the full_name as it name
        e->name = e->full_name;
    }
    else
        e->full_name = e->name;

    if (!(m_message_mgr.insert(std::make_pair(desc_->full_name(), e.get())).second))
    {
        m_err_msg = desc_->full_name() + " is not uniq, ERROR";
        return nullptr;
    }

    return e.release();
}

string PodMessage::GetHeaderPrologue() const
{
    string upper_str = m_base_file_name;
    std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(), ::toupper);
    return string("// This HEADER FILE ") + m_base_file_name +
           string(".h IS GENERATED BY TOOLS. PLEASE DON'T CHANGE IT.\n#ifndef _") + upper_str +
           string("_POD_H_\n#define _") + upper_str + string("_POD_H_\n");
}

string PodMessage::GetHeaderIncludeFile() const
{
    stringstream ss;
    ss << "\n";
    for (auto it : m_tree.includes)
    {
        if (it == "pod_options")
            continue;
        ss << "#include \"" << it << ".pb.h\"\n";
    }

    if (!m_tree.public_includes.empty())
    {
        ss << "// public include\n";
        for (auto it : m_tree.public_includes)
            ss << "#include \"" << it << ".pb.h\"\n";
    }
    ss << "#include \"" << m_base_file_name << ".pb.h\"\n";
    ss << "\n";
    return ss.str();
}

string PodMessage::GetHeaderDecl() const
{
    stringstream ss;
    ss << "namespace " << m_tree.space << "\n{\n";
    for (auto message : m_tree.root)
        message->DeclareStr(ss, "");
    ss << "\n} // namespace " << m_tree.space << "\n";
    return ss.str();
}

string PodMessage::GetHeaderEpilogue() const
{
    return string("#endif");
}

string PodMessage::GetSourcePrologue() const
{
    return string("// This SOURCE FILE ") + m_base_file_name +
           string(".cc IS GENERATED BY TOOLS. PLEASE DON'T CHANGE IT.\n");
}

string PodMessage::GetSourceIncludeFile() const
{
    return string("\n#include \"") + m_base_file_name + ".pod.h\"\n\n";
}

string PodMessage::GetSourceImpl() const
{
    stringstream ss;
    ss << "namespace " << m_tree.space << "\n{\n";
    for (auto message : m_tree.root)
        message->ImplStr(ss, "");
    ss << "\n} // namespace " << m_tree.space << "\n";
    return ss.str();
}

string PodMessage::GetSourceEpilogue() const
{
    return "";
}

bool PodMessage::HasPodMessage() const
{
    return !(m_message_vec.empty());
}

string PodMessage::base_file_name() const
{
    return m_base_file_name;
}

const char *PodMessage::err_msg() const
{
    return m_err_msg.c_str();
}

}  // namespace Pepper