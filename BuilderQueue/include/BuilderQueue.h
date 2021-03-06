#pragma once

#include "BuilderData.h"
#include <boost/asio/io_context.hpp>
#include <boost/optional.hpp>
#include <list>
#include <queue>
#include <set>
#include "boost/asio/deadline_timer.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace asio = boost::asio;

using FetchHandler = std::function<void(BuilderData builder)>;

// Note that the handler, from Connection, contains 'self' which maintains the lifetime of the connection instance
// If a client disconnects while the handler is pending it won't be cleaned up
// This should be fixed somehow, perhaps by checking the connection somehow
class BuilderQueue {
public:
    explicit BuilderQueue(asio::io_context &io_context) : io_context(io_context),
                                                          max_builder_count(25),
                                                          max_reserve_builder_count(10),
                                                          outstanding_create_count(0) {
        create_reserve_builders();
    }

    // Don't allow the queue to be copied or moved
    BuilderQueue(const BuilderQueue &) = delete;

    BuilderQueue &operator=(const BuilderQueue &) = delete;

    BuilderQueue(BuilderQueue &&) noexcept = delete;

    BuilderQueue &operator=(BuilderQueue &&)      = delete;

    // Add the specified handler to the queue
    // When a builder is ready the handler will be called and passed the builder
    template<typename FetchHandler>
    void checkout_builder(FetchHandler handler) {
        pending_handlers.emplace_back(handler);
        process_pending_handler();
    }

    // Return the builder to the queue which will destroy it
    void return_builder(BuilderData builder);

    // Process the next handler that is waiting to receive a builder, if one exists
    void process_pending_handler();

    // Create reserve builders as needed
    void create_reserve_builders();

    // Wrappers to persist timer if retrying command due to error
    // std::bind doesn't handle overloads and so wrappers seem necessary
    void retry_return_builder(BuilderData builder, std::shared_ptr<boost::asio::basic_deadline_timer<boost::posix_time::ptime> >& ignore) {
        return_builder(builder);
    }
    void retry_create_reserve_builders(std::shared_ptr<boost::asio::basic_deadline_timer<boost::posix_time::ptime> >&) {
        create_reserve_builders();
    }

    // Return a json formatted string representing the status of the queue
    std::string status_json();

private:
    asio::io_context &io_context;

    // Handlers waiting to be fulfilled
    std::list<FetchHandler> pending_handlers;

    // Builders currently available to use
    std::list<BuilderData> reserve_builders;

    // Builders currently in use by connections
    std::list<BuilderData> active_builders;

    // Maximum number of active and reserve builders
    const std::size_t max_builder_count;
    // Maximum number of builders to spin and keep up in reserve
    const std::size_t max_reserve_builder_count;

    // Number of build requests that have been requested but not yet completed
    int outstanding_create_count;
};