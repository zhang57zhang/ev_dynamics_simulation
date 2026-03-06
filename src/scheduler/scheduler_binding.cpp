/**
 * @file scheduler_binding.cpp
 * @brief 实时调度器Python绑定
 * 
 * 包括：
 * - RealtimeScheduler: 实时调度器
 * - TaskManager: 任务管理器
 * - Monitor: 性能监控
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <chrono>
#include <vector>
#include <string>
#include <functional>
#include <map>

namespace py = pybind11;

// ============================================================================
// 任务定义
// ============================================================================
struct Task {
    std::string name;
    double frequency; // Hz
    std::function<void(double)> callback;
    double last_execution_time;
    double execution_count;
    
    Task(const std::string& n, double freq, std::function<void(double)> cb)
        : name(n), frequency(freq), callback(cb),
          last_execution_time(0.0), execution_count(0.0) {}
};

// ============================================================================
// 任务管理器（占位符）
// ============================================================================
class TaskManager {
public:
    TaskManager() = default;
    
    void add_task(const std::string& name, double frequency, 
                  std::function<void(double)> callback) {
        tasks_.emplace_back(name, frequency, callback);
    }
    
    void remove_task(const std::string& name) {
        tasks_.erase(
            std::remove_if(tasks_.begin(), tasks_.end(),
                [&name](const Task& t) { return t.name == name; }),
            tasks_.end()
        );
    }
    
    std::vector<std::string> get_task_names() const {
        std::vector<std::string> names;
        for (const auto& task : tasks_) {
            names.push_back(task.name);
        }
        return names;
    }
    
    size_t get_task_count() const {
        return tasks_.size();
    }
    
private:
    std::vector<Task> tasks_;
};

// ============================================================================
// 性能监控（占位符）
// ============================================================================
class Monitor {
public:
    Monitor() : total_time_(0.0), frame_count_(0) {}
    
    void record_frame(double elapsed_time) {
        total_time_ += elapsed_time;
        frame_count_++;
    }
    
    double get_average_frame_time() const {
        if (frame_count_ == 0) return 0.0;
        return total_time_ / frame_count_;
    }
    
    double get_fps() const {
        if (total_time_ == 0.0) return 0.0;
        return frame_count_ / total_time_;
    }
    
    void reset() {
        total_time_ = 0.0;
        frame_count_ = 0;
    }
    
private:
    double total_time_;
    int frame_count_;
};

// ============================================================================
// 实时调度器（占位符）
// ============================================================================
class RealtimeScheduler {
public:
    RealtimeScheduler(double frequency)
        : frequency_(frequency), running_(false), current_time_(0.0) {
        dt_ = 1.0 / frequency;
    }
    
    void set_frequency(double frequency) {
        frequency_ = frequency;
        dt_ = 1.0 / frequency;
    }
    
    double get_frequency() const {
        return frequency_;
    }
    
    double get_dt() const {
        return dt_;
    }
    
    void start() {
        running_ = true;
    }
    
    void stop() {
        running_ = false;
    }
    
    bool is_running() const {
        return running_;
    }
    
    void step() {
        if (!running_) return;
        
        // 更新所有任务
        for (auto& task : tasks_) {
            double period = 1.0 / task.frequency;
            if (current_time_ - task.last_execution_time >= period) {
                task.callback(dt_);
                task.last_execution_time = current_time_;
                task.execution_count++;
            }
        }
        
        current_time_ += dt_;
    }
    
    void add_task(const std::string& name, double frequency,
                  std::function<void(double)> callback) {
        tasks_.emplace_back(name, frequency, callback);
    }
    
    double get_current_time() const {
        return current_time_;
    }
    
private:
    double frequency_;
    double dt_;
    bool running_;
    double current_time_;
    std::vector<Task> tasks_;
};

// ============================================================================
// Python模块定义
// ============================================================================
PYBIND11_MODULE(scheduler, m) {
    m.doc() = "Scheduler module: realtime scheduler, task manager, and monitor";
    
    // TaskManager类
    py::class_<TaskManager>(m, "TaskManager")
        .def(py::init<>())
        .def("add_task", &TaskManager::add_task,
             py::arg("name"), py::arg("frequency"), py::arg("callback"))
        .def("remove_task", &TaskManager::remove_task, py::arg("name"))
        .def("get_task_names", &TaskManager::get_task_names)
        .def("get_task_count", &TaskManager::get_task_count);
    
    // Monitor类
    py::class_<Monitor>(m, "Monitor")
        .def(py::init<>())
        .def("record_frame", &Monitor::record_frame, py::arg("elapsed_time"))
        .def("get_average_frame_time", &Monitor::get_average_frame_time)
        .def("get_fps", &Monitor::get_fps)
        .def("reset", &Monitor::reset);
    
    // RealtimeScheduler类
    py::class_<RealtimeScheduler>(m, "RealtimeScheduler")
        .def(py::init<double>(), py::arg("frequency"))
        .def("set_frequency", &RealtimeScheduler::set_frequency, py::arg("frequency"))
        .def("get_frequency", &RealtimeScheduler::get_frequency)
        .def("get_dt", &RealtimeScheduler::get_dt)
        .def("start", &RealtimeScheduler::start)
        .def("stop", &RealtimeScheduler::stop)
        .def("is_running", &RealtimeScheduler::is_running)
        .def("step", &RealtimeScheduler::step)
        .def("add_task", &RealtimeScheduler::add_task,
             py::arg("name"), py::arg("frequency"), py::arg("callback"))
        .def("get_current_time", &RealtimeScheduler::get_current_time);
}
