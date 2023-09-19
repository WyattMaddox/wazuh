/*
 * Wazuh content manager - Component Tests
 * Copyright (C) 2015, Wazuh Inc.
 * July 26, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "action_test.hpp"
#include "action.hpp"
#include "routerProvider.hpp"
#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>

/*
 * @brief Tests the instantiation of the Action class
 */
TEST_F(ActionTest, TestInstantiation)
{
    const auto& topicName {m_parameters.at("topicName").get_ref<const std::string&>()};
    const auto& outputFolder {m_parameters.at("configData").at("outputFolder").get_ref<const std::string&>()};

    auto routerProvider {std::make_shared<RouterProvider>(topicName)};

    EXPECT_NO_THROW(routerProvider->start());

    EXPECT_NO_THROW(std::make_shared<Action>(routerProvider, topicName, m_parameters));

    EXPECT_TRUE(std::filesystem::exists(outputFolder));

    EXPECT_NO_THROW(routerProvider->stop());
}

/*
 * @brief Tests the instantiation of the Action class without configData
 */
TEST_F(ActionTest, TestInstantiationWhitoutConfigData)
{
    // creates a copy of `m_parameters` because it's used in `TearDown` method
    auto parameters = m_parameters;

    const auto& topicName {parameters.at("topicName").get_ref<const std::string&>()};

    auto routerProvider {std::make_shared<RouterProvider>(topicName)};

    EXPECT_NO_THROW(routerProvider->start());

    parameters.erase("configData");

    EXPECT_THROW(std::make_shared<Action>(routerProvider, topicName, parameters), std::invalid_argument);

    EXPECT_NO_THROW(routerProvider->stop());
}

/*
 * @brief Tests the instantiation of the Action class and execution of startActionScheduler for raw data
 */
TEST_F(ActionTest, TestInstantiationAndStartActionSchedulerForRawData)
{
    const auto& topicName {m_parameters.at("topicName").get_ref<const std::string&>()};
    const auto& outputFolder {m_parameters.at("configData").at("outputFolder").get_ref<const std::string&>()};
    const auto& fileName {m_parameters.at("configData").at("contentFileName").get_ref<const std::string&>()};
    const auto& interval {m_parameters.at("interval").get_ref<const size_t&>()};
    const auto contentPath {outputFolder + "/" + CONTENTS_FOLDER + "/" + fileName};
    const auto downloadPath {outputFolder + "/" + DOWNLOAD_FOLDER + "/" + fileName};

    auto routerProvider {std::make_shared<RouterProvider>(topicName)};

    EXPECT_NO_THROW(routerProvider->start());

    auto action {std::make_shared<Action>(routerProvider, topicName, m_parameters)};

    EXPECT_TRUE(std::filesystem::exists(outputFolder));

    EXPECT_NO_THROW(action->startActionScheduler(interval));

    std::this_thread::sleep_for(std::chrono::seconds(interval + 1));

    EXPECT_NO_THROW(action->stopActionScheduler());

    // This file shouldn't exist because it's a test for raw data
    EXPECT_FALSE(std::filesystem::exists(downloadPath));

    EXPECT_TRUE(std::filesystem::exists(contentPath));

    EXPECT_TRUE(std::filesystem::exists(outputFolder));

    EXPECT_NO_THROW(routerProvider->stop());
}

/*
 * @brief Tests the instantiation of the Action class and execution of startActionScheduler for compressed data with
 * deleteDownloadedContent enabled
 */
TEST_F(ActionTest, TestInstantiationAndStartActionSchedulerForRawDataWithDeleteDownloadedContentEnabled)
{
    m_parameters["configData"]["url"] = "http://localhost:4444/xz";
    m_parameters["configData"]["compressionType"] = "xz";
    m_parameters["configData"]["deleteDownloadedContent"] = true;

    const auto& topicName {m_parameters.at("topicName").get_ref<const std::string&>()};
    const auto& outputFolder {m_parameters.at("configData").at("outputFolder").get_ref<const std::string&>()};
    const auto& fileName {m_parameters.at("configData").at("contentFileName").get_ref<const std::string&>()};
    const auto& interval {m_parameters.at("interval").get_ref<const size_t&>()};
    const auto contentPath {outputFolder + "/" + CONTENTS_FOLDER + "/" + fileName};
    const auto downloadPath {outputFolder + "/" + DOWNLOAD_FOLDER + "/" + fileName};

    auto routerProvider {std::make_shared<RouterProvider>(topicName)};

    EXPECT_NO_THROW(routerProvider->start());

    auto action {std::make_shared<Action>(routerProvider, topicName, m_parameters)};

    EXPECT_TRUE(std::filesystem::exists(outputFolder));

    EXPECT_NO_THROW(action->startActionScheduler(interval));

    std::this_thread::sleep_for(std::chrono::seconds(interval + 1));

    EXPECT_NO_THROW(action->stopActionScheduler());

    // This file shouldn't exist because deleteDownloadedContent is enabled
    EXPECT_FALSE(std::filesystem::exists(downloadPath));

    EXPECT_TRUE(std::filesystem::exists(contentPath));

    EXPECT_TRUE(std::filesystem::exists(outputFolder));

    EXPECT_NO_THROW(routerProvider->stop());
}

/*
 * @brief Tests the instantiation of the Action class and execution of startActionScheduler for
 * compressed data
 */
