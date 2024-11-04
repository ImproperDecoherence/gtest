#include <algorithm>
#include <exception>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ranges>
#include <vector>

#include "g_test_framework.hpp"

using namespace std;

namespace gtest {

namespace PrintColor {
constexpr string Black{"\033[30m"};
constexpr string Red{"\033[31m"};
constexpr string Green{"\033[32m"};
constexpr string Yellow{"\033[33m"};
constexpr string Blue{"\033[34m"};
constexpr string Magenta{"\033[35m"};
constexpr string Cyan{"\033[36m"};
constexpr string White{"\033[37m"};

constexpr string Reset{"\033[0m"};
} // namespace PrintColor

template <typename Type> void printRowColumn(int width, const string &color, const Type &element) {
    cout << color;
    cout << std::setw(width) << element;
    cout << PrintColor::Reset;
}

template <typename... Args>
void printTableRow(const std::vector<int> &columnWidths, const std::vector<string> &colors, Args... args) {
    ios_base::fmtflags savedCoutFlags(cout.flags());

    if (sizeof...(args) != columnWidths.size() || columnWidths.size() != colors.size()) {
        throw std::invalid_argument("Number of widths and colors must match the number of arguments!");
    }

    int columnIndex{0};
    int colorIndex{0};
    cout << right;
    (printRowColumn(columnWidths[columnIndex++], colors[colorIndex++], args), ...);
    cout << endl;

    cout.flags(savedCoutFlags); // restore cout formatting
}

const vector<int> testResultsTableColumnWidths{4, 30, 10, 10, 15};
const auto defaultTableColumnColors = vector<string>(testResultsTableColumnWidths.size(), PrintColor::Reset);

void printTestResultTableHeader() {
    printTableRow(testResultsTableColumnWidths, defaultTableColumnColors, "#", "Test Name", "Checks",
                  "Failed", "Status");
}

void printTestResultTableRow(int testNo, const TestResult &result) {
    string status;
    vector<string> colors{defaultTableColumnColors};
    constexpr auto resultColumn{4};

    if (!result.exceptions.empty()) {
        status = "EXCEPTION";
        colors[resultColumn] = PrintColor::Magenta;
    } else if (!result.failedChecks.empty()) {
        status = "FAILED";
        colors[resultColumn] = PrintColor::Red;
    } else if (result.numberExecutedChecks > 0) {
        status = "PASSED";
        colors[resultColumn] = PrintColor::Green;
    } else {
        status = "NOT PERFORMED";
    }

    printTableRow(testResultsTableColumnWidths, colors, testNo, result.testName, result.numberExecutedChecks,
                  result.failedChecks.size(), status);
}

int TestFramework::numberOfExecutedChecks() const {
    auto accExecutedChecks = [](int sum, const TestBase *test) {
        return sum + test->getTestResult().numberExecutedChecks;
    };

    return accumulate(tests_.begin(), tests_.end(), 0, accExecutedChecks);
}

int TestFramework::numberOfFailedChecks() const {
    auto accExecutedChecks = [](int sum, const TestBase *test) {
        return sum + test->getTestResult().failedChecks.size();
    };

    return accumulate(tests_.begin(), tests_.end(), 0, accExecutedChecks);
}

auto TestFramework::getPassedTests() const {
    auto passedTestFilter = [](const TestBase *test) {
        const TestResult &result = test->getTestResult();
        return (result.failedChecks.empty()) && (result.numberExecutedChecks > 0);
    };

    return tests_ | ranges::views::filter(passedTestFilter);
}

auto TestFramework::getFailedTests() const {
    auto failedTestFilter = [](const TestBase *test) {
        return test->getTestResult().failedChecks.size() > 0;
    };

    return tests_ | ranges::views::filter(failedTestFilter);
}

auto TestFramework::getTestsWithExceptions() const {
    auto exceptionFilter = [](const TestBase *test) { return test->getTestResult().exceptions.size() > 0; };

    return tests_ | ranges::views::filter(exceptionFilter);
}

auto TestFramework::getNumberOfExecutedChecks() const {
    auto accFunc = [](auto sum, const TestBase *test) {
        return sum + test->getTestResult().numberExecutedChecks;
    };

    return accumulate(tests_.begin(), tests_.end(), 0, accFunc);
}

void TestFramework::printTestSummary() const {
    const vector<int> testSummaryTableColumnWidths{20, 20, 20, 20};

    auto passedTests = getPassedTests();
    auto failedTests = getFailedTests();
    auto testsWithExceptions = getTestsWithExceptions();
    const auto noPassedTests = ranges::distance(passedTests);
    const auto noFailedTests = ranges::distance(failedTests);
    const auto noTestsWithExceptions = ranges::distance(testsWithExceptions);
    const auto noExecutedChecks = getNumberOfExecutedChecks();

    const string result{numberOfFailedChecks() == 0 ? "SUCCESS!" : "FAILED"};
    const string resultColor{numberOfFailedChecks() == 0 ? PrintColor::Green : PrintColor::Red};

    cout << endl;
    cout << "TEST SUMMARY: " << resultColor << result << PrintColor::Reset << endl;
    cout << "  " << noExecutedChecks << " checks executed for " << tests_.size() << " test cases." << endl;
    if (noFailedTests > 0) {
        cout << "  " << noPassedTests << " passed tests " << noFailedTests << " failed tests." << endl;
    }
    if (noTestsWithExceptions > 0) {
        cout << "  " << noTestsWithExceptions << " tests was terminated with an exception." << endl;
    }
    cout << endl;

    for (auto *test : failedTests) {
        const TestResult &result = test->getTestResult();

        for (auto check : result.failedChecks) {
            cout << "# Failed: " << test->getTestName() << " check " << check.checkNumber << " ("
                 << check.checkName << ") | " << check.failMessage << endl;
        }
    }

    for (auto *test : testsWithExceptions) {
        const TestResult &result = test->getTestResult();

        for (auto except : result.exceptions) {
            cout << "# Exception: " << test->getTestName() << except << endl;
        }
    }

    cout << endl << endl;
}

void TestFramework::executeTests() {
    printTestResultTableHeader();

    for (auto test : tests_) {
        test->execute();
        ++numberOfExecutedTests_;
        printTestResultTableRow(numberOfExecutedTests_, test->getTestResult());
    }

    printTestSummary();
}

void TestBase::execute() {
    try {
        testBody();
    } catch (const std::exception &exception) {
        testResult_.exceptions.emplace_back(ExceptionInfo{exception});
    }
}

} // namespace gtest