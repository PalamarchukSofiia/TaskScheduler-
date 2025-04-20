#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <stdexcept>

struct BaseTask {
    virtual void cute() = 0;
    virtual bool Icute() const = 0;
    virtual ~BaseTask() = default;
};

template<typename T>
struct TaskWithRes {
    virtual T* getres_ptr() = 0;
    virtual ~TaskWithRes() = default;
};

template<typename T>
class Future {
private:
    std::shared_ptr<TaskWithRes<T>> tasochka;
public:
    Future(std::shared_ptr<TaskWithRes<T>> t) : tasochka(std::move(t)) {}

    T get() {
        auto task = std::dynamic_pointer_cast<BaseTask>(tasochka);
        if (!task->Icute()) task->cute();
        return *tasochka->getres_ptr();
    }
};


template<typename T>
class TaskWithoutARG : public BaseTask, public TaskWithRes<T> {
private:
    T(*funk)();
    std::unique_ptr<T> res;
public:
    TaskWithoutARG(T(*f)()) : funk(f) {}

    void cute() override {
        if (!res) res = std::make_unique<T>(funk());
    }

    bool Icute() const override { return res != nullptr; }
    T* getres_ptr() override { return res.get(); }
};

template<typename T, typename ARG>
class TaskWith1ARG : public BaseTask, public TaskWithRes<T> {
private:
    T(*funk)(ARG);
    bool IF;
    ARG value;
    std::unique_ptr<Future<ARG>> future_value;
    std::unique_ptr<T> res;
public:
    TaskWith1ARG(T(*f)(ARG), ARG v) : funk(f), IF(false), value(v) {}
    TaskWith1ARG(T(*f)(ARG), Future<ARG> v)
        : funk(f), IF(true), future_value(std::make_unique<Future<ARG>>(v)) {}

    void cute() override {
        if (!res) {
            ARG arg = IF ? future_value->get() : value;
            res = std::make_unique<T>(funk(arg));
        }
    }

    bool Icute() const override { return res != nullptr; }
    T* getres_ptr() override { return res.get(); }
};

template<typename T, typename ARG1, typename ARG2>
class TaskWith2ARG : public BaseTask, public TaskWithRes<T> {
private:
    T(*funk)(ARG1, ARG2);
    bool IF1, IF2;
    ARG1 value1;
    ARG2 value2;
    std::unique_ptr<Future<ARG1>> future_value1;
    std::unique_ptr<Future<ARG2>> future_value2;
    std::unique_ptr<T> res;
public:
    TaskWith2ARG(T(*f)(ARG1, ARG2), ARG1 a1, ARG2 a2)
        : funk(f), IF1(false), IF2(false), value1(a1), value2(a2) {}

    TaskWith2ARG(T(*f)(ARG1, ARG2), ARG1 a1, Future<ARG2> fut2)
        : funk(f), IF1(false), IF2(true), value1(a1),
          future_value2(std::make_unique<Future<ARG2>>(fut2)) {}

    TaskWith2ARG(T(*f)(ARG1, ARG2), Future<ARG1> fut1, ARG2 a2)
        : funk(f), IF1(true), IF2(false), value2(a2),
          future_value1(std::make_unique<Future<ARG1>>(fut1)) {}

    TaskWith2ARG(T(*f)(ARG1, ARG2), Future<ARG1> fut1, Future<ARG2> fut2)
        : funk(f), IF1(true), IF2(true),
          future_value1(std::make_unique<Future<ARG1>>(fut1)),
          future_value2(std::make_unique<Future<ARG2>>(fut2)) {}

    void cute() override {
        if (!res) {
            ARG1 arg1 = IF1 ? future_value1->get() : value1;
            ARG2 arg2 = IF2 ? future_value2->get() : value2;
            res = std::make_unique<T>(funk(arg1, arg2));
        }
    }

    bool Icute() const override { return res != nullptr; }
    T* getres_ptr() override { return res.get(); }
};


template<typename T, typename C>
class MethodTask0 : public BaseTask, public TaskWithRes<T> {
private:
    T(C::*method)() const;
    C object;
    std::unique_ptr<T> res;
public:
    MethodTask0(T(C::*m)() const, C obj) : method(m), object(obj) {}
    void cute() override {
        if (!res) res = std::make_unique<T>((object.*method)());
    }
    bool Icute() const override { return res != nullptr; }
    T* getres_ptr() override { return res.get(); }
};

