#include <leatherman/logging/logging.hpp>
#include <boost/nowide/iostream.hpp>
#include <unistd.h>

using namespace std;

namespace leatherman { namespace logging {

    template <typename StreamT>
    void colorize_impl(StreamT& strm, log_level level)
    {
        if (!get_colorization()) {
            return;
        }

        static const string cyan = "\33[0;36m";
        static const string green = "\33[0;32m";
        static const string yellow = "\33[0;33m";
        static const string red = "\33[0;31m";
        static const string reset = "\33[0m";

        if (level == log_level::trace || level == log_level::debug) {
            strm << cyan;
        } else if (level == log_level::info) {
            strm << green;
        } else if (level == log_level::warning) {
            strm << yellow;
        } else if (level == log_level::error || level == log_level::fatal) {
            strm << red;
        } else {
            strm << reset;
        }
    }

    void colorize(ostream& dst, log_level level)
    {
        colorize_impl<ostream>(dst, level);
    }

    void colorize(boost::log::formatting_ostream& strm, log_level level)
    {
        colorize_impl<boost::log::formatting_ostream>(strm, level);
    }

    bool color_supported(ostream& dst)
    {
        return (&dst == &cout && isatty(fileno(stdout))) || (&dst == &cerr && isatty(fileno(stderr)));
    }

    void setup_event_logger(string locale)
    {
        // TODO: assert or throw something?
        return;
    }

}}  // namespace leatherman::logging
