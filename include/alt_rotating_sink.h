#pragma once

#include <mutex>

#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/common.h>
#include <spdlog/fmt/fmt.h>

#include <cerrno>
#include <ctime>
#include <chrono>
#include <mutex>
#include <iomanip>
#include <string>
#include <sstream>

template<typename Mutex>
class alt_rotating_file_sink final : public spdlog::sinks::base_sink<Mutex>
{
public:
     alt_rotating_file_sink(spdlog::filename_t base_filename, std::size_t max_size)
        : base_filename_(std::move(base_filename)), max_size_(max_size)
    {
        file_helper_.open(base_filename_);
        current_size_ = file_helper_.size(); // expensive. called only once
    }
    static spdlog::filename_t calc_filename(const spdlog::filename_t &filename)
    {
        std::chrono::milliseconds ms = 
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

        std::time_t t = ms.count() / 1000;
        auto millis = ms.count() % 1000;
        std::ostringstream tmstmp;
        tmstmp << std::put_time(std::localtime(&t), "%Y%m%d%H%M%S");

        return fmt::format(SPDLOG_FILENAME_T("{}.{}{:0>3}"), filename, tmstmp.str(), millis);
    }    
    spdlog::filename_t filename()
    {
        std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
        return file_helper_.filename();
    }    

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        current_size_ += formatted.size();
        if (current_size_ > max_size_)
        {
            rotate_();
            current_size_ = formatted.size();
        }
        file_helper_.write(formatted);
    }    
    void flush_() override
    {
        file_helper_.flush();
    }

private:
    // Rotate files:
    // log.txt -> log.txt.YYYYmmddHHMMSSmmm
    void rotate_()
    {
        file_helper_.close();

        if (!spdlog::details::os::path_exists(base_filename_))
        {
            return;
        }
        spdlog::filename_t target = calc_filename(base_filename_);

        if (!rename_file_(base_filename_, target))
        {
            // if failed try again after a small delay.
            // this is a workaround to a windows issue, where very high rotation
            // rates can cause the rename to fail with permission denied (because of antivirus?).
            spdlog::details::os::sleep_for_millis(100);
            if (!rename_file_(base_filename_, target))
            {
                file_helper_.reopen(true); // truncate the log file anyway to prevent it to grow beyond its limit!
                current_size_ = 0;
//		spdlog::throw_spdlog_ex("rotating_file_sink: failed renaming " 
//                                        + spdlog::details::os::filename_to_str(base_filename_) + " to " 
//                                        + spdlog::details::os::filename_to_str(target)
//                                        , errno);
            }
        }
        file_helper_.reopen(true);
    }

    // delete the target if exists, and rename the src file  to target
    // return true on success, false otherwise.
    bool rename_file_(const spdlog::filename_t &src_filename, const spdlog::filename_t &target_filename)
    {
        // try to delete the target file in case it already exists.
        (void)spdlog::details::os::remove(target_filename);
        return spdlog::details::os::rename(src_filename, target_filename) == 0;
    }

    spdlog::filename_t base_filename_;
    std::size_t max_size_;
    std::size_t current_size_;
    spdlog::details::file_helper file_helper_;
};

using alt_rotating_file_sink_mt = alt_rotating_file_sink<std::mutex>;
using alt_rotating_file_sink_st = alt_rotating_file_sink<spdlog::details::null_mutex>;

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<spdlog::logger> alt_rotating_logger_mt(
    const std::string &logger_name, const spdlog::filename_t &filename, size_t max_file_size)
{
    return Factory::template create<alt_rotating_file_sink_mt>(logger_name, filename, max_file_size);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<spdlog::logger> alt_rotating_logger_st(
    const std::string &logger_name, const spdlog::filename_t &filename, size_t max_file_size)
{
    return Factory::template create<alt_rotating_file_sink_st>(logger_name, filename, max_file_size);
}