template<typename T, typename C, typename ARG>
class MethodTask1 : public BaseTask, public TaskWithRes<T> {
private:
    T(C::*method)(ARG) const;
    C object;
    bool IF;
    ARG value;
    std::unique_ptr<Future<ARG>> future_value;
    std::unique_ptr<T> res;

public:
    MethodTask1(T(C::*m)(ARG) const, C obj, ARG val)
        : method(m), object(obj), IF(false), value(val) {}

    MethodTask1(T(C::*m)(ARG) const, C obj, Future<ARG> fut)
        : method(m), object(obj), IF(true),
          future_value(std::make_unique<Future<ARG>>(fut)) {}

    void cute() override {
        if (!res) {
            ARG arg = IF ? future_value->get() : value;
            res = std::make_unique<T>((object.*method)(arg));
        }
    }

    bool Icute() const override { return res != nullptr; }
    T* getres_ptr() override { return res.get(); }
};

template<typename T, typename C, typename ARG1, typename ARG2>
class MethodTask2 : public BaseTask, public TaskWithRes<T> {
private:
    T(C::*method)(ARG1, ARG2) const;
    C object;
    bool IF1, IF2;
    ARG1 val1;
    ARG2 val2;
    std::unique_ptr<Future<ARG1>> fut1;
    std::unique_ptr<Future<ARG2>> fut2;
    std::unique_ptr<T> res;

public:
    MethodTask2(T(C::*m)(ARG1, ARG2) const, C obj, ARG1 a1, ARG2 a2)
        : method(m), object(obj), IF1(false), IF2(false), val1(a1), val2(a2) {}

    MethodTask2(T(C::*m)(ARG1, ARG2) const, C obj, Future<ARG1> f1, ARG2 a2)
        : method(m), object(obj), IF1(true), IF2(false), val2(a2),
          fut1(std::make_unique<Future<ARG1>>(f1)) {}

    MethodTask2(T(C::*m)(ARG1, ARG2) const, C obj, ARG1 a1, Future<ARG2> f2)
        : method(m), object(obj), IF1(false), IF2(true), val1(a1),
          fut2(std::make_unique<Future<ARG2>>(f2)) {}

    MethodTask2(T(C::*m)(ARG1, ARG2) const, C obj, Future<ARG1> f1, Future<ARG2> f2)
        : method(m), object(obj), IF1(true), IF2(true),
          fut1(std::make_unique<Future<ARG1>>(f1)),
          fut2(std::make_unique<Future<ARG2>>(f2)) {}

    void cute() override {
        if (!res) {
            ARG1 a1 = IF1 ? fut1->get() : val1;
            ARG2 a2 = IF2 ? fut2->get() : val2;
            res = std::make_unique<T>((object.*method)(a1, a2));
        }
    }

    bool Icute() const override { return res != nullptr; }
    T* getres_ptr() override { return res.get(); }
};


template<typename T, typename F>
class LambdaTask0 : public BaseTask, public TaskWithRes<T> {
private:
    F callable;
    std::unique_ptr<T> res;
public:
    LambdaTask0(F f) : callable(std::move(f)) {}
    void cute() override {
        if (!res) res = std::make_unique<T>(callable());
    }
    bool Icute() const override { return res != nullptr; }
    T* getres_ptr() override { return res.get(); }
};

template<typename T, typename F, typename A>
class LambdaTask1 : public BaseTask, public TaskWithRes<T> {
private:
    F callable;
    bool IF;
    A value;
    std::unique_ptr<Future<A>> future_value;
    std::unique_ptr<T> res;
public:
    LambdaTask1(F f, A val) : callable(f), IF(false), value(val) {}
    LambdaTask1(F f, Future<A> fut)
        : callable(f), IF(true), future_value(std::make_unique<Future<A>>(fut)) {}
    void cute() override {
        if (!res) {
            A arg = IF ? future_value->get() : value;
            res = std::make_unique<T>(callable(arg));
        }
    }
    bool Icute() const override { return res != nullptr; }
    T* getres_ptr() override { return res.get(); }
};

template<typename T, typename F, typename ARG1, typename ARG2>
class LambdaTask2 : public BaseTask, public TaskWithRes<T> {
private:
    F callable;
    bool IF1, IF2;
    ARG1 val1;
    ARG2 val2;
    std::unique_ptr<Future<ARG1>> fut1;
    std::unique_ptr<Future<ARG2>> fut2;
    std::unique_ptr<T> res;
public:
    LambdaTask2(F f, ARG1 a1, ARG2 a2)
        : callable(f), IF1(false), IF2(false), val1(a1), val2(a2) {}

