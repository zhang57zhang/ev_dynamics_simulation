/**
 * @file pybind11_template.cpp
 * @brief Python绑定模板 - 供其他Agent参考
 * 
 * 这个文件展示了如何使用pybind11创建C++类的Python绑定。
 * 其他Agent在实现各自模块时，可以参考这个模板。
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // 支持STL容器
#include <pybind11/eigen.h> // 支持Eigen矩阵
#include <pybind11/functional.h> // 支持std::function

namespace py = pybind11;

// ============================================================================
// 示例1：绑定一个简单的类
// ============================================================================
class Example {
public:
    Example(int value) : value_(value) {}
    
    int get_value() const { return value_; }
    void set_value(int value) { value_ = value; }
    
    void increment() { value_++; }
    
private:
    int value_;
};

// ============================================================================
// 示例2：带复杂参数的类
// ============================================================================
class ComplexExample {
public:
    ComplexExample(double x, double y) : x_(x), y_(y) {}
    
    // 返回值
    double get_x() const { return x_; }
    double get_y() const { return y_; }
    
    // 修改成员
    void set_x(double x) { x_ = x; }
    void set_y(double y) { y_ = y; }
    
    // 计算方法
    double magnitude() const {
        return std::sqrt(x_ * x_ + y_ * y_);
    }
    
    // 静态方法
    static ComplexExample create_unit() {
        return ComplexExample(1.0, 1.0);
    }
    
    // 接受STL容器
    void set_values(const std::vector<double>& values) {
        if (values.size() >= 2) {
            x_ = values[0];
            y_ = values[1];
        }
    }
    
    std::vector<double> get_values() const {
        return {x_, y_};
    }
    
private:
    double x_;
    double y_;
};

// ============================================================================
// 示例3：继承和多态
// ============================================================================
class Base {
public:
    virtual ~Base() = default;
    virtual std::string name() const { return "Base"; }
};

class Derived : public Base {
public:
    std::string name() const override { return "Derived"; }
};

// ============================================================================
// 示例4：枚举类型
// ============================================================================
enum class VehicleType {
    Sedan,
    SUV,
    Truck,
    SportsCar
};

// ============================================================================
// 示例5：结构体
// ============================================================================
struct VehicleConfig {
    double mass;
    double length;
    double width;
    VehicleType type;
    
    VehicleConfig(double m, double l, double w, VehicleType t)
        : mass(m), length(l), width(w), type(t) {}
};

// ============================================================================
// 示例6：回调函数
// ============================================================================
class CallbackExample {
public:
    void set_callback(std::function<void(double)> callback) {
        callback_ = callback;
    }
    
    void execute_callback(double value) {
        if (callback_) {
            callback_(value);
        }
    }
    
private:
    std::function<void(double)> callback_;
};

// ============================================================================
// Python模块定义
// ============================================================================
PYBIND11_MODULE(example, m) {
    m.doc() = "Example module demonstrating pybind11 features";
    
    // 示例1：简单类
    py::class_<Example>(m, "Example")
        .def(py::init<int>(), py::arg("value"))
        .def("get_value", &Example::get_value)
        .def("set_value", &Example::set_value, py::arg("value"))
        .def("increment", &Example::increment)
        .def_property("value", &Example::get_value, &Example::set_value);
    
    // 示例2：复杂类
    py::class_<ComplexExample>(m, "ComplexExample")
        .def(py::init<double, double>(), py::arg("x"), py::arg("y"))
        .def("get_x", &ComplexExample::get_x)
        .def("get_y", &ComplexExample::get_y)
        .def("set_x", &ComplexExample::set_x, py::arg("x"))
        .def("set_y", &ComplexExample::set_y, py::arg("y"))
        .def("magnitude", &ComplexExample::magnitude)
        .def_static("create_unit", &ComplexExample::create_unit)
        .def("set_values", &ComplexExample::set_values, py::arg("values"))
        .def("get_values", &ComplexExample::get_values);
    
    // 示例3：继承
    py::class_<Base, std::shared_ptr<Base>>(m, "Base")
        .def("name", &Base::name);
    
    py::class_<Derived, Base, std::shared_ptr<Derived>>(m, "Derived")
        .def(py::init<>());
    
    // 示例4：枚举
    py::enum_<VehicleType>(m, "VehicleType")
        .value("Sedan", VehicleType::Sedan)
        .value("SUV", VehicleType::SUV)
        .value("Truck", VehicleType::Truck)
        .value("SportsCar", VehicleType::SportsCar)
        .export_values();
    
    // 示例5：结构体
    py::class_<VehicleConfig>(m, "VehicleConfig")
        .def(py::init<double, double, double, VehicleType>(),
             py::arg("mass"), py::arg("length"), py::arg("width"), py::arg("type"))
        .def_readwrite("mass", &VehicleConfig::mass)
        .def_readwrite("length", &VehicleConfig::length)
        .def_readwrite("width", &VehicleConfig::width)
        .def_readwrite("type", &VehicleConfig::type);
    
    // 示例6：回调
    py::class_<CallbackExample>(m, "CallbackExample")
        .def(py::init<>())
        .def("set_callback", &CallbackExample::set_callback)
        .def("execute_callback", &CallbackExample::execute_callback, py::arg("value"));
    
    // 模块级别的常量
    m.attr("VERSION") = "1.0.0";
    m.attr("PI") = 3.141592653589793;
    
    // 模块级别的函数
    m.def("add", [](int a, int b) { return a + b; }, 
          py::arg("a"), py::arg("b"), "Add two integers");
    
    m.def("multiply", [](double a, double b) { return a * b; },
          py::arg("a"), py::arg("b"), "Multiply two doubles");
}
