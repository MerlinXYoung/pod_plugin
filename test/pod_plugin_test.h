/*
 * * file name: pod_plugin_test.h
 * * description: ...
 * * author: snow
 * * create time:2019  6 05
 * */

#ifndef _POD_PLUGIN_TEST_H_
#define _POD_PLUGIN_TEST_H_

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include "gtest/gtest.h"
#include "test.pod.h"
#include <type_traits>
#include <array>
#include "array.hpp"

using std::map;
using std::set;
using namespace Test;
struct A{
    int a;
    double b;
};

struct B{
    int a;
    double b;
    ParentMsg::ParentNestedEnum parent_enum ;//{ ParentMsg::P_NESTED_ONE};
    void set_b(double d){b=d;}
    // B(int aa):a(aa),b(0.0f){}
};
TEST(PodPluginTest, to_pb)
{
    PODBaseMsg pod_base_msg;
    pod_base_msg.id = 10;

    BaseMsg base_msg;
    pod_base_msg.To(&base_msg);

    EXPECT_EQ(base_msg.id(), pod_base_msg.id);
    EXPECT_EQ(pod_base_msg.id, 10);

    using array_type = Array<A, uint8_t, 15>;

    EXPECT_TRUE(std::is_pod<array_type >::value);

    EXPECT_TRUE(std::is_pod<ParentMsg_ParentNestedEnum>::value);

    EXPECT_TRUE(std::is_pod<A>::value);
    EXPECT_TRUE(std::is_pod<B>::value);
    EXPECT_TRUE(std::is_standard_layout<B>::value);

    EXPECT_TRUE(std::is_pod<PODParentMsg>::value);
    EXPECT_TRUE(std::is_standard_layout<PODParentMsg>::value);

    EXPECT_TRUE(std::is_pod<PODBaseMsg::PODNestedMsg>::value);
    EXPECT_TRUE(std::is_standard_layout<PODBaseMsg::PODNestedMsg>::value);

    EXPECT_TRUE(std::is_pod<PODBaseMsg>::value);
    EXPECT_TRUE(std::is_standard_layout<PODBaseMsg>::value);
}

#endif
