#include <leatherman/logging/logging.hpp>
#include <leatherman/locale/locale.hpp>

#include <boost/nowide/iostream.hpp>
#include <boost/log/sinks/event_log_backend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>

#include <windows.h>

using namespace std;
namespace sinks = boost::log::sinks;

namespace leatherman { namespace logging {
    static HANDLE stdHandle;
    static WORD originalAttributes;


    void colorize(ostream& dst, log_level level)
    {
        if (!get_colorization()) {
            return;
        }

        // The ostream may have buffered data, and changing the console color will affect any buffered data written
        // later. Ensure the buffer is flushed before changing the console color.
        dst.flush();
        if (level == log_level::trace || level == log_level::debug) {
            SetConsoleTextAttribute(stdHandle, FOREGROUND_BLUE | FOREGROUND_GREEN);
        } else if (level == log_level::info) {
            SetConsoleTextAttribute(stdHandle, FOREGROUND_GREEN);
        } else if (level == log_level::warning) {
            SetConsoleTextAttribute(stdHandle, FOREGROUND_RED | FOREGROUND_GREEN);
        } else if (level == log_level::error || level == log_level::fatal) {
            SetConsoleTextAttribute(stdHandle, FOREGROUND_RED);
        } else {
            SetConsoleTextAttribute(stdHandle, originalAttributes);
        }
    }

    void colorize(boost::log::formatting_ostream& strm, log_level level)
    {
        return;
    }

    bool color_supported(ostream& dst)
    {
        bool colorize = false;
        if (&dst == &cout || &dst == &boost::nowide::cout) {
            stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
            colorize = true;
        } else if (&dst == &cerr || &dst == &boost::nowide::cerr) {
            stdHandle = GetStdHandle(STD_ERROR_HANDLE);
            colorize = true;
        }

        if (colorize) {
            CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
            GetConsoleScreenBufferInfo(stdHandle, &csbiInfo);
            originalAttributes = csbiInfo.wAttributes;
        }
        return colorize;
    }

    void setup_event_logger(string locale)
    {
        // Using sinks::simple_event_log_backend as the backend
        using sink_t = sinks::synchronous_sink<sinks::simple_event_log_backend>;

        // Create an event log sink
        boost::shared_ptr<sink_t> sink(new sink_t());

        // Set our formatter
        sink->set_formatter(&puppetlabs_native_formatter);

        // Imbue the logging sink with the requested locale.
        sink->imbue(leatherman::locale::get_locale(locale));

        // Map log levels to the event log event types

        // TODO: check level 'none'

        sinks::event_log::custom_event_type_mapping<log_level> type_mapping("Severity");

        type_mapping[log_level::trace] = sinks::event_log::info;
        type_mapping[log_level::debug] = sinks::event_log::info;
        type_mapping[log_level::info] = sinks::event_log::info;
        type_mapping[log_level::warning] = sinks::event_log::warning;
        type_mapping[log_level::error] = sinks::event_log::error;
        type_mapping[log_level::fatal] = sinks::event_log::error;

        sink->locked_backend()->set_event_type_mapper(type_mapping);
        auto core = boost::log::core::get();
        core->add_sink(sink);
        setup_common_logging_config();

        // Turn off colorization
        set_colorization(false);
    }

}}  // namespace leatherman::logging
