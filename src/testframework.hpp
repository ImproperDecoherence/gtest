/**
 * @file testframework.hpp
 * @author ImproperDecoherence (gustowny@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-09-28
 *
 * Copyright (c) 2024-2024 Jonas Gustavsson
 *
 */

#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#pragma once

namespace gtest {

struct ExceptionInfo {
    explicit ExceptionInfo(const std::exception &e) : message{e.what()}, type{typeid(e).name()} {}

    std::string message;
    std::string type;
};

constexpr std::ostream &operator<<(std::ostream &os, const ExceptionInfo &gei) {
    os << gei.type << '(' << gei.message << ')';
    return os;
};

constexpr std::ostream &operator<<(std::ostream &os, const std::exception &exception) {
    os << exception.what();
    return os;
};

/**
 * @brief Holds information about failed checks.
 */
struct FailedCheck {

    FailedCheck(int checkNr, const std::string &name, const std::string &message)
        : checkNumber{checkNr}, checkName{name}, failMessage{message} {}

    int checkNumber = 0;
    std::string checkName;
    std::string failMessage;
};

/**
 * @brief Holds the results of a test case.
 */
struct TestResult {
    std::string testName;
    int numberExecutedChecks = 0;
    std::vector<FailedCheck> failedChecks;
    std::vector<ExceptionInfo> exceptions;
};

class TestBase;

/**
 * @brief Implements a simple test framework where test cases can be registered
 * and executed. The results are sent to cout.
 */
class TestFramework {
  public:
    /**
     * @brief Gives a reference to the singleton instance of the test framework
     *
     * @return TestFramework&
     */
    constexpr static TestFramework &getInstance() {
        static TestFramework instance;
        return instance;
    }

    /**
     * @brief Registert a test case which will be executed when executeTests()
     * is in invoked.
     *
     * @param test
     */
    constexpr void registerTest(TestBase &test) { tests_.push_back(&test); }

    /**
     * @brief Executes registered test cases.
     */
    void executeTests();

  private:
    TestFramework() {}
    TestFramework(const TestFramework &) = delete;

    int numberOfExecutedChecks() const;
    int numberOfFailedChecks() const;
    auto getPassedTests() const;
    auto getFailedTests() const;
    auto getTestsWithExceptions() const;
    auto getNumberOfExecutedChecks() const;

    void printTestSummary() const;

    int numberOfExecutedTests_ = 0;
    int numberOfFailedTests_ = 0;
    std::vector<TestBase *> tests_;
};

/**
 * @brief The base class for test cases.
 *
 */
class TestBase {
  public:
    /**
     * @brief Construct a new TestBase object and registers it in the test
     * framework.
     *
     * @param testName The name of the test case.
     * @param fw A reference to the test framework
     */
    TestBase(const std::string &testName, TestFramework &fw) : framework_{fw.getInstance()} {
        framework_.registerTest(*this);
        testResult_.testName = testName;
    }

    /**
     * @brief This method will be called when its time to execute the test case.
     */
    void execute();

    /**
     * @brief This method must be overloaded by the test case to define what the
     * test shall do.
     */
    virtual void testBody() = 0;

    /**
     * @brief Get the name of the test case.
     */
    constexpr const std::string &getTestName() const { return testResult_.testName; }

    /**
     * @brief Get the results of the test case.
     */
    constexpr const TestResult &getTestResult() const { return testResult_; }

  private:
    TestBase() = delete;
    TestBase(const TestBase &) = delete;

    TestFramework &framework_;
    TestResult testResult_;

  protected:
    /**
     * @brief Performs a check to see that the given parameters are equal.
     *
     * The check result will be available via getTestResults().
     *
     * @tparam Type Any type that has the != opertor.
     * @param result The result from the test.
     * @param expected The expected result of the test.
     */
    template <typename Type> constexpr void GCHECK(const std::string &name, Type result, Type expected) {
        testResult_.numberExecutedChecks++;

        if (result != expected) {
            std::stringstream failMessage;
            failMessage << std::boolalpha << "Result: " << result << " | Expected: " << expected;
            testResult_.failedChecks.emplace_back(
                FailedCheck{testResult_.numberExecutedChecks, name, failMessage.str()});
        }
    }

    template <typename Type> constexpr void GCHECK(Type result, Type expected) {
        GCHECK(std::string{""}, result, expected);
    }
};

/**
 * @def GTEST(TestName)
 * @brief This macro creates a test case instance. The body of the test is
 * provided within a scope directly after.
 *
 * Example usage:
 * @code
 * #include "testframework.hpp"
 *
 * namespace test {
 *
 * GTEST(ExampleTest) {
 *     constexpr auto i1{2};
 *     constexpr auto i2{3};
 *
 *     auto addition = i1 + i2;
 *     auto expectedResult{5};
 *
 *     GCHECK(addition, expectedResult);
 * }
 *
 * } // namespace test
 * @endcode
 */
#define GTEST(TestName)                                                                                      \
    class TestName##Test : public gtest::TestBase {                                                          \
      public:                                                                                                \
        TestName##Test(const std::string &n, gtest::TestFramework &fw) : TestBase{n, fw} {}                  \
                                                                                                             \
        void testBody() override;                                                                            \
    };                                                                                                       \
                                                                                                             \
    static TestName##Test TestName##Instance(#TestName, gtest::TestFramework::getInstance());                \
                                                                                                             \
    void TestName##Test::testBody()

} // namespace gtest