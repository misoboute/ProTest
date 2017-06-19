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
#include <memory>
#include <string>
#include <vector>

namespace ProTest {

    class AccTestReportFormatter;
    template <class T> class AccTestStep;
    template <class T> class AccTestScenario;

    // This structure holds test result and other stats. After the test scenario is run, it can be retrieved by calling 
    // AccTestScenario::GetReport()

    struct AccTestReport {
        // Details of a test step

        struct Step {

            Step(const std::string& name, const std::string& description)
            : Name(name), Description(description) {
            }
            std::string Name;
            std::string Description;
        };

        bool AllPassed = false;
        bool RequiredStepFailure = false;
        bool ExceptionInTestProcedure = false;
        int NumberOfSteps = 0; // Total number of steps within the scenario
        int NumberOfActed = 0; // Number of steps which were acted
        int NumberOfPassed = 0; // Number of steps successfully passed
        int NumberOfFailed = 0; // Number of failed test steps
        int NumberOfOmitted = 0; // Number of test steps omitted because some required prior step failed.
        std::vector<Step> OmittedSteps; // Details of the omitted steps
        std::vector<Step> FailedSteps; // Details of the failed steps
        std::vector<Step> PassedSteps; // Details of the passed steps
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
    // expectations or reset them before the context is handed over to the unsuspecting next step. Also make sure to call 
    // SetSuccess to indicate whether you consider the test passed or not.
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
            SetPassed(true);
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

    protected:

        TestContextType* GetTestContext() {
            return m_Context;
        }

        void SetPassed(bool success) {
            m_IsVerified = true;
            m_Passed = success;
        }

        void SetActed() {
            m_HasActed = true;
        }

        void SetVerified() {
            m_IsVerified = true;
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
    };

    // The main test scenario which is composed of multiple steps must inherit AccTestScenario. You should create your test steps 
    // within the constructor your derived class and add them in the same order as you want them to be executed. Use AddStep() 
    // to add the next test step.
    // The test context is created and is accessible within you derived class as GetTestContext.
    // Override Setup and Teardown to provide code for test context initialization and finalization before and after serial 
    // execution of the steps. This is probably where you will want to create you application object and related test stubs, etc. 
    // Don't forget to get rid of everything in Teardown. 
    // When the scenario is run (by a call to Run), first the Setup() method is called once, then all the test steps are executed 
    // in the order they were added, and finally the Teardown() method is called to wrap up the test. Executing each steps 
    // calling their Setup, Expect, Act, Verify, and Teardown methods in the same order.
    // After executing Run(), you can retrieve the test results using GetReport(). You can use a AccTestReportFormatter to format 
    // the report as text and send it to an output stream.

    template <class T>
    class AccTestScenario {
    public:
        typedef T TestContextType;

        void Run() {
            InitializeReport();
            try {
                RunUnprotected();
            } catch (...) {
                m_Report.ExceptionInTestProcedure = true;
            }
            UpdateReport();
        }

        const AccTestReport& GetReport() const {
            return m_Report;
        }

    protected:

        virtual void Setup() {
        }

        virtual void Teardown() {
        }

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

        void InitializeReport() {
            m_Report.ExceptionInTestProcedure = false;
            m_Report.FailedSteps.clear();
            m_Report.PassedSteps.clear();
            m_Report.OmittedSteps.clear();
            UpdateReport();
        }

        void UpdateReport() {
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
                    m_Report.OmittedSteps.push_back(AccTestReport::Step(step->GetName(), step->GetDescription()));
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
            if (passedThrowReq)
                step->Verify();
            if (passedThrowReq && step->Passed()) {
                m_Report.PassedSteps.push_back(AccTestReport::Step(step->GetName(), step->GetDescription()));
            } else {
                m_Report.FailedSteps.push_back(AccTestReport::Step(step->GetName(), step->GetDescription()));
                if (step->IsRequired())
                    m_Report.RequiredStepFailure = true;
            }
        }

        std::vector< std::shared_ptr< AccTestStep<TestContextType> > > m_Steps;
        TestContextType m_TestContext;
        AccTestReport m_Report;
    };

    // Use AccTestReportFormatter to use the result from running a test scenario and generate a text report and send it to an 
    // output stream. You can provide options to indicate whether you want details of the passed, failed, and omitted test steps.
    // The default behaviour is to generate a detailed report only for failed test steps.

    class AccTestReportFormatter {
    public:
        AccTestReportFormatter() = default;

        AccTestReportFormatter(bool detailPassed, bool detailFailed, bool detailOmitted)
        : m_DetailPassedSteps(detailPassed), m_DetailFailedSteps(detailFailed), m_DetailOmittedSteps(detailOmitted) {
        }

        void GenerateReport(const AccTestReport& rep, std::ostream& strm = std::cout) {
            strm << "Total number of steps: " << rep.NumberOfSteps << std::endl;
            if (rep.AllPassed) {
                strm << "*** ALL TESTS PASSED ***" << std::endl;
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

            if (m_DetailFailedSteps && !rep.FailedSteps.empty()) {
                strm << "\n********************** FAILED STEPS **********************" << std::endl;
                DetailSteps(strm, rep.FailedSteps);
            }

            if (m_DetailPassedSteps && !rep.PassedSteps.empty()) {
                strm << "\n********************** PASSED STEPS **********************" << std::endl;
                DetailSteps(strm, rep.PassedSteps);
            }

            if (m_DetailOmittedSteps && !rep.OmittedSteps.empty()) {

                strm << "\n********************** OMITTED STEPS **********************" << std::endl;
                DetailSteps(strm, rep.OmittedSteps);
            }
        }

    private:

        void DetailSteps(std::ostream& strm, const std::vector<AccTestReport::Step>& reps) {
            for (const auto& stepRep : reps)
                strm << "\tName: " << stepRep.Name << std::endl <<
                    "\tDescription: " << stepRep.Description << std::endl << std::endl;
        }

        bool m_DetailFailedSteps = true;
        bool m_DetailPassedSteps = false;
        bool m_DetailOmittedSteps = false;
    };

} // namespace ProTest

// Use this macro to insert the default main() function for the test program. You can clone the code from here and write your 
// customized main() function as the default is rather limited in options.
#define ACC_TEST_DEFAULT_MAIN_FUNC(TEST_SCENARIO_NAME)  \
int main ()  \
{   \
    TEST_SCENARIO_NAME test;    \
    test.Run(); \
    const auto& report = test.GetReport();  \
    AccTestReportFormatter fmt;    \
    fmt.GenerateReport(report); \
    if (report.RequiredStepFailure) \
        return 2;   \
    else if (report.AllPassed)  \
        return 0;   \
    return 1;   \
}

#endif // __ACC_TEST_H__