    LambdaTask2(F f, Future<ARG1> a1, ARG2 a2)
        : callable(f), IF1(true), IF2(false), val2(a2),
        fut1(std::make_unique<Future<ARG1>>(a1)) {}

    LambdaTask2(F f, ARG1 a1, Future<ARG2> a2)
        : callable(f), IF1(false), IF2(true), val1(a1),
        fut2(std::make_unique<Future<ARG2>>(a2)) {}

    LambdaTask2(F f, Future<ARG1> a1, Future<ARG2> a2)
        : callable(f), IF1(true), IF2(true),
        fut1(std::make_unique<Future<ARG1>>(a1)),
        fut2(std::make_unique<Future<ARG2>>(a2)) {}

    void cute() override {
        if (!res) {
            ARG1 a1 = IF1 ? fut1->get() : val1;
            ARG2 a2 = IF2 ? fut2->get() : val2;
            res = std::make_unique<T>(callable(a1, a2));
        }
    }
    bool Icute() const override { return res != nullptr; }
    T* getres_ptr() override { return res.get(); }
};


using ID = std::shared_ptr<BaseTask>;

class TTaskScheduler {
private:
    std::vector<ID> tasochki;
public:

//------------------------------------------------ нет аргументов ---------------------------------------------------------------------------------

    template<typename T>
    ID add(T(*func)()) {
        auto task = std::make_shared<TaskWithoutARG<T>>(func);
        ID base = task;
        tasochki.push_back(base);
        return base;
    }

//------------------------------------------------ один аргумент ---------------------------------------------------------------------------------

    template<typename T, typename ARG>
    ID add(T(*func)(ARG), ARG arg) {
        auto task = std::make_shared<TaskWith1ARG<T, ARG>>(func, arg);
        ID base = task;
        tasochki.push_back(base);
        return base;
    }

    template<typename T, typename ARG>
    ID add(T(*func)(ARG), Future<ARG> arg) {
        auto task = std::make_shared<TaskWith1ARG<T, ARG>>(func, arg);
        ID base = task;
        tasochki.push_back(base);
        return base;
    }

//------------------------------------------------ два аргумента ---------------------------------------------------------------------------------

    template<typename T, typename ARG1, typename ARG2>
    ID add(T(*func)(ARG1, ARG2), ARG1 arg1, ARG2 arg2) {
        auto task = std::make_shared<TaskWith2ARG<T, ARG1, ARG2>>(func, arg1, arg2);
        ID base = task;
        tasochki.push_back(base);
        return base;
    }

    template<typename T, typename ARG1, typename ARG2>
    ID add(T(*func)(ARG1, ARG2), Future<ARG1> arg1, ARG2 arg2) {
        auto task = std::make_shared<TaskWith2ARG<T, ARG1, ARG2>>(func, arg1, arg2);
        ID base = task;
        tasochki.push_back(base);
        return base;
    }

    template<typename T, typename ARG1, typename ARG2>
    ID add(T(*func)(ARG1, ARG2), ARG1 arg1, Future<ARG2> arg2) {
        auto task = std::make_shared<TaskWith2ARG<T, ARG1, ARG2>>(func, arg1, arg2);
        ID base = task;
        tasochki.push_back(base);
        return base;
    }

    template<typename T, typename ARG1, typename ARG2>
    ID add(T(*func)(ARG1, ARG2), Future<ARG1> arg1, Future<ARG2> arg2) {
        auto task = std::make_shared<TaskWith2ARG<T, ARG1, ARG2>>(func, arg1, arg2);
        ID base = task;
        tasochki.push_back(base);
        return base;
    }


//------------------------------------------------ для метода класса---------------------------------------------------------------------------------

    template<typename T, typename C>
    ID add(T(C::*method)() const, C obj) {
        auto task = std::make_shared<MethodTask0<T, C>>(method, obj);
        tasochki.push_back(task);
        return task;
    }



    template<typename T, typename C, typename ARG>
    ID add(T(C::*method)(ARG) const, C obj, ARG val) {
        auto task = std::make_shared<MethodTask1<T, C, ARG>>(method, obj, val);
        tasochki.push_back(task);
        return task;
    }

    template<typename T, typename C, typename ARG>
    ID add(T(C::*method)(ARG) const, C obj, Future<ARG> fut) {
        auto task = std::make_shared<MethodTask1<T, C, ARG>>(method, obj, fut);
        tasochki.push_back(task);
        return task;
    }



