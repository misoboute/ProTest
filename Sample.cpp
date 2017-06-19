/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <algorithm>
#include <cctype>
#include <sstream>

// Include the test framework header.
#include "AccTest.h"

// All framework elements are isolated inside ProTest namespace.
using namespace ProTest;

// Abstract interface for the UI connected to our application under test.

class ICalcUserInterface {
public:
    virtual void SetStatusBar(const std::string& msg) = 0;
    virtual void SetResultContents(const std::string& msg) = 0;
    virtual void SetTitleBar(const std::string& msg) = 0;
    virtual void SetAddButtonCallBack(std::function<void () > callBack) = 0;
    virtual void SetSubtractButtonCallBack(std::function<void () > callBack) = 0;
    virtual std::string GetTextBoxContents() = 0;
    virtual void Close() = 0;
};

// This is the main application to be tested. A pointer to the UI is passed to it during construction. We shall take advantage 
// of this to pass in our own (fake, stub, mock, whatever) version of the UI. This is the only requirement for the application 
// test. Database connections, IO, network, etc could be replaced with simulators or stubs might be used for them as well.

class MyCalcApplication {
public:

    MyCalcApplication(ICalcUserInterface* ui)
    : m_UI(ui) {
        m_UI->SetAddButtonCallBack([this]() {
            Add(); });
        m_UI->SetSubtractButtonCallBack([this]() {
            Subtract(); });
    }

    void StartUp() {
        m_UI->SetTitleBar("My Calculator");
        m_UI->SetStatusBar("Ready");
        m_UI->SetResultContents("0");
    }

    void Exit() {
        m_UI->Close();
    }

private:

    void Add() {
        int value;
        if (!ParseTextBox(value))
            return;

        std::ostringstream output;
        m_CurrentResult += value;
        output << m_CurrentResult;
        m_UI->SetResultContents(output.str());
        m_UI->SetStatusBar("Ready");
    }

    void Subtract() {
        int value;
        if (!ParseTextBox(value))
            return;

        std::ostringstream output;
        m_CurrentResult -= value;
        output << m_CurrentResult;
        m_UI->SetResultContents(output.str());
        m_UI->SetStatusBar("Ready");
    }

    bool ParseTextBox(int& value) {
        auto text = m_UI->GetTextBoxContents();
        if (std::find_if(text.cbegin(), text.cend(), [](char ch) {
                return !std::isdigit(ch); }) != text.cend()) {
        m_UI->SetStatusBar("Error");
        return false;
    }
        std::istringstream input(text);
        input >> value;
        return true;
    }

    int m_CurrentResult = 0;
    ICalcUserInterface* m_UI;
};

// Our fake version of the user interface implementing the same abstract interface but only used in test.

class FakeCaclUserInterface : public ICalcUserInterface {
public:

    void SetAddButtonCallBack(std::function<void () > callBack) override {
        m_AddButtonCallBack = callBack;
    }

    void SetSubtractButtonCallBack(std::function<void () > callBack) override {
        m_SubtractButtonCallBack = callBack;
    }

    std::string GetTextBoxContents() override {
        return m_TextBoxContents;
    }

    void SetStatusBar(const std::string& msg) override {
        m_StatusBar = msg;
    }

    void SetResultContents(const std::string& msg) override {
        m_ResultContents = msg;
    }

    void SetTitleBar(const std::string& msg) override {
        m_TitleBar = msg;
    }

    void Close() override {
        if (m_ExpectedCallsToClose > 0)
            --m_ExpectedCallsToClose;
        else
            throw "Unexpected call to Close()";
    }

    void ExpectClose(int cardinality) {
        m_ExpectedCallsToClose += cardinality;
    }

    bool VerifyExpectedClose() {
        return m_ExpectedCallsToClose == 0;
    }

    std::string m_TitleBar, m_ResultContents, m_StatusBar, m_TextBoxContents;
    std::function<void() > m_AddButtonCallBack, m_SubtractButtonCallBack;

private:
    int m_ExpectedCallsToClose = 0;
};

// The test context is required to hold state of the application, test stubs, and other test related data. It is initialized
// within the test scenario Setup, used and affected by each step, passed on to the next step, and the disposed within the test
// test scenario Teardown.

struct CalcTestContext {
    FakeCaclUserInterface* UI;
    MyCalcApplication* App;
};

// Each step of the test scenario inherits AccTestStep<T> where T is the type serving as our text context.
// Apart from that, every step class will probably need to override the Act() and Verify() methods. Some test steps might also
// need to override Setup(), Expect(), and Teardown(). See AccTestStep for more info.

class TestStep1InitApp : public AccTestStep<CalcTestContext> {
public:

    TestStep1InitApp()
    : AccTestStep<CalcTestContext>(
    "TestStep1InitApp", "When the app is initialized title bar, status bar, and contents must be set correctly.", false, false) {
    }

