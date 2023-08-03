/*
 * Wazuh router - Interface tests
 * Copyright (C) 2015, Wazuh Inc.
 * Apr 29, 2022.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _INTERFACE_C_MODULE_TEST_HPP
#define _INTERFACE_C_MODULE_TEST_HPP

#include <gtest/gtest.h>

class RouterModuleCInterfaceTest : public ::testing::Test
{
protected:
    RouterModuleCInterfaceTest() = default;
    ~RouterModuleCInterfaceTest() override = default;

    void SetUp() override;
    void TearDown() override;
};
#endif //_INTERFACE_C_MODULE_TEST_HPP