    template<typename T, typename C, typename ARG1, typename ARG2>
    ID add(T(C::*method)(ARG1, ARG2) const, C obj, ARG1 a1, ARG2 a2) {
        auto task = std::make_shared<MethodTask2<T, C, ARG1, ARG2>>(method, obj, a1, a2);
        tasochki.push_back(task);
        return task;
    }

    template<typename T, typename C, typename ARG1, typename ARG2>
    ID add(T(C::*method)(ARG1, ARG2) const, C obj, Future<ARG1> a1, ARG2 a2) {
        auto task = std::make_shared<MethodTask2<T, C, ARG1, ARG2>>(method, obj, a1, a2);
        tasochki.push_back(task);
        return task;
    }

    template<typename T, typename C, typename ARG1, typename ARG2>
    ID add(T(C::*method)(ARG1, ARG2) const, C obj, ARG1 a1, Future<ARG2> a2) {
        auto task = std::make_shared<MethodTask2<T, C, ARG1, ARG2>>(method, obj, a1, a2);
        tasochki.push_back(task);
        return task;
    }

    template<typename T, typename C, typename ARG1, typename ARG2>
    ID add(T(C::*method)(ARG1, ARG2) const, C obj, Future<ARG1> a1, Future<ARG2> a2) {
        auto task = std::make_shared<MethodTask2<T, C, ARG1, ARG2>>(method, obj, a1, a2);
        tasochki.push_back(task);
        return task;
    }

//------------------------------------------------------ для лямбд ---------------------------------------------------------------------------------

    template<typename F, typename T = decltype(std::declval<F>()())>
    ID add(F f) {
        auto task = std::make_shared<LambdaTask0<T, F>>(f);
        tasochki.push_back(task);
        return task;
    }


    template<typename F, typename A, typename T = decltype(std::declval<F>()(std::declval<A>()))>
    ID add(F f, A val) {
        auto task = std::make_shared<LambdaTask1<T, F, A>>(f, val);
        tasochki.push_back(task);
        return task;
    }

    template<typename F, typename A, typename T = decltype(std::declval<F>()(std::declval<A>()))>
    ID add(F f, Future<A> fut) {
        auto task = std::make_shared<LambdaTask1<T, F, A>>(f, fut);
        tasochki.push_back(task);
        return task;
    }


    template<typename F, typename ARG1, typename ARG2, typename T = decltype(std::declval<F>()(std::declval<ARG1>(), std::declval<ARG2>()))>
    ID add(F f, ARG1 a1, ARG2 a2) {
        auto task = std::make_shared<LambdaTask2<T, F, ARG1, ARG2>>(f, a1, a2);
        tasochki.push_back(task);
        return task;
    }

    template<typename F, typename ARG1, typename ARG2, typename T = decltype(std::declval<F>()(std::declval<ARG1>(), std::declval<ARG2>()))>
    ID add(F f, Future<ARG1> a1, ARG2 a2) {
        auto task = std::make_shared<LambdaTask2<T, F, ARG1, ARG2>>(f, a1, a2);
        tasochki.push_back(task);
        return task;
    }

    template<typename F, typename ARG1, typename ARG2, typename T = decltype(std::declval<F>()(std::declval<ARG1>(), std::declval<ARG2>()))>
    ID add(F f, ARG1 a1, Future<ARG2> a2) {
        auto task = std::make_shared<LambdaTask2<T, F, ARG1, ARG2>>(f, a1, a2);
        tasochki.push_back(task);
        return task;
    }

    template<typename F, typename ARG1, typename ARG2, typename T = decltype(std::declval<F>()(std::declval<ARG1>(), std::declval<ARG2>()))>
    ID add(F f, Future<ARG1> a1, Future<ARG2> a2) {
        auto task = std::make_shared<LambdaTask2<T, F, ARG1, ARG2>>(f, a1, a2);
        tasochki.push_back(task);
        return task;
    }


//--------------------------------------------------------------------------------------------------------------------------------------------------
    template<typename T>
    Future<T> getFutureResult(ID id) {
        auto casted = std::dynamic_pointer_cast<TaskWithRes<T>>(id);
        if (!casted) throw std::runtime_error("Invalid task result type");
        return Future<T>(casted);
    }

    template<typename T>
    T getResult(ID id) {
        return getFutureResult<T>(id).get();
    }

    void executeAll() {
        for (auto& t : tasochki)
            if (!t->Icute()) t->cute();
    }
};