    void Act() override {
        auto ctx = GetTestContext();
        ctx->App->StartUp();
    }

    void Verify() override {
        auto ui = GetTestContext()->UI;
        bool success = ui->m_TitleBar == "My Calculator" && ui->m_StatusBar == "Ready" && ui->m_ResultContents == "0";
        SetPassed(success);
    }
};

// Another test step

class TestStep2Add10ToResult : public AccTestStep<CalcTestContext> {
public:

    TestStep2Add10ToResult()
    : AccTestStep<CalcTestContext>("TestStep2Adding10ToResult",
    "When the number 10 is put in and Add button pressed, status bar must show Ready, and result must show 10.", false, false) {
    }

    void Act() override {
        auto ui = GetTestContext()->UI;
        ui->m_TextBoxContents = "10";
        ui->m_AddButtonCallBack();
    }

    void Verify() override {
        auto ui = GetTestContext()->UI;
        bool success = ui->m_StatusBar == "Ready" && ui->m_ResultContents == "10";
        SetPassed(success);
    }
};

// Another test step

class TestStep3Add20ToResult : public AccTestStep<CalcTestContext> {
public:

    TestStep3Add20ToResult()
    : AccTestStep<CalcTestContext>("TestStep3Adding20ToResult",
    "When the number 20 is put in and Add button pressed, status bar must show Ready, and result must show 30.", false, false) {
    }

    void Act() override {
        auto ui = GetTestContext()->UI;
        ui->m_TextBoxContents = "20";
        ui->m_AddButtonCallBack();
    }

    void Verify() override {
        auto ui = GetTestContext()->UI;
        bool success = ui->m_StatusBar == "Ready" && ui->m_ResultContents == "30";
        SetPassed(success);
    }
};

// Another test step

class TestStep4Subtract15MustYield15 : public AccTestStep<CalcTestContext> {
public:

    TestStep4Subtract15MustYield15()
    : AccTestStep<CalcTestContext>("TestStep4Subtract15MustYield15",
    "When the number 15 is put in and Subtract button pressed, status bar must show Ready, and result must show 15.", false, false) {
    }

    void Act() override {
        auto ui = GetTestContext()->UI;
        ui->m_TextBoxContents = "15";
        ui->m_SubtractButtonCallBack();
    }

    void Verify() override {
        auto ui = GetTestContext()->UI;
        bool success = ui->m_StatusBar == "Ready" && ui->m_ResultContents == "15";
        SetPassed(success);
    }
};

// Another test step

class TestStep5Subtract7MustYield8 : public AccTestStep<CalcTestContext> {
public:

    TestStep5Subtract7MustYield8()
    : AccTestStep<CalcTestContext>("TestStep5Subtract7MustYield8",
    "When the number 7 is put in and Subtract button pressed, status bar must show Ready, and result must show 8.", false, false) {
    }

    void Act() override {
        auto ui = GetTestContext()->UI;
        ui->m_TextBoxContents = "7";
        ui->m_SubtractButtonCallBack();
    }

    void Verify() override {
        auto ui = GetTestContext()->UI;
        bool success = ui->m_StatusBar == "Ready" && ui->m_ResultContents == "8";
        SetPassed(success);
    }
};

// Another test step

class TestStep6Add52MustYield60 : public AccTestStep<CalcTestContext> {
public:

    TestStep6Add52MustYield60()
    : AccTestStep<CalcTestContext>("TestStep6Add52MustYield60",
    "When the number 52 is put in and Add button pressed, status bar must show Ready, and result must show 60.", false, false) {
    }

    void Act() override {
        auto ui = GetTestContext()->UI;
        ui->m_TextBoxContents = "52";
        ui->m_AddButtonCallBack();
    }

    void Verify() override {
        auto ui = GetTestContext()->UI;
        bool success = ui->m_StatusBar == "Ready" && ui->m_ResultContents == "60";
        SetPassed(success);
    }
};

// Another test step

class TestStep7AddNonNumericMustGetError : public AccTestStep<CalcTestContext> {
public:

    TestStep7AddNonNumericMustGetError()
    : AccTestStep<CalcTestContext>("TestStep7AddNonNumericMustGetError",
    "When a non-numerical string is put in and Add button pressed, status bar must show Error, and result must still be showing 60.", false, false) {
    }

    void Act() override {
        auto ui = GetTestContext()->UI;
        ui->m_TextBoxContents = "sn3wfsf";
        ui->m_AddButtonCallBack();
    }

    void Verify() override {
        auto ui = GetTestContext()->UI;
        bool success = ui->m_StatusBar == "Error" && ui->m_ResultContents == "60";
        SetPassed(success);
    }
};

// Another test step

class TestStep8Subtract23MustShow37 : public AccTestStep<CalcTestContext> {
public:

