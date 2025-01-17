#pragma once

#include "constants.hpp"
#include "libdatadog_helpers.hpp"

#include <atomic>
#include <optional>
#include <string>
#include <string_view>

namespace Datadog {

// One of the core intrigues with crashtracker is contextualization of crashes--did a crash occur
// because of some user code, or was it this library?
// It's really hard to rule out knock-on or indirect effects, but at least crashtracker
// can mark whether a Datadog component was on-CPU at the time of the crash, and even
// indicate what it was doing.
//
// Right now the caller is assumed to only tell this system _what_ it is doing.  There's no
// available "profiling, other" state.  Just sampling, unwinding, or serializing.
struct ProfilingState
{
    std::atomic<int> is_sampling{ 0 };
    std::atomic<int> is_unwinding{ 0 };
    std::atomic<int> is_serializing{ 0 };
};

class Crashtracker
{
  private:
    bool create_alt_stack = false;
    std::optional<std::string> stderr_filename{ std::nullopt };
    std::optional<std::string> stdout_filename{ std::nullopt };
    std::string path_to_receiver_binary;
    ddog_prof_StacktraceCollection resolve_frames = DDOG_PROF_STACKTRACE_COLLECTION_WITHOUT_SYMBOLS;
    uint64_t timeout_secs = g_crashtracker_timeout_secs;

    ProfilingState profiling_state;

    std::string env;
    std::string service;
    std::string version;
    std::string runtime;
    std::string runtime_version{ "0.0.0" };
    const std::string library_name{ "dd-trace-py" };
    const std::string family{ "python" };
    std::string library_version;
    std::string url;
    std::string runtime_id;

    // Helpers for initialization/restart
    ddog_Vec_Tag get_tags();
    ddog_prof_CrashtrackerConfiguration get_config();
    ddog_prof_CrashtrackerMetadata get_metadata(ddog_Vec_Tag& tags);
    ddog_prof_CrashtrackerReceiverConfig get_receiver_config();

  public:
    // Setters
    void set_env(std::string_view _env);
    void set_service(std::string_view _service);
    void set_version(std::string_view _version);
    void set_runtime(std::string_view _runtime);
    void set_runtime_version(std::string_view _runtime_version);
    void set_library_version(std::string_view _library_version);
    void set_url(std::string_view _url);
    void set_runtime_id(std::string_view _runtime_id);

    void set_create_alt_stack(bool _create_alt_stack);
    void set_stderr_filename(std::string_view _stderr_filename);
    void set_stdout_filename(std::string_view _stdout_filename);
    bool set_receiver_binary_path(std::string_view _path_to_receiver_binary);

    void set_resolve_frames(ddog_prof_StacktraceCollection _resolve_frames);

    // Helpers
    bool start();
    bool atfork_child();

    // State transition
    void sampling_start();
    void sampling_stop();
    void unwinding_start();
    void unwinding_stop();
    void serializing_start();
    void serializing_stop();
};

} // namespace Datadog
