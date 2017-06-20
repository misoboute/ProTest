# ProTest
A minimalistic test framework written in C++ providing scenario based, stateful acceptance testing where application user
interface is mocked using test stubs. Refer to Sample.cpp for usage guidance.
The sample defines a simple application class, fake user interface implementing the same abstract interface as the application
requires, a test context to hold test state and pass it on among steps, and a few test steps. It then adds all the test steps
to a test scenario. Then the default() main function is inserted. If you compile, link and execute the Sample, you will have 
an idea of what you should expect when you have finished writing your test.

Features:
- Acceptance testing immediately below UI
- Compatible with mocking frameworks (GMOCK, turtle, etc)
- Stateful scenario steps (each step picks up where the last one left)
- Suitable for applications with extensive requirements where post-conditions for one provide pre-conditions for another
- Massive code reuse: no need to build up the preconditions for each test case from scratch as each step is built on top of the 
  last
- Faster test runs: in acceptance testing, building the preconditions for each test case from scratch could be very time 
  consuming - mutiply it by the number of test cases that share the same context. Here the state is saved and passed between 
  test steps saving context building time during test runs.
- Header only: you don't need to build ProTest separately and link it to your test application. Just include the header and 
  that's all.
- Light-weight
- Easily readable and fairly customizable test reports
