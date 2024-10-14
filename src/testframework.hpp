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
#include <type_traits>
#include <vector>

#include "gbasictypes.hpp"
#include "gexceptions.hpp"
#include "gtypetools.hpp"

#pragma once

namespace test {

/**
 * @brief Holds information about failed checks.
 */
struct FailedCheck {

    FailedCheck(Integer checkNr, const String &name, const String &message)
        : checkNumber{checkNr}, checkName{name}, failMessage{message} {}

    Integer checkNumber = 0;
    String checkName;
    String failMessage;
};

/**
 * @brief Holds the results of a test case.
 */
struct GTestResult {
    String testName;
    Integer numberExecutedChecks = 0;
    std::vector<FailedCheck> failedChecks;
    std::vector<GExceptionInfo> exceptions;
};

class GTestBase;

/**
 * @brief Implements a simple test framework where test cases can be registered
 * and executed. The results are sent to cout.
 */
class GTestFramework {
  public:
    /**
     * @brief Gives a reference to the singleton instance of the test framework
     *
     * @return GTestFramework&
     */
    constexpr static GTestFramework &getInstance() {
        static GTestFramework instance;
        return instance;
    }

    /**
     * @brief Registert a test case which will be executed when executeTests()
     * is in invoked.
     *
     * @param test
     */
    constexpr void registerTest(GTestBase &test) { tests_.push_back(&test); }

    /**
     * @brief Executes registered test cases.
     */
    void executeTests();

  private:
    GTestFramework() {}
    GTestFramework(const GTestFramework &) = delete;

    Integer numberOfExecutedChecks() const;
    Integer numberOfFailedChecks() const;
    auto getPassedTests() const;
    auto getFailedTests() const;
    auto getTestsWithExceptions() const;
    auto getNumberOfExecutedChecks() const;

    void printTestSummary() const;

    Integer numberOfExecutedTests_ = 0;
    Integer numberOfFailedTests_ = 0;
    std::vector<GTestBase *> tests_;
};

/**
 * @brief The base class for test cases.
 *
 */
class GTestBase {
  public:
    /**
     * @brief Construct a new GTestBase object and registers it in the test
     * framework.
     *
     * @param testName The name of the test case.
     * @param fw A reference to the test framework
     */
    GTestBase(const String &testName, GTestFramework &fw) : framework_{fw.getInstance()} {
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
    constexpr const String &getTestName() const { return testResult_.testName; }

    /**
     * @brief Get the results of the test case.
     */
    constexpr const GTestResult &getTestResult() const { return testResult_; }

  private:
    GTestBase() = delete;
    GTestBase(const GTestBase &) = delete;

    GTestFramework &framework_;
    GTestResult testResult_;

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
    template <typename Type> constexpr void GCHECK(const String &name, Type result, Type expected) {
        testResult_.numberExecutedChecks++;

        if (result != expected) {
            std::stringstream failMessage;
            failMessage << "Result: " << typeToString(result) << " | Expected: " << typeToString(expected);
            testResult_.failedChecks.emplace_back(
                FailedCheck{testResult_.numberExecutedChecks, name, failMessage.str()});
        }
    }

    template <typename Type> constexpr void GCHECK(Type result, Type expected) {
        GCHECK(String{""}, result, expected);
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
    class TestName##Test : public GTestBase {                                                                \
      public:                                                                                                \
        TestName##Test(const String &n, GTestFramework &fw) : GTestBase{n, fw} {}                            \
                                                                                                             \
        void testBody() override;                                                                            \
    };                                                                                                       \
                                                                                                             \
    static TestName##Test TestName##Instance(#TestName, GTestFramework::getInstance());                      \
                                                                                                             \
    void TestName##Test::testBody()

} // namespace test