    TestStep8Subtract23MustShow37()
    : AccTestStep<CalcTestContext>("TestStep8Subtract23MustShow37",
    "When a when 23 is put in and Subtract button pressed, status bar must show Ready, and result must show 37.", false, false) {
    }

    void Act() override {
        auto ui = GetTestContext()->UI;
        ui->m_TextBoxContents = "23";
        ui->m_SubtractButtonCallBack();
    }

    void Verify() override {
        auto ui = GetTestContext()->UI;
        bool success = ui->m_StatusBar == "Ready" && ui->m_ResultContents == "37";
        SetPassed(success);
    }
};

// Another test step

class TestStep9SubtractNonNumericMustGetError : public AccTestStep<CalcTestContext> {
public:

    TestStep9SubtractNonNumericMustGetError()
    : AccTestStep<CalcTestContext>("TestStep9SubtractNonNumericMustGetError",
    "When a non-numerical string is put in and Subtract button pressed, "
    "status bar must show Error, and result must still be showing 37.", false, false) {
    }

    void Act() override {
        auto ui = GetTestContext()->UI;
        ui->m_TextBoxContents = "3wfsf";
        ui->m_SubtractButtonCallBack();
    }

    void Verify() override {
        auto ui = GetTestContext()->UI;
        bool success = ui->m_StatusBar == "Error" && ui->m_ResultContents == "37";
        SetPassed(success);
    }
};

// Another test step

class TestStep10Add32MustShow69 : public AccTestStep<CalcTestContext> {
public:

    TestStep10Add32MustShow69()
    : AccTestStep<CalcTestContext>("TestStep10Add32MustShow69",
    "When the number 32 is put in and Add button pressed, status bar must show Ready, and result must show 69.", false, false) {
    }

    void Act() override {
        auto ui = GetTestContext()->UI;
        ui->m_TextBoxContents = "32";
        ui->m_AddButtonCallBack();
    }

    void Verify() override {
        auto ui = GetTestContext()->UI;
        bool success = ui->m_StatusBar == "Ready" && ui->m_ResultContents == "69";
        SetPassed(success);
    }
};

// Another test step

class TestStep11WhenExitingAppTheUIMustBeClosed : public AccTestStep<CalcTestContext> {
public:

    TestStep11WhenExitingAppTheUIMustBeClosed()
    : AccTestStep<CalcTestContext>("TestStep11WhenExitingAppTheUIMustBeClosed",
    "When close request is sent to the app, "
    "the Close() from user interface must be called once.", false, false) {
    }

    void Expect() override {
        auto ui = GetTestContext()->UI;
        ui->ExpectClose(1);
    }

    void Act() override {
        auto app = GetTestContext()->App;
        app->Exit();
    }

    void Verify() override {
        auto ui = GetTestContext()->UI;
        bool success = ui->VerifyExpectedClose();
        SetPassed(ui->VerifyExpectedClose());
    }
};

// Each test scenario, which is usually composed of many steps, must inherit AccTestScenario. During construction, it must add 
// its respective test steps using AddStep in the same order which they must be run. The test scenario creates the context for you,
// but you need to create the application object and other test stubs. This can be done by overriding Setup(). Make sure to perform
// any clean-ups required within Teardown() by overriding it.

class MyTestScenario : public AccTestScenario<CalcTestContext> {
public:

    MyTestScenario() {
        AddStep(std::make_shared<TestStep1InitApp>());
        AddStep(std::make_shared<TestStep2Add10ToResult>());
        AddStep(std::make_shared<TestStep3Add20ToResult>());
        AddStep(std::make_shared<TestStep4Subtract15MustYield15>());
        AddStep(std::make_shared<TestStep5Subtract7MustYield8>());
        AddStep(std::make_shared<TestStep6Add52MustYield60>());
        AddStep(std::make_shared<TestStep7AddNonNumericMustGetError>());
        AddStep(std::make_shared<TestStep8Subtract23MustShow37>());
        AddStep(std::make_shared<TestStep9SubtractNonNumericMustGetError>());
        AddStep(std::make_shared<TestStep10Add32MustShow69>());
        AddStep(std::make_shared<TestStep11WhenExitingAppTheUIMustBeClosed>());
    }

    void Setup() override {
        auto ctx = GetTestContext();
        ctx->UI = new FakeCaclUserInterface();
        ctx->App = new MyCalcApplication(ctx->UI);
    }

    void Teardown() override {
        auto ctx = GetTestContext();
        delete ctx->App;
        delete ctx->UI;
    }

};

// Use the default main() function. You could also clone macro contents and create your own main() function to customize your test
// application. Currently, argc and argv parameters are ignored by the default main() function.
ACC_TEST_DEFAULT_MAIN_FUNC(MyTestScenario)