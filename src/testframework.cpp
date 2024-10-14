#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ranges>
#include <vector>

#include "gexceptions.hpp"
#include "gprinttools.hpp"
#include "testframework.hpp"

using namespace std;

namespace test {

const vector<Integer> testResultsTableColumnWidths{4, 20, 10, 10, 15};
const auto defaultTableColumnColors = vector<String>(testResultsTableColumnWidths.size(), CoutColor::Reset);

void printTestResultTableHeader() {
    printTableRow(testResultsTableColumnWidths, defaultTableColumnColors, "#", "Test Name", "Checks",
                  "Failed", "Status");
}

void printTestResultTableRow(Integer testNo, const GTestResult &result) {
    String status;
    vector<String> colors{defaultTableColumnColors};
    constexpr auto resultColumn{4};

    if (!result.exceptions.empty()) {
        status = "EXCEPTION";
        colors[resultColumn] = CoutColor::Magenta;
    } else if (!result.failedChecks.empty()) {
        status = "FAILED";
        colors[resultColumn] = CoutColor::Red;
    } else if (result.numberExecutedChecks > 0) {
        status = "PASSED";
        colors[resultColumn] = CoutColor::Green;
    } else {
        status = "NOT PERFORMED";
    }

    printTableRow(testResultsTableColumnWidths, colors, testNo, result.testName, result.numberExecutedChecks,
                  result.failedChecks.size(), status);
}

Integer GTestFramework::numberOfExecutedChecks() const {
    auto accExecutedChecks = [](Integer sum, const GTestBase *test) {
        return sum + test->getTestResult().numberExecutedChecks;
    };

    return accumulate(tests_.begin(), tests_.end(), 0, accExecutedChecks);
}

Integer GTestFramework::numberOfFailedChecks() const {
    auto accExecutedChecks = [](Integer sum, const GTestBase *test) {
        return sum + test->getTestResult().failedChecks.size();
    };

    return accumulate(tests_.begin(), tests_.end(), 0, accExecutedChecks);
}

auto GTestFramework::getPassedTests() const {
    auto passedTestFilter = [](const GTestBase *test) {
        const GTestResult &result = test->getTestResult();
        return (result.failedChecks.empty()) && (result.numberExecutedChecks > 0);
    };

    return tests_ | ranges::views::filter(passedTestFilter);
}

auto GTestFramework::getFailedTests() const {
    auto failedTestFilter = [](const GTestBase *test) {
        return test->getTestResult().failedChecks.size() > 0;
    };

    return tests_ | ranges::views::filter(failedTestFilter);
}

auto GTestFramework::getTestsWithExceptions() const {
    auto exceptionFilter = [](const GTestBase *test) { return test->getTestResult().exceptions.size() > 0; };

    return tests_ | ranges::views::filter(exceptionFilter);
}

auto GTestFramework::getNumberOfExecutedChecks() const {
    auto accFunc = [](auto sum, const GTestBase *test) {
        return sum + test->getTestResult().numberExecutedChecks;
    };

    return accumulate(tests_.begin(), tests_.end(), 0, accFunc);
}

void GTestFramework::printTestSummary() const {
    const vector<Integer> testSummaryTableColumnWidths{20, 20, 20, 20};

    auto passedTests = getPassedTests();
    auto failedTests = getFailedTests();
    auto testsWithExceptions = getTestsWithExceptions();
    const auto noPassedTests = ranges::distance(passedTests);
    const auto noFailedTests = ranges::distance(failedTests);
    const auto noTestsWithExceptions = ranges::distance(testsWithExceptions);
    const auto noExecutedChecks = getNumberOfExecutedChecks();

    const String result{numberOfFailedChecks() == 0 ? "SUCCESS!" : "FAILED"};
    const String resultColor{numberOfFailedChecks() == 0 ? CoutColor::Green : CoutColor::Red};

    cout << endl;
    cout << "TEST SUMMARY: " << resultColor << result << CoutColor::Reset << endl;
    cout << "  " << noExecutedChecks << " checks executed for " << tests_.size() << " test cases." << endl;
    if (noFailedTests > 0) {
        cout << "  " << noPassedTests << " passed tests " << noFailedTests << " failed tests." << endl;
    }
    if (noTestsWithExceptions > 0) {
        cout << "  " << noTestsWithExceptions << " tests was terminated with an exception." << endl;
    }
    cout << endl;

    for (auto *test : failedTests) {
        const GTestResult &result = test->getTestResult();

        for (auto check : result.failedChecks) {
            cout << "# Failed: " << test->getTestName() << " check " << check.checkNumber << " ("
                 << check.checkName << ") | " << check.failMessage << endl;
        }
    }

    for (auto *test : testsWithExceptions) {
        const GTestResult &result = test->getTestResult();

        for (auto except : result.exceptions) {
            cout << "# Exception: " << test->getTestName() << except << endl;
        }
    }

    cout << endl << endl;
}

void GTestFramework::executeTests() {
    printTestResultTableHeader();

    for (auto test : tests_) {
        test->execute();
        ++numberOfExecutedTests_;
        printTestResultTableRow(numberOfExecutedTests_, test->getTestResult());
    }

    printTestSummary();
}

void GTestBase::execute() {
    try {
        testBody();
    } catch (const GException &exception) {
        testResult_.exceptions.emplace_back(GExceptionInfo{exception});
    }
}

} // namespace test