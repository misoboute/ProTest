//    MIT License
//
//    Copyright (c) 2017 Muhammad Ismail Soboute
//
//    Permission is hereby granted, free of charge, to any person obtaining a copy
//    of this software and associated documentation files (the "Software"), to deal
//    in the Software without restriction, including without limitation the rights
//    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//    copies of the Software, and to permit persons to whom the Software is
//    furnished to do so, subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in all
//    copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//    SOFTWARE.

#ifndef __ACC_TEST_H__
#define __ACC_TEST_H__

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ProTest {

    class AccTestReportFormatter;
    template <class T> class AccTestStep;
    template <class T> class AccTestScenario;

    // Details of a test step

    struct AccTestStepReport {

        AccTestStepReport(const std::string& name, const std::string& description)
        : Name(name), Description(description) {
        }

        AccTestStepReport(const std::string& name, const std::string& description, const std::map<int, std::string> checkOutputs)
        : Name(name), Description(description), CheckOutputs(checkOutputs) {
        }

        std::string Name;
        std::string Description;
        std::map<int, std::string> CheckOutputs;
    };

    // This structure holds test result and other stats after the test scenario is run; it can be retrieved by calling 
    // AccTestScenario::GetReport()

    struct AccTestScenarioReport {
        bool AllPassed = false;
        bool RequiredStepFailure = false;
        bool ExceptionInTestProcedure = false;
        int NumberOfSteps = 0; // Total number of steps within the scenario
        int NumberOfActed = 0; // Number of steps which were acted
        int NumberOfPassed = 0; // Number of steps successfully passed
        int NumberOfFailed = 0; // Number of failed test steps
        int NumberOfOmitted = 0; // Number of test steps omitted because some required prior step failed.
        std::string Name = "N/A";
        std::string Description = "N/A";
        std::vector<AccTestStepReport> OmittedSteps; // Details of the omitted steps
        std::vector<AccTestStepReport> FailedSteps; // Details of the failed steps
        std::vector<AccTestStepReport> PassedSteps; // Details of the passed steps
    };

    // Results of running a test suite is kept within this structure. It is returned by AccTestSuite::Run().

    struct AccTestSuiteReport {
        bool AllPassed = false;
        int NumberOfScenarios = 0;
        int NumberOfPassed = 0;
        int NumberOfFailed = 0;
        int NumberOfTerminated = 0;
        std::vector<AccTestScenarioReport> PassedScenarios;
        std::vector<AccTestScenarioReport> FailedScenarios;
        std::vector<AccTestScenarioReport> TerminatedScenarios;
    };

    // Each test step must inherit AccTestStep and override one or more of the virtual methods. Your derived constructor must 
    // the base constructor and give it the name, description, and the flags isRequired and mustThrow. Turning isRequired on for 
    // test step means that its success is essential for proceeding to subsequent steps. If a required test step fails, all 
    // subsequent steps will be omitted. Turning on the mustThrow flag means that the executing the step Action must throw an 
    // exception. If this flag is raised but the action doesn't throw, the step fails. 
    // You will almost certainly want to override the Act() method. This is where the action happens - some 
    // message is sent to the application, an event is raised, etc. Just make that your implementation of Act() calls SetActed() 
    // at the end.
    // If you are using mock objects in your test, you should override Expect() and put your expectations there. But don't forget
    // to verify them in Verify().
    // You will also, almost very certainly, want to override Verify() to see if the action has had the expected effect on the 
    // context - otherwise, what's the point of testing! If you are using mock objects in your context, make sure to verify all the
    // expectations or reset them before the context is handed over to the unsuspecting next step. Use the protected Check() 
    // method within your Verify() implementation to check if a certain condition is met. If the condition is true the test step is 
    // considered passed. If one of the Check invocations within the test step is given a false value, the whole step is considered
    // failed. You can pass data to the return value from the Check() calls. This data will be sent to the output if the check 
    // should fail.
    // If you need somewhere to initialize the context before running the step, you will need to override Setup(). The finalizing
    // counterpart is, of course, Teardown().

    template <typename T>
    class AccTestStep {
    public:
        typedef T TestContextType;

        AccTestStep(const std::string& name, const std::string& description, bool isRequired = false, bool mustThrow = false)
        : m_Name(name), m_Description(description), m_IsRequired(isRequired), m_MustThrow(mustThrow) {
        }

        virtual void Setup() {
        }

        virtual void Expect() {
        }

        virtual void Act() {
            SetActed();
        }

        virtual void Verify() {
            Check(true) << "All Good";
            ;
        }

        virtual void Teardown() {
        }

        void SetContext(TestContextType* context) {
            m_Context = context;
        }

        bool Passed() {
            return m_Passed;
        }

        bool IsVerified() {
            return m_IsVerified;
        }

        bool HasActed() {
            return m_HasActed;
        }

        bool MustThrow() {
            return m_MustThrow;
        }

        bool IsRequired() {
            return m_IsRequired;
        }

        std::string GetName() {
            return m_Name;
        }

        std::string GetDescription() {
            return m_Description;
        }

        std::map<int, std::string> GetCheckOutputs() {
            std::map<int, std::string> outputs;
            for (auto& strm : m_CheckOutputs)
                outputs[strm.first] = strm.second.str();
            return outputs;
        }

    protected:

        TestContextType* GetTestContext() {
            return m_Context;
        }

        std::ostream& Check(bool predicate) {
            m_Passed = m_CheckCounter > 0 ? m_Passed && predicate : predicate;
            m_IsVerified = true;
            return m_CheckOutputs[m_CheckCounter++];
        }

        void SetActed() {
            m_HasActed = true;
        }

    private:
        bool m_HasActed = false;
        bool m_IsVerified = false;
        bool m_IsRequired = false;
        bool m_MustThrow = false;
        bool m_Passed = false;
        std::string m_Name = "NOT SET";
        std::string m_Description = "NOT SET";
        TestContextType* m_Context = nullptr;
        std::map<int, std::ostringstream> m_CheckOutputs;
        int m_CheckCounter = 0;
    };

    class AccTestScenarioBase {
    public:

        AccTestScenarioBase(const std::string& name, const std::string& description)
        : m_Name(name), m_Description(description) {
        }

        virtual void Run() = 0;
        virtual const AccTestScenarioReport& GetReport() const = 0;

        std::string GetName() {
            return m_Name;
        }

        std::string GetDescription() {
            return m_Description;
        }

    private:
        std::string m_Name = "NOT SET";
        std::string m_Description = "NOT SET";
    };

    // A test scenario which is composed of multiple steps must inherit AccTestScenario. You should create your test steps 
    // within the constructor of your derived class and add them in the same order as you want them to be executed. Use AddStep() 
    // to add the next test step.
    // The test context is created and is accessible within you derived class as GetTestContext.
    // Override Setup and Teardown to provide code for test context initialization and finalization before and after serial 
    // execution of the steps. This is probably where you will want to create you application object and related test stubs, etc. 
    // Don't forget to get rid of everything in Teardown. 
    // When the scenario is run (by a call to Run), first the Setup() method is called once, then all the test steps are executed 
    // in the order they were added, and finally the Teardown() method is called to wrap up the test. Executing each step 
    // involves calling its Setup, Expect, Act, Verify, and Teardown methods in the same order.
    // After executing Run(), you can retrieve the test results using GetReport(). You can use a AccTestReportFormatter to format 
    // the report as text and send it to an output stream.
    // When subclassing, pass the name and description of the test scenario to the base constructor. These information will 
    // subsequently be available using GetName and GetDescription.

    template <class T>
    class AccTestScenario : public AccTestScenarioBase {
    public:
        typedef T TestContextType;

        AccTestScenario(const std::string& name, const std::string& description)
        : AccTestScenarioBase(name, description) {
        }

        void Run() override {
            InitializeReport();
            try {
                RunUnprotected();
            } catch (...) {
                m_Report.ExceptionInTestProcedure = true;
            }
            UpdateReport();
        }

        const AccTestScenarioReport& GetReport() const override {
            return m_Report;
        }

    protected:

        void AddStep(std::shared_ptr< AccTestStep<TestContextType> > step) {
            m_Steps.push_back(step);
        }

        TestContextType* GetTestContext() {
            return &m_TestContext;
        }

    private:

        class ScenarioSetup {
        public:

            ScenarioSetup(AccTestScenario* scenario)
            : m_Scenario(scenario) {
                m_Scenario->Setup();
            }

            ~ScenarioSetup() {
                m_Scenario->Teardown();
            }

        private:
            AccTestScenario<TestContextType>* m_Scenario;
        };

        class StepSetup {
        public:

            StepSetup(AccTestStep<TestContextType>* step)
            : m_Step(step) {
                m_Step->Setup();
            }

            ~StepSetup() {
                if (!m_Step->IsVerified())
                    m_Step->Verify();
                m_Step->Teardown();
            }

        private:
            AccTestStep<TestContextType>* m_Step;
        };

        virtual void Setup() {
        }

        virtual void Teardown() {
        }

        void InitializeReport() {
            m_Report.ExceptionInTestProcedure = false;
            m_Report.FailedSteps.clear();
            m_Report.PassedSteps.clear();
            m_Report.OmittedSteps.clear();
            UpdateReport();
        }

        void UpdateReport() {
            m_Report.Name = GetName();
            m_Report.Description = GetDescription();
            m_Report.NumberOfSteps = m_Steps.size();
            m_Report.NumberOfPassed = m_Report.PassedSteps.size();
            m_Report.NumberOfFailed = m_Report.FailedSteps.size();
            m_Report.NumberOfOmitted = m_Report.OmittedSteps.size();
            m_Report.NumberOfActed = m_Report.NumberOfPassed + m_Report.NumberOfFailed;
            m_Report.RequiredStepFailure = !m_Report.OmittedSteps.empty();
            m_Report.AllPassed = m_Report.NumberOfPassed == m_Report.NumberOfSteps;
        }

        void RunUnprotected() {
            ScenarioSetup scenSetup(this);
            for (const auto& step : m_Steps) {
                if (m_Report.RequiredStepFailure) {
                    m_Report.OmittedSteps.push_back(AccTestStepReport(step->GetName(), step->GetDescription()));
                    continue;
                }
                RunStepUnprotected(step);
            }
        }

        void RunStepUnprotected(const std::shared_ptr< AccTestStep<TestContextType> > step) {
            step->SetContext(&m_TestContext);
            StepSetup stepSetup(step.get());
            step->Expect();
            bool didThrow = false;
            try {
                step->Act();
            } catch (...) {
                didThrow = true;
            }
            bool passedThrowReq = didThrow == step->MustThrow();
            step->Verify();
            if (passedThrowReq && step->Passed()) {
                m_Report.PassedSteps.push_back(AccTestStepReport(step->GetName(), step->GetDescription()));
            } else {
                m_Report.FailedSteps.push_back(AccTestStepReport(step->GetName(), step->GetDescription(), step->GetCheckOutputs()));
                if (step->IsRequired())
                    m_Report.RequiredStepFailure = true;
            }
        }

        std::vector< std::shared_ptr< AccTestStep<TestContextType> > > m_Steps;
        TestContextType m_TestContext;
        AccTestScenarioReport m_Report;
    };

    // In most cases you have more than one test scenario with sequential steps that has its own starting point, steps, and cleanup.
    // To accommodate these situations you can use a test suite which is basically a collection of unrelated test scenarios. Simply
    // inherit AccTestSuite and, within your constructor, create and add your test scenarios using calls to AddScenario. Afterwards,
    // you can run all the scenarios in the test suite by a call to the Run method. This executes all the scenarios in the same 
    // order as they were added. The order shouldn't matter as the test scenarios are supposed to be unrelated, i.e., each scenario
    // has its own Setup which is supposed to set the preconditions regardless of anything else that might have happened before.
    // The Run method returns the complete report for all the test scenarios within the suite.

    class AccTestSuite {
    public:

        AccTestSuiteReport Run() {
            AccTestSuiteReport suiteReport;
            for (auto test : m_Scenarios) {
                test->Run();
                const auto& scenReport = test->GetReport();
                if (scenReport.AllPassed)
                    suiteReport.PassedScenarios.push_back(scenReport);
                else if (scenReport.NumberOfOmitted > 0)
                    suiteReport.TerminatedScenarios.push_back(scenReport);
                else
                    suiteReport.FailedScenarios.push_back(scenReport);
            }
            suiteReport.NumberOfScenarios = m_Scenarios.size();
            suiteReport.NumberOfPassed = suiteReport.NumberOfPassed;
            suiteReport.NumberOfFailed = suiteReport.FailedScenarios.size();
            suiteReport.NumberOfTerminated = suiteReport.TerminatedScenarios.size();
            suiteReport.AllPassed = suiteReport.NumberOfPassed == suiteReport.NumberOfScenarios;
            return suiteReport;
        }

    protected:

        void AddScenario(std::shared_ptr<AccTestScenarioBase> scen) {
            m_Scenarios.push_back(scen);
        }

    private:
        std::vector< std::shared_ptr<AccTestScenarioBase> > m_Scenarios;
    };

    // Use AccTestReportFormatter to use the result from running a test scenario or a complete test suite and generate a text 
    // report and send it to an output stream. You can provide options to indicate whether you want details of the passed, 
    // failed, and terminated test scenarios, and the failed, passed, and omitted test steps for each detailed scenario.
    // The default behaviour is to generate a detailed report only for failed test steps of the failed test scenarios.

    class AccTestReportFormatter {
    public:
        AccTestReportFormatter() = default;

        AccTestReportFormatter(bool detailFailedTests, bool detailPassedTests, bool detailTerminatedTests, 
        bool detailFailedSteps, bool detailPassedSteps, bool detailOmittedSteps)
        : m_DetailPassedSteps(detailPassedSteps),
        m_DetailFailedSteps(detailFailedSteps),
        m_DetailOmittedSteps(detailOmittedSteps),
        m_DetailPassedScenarios(detailPassedTests),
        m_DetailFailedScenarios(detailFailedTests),
        m_DetailTerminatedScenarios(detailTerminatedTests) {
        }

        void GenerateReport(const AccTestSuiteReport& rep, std::ostream& strm = std::cout) {
            strm << "Total number of tests: " << rep.NumberOfScenarios << std::endl;
            if (rep.AllPassed) {
                strm << "*** ALL TESTS PASSED ***" << std::endl;
                if (!m_DetailPassedScenarios)
                    return;
            }

            strm << "*** ONE OR MORE TESTS FAILED ***" << std::endl;

            strm << "Number of failed tests: " << rep.NumberOfFailed << std::endl;
            strm << "Number of passed tests: " << rep.NumberOfPassed << std::endl;
            if (rep.NumberOfTerminated > 0)
                strm << "Number of steps terminated tests: " << rep.NumberOfTerminated << std::endl;

            if (m_DetailFailedScenarios)
                DetailScenarios(strm, rep.FailedScenarios,
                    "\n**********************************************************"
                    "\n********************** FAILED TESTS **********************");

            if (m_DetailPassedScenarios)
                DetailScenarios(strm, rep.PassedScenarios,
                    "\n**********************************************************"
                    "\n********************** PASSED TESTS **********************");

            if (m_DetailOmittedSteps)
                DetailScenarios(strm, rep.TerminatedScenarios,
                    "\n**********************************************************"
                    "\n******************** TERMINATED TESTS ********************");
        }

        void GenerateReport(const AccTestScenarioReport& rep, std::ostream& strm = std::cout) {
            strm << "Scenario name: " << rep.Name << std::endl;
            strm << "Description: " << rep.Description << std::endl;
            strm << "Total number of steps: " << rep.NumberOfSteps << std::endl;
            if (rep.AllPassed) {
                strm << "*** ALL STEPS PASSED ***" << std::endl;
                if (!m_DetailPassedSteps)
                    return;
            }

            strm << "*** ONE OR MORE TEST STEPS FAILED ***" << std::endl;

            if (rep.NumberOfActed != rep.NumberOfSteps)
                strm << "Number of steps taken: " << rep.NumberOfActed << std::endl;

            strm << "Number of steps successfully passed: " << rep.NumberOfPassed << std::endl;
            strm << "Number of steps failed: " << rep.NumberOfFailed << std::endl;

            if (rep.NumberOfOmitted > 0)
                strm << "Number of steps omitted: " << rep.NumberOfOmitted << std::endl;
            if (rep.RequiredStepFailure)
                strm << "** Trailing test steps were omitted because a required step failed. **" << std::endl;

            if (m_DetailFailedSteps)
                DetailSteps(strm, rep.FailedSteps, "\n********************** FAILED STEPS **********************");

            if (m_DetailPassedSteps)
                DetailSteps(strm, rep.PassedSteps, "\n********************** PASSED STEPS **********************");

            if (m_DetailOmittedSteps)
                DetailSteps(strm, rep.OmittedSteps, "\n********************** OMITTED STEPS **********************");
        }

    private:

        void DetailScenarios(std::ostream& strm, const std::vector<AccTestScenarioReport>& reps, const std::string& title) {
            if (reps.empty())
                return;

            strm << title << std::endl << std::endl;
            for (const auto& rep : reps)
                GenerateReport(rep, strm);
        }

        void DetailSteps(std::ostream& strm, const std::vector<AccTestStepReport>& reps, const std::string& title) {
            if (reps.empty())
                return;

            strm << title << std::endl;
            for (const auto& stepRep : reps) {
                strm << "\tName: " << stepRep.Name << std::endl <<
                        "\tDescription: " << stepRep.Description << std::endl;
                for (const auto& output : stepRep.CheckOutputs)
                    strm << "\t\tCheck #" << output.first << " => " << output.second << std::endl;
                strm << std::endl;
            }
        }

        bool m_DetailFailedSteps = true;
        bool m_DetailPassedSteps = false;
        bool m_DetailOmittedSteps = false;
        bool m_DetailFailedScenarios = true;
        bool m_DetailPassedScenarios = false;
        bool m_DetailTerminatedScenarios = false;
    };

    // In the main() function of your test executable you will probably have an instance of AccTestRunner specialized with your 
    // test suite class. AccTestRunner is supposed to take care of you argc and argv. Currently it simply throws them away!
    // You main() function will then call the Run() method and everything else taken care of: like all the test suite is run, and
    // the report passed to the report formatter and printed out to standard output.

    template <class T = AccTestSuite>
    class AccTestRunner {
    public:
        typedef T TestSuiteType;

        AccTestRunner(int argc, char** argv) {
        }

        int Run() {
            TestSuiteType testSuite;
            auto report = testSuite.Run();
            AccTestReportFormatter fmt(true, false, true, true, false, false);
            fmt.GenerateReport(report);
            return report.NumberOfScenarios - report.NumberOfPassed;
        }
    };

} // namespace ProTest

// Use this macro to insert the default main() function for the test program. You can clone the code from here and write your 
// customized main() function as the default is rather limited in options.
#define ACC_TEST_DEFAULT_MAIN_FUNC(TEST_SUITE_NAME)  \
int main (int argc, char** argv)  \
{   \
    return ProTest::AccTestRunner<TEST_SUITE_NAME>(argc, argv).Run();    \
}

#endif // __ACC_TEST_H__

// Use this macro in your implementation of Verify() within the test steps to check for equality of two values with suitable 
// failure output.
#define ACC_TEST_CHECK_EQUAL(LEFT, RIGHT) \
Check((LEFT) == (RIGHT)) << "NOT EQUAL: "#LEFT" = " << (LEFT) << ", "#RIGHT" = " << (RIGHT)