TEST_F(ActionTest, TestInstantiationAndStartActionSchedulerForCompressedData)
{
    m_parameters["configData"]["url"] = "http://localhost:4444/xz";
    m_parameters["configData"]["compressionType"] = "xz";

    const auto& topicName {m_parameters.at("topicName").get_ref<const std::string&>()};
    const auto& outputFolder {m_parameters.at("configData").at("outputFolder").get_ref<const std::string&>()};
    const auto& fileName {m_parameters.at("configData").at("contentFileName").get_ref<const std::string&>()};
    const auto& interval {m_parameters.at("interval").get_ref<const size_t&>()};
    const auto contentPath {outputFolder + "/" + CONTENTS_FOLDER + "/" + fileName};
    const auto downloadPath {outputFolder + "/" + DOWNLOAD_FOLDER + "/" + fileName};

    auto routerProvider {std::make_shared<RouterProvider>(topicName)};

    EXPECT_NO_THROW(routerProvider->start());

    auto action {std::make_shared<Action>(routerProvider, topicName, m_parameters)};

    EXPECT_TRUE(std::filesystem::exists(outputFolder));

    EXPECT_NO_THROW(action->startActionScheduler(interval));

    std::this_thread::sleep_for(std::chrono::seconds(interval + 1));

    EXPECT_NO_THROW(action->stopActionScheduler());

    // This file should exist because deleteDownloadedContent is not enabled
    EXPECT_TRUE(std::filesystem::exists(downloadPath));

    EXPECT_TRUE(std::filesystem::exists(contentPath));

    EXPECT_TRUE(std::filesystem::exists(outputFolder));

    EXPECT_NO_THROW(routerProvider->stop());
}

/*
 * @brief Tests the instantiation of the Action class and execution of registerActionOnDemand for raw data
 */
TEST_F(ActionTest, TestInstantiationAndRegisterActionOnDemandForRawData)
{
    const auto& topicName {m_parameters.at("topicName").get_ref<const std::string&>()};
    const auto& outputFolder {m_parameters.at("configData").at("outputFolder").get_ref<const std::string&>()};

    auto routerProvider {std::make_shared<RouterProvider>(topicName)};

    EXPECT_NO_THROW(routerProvider->start());

    m_parameters["ondemand"] = true;

    auto action {std::make_shared<Action>(routerProvider, topicName, m_parameters)};

    EXPECT_TRUE(std::filesystem::exists(outputFolder));

    EXPECT_NO_THROW(action->registerActionOnDemand());

    EXPECT_NO_THROW(action->unregisterActionOnDemand());

    EXPECT_NO_THROW(action->clearEndpoints());

    EXPECT_NO_THROW(routerProvider->stop());
}

/*
 * @brief Tests the instantiation of two Actions on demand with the same topicName
 */
TEST_F(ActionTest, TestInstantiationOfTwoActionsWithTheSameTopicName)
{
    const auto& topicName {m_parameters.at("topicName").get_ref<const std::string&>()};
    const auto& outputFolder {m_parameters.at("configData").at("outputFolder").get_ref<const std::string&>()};

    auto routerProvider {std::make_shared<RouterProvider>(topicName)};

    EXPECT_NO_THROW(routerProvider->start());

    m_parameters["ondemand"] = true;

    auto action1 {std::make_shared<Action>(routerProvider, topicName, m_parameters)};
    auto action2 {std::make_shared<Action>(routerProvider, topicName, m_parameters)};

    EXPECT_TRUE(std::filesystem::exists(outputFolder));

    EXPECT_NO_THROW(action1->registerActionOnDemand());
    EXPECT_THROW(action2->registerActionOnDemand(), std::runtime_error);

    EXPECT_NO_THROW(action1->unregisterActionOnDemand());

    EXPECT_NO_THROW(action1->clearEndpoints());

    EXPECT_NO_THROW(routerProvider->stop());
}

/*
 * @brief Tests the instantiation of the Action class and runActionOnDemand
 */
TEST_F(ActionTest, TestInstantiationAndRunActionOnDemand)
{
    m_parameters["ondemand"] = true;

    const auto& topicName {m_parameters.at("topicName").get_ref<const std::string&>()};
    const auto& outputFolder {m_parameters.at("configData").at("outputFolder").get_ref<const std::string&>()};
    const auto& fileName {m_parameters.at("configData").at("contentFileName").get_ref<const std::string&>()};
    const auto contentPath {outputFolder + "/" + CONTENTS_FOLDER + "/" + fileName};
    const auto downloadPath {outputFolder + "/" + DOWNLOAD_FOLDER + "/" + fileName};

    auto routerProvider {std::make_shared<RouterProvider>(topicName)};

    EXPECT_NO_THROW(routerProvider->start());

    auto action {std::make_shared<Action>(routerProvider, topicName, m_parameters)};

    EXPECT_TRUE(std::filesystem::exists(outputFolder));

    EXPECT_NO_THROW(action->registerActionOnDemand());

    EXPECT_NO_THROW(action->runActionOnDemand());

    std::this_thread::sleep_for(std::chrono::seconds(1));

    EXPECT_NO_THROW(action->unregisterActionOnDemand());
    EXPECT_NO_THROW(action->clearEndpoints());

    // This file shouldn't exist because it's a test for raw data
    EXPECT_FALSE(std::filesystem::exists(downloadPath));

    EXPECT_TRUE(std::filesystem::exists(contentPath));

    EXPECT_TRUE(std::filesystem::exists(outputFolder));

    EXPECT_NO_THROW(routerProvider->stop());
}
