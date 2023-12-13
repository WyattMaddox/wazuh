#ifndef _ROUTER_ROUTERADMIN_HPP
#define _ROUTER_ROUTERADMIN_HPP

#include <list>
#include <memory>
#include <shared_mutex>

#include <bk/icontroller.hpp>
#include <queue/iqueue.hpp>
#include <store/istore.hpp>

#include <builder/registry.hpp>
#include <parseEvent.hpp>

#include <router/iapi.hpp>
#include <router/types.hpp>

namespace router
{

using ProdQueueType = base::queue::iQueue<base::Event>;
using TestQueueType = base::queue::iQueue<test::QueueType>;

// Forward declarations
class Worker;
class EnvironmentBuilder;

// Change name to syncronizer
class Orchestrator
    : public IRouterAPI
    , public ITesterAPI
{

private:
    constexpr static const char* STORE_PATH_TESTER_TABLE = "router/tester/0"; ///< Default path for the tester state
    constexpr static const char* STORE_PATH_ROUTER_TABLE = "router/router/0"; ///< Default path for the router state

    // Workers synchronization
    std::list<std::shared_ptr<Worker>> m_workers; ///< List of workers
    mutable std::shared_mutex m_syncMutex;        ///< Mutex for the Workers synchronization (1 query at a time)

    // Workers configuration
    std::shared_ptr<ProdQueueType> m_eventQueue;      ///< The event queue
    std::shared_ptr<TestQueueType> m_testQueue;       ///< The test queue
    std::shared_ptr<EnvironmentBuilder> m_envBuilder; ///< The environment builder

    // Configuration options
    std::weak_ptr<store::IStoreInternal> m_wStore; ///< Read and store configurations
    base::Name m_storeTesterName;                  ///< Path of internal configuration state for testers
    base::Name m_storeRouterName;                  ///< Path of internal configuration state for routers
    std::size_t m_testTimeout;                     ///< Timeout for the tests

    using WorkerOp = std::function<base::OptError(const std::shared_ptr<Worker>&)>;
    base::OptError forEachWorker(const WorkerOp& f); ///< Apply the function f to each worker

    void dumpTesters() const; ///< Dump the testers to the store
    void dumpRouters() const; ///< Dump the routers to the store
    void initWorkers();      ///< Initialize the workers from the store when the orchestrator is created

public:
    ~Orchestrator() = default;
    Orchestrator() = delete;
    /**
     * @brief Configuration for the Orchestrator
     *
     */
    struct Options
    {
        int m_numThreads; ///< Number of workers to create

        std::weak_ptr<store::IStore> m_wStore; ///< Store to read namespaces and configurations
        std::weak_ptr<builder::internals::Registry<builder::internals::Builder>> m_wRegistry; ///< Registry of builders

        std::shared_ptr<bk::IControllerMaker> m_controllerMaker; ///< Controller maker for creating controllers
        std::shared_ptr<ProdQueueType> m_prodQueue;              ///< The event queue
        std::shared_ptr<TestQueueType> m_testQueue;              ///< The test queue

        int m_testTimeout; ///< Timeout for handlers of testers

        void validate() const; ///< Validate the configuration options if is invalid throw an  std::runtime_error
    };

    Orchestrator(const Options& opt);

    /**
     * @brief Start the router
     *
     */
    void start();

    /**
     * @brief Stop the router
     *
     */
    void stop();

    /**
     * @brief Push an event to the event queue
     *
     * @param eventStr The event to push
     */
    void pushEvent(const std::string& eventStr)
    {
        base::Event event;
        try
        {
            event = base::parseEvent::parseWazuhEvent(eventStr);
            m_eventQueue->push(std::move(event));
        }
        catch (const std::exception& e)
        {
            LOG_WARNING("Error parsing event: '{}' (discarding...)", e.what());
        }
    }

    /**************************************************************************
     * IRouterAPI
     *************************************************************************/

    /**
     * @copydoc router::IRouterAPI::postEnvironment
     */
    base::OptError postEntry(const prod::EntryPost& entry) override;

    /**
     * @copydoc router::IRouterAPI::deleteEnvironment
     */
    base::OptError deleteEntry(const std::string& name) override;

    /**
     * @copydoc router::IRouterAPI::getEnvironment
     */
    base::RespOrError<prod::Entry> getEntry(const std::string& name) const override;

    /**
     * @copydoc router::IRouterAPI::reloadEnvironment
     */
    base::OptError reloadEntry(const std::string& name) override;

    /**
     * @copydoc router::IRouterAPI::ChangeEnvironmentPriority
     */
    base::OptError changeEntryPriority(const std::string& name, size_t priority) override;

    /**
     * @copydoc router::IRouterAPI::getEntries
     */
    std::list<prod::Entry> getEntries() const override;

    /**
     * @copydoc router::IRouterAPI::postEvent
     */
    void postEvent(base::Event&& event) override { m_eventQueue->push(std::move(event)); }

    /**
     * @copydoc router::IRouterAPI::postStrEvent
     */
    base::OptError postStrEvent(std::string_view event) override;

    /**************************************************************************
     * ITesterAPI
     *************************************************************************/

    /**
     * @copydoc router::ITesterAPI::postTestEnvironment
     */
    base::OptError postTestEntry(const test::EntryPost& entry) override;

    /**
     * @copydoc router::ITesterAPI::deleteTestEnvironment
     */
    base::OptError deleteTestEntry(const std::string& name) override;

    /**
     * @copydoc router::ITesterAPI::getTestEnvironment
     */
    base::RespOrError<test::Entry> getTestEntry(const std::string& name) const override;

    /**
     * @copydoc router::ITesterAPI::reloadTestEnvironment
     */
    base::OptError reloadTestEntry(const std::string& name) override;

    /**
     * @copydoc router::ITesterAPI::getTestEntries
     */
    std::list<test::Entry> getTestEntries() const override;

    /**
     * @copydoc router::ITesterAPI::ingestTest
     */
    std::future<base::RespOrError<test::Output>> ingestTest(base::Event&& event, const test::Options& opt) override;

    /**
     * @copydoc router::ITesterAPI::ingestTest
     */
    std::future<base::RespOrError<test::Output>> ingestTest(std::string_view event, const test::Options& opt) override;

    /**
     * @copydoc router::ITesterAPI::getAssets
     */
    base::RespOrError<std::unordered_set<std::string>> getAssets(const std::string& name) const override;

    /**
     * @copydoc router::ITesterAPI::getTestTimeout
     */
    std::size_t getTestTimeout() const override { return m_testTimeout; }
};

} // namespace router

#endif // _ROUTER_ROUTERADMIN_HPP