#ifndef _ROUTER_ENVIRONMENT_BUILD_HPP
#define _ROUTER_ENVIRONMENT_BUILD_HPP

#include <memory>
#include <unordered_set>
#include <utility>

#include <bk/icontroller.hpp>

#include "environment.hpp"
#include "ibuilder.hpp"

namespace router
{

/**
 * @brief Class used to build an environment.
 *
 */
class EnvironmentBuilder
{
private:
    // TODO Make a m_builder a weak_ptr when the IBuilder lifetime is managed by the engine start
    std::shared_ptr<IBuilder> m_builder;                     ///< The builder used to construct the policy and filter.
    std::shared_ptr<bk::IControllerMaker> m_controllerMaker; ///< The controller maker used to construct the controller.

    /**
     * @brief Get the Expression object for a given filter.
     *
     * @param filterName The name of the filter.
     * @return base::Expression The constructed filter expression.
     * @throws std::runtime_error if the filter cannot be built.
     */
    base::Expression getExpression(const base::Name& filterName)
    {
        // TODO: Remove this check when the Builder can identify if it is a filter or not
        if (filterName.parts().size() == 0 || filterName.parts()[0] != "filter")
        {
            throw std::runtime_error {"The asset name is empty or it is not a filter"};
        }
        auto filter = m_builder->buildAsset(filterName);
        if (base::isError(filter))
        {
            throw std::runtime_error {base::getError(filter).message};
        }

        return base::getResponse(filter);
    }

public:
    /**
     * @brief Create a new EnvironmentBuilder
     *
     */
    EnvironmentBuilder(std::shared_ptr<IBuilder> builder, std::shared_ptr<bk::IControllerMaker> controllerMaker)
        : m_builder(std::move(builder))
        , m_controllerMaker(std::move(controllerMaker))
    {
        if (m_builder == nullptr)
        {
            throw std::runtime_error {"Cannot create BuildEnvironment with a null builder"};
        }

        if (m_controllerMaker == nullptr)
        {
            throw std::runtime_error {"Cannot create BuildEnvironment with a null controller maker"};
        }
    }

    EnvironmentBuilder() = delete;

    /**
     * @brief Get the Controller object for a given policy.
     *
     * @param policyName The name of the policy.
     * @param builder The builder used to construct the policy.
     * @return std::shared_ptr<bk::IController> The constructed controller.
     * @throws std::runtime_error if the policy has no assets or if the backend cannot be built. // TODO Move to
     * base::Error
     */
    auto makeController(const base::Name& policyName) -> std::pair<std::shared_ptr<bk::IController>, std::string>
    {
        if (policyName.parts().size() == 0 || policyName.parts()[0] != "policy")
        {
            throw std::runtime_error {"The asset name is empty or it is not a policy"};
        }
        // Build the policy and create the pipeline
        auto newPolicy = m_builder->buildPolicy(policyName);
        if (base::isError(newPolicy))
        {
            throw std::runtime_error {base::getError(newPolicy).message};
        }

        auto policy = base::getResponse(newPolicy);
        if (policy->assets().empty())
        {
            throw std::runtime_error {fmt::format("Policy '{}' has no assets", policyName)};
        }

        // TODO Check de assets names policy api (Return a string instead of a base::Names?)
        std::unordered_set<std::string> assetNames;
        std::transform(policy->assets().begin(),
                       policy->assets().end(),
                       std::inserter(assetNames, assetNames.begin()),
                       [](const auto& name) { return name.toStr(); });

        auto controller = m_controllerMaker->create();
        controller->build(policy->expression(), assetNames);
        return {controller, policy->hash()};
    }

    /**
     * @brief Create an environment based on a policy and a filter.
     *
     * @param policyName The name of the policy.
     * @param filterName The name of the filter.
     * @return Environment The created environment.
     * @throws std::runtime_error if failed to create the environment. // TODO CHange to base::Error
     */
    std::unique_ptr<Environment> create(const base::Name& policyName, const base::Name& filterName)
    {
        std::shared_ptr<bk::IController> controller = nullptr;
        try
        {
            std::string hash {};
            std::tie(controller, hash) = makeController(policyName);
            auto expression = getExpression(filterName);
            return std::make_unique<Environment>(std::move(expression), std::move(controller), std::move(hash));
        }
        catch (const std::runtime_error& e)
        {
            if (controller != nullptr)
            {
                controller->stop();
            }
            throw std::runtime_error {fmt::format(
                "Failed to create environment with policy '{}' and filter '{}': {}", policyName, filterName, e.what())};
        }
    }
};

} // namespace router

#endif //_ROUTER_ENVIRONMENT_BUILD_HPP